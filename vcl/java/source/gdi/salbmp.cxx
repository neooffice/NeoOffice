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
	mpBits( NULL ),
	mpData( NULL ),
	mpVCLBitmap( NULL ),
	mnVCLBitmapAcquireCount( 0 )
{
}

// ------------------------------------------------------------------

JavaSalBitmap::~JavaSalBitmap()
{
	Destroy();
}

// ------------------------------------------------------------------

com_sun_star_vcl_VCLBitmap *JavaSalBitmap::GetVCLBitmap()
{
	if ( !mpVCLBitmap )
	{
		if ( mpData )
			delete mpData;
		mpData = NULL;

		mpVCLBitmap = new com_sun_star_vcl_VCLBitmap( maSize.Width(), maSize.Height(), mnBitCount );
		if ( mpVCLBitmap && mpVCLBitmap->getJavaObject() )
		{
			// Fill the buffer with pointers to the Java buffer
			mpData = mpVCLBitmap->getData();
			if ( mpData )
			{
				// Force copying of the buffer if it has not already been done
				BitmapBuffer *pBuffer = AcquireBuffer( FALSE );
				if ( pBuffer )
				{
					VCLThreadAttach t;
					if ( t.pEnv )
					{
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
#ifdef POWERPC
							else if ( pBuffer->mnFormat & BMP_FORMAT_32BIT_TC_ARGB )
#else	// POWERPC
							else if ( pBuffer->mnFormat & BMP_FORMAT_32BIT_TC_BGRA )
#endif	// POWERPC
							{
								memcpy( pBits, mpBits, pBuffer->mnScanlineSize * pBuffer->mnHeight );
							}

							t.pEnv->ReleasePrimitiveArrayCritical( (jintArray)mpData->getJavaObject(), pBits, 0 );
						}
					}

					ReleaseBuffer( pBuffer, FALSE );
				}
			}
			else
			{
				delete mpVCLBitmap;
				mpVCLBitmap = NULL;
			}
		}
		else if ( mpVCLBitmap )
		{
			delete mpVCLBitmap;
			mpVCLBitmap = NULL;
		}
	}

	if ( mpVCLBitmap )
		mnVCLBitmapAcquireCount++;

	return mpVCLBitmap;
}

// ------------------------------------------------------------------

void JavaSalBitmap::ReleaseVCLBitmap( com_sun_star_vcl_VCLBitmap *pVCLBitmap, bool bCopyFromVCLBitmap )
{
	if ( pVCLBitmap && pVCLBitmap == mpVCLBitmap )
	{
		mnVCLBitmapAcquireCount--;

		if ( bCopyFromVCLBitmap && mpData )
		{
			// Force copying of the buffer if necessary
			BitmapBuffer *pBuffer = AcquireBuffer( TRUE );
			if ( pBuffer )
			{
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
#ifdef POWERPC
						else if ( pBuffer->mnFormat & BMP_FORMAT_32BIT_TC_ARGB )
#else	// POWERPC
						else if ( pBuffer->mnFormat & BMP_FORMAT_32BIT_TC_BGRA )
#endif	// POWERPC
						{
							memcpy( mpBits, pBits, pBuffer->mnScanlineSize * pBuffer->mnHeight );
						}

						t.pEnv->ReleasePrimitiveArrayCritical( (jintArray)mpData->getJavaObject(), pBits, JNI_ABORT );
					}
				}

				ReleaseBuffer( pBuffer, TRUE );
			}
		}

		if ( !mnVCLBitmapAcquireCount )
		{
			if ( mpData )
			{
				delete mpData;
				mpData = NULL;
			}

			if ( mpVCLBitmap )
			{
				delete mpVCLBitmap;
				mpVCLBitmap = NULL;
			}
		}
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

	mnVCLBitmapAcquireCount = 0;
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
