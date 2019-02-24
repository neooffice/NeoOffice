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
 * 
 *   Modified November 2016 by Patrick Luby. NeoOffice is only distributed
 *   under the GNU General Public License, Version 3 as allowed by Section 3.3
 *   of the Mozilla Public License, v. 2.0.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/linguistic2/XSearchableDictionaryList.hpp>

#include <com/sun/star/linguistic2/SpellFailure.hpp>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <tools/debug.hxx>
#include <osl/mutex.hxx>
#include <com/sun/star/ucb/XSimpleFileAccess.hpp>

#include <lingutil.hxx>
#include <hunspell.hxx>
#include <sspellimp.hxx>

#include <linguistic/lngprops.hxx>
#include <linguistic/spelldta.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <unotools/pathoptions.hxx>
#include <unotools/lingucfg.hxx>
#include <unotools/useroptions.hxx>
#include <osl/file.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/textenc.h>
#include <sal/log.hxx>

#include <list>
#include <set>
#include <string.h>

using namespace utl;
using namespace osl;
using namespace com::sun::star;
using namespace com::sun::star::beans;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace com::sun::star::linguistic2;
using namespace linguistic;

// XML-header of SPELLML queries
#if !defined SPELL_XML
#define SPELL_XML "<?xml?>"
#endif

// only available in hunspell >= 1.5
#if !defined MAXWORDLEN
#define MAXWORDLEN 176
#endif

#if defined USE_JAVA && defined MACOSX

static OUString ImplGetLocaleString( Locale aLocale )
{
    OUString aLocaleString( aLocale.Language );
    if ( aLocaleString.getLength() && aLocale.Country.getLength() )
    {
        aLocaleString += "_" + aLocale.Country;
        if ( aLocale.Variant.getLength() )
            aLocaleString += "_" + aLocale.Variant; 
    }
    return aLocaleString;
}

#endif // USE_JAVA && MACOSX

SpellChecker::SpellChecker() :
    m_aDicts(nullptr),
    m_aDEncs(nullptr),
    m_aDLocs(nullptr),
    m_aDNames(nullptr),
    m_nNumDict(0),
    m_aEvtListeners(GetLinguMutex()),
    m_pPropHelper(nullptr),
    m_bDisposing(false)
#ifdef USE_JAVA
    , m_nLastNumDicts(0)
#ifdef MACOSX
    , m_aLocales(nullptr)
#endif	// MACOSX
#endif	// USE_JAVA
{
}

SpellChecker::~SpellChecker()
{
#if defined USE_JAVA && defined MACOSX
    if ( m_aLocales )
        CFRelease( m_aLocales );
#endif	// USE_JAVA && MACOSX

    if (m_aDicts)
    {
       for (int i = 0; i < m_nNumDict; ++i)
       {
            delete m_aDicts[i];
       }
       delete[] m_aDicts;
    }
    delete[] m_aDEncs;
    delete[] m_aDLocs;
    delete[] m_aDNames;
    if (m_pPropHelper)
    {
        m_pPropHelper->RemoveAsPropListener();
        delete m_pPropHelper;
    }
}

PropertyHelper_Spelling & SpellChecker::GetPropHelper_Impl()
{
    if (!m_pPropHelper)
    {
        Reference< XLinguProperties >   xPropSet( GetLinguProperties(), UNO_QUERY );

        m_pPropHelper = new PropertyHelper_Spelling( static_cast<XSpellChecker *>(this), xPropSet );
        m_pPropHelper->AddAsPropListener();   //! after a reference is established
    }
    return *m_pPropHelper;
}

Sequence< Locale > SAL_CALL SpellChecker::getLocales()
{
    MutexGuard  aGuard( GetLinguMutex() );

    // this routine should return the locales supported by the installed
    // dictionaries.
#ifndef USE_JAVA
    if (!m_nNumDict)
    {
#endif	// !USE_JAVA
        SvtLinguConfig aLinguCfg;

        // get list of extension dictionaries-to-use
        // (or better speaking: the list of dictionaries using the
        // new configuration entries).
        std::list< SvtLinguConfigDictionaryEntry > aDics;
        uno::Sequence< OUString > aFormatList;
        aLinguCfg.GetSupportedDictionaryFormatsFor( "SpellCheckers",
                "org.openoffice.lingu.MySpellSpellChecker", aFormatList );
        sal_Int32 nLen = aFormatList.getLength();
        for (sal_Int32 i = 0;  i < nLen;  ++i)
        {
            std::vector< SvtLinguConfigDictionaryEntry > aTmpDic(
                    aLinguCfg.GetActiveDictionariesByFormat( aFormatList[i] ) );
            aDics.insert( aDics.end(), aTmpDic.begin(), aTmpDic.end() );
        }

        //!! for compatibility with old dictionaries (the ones not using extensions
        //!! or new configuration entries, but still using the dictionary.lst file)
        //!! Get the list of old style spell checking dictionaries to use...
        std::vector< SvtLinguConfigDictionaryEntry > aOldStyleDics(
                GetOldStyleDics( "DICT" ) );

        // to prefer dictionaries with configuration entries we will only
        // use those old style dictionaries that add a language that
        // is not yet supported by the list od new style dictionaries
        MergeNewStyleDicsAndOldStyleDics( aDics, aOldStyleDics );

#ifdef USE_JAVA
        // Update list of locales when the user installs or removes any
        // dictionaries
#ifdef MACOSX
        if (m_nLastNumDicts != aDics.size() || !m_aLocales)
#else	// MACOSX
        if (m_nLastNumDicts != aDics.size())
#endif	// MACOSX
        {
            m_nLastNumDicts = aDics.size();
#endif	// USE_JAVA
        if (!aDics.empty())
        {
            uno::Reference< lang::XMultiServiceFactory > xServiceFactory(comphelper::getProcessServiceFactory());
            uno::Reference< ucb::XSimpleFileAccess > xAccess(xServiceFactory->createInstance("com.sun.star.ucb.SimpleFileAccess"), uno::UNO_QUERY);
            // get supported locales from the dictionaries-to-use...
            sal_Int32 k = 0;
            std::set<OUString> aLocaleNamesSet;
            std::list< SvtLinguConfigDictionaryEntry >::const_iterator aDictIt;
            for (aDictIt = aDics.begin();  aDictIt != aDics.end();  ++aDictIt)
            {
                uno::Sequence< OUString > aLocaleNames( aDictIt->aLocaleNames );
                uno::Sequence< OUString > aLocations( aDictIt->aLocations );
                SAL_WARN_IF(
                    aLocaleNames.hasElements() && !aLocations.hasElements(),
                    "lingucomponent", "no locations");
                if (aLocations.hasElements())
                {
                    if (xAccess.is() && xAccess->exists(aLocations[0]))
                    {
                        sal_Int32 nLen2 = aLocaleNames.getLength();
                        for (k = 0;  k < nLen2;  ++k)
                        {
                            aLocaleNamesSet.insert( aLocaleNames[k] );
                        }
                    }
                    else
                    {
                        SAL_WARN(
                            "lingucomponent",
                            "missing <" << aLocations[0] << ">");
                    }
                }
            }
            // ... and add them to the resulting sequence
            m_aSuppLocales.realloc( aLocaleNamesSet.size() );
            std::set<OUString>::const_iterator aItB;
            k = 0;
            for (aItB = aLocaleNamesSet.begin();  aItB != aLocaleNamesSet.end();  ++aItB)
            {
                Locale aTmp( LanguageTag::convertToLocale( *aItB ));
                m_aSuppLocales[k++] = aTmp;
            }

            //! For each dictionary and each locale we need a separate entry.
            //! If this results in more than one dictionary per locale than (for now)
            //! it is undefined which dictionary gets used.
            //! In the future the implementation should support using several dictionaries
            //! for one locale.
            m_nNumDict = 0;
            for (aDictIt = aDics.begin();  aDictIt != aDics.end();  ++aDictIt)
                m_nNumDict = m_nNumDict + aDictIt->aLocaleNames.getLength();

            // add dictionary information
            m_aDicts  = new Hunspell* [m_nNumDict];
            m_aDEncs  = new rtl_TextEncoding [m_nNumDict];
            m_aDLocs  = new Locale [m_nNumDict];
            m_aDNames = new OUString [m_nNumDict];
            k = 0;
            for (aDictIt = aDics.begin();  aDictIt != aDics.end();  ++aDictIt)
            {
                if (aDictIt->aLocaleNames.getLength() > 0 &&
                    aDictIt->aLocations.getLength() > 0)
                {
                    uno::Sequence< OUString > aLocaleNames( aDictIt->aLocaleNames );
                    sal_Int32 nLocales = aLocaleNames.getLength();

                    // currently only one language per dictionary is supported in the actual implementation...
                    // Thus here we work-around this by adding the same dictionary several times.
                    // Once for each of its supported locales.
                    for (sal_Int32 i = 0;  i < nLocales;  ++i)
                    {
                        m_aDicts[k]  = nullptr;
                        m_aDEncs[k]  = RTL_TEXTENCODING_DONTKNOW;
                        m_aDLocs[k]  = LanguageTag::convertToLocale( aLocaleNames[i] );
                        // also both files have to be in the same directory and the
                        // file names must only differ in the extension (.aff/.dic).
                        // Thus we use the first location only and strip the extension part.
                        OUString aLocation = aDictIt->aLocations[0];
                        sal_Int32 nPos = aLocation.lastIndexOf( '.' );
                        aLocation = aLocation.copy( 0, nPos );
                        m_aDNames[k] = aLocation;

                        ++k;
                    }
                }
            }
            DBG_ASSERT( k == m_nNumDict, "index mismatch?" );
        }
        else
        {
            // no dictionary found so register no dictionaries
            m_nNumDict = 0;
            delete[] m_aDicts;
            m_aDicts  = nullptr;
            delete[] m_aDEncs;
            m_aDEncs = nullptr;
            delete[] m_aDLocs;
            m_aDLocs  = nullptr;
            delete[] m_aDNames;
            m_aDNames = nullptr;
            m_aSuppLocales.realloc(0);
        }

#if defined USE_JAVA && defined MACOSX
        ::std::list< Locale > aAppLocalesList;
        CFMutableArrayRef aAppLocales = CFArrayCreateMutable( kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks );
        if ( aAppLocales )
        {
            for ( sal_uInt16 nLang = 0x0; nLang < 0xffff; nLang++ )
            {
                Locale aLocale = LanguageTag::convertToLocale( LanguageType( nLang ), false );
                OUString aLocaleString( ImplGetLocaleString( aLocale ) );
                if ( aLocaleString.getLength() )
                {
                    CFStringRef aString = CFStringCreateWithCharactersNoCopy( nullptr, reinterpret_cast< const UniChar* >( aLocaleString.getStr() ), aLocaleString.getLength(), kCFAllocatorNull );
                    if ( aString )
                    {
                        CFArrayAppendValue( aAppLocales, aString );
                        aAppLocalesList.push_back( aLocale );
                        CFRelease( aString );
                    }

                    if ( aLocale.Variant.getLength() )
                    {
                        Locale aTmpLocale( aLocale.Language, aLocale.Country, OUString() );
                        OUString aTmpLocaleString( ImplGetLocaleString( aTmpLocale ) );
                        if ( aTmpLocaleString.getLength() )
                        {
                            CFStringRef aTmpString = CFStringCreateWithCharactersNoCopy( nullptr, reinterpret_cast< const UniChar* >( aTmpLocaleString.getStr() ), aTmpLocaleString.getLength(), kCFAllocatorNull );
                            if ( aTmpString )
                            {
                                CFArrayAppendValue( aAppLocales, aTmpString );
                                CFRelease( aTmpString );
                            }
                        }
                    }

                    if ( aLocale.Country.getLength() )
                    {
                        Locale aTmpLocale( aLocale.Language, OUString(), OUString() );
                        OUString aTmpLocaleString( ImplGetLocaleString( aTmpLocale ) );
                        if ( aTmpLocaleString.getLength() )
                        {
                            CFStringRef aTmpString = CFStringCreateWithCharactersNoCopy( nullptr, reinterpret_cast< const UniChar* >( aTmpLocaleString.getStr() ), aTmpLocaleString.getLength(), kCFAllocatorNull );
                            if ( aTmpString )
                            {
                                CFArrayAppendValue( aAppLocales, aTmpString );
                                CFRelease( aTmpString );
                            }
                        }
                    }
                }
            }
        }

        if ( m_aLocales )
        {
            CFRelease( m_aLocales );
            m_aLocales = nullptr;
        }
        m_aPrimaryNativeLocaleMap.clear();
        m_aSecondaryNativeLocaleMap.clear();

        m_aLocales = NSSpellChecker_getLocales( aAppLocales );
        if ( m_aLocales )
        {
            sal_Int32 nStart = m_aSuppLocales.getLength();
            CFIndex nItems = CFArrayGetCount( m_aLocales );
            m_aSuppLocales.realloc( nStart + nItems );
            Locale *pLocaleArray = m_aSuppLocales.getArray();

            CFIndex nItemsAdded = nStart;
            for ( CFIndex i = 0; i < nItems; i++ )
            {
                CFStringRef aString = static_cast< CFStringRef >( CFArrayGetValueAtIndex( m_aLocales, i ) );
                if ( aString )
                {
                    CFIndex nStringLen = CFStringGetLength( aString );
                    CFRange aStringRange = CFRangeMake( 0, nStringLen );
                    sal_Unicode pStringBuffer[ nStringLen + 1 ];
                    CFStringGetCharacters( aString, aStringRange, reinterpret_cast< UniChar* >( pStringBuffer ) );
                    pStringBuffer[ nStringLen ] = 0;
                    OUString aItem( pStringBuffer );

                    if ( !aItem.getLength() )
                        continue;

                    sal_Unicode cDelimiter = (sal_Unicode)'_';
                    sal_Int32 nIndex = 0;
                    OUString aLang = aItem.getToken( 0, cDelimiter, nIndex );
                    if ( nIndex < 0 )
                    {
                        // Mac OS X will sometimes use "-" as its delimiter
                        cDelimiter = (sal_Unicode)'-';
                        nIndex = 0;
                        aLang = aItem.getToken( 0, cDelimiter, nIndex );
                    }

                    OUString aCountry;
                    if ( nIndex >= 0 )
                        aCountry = aItem.getToken( 0, cDelimiter, nIndex );

                    OUString aVariant;
                    if ( nIndex >= 0 )
                        aVariant = aItem.getToken( 0, cDelimiter, nIndex );
    
                    // TODO: Handle the cases where the country is "Hans" or
                    // "Hant". Since these are only used with the "zh" locale
                    // and Chinese is not likely to have spellchecking support
                    // anytime soon as it is an ideographic language, we may
                    // never need to worry about this case.
                    Locale aLocale( aLang, aCountry, aVariant );
                    OUString aLocaleString( ImplGetLocaleString( aLocale ) );
                    if ( !aLocaleString.getLength() )
                        continue;

                    ::std::map< OUString, CFStringRef >::const_iterator it = m_aPrimaryNativeLocaleMap.find( aLocaleString );
                    if ( it == m_aPrimaryNativeLocaleMap.end() )
                    {
                        m_aPrimaryNativeLocaleMap[ aLocaleString ] = aString;

                        // Fix bug 2532 by checking for approximate and
                        // duplicate matches in the native locales
                        bool bAddLocale = true;
                        for ( CFIndex j = 0; j < nItemsAdded; j++ )
                        {
                            if ( aLocale == pLocaleArray[ j ] )
                            {
                                bAddLocale = false;
                                break;
                            }

                            if ( pLocaleArray[ j ].Variant.getLength() )
                            {
                                Locale aTmpLocale( pLocaleArray[ j ].Language, pLocaleArray[ j ].Country, OUString() );
                                if ( aLocale == aTmpLocale )
                                {
                                    OUString aTmpLocaleString( ImplGetLocaleString( aTmpLocale ) );
                                    if ( aTmpLocaleString.getLength() )
                                        m_aSecondaryNativeLocaleMap[ aTmpLocaleString ] = aString;
                                }
                            }

                            if ( pLocaleArray[ j ].Country.getLength() )
                            {
                                Locale aTmpLocale( pLocaleArray[ j ].Language, OUString(), OUString() );
                                if ( aLocale == aTmpLocale )
                                {
                                    OUString aTmpLocaleString( ImplGetLocaleString( aTmpLocale ) );
                                    if ( aTmpLocaleString.getLength() )
                                        m_aSecondaryNativeLocaleMap[ aTmpLocaleString ] = aString;
                                }
                            }
                        }

                       if ( bAddLocale )
                            pLocaleArray[ nItemsAdded++ ] = aLocale;
                    }
                }
            }

            m_aSuppLocales.realloc( nItemsAdded );
        }

        // Fix 2686 by finding partial matches to application locales
        sal_Int32 nStart = m_aSuppLocales.getLength();
        m_aSuppLocales.realloc( nStart + aAppLocalesList.size() );
        CFIndex nItemsAdded = nStart;
        Locale *pLocaleArray = m_aSuppLocales.getArray();
        for ( ::std::list< Locale >::const_iterator it = aAppLocalesList.begin(); it != aAppLocalesList.end(); ++it )
        {
            OUString aLocaleString( ImplGetLocaleString( *it ) );
            if ( !aLocaleString.getLength() )
                continue;

            bool bAddLocale = false;
            for ( CFIndex j = 0; j < nItemsAdded; j++ )
            {
                if ( pLocaleArray[ j ] == *it )
                    break;

                if ( (*it).Variant.getLength() )
                {
                    Locale aTmpLocale( (*it).Language, (*it).Variant, OUString() );
                    if ( pLocaleArray[ j ] == aTmpLocale )
                    {
                        bool bFound = false;
                        OUString aTmpLocaleString( ImplGetLocaleString( aTmpLocale ) );
                        ::std::map< OUString, CFStringRef >::const_iterator nlit = m_aPrimaryNativeLocaleMap.find( aTmpLocaleString );
                        if ( nlit != m_aPrimaryNativeLocaleMap.end() )
                        {
                            bFound = true;
                        }
                        else
                        {
                            nlit = m_aSecondaryNativeLocaleMap.find( aTmpLocaleString );
                            if ( nlit != m_aSecondaryNativeLocaleMap.end() )
                                bFound = true;
                        }

                        if ( bFound )
                        {
                            m_aSecondaryNativeLocaleMap[ aLocaleString ] = nlit->second;
                            bAddLocale = true;
                            break;
                        }
                    }
                }

                if ( (*it).Country.getLength() )
                {
                    Locale aTmpLocale( (*it).Language, OUString(), OUString() );
                    if ( pLocaleArray[ j ] == aTmpLocale )
                    {
                        bool bFound = false;
                        OUString aTmpLocaleString( ImplGetLocaleString( aTmpLocale ) );
                        ::std::map< OUString, CFStringRef >::const_iterator nlit = m_aPrimaryNativeLocaleMap.find( aTmpLocaleString );
                        if ( nlit != m_aPrimaryNativeLocaleMap.end() )
                        {
                            bFound = true;
                        }
                        else
                        {
                            nlit = m_aSecondaryNativeLocaleMap.find( aTmpLocaleString );
                            if ( nlit != m_aSecondaryNativeLocaleMap.end() )
                                bFound = true;
                        }

                        if ( bFound )
                        {
                            m_aSecondaryNativeLocaleMap[ aLocaleString ] = nlit->second;
                            bAddLocale = true;
                            break;
                        }
                    }
                }
            }

            if ( bAddLocale )
                pLocaleArray[ nItemsAdded++ ] = *it;
        }

        m_aSuppLocales.realloc( nItemsAdded );

        if ( aAppLocales )
            CFRelease( aAppLocales );
#endif	// USE_JAVA && MACOSX
    }

    return m_aSuppLocales;
}

sal_Bool SAL_CALL SpellChecker::hasLocale(const Locale& rLocale)
{
    MutexGuard  aGuard( GetLinguMutex() );

    bool bRes = false;
    if (!m_aSuppLocales.getLength())
        getLocales();

    const Locale *pLocale = m_aSuppLocales.getConstArray();
    sal_Int32 nLen = m_aSuppLocales.getLength();
#if defined USE_JAVA && defined MACOSX
    // Check native locales first 
    OUString aLocaleString( ImplGetLocaleString( rLocale ) );
    if ( aLocaleString.getLength() )
    {
        ::std::map< OUString, CFStringRef >::const_iterator it = m_aPrimaryNativeLocaleMap.find( aLocaleString );
        if ( it != m_aPrimaryNativeLocaleMap.end() )
            return sal_True;

        it = m_aSecondaryNativeLocaleMap.find( aLocaleString );
        if ( it != m_aSecondaryNativeLocaleMap.end() )
            return sal_True;

        // Fix bug 2513 by checking for approximate matches in the native locales
        if ( rLocale.Variant.getLength() )
        {
            bool bFound = false;
            Locale aTmpLocale( rLocale.Language, rLocale.Country, OUString() );
            OUString aTmpLocaleString = ImplGetLocaleString( aTmpLocale );
            it = m_aPrimaryNativeLocaleMap.find( aTmpLocaleString );
            if ( it != m_aPrimaryNativeLocaleMap.end() )
            {
                bFound = true;
            }
            else
            {
                it = m_aSecondaryNativeLocaleMap.find( aTmpLocaleString );
                if ( it != m_aSecondaryNativeLocaleMap.end() )
                    bFound = true;
            }

            if ( bFound )
            {
                m_aSuppLocales.realloc( ++nLen );
                Locale *pLocaleArray = m_aSuppLocales.getArray();
                pLocaleArray[ nLen - 1 ] = rLocale;
                m_aSecondaryNativeLocaleMap[ aLocaleString ] = it->second;
                return sal_True;
            }
        }

        if ( rLocale.Country.getLength() )
        {
            bool bFound = false;
            Locale aTmpLocale( rLocale.Language, OUString(), OUString() );
            OUString aTmpLocaleString = ImplGetLocaleString( aTmpLocale );
            it = m_aPrimaryNativeLocaleMap.find( aTmpLocaleString );
            if ( it != m_aPrimaryNativeLocaleMap.end() )
            {
                bFound = true;
            }
            else
            {
                it = m_aSecondaryNativeLocaleMap.find( aTmpLocaleString );
                if ( it != m_aSecondaryNativeLocaleMap.end() )
                    bFound = true;
            }

            if ( bFound )
            {
                m_aSuppLocales.realloc( ++nLen );
                Locale *pLocaleArray = m_aSuppLocales.getArray();
                pLocaleArray[ nLen - 1 ] = rLocale;
                m_aSecondaryNativeLocaleMap[ aLocaleString ] = it->second;
                return sal_True;
            }
        }
    }
#endif	// USE_JAVA && MACOSX

    for (sal_Int32 i = 0;  i < nLen;  ++i)
    {
        if (rLocale == pLocale[i])
        {
            bRes = true;
            break;
        }
    }
    return bRes;
}

sal_Int16 SpellChecker::GetSpellFailure(const OUString &rWord, const Locale &rLocale)
{
    if (rWord.getLength() > MAXWORDLEN)
        return -1;

    Hunspell * pMS = nullptr;
    rtl_TextEncoding eEnc = RTL_TEXTENCODING_DONTKNOW;

    // initialize a myspell object for each dictionary once
    // (note: mutex is held higher up in isValid)

    sal_Int16 nRes = -1;

    // first handle smart quotes both single and double
    OUStringBuffer rBuf(rWord);
    sal_Int32 n = rBuf.getLength();
    sal_Unicode c;
    sal_Int32 extrachar = 0;

    for (sal_Int32 ix=0; ix < n; ix++)
    {
        c = rBuf[ix];
        if ((c == 0x201C) || (c == 0x201D))
            rBuf[ix] = u'"';
        else if ((c == 0x2018) || (c == 0x2019))
            rBuf[ix] = u'\'';

        // recognize words with Unicode ligatures and ZWNJ/ZWJ characters (only
        // with 8-bit encoded dictionaries. For UTF-8 encoded dictionaries
        // set ICONV and IGNORE aff file options, if needed.)
        else if ((c == 0x200C) || (c == 0x200D) ||
            ((c >= 0xFB00) && (c <= 0xFB04)))
                extrachar = 1;
    }
    OUString nWord(rBuf.makeStringAndClear());

    if (n)
    {
#if defined USE_JAVA && defined MACOSX
        bool bHandled = false;
        bool bFound = false;
        OUString aLocaleString( ImplGetLocaleString( rLocale ) );
        ::std::map< OUString, CFStringRef >::const_iterator it = m_aPrimaryNativeLocaleMap.find( aLocaleString );
        if ( it != m_aPrimaryNativeLocaleMap.end() )
        {
            bFound = true;
        }
        else
        {
#endif	// USE_JAVA && MACOSX
        for (sal_Int32 i = 0; i < m_nNumDict; ++i)
        {
            pMS = nullptr;
            eEnc = RTL_TEXTENCODING_DONTKNOW;

            if (rLocale == m_aDLocs[i])
            {
                if (!m_aDicts[i])
                {
                    OUString dicpath = m_aDNames[i] + ".dic";
                    OUString affpath = m_aDNames[i] + ".aff";
                    OUString dict;
                    OUString aff;
                    osl::FileBase::getSystemPathFromFileURL(dicpath,dict);
                    osl::FileBase::getSystemPathFromFileURL(affpath,aff);
#if defined(_WIN32)
                    // workaround for Windows specific problem that the
                    // path length in calls to 'fopen' is limited to somewhat
                    // about 120+ characters which will usually be exceed when
                    // using dictionaries as extensions. (Hunspell waits UTF-8 encoded
                    // path with \\?\ long path prefix.)
                    OString aTmpaff = Win_AddLongPathPrefix(OUStringToOString(aff, RTL_TEXTENCODING_UTF8));
                    OString aTmpdict = Win_AddLongPathPrefix(OUStringToOString(dict, RTL_TEXTENCODING_UTF8));
#else
                    OString aTmpaff(OU2ENC(aff,osl_getThreadTextEncoding()));
                    OString aTmpdict(OU2ENC(dict,osl_getThreadTextEncoding()));
#endif

                    m_aDicts[i] = new Hunspell(aTmpaff.getStr(),aTmpdict.getStr());
#if defined(H_DEPRECATED)
                    m_aDEncs[i] = getTextEncodingFromCharset(m_aDicts[i]->get_dict_encoding().c_str());
#else
                    m_aDEncs[i] = getTextEncodingFromCharset(m_aDicts[i]->get_dic_encoding());
#endif
                }
                pMS = m_aDicts[i];
                eEnc = m_aDEncs[i];
            }

            if (pMS)
            {
                // we don't want to work with a default text encoding since following incorrect
                // results may occur only for specific text and thus may be hard to notice.
                // Thus better always make a clean exit here if the text encoding is in question.
                // Hopefully something not working at all will raise proper attention quickly. ;-)
                DBG_ASSERT( eEnc != RTL_TEXTENCODING_DONTKNOW, "failed to get text encoding! (maybe incorrect encoding string in file)" );
                if (eEnc == RTL_TEXTENCODING_DONTKNOW)
                    return -1;

#if defined USE_JAVA && defined MACOSX
                bHandled = true;
#endif	// USE_JAVA && MACOSX
                OString aWrd(OU2ENC(nWord,eEnc));
#if defined(H_DEPRECATED)
                bool bVal = pMS->spell(std::string(aWrd.getStr()));
#else
                bool bVal = pMS->spell(aWrd.getStr()) != 0;
#endif
                if (!bVal) {
                    if (extrachar && (eEnc != RTL_TEXTENCODING_UTF8)) {
                        OUStringBuffer aBuf(nWord);
                        n = aBuf.getLength();
                        for (sal_Int32 ix=n-1; ix >= 0; ix--)
                        {
                          switch (aBuf[ix]) {
                            case 0xFB00: aBuf.remove(ix, 1); aBuf.insert(ix, "ff"); break;
                            case 0xFB01: aBuf.remove(ix, 1); aBuf.insert(ix, "fi"); break;
                            case 0xFB02: aBuf.remove(ix, 1); aBuf.insert(ix, "fl"); break;
                            case 0xFB03: aBuf.remove(ix, 1); aBuf.insert(ix, "ffi"); break;
                            case 0xFB04: aBuf.remove(ix, 1); aBuf.insert(ix, "ffl"); break;
                            case 0x200C:
                            case 0x200D: aBuf.remove(ix, 1); break;
                          }
                        }
                        OUString aWord(aBuf.makeStringAndClear());
                        OString bWrd(OU2ENC(aWord, eEnc));
#if defined(H_DEPRECATED)
                        bVal = pMS->spell(std::string(bWrd.getStr()));
#else
                        bVal = pMS->spell(bWrd.getStr()) != 0;
#endif
                        if (bVal) return -1;
                    }
                    nRes = SpellFailure::SPELLING_ERROR;
                } else {
                    return -1;
                }
                pMS = nullptr;
            }
        }
#if defined USE_JAVA && defined MACOSX
        }
 
        if ( !bHandled )
        {
            if ( !bFound )
            {
                it = m_aSecondaryNativeLocaleMap.find( aLocaleString );
                if ( it != m_aSecondaryNativeLocaleMap.end() )
                    bFound = true;
            }

            if ( bFound )
            {
                CFStringRef aString = CFStringCreateWithCharactersNoCopy( kCFAllocatorDefault, reinterpret_cast< const UniChar* >( rWord.getStr() ), rWord.getLength(), kCFAllocatorNull );
                if ( aString )
                {
                    if ( !NSSpellChecker_checkSpellingOfString( aString, it->second ) )
                        nRes = SpellFailure::SPELLING_ERROR;
                    CFRelease( aString );
                }
            }
        }
#endif	// USE_JAVA && MACOSX
    }

    return nRes;
}

sal_Bool SAL_CALL SpellChecker::isValid( const OUString& rWord, const Locale& rLocale,
            const PropertyValues& rProperties )
{
    MutexGuard  aGuard( GetLinguMutex() );

#ifdef USE_JAVA
    // If the locale is empty, spellcheck using the current locale
    if ( rWord.isEmpty() )
#else	// USE_JAVA
     if (rLocale == Locale()  ||  rWord.isEmpty())
#endif	// USE_JAVA
        return true;

    if (!hasLocale( rLocale ))
        return true;

    // return sal_False to process SPELLML requests (they are longer than the header)
    if (rWord.match(SPELL_XML, 0) && (rWord.getLength() > 10)) return false;

    // Get property values to be used.
    // These are be the default values set in the SN_LINGU_PROPERTIES
    // PropertySet which are overridden by the supplied ones from the
    // last argument.
    // You'll probably like to use a simpler solution than the provided
    // one using the PropertyHelper_Spell.
    PropertyHelper_Spelling& rHelper = GetPropHelper();
    rHelper.SetTmpPropVals( rProperties );

    sal_Int16 nFailure = GetSpellFailure( rWord, rLocale );
    if (nFailure != -1 && !rWord.match(SPELL_XML, 0))
    {
        LanguageType nLang = LinguLocaleToLanguage( rLocale );
        // postprocess result for errors that should be ignored
        const bool bIgnoreError =
                (!rHelper.IsSpellUpperCase()  && IsUpper( rWord, nLang )) ||
                (!rHelper.IsSpellWithDigits() && HasDigits( rWord )) ||
                (!rHelper.IsSpellCapitalization()  &&  nFailure == SpellFailure::CAPTION_ERROR);
        if (bIgnoreError)
            nFailure = -1;
    }

    return (nFailure == -1);
}

Reference< XSpellAlternatives >
    SpellChecker::GetProposals( const OUString &rWord, const Locale &rLocale )
{
    // Retrieves the return values for the 'spell' function call in case
    // of a misspelled word.
    // Especially it may give a list of suggested (correct) words:
    Reference< XSpellAlternatives > xRes;
    // note: mutex is held by higher up by spell which covers both

    Hunspell* pMS = nullptr;
    rtl_TextEncoding eEnc = RTL_TEXTENCODING_DONTKNOW;

    // first handle smart quotes (single and double)
    OUStringBuffer rBuf(rWord);
    sal_Int32 n = rBuf.getLength();
    sal_Unicode c;
    for (sal_Int32 ix=0; ix < n; ix++)
    {
        c = rBuf[ix];
        if ((c == 0x201C) || (c == 0x201D))
            rBuf[ix] = u'"';
        if ((c == 0x2018) || (c == 0x2019))
            rBuf[ix] = u'\'';
    }
    OUString nWord(rBuf.makeStringAndClear());

    if (n)
    {
        LanguageType nLang = LinguLocaleToLanguage( rLocale );
        int numsug = 0;

        Sequence< OUString > aStr( 0 );
#if defined USE_JAVA && defined MACOSX
        bool bHandled = false;
        bool bFound = false;
        OUString aLocaleString( ImplGetLocaleString( rLocale ) );
        ::std::map< OUString, CFStringRef >::const_iterator it = m_aPrimaryNativeLocaleMap.find( aLocaleString );
        if ( it != m_aPrimaryNativeLocaleMap.end() )
        {
            bFound = true;
        }
        else
        {
#endif	// USE_JAVA && MACOSX
        for (int i = 0; i < m_nNumDict; i++)
        {
            pMS = nullptr;
            eEnc = RTL_TEXTENCODING_DONTKNOW;

            if (rLocale == m_aDLocs[i])
            {
                pMS = m_aDicts[i];
                eEnc = m_aDEncs[i];
            }

            if (pMS)
            {
#if defined USE_JAVA && defined MACOSX
                bHandled = true;
#endif	// USE_JAVA && MACOSX
                OString aWrd(OU2ENC(nWord,eEnc));
#if defined(H_DEPRECATED)
                std::vector<std::string> suglst = pMS->suggest(std::string(aWrd.getStr()));
                if (!suglst.empty())
                {
                    aStr.realloc(numsug + suglst.size());
                    OUString *pStr = aStr.getArray();
                    for (size_t ii = 0; ii < suglst.size(); ++ii)
                    {
                        OUString cvtwrd(suglst[ii].c_str(), suglst[ii].size(), eEnc);
                        pStr[numsug + ii] = cvtwrd;
                    }
                    numsug += suglst.size();
                }
#else
                char ** suglst = nullptr;
                int count = pMS->suggest(&suglst, aWrd.getStr());
                if (count)
                {
                    aStr.realloc( numsug + count );
                    OUString *pStr = aStr.getArray();
                    for (int ii=0; ii < count; ++ii)
                    {
                        OUString cvtwrd(suglst[ii],strlen(suglst[ii]),eEnc);
                        pStr[numsug + ii] = cvtwrd;
                    }
                    numsug += count;
                }
                pMS->free_list(&suglst, count);
#endif
            }
        }
#if defined USE_JAVA && defined MACOSX
        }

        if ( !bHandled )
        {
            if ( !bFound )
            {
                it = m_aSecondaryNativeLocaleMap.find( aLocaleString );
                if ( it != m_aSecondaryNativeLocaleMap.end() )
                    bFound = true;
            }

            if ( bFound )
            {
                CFStringRef aString = CFStringCreateWithCharactersNoCopy( kCFAllocatorDefault, reinterpret_cast< const UniChar* >( rWord.getStr() ), rWord.getLength(), kCFAllocatorNull );
                if ( aString )
                {
                    CFArrayRef aGuesses = NSSpellChecker_getGuesses( aString, it->second );
                    if ( aGuesses )
                    {
                        CFIndex nItems = CFArrayGetCount( aGuesses );
                        aStr.realloc( nItems );
                        OUString *pAlternativesArray = aStr.getArray();

                        CFIndex nItemsAdded = 0;
                        for ( CFIndex i = 0; i < nItems; i++ )
                        {
                            CFStringRef aGuessString = static_cast< CFStringRef >( CFArrayGetValueAtIndex( aGuesses, i ) );
                            if ( aGuessString )
                            {
                                CFIndex nStringLen = CFStringGetLength( aGuessString );
                                CFRange aStringRange = CFRangeMake( 0, nStringLen );
                                sal_Unicode pStringBuffer[ nStringLen + 1 ];
                                CFStringGetCharacters( aGuessString, aStringRange, reinterpret_cast< UniChar* >( pStringBuffer ) );
                                pStringBuffer[ nStringLen ] = 0;
                                OUString aItem( pStringBuffer );

                                if ( !aItem.getLength() )
                                    continue;

                                pAlternativesArray[ nItemsAdded++ ] = aItem;
                            }
                        }

                        aStr.realloc( nItemsAdded );

                        CFRelease( aGuesses );
                    }

                    CFRelease( aString );
                }
            }
        }
#endif	// USE_JAVA && MACOSX

        // now return an empty alternative for no suggestions or the list of alternatives if some found
        xRes = SpellAlternatives::CreateSpellAlternatives( rWord, nLang, SpellFailure::SPELLING_ERROR, aStr );
        return xRes;
    }
    return xRes;
}

Reference< XSpellAlternatives > SAL_CALL SpellChecker::spell(
        const OUString& rWord, const Locale& rLocale,
        const PropertyValues& rProperties )
{
    MutexGuard  aGuard( GetLinguMutex() );

#ifdef USE_JAVA
    // If the locale is empty, spellcheck using the current locale
    if ( rWord.isEmpty() )
#else	// USE_JAVA
     if (rLocale == Locale()  ||  rWord.isEmpty())
#endif	// USE_JAVA
        return nullptr;

    if (!hasLocale( rLocale ))
        return nullptr;

    Reference< XSpellAlternatives > xAlt;
    if (!isValid( rWord, rLocale, rProperties ))
    {
        xAlt =  GetProposals( rWord, rLocale );
    }
    return xAlt;
}

/// @throws Exception
Reference< XInterface > SAL_CALL SpellChecker_CreateInstance(
        const Reference< XMultiServiceFactory > & /*rSMgr*/ )
{

    Reference< XInterface > xService = static_cast<cppu::OWeakObject*>(new SpellChecker);
    return xService;
}

sal_Bool SAL_CALL SpellChecker::addLinguServiceEventListener(
        const Reference< XLinguServiceEventListener >& rxLstnr )
{
    MutexGuard  aGuard( GetLinguMutex() );

    bool bRes = false;
    if (!m_bDisposing && rxLstnr.is())
    {
        bRes = GetPropHelper().addLinguServiceEventListener( rxLstnr );
    }
    return bRes;
}

sal_Bool SAL_CALL SpellChecker::removeLinguServiceEventListener(
        const Reference< XLinguServiceEventListener >& rxLstnr )
{
    MutexGuard  aGuard( GetLinguMutex() );

    bool bRes = false;
    if (!m_bDisposing && rxLstnr.is())
    {
        bRes = GetPropHelper().removeLinguServiceEventListener( rxLstnr );
    }
    return bRes;
}

OUString SAL_CALL SpellChecker::getServiceDisplayName( const Locale& /*rLocale*/ )
{
#if defined USE_JAVA && defined MACOSX && defined PRODUCT_NAME
    return OUString( PRODUCT_NAME " Mac Spellchecker + Grammarchecker" );
#else	// USE_JAVA && MACOSX && PRODUCT_NAME
    return OUString( "Hunspell SpellChecker" );
#endif	// USE_JAVA && MACOSX && PRODUCT_NAME
}

void SAL_CALL SpellChecker::initialize( const Sequence< Any >& rArguments )
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (!m_pPropHelper)
    {
        sal_Int32 nLen = rArguments.getLength();
        if (2 == nLen)
        {
            Reference< XLinguProperties >   xPropSet;
            rArguments.getConstArray()[0] >>= xPropSet;
            // rArguments.getConstArray()[1] >>= xDicList;

            //! Pointer allows for access of the non-UNO functions.
            //! And the reference to the UNO-functions while increasing
            //! the ref-count and will implicitly free the memory
            //! when the object is no longer used.
            m_pPropHelper = new PropertyHelper_Spelling( static_cast<XSpellChecker *>(this), xPropSet );
            m_pPropHelper->AddAsPropListener();   //! after a reference is established
        }
        else {
            OSL_FAIL( "wrong number of arguments in sequence" );
        }
    }
}

void SAL_CALL SpellChecker::dispose()
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (!m_bDisposing)
    {
        m_bDisposing = true;
        EventObject aEvtObj( static_cast<XSpellChecker *>(this) );
        m_aEvtListeners.disposeAndClear( aEvtObj );
        if (m_pPropHelper)
        {
            m_pPropHelper->RemoveAsPropListener();
            delete m_pPropHelper;
            m_pPropHelper = nullptr;
        }
    }
}

void SAL_CALL SpellChecker::addEventListener( const Reference< XEventListener >& rxListener )
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (!m_bDisposing && rxListener.is())
        m_aEvtListeners.addInterface( rxListener );
}

void SAL_CALL SpellChecker::removeEventListener( const Reference< XEventListener >& rxListener )
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (!m_bDisposing && rxListener.is())
        m_aEvtListeners.removeInterface( rxListener );
}

// Service specific part
OUString SAL_CALL SpellChecker::getImplementationName()
{
    return getImplementationName_Static();
}

sal_Bool SAL_CALL SpellChecker::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}

Sequence< OUString > SAL_CALL SpellChecker::getSupportedServiceNames()
{
    return getSupportedServiceNames_Static();
}

#if defined USE_JAVA && defined MACOSX

sal_Bool SpellChecker::isSpellChecker()
{
    return sal_True;
}


ProofreadingResult SpellChecker::doProofreading( const OUString& aDocumentIdentifier, const OUString& aText, const Locale& aLocale, sal_Int32 nStartOfSentencePosition, sal_Int32 nSuggestedBehindEndOfSentencePosition, const Sequence< PropertyValue >& /* aProperties */ )
{
    MutexGuard    aGuard( GetLinguMutex() );

    ProofreadingResult aRet;
    aRet.aDocumentIdentifier = aDocumentIdentifier;
    aRet.aText = aText;
    aRet.aLocale = aLocale;
    aRet.nStartOfSentencePosition = nStartOfSentencePosition;
    aRet.nBehindEndOfSentencePosition = nSuggestedBehindEndOfSentencePosition;
    aRet.xProofreader = this;

    bool bFound = false;
    OUString aLocaleString( ImplGetLocaleString( aLocale ) );
    ::std::map< OUString, CFStringRef >::const_iterator it = m_aPrimaryNativeLocaleMap.find( aLocaleString );
    if ( it != m_aPrimaryNativeLocaleMap.end() )
    {
        bFound = true;
    }
    else
    {
        it = m_aSecondaryNativeLocaleMap.find( aLocaleString );
        if ( it != m_aSecondaryNativeLocaleMap.end() )
            bFound = true;
    }

    if ( bFound )
        NSSpellChecker_checkGrammarOfString( &aRet, it->second );

    return aRet;
}

void SpellChecker::ignoreRule( const OUString& /* aRuleIdentifier */, const Locale& /* aLocale */ )
{
#ifdef DEBUG
    fprintf( stderr, "SpellChecker::resetIgnoreRules not implemented\n" );
#endif
}

void SpellChecker::resetIgnoreRules()
{
#ifdef DEBUG
    fprintf( stderr, "SpellChecker::resetIgnoreRules not implemented\n" );
#endif
}

#endif    // USE_JAVA && MACOSX

Sequence< OUString > SpellChecker::getSupportedServiceNames_Static()
        throw()
{
#if defined USE_JAVA && defined MACOSX
    Sequence< OUString > aSNS { SN_SPELLCHECKER, SN_GRAMMARCHECKER };
#else	// USE_JAVA && MACOSX
    Sequence< OUString > aSNS { SN_SPELLCHECKER };
#endif	// USE_JAVA && MACOSX
    return aSNS;
}

void * SAL_CALL SpellChecker_getFactory( const sal_Char * pImplName,
            XMultiServiceFactory * pServiceManager  )
{
    void * pRet = nullptr;
    if ( SpellChecker::getImplementationName_Static().equalsAscii( pImplName ) )
    {
        Reference< XSingleServiceFactory > xFactory =
            cppu::createOneInstanceFactory(
                pServiceManager,
                SpellChecker::getImplementationName_Static(),
                SpellChecker_CreateInstance,
                SpellChecker::getSupportedServiceNames_Static());
        // acquire, because we return an interface pointer instead of a reference
        xFactory->acquire();
        pRet = xFactory.get();
    }
    return pRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
