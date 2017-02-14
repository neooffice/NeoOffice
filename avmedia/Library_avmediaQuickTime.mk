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

$(eval $(call gb_Library_Library,avmediaQuickTime))

$(eval $(call gb_Library_set_componentfile,avmediaQuickTime,avmedia/source/quicktime/avmediaQuickTime))

$(eval $(call gb_Library_set_include,avmediaQuickTime,\
	$$(INCLUDE) \
	-I$(SRCDIR)/avmedia/source/inc \
))

$(eval $(call gb_Library_use_external,avmediaQuickTime,boost_headers))

$(eval $(call gb_Library_use_sdk_api,avmediaQuickTime));

$(eval $(call gb_Library_use_libraries,avmediaQuickTime,\
	comphelper \
	cppu \
	cppuhelper \
	sal \
	tl \
	vcl \
	$(gb_UWINAPI) \
))

ifeq ($(strip $(GUIBASE)),java)

$(eval $(call gb_Library_use_system_darwin_frameworks,avmediaQuickTime,\
	AppKit \
))

$(eval $(call gb_Library_add_objcxxobjects,avmediaQuickTime,\
	avmedia/source/quicktime/quicktimecommon \
	avmedia/source/quicktime/quicktimeframegrabber \
	avmedia/source/quicktime/quicktimemanager \
	avmedia/source/quicktime/quicktimeplayer \
	avmedia/source/quicktime/quicktimeuno \
	avmedia/source/quicktime/quicktimewindow \
))

else	# GUIBASE == java

$(eval $(call gb_Library_use_system_darwin_frameworks,avmediaQuickTime,\
	Cocoa \
	QTKit \
))

$(eval $(call gb_Library_add_objcxxobjects,avmediaQuickTime,\
	avmedia/source/quicktime/framegrabber \
	avmedia/source/quicktime/manager \
	avmedia/source/quicktime/player \
	avmedia/source/quicktime/quicktimeuno \
	avmedia/source/quicktime/window \
))

endif	# GUIBASE == java

# vim: set noet sw=4 ts=4:
