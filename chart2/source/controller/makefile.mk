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

PRJ=				..$/..
PRJNAME=			chart2
TARGET=				chartcontroller

USE_DEFFILE=		TRUE
ENABLE_EXCEPTIONS=	TRUE
VISIBILITY_HIDDEN=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE: $(PRJ)$/util$/makefile.pmk

.IF "$(UPD)" == "310"
PREPENDLIBS=$(PRJ)$/..$/svtools$/$(INPATH)$/lib \
	-L$(PRJ)$/..$/svx$/$(INPATH)$/lib

# Link to modified libraries
SOLARLIB:=-L$(PREPENDLIBS) $(SOLARLIB)
SOLARLIBDIR:=$(PREPENDLIBS) -L$(SOLARLIBDIR)
.ENDIF		# "$(UPD)" == "310"

# --- export library -------------------------------------------------

#You can use several library macros of this form to build libraries that
#do not consist of all object files in a directory or to merge different libraries.
LIB1TARGET=		$(SLB)$/$(TARGET).lib

LIB1FILES=		\
				$(SLB)$/chcontroller.lib \
			    $(SLB)$/chcdrawinglayer.lib \
			    $(SLB)$/chcitemsetwrapper.lib \
			    $(SLB)$/chcdialogs.lib \
				$(SLB)$/chchartapiwrapper.lib \
				$(SLB)$/chcaccessibility.lib

#--------

#Indicates the filename of the shared library.
SHL1TARGET=		$(TARGET)$(DLLPOSTFIX)

#indicates dependencies:
.IF "$(COM)" == "MSC"
SHL1DEPN = \
		$(LB)$/icharttools.lib \
		$(LB)$/ichartview.lib
.ELSE
SHL1DEPN =
.ENDIF

#Specifies an import library to create. For Win32 only.
SHL1IMPLIB=		i$(TARGET)

#Specifies libraries from the same module to put into the shared library.
#was created above
SHL1LIBS= 		$(LIB1TARGET)

#Links import libraries.

SHL1STDLIBS=	$(CHARTTOOLS)		\
                $(CHARTVIEW)		\
				$(CPPULIB)			\
				$(CPPUHELPERLIB)	\
				$(COMPHELPERLIB)	\
				$(BASEGFXLIB)		\
	            $(DRAWINGLAYERLIB)	\
				$(GOODIESLIB)		\
				$(BASEGFXLIB) 		\
				$(SALLIB)			\
				$(SVLLIB)			\
				$(SVTOOLLIB)		\
				$(SVXCORELIB)			\
				$(SVXLIB)			\
				$(TKLIB)			\
				$(TOOLSLIB) 		\
				$(I18NISOLANGLIB)   \
				$(VCLLIB)           \
			    $(SFXLIB)			\
				$(UNOTOOLSLIB)		\
				$(SOTLIB)

#--------exports

#specifies the exported symbols for Windows only:
SHL1DEF=		$(MISC)$/$(SHL1TARGET).def

SHL1VERSIONMAP = controller.map

#--------definition file

#name of the definition file:
DEF1NAME=		$(SHL1TARGET)

# --- Resources ---------------------------------------------------------------

# sfx.srs is needed for the strings for UNDO and REDO in the UndoCommandDispatch
RESLIB1LIST=\
	$(SRS)$/chcdialogs.srs \
	$(SOLARCOMMONRESDIR)$/sfx.srs

RESLIB1NAME=	$(TARGET)
RESLIB1IMAGES=$(PRJ)$/res
RESLIB1SRSFILES=$(RESLIB1LIST)
RESLIB1DEPN=$(RESLIB1LIST)

#RESLIB1SRSFILES=$(SRS)$/$(TARGET).srs
#RESLIB1DEPN=	SRCFILES

# --- Targets -----------------------------------------------------------------

.INCLUDE: target.mk
