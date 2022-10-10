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
 *  Copyright 2006 Planamesa Inc.
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

#include <tools/gen.hxx>
#include <vcl/salbtype.hxx>

#include "salbmp.hxx"

struct BitmapBuffer;
class BitmapPalette;
class JavaSalGraphics;
class JavaSalVirtualDevice;

// -----------------
// - JavaSalBitmap -
// -----------------

class JavaSalBitmap : public SalBitmap
{
	Point					maPoint;
	Size					maSize;
	sal_uInt16				mnBitCount;
	sal_uInt8*				mpBits;
	BitmapPalette			maPalette;
	BitmapBuffer*			mpBuffer;
	JavaSalGraphics*		mpGraphics;
	JavaSalVirtualDevice*	mpVirDev;

public:
	static sal_uLong		Get32BitNativeFormat();
	static sal_uLong		GetNativeDirectionFormat();

							JavaSalBitmap();
	virtual					~JavaSalBitmap();

	bool					Create( BitmapBuffer *pBuffer );
	bool					Create( const Point& rPoint, const Size& rSize, JavaSalGraphics *pSrcGraphics, const BitmapPalette& rPal );
	Point					GetPoint() const { return maPoint; }
	JavaSalGraphics*		GetGraphics() { return mpGraphics; }

	virtual bool			Create( const Size& rSize, sal_uInt16 nBitCount, const BitmapPalette& rPal ) SAL_OVERRIDE;
	virtual bool			Create( const SalBitmap& rSalBmp ) SAL_OVERRIDE;
	virtual bool			Create( const SalBitmap& rSalBmp, SalGraphics* pGraphics ) SAL_OVERRIDE;
	virtual bool			Create( const SalBitmap& rSalBmp, sal_uInt16 nNewBitCount ) SAL_OVERRIDE;
	virtual bool			Create( const ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XBitmapCanvas > xBitmapCanvas, Size& rSize, bool bMask = false ) SAL_OVERRIDE;
	virtual void			Destroy() SAL_OVERRIDE;
	virtual Size			GetSize() const SAL_OVERRIDE { return maSize; }
	virtual sal_uInt16		GetBitCount() const SAL_OVERRIDE;
	virtual BitmapBuffer*	AcquireBuffer( BitmapAccessMode nMode ) SAL_OVERRIDE;
	virtual void			ReleaseBuffer( BitmapBuffer* pBuffer, BitmapAccessMode nMode ) SAL_OVERRIDE;
	virtual bool			GetSystemData( BitmapSystemData& rData ) SAL_OVERRIDE;
	virtual bool			Crop( const Rectangle& rRectPixel ) SAL_OVERRIDE;
	virtual bool			Erase( const Color& rFillColor ) SAL_OVERRIDE;
	virtual bool			Scale( const double& rScaleX, const double& rScaleY, sal_uInt32 nScaleFlag ) SAL_OVERRIDE;
	virtual bool			Replace( const Color& rSearchColor, const Color& rReplaceColor, sal_uLong nTol ) SAL_OVERRIDE;
};

extern "C" SAL_DLLPRIVATE void ReleaseBitmapBufferBytePointerCallback( void *pInfo, const void *pPointer, size_t nSize );

#endif // _SV_SALBMP_H
