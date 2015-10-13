/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile$
 * $Revision$
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
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
 *
 ************************************************************************/

#ifndef _AUTHRATR_HXX
#define _AUTHRATR_HXX

#include <tools/solar.h>
#include <tools/color.hxx>
#include "swdllapi.h"

#define COL_NONE		TRGB_COLORDATA( 0x80, 0xFF, 0xFF, 0xFF )

class SW_DLLPUBLIC AuthorCharAttr
{
public:
	USHORT	nItemId;
	USHORT	nAttr;
#if SUPD == 310
	ColorData	nColor;
#else	// SUPD == 310
	ULONG	nColor;
#endif	// SUPD == 310

	AuthorCharAttr();

	inline BOOL operator == ( const AuthorCharAttr& rAttr ) const
	{
		return	nItemId == rAttr.nItemId && nAttr == rAttr.nAttr &&
				nColor == rAttr.nColor;
	}
};


#endif
