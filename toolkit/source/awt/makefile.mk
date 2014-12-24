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
# Modified December 2014 by Patrick Luby. NeoOffice is distributed under
# GPL only under modification term 2 of the LGPL.
#
#*************************************************************************

PRJ=..$/..

PRJNAME=toolkit
TARGET=awt

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

.IF "$(GUIBASE)"=="aqua" || "$(GUIBASE)"=="java"
OBJCXXFLAGS=-x objective-c++ -fobjc-exceptions
CFLAGSCXX+=$(OBJCXXFLAGS)
.ENDIF	# "$(GUIBASE)"=="aqua" || "$(GUIBASE)"=="java"

SLOFILES=   \
			$(SLO)$/vclxaccessiblecomponent.obj         \
			$(SLO)$/vclxbitmap.obj 						\
			$(SLO)$/vclxcontainer.obj 					\
			$(SLO)$/vclxdevice.obj 						\
			$(SLO)$/vclxfont.obj 						\
			$(SLO)$/vclxgraphics.obj 					\
			$(SLO)$/vclxmenu.obj 						\
			$(SLO)$/vclxpointer.obj 					\
			$(SLO)$/vclxprinter.obj 					\
			$(SLO)$/vclxregion.obj 						\
			$(SLO)$/vclxsystemdependentwindow.obj		\
			$(SLO)$/vclxtoolkit.obj 					\
			$(SLO)$/vclxtopwindow.obj 					\
			$(SLO)$/vclxwindow.obj 						\
			$(SLO)$/vclxwindow1.obj 					\
			$(SLO)$/vclxwindows.obj                     \
			$(SLO)$/vclxspinbutton.obj                  \
			$(SLO)$/xsimpleanimation.obj                \
			$(SLO)$/xthrobber.obj						\
			$(SLO)$/asynccallback.obj\
			$(SLO)/vclxbutton.obj\
			$(SLO)/vclxdialog.obj\
			$(SLO)/vclxfixedline.obj\
	$(SLO)/vclxplugin.obj\
			$(SLO)/vclxscroller.obj\
			$(SLO)/vclxsplitter.obj\
	$(SLO)/vclxtabcontrol.obj\
	$(SLO)/vclxtabpage.obj

SRS1NAME=$(TARGET)
SRC1FILES=\
            xthrobber.src

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

