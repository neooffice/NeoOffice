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

#define _SV_SALSOUND_CXX

#ifndef _SV_SALDATA_HXX
#include <saldata.hxx>
#endif
#ifndef _SV_SALINST_HXX
#include <salinst.hxx>
#endif
#ifndef _SV_SALSOUND_HXX
#include <salsound.hxx>
#endif

// ========================================================================

SalSound::SalSound()
{
}

// ------------------------------------------------------------------------

SalSound::~SalSound()
{
}

// ------------------------------------------------------------------------

BOOL SalSound::Create()
{
#ifdef DEBUG
	fprintf( stderr, "SalSound::Create not implemented\n" );
#endif
	return FALSE;
}

// ------------------------------------------------------------------------

void SalSound::Release()
{
#ifdef DEBUG
	fprintf( stderr, "SalSound::Release not implemented\n" );
#endif
}

// ------------------------------------------------------------------------

BOOL SalSound::IsValid()
{
#ifdef DEBUG
	fprintf( stderr, "SalSound::IsValid not implemented\n" );
#endif
	return FALSE;
}

// ------------------------------------------------------------------------

BOOL SalSound::Init( SalFrame* pFrame, const XubString& rSoundName, ULONG& rSoundLen )
{
#ifdef DEBUG
	fprintf( stderr, "SalSound::Init not implemented\n" );
#endif
	return FALSE;
}

// ------------------------------------------------------------------------

BOOL SalSound::Init( SalFrame* pFrame, const BYTE* pSound, ULONG nDataLen, ULONG& rSoundLen )
{
#ifdef DEBUG
	fprintf( stderr, "SalSound::Init #2 not implemented\n" );
#endif
	return FALSE;
}

// ------------------------------------------------------------------------

void SalSound::Play( ULONG nStartTime, ULONG nPlayLen, BOOL bLoop )
{
#ifdef DEBUG
	fprintf( stderr, "SalSound::Play not implemented\n" );
#endif
}

// ------------------------------------------------------------------------

void SalSound::Stop()
{
#ifdef DEBUG
	fprintf( stderr, "SalSound::Stop not implemented\n" );
#endif
}

// ------------------------------------------------------------------------

void SalSound::Pause()
{
#ifdef DEBUG
	fprintf( stderr, "SalSound::Pause not implemented\n" );
#endif
}

// ------------------------------------------------------------------------

void SalSound::SetNotifyProc( void* pInst, SALSOUNDPROC pProc )
{
#ifdef DEBUG
	fprintf( stderr, "SalSound::SetNotifyProc not implemented\n" );
#endif
}
