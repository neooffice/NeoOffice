/*************************************************************************
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * $RCSfile$
 * $Revision$
 *
 * This file is part of NeoOffice.
 *
 * NeoOffice is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * NeoOffice is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU General Public License
 * version 3 along with NeoOffice.  If not, see
 * <http://www.gnu.org/licenses/gpl-3.0.txt>
 * for a copy of the GPLv3 License.
 *
 * Modified February 2006 by Patrick Luby. NeoOffice is distributed under
 * GPL only under modification term 2 of the LGPL.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"
#include <vcl/salbtype.hxx>
#include <vcl/bmpacc.hxx>

// ----------------
// - BitmapAccess -
// ----------------

IMPL_FORMAT_GETPIXEL_NOMASK( _1BIT_MSB_PAL )
{
	return( pScanline[ nX >> 3 ] & ( 1 << ( 7 - ( nX & 7 ) ) ) ? 1 : 0 );
}

// ------------------------------------------------------------------

IMPL_FORMAT_SETPIXEL_NOMASK( _1BIT_MSB_PAL )
{
	BYTE& rByte = pScanline[ nX >> 3 ];

	( rBitmapColor.GetIndex() & 1 ) ? ( rByte |= 1 << ( 7 - ( nX & 7 ) ) ) :
									  ( rByte &= ~( 1 << ( 7 - ( nX & 7 ) ) ) );
}

// ------------------------------------------------------------------

IMPL_FORMAT_GETPIXEL_NOMASK( _1BIT_LSB_PAL )
{
	return( pScanline[ nX >> 3 ] & ( 1 << ( nX & 7 ) ) ? 1 : 0 );
}

// ------------------------------------------------------------------

IMPL_FORMAT_SETPIXEL_NOMASK( _1BIT_LSB_PAL )
{
	BYTE& rByte = pScanline[ nX >> 3 ];

	( rBitmapColor.GetIndex() & 1 ) ? ( rByte |= 1 << ( nX & 7 ) ) :
									  ( rByte &= ~( 1 << ( nX & 7 ) ) );
}

// ------------------------------------------------------------------

IMPL_FORMAT_GETPIXEL_NOMASK( _4BIT_MSN_PAL )
{
	return( ( pScanline[ nX >> 1 ] >> ( nX & 1 ? 0 : 4 ) ) & 0x0f );
}

// ------------------------------------------------------------------

IMPL_FORMAT_SETPIXEL_NOMASK( _4BIT_MSN_PAL )
{
	BYTE& rByte = pScanline[ nX >> 1 ];

	( nX & 1 ) ? ( rByte &= 0xf0, rByte |= ( rBitmapColor.GetIndex() & 0x0f ) ) : 
				 ( rByte &= 0x0f, rByte |= ( rBitmapColor.GetIndex() << 4 ) );
}

// ------------------------------------------------------------------

IMPL_FORMAT_GETPIXEL_NOMASK( _4BIT_LSN_PAL )
{
	return( ( pScanline[ nX >> 1 ] >> ( nX & 1 ? 4 : 0 ) ) & 0x0f );
}

// ------------------------------------------------------------------

IMPL_FORMAT_SETPIXEL_NOMASK( _4BIT_LSN_PAL )
{
	BYTE& rByte = pScanline[ nX >> 1 ];

	( nX & 1 ) ? ( rByte &= 0x0f, rByte |= ( rBitmapColor.GetIndex() << 4 ) ) : 
				 ( rByte &= 0xf0, rByte |= ( rBitmapColor.GetIndex() & 0x0f ) );
}

// ------------------------------------------------------------------

IMPL_FORMAT_GETPIXEL_NOMASK( _8BIT_PAL )
{
	return pScanline[ nX ];
}

// ------------------------------------------------------------------

IMPL_FORMAT_SETPIXEL_NOMASK( _8BIT_PAL )
{	
	pScanline[ nX ] = rBitmapColor.GetIndex();
}

// ------------------------------------------------------------------

IMPL_FORMAT_GETPIXEL( _8BIT_TC_MASK )
{
	BitmapColor aColor;
	rMask.GetColorFor8Bit( aColor, pScanline + nX );
	return aColor;
}

// ------------------------------------------------------------------

IMPL_FORMAT_SETPIXEL( _8BIT_TC_MASK )
{
	rMask.SetColorFor8Bit( rBitmapColor, pScanline + nX );
}

// ------------------------------------------------------------------

IMPL_FORMAT_GETPIXEL( _16BIT_TC_MSB_MASK )
{
	BitmapColor aColor;
	rMask.GetColorFor16BitMSB( aColor, pScanline + ( nX << 1UL ) );
	return aColor;
}

// ------------------------------------------------------------------

IMPL_FORMAT_SETPIXEL( _16BIT_TC_MSB_MASK )
{
	rMask.SetColorFor16BitMSB( rBitmapColor, pScanline + ( nX << 1UL ) );
}

// ------------------------------------------------------------------

IMPL_FORMAT_GETPIXEL( _16BIT_TC_LSB_MASK )
{
    BitmapColor aColor;
    rMask.GetColorFor16BitLSB( aColor, pScanline + ( nX << 1UL ) );
    return aColor;
}

// ------------------------------------------------------------------

IMPL_FORMAT_SETPIXEL( _16BIT_TC_LSB_MASK )
{
    rMask.SetColorFor16BitLSB( rBitmapColor, pScanline + ( nX << 1UL ) );
}

// ------------------------------------------------------------------

IMPL_FORMAT_GETPIXEL_NOMASK( _24BIT_TC_BGR )
{
	BitmapColor aBitmapColor;

	aBitmapColor.SetBlue( *( pScanline = pScanline + nX * 3 )++ );
	aBitmapColor.SetGreen( *pScanline++ );
	aBitmapColor.SetRed( *pScanline );

	return aBitmapColor;
}

// ------------------------------------------------------------------

IMPL_FORMAT_SETPIXEL_NOMASK( _24BIT_TC_BGR )
{
	*( pScanline = pScanline + nX * 3 )++ = rBitmapColor.GetBlue();
	*pScanline++ = rBitmapColor.GetGreen();
	*pScanline = rBitmapColor.GetRed();
}

// ------------------------------------------------------------------

IMPL_FORMAT_GETPIXEL_NOMASK( _24BIT_TC_RGB )
{
	BitmapColor aBitmapColor;

	aBitmapColor.SetRed( *( pScanline = pScanline + nX * 3 )++ );
	aBitmapColor.SetGreen( *pScanline++ );
	aBitmapColor.SetBlue( *pScanline );

	return aBitmapColor;
}

// ------------------------------------------------------------------

IMPL_FORMAT_SETPIXEL_NOMASK( _24BIT_TC_RGB )
{
	*( pScanline = pScanline + nX * 3 )++ = rBitmapColor.GetRed();
	*pScanline++ = rBitmapColor.GetGreen();
	*pScanline = rBitmapColor.GetBlue();
}

// ------------------------------------------------------------------

IMPL_FORMAT_GETPIXEL( _24BIT_TC_MASK )
{
	BitmapColor aColor;
	rMask.GetColorFor24Bit( aColor, pScanline + nX * 3L );
	return aColor;
}

// ------------------------------------------------------------------

IMPL_FORMAT_SETPIXEL( _24BIT_TC_MASK )
{
	rMask.SetColorFor24Bit( rBitmapColor, pScanline + nX * 3L );
}

// ------------------------------------------------------------------

IMPL_FORMAT_GETPIXEL_NOMASK( _32BIT_TC_ABGR )
{
	BitmapColor aBitmapColor;

	aBitmapColor.SetBlue( *( pScanline = pScanline + ( nX << 2 ) + 1 )++ );
	aBitmapColor.SetGreen( *pScanline++ );
	aBitmapColor.SetRed( *pScanline );

	return aBitmapColor;
}

// ------------------------------------------------------------------

IMPL_FORMAT_SETPIXEL_NOMASK( _32BIT_TC_ABGR )
{
#ifdef USE_JAVA
	*( pScanline = pScanline + ( nX << 2 ) )++ = 0xFF;
#else	// USE_JAVA
	*( pScanline = pScanline + ( nX << 2 ) )++ = 0;
#endif	// USE_JAVA
	*pScanline++ = rBitmapColor.GetBlue();
	*pScanline++ = rBitmapColor.GetGreen();
	*pScanline = rBitmapColor.GetRed();
}

// ------------------------------------------------------------------

IMPL_FORMAT_GETPIXEL_NOMASK( _32BIT_TC_ARGB )
{
	BitmapColor aBitmapColor;

	aBitmapColor.SetRed( *( pScanline = pScanline + ( nX << 2 ) + 1 )++ );
	aBitmapColor.SetGreen( *pScanline++ );
	aBitmapColor.SetBlue( *pScanline );

	return aBitmapColor;
}

// ------------------------------------------------------------------

IMPL_FORMAT_SETPIXEL_NOMASK( _32BIT_TC_ARGB )
{
	*( pScanline = pScanline + ( nX << 2 ) )++ = 0;
	*pScanline++ = rBitmapColor.GetRed();
	*pScanline++ = rBitmapColor.GetGreen();
	*pScanline = rBitmapColor.GetBlue();
}

// ------------------------------------------------------------------

IMPL_FORMAT_GETPIXEL_NOMASK( _32BIT_TC_BGRA )
{
	BitmapColor aBitmapColor;

	aBitmapColor.SetBlue( *( pScanline = pScanline + ( nX << 2 ) )++ );
	aBitmapColor.SetGreen( *pScanline++ );
	aBitmapColor.SetRed( *pScanline );

	return aBitmapColor;
}

// ------------------------------------------------------------------

IMPL_FORMAT_SETPIXEL_NOMASK( _32BIT_TC_BGRA )
{		   
	*( pScanline = pScanline + ( nX << 2 ) )++ = rBitmapColor.GetBlue();
	*pScanline++ = rBitmapColor.GetGreen();
	*pScanline++ = rBitmapColor.GetRed();
#ifdef USE_JAVA
	*pScanline = 0xFF;
#else	// USE_JAVA
	*pScanline = 0;
#endif	// USE_JAVA
}

// ------------------------------------------------------------------

IMPL_FORMAT_GETPIXEL_NOMASK( _32BIT_TC_RGBA )
{
	BitmapColor aBitmapColor;

	aBitmapColor.SetRed( *( pScanline = pScanline + ( nX << 2 ) )++ );
	aBitmapColor.SetGreen( *pScanline++ );
	aBitmapColor.SetBlue( *pScanline );

	return aBitmapColor;
}

// ------------------------------------------------------------------

IMPL_FORMAT_SETPIXEL_NOMASK( _32BIT_TC_RGBA )
{
	*( pScanline = pScanline + ( nX << 2 ) )++ = rBitmapColor.GetRed();
	*pScanline++ = rBitmapColor.GetGreen();
	*pScanline++ = rBitmapColor.GetBlue();
	*pScanline = 0;
}

// ------------------------------------------------------------------

IMPL_FORMAT_GETPIXEL( _32BIT_TC_MASK )
{
	BitmapColor aColor;
	rMask.GetColorFor32Bit( aColor, pScanline + ( nX << 2UL ) );
	return aColor;
}

// ------------------------------------------------------------------

IMPL_FORMAT_SETPIXEL( _32BIT_TC_MASK )
{
	rMask.SetColorFor32Bit( rBitmapColor, pScanline + ( nX << 2UL ) );
}
