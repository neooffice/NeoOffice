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

#ifndef _SV_SALOBJ_H
#include <salobj.h>
#endif

// =======================================================================

JavaSalObject::JavaSalObject()
{
}

// -----------------------------------------------------------------------

JavaSalObject::~JavaSalObject()
{
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
#ifdef DEBUG
	fprintf( stderr, "JavaSalObject::SetPosSize not implemented\n" );
#endif
}

// -----------------------------------------------------------------------

void JavaSalObject::Show( BOOL bVisible )
{
#ifdef DEBUG
	fprintf( stderr, "JavaSalObject::Show not implemented\n" );
#endif
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
#ifdef DEBUG
	fprintf( stderr, "JavaSalObject::SetBackground not implemented\n" );
#endif
}

// -----------------------------------------------------------------------

void JavaSalObject::SetBackground( SalColor nSalColor )
{
#ifdef DEBUG
	fprintf( stderr, "JavaSalObject::SetBackground #2 not implemented\n" );
#endif
}

// -----------------------------------------------------------------------

const SystemEnvData* JavaSalObject::GetSystemData() const
{
#ifdef DEBUG
	fprintf( stderr, "JavaSalObject::GetSystemData not implemented\n" );
#endif
	return NULL;
}
