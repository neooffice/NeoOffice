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
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#
#   Modified November 2016 by Patrick Luby. NeoOffice is only distributed
#   under the GNU General Public License, Version 3 as allowed by Section 3.3
#   of the Mozilla Public License, v. 2.0.
#
#   You should have received a copy of the GNU General Public License
#   along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

$(eval $(call gb_Library_Library,ipt))

$(eval $(call gb_Library_use_external,ipt,boost_headers))

$(eval $(call gb_Library_use_sdk_api,ipt))

$(eval $(call gb_Library_use_libraries,ipt,\
	vcl \
	tl \
	basegfx \
	sal \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_add_exception_objects,ipt,\
	filter/source/graphicfilter/ipict/ipict \
	filter/source/graphicfilter/ipict/shape \
))

ifeq ($(strip $(GUIBASE)),java)
$(eval $(call gb_Library_use_system_darwin_frameworks,ipt,\
	ApplicationServices \
))
endif	# GUIBASE == java

# vim: set noet sw=4 ts=4:
