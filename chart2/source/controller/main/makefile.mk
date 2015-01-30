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
# Modified January 2015 by Patrick Luby. NeoOffice is distributed under
# GPL only under modification term 2 of the LGPL.
#
#*************************************************************************

PRJ=				..$/..$/..
PRJINC=				$(PRJ)$/source
PRJNAME=			chart2
TARGET=				chcontroller

ENABLE_EXCEPTIONS=	TRUE
VISIBILITY_HIDDEN=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE: settings.mk

.IF "$(UPD)" == "310"
.INCLUDE: $(PRJ)$/inc$/chart2.mk
.ENDIF		# "$(UPD)" == "310"

# --- export library -------------------------------------------------

#object files to build and link together to lib $(SLB)$/$(TARGET).lib
SLOFILES =  \
            $(SLO)$/ConfigurationAccess.obj	\
            $(SLO)$/SelectionHelper.obj	\
            $(SLO)$/PositionAndSizeHelper.obj	\
			$(SLO)$/ChartWindow.obj	\
			$(SLO)$/ChartController.obj	\
			$(SLO)$/ChartController_EditData.obj	\
			$(SLO)$/ChartController_Window.obj	\
			$(SLO)$/ChartController_Properties.obj	\
			$(SLO)$/ChartController_Insert.obj	\
			$(SLO)$/ChartController_TextEdit.obj \
			$(SLO)$/ChartController_Position.obj \
			$(SLO)$/ChartController_Tools.obj \
			$(SLO)$/ChartFrameloader.obj \
			$(SLO)$/ChartRenderer.obj \
			$(SLO)$/CommandDispatchContainer.obj \
			$(SLO)$/CommandDispatch.obj \
			$(SLO)$/ControllerCommandDispatch.obj \
			$(SLO)$/UndoCommandDispatch.obj \
			$(SLO)$/DragMethod_Base.obj \
			$(SLO)$/DragMethod_RotateDiagram.obj \
			$(SLO)$/DragMethod_PieSegment.obj \
			$(SLO)$/ObjectHierarchy.obj \
			$(SLO)$/_serviceregistration_controller.obj \
			$(SLO)$/ChartDropTargetHelper.obj \
			$(SLO)$/StatusBarCommandDispatch.obj \
			$(SLO)$/ChartTransferable.obj

#			$(SLO)$/CommonConverters.obj \
#			$(SLO)$/Scaling.obj	\

# --- Targets -----------------------------------------------------------------

.INCLUDE: target.mk
