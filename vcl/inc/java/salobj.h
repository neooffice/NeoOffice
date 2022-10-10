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
	bool					mbVisible;

public:
							JavaSalObject( SalFrame *pParent );
	virtual					~JavaSalObject();

	void					Destroy();
	void					Flush();

	virtual void			ResetClipRegion() SAL_OVERRIDE;
	virtual sal_uInt16		GetClipRegionType() SAL_OVERRIDE;
	virtual void			BeginSetClipRegion( sal_uLong nRects ) SAL_OVERRIDE;
	virtual void			UnionClipRegion( long nX, long nY, long nWidth, long nHeight ) SAL_OVERRIDE;
	virtual void			EndSetClipRegion() SAL_OVERRIDE;
	virtual void			SetPosSize( long nX, long nY, long nWidth, long nHeight ) SAL_OVERRIDE;
	virtual void			Show( bool bVisible ) SAL_OVERRIDE;
	virtual void			SetBackground() SAL_OVERRIDE;
	virtual void			SetBackground( SalColor nSalColor ) SAL_OVERRIDE;
	virtual const SystemEnvData*	GetSystemData() const SAL_OVERRIDE;
};

#endif // _SV_SALOBJ_H
