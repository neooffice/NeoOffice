.PHONY: build.neo_configure_phony

build.neo_configure_phony:
	rm -f "$(OO_ENV_JAVA)"
	sed 's#^setenv GUIBASE "aqua"$$#setenv GUIBASE "java"#' "$(OO_ENV_AQUA)" | sed 's#^setenv ENVCDEFS "#&-DUSE_JAVA#' | sed 's#^setenv DELIVER .*$$#setenv DELIVER "true"#' | sed 's#^alias deliver .*$$#alias deliver "echo The deliver command has been disabled"#' > "$(OO_ENV_JAVA)"
	sh -e -c 'if ! grep -q "^setenv ENVCDEFS " "$(OO_ENV_JAVA)" ; then echo "setenv ENVCDEFS -DUSE_JAVA" >> "$(OO_ENV_JAVA)" ; fi'
	echo "setenv PRODUCT_NAME '$(PRODUCT_NAME)'" >> "$(OO_ENV_JAVA)"
	echo "setenv PRODUCT_DIR_NAME '$(PRODUCT_DIR_NAME)'" >> "$(OO_ENV_JAVA)"
	echo "setenv PRODUCT_DIR_NAME2 '$(PRODUCT_DIR_NAME2)'" >> "$(OO_ENV_JAVA)"
	echo "setenv PRODUCT_DIR_NAME3 '$(PRODUCT_DIR_NAME3)'" >> "$(OO_ENV_JAVA)"
	echo "setenv PRODUCT_DOMAIN '$(PRODUCT_DOMAIN)'" >> "$(OO_ENV_JAVA)"
	echo "setenv PRODUCT_FILETYPE '$(PRODUCT_FILETYPE)'" >> "$(OO_ENV_JAVA)"
	echo "setenv PRODUCT_MAC_APP_STORE_URL '$(PRODUCT_MAC_APP_STORE_URL)'" >> "$(OO_ENV_JAVA)"
	echo "setenv PRODUCT_JAVA_DOWNLOAD_URL '$(PRODUCT_JAVA_DOWNLOAD_URL)'" >> "$(OO_ENV_JAVA)"
	echo "setenv PRODUCT_MIN_OSVERSION '$(PRODUCT_MIN_OSVERSION)'" >> "$(OO_ENV_JAVA)"
	echo "setenv PRODUCT_MAX_OSVERSION '$(PRODUCT_MAX_OSVERSION)'" >> "$(OO_ENV_JAVA)"
