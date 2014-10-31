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

#ifndef INCLUDED_VCL_GFXLINK_HXX
#define INCLUDED_VCL_GFXLINK_HXX

#include <rtl/ustring.hxx>
#include <tools/stream.hxx>
#include <tools/solar.h>
#include <vcl/dllapi.h>
#include <vcl/mapmod.hxx>

#if SUPD == 310
#include <tools/urlobj.hxx>
#endif	// SUPD == 310


// - ImpBuffer -


struct ImpBuffer
{
#if SUPD == 310
    ULONG           mnRefCount;
#else	// SUPD == 310
    sal_uLong       mnRefCount;
#endif	// SUPD == 310
    sal_uInt8*      mpBuffer;

#if SUPD == 310
				ImpBuffer( ULONG nSize )
#else	// SUPD == 310
                ImpBuffer( sal_uLong nSize )
#endif	// SUPD == 310
                {
                    mnRefCount = 1UL;
                    mpBuffer = nSize ? new sal_uInt8[ nSize ] : NULL;
                }

                ImpBuffer( sal_uInt8* pBuf ) { mnRefCount = 1UL; mpBuffer = pBuf; }

                ~ImpBuffer() { delete[] mpBuffer; }
};


// - ImpSwap -


struct ImpSwap
{
#if SUPD == 310
    INetURLObject   maURL;
    ULONG               mnDataSize;
    ULONG               mnRefCount;

                    ImpSwap( sal_uInt8* pData, ULONG nDataSize );
#else	// SUPD == 310
    OUString   maURL;
    sal_uLong           mnDataSize;
    sal_uLong           mnRefCount;

                    ImpSwap( sal_uInt8* pData, sal_uLong nDataSize );
#endif	// SUPD == 310
                    ~ImpSwap();

    sal_uInt8*          GetData() const;

#if SUPD == 310
	BOOL			IsSwapped() const { return maURL.GetMainURL( INetURLObject::NO_DECODE ).getLength() > 0; }
#else	// SUPD == 310
    bool            IsSwapped() const { return maURL.getLength() > 0; }
#endif	// SUPD == 310

    void            WriteTo( SvStream& rOStm ) const;
};


// - ImpGfxLink -


struct ImpGfxLink
{
    MapMode         maPrefMapMode;
    Size            maPrefSize;
    bool            mbPrefMapModeValid;
    bool            mbPrefSizeValid;

    ImpGfxLink() :
        maPrefMapMode(),
        maPrefSize(),
        mbPrefMapModeValid( false ),
        mbPrefSizeValid( false )
    {}
};

//#endif // __PRIVATE


// - GfxLinkType -


enum GfxLinkType
{
    GFX_LINK_TYPE_NONE          = 0,
    GFX_LINK_TYPE_EPS_BUFFER    = 1,
    GFX_LINK_TYPE_NATIVE_GIF    = 2,    // Don't forget to update the following defines
    GFX_LINK_TYPE_NATIVE_JPG    = 3,    // Don't forget to update the following defines
    GFX_LINK_TYPE_NATIVE_PNG    = 4,    // Don't forget to update the following defines
    GFX_LINK_TYPE_NATIVE_TIF    = 5,    // Don't forget to update the following defines
    GFX_LINK_TYPE_NATIVE_WMF    = 6,    // Don't forget to update the following defines
    GFX_LINK_TYPE_NATIVE_MET    = 7,    // Don't forget to update the following defines
    GFX_LINK_TYPE_NATIVE_PCT    = 8,    // Don't forget to update the following defines
    GFX_LINK_TYPE_NATIVE_SVG    = 9,    // Don't forget to update the following defines
    GFX_LINK_TYPE_NATIVE_MOV    = 10,   // Don't forget to update the following defines
    // #i15508# added BMP type support
    GFX_LINK_TYPE_NATIVE_BMP    = 11,   // Don't forget to update the following defines
    GFX_LINK_TYPE_USER          = 0xffff
};

#define GFX_LINK_FIRST_NATIVE_ID    GFX_LINK_TYPE_NATIVE_GIF
#define GFX_LINK_LAST_NATIVE_ID     GFX_LINK_TYPE_NATIVE_BMP


// - GfxLink -


struct ImpBuffer;
struct ImpSwap;
struct ImpGfxLink;
class Graphic;

class VCL_DLLPUBLIC GfxLink
{
private:

    GfxLinkType         meType;
    ImpBuffer*          mpBuf;
    ImpSwap*            mpSwap;
    sal_uInt32          mnBufSize;
    sal_uInt32          mnUserId;
    ImpGfxLink*         mpImpData;

    SAL_DLLPRIVATE void ImplCopy( const GfxLink& rGfxLink );

public:
                        GfxLink();
                        GfxLink( const GfxLink& );
                        GfxLink( const OUString& rPath, GfxLinkType nType );
#if SUPD == 310
                        GfxLink( sal_uInt8* pBuf, sal_uInt32 nBufSize, GfxLinkType nType, BOOL bOwns );
#else	// SUPD == 310
                        GfxLink( sal_uInt8* pBuf, sal_uInt32 nBufSize, GfxLinkType nType, bool bOwns );
#endif	// SUPD == 310
                        ~GfxLink();

    GfxLink&            operator=( const GfxLink& );
#if SUPD == 310
	sal_Bool			IsEqual( const GfxLink& ) const;
#else	// SUPD == 310
    bool            IsEqual( const GfxLink& ) const;
#endif	// SUPD == 310

    GfxLinkType         GetType() const;

    void                SetUserId( sal_uInt32 nUserId ) { mnUserId = nUserId; }
    sal_uInt32          GetUserId() const { return mnUserId; }

    sal_uInt32          GetDataSize() const;
#if SUPD == 310
    void                SetData( sal_uInt8* pBuf, sal_uInt32 nSize, GfxLinkType nType, BOOL bOwns );
#else	// SUPD == 310
    void                SetData( sal_uInt8* pBuf, sal_uInt32 nSize, GfxLinkType nType, bool bOwns );
#endif	// SUPD == 310
    const sal_uInt8*            GetData() const;

    const Size&         GetPrefSize() const;
    void                SetPrefSize( const Size& rPrefSize );
    bool                IsPrefSizeValid();

    const MapMode&      GetPrefMapMode() const;
    void                SetPrefMapMode( const MapMode& rPrefMapMode );
    bool                IsPrefMapModeValid();

#if SUPD == 310
    BOOL                IsNative() const;
    BOOL                IsUser() const { return( GFX_LINK_TYPE_USER == meType ); }

    BOOL                LoadNative( Graphic& rGraphic );

    BOOL                ExportNative( SvStream& rOStream ) const;
#else	// SUPD == 310
    bool                IsNative() const;
    bool                IsUser() const { return( GFX_LINK_TYPE_USER == meType ); }

    bool                LoadNative( Graphic& rGraphic );

    bool                ExportNative( SvStream& rOStream ) const;
#endif	// SUPD == 310

    void                SwapOut();
    void                SwapIn();
#if SUPD == 310
    BOOL                IsSwappedOut() const { return( mpSwap != NULL ); }
#else	// SUPD == 310
    bool                IsSwappedOut() const { return( mpSwap != NULL ); }
#endif	// SUPD == 310

public:

#if SUPD == 310
	friend VCL_DLLPUBLIC SvStream&	operator<<( SvStream& rOStream, const GfxLink& rGfxLink );
	friend VCL_DLLPUBLIC SvStream&	operator>>( SvStream& rIStream, GfxLink& rGfxLink );
#endif	// SUPD == 310
    friend VCL_DLLPUBLIC SvStream&  WriteGfxLink( SvStream& rOStream, const GfxLink& rGfxLink );
    friend VCL_DLLPUBLIC SvStream&  ReadGfxLink( SvStream& rIStream, GfxLink& rGfxLink );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
