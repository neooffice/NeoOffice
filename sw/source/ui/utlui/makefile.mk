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
# Modified February 2013 by Patrick Luby. NeoOffice is distributed under
# GPL only under modification term 2 of the LGPL.
#
#*************************************************************************

PRJ=..$/..$/..

PRJNAME=sw
TARGET=utlui

# --- Settings -----------------------------------------------------

.INCLUDE :  $(PRJ)$/inc$/swpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/inc$/sw.mk

# --- Files --------------------------------------------------------


SRS1NAME=$(TARGET)
SRC1FILES =  \
		initui.src \
		gloslst.src \
		navipi.src \
		poolfmt.src \
		attrdesc.src \
		unotools.src \
                utlui.src

EXCEPTIONSFILES= \
		$(SLO)$/unotools.obj	\
		$(SLO)$/swrenamexnameddlg.obj

SLOFILES =  $(EXCEPTIONSFILES) \
		$(SLO)$/bookctrl.obj \
		$(SLO)$/condedit.obj \
		$(SLO)$/content.obj \
		$(SLO)$/gloslst.obj \
		$(SLO)$/glbltree.obj \
		$(SLO)$/initui.obj \
		$(SLO)$/navipi.obj \
		$(SLO)$/navicfg.obj \
		$(SLO)$/numfmtlb.obj \
		$(SLO)$/prcntfld.obj \
		$(SLO)$/textcontrolcombo.obj \
		$(SLO)$/tmplctrl.obj \
		$(SLO)$/uitool.obj \
		$(SLO)$/uiitems.obj \
		$(SLO)$/attrdesc.obj \
		$(SLO)$/shdwcrsr.obj \
                $(SLO)$/zoomctrl.obj \
                $(SLO)$/viewlayoutctrl.obj

.IF "$(NO_STATUSBAR_WORDCOUNT)" == ""
SLOFILES += $(SLO)$/wordcountctrl.obj
.ENDIF		# "$(NO_STATUSBAR_WORDCOUNT)" == ""

# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk

