##########################################################################
# 
#   $RCSfile$
# 
#   $Revision$
# 
#   last change: $Author$ $Date$
# 
#   The Contents of this file are made available subject to the terms of
#   either of the following licenses
# 
#          - GNU General Public License Version 2.1
# 
#   Patrick Luby, June 2003
# 
#   GNU General Public License Version 2.1
#   =============================================
#   Copyright 2003 by Patrick Luby (patrick.luby@planamesa.com)
# 
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU General Public
#   License version 2.1, as published by the Free Software Foundation.
# 
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   General Public License for more details.
# 
#   You should have received a copy of the GNU General Public
#   License along with this library; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#   MA  02111-1307  USA
# 
##########################################################################

PRJ=..$/..$/..

PRJNAME=sysui
TARGET=scripts

# --- Settings -----------------------------------------------------------

.INCLUDE :  settings.mk

# --- Targets -------------------------------------------------------------

.IF "$(OS)"!="MACOSX"

dummy:
	@echo "Nothing to do on this platform"

.ELSE			# "$(OS)"!="MACOSX"

PRODUCTFILES = \
	$(MISC)$/PkgInfo \
	$(MISC)$/Info.plist

COPYFILES = \
	$(MISC)$/ship.icns

.INCLUDE :  target.mk

ALLTAR : $(VERSIONFILES)

$(PRODUCTFILES) :
	cat /dev/null $(@:f) | $(SED) s#\$$\(PRODUCT_VERSION\)#$(PRODUCT_VERSION)#g | $(SED) s#\$$\(PRODUCT_NAME\)#$(PRODUCT_NAME)#g | $(SED) s#\$$\(RSCVERSION\)#$(RSCVERSION)#g | $(SED) s#\$$\(PRODUCT_FILETYPE\)#$(PRODUCT_FILETYPE)#g > $@

$(COPYFILES) :
	$(COPY) $(@:f) $@

.ENDIF			# "$(OS)"!="MACOSX"

