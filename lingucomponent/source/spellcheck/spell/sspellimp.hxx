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

#ifndef INCLUDED_LINGUCOMPONENT_SOURCE_SPELLCHECK_SPELL_SSPELLIMP_HXX
#define INCLUDED_LINGUCOMPONENT_SOURCE_SPELLCHECK_SPELL_SSPELLIMP_HXX

#include <cppuhelper/implbase1.hxx>
#if defined USE_JAVA && defined MACOSX
#include <cppuhelper/implbase7.hxx>
#else	// USE_JAVA && MACOSX
#include <cppuhelper/implbase6.hxx>
#endif	// USE_JAVA && MACOSX
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceDisplayName.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/PropertyValues.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/linguistic2/XSpellChecker.hpp>
#include <com/sun/star/linguistic2/XSearchableDictionaryList.hpp>
#include <com/sun/star/linguistic2/XLinguServiceEventBroadcaster.hpp>

#include <linguistic/misc.hxx>
#include <linguistic/lngprophelp.hxx>

#include <lingutil.hxx>

#if defined USE_JAVA && defined MACOSX

#include <com/sun/star/linguistic2/XProofreader.hpp>

#include <map>
#include "sspellimp_cocoa.h"
 
#endif	// USE_JAVA && MACOSX

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::linguistic2;

class Hunspell;

class SpellChecker :
#if defined USE_JAVA && defined MACOSX
    public cppu::WeakImplHelper7
#else	// USE_JAVA && MACOSX
    public cppu::WeakImplHelper6
#endif	// USE_JAVA && MACOSX
    <
        XSpellChecker,
        XLinguServiceEventBroadcaster,
        XInitialization,
        XComponent,
        XServiceInfo,
        XServiceDisplayName
#if defined USE_JAVA && defined MACOSX
        , XProofreader
#endif	// USE_JAVA && MACOSX
    >
{
    Sequence< Locale >                 aSuppLocales;
    Hunspell **                        aDicts;
    rtl_TextEncoding *                 aDEncs;
    Locale *                           aDLocs;
    OUString *                         aDNames;
    sal_Int32                          numdict;

    ::cppu::OInterfaceContainerHelper       aEvtListeners;
    linguistic::PropertyHelper_Spelling*    pPropHelper;
    bool                                    bDisposing;
#ifdef USE_JAVA
    size_t                                  mnLastNumDics;
#ifdef MACOSX
    CFArrayRef                              maLocales;
    ::std::map< OUString, CFStringRef >     maPrimaryNativeLocaleMap;
    ::std::map< OUString, CFStringRef >     maSecondaryNativeLocaleMap;
#endif	// MACOSX
#endif	// USE_JAVA

    // disallow copy-constructor and assignment-operator for now
    SpellChecker(const SpellChecker &);
    SpellChecker & operator = (const SpellChecker &);

    linguistic::PropertyHelper_Spelling&  GetPropHelper_Impl();
    linguistic::PropertyHelper_Spelling&  GetPropHelper()
    {
        return pPropHelper ? *pPropHelper : GetPropHelper_Impl();
    }

    sal_Int16   GetSpellFailure( const OUString &rWord, const Locale &rLocale );
    Reference< XSpellAlternatives > GetProposals( const OUString &rWord, const Locale &rLocale );

public:
    SpellChecker();
    virtual ~SpellChecker();

    // XSupportedLocales (for XSpellChecker)
    virtual Sequence< Locale > SAL_CALL getLocales() throw(RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL hasLocale( const Locale& rLocale ) throw(RuntimeException, std::exception) SAL_OVERRIDE;

    // XSpellChecker
    virtual sal_Bool SAL_CALL isValid( const OUString& rWord, const Locale& rLocale, const PropertyValues& rProperties ) throw(IllegalArgumentException, RuntimeException, std::exception) SAL_OVERRIDE;
    virtual Reference< XSpellAlternatives > SAL_CALL spell( const OUString& rWord, const Locale& rLocale, const PropertyValues& rProperties ) throw(IllegalArgumentException, RuntimeException, std::exception) SAL_OVERRIDE;

    // XLinguServiceEventBroadcaster
    virtual sal_Bool SAL_CALL addLinguServiceEventListener( const Reference< XLinguServiceEventListener >& rxLstnr ) throw(RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL removeLinguServiceEventListener( const Reference< XLinguServiceEventListener >& rxLstnr ) throw(RuntimeException, std::exception) SAL_OVERRIDE;

    // XServiceDisplayName
    virtual OUString SAL_CALL getServiceDisplayName( const Locale& rLocale ) throw(RuntimeException, std::exception) SAL_OVERRIDE;

    // XInitialization
    virtual void SAL_CALL initialize( const Sequence< Any >& rArguments ) throw(Exception, RuntimeException, std::exception) SAL_OVERRIDE;

    // XComponent
    virtual void SAL_CALL dispose() throw(RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL addEventListener( const Reference< XEventListener >& rxListener ) throw(RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeEventListener( const Reference< XEventListener >& rxListener ) throw(RuntimeException, std::exception) SAL_OVERRIDE;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw(RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL supportsService( const OUString& rServiceName ) throw(RuntimeException, std::exception) SAL_OVERRIDE;
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames() throw(RuntimeException, std::exception) SAL_OVERRIDE;

#if defined USE_JAVA && defined MACOSX
    // XProofreader
    virtual sal_Bool SAL_CALL isSpellChecker() throw(RuntimeException) SAL_OVERRIDE;
    virtual ProofreadingResult SAL_CALL doProofreading( const OUString& aDocumentIdentifier, const OUString& aText, const Locale& aLocale, sal_Int32 nStartOfSentencePosition, sal_Int32 nSuggestedBehindEndOfSentencePosition, const Sequence< PropertyValue >& aProperties ) throw (IllegalArgumentException, RuntimeException) SAL_OVERRIDE;
    virtual void SAL_CALL ignoreRule( const OUString& aRuleIdentifier, const Locale& aLocale ) throw (IllegalArgumentException, RuntimeException) SAL_OVERRIDE;
    virtual void SAL_CALL resetIgnoreRules() throw(RuntimeException) SAL_OVERRIDE;
#endif	// USE_JAVA && MACOSX

    static inline OUString  getImplementationName_Static() throw();
    static Sequence< OUString > getSupportedServiceNames_Static() throw();
};

inline OUString SpellChecker::getImplementationName_Static() throw()
{
    return OUString( "org.openoffice.lingu.MySpellSpellChecker" );
}

void * SAL_CALL SpellChecker_getFactory(
    char const * pImplName, css::lang::XMultiServiceFactory * pServiceManager,
    void *);

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
