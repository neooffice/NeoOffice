#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile$
#
# $Revision$
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

PRJ					= ..$/..
PRJNAME				= xmloff
TARGET				= chart
AUTOSEG				= true
ENABLE_EXCEPTIONS	= TRUE

# --- Settings -----------------------------------------------------

.INCLUDE : settings.mk
.INCLUDE: $(PRJ)$/util$/makefile.pmk

.IF "$(UPD)" == "310"
INCLOCAL += -I$(PRJ)$/..$/sal/inc
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

