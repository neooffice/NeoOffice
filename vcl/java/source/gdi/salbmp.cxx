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
	maSize( 0, 0 ),
	mnAcquireCount( 0 ),
	mnBitCount( 0 ),
	mpBits( NULL ),
	mbCopyFromVCLBitmap( false ),
	mpData( NULL ),
	mpVCLBitmap( NULL )
{
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

	if ( mpVCLBitmap && mpVCLBitmap->getJavaObject() )
	{
		// Cache the bit count
		mnBitCount = mpVCLBitmap->getBitCount();

		// Save the palette
		USHORT nColors = ( ( mnBitCount <= 8 ) ? ( 1 << mnBitCount ) : 0 );
		if ( nColors )
		{
			maPalette = rPal;
			maPalette.SetEntryCount( nColors );
		}

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

	BOOL bRet = Create( rSalBmp.GetSize(), rSalBmp.GetBitCount(), rSalBmp.maPalette );

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
	mbCopyFromVCLBitmap = false;

	if ( mpBits )
	{
		rtl_freeMemory( mpBits );
		mpBits = NULL;
	}

	if ( mpData )
	{
		delete mpData;
		mpData = NULL;
	}

	maPalette.SetEntryCount( 0 );

	if ( mpVCLBitmap )
	{
		delete mpVCLBitmap;
		mpVCLBitmap = NULL;
	}
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
	if ( !mpVCLBitmap || !mpData )
		return NULL;

	BitmapBuffer *pBuffer = new BitmapBuffer();

	pBuffer->mnBitCount = mnBitCount;
	pBuffer->mnFormat = BMP_FORMAT_TOP_DOWN;
	if ( mnBitCount <= 1 )
	{
		pBuffer->mnFormat |= BMP_FORMAT_1BIT_MSB_PAL;
	}
	else if ( mnBitCount <= 4 )
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
	pBuffer->maPalette = maPalette;

	if ( !mpBits )
	{
		mpBits = (BYTE *)rtl_allocateMemory( pBuffer->mnScanlineSize * pBuffer->mnHeight );
		if ( mpBits )
		{
			if ( mbCopyFromVCLBitmap )
			{
				mbCopyFromVCLBitmap = false;

				VCLThreadAttach t;
				if ( t.pEnv )
				{
					jboolean bCopy( sal_False );
					jint *pBits = (jint *)t.pEnv->GetPrimitiveArrayCritical( (jintArray)mpData->getJavaObject(), &bCopy );
					if ( pBits )
					{
						jint *pBitsIn = pBits;
						BYTE *pBitsOut = mpBits;
	
						if ( pBuffer->mnFormat & BMP_FORMAT_1BIT_MSB_PAL )
						{
							for ( long i = 0; i < pBuffer->mnHeight; i++ )
							{
								for ( long j = 0; j < pBuffer->mnWidth; j++ )
								{
									BYTE& rByte = pBitsOut[ j >> 3 ];
									USHORT nIndex = maPalette.GetBestIndex( BitmapColor( (BYTE)( pBitsIn[ j ] >> 16 ), (BYTE)( pBitsIn[ j ] >> 8 ), (BYTE)pBitsIn[ j ] ) );
									( nIndex & 1 ) ? ( rByte |= 1 << ( 7 - ( j & 7 ) ) ) : ( rByte &= ~( 1 << ( 7 - ( j & 7 ) ) ) );
								}

								pBitsIn += pBuffer->mnWidth;
								pBitsOut += pBuffer->mnScanlineSize;
							}
						}
						else if ( pBuffer->mnFormat & BMP_FORMAT_4BIT_MSN_PAL )
						{
							for ( long i = 0; i < pBuffer->mnHeight; i++ )
							{
								for ( long j = 0; j < pBuffer->mnWidth; j++ )
								{
									BYTE& rByte = pBitsOut[ j >> 1 ];
									USHORT nIndex = maPalette.GetBestIndex( BitmapColor( (BYTE)( pBitsIn[ j ] >> 16 ), (BYTE)( pBitsIn[ j ] >> 8 ), (BYTE)pBitsIn[ j ] ) );
									( j & 1 ) ? ( rByte &= 0xf0, rByte |= ( nIndex & 0x0f ) ) : ( rByte &= 0x0f, rByte |= ( nIndex << 4 ) );
								}

								pBitsIn += pBuffer->mnWidth;
								pBitsOut += pBuffer->mnScanlineSize;
							}
						}
						else if ( pBuffer->mnFormat & BMP_FORMAT_8BIT_PAL )
						{
							for ( long i = 0; i < pBuffer->mnHeight; i++ )
							{
								for ( long j = 0; j < pBuffer->mnWidth; j++ )
									pBitsOut[ j ] = (BYTE)maPalette.GetBestIndex( BitmapColor( (BYTE)( pBitsIn[ j ] >> 16 ), (BYTE)( pBitsIn[ j ] >> 8 ), (BYTE)pBitsIn[ j ] ) );

								pBitsIn += pBuffer->mnWidth;
								pBitsOut += pBuffer->mnScanlineSize;
							}
						}
						else if ( pBuffer->mnFormat & BMP_FORMAT_16BIT_TC_MSB_MASK )
						{
							for ( long i = 0; i < pBuffer->mnHeight; i++ )
							{
								for ( long j = 0; j < pBuffer->mnWidth; j++ )
									pBuffer->maColorMask.SetColorFor16BitMSB( BitmapColor( (BYTE)( pBitsIn[ j ] >> 16 ), (BYTE)( pBitsIn[ j ] >> 8 ), (BYTE)pBitsIn[ j ] ), pBitsOut + ( j << 1UL ) );

								pBitsIn += pBuffer->mnWidth;
								pBitsOut += pBuffer->mnScanlineSize;
							}
						}
						else if ( pBuffer->mnFormat & BMP_FORMAT_24BIT_TC_RGB )
						{
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
						}

						t.pEnv->ReleasePrimitiveArrayCritical( (jintArray)mpData->getJavaObject(), pBits, JNI_ABORT );
					}
				}
			}
			else
			{
				memset( mpBits, 0, pBuffer->mnScanlineSize * pBuffer->mnHeight );
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
				if ( !bReadOnly )
				{
					// Save the palette
					USHORT nColors = ( ( mnBitCount <= 8 ) ? ( 1 << mnBitCount ) : 0 );
					if ( nColors )
					{
						maPalette = pBuffer->maPalette;
						maPalette.SetEntryCount( nColors );
					}

					jboolean bCopy( sal_False );
					jint *pBits = (jint *)t.pEnv->GetPrimitiveArrayCritical( (jintArray)mpData->getJavaObject(), &bCopy );
					if ( pBits )
					{
						BYTE *pBitsIn = mpBits;
						jint *pBitsOut = pBits;

						if ( pBuffer->mnFormat & BMP_FORMAT_1BIT_MSB_PAL )
						{
							for ( long i = 0; i < pBuffer->mnHeight; i++ )
							{
								for ( long j = 0; j < pBuffer->mnWidth; j++ )
								{
									BitmapColor& rColor = maPalette[ pBitsIn[ j >> 3 ] & ( 1 << ( 7 - ( j & 7 ) ) ) ? 1 : 0 ];
									pBitsOut[ j ] = MAKE_SALCOLOR( rColor.GetRed(), rColor.GetGreen(), rColor.GetBlue() ) | 0xff000000;
								}
		
								pBitsIn += pBuffer->mnScanlineSize;
								pBitsOut += pBuffer->mnWidth;
							}
						}
						else if ( pBuffer->mnFormat & BMP_FORMAT_4BIT_MSN_PAL )
						{
							for ( long i = 0; i < pBuffer->mnHeight; i++ )
							{
								for ( long j = 0; j < pBuffer->mnWidth; j++ )
								{
									BitmapColor& rColor = maPalette[ ( pBitsIn[ j >> 1 ] >> ( j & 1 ? 0 : 4 ) ) & 0x0f ];
									pBitsOut[ j ] = MAKE_SALCOLOR( rColor.GetRed(), rColor.GetGreen(), rColor.GetBlue() ) | 0xff000000;
								}
		
								pBitsIn += pBuffer->mnScanlineSize;
								pBitsOut += pBuffer->mnWidth;
							}
						}
						else if ( pBuffer->mnFormat & BMP_FORMAT_8BIT_PAL )
						{
							for ( long i = 0; i < pBuffer->mnHeight; i++ )
							{
								for ( long j = 0; j < pBuffer->mnWidth; j++ )
								{
									BitmapColor& rColor = maPalette[ pBitsIn[ j ] ];
									pBitsOut[ j ] = MAKE_SALCOLOR( rColor.GetRed(), rColor.GetGreen(), rColor.GetBlue() ) | 0xff000000;
								}
		
								pBitsIn += pBuffer->mnScanlineSize;
								pBitsOut += pBuffer->mnWidth;
							}
						}
						else if ( pBuffer->mnFormat & BMP_FORMAT_16BIT_TC_MSB_MASK )
						{
							BitmapColor aColor;
							for ( long i = 0; i < pBuffer->mnHeight; i++ )
							{
								for ( long j = 0; j < pBuffer->mnWidth; j++ )
								{
									pBuffer->maColorMask.GetColorFor16BitMSB( aColor, pBitsIn + ( j << 1UL ) );
									pBitsOut[ j ] = MAKE_SALCOLOR( aColor.GetRed(), aColor.GetGreen(), aColor.GetBlue() ) | 0xff000000;
								}
		
								pBitsIn += pBuffer->mnScanlineSize;
								pBitsOut += pBuffer->mnWidth;
							}
						}
						else if ( pBuffer->mnFormat & BMP_FORMAT_24BIT_TC_RGB )
						{
							for ( long i = 0; i < pBuffer->mnHeight; i++ )
							{
								long j;
								long k;
								for ( j = 0, k = 0; j < pBuffer->mnWidth; j++, k += 3 )
									pBitsOut[ j ] = MAKE_SALCOLOR( pBitsIn[ k ], pBitsIn[ k + 1 ], pBitsIn[ k + 2 ] ) | 0xff000000;
		
								pBitsIn += pBuffer->mnScanlineSize;
								pBitsOut += pBuffer->mnWidth;
							}
						}

						t.pEnv->ReleasePrimitiveArrayCritical( (jintArray)mpData->getJavaObject(), pBits, 0 );
					}
				}
			}

			if ( !mnAcquireCount && mpBits )
			{
				rtl_freeMemory( mpBits );
				mpBits = NULL;
				mbCopyFromVCLBitmap = true;
			}
		}
		delete pBuffer;
	}
}
