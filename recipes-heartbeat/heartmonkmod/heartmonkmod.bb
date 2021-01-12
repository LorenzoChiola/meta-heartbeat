DESCRIPTION = "Kernel module for the heart rate monitor."
LICENSE = "GPLv3"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/GPL-3.0;md5=c79ff39f19dfec6d293b95dea7b07891"

inherit module
COMPATIBLE_MACHINE = "qemuarm"

SRC_URI = "file://heartmonkmod.c file://data.h file://Makefile"
S = "${WORKDIR}"

# Examples:
# https://wiki.yoctoproject.org/wiki/Cookbook:Appliance:Startup_Scripts
