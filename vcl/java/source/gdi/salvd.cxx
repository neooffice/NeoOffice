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

#define _SV_SALVD_CXX

#ifndef _SV_SALVD_H
#include <salvd.h>
#endif
#ifndef _SV_SALGDI_H
#include <salgdi.h>
#endif
#ifndef _SV_COM_SUN_STAR_VCL_VCLGRAPHICS_HXX
#include <com/sun/star/vcl/VCLGraphics.hxx>
#endif
#ifndef _SV_COM_SUN_STAR_VCL_VCLIMAGE_HXX
#include <com/sun/star/vcl/VCLImage.hxx>
#endif

using namespace vcl;

// =======================================================================

JavaSalVirtualDevice::JavaSalVirtualDevice()
{
	mpVCLImage = NULL;
	mnBitCount = 0;
	mpGraphics = new JavaSalGraphics();
	mbGraphics = FALSE;

	// By default no mirroring for VirtualDevices
	mpGraphics->SetLayout( 0 );
	mpGraphics->mpVirDev = this;
}

// -----------------------------------------------------------------------

JavaSalVirtualDevice::~JavaSalVirtualDevice()
{
	if ( mpGraphics )
		delete mpGraphics;

	if ( mpVCLImage )
	{
		mpVCLImage->dispose();
		delete mpVCLImage;
	}
}

// -----------------------------------------------------------------------

SalGraphics* JavaSalVirtualDevice::GetGraphics()
{
	if ( mbGraphics )
		return NULL;

	if ( !mpGraphics->mpVCLGraphics )
		mpGraphics->mpVCLGraphics = mpVCLImage->getGraphics();
	mbGraphics = TRUE;

	return mpGraphics;
}

// -----------------------------------------------------------------------

void JavaSalVirtualDevice::ReleaseGraphics( SalGraphics* pGraphics )
{
	if ( pGraphics != mpGraphics )
		return;

	mbGraphics = FALSE;
}

// -----------------------------------------------------------------------

BOOL JavaSalVirtualDevice::SetSize( long nDX, long nDY )
{
	BOOL bRet = FALSE;

	if ( mpGraphics->mpVCLGraphics )
	{
		delete mpGraphics->mpVCLGraphics;
		mpGraphics->mpVCLGraphics = NULL;
	}

	if ( mpVCLImage )
	{
		mpVCLImage->dispose();
		delete mpVCLImage;
		mpVCLImage = NULL;
	}

	if ( nDX > 0 && nDY > 0 )
	{
		com_sun_star_vcl_VCLImage *pVCLImage = new com_sun_star_vcl_VCLImage( nDX, nDY, mnBitCount );
		if ( pVCLImage )
		{
			if ( pVCLImage->getJavaObject() )
			{
				mpVCLImage = pVCLImage;
				mpGraphics->mpVCLGraphics = mpVCLImage->getGraphics();
				bRet = TRUE;
			}
			else
			{
				delete pVCLImage;
			}
		}
	}

	if ( !mpVCLImage )
	{
		// Try to create something so that we don't crash
		mpVCLImage = new com_sun_star_vcl_VCLImage( 1, 1, mnBitCount );
		if ( mpVCLImage && mpVCLImage->getJavaObject() )
			mpGraphics->mpVCLGraphics = mpVCLImage->getGraphics();
	}

	return bRet;
}

// -----------------------------------------------------------------------

void JavaSalVirtualDevice::GetSize( long& rWidth, long& rHeight )
{
	if ( mpVCLImage )
	{
		rWidth = mpVCLImage->getWidth();
		rHeight = mpVCLImage->getHeight();
	}
	else
	{
		rWidth = 0;
		rHeight = 0;
	}
}
