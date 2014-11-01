# *************************************************************
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
# *************************************************************
PRJ=..$/..

PRJNAME=oox
TARGET=export
AUTOSEG=true

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE: $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

SLOFILES =	\
		$(SLO)$/drawingml.obj \
		$(SLO)$/shapes.obj \
		$(SLO)$/vmlexport.obj

.IF "$(UPD)" == "310"
SLOFILES += \
		$(SLO)$/ColorPropertySet.obj \
		$(SLO)$/chartexport.obj
.ELSE		# "$(UPD)" == "310"
SLOFILES += \
		$(SLO)$/vmlexport-shape-types.obj
.ENDIF		# "$(UPD)" == "310"

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk

.IF "$(UPD)" == "310"

ALLTAR: \
	$(MISC)$/oox-drawingml-adj-names \
	$(MISC)$/vmlexport-shape-types

$(MISC)$/oox-drawingml-adj-names : preset-definitions-to-shape-types.pl presetShapeDefinitions.xml presetTextWarpDefinitions.xml
	sh -c "$(PERL) preset-definitions-to-shape-types.pl --drawingml-adj-names-data presetShapeDefinitions.xml presetTextWarpDefinitions.xml > $@.in_progress 2> $@.log" && mv $@.in_progress $@

$(MISC)$/vmlexport-shape-types : preset-definitions-to-shape-types.pl presetShapeDefinitions.xml presetTextWarpDefinitions.xml
	sh -c "$(PERL) preset-definitions-to-shape-types.pl --vml-shape-types-data presetShapeDefinitions.xml presetTextWarpDefinitions.xml > $@.in_progress 2> $@.log" && mv $@.in_progress $@

.ELSE		# "$(UPD)" == "310"

$(MISC)$/vmlexport-shape-types.cxx : preset-definitions-to-shape-types.pl presetShapeDefinitions.xml presetTextWarpDefinitions.xml
	$(PERL) $< > $@.in_progress 2> $(MISC)$/vmlexport-shape-types.log && mv $@.in_progress $@

.ENDIF		# "$(UPD)" == "310"
