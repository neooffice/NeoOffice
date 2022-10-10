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
#

$(eval $(call gb_CppunitTest_CppunitTest,extensions_test_update))

$(eval $(call gb_CppunitTest_add_exception_objects,extensions_test_update, \
	extensions/qa/update/test_update \
))

$(eval $(call gb_CppunitTest_use_external,extensions_test_update,boost_headers))

$(eval $(call gb_CppunitTest_use_libraries,extensions_test_update, \
	cppu \
	cppuhelper \
	sal \
	salhelper \
	test \
	unotest \
	$(gb_UWINAPI) \
))

ifeq ($(strip $(PRODUCT_BUILD_TYPE)),java)
$(eval $(call gb_CppunitTest_use_libraries,extensions_test_update, \
	i18nlangtag \
	tl \
	utl \
	vcl \
))
endif	# PRODUCT_BUILD_TYPE == java

$(eval $(call gb_CppunitTest_use_library_objects,extensions_test_update, \
	updchk \
))

ifeq ($(OS),WNT)
$(eval $(call gb_CppunitTest_use_system_win32_libs,extensions_test_update,\
	shell32 \
	ole32 \
	wininet \
))
endif

$(eval $(call gb_CppunitTest_use_external,extensions_test_update,curl))

$(eval $(call gb_CppunitTest_set_include,extensions_test_update,\
	$$(INCLUDE) \
	-I$(SRCDIR)/extensions/inc \
))

$(eval $(call gb_CppunitTest_use_api,extensions_test_update,\
	offapi \
	udkapi \
))

$(eval $(call gb_CppunitTest_use_ure,extensions_test_update))
$(eval $(call gb_CppunitTest_use_vcl,extensions_test_update))

$(eval $(call gb_CppunitTest_use_components,extensions_test_update,\
    configmgr/source/configmgr \
    extensions/source/update/feed/updatefeed \
    ucb/source/core/ucb1 \
    ucb/source/ucp/file/ucpfile1 \
    unoxml/source/service/unoxml \
))

ifeq ($(strip $(GUIBASE)),java)
$(eval $(call gb_CppunitTest_use_system_darwin_frameworks,extensions_test_update,\
    Cocoa \
    WebKit \
))
endif	# GUIBASE == java

$(eval $(call gb_CppunitTest_use_configuration,extensions_test_update))

# vim:set shiftwidth=4 softtabstop=4 noexpandtab:
