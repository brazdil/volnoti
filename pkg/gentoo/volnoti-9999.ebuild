# Copyright 1999-2011 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v3
# $Header: /var/cvsroot/gentoo-x86/media-sound/volti/volnoti-0.1.ebuild $

EAPI=5

inherit git-r3
inherit autotools

SLOT="0"
MY_P="${PN}-${PV}"
IUSE=""

DESCRIPTION="Lightweight volume notification"
HOMEPAGE="https://github.com/davidbrazdil/volnoti"
EGIT_REPO_URI="https://github.com/davidbrazdil/volnoti.git"

LICENSE="GPL-3"
KEYWORDS="~amd64 ~x86"

RDEPEND="sys-apps/dbus
         dev-libs/dbus-glib
         x11-libs/gtk+
         x11-libs/gdk-pixbuf"
DEPEND="dev-util/pkgconfig
		dev-libs/dbus-glib"  # needed for dbus-binding-tool

S=${WORKDIR}/${MY_P}

src_prepare() {
	eautoreconf --force --install
	./configure --prefix="${D}/usr"
	echo "Updating src/value-client-stub.h with dbus-binding-tool"
	dbus-binding-tool --prefix=volume_object --mode=glib-client \
		src/specs.xml > src/value-client-stub.h
	echo "Updating src/value-daemon-stub.h with dbus-binding-tool"
	dbus-binding-tool --prefix=volume_object --mode=glib-server \
		src/specs.xml > src/value-daemon-stub.h
}

src_compile() {
	emake
}
