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

#ifndef _SV_SALBMP_HXX
#define _SV_SALBMP_HXX

#ifndef _SV_SV_H
#include <sv.h>
#endif
#ifndef _SV_GEN_HXX
#include <tools/gen.hxx>
#endif
#ifndef _SV_SALBTYPE_HXX
#include <salbtype.hxx>
#endif
#ifndef _SV_COM_SUN_STAR_VCL_VCLBITMAP_HXX
#include <com/sun/star/vcl/VCLBitmap.hxx>
#endif

struct	BitmapBuffer;
class	BitmapPalette;
class	SalGraphics;

// -------------
// - SalBitmap -
// -------------

class SalBitmap
{
	friend class	SalGraphics;
	friend class	SalVirtualDevice;

	Size 			maSize;
	USHORT			mnAcquired;
	USHORT			mnBitCount;
	::vcl::java_lang_Object*	mpData;
	BYTE*			mpBits;
	::vcl::com_sun_star_vcl_VCLBitmap*	mpVCLBitmap;
public:	
					SalBitmap();
					~SalBitmap();

	BOOL			Create( const Size& rSize, USHORT nBitCount, const BitmapPalette& rPal );
	BOOL			Create( const SalBitmap& rSalBmp );
	BOOL			Create( const SalBitmap& rSalBmp, SalGraphics *pGraphics );
	BOOL			Create( const SalBitmap& rSalBmp, USHORT nNewBitCount );

	void			Destroy();

	USHORT			GetBitCount() const;
	Size			GetSize() const;

	BitmapBuffer*	AcquireBuffer( BOOL bReadOnly );
	void			ReleaseBuffer( BitmapBuffer* pBuffer, BOOL bReadOnly );
};

#endif // _SV_SALBMP_HXX
