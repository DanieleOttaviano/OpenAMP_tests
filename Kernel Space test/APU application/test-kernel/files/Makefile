
APP = test-kernel
APP_OBJS = test-kernel.o list-test-kernel.o tasks-managment.o

# Add any other object files to this list below


all: $(APP)

$(APP): $(APP_OBJS)
	$(CC) $(LDFLAGS) -o $@ $(APP_OBJS) $(LDLIBS) -lm

clean:
	rm -rf $(APP) *.o

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

