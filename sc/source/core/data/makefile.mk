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
# Modified December 2016 by Patrick Luby. NeoOffice is distributed under
# GPL only under modification term 2 of the LGPL.
#
#*************************************************************************

PRJ=..$/..$/..

PRJNAME=sc
TARGET=data

PROJECTPCH4DLL=TRUE
PROJECTPCH=core_pch
PROJECTPCHSOURCE=..\pch\core_pch

AUTOSEG=true

# --- Settings -----------------------------------------------------

.INCLUDE :  scpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sc.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

.IF "$(UPD)" == "310"
INCLOCAL += \
	-I$(PRJ)$/..$/svx$/inc
.ENDIF		# "$(UPD)" == "310"

# --- Files --------------------------------------------------------

SLOFILES =  \
	$(SLO)$/attarray.obj \
	$(SLO)$/attrib.obj \
    $(SLO)$/autonamecache.obj \
	$(SLO)$/bcaslot.obj \
	$(SLO)$/cell.obj \
	$(SLO)$/cell2.obj \
        $(SLO)$/clipparam.obj \
	$(SLO)$/column.obj \
	$(SLO)$/column2.obj \
	$(SLO)$/column3.obj \
	$(SLO)$/compressedarray.obj \
	$(SLO)$/conditio.obj \
	$(SLO)$/dbdocutl.obj \
	$(SLO)$/dociter.obj \
	$(SLO)$/docpool.obj \
	$(SLO)$/documen2.obj \
	$(SLO)$/documen3.obj \
	$(SLO)$/documen4.obj \
	$(SLO)$/documen5.obj \
	$(SLO)$/documen6.obj \
	$(SLO)$/documen7.obj \
	$(SLO)$/documen8.obj \
	$(SLO)$/documen9.obj \
	$(SLO)$/document.obj \
        $(SLO)$/dpcachetable.obj \
        $(SLO)$/dpdimsave.obj \
        $(SLO)$/dpgroup.obj \
	$(SLO)$/dpobject.obj \
	$(SLO)$/dpoutput.obj \
	$(SLO)$/dpoutputgeometry.obj \
	$(SLO)$/dpsave.obj \
	$(SLO)$/dpsdbtab.obj \
	$(SLO)$/dpshttab.obj \
	$(SLO)$/dptabdat.obj \
	$(SLO)$/dptabres.obj \
	$(SLO)$/dptabsrc.obj \
	$(SLO)$/drawpage.obj \
	$(SLO)$/drwlayer.obj \
	$(SLO)$/fillinfo.obj \
	$(SLO)$/global.obj \
	$(SLO)$/global2.obj \
	$(SLO)$/globalx.obj \
	$(SLO)$/markarr.obj \
	$(SLO)$/markdata.obj \
	$(SLO)$/olinetab.obj \
	$(SLO)$/pagepar.obj \
	$(SLO)$/patattr.obj \
	$(SLO)$/pivot.obj \
	$(SLO)$/pivot2.obj \
	$(SLO)$/phonetic.obj \
	$(SLO)$/poolhelp.obj \
	$(SLO)$/scimpexpmsg.obj \
	$(SLO)$/segmenttree.obj \
	$(SLO)$/sortparam.obj \
	$(SLO)$/stlpool.obj \
	$(SLO)$/stlsheet.obj \
	$(SLO)$/table1.obj \
	$(SLO)$/table2.obj \
	$(SLO)$/table3.obj \
	$(SLO)$/table4.obj \
	$(SLO)$/table5.obj \
	$(SLO)$/table6.obj \
	$(SLO)$/tabprotection.obj \
	$(SLO)$/userdat.obj \
	$(SLO)$/validat.obj \
	$(SLO)$/postit.obj

EXCEPTIONSFILES= \
    $(SLO)$/autonamecache.obj \
    $(SLO)$/bcaslot.obj \
    $(SLO)$/cell2.obj \
    $(SLO)$/column.obj \
    $(SLO)$/column3.obj \
    $(SLO)$/documen2.obj \
    $(SLO)$/document.obj \
    $(SLO)$/dpdimsave.obj \
    $(SLO)$/dpgroup.obj \
	$(SLO)$/dpshttab.obj \
    $(SLO)$/dptabres.obj \
	$(SLO)$/dptabdat.obj \
	$(SLO)$/global2.obj \
    $(SLO)$/table1.obj \
    $(SLO)$/table3.obj \
	$(SLO)$/tabprotection.obj \
	$(SLO)$/postit.obj \
	$(SLO)$/documen3.obj \
	$(SLO)$/documen5.obj \
	$(SLO)$/documen6.obj \
	$(SLO)$/documen9.obj \
        $(SLO)$/dpcachetable.obj \
	$(SLO)$/dpsdbtab.obj \
	$(SLO)$/dpobject.obj \
	$(SLO)$/dpoutput.obj \
	$(SLO)$/dpoutputgeometry.obj \
	$(SLO)$/dpsave.obj \
	$(SLO)$/dbdocutl.obj \
	$(SLO)$/dptabsrc.obj \
	$(SLO)$/drwlayer.obj \
	$(SLO)$/globalx.obj \
	$(SLO)$/segmenttree.obj

.IF "$(OS)$(COM)$(CPUNAME)"=="LINUXGCCSPARC"
NOOPTFILES= \
		$(SLO)$/column2.obj \
		$(SLO)$/column3.obj \
		$(SLO)$/table3.obj \
		$(SLO)$/table4.obj  \
		$(SLO)$/documen4.obj \
		$(SLO)$/conditio.obj \
		$(SLO)$/validat.obj
EXCEPTIONSNOOPTFILES= \
        $(SLO)$/cell.obj
.ELSE
EXCEPTIONSFILES+= \
	$(SLO)$/cell.obj \
	$(SLO)$/global.obj
.ENDIF

# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk

