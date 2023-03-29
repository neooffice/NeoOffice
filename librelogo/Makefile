# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-

module_directory:=$(dir $(realpath $(firstword $(MAKEFILE_LIST))))

ifneq ($(strip $(PRODUCT_BUILD_TYPE)),java)
include $(module_directory)../build.neo_configure
endif	# PRODUCT_BUILD_TYPE != java

include $(module_directory)/../solenv/gbuild/partial_build.mk

# vim: set noet sw=4 ts=4:
