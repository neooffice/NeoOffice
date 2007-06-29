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
 *  Patrick Luby, February 2006
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2006 Planamesa Inc.
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

#ifndef _SV_SALSOUND_H
#define _SV_SALSOUND_H

#ifndef _SV_SALSOUND_HXX
#include <salsound.hxx>
#endif 
#ifndef _SV_SV_H
#include <sv.h>
#endif 
#ifndef _SV_SALSTYPE_HXX
#include <salstype.hxx>
#endif

class SalSoundNativeData;

// ----------------
// - JavaSalSound -
// ----------------

class JavaSalSound : public SalSound
{
private:
	static ULONG			mnSoundState;

	void*					mpInst;
	void*					mpNativeContext;
	SalSoundNativeData*		mpNativeData;
	bool					mbPlaying;
	SALSOUNDPROC			mpProc;

public:
							JavaSalSound();
	virtual					~JavaSalSound();

	virtual bool			IsValid();
	virtual bool			Init( const String& rSoundName, ULONG& rSoundLen );
	virtual void			Play( ULONG nStartTime, ULONG nPlayTime, bool bLoop );
	virtual void			Stop();
	virtual void			Pause();
	virtual void			Continue();
	virtual bool			IsLoopMode() const;
	virtual bool			IsPlaying() const;
	virtual bool			IsPaused() const;
};

#endif // _SV_SALSOUND_H
