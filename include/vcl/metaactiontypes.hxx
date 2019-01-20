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
 *   Licensed to the Apache Software Foundation (ASF), under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 * 
 *   Modified January 2019 by Patrick Luby. NeoOffice is only distributed
 *   under the GNU General Public License, Version 3 as allowed by Section 3.3
 *   of the Mozilla Public License, v. 2.0.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef INCLUDED_VCL_METAACTIONTYPES_HXX
#define INCLUDED_VCL_METAACTIONTYPES_HXX

enum class MetaActionType
{
    NONE                    = 0,
    PIXEL                   = 100,
    POINT                   = 101,
    LINE                    = 102,
    RECT                    = 103,
    ROUNDRECT               = 104,
    ELLIPSE                 = 105,
    ARC                     = 106,
    PIE                     = 107,
    CHORD                   = 108,
    POLYLINE                = 109,
    POLYGON                 = 110,
    POLYPOLYGON             = 111,
    TEXT                    = 112,
    TEXTARRAY               = 113,
    STRETCHTEXT             = 114,
    TEXTRECT                = 115,
    BMP                     = 116,
    BMPSCALE                = 117,
    BMPSCALEPART            = 118,
    BMPEX                   = 119,
    BMPEXSCALE              = 120,
    BMPEXSCALEPART          = 121,
    MASK                    = 122,
    MASKSCALE               = 123,
    MASKSCALEPART           = 124,
    GRADIENT                = 125,
    HATCH                   = 126,
    WALLPAPER               = 127,
    CLIPREGION              = 128,
    ISECTRECTCLIPREGION     = 129,
    ISECTREGIONCLIPREGION   = 130,
    MOVECLIPREGION          = 131,
    LINECOLOR               = 132,
    FILLCOLOR               = 133,
    TEXTCOLOR               = 134,
    TEXTFILLCOLOR           = 135,
    TEXTALIGN               = 136,
    MAPMODE                 = 137,
    FONT                    = 138,
    PUSH                    = 139,
    POP                     = 140,
    RASTEROP                = 141,
    Transparent             = 142,
    EPS                     = 143,
    REFPOINT                = 144,
    TEXTLINECOLOR           = 145,
    TEXTLINE                = 146,
    FLOATTRANSPARENT        = 147,
    GRADIENTEX              = 148,
    LAYOUTMODE              = 149,
    TEXTLANGUAGE            = 150,
    OVERLINECOLOR           = 151,

    COMMENT                 = 512,
#if defined USE_JAVA && defined MACOSX
    NEWPAGEPDF              = 5000,
    JPGPDF                  = 5001,
    POLYLINEPDF             = 5002,
    CREATELINKPDF           = 5003,
    CREATEDESTPDF           = 5004,
    SETLINKDESTPDF          = 5005,
    SETLINKURLPDF           = 5006,
    SETLINKPROPERTYIDPDF    = 5007,
    CREATEOUTLINEITEMPDF    = 5008,
    CREATENOTEPDF           = 5012,
    BEGINSTRUCTUREELEMENTPDF = 5013,
    ENDSTRUCTUREELEMENTPDF  = 5014,
    SETCURRENTSTRUCTUREELEMENTPDF = 5015,
    SETSTRUCTUREATTRIBUTEPDF = 5016,
    SETSTRUCTUREATTRIBUTENUMERICALPDF = 5017,
    SETSTRUCTUREBOUNDINGBOXPDF = 5018,
    SETACTUALTEXTPDF        = 5019,
    SETALTERNATETEXTPDF     = 5020,
    SETPAGETRANSITIONPDF    = 5022,
    CREATECONTROLPDF        = 5023,
    DIGITLANGUAGEPDF        = 5024,
    BEGINTRANSPARENCYGROUPPDF = 5025,
    ENDTRANSPARENCYGROUPPDF = 5026,
    SETLOCALEPDF            = 5027,
    CREATENAMEDDESTPDF      = 5028,
    ADDSTREAMPDF            = 5029,
    REGISTERDESTREFERENCEPDF = 5030,
    PLAYMETAFILEPDF         = 5031,
    BMPSCALEPDF             = 5032,
    CREATESCREENPDF         = 5033,
    SETSCREENSTREAMPDF      = 5034,
    LAST                    = BMPSCALEPDF
#else	// USE_JAVA && MACOSX
    LAST                    = COMMENT
#endif	// USE_JAVA && MACOSX
};

#endif // INCLUDED_VCL_METAACTIONTYPES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
