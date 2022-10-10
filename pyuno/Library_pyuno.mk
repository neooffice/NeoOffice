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

$(eval $(call gb_Library_Library,pyuno))

$(eval $(call gb_Library_set_include,pyuno,\
    -I$(SRCDIR)/pyuno/source/loader \
    -I$(SRCDIR)/pyuno/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_Library_add_defs,pyuno,\
	-DLO_DLLIMPLEMENTATION_PYUNO \
))

$(eval $(call gb_Library_use_api,pyuno,\
    udkapi \
))

$(eval $(call gb_Library_use_libraries,pyuno,\
    cppu \
    cppuhelper \
    sal \
    salhelper \
    $(if $(filter $(PRODUCT_BUILD_TYPE),java),tl) \
))

$(eval $(call gb_Library_use_externals,pyuno,\
	boost_headers \
    python \
))

$(eval $(call gb_Library_add_exception_objects,pyuno,\
    pyuno/source/module/pyuno_runtime \
    pyuno/source/module/pyuno \
    pyuno/source/module/pyuno_callable \
    pyuno/source/module/pyuno_module \
    pyuno/source/module/pyuno_type \
    pyuno/source/module/pyuno_util \
    pyuno/source/module/pyuno_except \
    pyuno/source/module/pyuno_adapter \
    pyuno/source/module/pyuno_gc \
))

# vim:set noet sw=4 ts=4:
