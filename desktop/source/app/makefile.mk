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
#   Sun Microsystems Inc., October, 2000
#
#   GNU General Public License Version 2.1
#   =============================================
#   Copyright 2000 by Sun Microsystems, Inc.
#   901 San Antonio Road, Palo Alto, CA 94303, USA
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
#   =================================================
#   Modified June 2003 by Patrick Luby. SISSL Removed. NeoOffice is
#   distributed under GPL only under modification term 3 of the LGPL.
#
#   Contributor(s): _______________________________________
#
##########################################################################

PRJ=..$/..

PRJNAME=desktop
TARGET=dkt
LIBTARGET=NO
AUTOSEG=true
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(UPD)minor.mk
RSCUPDVER=$(RSCREVISION)(SV$(UPD)$(UPDMINOR))

.IF "$(PRODUCT_DIR_NAME)" != ""
CDEFS += -DPRODUCT_DIR_NAME='"$(PRODUCT_DIR_NAME)"'
.ENDIF

.IF "$(BUILD_SOSL)" == ""
        CDEFS += "-DTIMEBOMB"
.ELSE
        CDEFS += "-DBUILD_SOSL"
.ENDIF

# --- Files --------------------------------------------------------

OBJFILES = \
        $(OBJ)$/app.obj					\
		$(OBJ)$/intro.obj				\
		$(OBJ)$/officeipcthread.obj		\
		$(OBJ)$/appinit.obj				\
		$(OBJ)$/cmdlineargs.obj			\
		$(OBJ)$/pluginacceptthread.obj	\
		$(OBJ)$/officeacceptthread.obj	\
		$(OBJ)$/oinstanceprovider.obj	\
		$(OBJ)$/opluginframefactory.obj	\
		$(OBJ)$/appsys.obj				\
		$(OBJ)$/desktopresid.obj		\
		$(OBJ)$/dispatchwatcher.obj

SRC1FILES=	desktop.src
SRS1NAME=	desktop

# --- Targets ------------------------------------------------------

.IF "$(depend)" != ""
SRCFILES=$(SRC1FILES)
.ENDIF

.INCLUDE :  target.mk

