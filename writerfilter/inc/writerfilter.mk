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



CDEFS+=-DWRITERFILTER_DLLIMPLEMENTATION
 
.IF "$(UPD)" == "310"
INCLOCAL += \
	-I$(PRJ)$/source \
	-I$(PRJ)$/..$/comphelper/inc \
	-I$(PRJ)$/..$/i18npool/inc \
	-I$(PRJ)$/..$/offapi$/$(INPATH)$/inc$/cssdrawing \
	-I$(PRJ)$/..$/offapi$/$(INPATH)$/inc$/csstable \
	-I$(PRJ)$/..$/offapi$/$(INPATH)$/inc$/csstext \
	-I$(PRJ)$/..$/offapi$/$(INPATH)$/inc$/cssutil \
	-I$(PRJ)$/..$/offapi$/$(INPATH)$/inc$/cssxmldom \
	-I$(PRJ)$/..$/offapi$/$(INPATH)$/inc$/cssxmlsax \
	-I$(PRJ)$/..$/oox/inc \
	-I$(PRJ)$/..$/oox$/$(INPATH)$/inc \
	-I$(PRJ)$/..$/sal/inc \
	-I$(PRJ)$/..$/sax/inc \
	-I$(PRJ)$/..$/svtools/inc \
	-I$(PRJ)$/..$/svx/inc \
	-I$(PRJ)$/..$/tools/inc \
	-I$(PRJ)$/..$/unotools/inc \
	-I$(PRJ)$/..$/vcl/inc \
	-I$(PRJ)$/..$/xmloff/inc \
	$(LIBXML_CFLAGS)
.ENDIF		# "$(UPD)" == "310"
