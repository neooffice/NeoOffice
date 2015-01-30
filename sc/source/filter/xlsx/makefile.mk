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
# Modified November 2014 by Patrick Luby. NeoOffice is distributed under
# GPL only under modification term 2 of the LGPL.
#
#*************************************************************************

PRJ=..$/..$/..

PRJNAME=sc
TARGET=xlsx

AUTOSEG=true

PROJECTPCH4DLL=TRUE
PROJECTPCH=filt_pch
PROJECTPCHSOURCE=..\pch\filt_pch

# --- Settings -----------------------------------------------------

.INCLUDE :  scpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sc.mk

CFLAGS+=-DXLSX_COPY

.IF "$(UPD)" == "310"
.INCLUDE : $(PRJ)$/inc$/sc.mk
.ENDIF		# "$(UPD)" == "310"

# --- Files --------------------------------------------------------

SLOFILES =	\
		$(SLO)$/xlsx-biffdump.obj				\
		$(SLO)$/xlsx-colrowst.obj				\
		$(SLO)$/xlsx-excdoc.obj					\
		$(SLO)$/xlsx-excel.obj					\
		$(SLO)$/xlsx-excform.obj					\
		$(SLO)$/xlsx-excform8.obj				\
		$(SLO)$/xlsx-excimp8.obj					\
		$(SLO)$/xlsx-excrecds.obj				\
		$(SLO)$/xlsx-exctools.obj				\
		$(SLO)$/xlsx-expop2.obj					\
		$(SLO)$/xlsx-fontbuff.obj				\
		$(SLO)$/xlsx-frmbase.obj					\
		$(SLO)$/xlsx-impop.obj					\
		$(SLO)$/xlsx-namebuff.obj				\
		$(SLO)$/xlsx-read.obj					\
		$(SLO)$/xlsx-tokstack.obj				\
		$(SLO)$/xlsx-xechart.obj					\
		$(SLO)$/xlsx-xecontent.obj				\
		$(SLO)$/xlsx-xeescher.obj				\
		$(SLO)$/xlsx-xeformula.obj				\
		$(SLO)$/xlsx-xehelper.obj				\
		$(SLO)$/xlsx-xelink.obj					\
		$(SLO)$/xlsx-xename.obj					\
		$(SLO)$/xlsx-xepage.obj					\
		$(SLO)$/xlsx-xepivot.obj					\
		$(SLO)$/xlsx-xerecord.obj				\
		$(SLO)$/xlsx-xeroot.obj					\
		$(SLO)$/xlsx-xestream.obj				\
		$(SLO)$/xlsx-xestring.obj				\
		$(SLO)$/xlsx-xestyle.obj					\
		$(SLO)$/xlsx-xetable.obj					\
		$(SLO)$/xlsx-xeview.obj					\
		$(SLO)$/xlsx-xichart.obj					\
		$(SLO)$/xlsx-xicontent.obj				\
		$(SLO)$/xlsx-xiescher.obj				\
		$(SLO)$/xlsx-xiformula.obj				\
		$(SLO)$/xlsx-xihelper.obj				\
		$(SLO)$/xlsx-xilink.obj					\
		$(SLO)$/xlsx-xiname.obj					\
		$(SLO)$/xlsx-xipage.obj					\
		$(SLO)$/xlsx-xipivot.obj					\
		$(SLO)$/xlsx-xiroot.obj					\
		$(SLO)$/xlsx-xistream.obj				\
		$(SLO)$/xlsx-xistring.obj				\
		$(SLO)$/xlsx-xistyle.obj					\
		$(SLO)$/xlsx-xiview.obj					\
		$(SLO)$/xlsx-xladdress.obj				\
		$(SLO)$/xlsx-xlchart.obj					\
		$(SLO)$/xlsx-xlescher.obj				\
		$(SLO)$/xlsx-xlformula.obj				\
		$(SLO)$/xlsx-xlpage.obj					\
		$(SLO)$/xlsx-xlpivot.obj					\
		$(SLO)$/xlsx-xlroot.obj					\
		$(SLO)$/xlsx-xlstream.obj				\
		$(SLO)$/xlsx-xlstyle.obj					\
		$(SLO)$/xlsx-xltools.obj					\
		$(SLO)$/xlsx-xltracer.obj				\
		$(SLO)$/xlsx-XclExpChangeTrack.obj				\
		$(SLO)$/xlsx-xcl97esc.obj				\
		$(SLO)$/xlsx-xcl97rec.obj				\
		$(SLO)$/xlsx-xlview.obj

.IF "$(OS)$(COM)$(CPUNAME)"=="LINUXGCCSPARC"
NOOPTFILES = \
		$(SLO)$/xlsx-xiescher.obj
.ENDIF

EXCEPTIONSFILES = \
		$(SLO)$/xlsx-excdoc.obj					\
		$(SLO)$/xlsx-excel.obj					\
		$(SLO)$/xlsx-excform.obj					\
		$(SLO)$/xlsx-excform8.obj				\
		$(SLO)$/xlsx-excimp8.obj					\
		$(SLO)$/xlsx-excrecds.obj				\
		$(SLO)$/xlsx-expop2.obj					\
		$(SLO)$/xlsx-namebuff.obj				\
		$(SLO)$/xlsx-tokstack.obj				\
		$(SLO)$/xlsx-xecontent.obj				\
		$(SLO)$/xlsx-xeescher.obj				\
		$(SLO)$/xlsx-xeformula.obj				\
		$(SLO)$/xlsx-xehelper.obj				\
		$(SLO)$/xlsx-xelink.obj					\
		$(SLO)$/xlsx-xename.obj					\
		$(SLO)$/xlsx-xepage.obj					\
		$(SLO)$/xlsx-xepivot.obj					\
		$(SLO)$/xlsx-xechart.obj					\
		$(SLO)$/xlsx-xestream.obj				\
		$(SLO)$/xlsx-xestring.obj				\
		$(SLO)$/xlsx-xestyle.obj					\
		$(SLO)$/xlsx-xetable.obj					\
		$(SLO)$/xlsx-xeview.obj					\
		$(SLO)$/xlsx-xichart.obj					\
		$(SLO)$/xlsx-xicontent.obj				\
		$(SLO)$/xlsx-xiescher.obj				\
		$(SLO)$/xlsx-xihelper.obj				\
		$(SLO)$/xlsx-xilink.obj					\
		$(SLO)$/xlsx-xipage.obj					\
		$(SLO)$/xlsx-xipivot.obj					\
		$(SLO)$/xlsx-xistream.obj				\
		$(SLO)$/xlsx-xistring.obj				\
		$(SLO)$/xlsx-xistyle.obj					\
		$(SLO)$/xlsx-xladdress.obj				\
		$(SLO)$/xlsx-xiescher.obj				\
		$(SLO)$/xlsx-xlchart.obj					\
		$(SLO)$/xlsx-xlformula.obj				\
		$(SLO)$/xlsx-xlpivot.obj					\
		$(SLO)$/xlsx-xlstyle.obj					\
		$(SLO)$/xlsx-xcl97esc.obj				\
		$(SLO)$/xlsx-xcl97rec.obj				\
		$(SLO)$/xlsx-xlview.obj

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk

