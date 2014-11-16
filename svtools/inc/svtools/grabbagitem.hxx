/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef INCLUDED_SVL_GRABBAGITEM_HXX
#define INCLUDED_SVL_GRABBAGITEM_HXX

#include <map>

#if SUPD == 310
#include <svtools/svldllapi.h>
#else	// SUPD == 310
#include <svl/svldllapi.h>
#endif	// SUPD == 310
#include <tools/rtti.hxx>
#if SUPD == 310
#include <svtools/poolitem.hxx>
#else	// SUPD == 310
#include <svl/poolitem.hxx>
#endif	// SUPD == 310
#include <com/sun/star/uno/Any.hxx>

/// Grab bag item provides a string-any map for interim interop purposes.
class SVL_DLLPUBLIC SfxGrabBagItem : public SfxPoolItem
{
private:
    std::map<OUString, com::sun::star::uno::Any> m_aMap;

public:
#if SUPD == 310
    TYPEINFO();
#else	// SUPD == 310
    TYPEINFO_OVERRIDE();
#endif	// SUPD == 310

    SfxGrabBagItem();
    SfxGrabBagItem(sal_uInt16 nWhich, const std::map<OUString, com::sun::star::uno::Any>* pMap = 0);
    SfxGrabBagItem(const SfxGrabBagItem& rItem);
    virtual ~SfxGrabBagItem();

    const std::map<OUString, com::sun::star::uno::Any>& GetGrabBag() const;

#if SUPD == 310
    virtual int operator==(const SfxPoolItem&) const SAL_OVERRIDE;
#else	// SUPD == 310
    virtual bool operator==(const SfxPoolItem&) const SAL_OVERRIDE;
#endif	// SUPD == 310
    virtual SfxPoolItem* Clone(SfxItemPool* pPool = 0) const SAL_OVERRIDE;

#if SUPD == 310
    virtual BOOL PutValue(const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0) SAL_OVERRIDE;
    virtual BOOL QueryValue(com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0) const SAL_OVERRIDE;
#else	// SUPD == 310
    virtual bool PutValue(const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0) SAL_OVERRIDE;
    virtual bool QueryValue(com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0) const SAL_OVERRIDE;
#endif	// SUPD == 310
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
