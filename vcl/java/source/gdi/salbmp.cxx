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

#define _SV_SALBMP_CXX

#ifndef _SV_SALBMP_HXX
#include <salbmp.hxx>
#endif
#ifndef _SV_SALGDI_HXX
#include <salgdi.hxx>
#endif
#ifndef _SV_COM_SUN_STAR_VCL_VCLGRAPHICS_HXX
#include <com/sun/star/vcl/VCLGraphics.hxx>
#endif

using namespace vcl;

// ==================================================================

SalBitmap::SalBitmap() :
	maSize( 0, 0 )
{
	mnBitCount = 0;
	mpData = NULL;
	mpBits = NULL;
	mpVCLBitmap = NULL;
}

// ------------------------------------------------------------------

SalBitmap::~SalBitmap()
{
	Destroy();
}

// ------------------------------------------------------------------

BOOL SalBitmap::Create( const Size& rSize, USHORT nBitCount, const BitmapPalette& rPal )
{
	Destroy();

	// Check and save size
	Size aSize( rSize.Width(), rSize.Height() );
	if ( aSize.Width() <= 0 )
		aSize.setWidth( 1 );
	if ( aSize.Height() <= 0 )
		aSize.setHeight( 1 );
	maSize = aSize;

	// Adjust bit count
	if ( nBitCount <= 1)
		nBitCount = 1;
	else if ( nBitCount <= 4)
		nBitCount = 4;
	else if ( nBitCount <= 8)
		nBitCount = 8;
	else
		nBitCount = 24;

	mpVCLBitmap = new com_sun_star_vcl_VCLBitmap( maSize.Width(), maSize.Height(), nBitCount );

	if ( mpVCLBitmap ) {
		// Cache the bit count
		mnBitCount = mpVCLBitmap->getBitCount();

		// Save the palette
		if ( rPal.GetEntryCount() )
			mpVCLBitmap->setPalette( rPal );

		// Fill the buffer with pointers to the Java buffer
		mpData = mpVCLBitmap->getData();
		VCLThreadAttach t;
		if ( t.pEnv )
		{
			jboolean bCopy( sal_False );
			mpBits = (BYTE *)t.pEnv->GetByteArrayElements( (jbyteArray)mpData->getJavaObject(), &bCopy );
		}

	}

	if ( !mpVCLBitmap || !mpData || !mpBits )
	{
		Destroy();
		return FALSE;
	}

	return TRUE;
}

// ------------------------------------------------------------------

BOOL SalBitmap::Create( const SalBitmap& rSalBmp )
{
	Destroy();

	BitmapPalette aPalette;
	if ( rSalBmp.mpVCLBitmap )
		rSalBmp.mpVCLBitmap->getPalette( aPalette );

	BOOL bRet = Create( rSalBmp.GetSize(), rSalBmp.GetBitCount(), aPalette );

	if ( bRet )
	{
		BitmapBuffer *pSrcBuffer = rSalBmp.AcquireBuffer( TRUE );
		BitmapBuffer *pDestBuffer = AcquireBuffer( FALSE );
		memcpy( pDestBuffer->mpBits, pSrcBuffer->mpBits, sizeof( BYTE ) * pDestBuffer->mnScanlineSize * pDestBuffer->mnHeight );
		ReleaseBuffer( pDestBuffer, FALSE );
		rSalBmp.ReleaseBuffer( pSrcBuffer, TRUE );
	}

	return bRet;
}

// ------------------------------------------------------------------

BOOL SalBitmap::Create( const SalBitmap& rSalBmp, SalGraphics* pGraphics )
{
	return FALSE;
}

// ------------------------------------------------------------------

BOOL SalBitmap::Create( const SalBitmap& rSalBmp, USHORT nNewBitCount )
{
	FALSE;
}

// ------------------------------------------------------------------

BOOL SalBitmap::Create( const SalBitmap& rSalBmp, const SalTwoRect& rPosAry )
{
	Destroy();

	BitmapPalette aPalette;
	if ( rSalBmp.mpVCLBitmap )
		rSalBmp.mpVCLBitmap->getPalette( aPalette );

	BitmapBuffer *pSrcBuffer = rSalBmp.AcquireBuffer( TRUE );
	BitmapBuffer *pScaledBuffer = StretchAndConvert( *pSrcBuffer, rPosAry, pSrcBuffer->mnFormat, &aPalette );
	BOOL bRet = Create( Size( pScaledBuffer->mnWidth, pScaledBuffer->mnHeight ), pScaledBuffer->mnBitCount, pScaledBuffer->maPalette );

	if ( bRet )
	{
		BitmapBuffer *pDestBuffer = AcquireBuffer( FALSE );
		memcpy( pDestBuffer->mpBits, pScaledBuffer->mpBits, sizeof( BYTE ) * pScaledBuffer->mnScanlineSize * pScaledBuffer->mnHeight );
		ReleaseBuffer( pDestBuffer, FALSE );
	}

	delete pScaledBuffer;
	rSalBmp.ReleaseBuffer( pSrcBuffer, TRUE );

	return bRet;
}

// ------------------------------------------------------------------

void SalBitmap::Destroy()
{
	maSize = Size( 0, 0 );

	mnBitCount = 0;

	if ( mpData )
	{
		if ( mpBits )
		{
			VCLThreadAttach t;
			if ( t.pEnv )
				t.pEnv->ReleaseByteArrayElements( (jbyteArray)mpData->getJavaObject(), (jbyte *)mpBits, JNI_ABORT );
		}
		mpBits = NULL;
		delete mpData;
	}
	mpData = NULL;

	if ( mpVCLBitmap )
		delete mpVCLBitmap;
	mpVCLBitmap = NULL;
}

// ------------------------------------------------------------------

USHORT SalBitmap::GetBitCount() const
{
	return mnBitCount;
}

// ------------------------------------------------------------------

Size SalBitmap::GetSize() const
{
	return maSize;
}

// ------------------------------------------------------------------

BitmapBuffer* SalBitmap::AcquireBuffer( BOOL bReadOnly )
{
	BitmapBuffer *pBuffer = new BitmapBuffer();

	// Set buffer values
	pBuffer->mnBitCount = mnBitCount;
	pBuffer->mnFormat = BMP_FORMAT_TOP_DOWN;
	if ( mnBitCount <= 8 )
		pBuffer->mnFormat |= BMP_FORMAT_8BIT_PAL;
	else
		pBuffer->mnFormat |= BMP_FORMAT_24BIT_TC_RGB;
	pBuffer->mnWidth = maSize.Width();
	pBuffer->mnHeight = maSize.Height();
	pBuffer->mnScanlineSize = AlignedWidth4Bytes( mnBitCount * maSize.Width() );
	mpVCLBitmap->getPalette( pBuffer->maPalette );
	pBuffer->mpBits = mpBits;

	return pBuffer;
}

// ------------------------------------------------------------------

void SalBitmap::ReleaseBuffer( BitmapBuffer* pBuffer, BOOL bReadOnly )
{
	if ( pBuffer )
	{
		if ( mpData && mpBits )
		{
			VCLThreadAttach t;
			if ( t.pEnv )
			{
				jbyteArray pArray = (jbyteArray)mpData->getJavaObject();
				if ( !bReadOnly )
				{
					t.pEnv->ReleaseByteArrayElements( pArray, (jbyte *)mpBits, JNI_COMMIT );
					// Save the palette
					mpVCLBitmap->setPalette( pBuffer->maPalette );
				}
			}
		}
		delete pBuffer;
	}
}
