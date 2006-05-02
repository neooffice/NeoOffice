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
 *  Patrick Luby, February 2006
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2006 by Patrick Luby (patrick.luby@planamesa.com)
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

#ifndef _SV_SALBMP_H
#define _SV_SALBMP_H

#ifndef _SV_SALBMP_HXX
#include <salbmp.hxx>
#endif
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

// -----------------
// - JavaSalBitmap -
// -----------------

class JavaSalBitmap : public SalBitmap
{
	Size					maSize;
	USHORT					mnAcquireCount;
	USHORT					mnBitCount;
	BYTE*					mpBits;
	bool					mbCopyFromVCLBitmap;
	::vcl::java_lang_Object*	mpData;
	BitmapPalette			maPalette;
	::vcl::com_sun_star_vcl_VCLBitmap*	mpVCLBitmap;
	USHORT					mnVCLBitmapAcquireCount;

public:
	static ULONG			Get32BitNativeFormat();

							JavaSalBitmap();
	virtual					~JavaSalBitmap();

	bool					Create( BitmapBuffer *pBuffer );
	::vcl::com_sun_star_vcl_VCLBitmap*	GetVCLBitmap();
	void					ReleaseVCLBitmap( ::vcl::com_sun_star_vcl_VCLBitmap *pVCLBitmap, bool bCopyFromVCLBitmap );

	virtual bool			Create( const Size& rSize, USHORT nBitCount, const BitmapPalette& rPal );
	virtual bool			Create( const SalBitmap& rSalBmp );
	virtual bool			Create( const SalBitmap& rSalBmp, SalGraphics* pGraphics );
	virtual bool			Create( const SalBitmap& rSalBmp, USHORT nNewBitCount );
	virtual void			Destroy();
	virtual Size			GetSize() const { return maSize; }
	virtual USHORT			GetBitCount() const;
	virtual BitmapBuffer*	AcquireBuffer( bool bReadOnly );
	virtual void			ReleaseBuffer( BitmapBuffer* pBuffer, bool bReadOnly );
	virtual bool			GetSystemData( BitmapSystemData& rData );
};

#endif // _SV_SALBMP_H
