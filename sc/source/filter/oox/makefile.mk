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
#   Patrick Luby, November 2014
# 
#   GNU General Public License Version 2.1
#   =============================================
#   Copyright 2014 Planamesa Inc.
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

PRJNAME=sc
TARGET=oox

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE : $(PRJ)/inc/sc.mk

# --- Files --------------------------------------------------------

.IF "$(UPD)" == "310"
SLOFILES = \
	$(SLO)$/excelvbaproject.obj
.ELSE		# "$(UPD)" == "310"
SLOFILES = \
	$(SLO)$/excelfilter.obj \
	$(SLO)$/addressconverter.obj \
	$(SLO)$/autofilterbuffer.obj \
	$(SLO)$/autofiltercontext.obj \
	$(SLO)$/biffcodec.obj \
	$(SLO)$/biffhelper.obj \
	$(SLO)$/biffinputstream.obj \
	$(SLO)$/chartsheetfragment.obj \
	$(SLO)$/commentsbuffer.obj \
	$(SLO)$/commentsfragment.obj \
	$(SLO)$/condformatbuffer.obj \
	$(SLO)$/condformatcontext.obj \
	$(SLO)$/connectionsbuffer.obj \
	$(SLO)$/connectionsfragment.obj \
	$(SLO)$/defnamesbuffer.obj \
	$(SLO)$/drawingbase.obj \
	$(SLO)$/drawingfragment.obj \
	$(SLO)$/drawingmanager.obj \
	$(SLO)$/excelchartconverter.obj \
	$(SLO)$/excelhandlers.obj \
	$(SLO)$/excelvbaproject.obj \
	$(SLO)$/externallinkbuffer.obj \
	$(SLO)$/externallinkfragment.obj \
	$(SLO)$/extlstcontext.obj \
	$(SLO)$/formulabase.obj \
	$(SLO)$/formulabuffer.obj \
	$(SLO)$/formulaparser.obj \
	$(SLO)$/numberformatsbuffer.obj \
	$(SLO)$/ooxformulaparser.obj \
	$(SLO)$/pagesettings.obj \
	$(SLO)$/pivotcachebuffer.obj \
	$(SLO)$/pivotcachefragment.obj \
	$(SLO)$/pivottablebuffer.obj \
	$(SLO)$/pivottablefragment.obj \
	$(SLO)$/querytablebuffer.obj \
	$(SLO)$/querytablefragment.obj \
	$(SLO)$/revisionfragment.obj \
	$(SLO)$/richstringcontext.obj \
	$(SLO)$/richstring.obj \
	$(SLO)$/scenariobuffer.obj \
	$(SLO)$/scenariocontext.obj \
	$(SLO)$/sharedstringsbuffer.obj \
	$(SLO)$/sharedstringsfragment.obj \
	$(SLO)$/sheetdatabuffer.obj \
	$(SLO)$/sheetdatacontext.obj \
	$(SLO)$/stylesbuffer.obj \
	$(SLO)$/stylesfragment.obj \
	$(SLO)$/tablebuffer.obj \
	$(SLO)$/tablefragment.obj \
	$(SLO)$/themebuffer.obj \
	$(SLO)$/threadpool.obj \
	$(SLO)$/unitconverter.obj \
	$(SLO)$/viewsettings.obj \
	$(SLO)$/workbookfragment.obj \
	$(SLO)$/workbookhelper.obj \
	$(SLO)$/workbooksettings.obj \
	$(SLO)$/worksheetbuffer.obj \
	$(SLO)$/worksheetfragment.obj \
	$(SLO)$/worksheethelper.obj \
	$(SLO)$/worksheetsettings.obj
.ENDIF		# "$(UPD)" == "310"

EXCEPTIONSFILES = $(SLOFILES)

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk

