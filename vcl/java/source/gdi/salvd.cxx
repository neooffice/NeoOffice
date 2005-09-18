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

#ifndef _SV_SALVD_HXX
#include <salvd.hxx>
#endif
#ifndef _SV_COM_SUN_STAR_VCL_VCLGRAPHICS_HXX
#include <com/sun/star/vcl/VCLGraphics.hxx>
#endif

using namespace vcl;

// =======================================================================

SalVirtualDevice::SalVirtualDevice()
{
	maVirDevData.mpGraphics->maGraphicsData.mpVirDev = this;
}

// -----------------------------------------------------------------------

SalVirtualDevice::~SalVirtualDevice()
{
}

// -----------------------------------------------------------------------

SalGraphics* SalVirtualDevice::GetGraphics()
{
	if ( maVirDevData.mbGraphics )
		return NULL;

	maVirDevData.mpGraphics->maGraphicsData.mpVCLGraphics = maVirDevData.mpVCLImage->getGraphics();
	maVirDevData.mbGraphics = TRUE;

	return maVirDevData.mpGraphics;
}

// -----------------------------------------------------------------------

void SalVirtualDevice::ReleaseGraphics( SalGraphics* pGraphics )
{
	if ( pGraphics != maVirDevData.mpGraphics )
		return;

	if ( maVirDevData.mpGraphics->maGraphicsData.mpVCLGraphics )
		delete maVirDevData.mpGraphics->maGraphicsData.mpVCLGraphics;
	maVirDevData.mpGraphics->maGraphicsData.mpVCLGraphics = NULL;
	maVirDevData.mbGraphics = FALSE;
}

// -----------------------------------------------------------------------

BOOL SalVirtualDevice::SetSize( long nDX, long nDY )
{
	BOOL bRet = FALSE;

	if ( nDX > 0 && nDY > 0 )
	{
		com_sun_star_vcl_VCLImage *pVCLImage = new com_sun_star_vcl_VCLImage( nDX, nDY, maVirDevData.mnBitCount );
		if ( pVCLImage && pVCLImage->getJavaObject() )
		{
			if ( maVirDevData.mpVCLImage )
			{
				maVirDevData.mpVCLImage->dispose();
				delete maVirDevData.mpVCLImage;
			}

			maVirDevData.mpVCLImage = pVCLImage;

			if ( maVirDevData.mpGraphics->maGraphicsData.mpVCLGraphics )
				delete maVirDevData.mpGraphics->maGraphicsData.mpVCLGraphics;
			maVirDevData.mpGraphics->maGraphicsData.mpVCLGraphics = maVirDevData.mpVCLImage->getGraphics();

			bRet = TRUE;
		}
		else
		{
			if ( pVCLImage )
				delete pVCLImage;
		}
	}

	return bRet;
}

// =======================================================================

SalVirDevData::SalVirDevData()
{
	mpVCLImage = NULL;
	mnBitCount = 0;
	mpGraphics = new SalGraphicsLayout();
	mbGraphics = FALSE;

	// By default no mirroring for VirtualDevices
	mpGraphics->SetLayout( 0 );

}

// -----------------------------------------------------------------------

SalVirDevData::~SalVirDevData()
{
	if ( mpGraphics )
		delete mpGraphics;

	if ( mpVCLImage )
	{
		mpVCLImage->dispose();
		delete mpVCLImage;
	}
}
