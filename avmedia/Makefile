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
#   Modified October 2016 by Patrick Luby. NeoOffice is only distributed
#   under the GNU General Public License, Version 3 as allowed by Section 3.3
#   of the Mozilla Public License, v. 2.0.
#
#   You should have received a copy of the GNU General Public License
#   along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

module_directory:=$(dir $(realpath $(firstword $(MAKEFILE_LIST))))

ifndef NO_USE_JAVA
include $(module_directory)../build.neo_configure
include $(SRCDIR)/solenv/gbuild/partial_build.mk
else
include $(module_directory)/../solenv/gbuild/partial_build.mk
endif

# vim: set noet sw=4 ts=4:
