/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#if SUPD == 310
#include <svtools/grabbagitem.hxx>
#include <svtools/poolitem.hxx>
#else	// SUPD == 310
#include <svl/grabbagitem.hxx>
#include <svl/poolitem.hxx>
#endif	// SUPD == 310
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <comphelper/sequence.hxx>

#include <com/sun/star/beans/PropertyValue.hpp>

#if SUPD == 310
#include <sal/log.hxx>
#endif	// SUPD == 310

TYPEINIT1_AUTOFACTORY(SfxGrabBagItem, SfxPoolItem);

using namespace com::sun::star;

SfxGrabBagItem::SfxGrabBagItem()
{
}

SfxGrabBagItem::SfxGrabBagItem(sal_uInt16 nWhich, const std::map<OUString, uno::Any>* pMap) :
    SfxPoolItem(nWhich)
{
    if (pMap)
        m_aMap = *pMap;
}

SfxGrabBagItem::SfxGrabBagItem(const SfxGrabBagItem& rItem) :
    SfxPoolItem(rItem),
    m_aMap(rItem.m_aMap)
{
}

SfxGrabBagItem::~SfxGrabBagItem()
{
}

const std::map<OUString, uno::Any>& SfxGrabBagItem::GetGrabBag() const
{
    return m_aMap;
}

#if SUPD == 310
int SfxGrabBagItem::operator==(const SfxPoolItem& rItem) const
#else	// SUPD == 310
bool SfxGrabBagItem::operator==(const SfxPoolItem& rItem) const
#endif	// SUPD == 310
{
    SfxGrabBagItem* pItem = (SfxGrabBagItem*)&rItem;

    return m_aMap == pItem->m_aMap;
}

SfxPoolItem* SfxGrabBagItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new SfxGrabBagItem(*this);
}

#if SUPD == 310
BOOL SfxGrabBagItem::PutValue(const uno::Any& rVal, sal_uInt8 /*nMemberId*/)
#else	// SUPD == 310
bool SfxGrabBagItem::PutValue(const uno::Any& rVal, sal_uInt8 /*nMemberId*/)
#endif	// SUPD == 310
{
    uno::Sequence<beans::PropertyValue> aValue;
    if (rVal >>= aValue)
    {
        m_aMap.clear();
        comphelper::OSequenceIterator<beans::PropertyValue> i(aValue);
        while (i.hasMoreElements())
        {
            beans::PropertyValue aPropertyValue = i.nextElement().get<beans::PropertyValue>();
            m_aMap[aPropertyValue.Name] = aPropertyValue.Value;
        }
        return true;
    }

    SAL_WARN("svl", "SfxGrabBagItem::PutValue: wrong type");
    return false;
}

#if SUPD == 310
BOOL SfxGrabBagItem::QueryValue(uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
#else	// SUPD == 310
bool SfxGrabBagItem::QueryValue(uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
#endif	// SUPD == 310
{
    uno::Sequence<beans::PropertyValue> aValue(m_aMap.size());
    beans::PropertyValue* pValue = aValue.getArray();
    for (std::map<OUString, com::sun::star::uno::Any>::const_iterator i = m_aMap.begin(); i != m_aMap.end(); ++i)
    {
        pValue[0].Name = i->first;
        pValue[0].Value = i->second;
        ++pValue;
    }
    rVal = uno::makeAny(aValue);
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
