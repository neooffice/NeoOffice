# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file incorporates work covered by the following license notice:
# 
#   Modified April 2017 by Patrick Luby. NeoOffice is only distributed
#   under the GNU General Public License, Version 3 as allowed by Section 3.3
#   of the Mozilla Public License, v. 2.0.
#
#   You should have received a copy of the GNU General Public License
#   along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

module_directory:=$(dir $(realpath $(firstword $(MAKEFILE_LIST))))

ifneq ($(strip $(PRODUCT_BUILD_TYPE)),java)
include $(module_directory)../build.neo_configure
endif	# PRODUCT_BUILD_TYPE != java

include $(module_directory)/../solenv/gbuild/partial_build.mk

# vim: set noet sw=4 ts=4:
