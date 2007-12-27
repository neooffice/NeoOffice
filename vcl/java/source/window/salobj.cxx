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
	mpParent = (JavaSalFrame *)pParent;
	mpChildWindow = VCLChildWindow_create();

	memset( &maSysData, 0, sizeof( SystemEnvData ) );
	maSysData.nSize = sizeof( SystemEnvData );
}

// -----------------------------------------------------------------------

JavaSalObject::~JavaSalObject()
{
	VCLChildWindow_release( mpChildWindow );
}

// -----------------------------------------------------------------------

void JavaSalObject::ResetClipRegion()
{
#ifdef DEBUG
	fprintf( stderr, "JavaSalObject::ResetClipRegion not implemented\n" );
#endif
}

// -----------------------------------------------------------------------

USHORT JavaSalObject::GetClipRegionType()
{
#ifdef DEBUG
	fprintf( stderr, "JavaSalObject::GetClipRegionType not implemented\n" );
#endif
	return SAL_OBJECT_CLIP_INCLUDERECTS;
}

// -----------------------------------------------------------------------

void JavaSalObject::BeginSetClipRegion( ULONG nRects )
{
#ifdef DEBUG
	fprintf( stderr, "JavaSalObject::BeginSetClipRegion not implemented\n" );
#endif
}

// -----------------------------------------------------------------------

void JavaSalObject::UnionClipRegion( long nX, long nY, long nWidth, long nHeight )
{
#ifdef DEBUG
	fprintf( stderr, "JavaSalObject::UnionClipRegion not implemented\n" );
#endif
}

// -----------------------------------------------------------------------

void JavaSalObject::EndSetClipRegion()
{
#ifdef DEBUG
	fprintf( stderr, "JavaSalObject::EndSetClipRegion not implemented\n" );
#endif
}

// -----------------------------------------------------------------------

void JavaSalObject::SetPosSize( long nX, long nY, long nWidth, long nHeight )
{
	VCLChildWindow_setBounds( mpChildWindow, nX, nY, nWidth, nHeight );
}

// -----------------------------------------------------------------------

void JavaSalObject::Show( BOOL bVisible )
{
	if ( bVisible )
	{
		void *pParentNSWindow;
		if ( mpParent && mpParent->mpVCLFrame )
			pParentNSWindow = mpParent->mpVCLFrame->getNativeWindow();
		else
			pParentNSWindow = NULL;
		maSysData.aWindow = (long)VCLChildWindow_show( mpChildWindow, pParentNSWindow, bVisible );
	}
	else
	{
		maSysData.aWindow = (long)VCLChildWindow_show( mpChildWindow, NULL, bVisible );
	}
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
}

// -----------------------------------------------------------------------

void JavaSalObject::SetBackground( SalColor nSalColor )
{
}

// -----------------------------------------------------------------------

const SystemEnvData* JavaSalObject::GetSystemData() const
{
	return &maSysData;
}
