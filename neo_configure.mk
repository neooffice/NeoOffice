.PHONY: build.neo_configure_phony

PRODUCT_DONATION_URL=$(PRODUCT_BASE_URL)/donate.php
PRODUCT_WELCOME_URL=$(PRODUCT_BASE_URL)/welcome.php
X11_PRODUCT_DONATION_URL=$(X11_PRODUCT_BASE_URL)/donate.php
X11_PRODUCT_WELCOME_URL=$(X11_PRODUCT_BASE_URL)/welcome.php
PRODUCT_FILETYPE=NO%F
BUILD_MACHINE=$(shell echo `id -nu`:`hostname`.`domainname`)

build.neo_configure_phony:
	rm -f "$(OO_ENV_JAVA)"
	sed 's#^setenv GUIBASE .*$$#setenv GUIBASE "java"#' "$(OO_ENV_X11)" | sed 's#^setenv ENVCDEFS "#&-DUSE_JAVA#' | sed 's#^setenv CLASSPATH .*$$#setenv CLASSPATH "$$SOLARVER/$$UPD/$$INPATH/bin/vcl.jar"#' | sed 's#^setenv DELIVER .*$$#setenv DELIVER "true"#' | sed 's#^alias deliver .*$$#alias deliver "echo The deliver command has been disabled"#' > "$(OO_ENV_JAVA)"
	echo "setenv PRODUCT_NAME '$(PRODUCT_NAME)'" >> "$(OO_ENV_JAVA)"
	echo "setenv PRODUCT_DIR_NAME '$(PRODUCT_DIR_NAME)'" >> "$(OO_ENV_JAVA)"
	echo "setenv PRODUCT_DONATION_URL '$(PRODUCT_DONATION_URL)'" >> "$(OO_ENV_JAVA)"
	echo "setenv PRODUCT_SUPPORT_URL '$(PRODUCT_SUPPORT_URL)'" >> "$(OO_ENV_JAVA)"
	echo "setenv PRODUCT_WELCOME_URL '$(PRODUCT_WELCOME_URL)'" >> "$(OO_ENV_JAVA)"
	echo "setenv PRODUCT_FILETYPE '$(PRODUCT_FILETYPE)'" >> "$(OO_ENV_JAVA)"
	echo "setenv BUILD_MACHINE '$(BUILD_MACHINE)'" >> "$(OO_ENV_JAVA)"
	echo "setenv X11_PRODUCT_NAME '$(X11_PRODUCT_NAME)'" >> "$(OO_ENV_JAVA)"
	echo "setenv X11_PRODUCT_DIR_NAME '$(X11_PRODUCT_DIR_NAME)'" >> "$(OO_ENV_JAVA)"
	echo "setenv X11_PRODUCT_DONATION_URL '$(X11_PRODUCT_DONATION_URL)'" >> "$(OO_ENV_JAVA)"
	echo "setenv X11_PRODUCT_WELCOME_URL '$(X11_PRODUCT_WELCOME_URL)'" >> "$(OO_ENV_JAVA)"
