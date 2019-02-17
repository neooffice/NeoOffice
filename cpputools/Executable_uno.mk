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
#   Modified January 2017 by Patrick Luby. NeoOffice is only distributed
#   under the GNU General Public License, Version 3 as allowed by Section 3.3
#   of the Mozilla Public License, v. 2.0.
#
#   You should have received a copy of the GNU General Public License
#   along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

$(eval $(call gb_Executable_Executable,uno))

$(eval $(call gb_Executable_use_internal_comprehensive_api,uno,\
    udkapi \
))

$(eval $(call gb_Executable_use_libraries,uno,\
    cppu \
    cppuhelper \
    sal \
    salhelper \
))

$(eval $(call gb_Executable_use_externals,uno,\
    libxml2 \
))

$(eval $(call gb_Executable_add_exception_objects,uno,\
    cpputools/source/unoexe/unoexe \
))

ifeq ($(strip $(GUIBASE)),java)
$(eval $(call gb_Executable_add_objcxxobjects,uno,\
    cpputools/source/unoexe/unoexe_cocoa \
))

$(eval $(call gb_Executable_use_system_darwin_frameworks,uno,\
    AppKit \
))
endif	# GUIBASE == java

# vim:set noet sw=4 ts=4:
