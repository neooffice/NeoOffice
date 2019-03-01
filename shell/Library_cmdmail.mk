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

$(eval $(call gb_Library_Library,cmdmail))

$(eval $(call gb_Library_use_sdk_api,cmdmail))

$(eval $(call gb_Library_use_libraries,cmdmail,\
	cppu \
	cppuhelper \
	sal \
))

$(eval $(call gb_Library_set_componentfile,cmdmail,shell/source/cmdmail/cmdmail))

$(eval $(call gb_Library_add_exception_objects,cmdmail,\
    shell/source/cmdmail/cmdmailentry \
    shell/source/cmdmail/cmdmailmsg \
    shell/source/cmdmail/cmdmailsuppl \
))

ifeq ($(strip $(GUIBASE)),java)
$(eval $(call gb_Library_add_objcxxobjects,cmdmail,\
    shell/source/cmdmail/cmdmailsuppl_cocoa \
))

$(eval $(call gb_Library_use_system_darwin_frameworks,cmdmail,\
	AppKit \
))
endif	# GUIBASE == java

# vim: set shiftwidth=4 tabstop=4 noexpandtab:
