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

#define _SV_SALOBJ_CXX

#ifndef _SV_SALOBJ_H
#include <salobj.h>
#endif
#ifndef _SV_SALFRAME_H
#include <salframe.h>
#endif
#ifndef _SV_COM_SUN_STAR_VCL_VCLFRAME_HXX
#include <com/sun/star/vcl/VCLFrame.hxx>
#endif

#include "salobj_cocoa.h"

// =======================================================================

JavaSalObject::JavaSalObject( SalFrame *pParent )
{
	mpChildView = VCLChildView_create();
	mpParent = (JavaSalFrame *)pParent;

	memset( &maSysData, 0, sizeof( SystemEnvData ) );
	maSysData.nSize = sizeof( SystemEnvData );

	// Set window value now as the avmedia module needs access to it before
	// it is actually shown
	maSysData.aWindow = (long)mpChildView;
}

// -----------------------------------------------------------------------

JavaSalObject::~JavaSalObject()
{
	Destroy();
}

// -----------------------------------------------------------------------

void JavaSalObject::Destroy()
{
	if ( mpParent )
	{
		mpParent->RemoveObject( this );
		mpParent = NULL;
	}

	VCLChildView_release( mpChildView );
	mpChildView = NULL;
}

// -----------------------------------------------------------------------

void JavaSalObject::ResetClipRegion()
{
	maClipRect = Rectangle();
	VCLChildView_setClip( mpChildView, maClipRect.nLeft, maClipRect.nTop, maClipRect.GetWidth(), maClipRect.GetHeight() );
}

// -----------------------------------------------------------------------

USHORT JavaSalObject::GetClipRegionType()
{
	return SAL_OBJECT_CLIP_INCLUDERECTS;
}

// -----------------------------------------------------------------------

void JavaSalObject::BeginSetClipRegion( ULONG nRects )
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
	VCLChildView_setClip( mpChildView, maClipRect.nLeft, maClipRect.nTop, maClipRect.GetWidth(), maClipRect.GetHeight() );
}

// -----------------------------------------------------------------------

void JavaSalObject::SetPosSize( long nX, long nY, long nWidth, long nHeight )
{
	VCLChildView_setBounds( mpChildView, nX, nY, nWidth, nHeight );
}

// -----------------------------------------------------------------------

void JavaSalObject::Show( BOOL bVisible )
{
	void *pParentNSWindow;
	if ( bVisible && mpParent && mpParent->mpVCLFrame )
		pParentNSWindow = mpParent->mpVCLFrame->getNativeWindow();
	else
		pParentNSWindow = NULL;

	if ( mpParent )
		mpParent->RemoveObject( this );

	VCLChildView_show( mpChildView, pParentNSWindow, bVisible );

	if ( bVisible && mpParent )
		mpParent->AddObject( this );
}

// -----------------------------------------------------------------------

void JavaSalObject::Enable( BOOL bEnable )
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
	VCLChildView_setBackgroundColor( mpChildView, 0x00000000 );
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
