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
#  Modified November 2014 by Patrick Luby. NeoOffice is distributed under
#  GPL only under Section 4 of the Apache License v2.0.
#  
#**************************************************************


PRJ=..$/..
PRJNAME=writerfilter
TARGET=dmapper
GEN_HID=TRUE

ENABLE_EXCEPTIONS=TRUE

# --- Settings ----------------------------------

.INCLUDE : settings.mk
.INCLUDE :  $(PRJ)$/inc$/writerfilter.mk

# --- Files -------------------------------------

SLOFILES= \
    $(SLO)$/BorderHandler.obj \
    $(SLO)$/CellColorHandler.obj \
    $(SLO)$/CellMarginHandler.obj \
    $(SLO)$/ConversionHelper.obj \
    $(SLO)$/DomainMapper.obj \
    $(SLO)$/DomainMapperTableHandler.obj \
    $(SLO)$/DomainMapperTableManager.obj \
    $(SLO)$/DomainMapper_Impl.obj \
    $(SLO)$/FFDataHandler.obj \
    $(SLO)$/FontTable.obj \
    $(SLO)$/FormControlHelper.obj \
    $(SLO)$/GraphicHelpers.obj \
    $(SLO)$/GraphicImport.obj \
    $(SLO)$/MeasureHandler.obj \
    $(SLO)$/ModelEventListener.obj \
    $(SLO)$/NumberingManager.obj  \
    $(SLO)$/OLEHandler.obj \
    $(SLO)$/PageBordersHandler.obj \
    $(SLO)$/PropertyIds.obj \
    $(SLO)$/PropertyMap.obj \
    $(SLO)$/PropertyMapHelper.obj \
    $(SLO)$/SectionColumnHandler.obj \
    $(SLO)$/SettingsTable.obj \
    $(SLO)$/StyleSheetTable.obj \
    $(SLO)$/TDefTableHandler.obj \
    $(SLO)$/TablePropertiesHandler.obj \
    $(SLO)$/TblStylePrHandler.obj \
    $(SLO)$/ThemeTable.obj \
    $(SLO)$/WrapPolygonHandler.obj \

.IF "$(UPD)" == "310"
SLOFILES += \
    $(SLO)$/LatentStyleHandler.obj \
    $(SLO)$/TrackChangesHandler.obj \
    $(SLO)$/SdtHelper.obj \
    $(SLO)$/TablePositionHandler.obj \
    $(SLO)$/TextEffectsHandler.obj
.ENDIF		# "$(UPD)" == "310"


# --- Targets ----------------------------------

.INCLUDE : target.mk



