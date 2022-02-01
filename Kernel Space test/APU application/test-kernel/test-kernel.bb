#
# This file is the test-kernel recipe.
#

SUMMARY = "Simple test-kernel application"
SECTION = "PETALINUX/apps"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = "\
	file://LICENSE \
	file://Makefile \
	file://test-kernel.c \
	file://list-test-kernel.c \
	file://list-test-kernel.h \
	file://tasks-managment.c \
	file://tasks-managment.h \
	"

S = "${WORKDIR}"

RRECOMMENDS_${PN} = "kernel-module-rpmsg-char"

FILES_${PN} = "\
	/usr/bin/test-kernel\
"
do_install() {
	     install -d ${D}/usr/bin
	     install -m 0755 ${S}/test-kernel ${D}/usr/bin/test-kernel
}
