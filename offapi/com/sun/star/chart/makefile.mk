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

PRJ=..$/..$/..$/..

PRJNAME=offapi
TARGET=csschart
PACKAGE=com$/sun$/star$/chart

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# Add locally built types registry to cppumaker search path
UNOUCRRDB+=$(OUT)$/ucr$/$(TARGET).db

# --- Files --------------------------------------------------------

IDLFILES= \
	ChartLegendExpansion.idl \
	TimeIncrement.idl \
	TimeInterval.idl \
	TimeUnit.idl \
	XAxis.idl \
	XComplexDescriptionAccess.idl \
	XDateCategories.idl \
	XDiagramPositioning.idl

UNOTYPES= \
	com.sun.star.chart.ChartLegendExpansion \
	com.sun.star.chart.TimeIncrement \
	com.sun.star.chart.TimeInterval \
	com.sun.star.chart.TimeUnit \
	com.sun.star.chart.XAxis \
	com.sun.star.chart.XComplexDescriptionAccess \
	com.sun.star.chart.XDateCategories \
	com.sun.star.chart.XDiagramPositioning

# Force creation of the IDL header files before the compiling source files
UNOUCRDEP=$(OUT)$/ucr$/$(TARGET).db

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
