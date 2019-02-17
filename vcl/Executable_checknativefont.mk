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
#   Modified December 2016 by Patrick Luby. NeoOffice is only distributed
#   under the GNU General Public License, Version 3 as allowed by Section 3.3
#   of the Mozilla Public License, v. 2.0.
#
#   You should have received a copy of the GNU General Public License
#   along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

ifeq ($(strip $(GUIBASE)),java)

$(eval $(call gb_Executable_Executable,checknativefont))

$(eval $(call gb_Executable_add_cobjects,checknativefont,\
    vcl/java/source/gdi/salchecknativefont \
))

$(eval $(call gb_Executable_use_system_darwin_frameworks,checknativefont,\
    CoreFoundation \
    CoreText \
))

endif	# GUIBASE == java

# vim:set noet sw=4 ts=4:
