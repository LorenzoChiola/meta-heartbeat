DESCRIPTION = "Heartbeat: heart rate monitor."
LICENSE = "GPLv3"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/GPL-3.0;md5=c79ff39f19dfec6d293b95dea7b07891"
SRC_URI = "file://heartbeat.c"
S = "${WORKDIR}"

# Examples:
# https://wiki.yoctoproject.org/wiki/Cookbook:Appliance:Startup_Scripts

do_compile() {
	set CFLAGS -g
	${CC} ${CFLAGS} heartbeat.c ${LDFLAGS} -o heartbeat -lm -Wall -O2
	unset CFLAGS
}

do_install() {
	install -d ${D}${bindir}/
	install -m 0755 heartbeat ${D}${bindir}/
}
