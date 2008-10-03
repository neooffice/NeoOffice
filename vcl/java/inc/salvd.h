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

#ifndef _SV_SALVD_HXX
#include <vcl/salvd.hxx>
#endif
#ifndef _SV_SV_H
#include <vcl/sv.h>
#endif

namespace vcl
{
class com_sun_star_vcl_VCLImage;
}

class JavaSalGraphics;
class SalGraphics;

// ------------------------
// - JavaSalVirtualDevice -
// ------------------------

class JavaSalVirtualDevice : public SalVirtualDevice
{
public:
	::vcl::com_sun_star_vcl_VCLImage*	mpVCLImage;
	USHORT					mnBitCount;
	JavaSalGraphics*		mpGraphics; 
	BOOL					mbGraphics;

							JavaSalVirtualDevice();
	virtual					~JavaSalVirtualDevice();

	virtual SalGraphics*	GetGraphics();
	virtual void			ReleaseGraphics( SalGraphics* pGraphics );
	virtual BOOL			SetSize( long nNewDX, long nNewDY );
	virtual void                    GetSize( long& rWidth, long& rHeight );
};

#endif // _SV_SALVD_H
