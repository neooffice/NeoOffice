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
 *  Copyright 2003 by Patrick Luby (patrick.luby@planamesa.com)
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

#include <stdio.h>

#ifndef _SV_SALOBJ_HXX
#include <salobj.hxx>
#endif

// =======================================================================

long ImplSalObjCallbackDummy( void*, SalObject*, USHORT, const void* )
{
	return 0;
}

// =======================================================================

SalObject::SalObject()
{
}

// -----------------------------------------------------------------------

SalObject::~SalObject()
{
}

// -----------------------------------------------------------------------

void SalObject::ResetClipRegion()
{
#ifdef DEBUG
	fprintf( stderr, "SalObject::ResetClipRegion not implemented\n" );
#endif
}

// -----------------------------------------------------------------------

USHORT SalObject::GetClipRegionType()
{
#ifdef DEBUG
	fprintf( stderr, "SalObject::GetClipRegionType not implemented\n" );
#endif
	return SAL_OBJECT_CLIP_INCLUDERECTS;
}

// -----------------------------------------------------------------------

void SalObject::BeginSetClipRegion( ULONG nRectCount )
{
#ifdef DEBUG
	fprintf( stderr, "SalObject::BeginSetClipRegion not implemented\n" );
#endif
}

// -----------------------------------------------------------------------

void SalObject::UnionClipRegion( long nX, long nY, long nWidth, long nHeight )
{
#ifdef DEBUG
	fprintf( stderr, "SalObject::UnionClipRegion not implemented\n" );
#endif
}

// -----------------------------------------------------------------------

void SalObject::EndSetClipRegion()
{
#ifdef DEBUG
	fprintf( stderr, "SalObject::EndSetClipRegion not implemented\n" );
#endif
}

// -----------------------------------------------------------------------

void SalObject::SetPosSize( long nX, long nY, long nWidth, long nHeight )
{
#ifdef DEBUG
	fprintf( stderr, "SalObject::SetPosSize not implemented\n" );
#endif
}

// -----------------------------------------------------------------------

void SalObject::Show( BOOL bVisible )
{
#ifdef DEBUG
	fprintf( stderr, "SalObject::Show not implemented\n" );
#endif
}

// -----------------------------------------------------------------------

void SalObject::Enable( BOOL bEnable )
{
#ifdef DEBUG
	fprintf( stderr, "SalObject::Enable not implemented\n" );
#endif
}

// -----------------------------------------------------------------------

void SalObject::GrabFocus()
{
#ifdef DEBUG
	fprintf( stderr, "SalObject::GrabFocus not implemented\n" );
#endif
}

// -----------------------------------------------------------------------

void SalObject::SetBackground()
{
#ifdef DEBUG
	fprintf( stderr, "SalObject::SetBackground not implemented\n" );
#endif
}

// -----------------------------------------------------------------------

void SalObject::SetBackground( SalColor nSalColor )
{
#ifdef DEBUG
	fprintf( stderr, "SalObject::SetBackground #2 not implemented\n" );
#endif
}

// -----------------------------------------------------------------------

const SystemEnvData* SalObject::GetSystemData() const
{
#ifdef DEBUG
	fprintf( stderr, "SalObject::GetSystemData not implemented\n" );
#endif
	return NULL;
}

// -----------------------------------------------------------------------

void SalObject::SetCallback( void* pInst, SALOBJECTPROC pProc )
{
#ifdef DEBUG
	fprintf( stderr, "SalObject::SetCallback not implemented\n" );
#endif
}

// =======================================================================

SalObjectData::SalObjectData()
{
}

// -----------------------------------------------------------------------

SalObjectData::~SalObjectData()
{
}
