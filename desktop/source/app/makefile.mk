#*************************************************************************
#
#   $RCSfile$
#
#   $Revision$
#
#   last change: $Author$ $Date$
#
#   The Contents of this file are made available subject to
#   the terms of GNU General Public License Version 2.1.
#
#
#     GNU General Public License Version 2.1
#     =============================================
#     Copyright 2005 by Sun Microsystems, Inc.
#     901 San Antonio Road, Palo Alto, CA 94303, USA
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU General Public
#     License version 2.1, as published by the Free Software Foundation.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     General Public License for more details.
#
#     You should have received a copy of the GNU General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA  02111-1307  USA
#
#     Modified December 2005 by Patrick Luby. NeoOffice is distributed under
#     GPL only under modification term 3 of the LGPL.
#
#*************************************************************************

PRJ=..$/..

PRJNAME=desktop
TARGET=dkt
LIBTARGET=NO
AUTOSEG=true
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

.IF "$(PRODUCT_DIR_NAME)" != ""
CDEFS += -DPRODUCT_DIR_NAME='"$(PRODUCT_DIR_NAME)"'
.ENDIF

.IF "$(PRODUCT_DONATION_URL)" != ""
CDEFS += -DPRODUCT_DONATION_URL='"$(PRODUCT_DONATION_URL)"'
.ENDIF

# --- Files --------------------------------------------------------

OBJFILES = \
        $(OBJ)$/main.obj					\
        $(OBJ)$/app.obj						\
        $(OBJ)$/copyright_ascii_sun.obj		\
        $(OBJ)$/copyright_ascii_ooo.obj		\
        $(OBJ)$/lockfile.obj				\
        $(OBJ)$/lockfile2.obj				\
		$(OBJ)$/intro.obj					\
		$(OBJ)$/officeipcthread.obj			\
		$(OBJ)$/appinit.obj					\
		$(OBJ)$/cmdlineargs.obj				\
		$(OBJ)$/oinstanceprovider.obj		\
		$(OBJ)$/opluginframefactory.obj		\
		$(OBJ)$/appsys.obj					\
		$(OBJ)$/desktopresid.obj			\
		$(OBJ)$/dispatchwatcher.obj			\
		$(OBJ)$/configinit.obj				\
		$(OBJ)$/checkinstall.obj			\
		$(OBJ)$/langselect.obj			    \
		$(OBJ)$/cmdlinehelp.obj             \
		$(OBJ)$/userinstall.obj             \
		$(OBJ)$/desktopcontext.obj


SRS1NAME=	desktop
SRC1FILES=	desktop.src	

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

