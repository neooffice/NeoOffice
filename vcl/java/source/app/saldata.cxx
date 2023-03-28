/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "java/saldata.hxx"
#include "java/salgdi.h"
#include "java/salinst.h"

using namespace vcl;

// ========================================================================

SalData::SalData() :
	mpFirstInstance( NULL ),
	mpFocusFrame( NULL ),
	mnTimerInterval( 0 ),
	mpPresentationFrame( NULL ),
	mbInNativeModalSheet( false ),
	mpNativeModalSheetFrame( NULL ),
	mpLastDragFrame( NULL ),
	mbInSignalHandler( false ),
	mbDoubleScrollbarArrows( false ),
	mpCaptureFrame( NULL ),
	mpLastResizeFrame( NULL ),
	mpLastMouseMoveFrame( NULL )
{
	maTimeout.tv_sec = 0;
	maTimeout.tv_usec = 0;
	maLastResizeTime.tv_sec = 0;
	maLastResizeTime.tv_usec = 0;

	// Set condition so that they don't block
	maNativeEventCondition.set();
	maLastPointerState.mnState = 0;
	maLastPointerState.maPos = Point( 0, 0 );
}

// ------------------------------------------------------------------------

SalData::~SalData()
{
	for ( ::std::map< OUString, JavaPhysicalFontFace* >::const_iterator it = maFontNameMapping.begin(); it != maFontNameMapping.end(); ++it )
		delete it->second;

	while ( maPendingDocumentEventsList.size() )
	{
		maPendingDocumentEventsList.front()->release();
		maPendingDocumentEventsList.pop_front();
	}
}
