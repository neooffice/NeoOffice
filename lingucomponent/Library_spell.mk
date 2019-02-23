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

$(eval $(call gb_Library_Library,spell))

$(eval $(call gb_Library_set_componentfile,spell,lingucomponent/source/spellcheck/spell/spell))

$(eval $(call gb_Library_set_include,spell,\
	$$(INCLUDE) \
	-I$(SRCDIR)/lingucomponent/source/lingutil \
))

ifneq ($(strip $(PRODUCT_NAME)),)
$(eval $(call gb_Library_add_defs,spell,\
	-DPRODUCT_NAME='"$(PRODUCT_NAME)"' \
))
endif	# PRODUCT_NAME != ""

$(eval $(call gb_Library_use_sdk_api,spell))

$(eval $(call gb_Library_use_libraries,spell,\
	comphelper \
	cppu \
	cppuhelper \
	i18nlangtag \
	lng \
	sal \
	tl \
	utl \
))

$(eval $(call gb_Library_use_static_libraries,spell,\
	ulingu \
))

$(eval $(call gb_Library_use_externals,spell,\
	boost_headers \
	hunspell \
	icuuc \
))

$(eval $(call gb_Library_add_exception_objects,spell,\
	lingucomponent/source/spellcheck/spell/sreg \
	lingucomponent/source/spellcheck/spell/sspellimp \
))

ifeq ($(strip $(GUIBASE)),java)
$(eval $(call gb_Library_add_objcxxobjects,spell,\
	lingucomponent/source/spellcheck/spell/sspellimp_cocoa \
))

$(eval $(call gb_Library_use_system_darwin_frameworks,spell,\
	AppKit \
))
endif	# GUIBASE == java

# vim: set noet sw=4 ts=4:
