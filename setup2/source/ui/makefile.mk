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

PROJECTPCH=setuppch
PROJECTPCHSOURCE=$(PRJ)$/source\pch\setuppch

PRJNAME=setup2
TARGET=ui

# --- Settings -----------------------------------------------------

.INCLUDE :  svpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sv.mk

# --- Files --------------------------------------------------------

CXXFILES =           		\
		main.cxx     		\
		mainwnd.cxx  		\
		mainevt.cxx  		\
		magenda.cxx  		\
		rspwizard.cxx  		\
		textani.cxx

SRCFILES =				\
		error.src		\
		app.src

SRS1NAME = 				\
		appmain

SRC1FILES = 			\
		appmain.src

EXCEPTIONSFILES=\
		$(OBJ)$/main.obj

OBJFILES =				\
		$(OBJ)$/main.obj	\
		$(OBJ)$/mainwnd.obj	\
		$(OBJ)$/mainevt.obj	\
		$(OBJ)$/magenda.obj	\
		$(OBJ)$/rspwizard.obj	\
		$(OBJ)$/textani.obj	\

.IF "$(GUI)"=="UNX"
.IF "$(OS)"=="MACOSX"
.IF "$(GUIBASE)"=="unx" || "$(GUIBASE)"=="java"
OBJFILES += $(OBJ)$/unxmain.obj
.ENDIF
.ELSE
OBJFILES += $(OBJ)$/unxmain.obj
.ENDIF
.ENDIF

# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk

