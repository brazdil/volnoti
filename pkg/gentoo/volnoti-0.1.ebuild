# Copyright 1999-2011 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v3
# $Header: /var/cvsroot/gentoo-x86/media-sound/volti/volnoti-0.1.ebuild $

EAPI=3

inherit toolchain-funcs multilib

SLOT="0"
MY_P="${PN}-${PV}"

DESCRIPTION="Lightweight volume notification"
HOMEPAGE="https://github.com/davidbrazdil/volnoti"
SRC_URI="https://github.com/downloads/davidbrazdil/volnoti/${MY_P}.tar.gz"

LICENSE="GPL-3"
KEYWORDS="amd64 x86"

RDEPEND="sys-apps/dbus
         dev-libs/dbus-glib
         x11-libs/gtk+
         x11-libs/gdk-pixbuf"
DEPEND="sys-devel/make
        dev-util/pkgconfig
        sys-devel/gcc"

S=${WORKDIR}/${MY_P}

src_unpack() {
	unpack ${A}
	cd "${S}"
}

src_compile() {
	tc-export CC
	emake || die "emake failed"
}

src_install() {
	emake DESTDIR="${D}" install || die "emake install failed"
	dodoc README || die
}
