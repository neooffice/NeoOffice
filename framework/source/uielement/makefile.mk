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
# Modified December 2009 by Patrick Luby. NeoOffice is distributed under
# GPL only under modification term 2 of the LGPL.
#
#*************************************************************************
PRJ=..$/..

PRJNAME=            framework
TARGET=             fwk_uielement
USE_DEFFILE=        TRUE
ENABLE_EXCEPTIONS=  TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :          settings.mk

.IF "$(GUIBASE)"=="java"
CDEFS+=-DDLLPOSTFIX=$(DLLPOSTFIX)
.ENDIF      # "$(GUIBASE)"=="java"

# --- Generate -----------------------------------------------------

SLOFILES=           \
                    $(SLO)$/addonstoolbarmanager.obj            \
                    $(SLO)$/addonstoolbarwrapper.obj            \
                    $(SLO)$/buttontoolbarcontroller.obj         \
                    $(SLO)$/comboboxtoolbarcontroller.obj       \
                    $(SLO)$/complextoolbarcontroller.obj        \
                    $(SLO)$/constitemcontainer.obj              \
                    $(SLO)$/controlmenucontroller.obj           \
                    $(SLO)$/dropdownboxtoolbarcontroller.obj    \
                    $(SLO)$/edittoolbarcontroller.obj           \
                    $(SLO)$/fontmenucontroller.obj              \
                    $(SLO)$/fontsizemenucontroller.obj          \
                    $(SLO)$/footermenucontroller.obj            \
                    $(SLO)$/generictoolbarcontroller.obj        \
                    $(SLO)$/headermenucontroller.obj            \
                    $(SLO)$/imagebuttontoolbarcontroller.obj    \
                    $(SLO)$/itemcontainer.obj                   \
                    $(SLO)$/langselectionmenucontroller.obj     \
                    $(SLO)$/langselectionstatusbarcontroller.obj \
                    $(SLO)$/logoimagestatusbarcontroller.obj    \
                    $(SLO)$/logotextstatusbarcontroller.obj     \
                    $(SLO)$/macrosmenucontroller.obj            \
                    $(SLO)$/menubarmanager.obj                  \
                    $(SLO)$/menubarmerger.obj                   \
                    $(SLO)$/menubarwrapper.obj                  \
                    $(SLO)$/newmenucontroller.obj               \
                    $(SLO)$/objectmenucontroller.obj            \
                    $(SLO)$/progressbarwrapper.obj              \
                    $(SLO)$/recentfilesmenucontroller.obj       \
                    $(SLO)$/rootitemcontainer.obj               \
                    $(SLO)$/simpletextstatusbarcontroller.obj   \
                    $(SLO)$/spinfieldtoolbarcontroller.obj      \
                    $(SLO)$/statusbar.obj                       \
                    $(SLO)$/statusbarmanager.obj                \
                    $(SLO)$/statusbarwrapper.obj                \
                    $(SLO)$/statusindicatorinterfacewrapper.obj \
                    $(SLO)$/togglebuttontoolbarcontroller.obj   \
                    $(SLO)$/toolbar.obj                         \
                    $(SLO)$/toolbarmanager.obj                  \
                    $(SLO)$/toolbarmerger.obj                   \
                    $(SLO)$/toolbarsmenucontroller.obj          \
                    $(SLO)$/toolbarwrapper.obj                  \
                    $(SLO)$/uicommanddescription.obj            \

# --- Targets ------------------------------------------------------

.INCLUDE :          target.mk
