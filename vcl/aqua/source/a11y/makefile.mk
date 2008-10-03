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
# Modified October 2008 by Patrick Luby. NeoOffice is distributed under
# GPL only under modification term 2 of the LGPL.
#
#*************************************************************************

PRJ=..$/..$/..

PRJNAME=vcl
TARGET=sala11y
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :	$(PRJ)$/util$/makefile2.pmk

# --- Files --------------------------------------------------------

.IF "$(GUIBASE)"!="aqua" && "$(GUIBASE)"!="java"

dummy:
    @echo "Nothing to build for GUIBASE $(GUIBASE)"

.ELSE		# "$(GUIBASE)"!="aqua" && "$(GUIBASE)"!="java"

SLOFILES=	\
		$(SLO)$/aqua11ywrapper.obj \
		$(SLO)$/aqua11yfactory.obj \
		$(SLO)$/aqua11yfocuslistener.obj \
		$(SLO)$/aqua11yfocustracker.obj \
		$(SLO)$/aqua11ylistener.obj \
		$(SLO)$/aqua11yrolehelper.obj \
		$(SLO)$/aqua11yactionwrapper.obj \
		$(SLO)$/aqua11ycomponentwrapper.obj \
		$(SLO)$/aqua11yselectionwrapper.obj \
		$(SLO)$/aqua11ytablewrapper.obj \
		$(SLO)$/aqua11ytextattributeswrapper.obj \
		$(SLO)$/aqua11ytextwrapper.obj \
		$(SLO)$/aqua11yutil.obj \
		$(SLO)$/aqua11yvaluewrapper.obj \
		$(SLO)$/aqua11ywrapperbutton.obj \
		$(SLO)$/aqua11ywrappercheckbox.obj \
		$(SLO)$/aqua11ywrappercombobox.obj \
		$(SLO)$/aqua11ywrappergroup.obj \
		$(SLO)$/aqua11ywrapperlist.obj \
		$(SLO)$/aqua11ywrapperradiobutton.obj \
		$(SLO)$/aqua11ywrapperradiogroup.obj \
		$(SLO)$/aqua11ywrapperrow.obj \
		$(SLO)$/aqua11ywrapperscrollarea.obj \
		$(SLO)$/aqua11ywrapperscrollbar.obj \
		$(SLO)$/aqua11ywrappersplitter.obj \
		$(SLO)$/aqua11ywrapperstatictext.obj \
		$(SLO)$/aqua11ywrappertabgroup.obj \
		$(SLO)$/aqua11ywrappertextarea.obj \
		$(SLO)$/aqua11ywrappertoolbar.obj \
		$(SLO)$/documentfocuslistener.obj

.ENDIF		# "$(GUIBASE)"!="aqua" && "$(GUIBASE)"!="java"

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

.INCLUDE :  $(PRJ)$/util$/target.pmk
