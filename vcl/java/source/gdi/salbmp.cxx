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

#ifndef _SV_SALBMP_H
#include <salbmp.h>
#endif
#ifndef _SV_SALGDI_HXX
#include <salgdi.hxx>
#endif
#ifndef _SV_COM_SUN_STAR_VCL_VCLGRAPHICS_HXX
#include <com/sun/star/vcl/VCLGraphics.hxx>
#endif

using namespace vcl;

// ==================================================================

ULONG JavaSalBitmap::Get32BitNativeFormat()
{
#ifdef POWERPC
	return BMP_FORMAT_32BIT_TC_ARGB;
#else	// POWERPC
	return BMP_FORMAT_32BIT_TC_BGRA;
#endif	// POWERPC
}

// ------------------------------------------------------------------

JavaSalBitmap::JavaSalBitmap() :
	maSize( 0, 0 ),
	mnAcquireCount( 0 ),
	mnBitCount( 0 ),
	mpBits( NULL )
{
}

// ------------------------------------------------------------------

JavaSalBitmap::~JavaSalBitmap()
{
	Destroy();
}

// ------------------------------------------------------------------

com_sun_star_vcl_VCLBitmap *JavaSalBitmap::GetVCLBitmap( long nX, long nY, long nWidth, long nHeight )
{
	if ( nWidth < 1 || nHeight < 1 )
		return NULL;

	com_sun_star_vcl_VCLBitmap *pVCLBitmap = new com_sun_star_vcl_VCLBitmap( nWidth, nHeight, mnBitCount );
	if ( pVCLBitmap && pVCLBitmap->getJavaObject() )
	{
		// Fill the buffer with pointers to the Java buffer
		java_lang_Object *pData = pVCLBitmap->getData();
		if ( pData )
		{
			// Force copying of the buffer if it has not already been done
			BitmapBuffer *pBuffer = AcquireBuffer( FALSE );
			if ( pBuffer )
			{
				VCLThreadAttach t;
				if ( t.pEnv )
				{
					jboolean bCopy( sal_False );
					jint *pBits = (jint *)t.pEnv->GetPrimitiveArrayCritical( (jintArray)pData->getJavaObject(), &bCopy );
					if ( pBits )
					{
						BYTE *pBitsIn = mpBits + ( nY * pBuffer->mnScanlineSize ) + ( nX * mnBitCount / 8 );
						jint *pBitsOut = pBits;

						if ( pBuffer->mnFormat & BMP_FORMAT_1BIT_MSB_PAL )
						{
							for ( long i = 0; i < nHeight; i++ )
							{
								for ( long j = 0; j < nWidth; j++ )
								{
									BitmapColor& rColor = maPalette[ pBitsIn[ j >> 3 ] & ( 1 << ( 7 - ( j & 7 ) ) ) ? 1 : 0 ];
									pBitsOut[ j ] = MAKE_SALCOLOR( rColor.GetRed(), rColor.GetGreen(), rColor.GetBlue() ) | 0xff000000;
								}
		
								pBitsIn += pBuffer->mnScanlineSize;
								pBitsOut += nWidth;
							}
						}
						else if ( pBuffer->mnFormat & BMP_FORMAT_4BIT_MSN_PAL )
						{
							for ( long i = 0; i < nHeight; i++ )
							{
								for ( long j = 0; j < nWidth; j++ )
								{
									BitmapColor& rColor = maPalette[ ( pBitsIn[ j >> 1 ] >> ( j & 1 ? 0 : 4 ) ) & 0x0f ];
									pBitsOut[ j ] = MAKE_SALCOLOR( rColor.GetRed(), rColor.GetGreen(), rColor.GetBlue() ) | 0xff000000;
								}
		
								pBitsIn += pBuffer->mnScanlineSize;
								pBitsOut += nWidth;
							}
						}
						else if ( pBuffer->mnFormat & BMP_FORMAT_8BIT_PAL )
						{
							for ( long i = 0; i < nHeight; i++ )
							{
								for ( long j = 0; j < nWidth; j++ )
								{
									BitmapColor& rColor = maPalette[ pBitsIn[ j ] ];
									pBitsOut[ j ] = MAKE_SALCOLOR( rColor.GetRed(), rColor.GetGreen(), rColor.GetBlue() ) | 0xff000000;
								}
		
								pBitsIn += pBuffer->mnScanlineSize;
								pBitsOut += nWidth;
							}
						}
						else if ( pBuffer->mnFormat & BMP_FORMAT_16BIT_TC_MSB_MASK )
						{
							BitmapColor aColor;
							for ( long i = 0; i < nHeight; i++ )
							{
								for ( long j = 0; j < nWidth; j++ )
								{
									pBuffer->maColorMask.GetColorFor16BitMSB( aColor, pBitsIn + ( j << 1UL ) );
									pBitsOut[ j ] = MAKE_SALCOLOR( aColor.GetRed(), aColor.GetGreen(), aColor.GetBlue() ) | 0xff000000;
								}
			
								pBitsIn += pBuffer->mnScanlineSize;
								pBitsOut += nWidth;
							}
						}
						else if ( pBuffer->mnFormat & BMP_FORMAT_24BIT_TC_RGB )
						{
							for ( long i = 0; i < nHeight; i++ )
							{
								long j;
								long k;
								for ( j = 0, k = 0; j < nWidth; j++, k += 3 )
									pBitsOut[ j ] = MAKE_SALCOLOR( pBitsIn[ k ], pBitsIn[ k + 1 ], pBitsIn[ k + 2 ] ) | 0xff000000;
		
								pBitsIn += pBuffer->mnScanlineSize;
								pBitsOut += nWidth;
							}
						}
#ifdef POWERPC
						else if ( pBuffer->mnFormat & BMP_FORMAT_32BIT_TC_ARGB )
#else	// POWERPC
						else if ( pBuffer->mnFormat & BMP_FORMAT_32BIT_TC_BGRA )
#endif	// POWERPC
						{
							long nByteCount = nWidth * sizeof( jint );
							for ( long i = 0; i < nHeight; i++ )
							{
								memcpy( pBitsOut, pBitsIn, nByteCount );

								pBitsIn += pBuffer->mnScanlineSize;
								pBitsOut += nWidth;
							}
						}

						t.pEnv->ReleasePrimitiveArrayCritical( (jintArray)pData->getJavaObject(), pBits, 0 );
					}
				}

				ReleaseBuffer( pBuffer, FALSE );
			}

			delete pData;
		}
		else
		{
			delete pVCLBitmap;
			pVCLBitmap = NULL;
		}
	}
	else if ( pVCLBitmap )
	{
		delete pVCLBitmap;
		pVCLBitmap = NULL;
	}

	return pVCLBitmap;
}

// ------------------------------------------------------------------

void JavaSalBitmap::ReleaseVCLBitmap( com_sun_star_vcl_VCLBitmap *pVCLBitmap, bool bCopyFromVCLBitmap, long nX, long nY, long nWidth, long nHeight )
{
	if ( pVCLBitmap )
	{
		if ( bCopyFromVCLBitmap && nWidth > 0 && nHeight > 0 )
		{
			// Force copying of the buffer if necessary
			java_lang_Object *pData = pVCLBitmap->getData();
			if ( pData )
			{
				BitmapBuffer *pBuffer = AcquireBuffer( TRUE );
				if ( pBuffer )
				{
					VCLThreadAttach t;
					if ( t.pEnv )
					{
						jboolean bCopy( sal_False );
						jint *pBits = (jint *)t.pEnv->GetPrimitiveArrayCritical( (jintArray)pData->getJavaObject(), &bCopy );
						if ( pBits )
						{
							jint *pBitsIn = pBits;
							BYTE *pBitsOut = mpBits + ( nY * pBuffer->mnScanlineSize ) + ( nX * mnBitCount / 8 );

							if ( pBuffer->mnFormat & BMP_FORMAT_1BIT_MSB_PAL )
							{
								for ( long i = 0; i < nHeight; i++ )
								{
									for ( long j = 0; j < nWidth; j++ )
									{
										BYTE& rByte = pBitsOut[ j >> 3 ];
										USHORT nIndex = maPalette.GetBestIndex( BitmapColor( (BYTE)( pBitsIn[ j ] >> 16 ), (BYTE)( pBitsIn[ j ] >> 8 ), (BYTE)pBitsIn[ j ] ) );
										( nIndex & 1 ) ? ( rByte |= 1 << ( 7 - ( j & 7 ) ) ) : ( rByte &= ~( 1 << ( 7 - ( j & 7 ) ) ) );
									}

									pBitsIn += nWidth;
									pBitsOut += pBuffer->mnScanlineSize;
								}
							}
							else if ( pBuffer->mnFormat & BMP_FORMAT_4BIT_MSN_PAL )
							{
								for ( long i = 0; i < nHeight; i++ )
								{
									for ( long j = 0; j < nWidth; j++ )
									{
										BYTE& rByte = pBitsOut[ j >> 1 ];
										USHORT nIndex = maPalette.GetBestIndex( BitmapColor( (BYTE)( pBitsIn[ j ] >> 16 ), (BYTE)( pBitsIn[ j ] >> 8 ), (BYTE)pBitsIn[ j ] ) );
										( j & 1 ) ? ( rByte &= 0xf0, rByte |= ( nIndex & 0x0f ) ) : ( rByte &= 0x0f, rByte |= ( nIndex << 4 ) );
									}

									pBitsIn += nWidth;
									pBitsOut += pBuffer->mnScanlineSize;
								}
							}
							else if ( pBuffer->mnFormat & BMP_FORMAT_8BIT_PAL )
							{
								for ( long i = 0; i < nHeight; i++ )
								{
									for ( long j = 0; j < nWidth; j++ )
										pBitsOut[ j ] = (BYTE)maPalette.GetBestIndex( BitmapColor( (BYTE)( pBitsIn[ j ] >> 16 ), (BYTE)( pBitsIn[ j ] >> 8 ), (BYTE)pBitsIn[ j ] ) );

									pBitsIn += nWidth;
									pBitsOut += pBuffer->mnScanlineSize;
								}
							}
							else if ( pBuffer->mnFormat & BMP_FORMAT_16BIT_TC_MSB_MASK )
							{
								for ( long i = 0; i < nHeight; i++ )
								{
									for ( long j = 0; j < nWidth; j++ )
										pBuffer->maColorMask.SetColorFor16BitMSB( BitmapColor( (BYTE)( pBitsIn[ j ] >> 16 ), (BYTE)( pBitsIn[ j ] >> 8 ), (BYTE)pBitsIn[ j ] ), pBitsOut + ( j << 1UL ) );

									pBitsIn += nWidth;
									pBitsOut += pBuffer->mnScanlineSize;
								}
							}
							else if ( pBuffer->mnFormat & BMP_FORMAT_24BIT_TC_RGB )
							{
								for ( long i = 0; i < nHeight; i++ )
								{
									long j;
									long k;
									for ( j = 0, k = 0; j < nWidth; j++ )
									{
										pBitsOut[ k++ ] = SALCOLOR_RED( pBitsIn[ j ] );
										pBitsOut[ k++ ] = SALCOLOR_GREEN( pBitsIn[ j ] );
										pBitsOut[ k++ ] = SALCOLOR_BLUE( pBitsIn[ j ] );
									}

									pBitsIn += nWidth;
									pBitsOut += pBuffer->mnScanlineSize;
								}
							}
#ifdef POWERPC
							else if ( pBuffer->mnFormat & BMP_FORMAT_32BIT_TC_ARGB )
#else	// POWERPC
							else if ( pBuffer->mnFormat & BMP_FORMAT_32BIT_TC_BGRA )
#endif	// POWERPC
							{
								long nByteCount = nWidth * sizeof( jint );
								for ( long i = 0; i < nHeight; i++ )
								{
									memcpy( pBitsOut, pBitsIn, nByteCount );

									pBitsIn += nWidth;
									pBitsOut += pBuffer->mnScanlineSize;
								}
							}

							t.pEnv->ReleasePrimitiveArrayCritical( (jintArray)pData->getJavaObject(), pBits, JNI_ABORT );
						}
					}

					ReleaseBuffer( pBuffer, TRUE );
				}

				delete pData;
			}
		}

		delete pVCLBitmap;
	}
}

// ------------------------------------------------------------------

/**
 * Warning: this method takes ownership of the BitmapBuffer's mpBits member
 * so after calling this method, the mpBits member will be set to NULL.
 */
bool JavaSalBitmap::Create( BitmapBuffer *pBuffer )
{
	Destroy();

	bool bRet = false;

	if ( !pBuffer || !pBuffer->mpBits )
		return bRet;

	Size aSize( pBuffer->mnWidth, pBuffer->mnHeight );
	bRet = Create( aSize, pBuffer->mnBitCount, pBuffer->maPalette );
	if ( bRet )
	{
		mpBits = pBuffer->mpBits;
		pBuffer->mpBits = NULL;
	}

	return bRet;
}

// ------------------------------------------------------------------

bool JavaSalBitmap::Create( const Size& rSize, USHORT nBitCount, const BitmapPalette& rPal )
{
	Destroy();

	maSize = Size( rSize );
	if ( maSize.Width() <= 0 && maSize.Height() <= 0 )
		return false;

	if ( nBitCount <= 1 )
		mnBitCount = 1;
	else if ( nBitCount <= 4 )
		mnBitCount = 4;
	else if ( nBitCount <= 8 )
		mnBitCount = 8;
	else if ( nBitCount <= 16 )
		mnBitCount = 16;
	else if ( nBitCount <= 24 )
		mnBitCount = 24;
	else
		mnBitCount = 32;

	// Save the palette
	USHORT nColors = ( ( mnBitCount <= 8 ) ? ( 1 << mnBitCount ) : 0 );
	if ( nColors )
	{
		maPalette = rPal;
		maPalette.SetEntryCount( nColors );
	}

	return true;
}

// ------------------------------------------------------------------

bool JavaSalBitmap::Create( const SalBitmap& rSalBmp )
{
	Destroy();

	JavaSalBitmap& rJavaSalBmp = (JavaSalBitmap&)rSalBmp;
	bool bRet = Create( rJavaSalBmp.GetSize(), rJavaSalBmp.GetBitCount(), rJavaSalBmp.maPalette );

	if ( bRet )
	{
		BitmapBuffer *pSrcBuffer = rJavaSalBmp.AcquireBuffer( TRUE );
		if ( pSrcBuffer )
		{
			BitmapBuffer *pDestBuffer = AcquireBuffer( FALSE );
			if ( pDestBuffer )
			{
				memcpy( pDestBuffer->mpBits, pSrcBuffer->mpBits, pDestBuffer->mnScanlineSize * pDestBuffer->mnHeight );
				ReleaseBuffer( pDestBuffer, FALSE );
			}
			else
			{
				Destroy();
				bRet = false;
			}
			rJavaSalBmp.ReleaseBuffer( pSrcBuffer, TRUE );
		}
		else
		{
			Destroy();
			bRet = false;
		}
	}

	return bRet;
}

// ------------------------------------------------------------------

bool JavaSalBitmap::Create( const SalBitmap& rSalBmp, SalGraphics* pGraphics )
{
	return false;
}

// ------------------------------------------------------------------

bool JavaSalBitmap::Create( const SalBitmap& rSalBmp, USHORT nNewBitCount )
{
	return false;
}

// ------------------------------------------------------------------

void JavaSalBitmap::Destroy()
{
	maSize = Size( 0, 0 );
	mnAcquireCount = 0;
	mnBitCount = 0;

	if ( mpBits )
	{
		delete[] mpBits;
		mpBits = NULL;
	}

	maPalette.SetEntryCount( 0 );
}

// ------------------------------------------------------------------

USHORT JavaSalBitmap::GetBitCount() const
{
	return mnBitCount;
}

// ------------------------------------------------------------------

BitmapBuffer* JavaSalBitmap::AcquireBuffer( bool bReadOnly )
{
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
	else if ( mnBitCount <= 24 )
	{
		pBuffer->mnFormat |= BMP_FORMAT_24BIT_TC_RGB;
	}
	else
	{
		pBuffer->mnFormat |= JavaSalBitmap::Get32BitNativeFormat();
	}

	pBuffer->mnWidth = maSize.Width();
	pBuffer->mnHeight = maSize.Height();
	pBuffer->mnScanlineSize = AlignedWidth4Bytes( mnBitCount * maSize.Width() );
	pBuffer->maPalette = maPalette;

	if ( !mpBits )
	{
		mpBits = new BYTE[ pBuffer->mnScanlineSize * pBuffer->mnHeight ];
		if ( mpBits )
		{
			memset( mpBits, 0, pBuffer->mnScanlineSize * pBuffer->mnHeight );
		}
		else
		{
			delete pBuffer;
			return NULL;
		}
	}

	if ( !bReadOnly )
		mnAcquireCount++;
	pBuffer->mpBits = mpBits;

	return pBuffer;
}

// ------------------------------------------------------------------

void JavaSalBitmap::ReleaseBuffer( BitmapBuffer* pBuffer, bool bReadOnly )
{
	if ( pBuffer )
	{
		if ( !bReadOnly )
		{
			mnAcquireCount--;

			// Save the palette
			USHORT nColors = ( ( mnBitCount <= 8 ) ? ( 1 << mnBitCount ) : 0 );
			if ( nColors )
			{
				maPalette = pBuffer->maPalette;
				maPalette.SetEntryCount( nColors );
			}
		}
		delete pBuffer;
	}
}

// ------------------------------------------------------------------

bool JavaSalBitmap::GetSystemData( BitmapSystemData& rData )
{
#ifdef DEBUG
	fprintf( stderr, "JavaSalBitmap::GetSystemData not implemented\n" );
#endif
	return false;
}
