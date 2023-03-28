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

#ifndef _SV_SALVD_H
#define _SV_SALVD_H

#include <premac.h>
#include <ApplicationServices/ApplicationServices.h>
#include <postmac.h>

#include "salvd.hxx"

class JavaSalGraphics;
class SalGraphics;

// ------------------------
// - JavaSalVirtualDevice -
// ------------------------

class JavaSalVirtualDevice : public SalVirtualDevice
{
	long					mnWidth;
	long					mnHeight;
	CGLayerRef				maVirDevLayer;
	JavaSalGraphics*		mpGraphics; 
	sal_Bool				mbGraphics;

public:
							JavaSalVirtualDevice();
	virtual					~JavaSalVirtualDevice();

	bool					ScreenParamsChanged();

	virtual SalGraphics*	AcquireGraphics() SAL_OVERRIDE;
	virtual void			ReleaseGraphics( SalGraphics* pGraphics ) SAL_OVERRIDE;
	virtual bool			SetSize( long nNewDX, long nNewDY ) SAL_OVERRIDE;
	virtual long			GetWidth() const SAL_OVERRIDE;
	virtual long			GetHeight() const SAL_OVERRIDE;
};

#endif // _SV_SALVD_H
