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
# Modified December 2005 by Patrick Luby. NeoOffice is distributed under
# GPL only under modification term 2 of the LGPL.
#
#*************************************************************************

PRJ=..$/..

PRJNAME=sfx2
TARGET=doc
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

SRS1NAME=$(TARGET)
SRC1FILES = \
        doc.src new.src doctdlg.src docvor.src doctempl.src sfxbasemodel.src graphhelp.src

SLOFILES =	\
        $(SLO)$/printhelper.obj \
        $(SLO)$/docinf.obj \
        $(SLO)$/oleprops.obj \
        $(SLO)$/iframe.obj \
        $(SLO)$/applet.obj \
        $(SLO)$/plugin.obj \
		$(SLO)$/docfile.obj \
		$(SLO)$/objuno.obj \
		$(SLO)$/frmdescr.obj \
		$(SLO)$/objxtor.obj \
		$(SLO)$/objmisc.obj \
		$(SLO)$/objstor.obj \
		$(SLO)$/objcont.obj \
		$(SLO)$/objserv.obj \
		$(SLO)$/objitem.obj \
		$(SLO)$/ownsubfilterservice.obj \
		$(SLO)$/docfac.obj \
		$(SLO)$/docfilt.obj \
		$(SLO)$/doctempl.obj \
		$(SLO)$/doctemplates.obj \
		$(SLO)$/doctemplateslocal.obj \
		$(SLO)$/docvor.obj \
		$(SLO)$/new.obj \
		$(SLO)$/doctdlg.obj \
		$(SLO)$/sfxbasemodel.obj \
		$(SLO)$/guisaveas.obj\
		$(SLO)$/objembed.obj\
		$(SLO)$/graphhelp.obj \
		$(SLO)$/QuerySaveDocument.obj \
        $(SLO)$/opostponedtruncationstream.obj \
        $(SLO)$/docinsert.obj \
        $(SLO)$/docmacromode.obj \
        $(SLO)$/SfxDocumentMetaData.obj \
        $(SLO)$/sfxmodelfactory.obj \
        $(SLO)$/docstoragemodifylistener.obj

.IF "$(GUIBASE)" == "java"
SLOFILES += $(SLO)$/objmisc_cocoa.obj
.ENDIF

# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk


