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

PRJNAME=sw
TARGET=vbaswobj
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.IF "$(UPD)" == "310"
.INCLUDE :	$(PRJ)$/inc$/swpre.mk
.ENDIF		# "$(UPD)" == "310"
.INCLUDE :  settings.mk
.IF "$(UPD)" == "310"
.INCLUDE :	$(PRJ)$/inc$/sw.mk
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
		$(SLO)$/vbaapplication.obj \
		$(SLO)$/vbadocument.obj \
		$(SLO)$/vbawindow.obj \
		$(SLO)$/vbasystem.obj \
		$(SLO)$/vbarangehelper.obj \
		$(SLO)$/vbarange.obj \
		$(SLO)$/vbabookmark.obj \
		$(SLO)$/vbabookmarks.obj \
		$(SLO)$/vbavariable.obj \
		$(SLO)$/vbavariables.obj \
		$(SLO)$/vbaview.obj \
		$(SLO)$/wordvbahelper.obj \
				$(SLO)$/service.obj \
				$(SLO)$/vbadocumentproperties.obj \
				$(SLO)$/vbapane.obj \
				$(SLO)$/vbapanes.obj \
				$(SLO)$/vbaoptions.obj \
				$(SLO)$/vbaselection.obj \
 				$(SLO)$/vbatemplate.obj \
				$(SLO)$/vbaparagraphformat.obj \
				$(SLO)$/vbaautotextentry.obj \
				$(SLO)$/vbaparagraph.obj \
				$(SLO)$/vbafind.obj \
				$(SLO)$/vbareplacement.obj \
				$(SLO)$/vbastyle.obj \
				$(SLO)$/vbastyles.obj \
				$(SLO)$/vbafont.obj \
				$(SLO)$/vbapalette.obj \
				$(SLO)$/vbainformationhelper.obj \
				$(SLO)$/vbatable.obj \
				$(SLO)$/vbatables.obj \
				$(SLO)$/vbafield.obj \
				$(SLO)$/vbaborders.obj \
				$(SLO)$/vbadocuments.obj \
				$(SLO)$/vbaheaderfooter.obj \
				$(SLO)$/vbaheaderfooterhelper.obj \
				$(SLO)$/vbaaddin.obj \
				$(SLO)$/vbaaddins.obj \
				$(SLO)$/vbadialogs.obj \
				$(SLO)$/vbadialog.obj \
				$(SLO)$/vbawrapformat.obj \
				$(SLO)$/vbapagesetup.obj \
				$(SLO)$/vbasection.obj \
				$(SLO)$/vbasections.obj \

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

ALLTAR : \
        $(MISC)$/$(TARGET).don \

$(SLOFILES) : $(MISC)$/$(TARGET).don

$(MISC)$/$(TARGET).don : $(SOLARBINDIR)$/oovbaapi.rdb
        +$(CPPUMAKER) -O$(INCCOM)$/$(TARGET) -BUCR $(SOLARBINDIR)$/oovbaapi.rdb -X$(SOLARBINDIR)$/types.rdb && echo > $@
        echo $@

