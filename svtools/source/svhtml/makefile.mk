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
#   Modified November 2004 by Patrick Luby. SISSL Removed. NeoOffice is
#   distributed under GPL only under modification term 3 of the LGPL.
# 
#   Contributor(s): _______________________________________
# 
##########################################################################

PRJ=..$/..

PRJNAME=SVTOOLS
TARGET=svhtml
AUTOSEG=true
LIBTARGET=NO

# --- Settings -----------------------------------------------------

.INCLUDE :  svpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sv.mk

# --- Files --------------------------------------------------------

.IF "$(header)" == ""

.IF "$(OS)"=="MACOSX"
CFLAGSOPT=-O
.ENDIF

CXXFILES = \
		parhtml.cxx 		\
		htmlkywd.cxx 	\
		htmlkey2.cxx 	\
		htmlsupp.cxx 	\
		htmlout.cxx

OBJFILES =  $(OBJ)$/htmlkey2.obj

SLOFILES=\
		$(LIB3OBJFILES) \
		$(LIB4OBJFILES)

# nur damit's was zum builden gibt
LIB2TARGET =$(LB)$/svhtmlk2.lib
LIB2OBJFILES  =$(OBJFILES)

LIB3TARGET=$(SLB)$/svhtml1.lib
LIB3OBJFILES=\
	$(SLO)$/htmlkey2.obj	\
	$(SLO)$/htmlkywd.obj	\
	$(SLO)$/htmlsupp.obj	\
	$(SLO)$/parhtml.obj

LIB4TARGET=$(SLB)$/svhtml2.lib
LIB4OBJFILES=\
	$(SLO)$/htmlout.obj

.ENDIF

# ==========================================================================

.INCLUDE :  target.mk
