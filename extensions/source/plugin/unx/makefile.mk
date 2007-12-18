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
#     Modified December 2007 by Patrick Luby. NeoOffice is distributed under
#     GPL only under modification term 3 of the LGPL.
#
#*************************************************************************
PRJ=..$/..$/..

PRJNAME=extensions
TARGET=plunx
TARGETTYPE=CUI

.INCLUDE :  ..$/util$/makefile.pmk

# --- Files --------------------------------------------------------

INCPRE+=-I$(SOLARINCDIR)$/mozilla$/plugin
.IF "$(SOLAR_JAVA)" != ""
INCPRE+=-I$(SOLARINCDIR)$/mozilla$/java
INCPRE+=-I$(SOLARINCDIR)$/mozilla$/nspr
CDEFS+=-DOJI
.ENDIF

.IF "$(GUIBASE)" != "java"

.IF "$(WITH_MOZILLA)" != "NO"

.IF "$(DISABLE_XAW)" == "TRUE"
CDEFS+=-DDISABLE_XAW
.ENDIF

SLOFILES=\
	$(SLO)$/nppapi.obj		\
	$(SLO)$/sysplug.obj		\
	$(SLO)$/mediator.obj	\
	$(SLO)$/plugcon.obj		\
	$(SLO)$/unxmgr.obj

OBJFILES=\
	$(OBJ)$/npwrap.obj		\
	$(OBJ)$/npnapi.obj		\
	$(OBJ)$/mediator.obj	\
	$(OBJ)$/plugcon.obj

APP1TARGET=pluginapp.bin
APP1OBJS=$(OBJFILES)
APP1STDLIBS=\
	$(TOOLSLIB) 				\
	$(VOSLIB)					\
	$(SALLIB)
.IF "$(OS)"=="SOLARIS" || "$(OS)"=="SCO" || "$(OS)"=="HPUX"
APP1STDLIBS+=-lXm -lXt -lXext -lX11 -ldl
.ELSE
.IF "$(DISABLE_XAW)" != "TRUE"
APP1STDLIBS+=-lXaw 
.ENDIF
.IF "$(OS)"=="FREEBSD" || "$(OS)"=="NETBSD"
APP1STDLIBS+= -lXt -lXext -lX11
.ELSE
APP1STDLIBS+= -lXt -lXext -lX11 -ldl
.ENDIF
.ENDIF

.IF "$(HAVE_READDIR_R)" != "YES"
.IF "$(OS)" == "WNT"
APP1STDLIBS+= gnu_readdir.lib
.ELSE
APP1STDLIBS+=-lgnu_readdir_r
.ENDIF
.ENDIF

APP1DEF=	$(MISC)$/$(TARGET).def

.ENDIF # $(WITH_MOZILLA) != "NO"

.ENDIF		# "$(GUIBASE)" != "java"

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

