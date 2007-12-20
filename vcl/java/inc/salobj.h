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
 *		 - GNU General Public License Version 2.1
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

#ifndef _SV_SALOBJ_H
#define _SV_SALOBJ_H

#ifndef _SV_SALOBJ_HXX
#include <salobj.hxx>
#endif
#ifndef _SV_SV_H
#include <sv.h>
#endif
#ifndef _SV_GEN_HXX
#include <tools/gen.hxx>
#endif
#ifndef _SV_SYSDATA_HXX
#include <sysdata.hxx>
#endif

class JavaSalFrame;
class SalFrame;

// -----------------
// - JavaSalObject -
// -----------------

class JavaSalObject : public SalObject
{
	Rectangle				maBounds;
	JavaSalFrame*			mpParent;
	SystemEnvData			maSysData;
	void*					mpVCLChildFrame;
	BOOL					mbVisible;

public:
							JavaSalObject( SalFrame *pParent );
	virtual					~JavaSalObject();

	virtual void			ResetClipRegion();
	virtual USHORT			GetClipRegionType();
	virtual void			BeginSetClipRegion( ULONG nRects );
	virtual void			UnionClipRegion( long nX, long nY, long nWidth, long nHeight );
	virtual void			EndSetClipRegion();
	virtual void			SetPosSize( long nX, long nY, long nWidth, long nHeight );
	virtual void			Show( BOOL bVisible );
	virtual void			Enable( BOOL nEnable );
	virtual void			GrabFocus();
	virtual void			SetBackground();
	virtual void			SetBackground( SalColor nSalColor );
	virtual const SystemEnvData*	GetSystemData() const;
};

#endif // _SV_SALOBJ_H
