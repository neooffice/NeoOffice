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
#   Modified November 2016 by Patrick Luby. NeoOffice is only distributed
#   under the GNU General Public License, Version 3 as allowed by Section 3.3
#   of the Mozilla Public License, v. 2.0.
#
#   You should have received a copy of the GNU General Public License
#   along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

$(eval $(call gb_CppunitTest_CppunitTest,dbaccess_nolib_save))

$(eval $(call gb_CppunitTest_add_exception_objects,dbaccess_nolib_save, \
    dbaccess/qa/extras/nolib-save \
))

$(eval $(call gb_CppunitTest_use_externals,dbaccess_nolib_save,\
    boost_headers \
))

$(eval $(call gb_CppunitTest_use_libraries,dbaccess_nolib_save, \
    basegfx \
    comphelper \
    cppu \
    cppuhelper \
    dba \
    dbu \
    sdbt \
    drawinglayer \
    editeng \
    for \
    forui \
    i18nlangtag \
    msfilter \
    oox \
    sal \
    salhelper \
    sax \
    sb \
    sfx \
    sot \
    svl \
    svt \
    svx \
    svxcore \
    test \
    subsequenttest \
    tl \
    tk \
    ucbhelper \
	unotest \
    utl \
    vbahelper \
    vcl \
    xo \
))

$(eval $(call gb_CppunitTest_use_api,dbaccess_nolib_save,\
    offapi \
    oovbaapi \
    udkapi \
))

$(eval $(call gb_CppunitTest_use_ure,dbaccess_nolib_save))
$(eval $(call gb_CppunitTest_use_vcl,dbaccess_nolib_save))

$(eval $(call gb_CppunitTest_use_components,dbaccess_nolib_save,\
    basic/util/sb \
    comphelper/util/comphelp \
    configmgr/source/configmgr \
    dbaccess/util/dba \
    dbaccess/util/dbu \
    dbaccess/util/sdbt \
    dbaccess/source/filter/xml/dbaxml \
    filter/source/config/cache/filterconfig1 \
    forms/util/frm \
    framework/util/fwk \
    i18npool/util/i18npool \
    linguistic/source/lng \
    oox/util/oox \
    package/source/xstor/xstor \
    package/util/package2 \
    sax/source/expatwrap/expwrap \
    scripting/source/basprov/basprov \
    scripting/util/scriptframe \
    sfx2/util/sfx \
    sot/util/sot \
    svl/source/fsstor/fsstorage \
    svl/util/svl \
    toolkit/util/tk \
    ucb/source/core/ucb1 \
    ucb/source/ucp/file/ucpfile1 \
    ucb/source/ucp/tdoc/ucptdoc1 \
    unotools/util/utl \
    unoxml/source/rdf/unordf \
    unoxml/source/service/unoxml \
    uui/util/uui \
    xmloff/util/xo \
))

$(eval $(call gb_CppunitTest_use_configuration,dbaccess_nolib_save))

$(call gb_CppunitTest_get_target,dbaccess_nolib_save) : $(WORKDIR)/CppunitTest/testNolibSave.odb
$(WORKDIR)/CppunitTest/testNolibSave.odb : $(SRCDIR)/dbaccess/qa/extras/testdocuments/testDialogSave.odb
	mkdir -p $(dir $@)
ifeq ($(strip $(PRODUCT_BUILD_TYPE)),java)
	rm -f "$@"
endif	# PRODUCT_BUILD_TYPE == java
	cp -P -f "$<" "$@"
.PHONY: $(WORKDIR)/CppunitTest/testNolibSave.odb

$(call gb_CppunitTest_get_target,dbaccess_nolib_save): \
    $(call gb_AllLangResTarget_get_target,ofa)

# vim: set noet sw=4 ts=4:
