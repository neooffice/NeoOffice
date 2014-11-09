#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.
#  
#  $RCSfile$
#  $Revision$
#  
#  This file is part of NeoOffice.
#  
#  NeoOffice is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License version 3
#  only, as published by the Free Software Foundation.
#  
#  NeoOffice is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License version 3 for more details
#  (a copy is included in the LICENSE file that accompanied this code).
#  
#  You should have received a copy of the GNU General Public License
#  version 3 along with NeoOffice.  If not, see
#  <http://www.gnu.org/licenses/gpl-3.0.txt>
#  for a copy of the GPLv3 License.
#  
#  Modified November 2014 by Patrick Luby. NeoOffice is distributed under
#  GPL only under Section 4 of the Apache License v2.0.
#  
#**************************************************************
PRJ=..$/..
PRJNAME=writerfilter
TARGET=ooxml
#LIBTARGET=NO
#USE_DEFFILE=TRUE
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/inc$/writerfilter.mk

#CFLAGS+=-DISOLATION_AWARE_ENABLED -DWIN32_LEAN_AND_MEAN -DXML_UNICODE -D_NTSDK -DUNICODE -D_UNICODE -D_WIN32_WINNT=0x0501
#CFLAGS+=-wd4710 -wd4711 -wd4514 -wd4619 -wd4217 -wd4820

NAMESPACES= \
	wml \
	dml-stylesheet \
	dml-styleDefaults \
	dml-shape3DLighting \
	dml-shape3DScene \
	dml-shape3DStyles \
	dml-shape3DCamera \
	dml-baseStylesheet \
	dml-textCharacter \
	dml-shapeEffects \
	dml-shapeLineProperties \
	dml-shapeProperties \
	dml-baseTypes \
	dml-documentProperties \
	dml-graphicalObject \
	dml-shapeGeometry \
	dml-wordprocessingDrawing \
	sml-customXmlMappings \
	shared-math \
	shared-relationshipReference \
	dml-chartDrawing \
	vml-main \
	vml-officeDrawing \
	vml-wordprocessingDrawing

.IF "$(UPD)" == "310"
NAMESPACES += \
	a14 \
	mce \
	w14 \
	wp14
.ENDIF		# "$(UPD)" == "310"

# --- Files --------------------------------------------------------

SLOFACTORIESNAMESPACES= \
    $(SLO)$/OOXMLFactory_{$(NAMESPACES)}.obj

SLOFILES= \
    $(SLOFACTORIESNAMESPACES) \
    $(SLOFASTNAMESPACES) \
    $(SLO)$/OOXMLFactory_generated.obj \
    $(SLO)$/OOXMLFactory.obj \
	$(SLO)$/OOXMLBinaryObjectReference.obj\
	$(SLO)$/OOXMLFactory_values.obj \
	$(SLO)$/OOXMLPropertySetImpl.obj \
	$(SLO)$/OOXMLParserState.obj \
	$(SLO)$/Handler.obj \
	$(SLO)$/OOXMLDocumentImpl.obj \
	$(SLO)$/OOXMLStreamImpl.obj \
	$(SLO)$/OOXMLFastDocumentHandler.obj \
	$(SLO)$/OOXMLFastContextHandler.obj \
	$(SLO)$/OOXMLFastTokenHandler.obj

.IF "$(UPD)" != "310"
SHL1TARGET=$(TARGET)

.IF "$(GUI)"=="UNX" || "$(GUI)"=="MAC" || "$(GUI)"=="OS2"
RESOURCEMODELLIB=-lresourcemodel
.ELIF "$(GUI)"=="WNT"
.IF "$(COM)"=="GCC"
RESOURCEMODELLIB=-lresourcemodel
.ELSE
RESOURCEMODELLIB=$(LB)$/iresourcemodel.lib
.ENDIF
.ENDIF

SHL1STDLIBS=$(SALLIB)\
	$(CPPULIB)\
	$(CPPUHELPERLIB) \
	$(COMPHELPERLIB) \
	$(RESOURCEMODELLIB)
SHL1IMPLIB=i$(SHL1TARGET)
SHL1USE_EXPORTS=name

SHL1OBJS=$(SLOFILES)

SHL1DEF=$(MISC)$/$(SHL1TARGET).def
DEF1NAME=$(SHL1TARGET)
DEFLIB1NAME=$(TARGET)
.ENDIF		# "$(UPD)" != "310"

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk
.IF "$(UPD)" == "310"
GPERF=gperf
.ENDIF		# "$(UPD)" == "310"

.PHONY : test

test:
	echo $(SLOFILES)

OOXMLCXXOUTDIR=$(MISC)
OOXMLHXXOUTDIR=$(INCCOM)/ooxml
OOXMLHXXOUTDIRCREATED=$(OOXMLHXXOUTDIR)/created

OOXMLFACTORYCXXS=$(OOXMLCXXOUTDIR)$/OOXMLFactory_{$(NAMESPACES)}.cxx
OOXMLFACTORYHXXS=$(OOXMLHXXOUTDIR)$/OOXMLFactory_{$(NAMESPACES)}.hxx

OOXMLMODEL=model.xml
OOXMLPREPROCESSXSL=modelpreprocess.xsl
OOXMLFACTORYTOOLSXSL=factorytools.xsl
OOXMLRESORUCETOKENSXSL=resourcestokens.xsl
OOXMLFASTTOKENSXSL=fasttokens.xsl
OOXMLRESOURCESIMPLXSL=resourcesimpl.xsl
OOXMLNAMESPACEIDSXSL=namespaceids.xsl
OOXMLFACTORYVALUESXSL=factory_values.xsl
OOXMLFACTORYVALUESIMPLXSL=factoryimpl_values.xsl
OOXMLRESOURCEIDSXSL=resourceids.xsl
OOXMLGPERFFASTTOKENXSL=gperffasttokenhandler.xsl

OOXMLRESOURCEIDSHXX=$(OOXMLHXXOUTDIR)$/resourceids.hxx

TOKENXML=$(OOXMLCXXOUTDIR)$/token.xml
TOKENXMLTMP=$(OOXMLCXXOUTDIR)$/token.tmp

OOXMLFACTORYGENERATEDHXX=$(OOXMLHXXOUTDIR)$/OOXMLFactory_generated.hxx
OOXMLFACTORYGENERATEDCXX=$(OOXMLCXXOUTDIR)$/OOXMLFactory_generated.cxx
OOXMLFASTTOKENSHXX=$(OOXMLHXXOUTDIR)$/OOXMLFastTokens.hxx
OOXMLNAMESPACEIDSHXX=$(OOXMLHXXOUTDIR)$/OOXMLnamespaceids.hxx
OOXMLFACTORYVALUESHXX=$(OOXMLCXXOUTDIR)$/OOXMLFactory_values.hxx
OOXMLFACTORYVALUESCXX=$(OOXMLCXXOUTDIR)$/OOXMLFactory_values.cxx
GPERFFASTTOKENHXX=$(OOXMLHXXOUTDIR)$/gperffasttoken.hxx
MODELPROCESSED=$(MISC)$/model_preprocessed.xml

OOXMLGENHEADERS= \
	$(OOXMLFACTORYGENERATEDHXX) \
	$(OOXMLFACTORYHXXS) \
	$(OOXMLFASTTOKENSHXX) \
	$(OOXMLFACTORYVALUESHXX) \
	$(GPERFFASTTOKENHXX) \
	$(OOXMLRESOURCEIDSHXX) \
	$(OOXMLNAMESPACEIDSHXX)

GENERATEDFILES= \
	$(OOXMLGENHEADERS) \
	$(OOXMLFACTORYGENERATEDCXX) \
	$(OOXMLFACTORYCXXS) \
	$(OOXMLFACTORYVALUESCXX) \
	$(TOKENXMLTMP) \
	$(TOKENXML)

.IF "$(UPD)" == "310"
$(TOKENXMLTMP): $(PRJ)$/..$/oox$/source$/token$/tokens.txt
    @$(TYPE) $(PRJ)$/..$/oox$/source$/token$/tokens.txt | $(SED) "s#\(.*\)#<fasttoken>\1</fasttoken>#" > $@
.ELSE		# "$(UPD)" == "310"
$(TOKENXMLTMP): $(SOLARVER)$/$(INPATH)$/inc$(UPDMINOREXT)$/oox$/token$/tokens.txt
    @$(TYPE) $(SOLARVER)$/$(INPATH)$/inc$(UPDMINOREXT)$/oox$/token$/tokens.txt | $(SED) "s#\(.*\)#<fasttoken>\1</fasttoken>#" > $@
.ENDIF		# "$(UPD)" == "310"

$(TOKENXML): tokenxmlheader $(TOKENXMLTMP) tokenxmlfooter
	@$(TYPE) tokenxmlheader $(TOKENXMLTMP) tokenxmlfooter > $@

$(OOXMLHXXOUTDIRCREATED):
	$(MKDIRHIER) $(OOXMLHXXOUTDIR)
	@$(TOUCH) $@

$(OOXMLGENHEADERS): $(OOXMLHXXOUTDIRCREATED)

$(OOXMLFASTTOKENSHXX): $(OOXMLFASTTOKENSXSL) $(TOKENXML)
    @echo "Making:   " $(@:f)   
	$(COMMAND_ECHO)$(XSLTPROC) $(OOXMLFASTTOKENSXSL) $(TOKENXML) > $@

$(OOXMLFACTORYGENERATEDHXX): factoryinc.xsl
    @echo "Making:   " $(@:f)   
    $(COMMAND_ECHO)$(XSLTPROC) $< $(MODELPROCESSED) > $@

$(OOXMLFACTORYGENERATEDCXX): factoryimpl.xsl
    @echo "Making:   " $(@:f)   
    $(COMMAND_ECHO)$(XSLTPROC) $< $(MODELPROCESSED) > $@

$(OOXMLFACTORYGENERATEDCXX): $(MODELPROCESSED)

$(OOXMLFACTORYGENERATEDHXX): $(MODELPROCESSED)

$(OOXMLFACTORYCXXS): $(MODELPROCESSED)

$(OOXMLFACTORYHXXS): $(MODELPROCESSED)

$(OOXMLCXXOUTDIR)$/OOXMLFactory%.cxx: factoryimpl_ns.xsl
    @echo "Making:   " $(@:f)   
	$(COMMAND_ECHO)$(XSLTPROC) --stringparam file $@ $< $(MODELPROCESSED) > $@

$(OOXMLHXXOUTDIR)$/OOXMLFactory%.hxx: factory_ns.xsl
    @echo "Making:   " $(@:f)   
	$(COMMAND_ECHO)$(XSLTPROC) --stringparam file $@ $< $(MODELPROCESSED) > $@

$(OOXMLFACTORYVALUESHXX): $(OOXMLFACTORYVALUESXSL) $(MODELPROCESSED)
    @echo "Making:   " $(@:f)   
	$(COMMAND_ECHO)$(XSLTPROC) $(OOXMLFACTORYVALUESXSL) $(MODELPROCESSED) > $@

.IF "$(UPD)" != "310"
$(OOXMLFACTORYVALUESCXX): $(OOXMLFACTORYVALUESIMPLXSL) $(MODELPROCESSED)
    @echo "Making:   " $(@:f)   
	$(COMMAND_ECHO)$(XSLTPROC) $(OOXMLFACTORYVALUESIMPLXSL) $(MODELPROCESSED) > $@
.ENDIF		# "$(UPD)" != "310"

$(OOXMLRESOURCEIDSHXX):  $(OOXMLHXXOUTDIRCREATED) $(OOXMLRESOURCEIDSXSL) \
	$(MODELPROCESSED)
    @echo "Making:   " $(@:f)   
	$(COMMAND_ECHO)$(XSLTPROC) $(OOXMLRESOURCEIDSXSL) $(MODELPROCESSED) > $@

$(OOXMLNAMESPACEIDSHXX):  $(OOXMLHXXOUTDIRCREATED) $(OOXMLNAMESPACEIDSXSL) \
	$(MODELPROCESSED)
    @echo "Making:   " $(@:f)   
	$(COMMAND_ECHO)$(XSLTPROC) $(OOXMLNAMESPACEIDSXSL) $(MODELPROCESSED) > $@

.IF "$(UPD)" == "310"
$(GPERFFASTTOKENHXX): $(OOXMLGPERFFASTTOKENXSL) $(TOKENXML)
    @echo "Making:   " $(@:f)   
	$(COMMAND_ECHO)$(XSLTPROC) $(OOXMLGPERFFASTTOKENXSL) $(TOKENXML) | tr -d '\r' | $(GPERF) -c -E -G -I  -LC++ -S1 -t  > $@
.ELSE		# "$(UPD)" == "310"
$(GPERFFASTTOKENHXX): $(OOXMLGPERFFASTTOKENXSL) $(MODELPROCESSED)
    @echo "Making:   " $(@:f)   
	$(COMMAND_ECHO)$(XSLTPROC) $(OOXMLGPERFFASTTOKENXSL) $(MODELPROCESSED) | tr -d '\r' | $(GPERF) -I -t -E -S1 -c -G -LC++ > $@
.ENDIF		# "$(UPD)" == "310"

$(SLOFACTORIESNAMESPACES): $(OOXMLFACTORYSCXXS) $(OOXMLGENHEADERS)

$(GENERATEDFILES): $(OOXMLFACTORYTOOLSXSL)

$(SLOFILES): $(OOXMLGENHEADERS)

$(SLO)/OOXMLFactory_values.obj: $(OOXMLFACTORYVALUESCXX) $(OOXMLFACTORYVALUESHXX)

$(SLO)$/OOXMLFactory_generated.obj: $(OOXMLFACTORYGENERATEDCXX) $(OOXMLGENHEADERS)

.PHONY: genclean genmake genheaders

genclean:
	rm -f $(GENERATEDFILES)

genmake: $(GENERATEDFILES)

genheaders: $(GENHEADERS)
