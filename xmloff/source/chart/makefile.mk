#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.
#  
#  $RCSfile$
#  $Revision$
#  
#  This file is part of NeoOffice.
#  
#  NeoOffice is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License version 3
#  only, as published by the Free Software Foundation.
#  
#  NeoOffice is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License version 3 for more details
#  (a copy is included in the LICENSE file that accompanied this code).
#  
#  You should have received a copy of the GNU General Public License
#  version 3 along with NeoOffice.  If not, see
#  <http://www.gnu.org/licenses/gpl-3.0.txt>
#  for a copy of the GPLv3 License.
#  
#  Modified October 2014 by Patrick Luby. NeoOffice is distributed under
#  GPL only under Section 4 of the Apache License v2.0.
#  
#**************************************************************

PRJ					= ..$/..
PRJNAME				= xmloff
TARGET				= chart
AUTOSEG				= true
ENABLE_EXCEPTIONS	= TRUE

# --- Settings -----------------------------------------------------

.INCLUDE : settings.mk
.INCLUDE: $(PRJ)$/util$/makefile.pmk

.IF "$(UPD)" == "310"
INCLOCAL+=-I$(PRJ)$/..$/sal/inc
.ENDIF		# "$(UPD)" == "310"

# --- Files --------------------------------------------------------

SLOFILES =	$(SLO)$/ColorPropertySet.obj \
            $(SLO)$/SchXMLTools.obj \
            $(SLO)$/SchXMLExport.obj \
			$(SLO)$/SchXMLImport.obj \
			$(SLO)$/contexts.obj \
			$(SLO)$/SchXMLTableContext.obj \
			$(SLO)$/SchXMLChartContext.obj \
			$(SLO)$/SchXMLPlotAreaContext.obj \
			$(SLO)$/SchXMLParagraphContext.obj \
			$(SLO)$/SchXMLSeriesHelper.obj \
			$(SLO)$/SchXMLSeries2Context.obj \
			$(SLO)$/PropertyMaps.obj \
			$(SLO)$/XMLChartStyleContext.obj \
			$(SLO)$/XMLErrorIndicatorPropertyHdl.obj \
			$(SLO)$/XMLErrorBarStylePropertyHdl.obj \
			$(SLO)$/SchXMLAutoStylePoolP.obj \
			$(SLO)$/XMLChartPropertyContext.obj \
			$(SLO)$/XMLSymbolImageContext.obj \
			$(SLO)$/XMLLabelSeparatorContext.obj \
			$(SLO)$/XMLTextOrientationHdl.obj \
			$(SLO)$/XMLSymbolTypePropertyHdl.obj \
			$(SLO)$/XMLAxisPositionPropertyHdl.obj \
			$(SLO)$/transporttypes.obj

# --- Targets --------------------------------------------------------------

.INCLUDE : target.mk

