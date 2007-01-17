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

#define _SV_SALINFO_CXX

#include <stdio.h>

#ifndef _SV_SALSYS_H
#include <salsys.h>
#endif

// =======================================================================

JavaSalSystem::JavaSalSystem()
{
}

// -----------------------------------------------------------------------

JavaSalSystem::~JavaSalSystem()
{
}

// -----------------------------------------------------------------------

unsigned int JavaSalSystem::GetDisplayScreenCount()
{
#ifdef DEBUG
	fprintf( stderr, "JavaSalSystem::GetDisplayScreenCount not implemented\n" );
#endif
	return 1;
}

// -----------------------------------------------------------------------

bool JavaSalSystem::IsMultiDisplay()
{
#ifdef DEBUG
	fprintf( stderr, "JavaSalSystem::IsMultiDisplay not implemented\n" );
#endif
	return false;
}

// -----------------------------------------------------------------------

unsigned int JavaSalSystem::GetDefaultDisplayNumber()
{
#ifdef DEBUG
	fprintf( stderr, "JavaSalSystem::GetDefaultDisplayNumber not implemented\n" );
#endif
	return 0;
}

// -----------------------------------------------------------------------

Rectangle JavaSalSystem::GetDisplayScreenPosSizePixel( unsigned int nScreen )
{
#ifdef DEBUG
	fprintf( stderr, "JavaSalSystem::GetDisplayScreenPosSizePixel not implemented\n" );
#endif
	return Rectangle();
}

// -----------------------------------------------------------------------

Rectangle JavaSalSystem::GetDisplayWorkAreaPosSizePixel( unsigned int nScreen )
{
#ifdef DEBUG
	fprintf( stderr, "JavaSalSystem::GetDisplayWorkAreaPosSizePixel not implemented\n" );
#endif
	return Rectangle();
}

// -----------------------------------------------------------------------

int JavaSalSystem::ShowNativeMessageBox( const String& rTitle, const String& rMessage, int nButtonCombination, int nDefaultButton )
{
#ifdef DEBUG
	fprintf( stderr, "JavaSalSystem::ShowNativeMessageBox not implemented\n" );
#endif
	return -1;
}
