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

#ifndef _SV_SALVD_H
#define _SV_SALVD_H

#include <premac.h>
#include <ApplicationServices/ApplicationServices.h>
#include <postmac.h>
#undef check

#include "salvd.hxx"

class JavaSalGraphics;
class SalGraphics;

// ------------------------
// - JavaSalVirtualDevice -
// ------------------------

class JavaSalVirtualDevice : public SalVirtualDevice
{
	long					mnWidth;
	long					mnHeight;
	CGLayerRef				maVirDevLayer;
	JavaSalGraphics*		mpGraphics; 
	sal_Bool				mbGraphics;

public:
							JavaSalVirtualDevice();
	virtual					~JavaSalVirtualDevice();

	bool					ScreenParamsChanged();

	virtual SalGraphics*	AcquireGraphics() SAL_OVERRIDE;
	virtual void			ReleaseGraphics( SalGraphics* pGraphics ) SAL_OVERRIDE;
	virtual bool			SetSize( long nNewDX, long nNewDY ) SAL_OVERRIDE;
	virtual long			GetWidth() const SAL_OVERRIDE;
	virtual long			GetHeight() const SAL_OVERRIDE;
};

#endif // _SV_SALVD_H
