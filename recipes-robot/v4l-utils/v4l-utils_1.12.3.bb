SUMMARY = "v4l2 and IR applications"
LICENSE = "GPLv2 & LGPLv2.1"
LIC_FILES_CHKSUM = "file://COPYING;md5=48da9957849056017dc568bbc43d8975 \
                    file://COPYING.libv4l;md5=d749e86a105281d7a44c2328acebc4b0"
PROVIDES = "libv4l media-ctl"

DEPENDS = "jpeg \
           ${@bb.utils.contains('DISTRO_FEATURES', 'x11', 'virtual/libx11', '', d)}"
DEPENDS_append_libc-musl = " argp-standalone"
DEPENDS_append_class-target = " udev"
LDFLAGS_append = " -pthread"

inherit autotools gettext pkgconfig

PACKAGECONFIG ??= "media-ctl"
PACKAGECONFIG[media-ctl] = "--enable-v4l-utils,--disable-v4l-utils,,"

SRC_URI = "http://linuxtv.org/downloads/v4l-utils/v4l-utils-${PV}.tar.bz2 \
           file://0001-Revert-media-ctl-Don-t-install-libmediactl-and-libv4.patch \
           file://0002-buildsystem-do-not-assume-building-in-source-tree.patch \
           file://0003-mediactl-pkgconfig.patch \
           file://0004-export-mediactl-headers.patch \
           file://0005-ir-ctl-Define-TEMP_FAILURE_RETRY-if-undefined.patch \
           file://0006-contrib-test-Link-mc_nextgen_test-with-libargp-if-ne.patch \
           file://0007-v4l2-ctl-Do-not-use-getsubopt.patch \
           "
SRC_URI[md5sum] = "89e1ed6c69c94e0489dc0a638c7841aa"
SRC_URI[sha256sum] = "5a47dd6f0e7dfe902d94605c01d385a4a4e87583ff5856d6f181900ea81cf46e"

EXTRA_OECONF = "--disable-qv4l2 --enable-shared --with-udevdir=${base_libdir}/udev"

VIRTUAL-RUNTIME_ir-keytable-keymaps ?= "rc-keymaps"

PACKAGES =+ "media-ctl ir-keytable rc-keymaps libv4l libv4l-dbg libv4l-dev"

FILES_media-ctl = "${bindir}/media-ctl ${libdir}/libmediactl.so.*"

FILES_ir-keytable = "${bindir}/ir-keytable ${base_libdir}/udev/rules.d/*-infrared.rules"
RDEPENDS_ir-keytable += "${VIRTUAL-RUNTIME_ir-keytable-keymaps}"

FILES_rc-keymaps = "${sysconfdir}/rc* ${base_libdir}/udev/rc*"

FILES_${PN} = "${bindir} ${sbindir}"

FILES_libv4l += "${libdir}/libv4l*${SOLIBS} ${libdir}/libv4l/*.so ${libdir}/libv4l/plugins/*.so \
                 ${libdir}/libdvbv5*${SOLIBS} \
                 ${libdir}/libv4l/*-decomp"

FILES_libv4l-dbg += "${libdir}/libv4l/.debug ${libdir}/libv4l/plugins/.debug"
FILES_libv4l-dev += "${includedir} ${libdir}/pkgconfig \
                     ${libdir}/libv4l*${SOLIBSDEV} ${libdir}/*.la \
                     ${libdir}/v4l*${SOLIBSDEV} ${libdir}/libv4l/*.la ${libdir}/libv4l/plugins/*.la"

PARALLEL_MAKE_class-native = ""
BBCLASSEXTEND = "native"
