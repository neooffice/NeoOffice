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

$(eval $(call gb_Library_Library,syssh))

$(eval $(call gb_Library_use_sdk_api,syssh))

$(eval $(call gb_Library_use_libraries,syssh,\
	cppu \
	cppuhelper \
	sal \
))

ifeq ($(OS),WNT)

$(eval $(call gb_Library_use_system_win32_libs,syssh,\
	ole32 \
	shell32 \
))

$(eval $(call gb_Library_set_componentfile,syssh,shell/source/win32/syssh))

$(eval $(call gb_Library_add_exception_objects,syssh,\
	shell/source/win32/SysShExec \
	shell/source/win32/SysShentry \
))

else # OS != WNT

$(eval $(call gb_Library_use_static_libraries,syssh,\
	shell_xmlparser \
))

$(eval $(call gb_Library_set_componentfile,syssh,shell/source/unix/exec/syssh))

$(eval $(call gb_Library_add_exception_objects,syssh,\
	shell/source/unix/exec/shellexec \
	shell/source/unix/exec/shellexecentry \
))

ifeq ($(strip $(GUIBASE)),java)
$(eval $(call gb_Library_add_objcxxobjects,syssh,\
	shell/source/unix/exec/shellexec_cocoa \
))

$(eval $(call gb_Library_use_system_darwin_frameworks,syssh,\
	AppKit \
))
endif	# GUIBASE == java

endif # OS

# vim: set shiftwidth=4 tabstop=4 noexpandtab:
