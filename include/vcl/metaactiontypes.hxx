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
 *   Modified October 2022 by Patrick Luby. NeoOffice is only distributed
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
#ifdef USE_JAVA
    NEW_PAGE_PDF            = 10000,
    JPG_PDF                 = 10001,
    POLYLINE_PDF            = 10002,
    CREATELINK_PDF          = 10003,
    CREATEDEST_PDF          = 10004,
    SETLINKDEST_PDF         = 10005,
    SETLINKURL_PDF          = 10006,
    SETLINKPROPERTYID_PDF   = 10007,
    CREATEOUTLINEITEM_PDF   = 10008,
    SETOUTLINEITEMPARENT_PDF = 10009,
    SETOUTLINEITEMTEXT_PDF  = 10010,
    SETOUTLINEITEMDEST_PDF  = 10011,
    CREATENOTE_PDF          = 10012,
    BEGINSTRUCTUREELEMENT_PDF = 10013,
    ENDSTRUCTUREELEMENT_PDF = 10014,
    SETCURRENTSTRUCTUREELEMENT_PDF = 10015,
    SETSTRUCTUREATTRIBUTE_PDF = 10016,
    SETSTRUCTUREATTRIBUTENUMERICAL_PDF = 10017,
    SETSTRUCTUREBOUNDINGBOX_PDF = 10018,
    SETACTUALTEXT_PDF       = 10019,
    SETALTERNATETEXT_PDF    = 10020,
    SETAUTOADVANCETIME_PDF  = 10021,
    SETPAGETRANSITION_PDF   = 10022,
    CREATECONTROL_PDF       = 10023,
    DIGITLANGUAGE_PDF       = 10024,
    BEGINTRANSPARENCYGROUP_PDF = 10025,
    ENDTRANSPARENCYGROUP_PDF = 10026,
    SETLOCALE_PDF           = 10027,
    CREATENAMEDDEST_PDF     = 10028,
    ADDSTREAM_PDF           = 10029,
    REGISTERDESTREFERENCE_PDF = 10030,
    PLAYMETAFILE_PDF        = 10031,
    LAST                    = PLAYMETAFILE_PDF
#else	// USE_JAVA
    LAST                    = COMMENT
#endif	// USE_JAVA
};

#endif // INCLUDED_VCL_METAACTIONTYPES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
