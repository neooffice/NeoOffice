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
	mnAcquireCount = 0;
	mnBitCount = 0;
	mpBits = NULL;
	mpData = NULL;
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

	// Check size and save size
	if ( rSize.Width() > 0 && rSize.Height() > 0 )
	{
		maSize = Size( rSize.Width(), rSize.Height() );
		mpVCLBitmap = new com_sun_star_vcl_VCLBitmap( maSize.Width(), maSize.Height(), nBitCount );
	}

	if ( mpVCLBitmap ) {
		// Cache the bit count
		mnBitCount = mpVCLBitmap->getBitCount();

		// Save the palette
		if ( rPal.GetEntryCount() )
			mpVCLBitmap->setPalette( rPal );

		// Fill the buffer with pointers to the Java buffer
		mpData = mpVCLBitmap->getData();

		return TRUE;
	}
	else
	{
		Destroy();
		return FALSE;
	}

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
		if ( pSrcBuffer )
		{
			BitmapBuffer *pDestBuffer = AcquireBuffer( FALSE );
			if ( pDestBuffer )
			{
				memcpy( pDestBuffer->mpBits, pSrcBuffer->mpBits, sizeof( BYTE ) * pDestBuffer->mnScanlineSize * pDestBuffer->mnHeight );
				ReleaseBuffer( pDestBuffer, FALSE );
			}
			else
			{
				Destroy();
				bRet = FALSE;
			}
			rSalBmp.ReleaseBuffer( pSrcBuffer, TRUE );
		}
		else
		{
			Destroy();
			bRet = FALSE;
		}
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

void SalBitmap::Destroy()
{
	maSize = Size( 0, 0 );
	mnAcquireCount = 0;
	mnBitCount = 0;

	if ( mpData )
	{
		if ( mpBits )
		{
			VCLThreadAttach t;
			if ( t.pEnv )
			{
				if ( mnBitCount <= 8 )
					t.pEnv->ReleaseByteArrayElements( (jbyteArray)mpData->getJavaObject(), (jbyte *)mpBits, JNI_ABORT );
				else if ( mnBitCount <= 16 )
					t.pEnv->ReleaseShortArrayElements( (jshortArray)mpData->getJavaObject(), (jshort *)mpBits, JNI_ABORT );
				else
					rtl_freeMemory( mpBits );
			}
		}
		delete mpData;
	}
	mpBits = NULL;
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

	// Set buffer values. Note that the JVM cannot draw 1 bit bitmaps
	// correctly so we don't use them.
	pBuffer->mnBitCount = mnBitCount;
	pBuffer->mnFormat = BMP_FORMAT_TOP_DOWN;
	if ( mnBitCount <= 4 )
	{
		pBuffer->mnFormat |= BMP_FORMAT_4BIT_MSN_PAL;
	}
	else if ( mnBitCount <= 8 )
	{
		pBuffer->mnFormat |= BMP_FORMAT_8BIT_PAL;
	}
	else if ( mnBitCount <= 16 )
	{
		pBuffer->mnFormat |= BMP_FORMAT_16BIT_TC_MSB_MASK;
		pBuffer->maColorMask = ColorMask( 0x7c00, 0x03e0, 0x001f );
	}
	else
	{
		pBuffer->mnFormat |= BMP_FORMAT_24BIT_TC_RGB;
	}

	pBuffer->mnWidth = maSize.Width();
	pBuffer->mnHeight = maSize.Height();
	pBuffer->mnScanlineSize = AlignedWidth4Bytes( mnBitCount * maSize.Width() );
	mpVCLBitmap->getPalette( pBuffer->maPalette );

	if ( !mpData )
	{
		delete pBuffer;
		return NULL;
	}

	if ( !mpBits )
	{
		VCLThreadAttach t;
		if ( t.pEnv )
		{
			jboolean bCopy( sal_False );
			if ( mnBitCount <= 8 )
			{
				mpBits = (BYTE *)t.pEnv->GetByteArrayElements( (jbyteArray)mpData->getJavaObject(), &bCopy );
			}
			else if ( mnBitCount <= 16 )
			{
				mpBits = (BYTE *)t.pEnv->GetShortArrayElements( (jshortArray)mpData->getJavaObject(), &bCopy );
			}
			else
			{
				jint *pBits = (jint *)t.pEnv->GetPrimitiveArrayCritical( (jarray)mpData->getJavaObject(), &bCopy );
				if ( pBits )
				{
					mpBits = (BYTE *)rtl_allocateMemory( pBuffer->mnScanlineSize * pBuffer->mnHeight );
					jint *pBitsIn = pBits;
					BYTE *pBitsOut = mpBits;
					for ( long i = 0; i < pBuffer->mnHeight; i++ )
					{
						long j;
						long k;
						for ( j = 0, k = 0; j < pBuffer->mnWidth; j++ )
						{
							pBitsOut[ k++ ] = SALCOLOR_RED( pBitsIn[ j ] );
							pBitsOut[ k++ ] = SALCOLOR_GREEN( pBitsIn[ j ] );
							pBitsOut[ k++ ] = SALCOLOR_BLUE( pBitsIn[ j ] );
						}

						pBitsIn += pBuffer->mnWidth;
						pBitsOut += pBuffer->mnScanlineSize;
					}
					t.pEnv->ReleasePrimitiveArrayCritical( (jarray)mpData->getJavaObject(), pBits, JNI_ABORT );
				}
			}
		}
	}

	if ( !mpBits )
	{
		delete pBuffer;
		return NULL;
	}

	mnAcquireCount++;
	pBuffer->mpBits = mpBits;

	return pBuffer;
}

// ------------------------------------------------------------------

void SalBitmap::ReleaseBuffer( BitmapBuffer* pBuffer, BOOL bReadOnly )
{
	if ( pBuffer )
	{
		mnAcquireCount--;

		if ( mpData && mpBits )
		{
			VCLThreadAttach t;
			if ( t.pEnv )
			{
				jint nCommit = JNI_ABORT;
				if ( !mnAcquireCount )
				{
					if ( !bReadOnly )
						nCommit = 0;
				}
				else if ( !bReadOnly )
				{
					nCommit = JNI_COMMIT;
				}

				if ( mnBitCount <= 8 )
				{
					t.pEnv->ReleaseByteArrayElements( (jbyteArray)mpData->getJavaObject(), (jbyte *)mpBits, nCommit );
				}
				else if ( mnBitCount <= 16 )
				{
					t.pEnv->ReleaseShortArrayElements( (jshortArray)mpData->getJavaObject(), (jshort *)mpBits, nCommit );
				}
				else
				{
					if ( !nCommit || nCommit == JNI_COMMIT )
					{
						jboolean bCopy( sal_False );
						jint *pBits = (jint *)t.pEnv->GetPrimitiveArrayCritical( (jarray)mpData->getJavaObject(), &bCopy );
						BYTE *pBitsIn = mpBits;
						jint *pBitsOut = pBits;
						for ( long i = 0; i < pBuffer->mnHeight; i++ )
						{
							long j;
							long k;
							for ( j = 0, k = 0; j < pBuffer->mnWidth; j++, k += 3 )
								pBitsOut[ j ] = MAKE_SALCOLOR( pBitsIn[ k ], pBitsIn[ k + 1 ], pBitsIn[ k + 2 ] ) | 0xff000000;
	
							pBitsIn += pBuffer->mnScanlineSize;
							pBitsOut += pBuffer->mnWidth;
						}
						t.pEnv->ReleasePrimitiveArrayCritical( (jarray)mpData->getJavaObject(), pBits, JNI_ABORT );
					}

					if ( !mnAcquireCount )
						rtl_freeMemory( mpBits );
				}

				if ( !mnAcquireCount )
					mpBits = NULL;

				// Save the palette
				if ( !bReadOnly )
					mpVCLBitmap->setPalette( pBuffer->maPalette );
			}
		}
		delete pBuffer;
	}
}
