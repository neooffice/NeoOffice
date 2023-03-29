# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
$(eval $(call gb_CppunitTest_CppunitTest,filter_pict_test))

$(eval $(call gb_CppunitTest_use_externals,filter_pict_test,\
	boost_headers \
	libxml2 \
))

$(eval $(call gb_CppunitTest_add_exception_objects,filter_pict_test, \
    filter/qa/cppunit/filters-pict-test \
))

$(eval $(call gb_CppunitTest_use_libraries,filter_pict_test, \
	basegfx \
	sal \
	test \
	tl \
	unotest \
	vcl \
	$(gb_UWINAPI) \
))

$(eval $(call gb_CppunitTest_use_library_objects,filter_pict_test, \
    ipt \
))

$(eval $(call gb_CppunitTest_use_api,filter_pict_test,\
    udkapi \
    offapi \
))

$(eval $(call gb_CppunitTest_use_ure,filter_pict_test))
$(eval $(call gb_CppunitTest_use_vcl,filter_pict_test))

$(eval $(call gb_CppunitTest_use_components,filter_pict_test,\
    configmgr/source/configmgr \
    i18npool/util/i18npool \
))

ifeq ($(strip $(GUIBASE)),java)
$(eval $(call gb_CppunitTest_use_system_darwin_frameworks,filter_pict_test,\
	ApplicationServices \
))
endif	# GUIBASE == java

$(eval $(call gb_CppunitTest_use_configuration,filter_pict_test))

# vim: set noet sw=4 ts=4:
