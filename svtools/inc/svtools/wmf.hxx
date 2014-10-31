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

#ifndef INCLUDED_VCL_WMF_HXX
#define INCLUDED_VCL_WMF_HXX

#if SUPD == 310
#include "svtools/svtdllapi.h"
#include <svtools/fltcall.hxx>
#else	// SUPD == 310
#include <vcl/dllapi.h>
#endif	// SUPD == 310

class FilterConfigItem;
class GDIMetaFile;
class SvStream;

struct WMF_EXTERNALHEADER
{
    sal_uInt16 xExt;
    sal_uInt16 yExt;

    /** One of the following values:
        <ul>
            <li>MM_TEXT</li>
            <li>MM_LOMETRIC</li>
            <li>MM_HIMETRIC</li>
            <li>MM_LOENGLISH</li>
            <li>MM_HIENGLISH</li>
            <li>MM_TWIPS</li>
            <li>MM_ISOTROPIC</li>
            <li>MM_ANISOTROPIC</li>
        </ul>
        If this value is 0, then no external mapmode has been defined,
        the internal one should then be used.
     */
    sal_uInt16 mapMode;

    WMF_EXTERNALHEADER() :
        xExt( 0 ),
        yExt( 0 ),
        mapMode( 0 )
    {
    }
};

#if SUPD == 310
BOOL ConvertWMFToGDIMetaFile( SvStream & rStreamWMF, GDIMetaFile & rGDIMetaFile, FilterConfigItem* pConfigItem = NULL );
#else	// SUPD == 310
bool ConvertWMFToGDIMetaFile( SvStream & rStreamWMF, GDIMetaFile & rGDIMetaFile, FilterConfigItem* pConfigItem = NULL, WMF_EXTERNALHEADER *pExtHeader = NULL );
#endif	// SUPD == 310

#if SUPD == 310
SVT_DLLPUBLIC BOOL ReadWindowMetafile( SvStream& rStream, GDIMetaFile& rMTF, FilterConfigItem* pConfigItem );
#else	// SUPD == 310
VCL_DLLPUBLIC bool ReadWindowMetafile( SvStream& rStream, GDIMetaFile& rMTF, FilterConfigItem* pConfigItem = NULL );
#endif	// SUPD == 310

#if SUPD == 310
SVT_DLLPUBLIC BOOL ConvertGDIMetaFileToWMF( const GDIMetaFile & rMTF, SvStream & rTargetStream, FilterConfigItem* pConfigItem = NULL, BOOL bPlaceable = TRUE );
#else	// SUPD == 310
VCL_DLLPUBLIC bool ConvertGDIMetaFileToWMF( const GDIMetaFile & rMTF, SvStream & rTargetStream, FilterConfigItem* pConfigItem = NULL, bool bPlaceable = true );
#endif	// SUPD == 310

#if SUPD == 310
BOOL ConvertGDIMetaFileToEMF( const GDIMetaFile & rMTF, SvStream & rTargetStream, FilterConfigItem* pConfigItem = NULL );
#else	// SUPD == 310
bool ConvertGDIMetaFileToEMF(const GDIMetaFile & rMTF, SvStream & rTargetStream);
#endif	// SUPD == 310

#if SUPD == 310
SVT_DLLPUBLIC BOOL WriteWindowMetafile( SvStream& rStream, const GDIMetaFile& rMTF );

SVT_DLLPUBLIC BOOL WriteWindowMetafileBits( SvStream& rStream, const GDIMetaFile& rMTF );
#else	// SUPD == 310
VCL_DLLPUBLIC bool WriteWindowMetafileBits( SvStream& rStream, const GDIMetaFile& rMTF );
#endif	// SUPD == 310

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
