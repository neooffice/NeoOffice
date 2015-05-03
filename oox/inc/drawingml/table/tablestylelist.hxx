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

#ifndef INCLUDED_OOX_DRAWINGML_TABLE_TABLESTYLELIST_HXX
#define INCLUDED_OOX_DRAWINGML_TABLE_TABLESTYLELIST_HXX

#include <rtl/ustring.hxx>
#include <boost/shared_ptr.hpp>
#include <vector>

#if SUPD == 310
#include <oox/dllapi.h>
#endif	// SUPD == 310

namespace oox { namespace drawingml { namespace table {

class TableStyle;

class TableStyleList
{
public:

    TableStyleList();
    ~TableStyleList();

    OUString&              getDefaultStyleId() { return maDefaultStyleId; };
    std::vector< TableStyle >&  getTableStyles(){ return maTableStyles; };

private:

    OUString               maDefaultStyleId;
    std::vector< TableStyle >   maTableStyles;

};

typedef boost::shared_ptr< TableStyleList > TableStyleListPtr;

} } }

#endif // INCLUDED_OOX_DRAWINGML_TABLE_TABLESTYLELIST_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
