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

PRJ=..$/..$/..$/..

PRJNAME=sysui
TARGET=macosx_icons

# --- Settings -----------------------------------------------------------

.INCLUDE :  settings.mk

# --- Targets -------------------------------------------------------------

.IF "$(OS)"!="MACOSX"

dummy:
	@echo "Nothing to do on this platform"

.ELSE			# "$(OS)"!="MACOSX"

COPYFILES = \
	$(MISC)/License \
	$(MISC)/Readme \
	$(MISC)/dbf_dif.icns \
	$(MISC)/generic-inside.icns \
	$(MISC)/generic.icns \
	$(MISC)/html.icns \
	$(MISC)/rtf.icns \
	$(MISC)/sds.icns \
	$(MISC)/stationery-generic-inside.icns \
	$(MISC)/stationery-generic.icns \
	$(MISC)/stationery-plain_text.icns \
	$(MISC)/stationery-rich_text.icns \
	$(MISC)/stc_xlt.icns \
	$(MISC)/std.icns \
	$(MISC)/sti_pot.icns \
	$(MISC)/stw_dot_Option_1.icns \
	$(MISC)/stw_dot_Option_2.icns \
	$(MISC)/sxc_sdc_slk_xls.icns \
	$(MISC)/sxd_sda.icns \
	$(MISC)/sxi_sdd_ppt.icns \
	$(MISC)/sxm_smf_mml.icns \
	$(MISC)/sxw_sdw_doc_Option_1.icns \
	$(MISC)/sxw_sdw_doc_Option_2.icns \
	$(MISC)/txt.icns

.INCLUDE :  target.mk

ALLTAR : $(COPYFILES)

$(COPYFILES) :
	$(COPY) $(@:f) $@

.ENDIF			# "$(OS)"!="MACOSX"

