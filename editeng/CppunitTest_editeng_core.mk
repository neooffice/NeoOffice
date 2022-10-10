# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#*************************************************************************
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
#*************************************************************************

$(eval $(call gb_CppunitTest_CppunitTest,editeng_core))

$(eval $(call gb_CppunitTest_add_exception_objects,editeng_core, \
    editeng/qa/unit/core-test \
))

$(eval $(call gb_CppunitTest_use_library_objects,editeng_core,editeng))

$(eval $(call gb_CppunitTest_use_libraries,editeng_core, \
    basegfx \
    comphelper \
    cppu \
    cppuhelper \
    i18nlangtag \
    i18nutil \
    lng \
    sal \
    salhelper \
    sax \
    sot \
    svl \
    svt \
    test \
    tk \
    tl \
    ucbhelper \
    utl \
    vcl \
    xo \
	$(gb_UWINAPI) \
))

ifeq ($(strip $(GUIBASE)),java)
$(eval $(call gb_CppunitTest_use_system_darwin_frameworks,editeng_core,\
	CoreFoundation \
))
endif	# GUIBASE == java

$(eval $(call gb_CppunitTest_use_externals,editeng_core,\
	boost_headers \
    icuuc \
))

$(eval $(call gb_CppunitTest_set_include,editeng_core,\
    -I$(SRCDIR)/editeng/source \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_api,editeng_core,\
    offapi \
    udkapi \
))

$(eval $(call gb_CppunitTest_use_ure,editeng_core))
$(eval $(call gb_CppunitTest_use_vcl,editeng_core))

$(eval $(call gb_CppunitTest_use_components,editeng_core,\
    configmgr/source/configmgr \
    framework/util/fwk \
    i18npool/util/i18npool \
	i18npool/source/search/i18nsearch \
    linguistic/source/lng \
    sfx2/util/sfx \
    ucb/source/core/ucb1 \
    ucb/source/ucp/file/ucpfile1 \
    unoxml/source/service/unoxml \
))

$(eval $(call gb_CppunitTest_use_configuration,editeng_core))

# vim: set noet sw=4 ts=4:
