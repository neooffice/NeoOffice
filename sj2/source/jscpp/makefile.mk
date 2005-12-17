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

PRJNAME=sj2
TARGET=jscpp
# --- Settings -----------------------------------------------------

# Applet support is not yet implemented
.IF "$(GUIBASE)"=="java"
SOLAR_JAVA:=
.ENDIF

.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

UNOTYPES= \
	com.sun.star.uno.Exception	\
	com.sun.star.uno.XInterface	\
	com.sun.star.uno.TypeClass	\
	com.sun.star.awt.XControl	\
	com.sun.star.lang.XMultiServiceFactory	\
	com.sun.star.java.XJavaVM				



CXXFILES=	\
	sjapplet.cxx							\
	sjapplet_impl.cxx 


#.IF "$(GUI)"=="UNX"
#CXXFILES += widget.cxx
#.ENDIF

SLOFILES=	\
	$(SLO)$/sjapplet.obj						\
	$(SLO)$/sjapplet_impl.obj


# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

.INCLUDE :  $(PRJ)$/util$/target.pmk

