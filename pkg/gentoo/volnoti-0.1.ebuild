# Copyright 1999-2011 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v3
# $Header: /var/cvsroot/gentoo-x86/media-sound/volti/volnoti-0.1.ebuild $

EAPI=4

inherit toolchain-funcs

SLOT="0"
MY_P="${PN}-${PV}"
IUSE=""

DESCRIPTION="Lightweight volume notification"
HOMEPAGE="https://github.com/davidbrazdil/volnoti"
SRC_URI="mirror://github/davidbrazdil/volnoti/${MY_P}.tar.gz"

LICENSE="GPL-3"
KEYWORDS="~amd64 ~x86"

RDEPEND="sys-apps/dbus
         dev-libs/dbus-glib
         x11-libs/gtk+
         x11-libs/gdk-pixbuf"
DEPEND="dev-util/pkgconfig"

S=${WORKDIR}/${MY_P}

src_compile() {
	tc-export CC
	emake
}
