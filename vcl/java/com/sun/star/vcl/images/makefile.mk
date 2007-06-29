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
#   Copyright 2003 Planamesa Inc.
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

PRJ=..$/..$/..$/..$/..$/..

PACKAGE = com$/sun$/star$/vcl$/images
PRJNAME=vcl
TARGET=com_sun_star_vcl_images

# --- Settings -----------------------------------------------------

.INCLUDE :  svpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sv.mk

# --- Files --------------------------------------------------------

.IF "$(GUIBASE)$(OS)"!="javaMACOSX"

dummy:
    @echo "Nothing to build for GUIBASE $(GUIBASE)"

.ELSE		# "$(GUIBASE)$(OS)"!="javaMACOSX"

COPYDIR = $(CLASSDIR)$/$(PACKAGE)

COPYFILES = \
	$(COPYDIR)$/airbrush.gif \
	$(COPYDIR)$/ase.gif \
	$(COPYDIR)$/asn.gif \
	$(COPYDIR)$/asne.gif \
	$(COPYDIR)$/asns.gif \
	$(COPYDIR)$/asnswe.gif \
	$(COPYDIR)$/asnw.gif \
	$(COPYDIR)$/ass.gif \
	$(COPYDIR)$/asse.gif \
	$(COPYDIR)$/assw.gif \
	$(COPYDIR)$/asw.gif \
	$(COPYDIR)$/aswe.gif \
	$(COPYDIR)$/chain.gif \
	$(COPYDIR)$/chainnot.gif \
	$(COPYDIR)$/chart.gif \
	$(COPYDIR)$/copydata.gif \
	$(COPYDIR)$/copydlnk.gif \
	$(COPYDIR)$/copyf.gif \
	$(COPYDIR)$/copyf2.gif \
	$(COPYDIR)$/copyflnk.gif \
	$(COPYDIR)$/crook.gif \
	$(COPYDIR)$/crop.gif \
	$(COPYDIR)$/detectiv.gif \
	$(COPYDIR)$/fill.gif \
	$(COPYDIR)$/hand.gif \
	$(COPYDIR)$/help.gif \
	$(COPYDIR)$/hshear.gif \
	$(COPYDIR)$/hsize.gif \
	$(COPYDIR)$/hsizebar.gif \
	$(COPYDIR)$/hsplit.gif \
	$(COPYDIR)$/linkdata.gif \
	$(COPYDIR)$/linkf.gif \
	$(COPYDIR)$/magnify.gif \
	$(COPYDIR)$/mirror.gif \
	$(COPYDIR)$/move.gif \
	$(COPYDIR)$/movebw.gif \
	$(COPYDIR)$/movedata.gif \
	$(COPYDIR)$/movedlnk.gif \
	$(COPYDIR)$/movef.gif \
	$(COPYDIR)$/movef2.gif \
	$(COPYDIR)$/moveflnk.gif \
	$(COPYDIR)$/movept.gif \
	$(COPYDIR)$/neswsize.gif \
	$(COPYDIR)$/notallow.gif \
	$(COPYDIR)$/nullptr.gif \
	$(COPYDIR)$/nwsesize.gif \
	$(COPYDIR)$/pen.gif \
	$(COPYDIR)$/pivotcol.gif \
	$(COPYDIR)$/pivotdel.gif \
	$(COPYDIR)$/pivotfld.gif \
	$(COPYDIR)$/pivotrow.gif \
	$(COPYDIR)$/pntbrsh.gif \
	$(COPYDIR)$/refhand.gif \
	$(COPYDIR)$/rotate.gif \
	$(COPYDIR)$/tblsele.gif \
	$(COPYDIR)$/tblsels.gif \
	$(COPYDIR)$/tblselse.gif \
	$(COPYDIR)$/tblselsw.gif \
	$(COPYDIR)$/tblselw.gif \
	$(COPYDIR)$/timemove.gif \
	$(COPYDIR)$/timesize.gif \
	$(COPYDIR)$/vshear.gif \
	$(COPYDIR)$/vsize.gif \
	$(COPYDIR)$/vsizebar.gif \
	$(COPYDIR)$/vsplit.gif

ALLTAR : $(COPYDIR) $(COPYFILES)

$(COPYDIR) :
	$(MKDIRHIER) $(COPYDIR)

$(CLASSDIR)$/$(PACKAGE)$/% : %
	$(COPY) $(@:f) $@

.ENDIF		# "$(GUIBASE)$(OS)"!="javaMACOSX"

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

.INCLUDE :  $(PRJ)$/util$/target.pmk
