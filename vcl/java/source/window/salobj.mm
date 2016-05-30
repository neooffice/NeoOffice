/*************************************************************************
 *
 *  $RCSfile$
 *
 *  $Revision$
 *
 *  last change: $Author$ $Date$
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU General Public License Version 2.1
 *
 *  Patrick Luby, June 2003
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2003 Planamesa Inc.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 ************************************************************************/

#include "java/salframe.h"
#include "java/salgdi.h"

#include "salobj_cocoa.h"

// =======================================================================

JavaSalObject::JavaSalObject( SalFrame *pParent )
{
	mpChildView = VCLChildView_create();
	mbInFlush = sal_False;
	mpParent = (JavaSalFrame *)pParent;
	mbVisible = sal_False;

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
	Show( sal_False );

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
		Show( sal_True );
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
		VCLChildView_setClip( mpChildView, NSMakeRect( maClipRect.nLeft, maSize.Height() - maClipRect.GetHeight() - maClipRect.nTop, maClipRect.GetWidth(), maClipRect.GetHeight() ) );
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

void JavaSalObject::Show( sal_Bool bVisible )
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

void JavaSalObject::Enable( sal_Bool /* bEnable */ )
{
#ifdef DEBUG
	fprintf( stderr, "JavaSalObject::Enable not implemented\n" );
#endif
}

// -----------------------------------------------------------------------

void JavaSalObject::GrabFocus()
{
#ifdef DEBUG
	fprintf( stderr, "JavaSalObject::GrabFocus not implemented\n" );
#endif
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

// -----------------------------------------------------------------------

void JavaSalObject::InterceptChildWindowKeyDown( sal_Bool /* bIntercept */ )
{
#ifdef DEBUG
	fprintf( stderr, "JavaSalObject::InterceptChildWindowKeyDown not implemented\n" );
#endif
}
