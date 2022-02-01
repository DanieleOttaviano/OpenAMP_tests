/*Include Section*/
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <time.h>
#include <fcntl.h>
#include <linux/rpmsg.h>
#include <stdint.h>

#include "list-test-kernel.h"
#include "tasks-managment.h"

/*Static variables*/
static int charfd = -1, fd = -1, err_cnt;

#define RPMSG_GET_KFIFO_SIZE      1
#define RPMSG_GET_AVAIL_DATA_SIZE 2
#define RPMSG_GET_FREE_SPACE      3

#define RPMSG_HEADER_LEN    16
#define MAX_RPMSG_BUFF_SIZE (512 - RPMSG_HEADER_LEN)

//RPMSG path in file system
#define RPMSG_BUS_SYS "/sys/bus/rpmsg"

static int rpmsg_create_ept(int rpfd, struct rpmsg_endpoint_info *eptinfo)
{
	int ret;

	ret = ioctl(rpfd, RPMSG_CREATE_EPT_IOCTL, eptinfo);
	if (ret)
		perror("Failed to create endpoint.\n");
	return ret;
}

static char *get_rpmsg_ept_dev_name(const char *rpmsg_char_name,
				    const char *ept_name,
				    char *ept_dev_name)
{
	char sys_rpmsg_ept_name_path[64];
	char svc_name[64];
	char *sys_rpmsg_path = "/sys/class/rpmsg";
	FILE *fp;
	int i;
	int ept_name_len;

	for (i = 0; i < 128; i++) {
		sprintf(sys_rpmsg_ept_name_path, "%s/%s/rpmsg%d/name",
			sys_rpmsg_path, rpmsg_char_name, i);
		printf("checking %s\n", sys_rpmsg_ept_name_path);
		if (access(sys_rpmsg_ept_name_path, F_OK) < 0)
			continue;
		fp = fopen(sys_rpmsg_ept_name_path, "r");
		if (!fp) {
			printf("failed to open %s\n", sys_rpmsg_ept_name_path);
			break;
		}
		fgets(svc_name, sizeof(svc_name), fp);
		fclose(fp);
		printf("svc_name: %s.\n",svc_name);
		ept_name_len = strlen(ept_name);
		if (ept_name_len > sizeof(svc_name))
			ept_name_len = sizeof(svc_name);
		if (!strncmp(svc_name, ept_name, ept_name_len)) {
			sprintf(ept_dev_name, "rpmsg%d", i);
			return ept_dev_name;
		}
	}

	printf("Not able to RPMsg endpoint file for %s:%s.\n",
	       rpmsg_char_name, ept_name);
	return NULL;
}

static int bind_rpmsg_chrdev(const char *rpmsg_dev_name)
{
	char fpath[256];
	char *rpmsg_chdrv = "rpmsg_chrdev";
	int fd;
	int ret;


	/* rpmsg dev overrides path */
	sprintf(fpath, "%s/devices/%s/driver_override",
		RPMSG_BUS_SYS, rpmsg_dev_name);
	fd = open(fpath, O_WRONLY);
	if (fd < 0) {
		fprintf(stderr, "Failed to open %s, %s\n",
			fpath, strerror(errno));
		return -EINVAL;
	}
	ret = write(fd, rpmsg_chdrv, strlen(rpmsg_chdrv) + 1);
	if (ret < 0) {
		fprintf(stderr, "Failed to write %s to %s, %s\n",
			rpmsg_chdrv, fpath, strerror(errno));
		return -EINVAL;
	}
	close(fd);

	/* bind the rpmsg device to rpmsg char driver */
	sprintf(fpath, "%s/drivers/%s/bind", RPMSG_BUS_SYS, rpmsg_chdrv);
	fd = open(fpath, O_WRONLY);
	if (fd < 0) {
		fprintf(stderr, "Failed to open %s, %s\n",
			fpath, strerror(errno));
		return -EINVAL;
	}
	ret = write(fd, rpmsg_dev_name, strlen(rpmsg_dev_name) + 1);
	if (ret < 0) {
		fprintf(stderr, "Failed to write %s to %s, %s\n",
			rpmsg_dev_name, fpath, strerror(errno));
		return -EINVAL;
	}
	close(fd);
	return 0;
}

static int get_rpmsg_chrdev_fd(const char *rpmsg_dev_name,
			       char *rpmsg_ctrl_name)
{
	char dpath[256];
	char fpath[256];
	char *rpmsg_ctrl_prefix = "rpmsg_ctrl";
	DIR *dir;
	struct dirent *ent;
	int fd;

	sprintf(dpath, "%s/devices/%s/rpmsg", RPMSG_BUS_SYS, rpmsg_dev_name);
	dir = opendir(dpath);
	if (dir == NULL) {
		fprintf(stderr, "Failed to open dir %s\n", dpath);
		return -EINVAL;
	}
	while ((ent = readdir(dir)) != NULL) {
		if (!strncmp(ent->d_name, rpmsg_ctrl_prefix,
			    strlen(rpmsg_ctrl_prefix))) {
			printf("Opening file %s.\n", ent->d_name);
			sprintf(fpath, "/dev/%s", ent->d_name);
			fd = open(fpath, O_RDWR | O_NONBLOCK);
			if (fd < 0) {
				fprintf(stderr,
					"Failed to open rpmsg char dev %s,%s\n",
					fpath, strerror(errno));
				return fd;
			}
			sprintf(rpmsg_ctrl_name, "%s", ent->d_name);
			return fd;
		}
	}

	fprintf(stderr, "No rpmsg char dev file is found\n");
	return -EINVAL;
}

int main(int argc, char *argv[])
{
	int ret, i, j, n;
	char *rpmsg_dev="virtio0.rpmsg-openamp-demo-channel.-1.0";
	char fpath[256];
	char rpmsg_char_name[16];
	struct rpmsg_endpoint_info eptinfo;
	char ept_dev_name[16];
	char ept_dev_path[32];
	char data_payload[MAX_DATA_LENGHT];

	printf("\r\n[APU]: test start \r\n");

	/*********** FASE INIZIALIZZAZIONE SISTEMA E CREAZIONE CANALE DI COMUNICAZIONE ********************/

	/* Load rpmsg_char driver */
	printf("\r\n[APU]: Master>probe rpmsg_char\r\n");
	ret = system("modprobe rpmsg_char");
	if (ret < 0) {
		perror("Failed to load rpmsg_char driver.\n");
		return -EINVAL;
	}

	printf("\r\n [APU]: Open rpmsg dev %s! \r\n", rpmsg_dev);
	sprintf(fpath, "%s/devices/%s", RPMSG_BUS_SYS, rpmsg_dev);
	if (access(fpath, F_OK)) {
		fprintf(stderr, "[APU]: Not able to access rpmsg device %s, %s\n",
			fpath, strerror(errno));
		return -EINVAL;
	}
	ret = bind_rpmsg_chrdev(rpmsg_dev);
	if (ret < 0)
		return ret;
	charfd = get_rpmsg_chrdev_fd(rpmsg_dev, rpmsg_char_name);
	if (charfd < 0)
		return charfd;

	/* Create endpoint from rpmsg char driver */
	strcpy(eptinfo.name, "rpmsg-openamp-demo-channel");
	eptinfo.src = 0;
	eptinfo.dst = 0xFFFFFFFF;
	ret = rpmsg_create_ept(charfd, &eptinfo);
	if (ret) {
		printf("[APU]: failed to create RPMsg endpoint.\n");
		return -EINVAL;
	}
	if (!get_rpmsg_ept_dev_name(rpmsg_char_name, eptinfo.name,
				    ept_dev_name))
		return -EINVAL;
	sprintf(ept_dev_path, "/dev/%s", ept_dev_name);
	fd = open(ept_dev_path, O_RDWR | O_NONBLOCK);
	if (fd < 0) {
		perror("Failed to open rpmsg device.");
		close(charfd);
		return -1;
	}

	//sistema correttamente inizializzato, è possibile iniziare il test
	printf("\r\n **********************************");
	printf("****\r\n");
	printf("\r\n Test Start APU<->RPU \r\n");
	printf("\r\n **********************************");
	printf("****\r\n");

	struct List *Lista = init_list();

	sprintf(data_payload, "MESSAGGIO 1 DA APU AD RPU");
	printf("\n[APU]: Request for Task 1\n");	
	if(task_offloading(TASK_1, data_payload, Lista, fd) != 0){
		perror("[APU] Failed to create a new Task");
	}
	usleep(10000);

	sprintf(data_payload, "MESSAGGIO 2 DA APU AD RPU");
	printf("\n[APU]: Request for Task 2\n");	
	if(task_offloading(TASK_2, data_payload, Lista, fd) != 0){
		perror("[APU]: Failed to create a new Task");
	}
	usleep(10000);

	sprintf(data_payload, "MESSAGGIO 3 DA APU AD RPU");
	printf("\n[APU]: Request for Task 3\n");	
	if(task_offloading(TASK_3, data_payload, Lista, fd) != 0){
		perror("[APU]: Failed to create a new Task");
	}

	sprintf(data_payload, "MESSAGGIO 4 DA APU AD RPU");
	printf("\n[APU]: Request for Task 4\n");	
	if(task_offloading(TASK_4, data_payload, Lista, fd) != 0){
		perror("[APU]: Failed to create a new Task");
	}

	sprintf(data_payload, "MESSAGGIO 5 DA APU AD RPU");
	printf("\n[APU]: Request for Task 5\n");	
	if(task_offloading(TASK_5, data_payload, Lista, fd) != 0){
		perror("[APU]: Failed to create a new Task");
	}

	sprintf(data_payload, "MESSAGGIO 6 DA APU AD RPU");
	printf("\n[APU]: Request for Task 6\n");	
	if(task_offloading(0x12345678, data_payload, Lista, fd) != 0){
		perror("[APU]: Failed to create a new Task");
	}

	printf("[APU]: List:\n\n");
	print_list(Lista);

	/*	STOP EXAMPLE
	printf("[APU]: STOP TASK 2\n");
	stop_task(2,Lista, fd);

	printf("[APU]: List:\n\n");
	print_list(Lista);
	*/

	//Attesa 
	while(true){
		printf("[APU]: APU processors alive!\n");
		usleep(10000);
		//unfeasible task request 
		//if(task_offloading(TASK_1, data_payload, Lista, fd) != 0){
			//perror("[APU]: Failed to create a new Task");
		//}
	}
	
	shutdown_RPU(Lista, fd);

	close(fd);
	if (charfd >= 0)
		close(charfd);
	return 0;
}

