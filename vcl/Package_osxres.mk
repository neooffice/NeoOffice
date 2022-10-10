# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Modified December 2016 by Patrick Luby. NeoOffice is only distributed
#   under the GNU General Public License, Version 3 as allowed by Section 3.3
#   of the Mozilla Public License, v. 2.0.
#
#   You should have received a copy of the GNU General Public License
#   along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

ifeq ($(strip $(GUIBASE)),java)

$(eval $(call gb_Package_Package,vcl_osxres,$(SRCDIR)/vcl/java/source/res))

$(eval $(call gb_Package_add_files_with_dir,vcl_osxres,Resources,\
    MainMenu.nib/classes.nib \
    MainMenu.nib/designable.nib \
    MainMenu.nib/info.nib \
    MainMenu.nib/keyedobjects.nib \
    cursors/airbrush.png \
    cursors/ase.png \
    cursors/asn.png \
    cursors/asne.png \
    cursors/asns.png \
    cursors/asnswe.png \
    cursors/asnw.png \
    cursors/ass.png \
    cursors/asse.png \
    cursors/assw.png \
    cursors/asw.png \
    cursors/aswe.png \
    cursors/chain.png \
    cursors/chainnot.png \
    cursors/chart.png \
    cursors/copydata.png \
    cursors/copydlnk.png \
    cursors/copyf.png \
    cursors/copyf2.png \
    cursors/copyflnk.png \
    cursors/crook.png \
    cursors/crop.png \
    cursors/cross.png \
    cursors/detectiv.png \
    cursors/fill.png \
    cursors/hand.png \
    cursors/help.png \
    cursors/hshear.png \
    cursors/hsize.png \
    cursors/hsizebar.png \
    cursors/hsplit.png \
    cursors/linkdata.png \
    cursors/linkf.png \
    cursors/magnify.png \
    cursors/mirror.png \
    cursors/move.png \
    cursors/movebw.png \
    cursors/movedata.png \
    cursors/movedlnk.png \
    cursors/movef.png \
    cursors/movef2.png \
    cursors/moveflnk.png \
    cursors/movept.png \
    cursors/neswsize.png \
    cursors/notallow.png \
    cursors/nullptr.png \
    cursors/nwsesize.png \
    cursors/pen.png \
    cursors/pivotcol.png \
    cursors/pivotdel.png \
    cursors/pivotfld.png \
    cursors/pivotrow.png \
    cursors/pntbrsh.png \
    cursors/refhand.png \
    cursors/rotate.png \
    cursors/tblsele.png \
    cursors/tblsels.png \
    cursors/tblselse.png \
    cursors/tblselsw.png \
    cursors/tblselw.png \
    cursors/timemove.png \
    cursors/timesize.png \
    cursors/vshear.png \
    cursors/vsize.png \
    cursors/vsizebar.png \
    cursors/vsplit.png \
))

else	# GUIBASE == java

$(eval $(call gb_Package_Package,vcl_osxres,$(SRCDIR)/vcl/osx/res))

$(eval $(call gb_Package_add_files_with_dir,vcl_osxres,Resources,\
    MainMenu.nib/classes.nib \
    MainMenu.nib/info.nib \
    MainMenu.nib/keyedobjects.nib \
    cursors/airbrush.png \
    cursors/ase.png \
    cursors/asn.png \
    cursors/asne.png \
    cursors/asns.png \
    cursors/asnswe.png \
    cursors/asnw.png \
    cursors/ass.png \
    cursors/asse.png \
    cursors/assw.png \
    cursors/asw.png \
    cursors/aswe.png \
    cursors/chain.png \
    cursors/chainnot.png \
    cursors/chart.png \
    cursors/copydata.png \
    cursors/copydlnk.png \
    cursors/copyf.png \
    cursors/copyf2.png \
    cursors/copyflnk.png \
    cursors/crook.png \
    cursors/crop.png \
    cursors/darc.png \
    cursors/dbezier.png \
    cursors/dcapt.png \
    cursors/dcirccut.png \
    cursors/dconnect.png \
    cursors/dellipse.png \
    cursors/detectiv.png \
    cursors/dfree.png \
    cursors/dline.png \
    cursors/dpie.png \
    cursors/dpolygon.png \
    cursors/drect.png \
    cursors/dtext.png \
    cursors/fill.png \
    cursors/help.png \
    cursors/hourglass.png \
    cursors/hshear.png \
    cursors/linkdata.png \
    cursors/linkf.png \
    cursors/magnify.png \
    cursors/mirror.png \
    cursors/movebw.png \
    cursors/movedata.png \
    cursors/movedlnk.png \
    cursors/movef.png \
    cursors/movef2.png \
    cursors/moveflnk.png \
    cursors/movept.png \
    cursors/neswsize.png \
    cursors/notallow.png \
    cursors/nullptr.png \
    cursors/nwsesize.png \
    cursors/pen.png \
    cursors/pivotcol.png \
    cursors/pivotdel.png \
    cursors/pivotfld.png \
    cursors/pivotrow.png \
    cursors/pntbrsh.png \
    cursors/rotate.png \
    cursors/tblsele.png \
    cursors/tblsels.png \
    cursors/tblselse.png \
    cursors/tblselsw.png \
    cursors/tblselw.png \
    cursors/timemove.png \
    cursors/timesize.png \
    cursors/vshear.png \
    cursors/vtext.png \
))

endif	# GUIBASE == java

# vim:set noet sw=4 ts=4:
