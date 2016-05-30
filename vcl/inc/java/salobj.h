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

#include <vcl/sv.h>
#include <vcl/sysdata.hxx>

#include "salobj.hxx"

class JavaSalFrame;
class SalFrame;

// -----------------
// - JavaSalObject -
// -----------------

class JavaSalObject : public SalObject
{
	Size					maSize;
	id						mpChildView;
	Rectangle				maClipRect;
	sal_Bool				mbInFlush;
	JavaSalFrame*			mpParent;
	SystemEnvData			maSysData;
	sal_Bool				mbVisible;

public:
							JavaSalObject( SalFrame *pParent );
	virtual					~JavaSalObject();

	void					Destroy();
	void					Flush();

	virtual void			ResetClipRegion();
	virtual sal_uInt16		GetClipRegionType();
	virtual void			BeginSetClipRegion( sal_uLong nRects );
	virtual void			UnionClipRegion( long nX, long nY, long nWidth, long nHeight );
	virtual void			EndSetClipRegion();
	virtual void			SetPosSize( long nX, long nY, long nWidth, long nHeight );
	virtual void			Show( sal_Bool bVisible );
	virtual void			Enable( sal_Bool nEnable );
	virtual void			GrabFocus();
	virtual void			SetBackground();
	virtual void			SetBackground( SalColor nSalColor );
	virtual const SystemEnvData*	GetSystemData() const;
	virtual void			InterceptChildWindowKeyDown( sal_Bool bIntercept );
};

#endif // _SV_SALOBJ_H
