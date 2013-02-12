/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Copyright 2012 LibreOffice contributors.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "wordcountctrl.hxx"
#if SUPD == 310
#include <svtools/stritem.hxx>
#else	// SUPD == 310
#include <svl/stritem.hxx>
#endif	// SUPD == 310

SFX_IMPL_STATUSBAR_CONTROL(SwWordCountStatusBarControl, SfxStringItem);

SwWordCountStatusBarControl::SwWordCountStatusBarControl(
        sal_uInt16 _nSlotId,
        sal_uInt16 _nId,
        StatusBar& rStb) :
    SfxStatusBarControl(_nSlotId, _nId, rStb)
{
}

SwWordCountStatusBarControl::~SwWordCountStatusBarControl()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
