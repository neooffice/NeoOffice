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

$(eval $(call gb_Executable_Executable,soffice2_bin))

$(eval $(call gb_Executable_set_targettype_gui,soffice2_bin,YES))

$(eval $(call gb_Executable_set_include,soffice2_bin,\
    $$(INCLUDE) \
    -I$(SRCDIR)/desktop/source/inc \
))

ifneq ($(strip $(PRODUCT_NAME)),)
$(eval $(call gb_Executable_add_defs,soffice2_bin,\
    -DPRODUCT_NAME='"$(PRODUCT_NAME)"' \
))
endif	# PRODUCT_NAME != ""

ifneq ($(strip $(PRODUCT_MIN_OSVERSION)),)
$(eval $(call gb_Executable_add_defs,soffice2_bin,\
    -DPRODUCT_MIN_OSVERSION='"$(PRODUCT_MIN_OSVERSION)"' \
))
endif	# PRODUCT_MIN_OSVERSION != ""

ifneq ($(strip $(PRODUCT_MAX_OSVERSION)),)
$(eval $(call gb_Executable_add_defs,soffice2_bin,\
    -DPRODUCT_MAX_OSVERSION='"$(PRODUCT_MAX_OSVERSION)"' \
))
endif	# PRODUCT_MAX_OSVERSION != ""

ifneq ($(strip $(GUIBASE)),java)
$(eval $(call gb_Executable_use_libraries,soffice2_bin,\
    sal \
    sofficeapp \
	$(gb_UWINAPI) \
))
endif	# GUIBASE != java

$(eval $(call gb_Executable_add_cobjects,soffice2_bin,\
    desktop/source/app/main2 \
))

ifeq ($(strip $(GUIBASE)),java)
$(eval $(call gb_Executable_add_objcxxobjects,soffice2_bin,\
    desktop/source/app/main_java2 \
))

$(eval $(call gb_Executable_add_cobjects,soffice2_bin,\
    desktop/source/app/main_java_init2 \
))
endif	# GUIBASE == java

ifeq ($(OS),MACOSX)

$(eval $(call gb_Executable_set_ldflags,\
    $(filter-out -bind_at_load,$$(LDFLAGS)) \
))

endif

ifeq ($(strip $(GUIBASE)),java)
$(eval $(call gb_Executable_use_system_darwin_frameworks,soffice2_bin,\
    AppKit \
))
endif	# GUIBASE == java

ifeq ($(OS),WNT)

$(eval $(call gb_Executable_use_static_libraries,soffice2_bin,\
    ooopathutils \
    winextendloaderenv \
))

$(eval $(call gb_Executable_set_targettype_gui,soffice2_bin,YES))

$(eval $(call gb_Executable_add_nativeres,soffice2_bin,sofficebin/officeloader))

ifeq ($(COM),MSC)

$(eval $(call gb_Executable_add_ldflags,soffice2_bin,\
    /STACK:10000000 \
))

endif

endif

# vim: set ts=4 sw=4 et:
