/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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
