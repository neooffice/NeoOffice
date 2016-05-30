#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
#  This file incorporates work covered by the following license notice:
# 
#    Modified May 2016 by Patrick Luby. NeoOffice is only distributed
#    under the GNU General Public License, Version 3 as allowed by Section 4
#    of the Apache License, Version 2.0.
#
#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.
#  
#**************************************************************



PRJ=..
TARGET=prj

.INCLUDE : settings.mk

.IF "$(VERBOSE)"!=""
VERBOSEFLAG :=
.ELSE
VERBOSEFLAG := -s
.ENDIF

.IF "$(DEBUG)"!=""
DEBUG_ARGUMENT=DEBUG=$(DEBUG)
.ELIF "$(debug)"!=""
DEBUG_ARGUMENT=debug=$(debug)
.ELSE
DEBUG_ARGUMENT=
.ENDIF

.IF "$(PRODUCT_BUILD_TYPE)" == "java"

SOLARSRC := $(shell cd $(PRJ)$/.. && pwd)
WORKDIR := $(shell cd $(PRJ) && pwd)$/$(INPATH)
gb_REPOS := $(SOLARSRC) 

all:
	cd $(PRJ) && $(GNUMAKE) $(VERBOSEFLAG) -r -j$(MAXPROCESS) $(gb_MAKETARGET) $(DEBUG_ARGUMENT) SOLARSRC="$(SOLARSRC)" gb_REPOS="$(gb_REPOS)" WORKDIR="$(WORKDIR)"

.ELSE		# "$(PRODUCT_TYPE)" == "java"

all:
	cd $(PRJ) && $(GNUMAKE) $(VERBOSEFLAG) -r -j$(MAXPROCESS) $(gb_MAKETARGET) $(DEBUG_ARGUMENT)

.ENDIF		# "$(PRODUCT_TYPE)" == "java"
