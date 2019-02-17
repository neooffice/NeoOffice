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

$(eval $(call gb_Library_Library,updchk))

$(eval $(call gb_Library_set_componentfile,updchk,extensions/source/update/check/updchk.uno))

$(eval $(call gb_Library_set_include,updchk,\
	$$(INCLUDE) \
	-I$(SRCDIR)/extensions/inc \
))

ifeq ($(strip $(GUIBASE)),java)
$(eval $(call gb_Library_set_include,updchk,\
	$$(INCLUDE) \
	-I$(LIBO_SRCDIR)/vcl/inc \
))
endif	# GUIBASE == java

$(eval $(call gb_Library_use_sdk_api,updchk))

$(eval $(call gb_Library_use_libraries,updchk,\
	cppuhelper \
	cppu \
	sal \
	salhelper \
))

ifeq ($(strip $(PRODUCT_BUILD_TYPE)),java)
$(eval $(call gb_Library_use_libraries,updchk,\
	i18nlangtag \
	tl \
	utl \
	vcl \
))
endif	# PRODUCT_BUILD_TYPE == java

ifeq ($(OS),WNT)
$(eval $(call gb_Library_use_system_win32_libs,updchk,\
	ole32 \
	shell32 \
	wininet \
))

$(eval $(call gb_Library_add_exception_objects,updchk,\
	extensions/source/update/check/onlinecheck \
))
endif # OS WNT

$(eval $(call gb_Library_use_externals,updchk,\
	boost_headers \
    curl \
))

$(eval $(call gb_Library_add_exception_objects,updchk,\
	extensions/source/update/check/download \
	extensions/source/update/check/updatecheck \
	extensions/source/update/check/updatecheckconfig \
	extensions/source/update/check/updatecheckjob \
	extensions/source/update/check/updatehdl \
	extensions/source/update/check/updateprotocol \
))

ifeq ($(strip $(PRODUCT_BUILD_TYPE)),java)
$(eval $(call gb_Library_add_exception_objects,updchk,\
	extensions/source/update/check/update_java \
))
endif	# PRODUCT_BUILD_TYPE == java

ifeq ($(strip $(GUIBASE)),java)
$(eval $(call gb_Library_add_objcxxobjects,updchk,\
	extensions/source/update/check/update_cocoa \
	extensions/source/update/check/updatei18n_cocoa \
	extensions/source/update/check/updatewebview_cocoa \
))

$(eval $(call gb_Library_use_system_darwin_frameworks,updchk,\
    Cocoa \
    WebKit \
))
endif	# GUIBASE == java

# vim:set noet sw=4 ts=4:
