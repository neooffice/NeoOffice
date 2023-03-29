# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

module_directory:=$(dir $(realpath $(firstword $(MAKEFILE_LIST))))

ifneq ($(strip $(PRODUCT_BUILD_TYPE)),java)
include $(module_directory)../build.neo_configure

check_env_vars:
	sh -c -e 'if [ "$$PRODUCT_NAME" != "$(PARENT_PRODUCT_NAME)" -o "$$PRODUCT_DOMAIN" != "$(PARENT_PRODUCT_DOMAIN)" -o "$$PRODUCT_DIR_NAME" != "$(PARENT_PRODUCT_DIR_NAME)" -o "$$PRODUCT_DIR_NAME2" != "$(PARENT_PRODUCT_DIR_NAME2)" -o "$$PRODUCT_DIR_NAME3" != "$(PARENT_PRODUCT_DIR_NAME3)" -o "$$PRODUCT_MAC_APP_STORE_URL" != "$(PARENT_PRODUCT_MAC_APP_STORE_URL)" -o "$$PRODUCT_JAVA_DOWNLOAD_URL" != "$(PARENT_PRODUCT_JAVA_DOWNLOAD_URL)" -o "$$PRODUCT_MIN_OSVERSION" != "$(PARENT_PRODUCT_MIN_OSVERSION)" -o "$$PRODUCT_MAX_OSVERSION" != "$(PARENT_PRODUCT_MAX_OSVERSION)" ] ; then echo "You must rebuild the build.neo_configure target before you can build this target" ; exit 1 ; fi'
endif	# PRODUCT_BUILD_TYPE != java

# vim: set noet sw=4 ts=4:
