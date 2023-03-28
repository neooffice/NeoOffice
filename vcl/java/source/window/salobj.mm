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

#include "java/salframe.h"
#include "java/salgdi.h"

#include "salobj_cocoa.h"

// =======================================================================

JavaSalObject::JavaSalObject( SalFrame *pParent ) :
	mbInFlush( sal_False ),
	mpParent( (JavaSalFrame *)pParent ),
	mbVisible( false )

{
	mpChildView = VCLChildView_create();

	memset( &maSysData, 0, sizeof( SystemEnvData ) );
	maSysData.nSize = sizeof( SystemEnvData );

	// Set window value now as the avmedia module needs access to it before
	// it is actually shown
	maSysData.mpNSView = (NSView *)mpChildView;

	if ( mpParent )
		mpParent->AddObject( this, false );
}

// -----------------------------------------------------------------------

JavaSalObject::~JavaSalObject()
{
	Destroy();
}

// -----------------------------------------------------------------------

void JavaSalObject::Destroy()
{
	Show( false );

	if ( mpParent )
	{
		mpParent->RemoveObject( this, true );
		mpParent = NULL;
	}

	VCLChildView_release( mpChildView );
	mpChildView = NULL;
}

// -----------------------------------------------------------------------

void JavaSalObject::Flush()
{
	if ( mbVisible )
	{
		mbInFlush = sal_True;
		Show( true );
		mbInFlush = sal_False;
	}
}

// -----------------------------------------------------------------------

void JavaSalObject::ResetClipRegion()
{
	maClipRect = Rectangle();
	VCLChildView_setClip( mpChildView, NSZeroRect );
}

// -----------------------------------------------------------------------

sal_uInt16 JavaSalObject::GetClipRegionType()
{
	return SAL_OBJECT_CLIP_INCLUDERECTS;
}

// -----------------------------------------------------------------------

void JavaSalObject::BeginSetClipRegion( sal_uLong /* nRects */ )
{
	maClipRect = Rectangle();
}

// -----------------------------------------------------------------------

void JavaSalObject::UnionClipRegion( long nX, long nY, long nWidth, long nHeight )
{
	Rectangle aRect( Point( nX, nY ), Size( nWidth, nHeight ) );
	if ( !aRect.IsEmpty() )
	{
		if ( maClipRect.IsEmpty() )
			maClipRect = aRect;
		else
			maClipRect = aRect.Union( maClipRect );
	}
}

// -----------------------------------------------------------------------

void JavaSalObject::EndSetClipRegion()
{
	if ( !maClipRect.IsEmpty() )
	{
		VCLChildView_setClip( mpChildView, NSMakeRect( maClipRect.Left(), maSize.Height() - maClipRect.GetHeight() - maClipRect.Top(), maClipRect.GetWidth(), maClipRect.GetHeight() ) );
	}
	else
	{
		VCLChildView_setClip( mpChildView, NSZeroRect );
	}
}

// -----------------------------------------------------------------------

void JavaSalObject::SetPosSize( long nX, long nY, long nWidth, long nHeight )
{
	if ( mpParent && mpParent->mpGraphics )
	{
		CGRect aUnflippedRect = UnflipFlippedRect( CGRectMake( nX, nY, nWidth, nHeight ), mpParent->mpGraphics->maNativeBounds );
		maSize = Size( aUnflippedRect.size.width, aUnflippedRect.size.height );
		VCLChildView_setBounds( mpChildView, NSRectFromCGRect( aUnflippedRect ) );
	}
}

// -----------------------------------------------------------------------

void JavaSalObject::Show( bool bVisible )
{
	mbVisible = bVisible;

	void *pParentNSWindow;
	if ( mbVisible && mbInFlush && mpParent )
		pParentNSWindow = mpParent->GetNativeWindow();
	else
		pParentNSWindow = NULL;

	if ( mpParent )
		mpParent->RemoveObject( this, false );

	// Don't attach subview unless we are in the Flush() method
	VCLChildView_show( mpChildView, (id)pParentNSWindow, mbVisible && pParentNSWindow ? sal_True : sal_False );

	if ( mpParent )
		mpParent->AddObject( this, mbVisible );
}

// -----------------------------------------------------------------------

void JavaSalObject::SetBackground()
{
	VCLChildView_setBackgroundColor( mpChildView, 0xffffffff );
}

// -----------------------------------------------------------------------

void JavaSalObject::SetBackground( SalColor nSalColor )
{
	VCLChildView_setBackgroundColor( mpChildView, nSalColor | 0xff000000 );
}

// -----------------------------------------------------------------------

const SystemEnvData* JavaSalObject::GetSystemData() const
{
	return &maSysData;
}
