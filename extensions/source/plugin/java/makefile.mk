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
#   Patrick Luby, December 2007
# 
#   GNU General Public License Version 2.1
#   =============================================
#   Copyright 2007 Planamesa Inc.
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
##########################################################################

PRJ=..$/..$/..

PRJNAME=extensions
TARGET=pljava
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.IF "$(GUIBASE)" == "java"

.INCLUDE :  ..$/util$/makefile.pmk

INCPRE+=-I$(SOLARINCDIR)$/mozilla$/plugin
.IF "$(SOLAR_JAVA)" != ""
INCPRE+=-I$(SOLARINCDIR)$/mozilla$/java
INCPRE+=-I$(SOLARINCDIR)$/mozilla$/nspr
CDEFS+=-DOJI
.ENDIF

# --- Types -------------------------------------

.IF "$(WITH_MOZILLA)" != "NO"

SLOFILES =	$(SLO)$/javamgr.obj	\
			$(SLO)$/sysplug.obj

.ENDIF	# $(WITH_MOZILLA) != "NO"

.ENDIF	# "$(GUIBASE)" == "java"

# ------------------------------------------------------------------

.INCLUDE :  target.mk
