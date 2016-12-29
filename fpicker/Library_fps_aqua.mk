# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
# 
#   Modified November 2016 by Patrick Luby. NeoOffice is only distributed
#   under the GNU General Public License, Version 3 as allowed by Section 3.3
#   of the Mozilla Public License, v. 2.0.
#
#   You should have received a copy of the GNU General Public License
#   along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

$(eval $(call gb_Library_Library,fps_aqua))

$(eval $(call gb_Library_set_componentfile,fps_aqua,fpicker/source/aqua/fps_aqua))

$(eval $(call gb_Library_use_external,fps_aqua,boost_headers))

$(eval $(call gb_Library_use_sdk_api,fps_aqua))

$(eval $(call gb_Library_use_system_darwin_frameworks,fps_aqua,\
    Cocoa \
    CoreFoundation \
))

$(eval $(call gb_Library_use_libraries,fps_aqua,\
	cppu \
	cppuhelper \
	i18nlangtag \
	sal \
	tl \
	vcl \
))

ifeq ($(strip $(GUIBASE)),java)

$(eval $(call gb_Library_add_objcxxobjects,fps_aqua,\
	fpicker/source/aqua/CFStringUtilities \
	fpicker/source/aqua/FPentry \
	fpicker/source/java/cocoa_dialog \
	fpicker/source/java/java_filepicker \
	fpicker/source/java/java_folderpicker \
))

else	# GUIBASE == java

$(eval $(call gb_Library_add_objcxxobjects,fps_aqua,\
	fpicker/source/aqua/AquaFilePickerDelegate \
	fpicker/source/aqua/CFStringUtilities \
	fpicker/source/aqua/ControlHelper \
	fpicker/source/aqua/FilterHelper \
	fpicker/source/aqua/FPentry \
	fpicker/source/aqua/NSString_OOoAdditions \
	fpicker/source/aqua/NSURL_OOoAdditions \
	fpicker/source/aqua/resourceprovider \
	fpicker/source/aqua/SalAquaFilePicker \
	fpicker/source/aqua/SalAquaFolderPicker \
	fpicker/source/aqua/SalAquaPicker \
))

endif	# GUIBASE == java

# vim: set noet sw=4 ts=4:
