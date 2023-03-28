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

#ifndef _SV_SALOBJ_H
#define _SV_SALOBJ_H

#include <vcl/sysdata.hxx>

#include "salobj.hxx"

class JavaSalFrame;
class SalFrame;

// -----------------
// - JavaSalObject -
// -----------------

class JavaSalObject : public SalObject
{
	Size					maSize;
	id						mpChildView;
	Rectangle				maClipRect;
	sal_Bool				mbInFlush;
	JavaSalFrame*			mpParent;
	SystemEnvData			maSysData;
	bool					mbVisible;

public:
							JavaSalObject( SalFrame *pParent );
	virtual					~JavaSalObject();

	void					Destroy();
	void					Flush();

	virtual void			ResetClipRegion() SAL_OVERRIDE;
	virtual sal_uInt16		GetClipRegionType() SAL_OVERRIDE;
	virtual void			BeginSetClipRegion( sal_uLong nRects ) SAL_OVERRIDE;
	virtual void			UnionClipRegion( long nX, long nY, long nWidth, long nHeight ) SAL_OVERRIDE;
	virtual void			EndSetClipRegion() SAL_OVERRIDE;
	virtual void			SetPosSize( long nX, long nY, long nWidth, long nHeight ) SAL_OVERRIDE;
	virtual void			Show( bool bVisible ) SAL_OVERRIDE;
	virtual void			SetBackground() SAL_OVERRIDE;
	virtual void			SetBackground( SalColor nSalColor ) SAL_OVERRIDE;
	virtual const SystemEnvData*	GetSystemData() const SAL_OVERRIDE;
};

#endif // _SV_SALOBJ_H
