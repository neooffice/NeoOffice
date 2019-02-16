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
#   Modified October 2016 by Patrick Luby. NeoOffice is only distributed
#   under the GNU General Public License, Version 3 as allowed by Section 3.3
#   of the Mozilla Public License, v. 2.0.
#
#   You should have received a copy of the GNU General Public License
#

$(eval $(call gb_Library_Library,macabdrv1))

$(eval $(call gb_Library_use_external,macabdrv1,boost_headers))

$(eval $(call gb_Library_use_sdk_api,macabdrv1))

$(eval $(call gb_Library_use_system_darwin_frameworks,macabdrv1,\
	$(if $(filter $(PRODUCT_BUILD_TYPE),java),CoreFoundation,Carbon) \
	AddressBook \
))

$(eval $(call gb_Library_use_libraries,macabdrv1,\
	comphelper \
	cppu \
	cppuhelper \
	dbtools \
	sal \
	salhelper \
))

$(eval $(call gb_Library_set_include,macabdrv1,\
        $$(INCLUDE) \
        -I$(SRCDIR)/connectivity/inc \
        -I$(SRCDIR)/connectivity/source/inc \
	-I$(WORKDIR)/YaccTarget/connectivity/source/parse \
))

$(eval $(call gb_Library_add_exception_objects,macabdrv1,\
	connectivity/source/drivers/macab/MacabColumns \
	connectivity/source/drivers/macab/MacabTable \
	connectivity/source/drivers/macab/MacabTables \
	connectivity/source/drivers/macab/MacabCatalog \
	connectivity/source/drivers/macab/MacabResultSet \
	connectivity/source/drivers/macab/MacabStatement \
	connectivity/source/drivers/macab/MacabPreparedStatement \
	connectivity/source/drivers/macab/MacabDatabaseMetaData \
	connectivity/source/drivers/macab/MacabConnection \
	connectivity/source/drivers/macab/MacabResultSetMetaData \
	connectivity/source/drivers/macab/macabcondition \
	connectivity/source/drivers/macab/macaborder \
	connectivity/source/drivers/macab/MacabRecord \
	connectivity/source/drivers/macab/MacabRecords \
	connectivity/source/drivers/macab/MacabHeader \
	connectivity/source/drivers/macab/MacabGroup \
	connectivity/source/drivers/macab/MacabAddressBook \
))

# vim: set noet sw=4 ts=4:
