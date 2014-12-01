#*************************************************************************
#
# Copyright 2008 by Sun Microsystems, Inc.
#
# $RCSfile$
#
# $Revision$
#
# This file is part of NeoOffice.
#
# NeoOffice is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 3
# only, as published by the Free Software Foundation.
#
# NeoOffice is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU General Public License
# version 3 along with NeoOffice.  If not, see
# <http://www.gnu.org/licenses/gpl-3.0.txt>
# for a copy of the GPLv3 License.
#
# Modified November 2014 by Patrick Luby. NeoOffice is distributed under
# GPL only under modification term 2 of the LGPL.
#
#*************************************************************************

PRJ=..$/..$/..

PRJNAME=sc
TARGET=vbaobj
ENABLE_EXCEPTIONS=TRUE
VISIBILITY_HIDDEN=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

.IF "$(UPD)" == "310"
.INCLUDE : $(PRJ)/inc/sc.mk
.ENDIF		# "$(UPD)" == "310"

DLLPRE =

.IF "$(ENABLE_VBA)"!="YES"
dummy:
        @echo "not building vba..."
.ENDIF
 
INCPRE=$(INCCOM)$/$(TARGET)
CDEFS+=-DVBA_OOBUILD_HACK
# ------------------------------------------------------------------

SLOFILES= \
		$(SLO)$/vbaglobals.obj \
		$(SLO)$/vbaworkbook.obj \
		$(SLO)$/vbaworksheets.obj \
		$(SLO)$/vbaapplication.obj \
		$(SLO)$/vbarange.obj \
		$(SLO)$/vbaname.obj \
		$(SLO)$/vbanames.obj \
		$(SLO)$/vbacomment.obj \
		$(SLO)$/vbacomments.obj \
		$(SLO)$/vbaworkbooks.obj \
		$(SLO)$/vbaworksheet.obj \
		$(SLO)$/vbaoutline.obj \
		$(SLO)$/vbafont.obj\
		$(SLO)$/excelvbahelper.obj\
		$(SLO)$/vbainterior.obj\
		$(SLO)$/vbawsfunction.obj\
		$(SLO)$/vbawindow.obj\
		$(SLO)$/vbachart.obj\
		$(SLO)$/vbachartobject.obj\
		$(SLO)$/vbachartobjects.obj\
		$(SLO)$/vbaseriescollection.obj\
		$(SLO)$/vbadialogs.obj \
		$(SLO)$/vbadialog.obj	\
		$(SLO)$/vbapivottable.obj \
		$(SLO)$/vbapivotcache.obj \
		$(SLO)$/vbapivottables.obj \
		$(SLO)$/vbawindows.obj \
		$(SLO)$/vbapalette.obj \
		$(SLO)$/vbaborders.obj \
		$(SLO)$/vbacharacters.obj \
		$(SLO)$/vbavalidation.obj \
                $(SLO)$/vbaoleobject.obj \
                $(SLO)$/vbaoleobjects.obj \
                $(SLO)$/vbatextboxshape.obj \
                $(SLO)$/vbapane.obj \
                $(SLO)$/vbatextframe.obj \
                $(SLO)$/vbacharttitle.obj \
                $(SLO)$/vbacharts.obj \
                $(SLO)$/vbaaxistitle.obj \
                $(SLO)$/vbaaxes.obj \
                $(SLO)$/vbaaxis.obj \
                $(SLO)$/vbaformat.obj \
                $(SLO)$/vbacondition.obj \
                $(SLO)$/vbaformatcondition.obj \
                $(SLO)$/vbaformatconditions.obj \
                $(SLO)$/vbastyle.obj \
                $(SLO)$/vbastyles.obj \
                $(SLO)$/vbaassistant.obj \
		        $(SLO)$/vbahyperlink.obj \
        		$(SLO)$/vbapagesetup.obj \
		        $(SLO)$/vbapagebreak.obj \
        		$(SLO)$/vbapagebreaks.obj \
				$(SLO)$/service.obj \
        $(SLO)$/vbaeventshelper.obj \
        $(SLO)$/vbamenubar.obj  \
        $(SLO)$/vbamenubars.obj \
        $(SLO)$/vbamenu.obj \
        $(SLO)$/vbamenus.obj \
        $(SLO)$/vbamenuitem.obj \
        $(SLO)$/vbamenuitems.obj \

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

ALLTAR : \
        $(MISC)$/$(TARGET).don \

$(SLOFILES) : $(MISC)$/$(TARGET).don

$(MISC)$/$(TARGET).don : $(SOLARBINDIR)$/oovbaapi.rdb
        +$(CPPUMAKER) -O$(INCCOM)$/$(TARGET) -BUCR $(SOLARBINDIR)$/oovbaapi.rdb -X$(SOLARBINDIR)$/types.rdb && echo > $@
        echo $@

