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

#ifndef INCLUDED_SW_SOURCE_CORE_ACCESS_ACCFRAMEBASE_HXX
#define INCLUDED_SW_SOURCE_CORE_ACCESS_ACCFRAMEBASE_HXX

#include <acccontext.hxx>
#include <calbck.hxx>
#include <pam.hxx>

class SwFlyFrm;

class SwAccessibleFrameBase : public SwAccessibleContext,
                              public SwClient
{
    bool    bIsSelected;    // protected by base class mutex
    bool    IsSelected();

protected:
    // Set states for getAccessibleStateSet.
    // This derived class additionally sets SELECTABLE(1), SELECTED(+),
    // FOCUSABLE(1) and FOCUSED(+)
    virtual void GetStates( ::utl::AccessibleStateSetHelper& rStateSet ) SAL_OVERRIDE;
    SwFlyFrm* getFlyFrm() const;
    bool GetSelectedState( );
    SwPaM* GetCrsr();

    virtual void _InvalidateCursorPos() SAL_OVERRIDE;
    virtual void _InvalidateFocus() SAL_OVERRIDE;

    virtual ~SwAccessibleFrameBase();
    virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew) SAL_OVERRIDE;

public:
#ifdef NO_LIBO_BUG_58624_FIX
    SwAccessibleFrameBase( SwAccessibleMap* pInitMap,
#else	// NO_LIBO_BUG_58624_FIX
    SwAccessibleFrameBase(std::shared_ptr<SwAccessibleMap> const& pInitMap,
#endif	// NO_LIBO_BUG_58624_FIX
                           sal_Int16 nInitRole,
                           const SwFlyFrm *pFlyFrm );

    virtual bool HasCursor() SAL_OVERRIDE;   // required by map to remember that object

    static sal_uInt8 GetNodeType( const SwFlyFrm *pFlyFrm );

    // The object is not visible an longer and should be destroyed
#ifdef NO_LIBO_BUG_58624_FIX
    virtual void Dispose( bool bRecursive = false ) SAL_OVERRIDE;
#else	// NO_LIBO_BUG_58624_FIX
    virtual void Dispose(bool bRecursive = false, bool bCanSkipInvisible = true) override;
#endif	// NO_LIBO_BUG_58624_FIX
    virtual bool SetSelectedState( bool bSeleted ) SAL_OVERRIDE;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
