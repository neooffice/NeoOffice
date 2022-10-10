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
#include <cppuhelper/factory.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <tools/debug.hxx>
#include <osl/mutex.hxx>

#include <lingutil.hxx>
#include <hunspell.hxx>
#include <dictmgr.hxx>
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
#define SPELLML_HEADER "<?xml?>"

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
    aDicts(NULL),
    aDEncs(NULL),
    aDLocs(NULL),
    aDNames(NULL),
    numdict(0),
    aEvtListeners(GetLinguMutex()),
    pPropHelper(NULL),
    bDisposing(false)
#ifdef USE_JAVA
    , mnLastNumDics(0)
#ifdef MACOSX
    , maLocales(NULL)
#endif	// MACOSX
#endif	// USE_JAVA
{
}

SpellChecker::~SpellChecker()
{
#if defined USE_JAVA && defined MACOSX
    if ( maLocales )
        CFRelease( maLocales );
#endif	// USE_JAVA && MACOSX

    if (aDicts)
    {
       for (int i = 0; i < numdict; ++i)
       {
            delete aDicts[i];
       }
       delete[] aDicts;
    }
    delete[] aDEncs;
    delete[] aDLocs;
    delete[] aDNames;
    if (pPropHelper)
    {
        pPropHelper->RemoveAsPropListener();
        delete pPropHelper;
    }
}

PropertyHelper_Spelling & SpellChecker::GetPropHelper_Impl()
{
    if (!pPropHelper)
    {
        Reference< XLinguProperties >   xPropSet( GetLinguProperties(), UNO_QUERY );

        pPropHelper = new PropertyHelper_Spelling( (XSpellChecker *) this, xPropSet );
        pPropHelper->AddAsPropListener();   //! after a reference is established
    }
    return *pPropHelper;
}

Sequence< Locale > SAL_CALL SpellChecker::getLocales()
        throw(RuntimeException, std::exception)
{
    MutexGuard  aGuard( GetLinguMutex() );

    // this routine should return the locales supported by the installed
    // dictionaries.
#ifndef USE_JAVA
    if (!numdict)
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
        if (mnLastNumDics != aDics.size() || !maLocales)
#else	// MACOSX
        if (mnLastNumDics != aDics.size())
#endif	// MACOSX
        {
            mnLastNumDics = aDics.size();
#endif	// USE_JAVA
        if (!aDics.empty())
        {
            // get supported locales from the dictionaries-to-use...
            sal_Int32 k = 0;
            std::set< OUString, lt_rtl_OUString > aLocaleNamesSet;
            std::list< SvtLinguConfigDictionaryEntry >::const_iterator aDictIt;
            for (aDictIt = aDics.begin();  aDictIt != aDics.end();  ++aDictIt)
            {
                uno::Sequence< OUString > aLocaleNames( aDictIt->aLocaleNames );
                sal_Int32 nLen2 = aLocaleNames.getLength();
                for (k = 0;  k < nLen2;  ++k)
                {
                    aLocaleNamesSet.insert( aLocaleNames[k] );
                }
            }
            // ... and add them to the resulting sequence
            aSuppLocales.realloc( aLocaleNamesSet.size() );
            std::set< OUString, lt_rtl_OUString >::const_iterator aItB;
            k = 0;
            for (aItB = aLocaleNamesSet.begin();  aItB != aLocaleNamesSet.end();  ++aItB)
            {
                Locale aTmp( LanguageTag::convertToLocale( *aItB ));
                aSuppLocales[k++] = aTmp;
            }

            //! For each dictionary and each locale we need a separate entry.
            //! If this results in more than one dictionary per locale than (for now)
            //! it is undefined which dictionary gets used.
            //! In the future the implementation should support using several dictionaries
            //! for one locale.
            numdict = 0;
            for (aDictIt = aDics.begin();  aDictIt != aDics.end();  ++aDictIt)
                numdict = numdict + aDictIt->aLocaleNames.getLength();

            // add dictionary information
            aDicts  = new Hunspell* [numdict];
            aDEncs  = new rtl_TextEncoding [numdict];
            aDLocs  = new Locale [numdict];
            aDNames = new OUString [numdict];
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
                    // Once for each of it's supported locales.
                    for (sal_Int32 i = 0;  i < nLocales;  ++i)
                    {
                        aDicts[k]  = NULL;
                        aDEncs[k]  = RTL_TEXTENCODING_DONTKNOW;
                        aDLocs[k]  = LanguageTag::convertToLocale( aLocaleNames[i] );
                        // also both files have to be in the same directory and the
                        // file names must only differ in the extension (.aff/.dic).
                        // Thus we use the first location only and strip the extension part.
                        OUString aLocation = aDictIt->aLocations[0];
                        sal_Int32 nPos = aLocation.lastIndexOf( '.' );
                        aLocation = aLocation.copy( 0, nPos );
                        aDNames[k] = aLocation;

                        ++k;
                    }
                }
            }
            DBG_ASSERT( k == numdict, "index mismatch?" );
        }
        else
        {
            // no dictionary found so register no dictionaries
            numdict = 0;
            delete[] aDicts;
            aDicts  = NULL;
            delete[] aDEncs;
            aDEncs = NULL;
            delete[] aDLocs;
            aDLocs  = NULL;
            delete[] aDNames;
            aDNames = NULL;
            aSuppLocales.realloc(0);
        }

#if defined USE_JAVA && defined MACOSX
        ::std::list< Locale > aAppLocalesList;
        CFMutableArrayRef aAppLocales = CFArrayCreateMutable( kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks );
        if ( aAppLocales )
        {
            for ( unsigned short nLang = 0x0; nLang < 0xffff; nLang++ )
            {
                Locale aLocale = LanguageTag::convertToLocale( nLang, false );
                OUString aLocaleString( ImplGetLocaleString( aLocale ) );
                if ( aLocaleString.getLength() )
                {
                    CFStringRef aString = CFStringCreateWithCharactersNoCopy( NULL, aLocaleString.getStr(), aLocaleString.getLength(), kCFAllocatorNull );
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
                            CFStringRef aTmpString = CFStringCreateWithCharactersNoCopy( NULL, aTmpLocaleString.getStr(), aTmpLocaleString.getLength(), kCFAllocatorNull );
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
                            CFStringRef aTmpString = CFStringCreateWithCharactersNoCopy( NULL, aTmpLocaleString.getStr(), aTmpLocaleString.getLength(), kCFAllocatorNull );
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

        if ( maLocales )
        {
            CFRelease( maLocales );
            maLocales = NULL;
        }
        maPrimaryNativeLocaleMap.clear();
        maSecondaryNativeLocaleMap.clear();

        maLocales = NSSpellChecker_getLocales( aAppLocales );
        if ( maLocales )
        {
            sal_Int32 nStart = aSuppLocales.getLength();
            CFIndex nItems = CFArrayGetCount( maLocales );
            aSuppLocales.realloc( nStart + nItems );
            Locale *pLocaleArray = aSuppLocales.getArray();

            CFIndex nItemsAdded = nStart;
            for ( CFIndex i = 0; i < nItems; i++ )
            {
                CFStringRef aString = (CFStringRef)CFArrayGetValueAtIndex( maLocales, i );
                if ( aString )
                {
                    CFIndex nLen = CFStringGetLength( aString );
                    CFRange aRange = CFRangeMake( 0, nLen );
                    sal_Unicode pBuffer[ nLen + 1 ];
                    CFStringGetCharacters( aString, aRange, pBuffer );
                    pBuffer[ nLen ] = 0;
                    OUString aItem( pBuffer );

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

                    ::std::map< OUString, CFStringRef >::const_iterator it = maPrimaryNativeLocaleMap.find( aLocaleString );
                    if ( it == maPrimaryNativeLocaleMap.end() )
                    {
                        maPrimaryNativeLocaleMap[ aLocaleString ] = aString;

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
                                        maSecondaryNativeLocaleMap[ aTmpLocaleString ] = aString;
                                }
                            }

                            if ( pLocaleArray[ j ].Country.getLength() )
                            {
                                Locale aTmpLocale( pLocaleArray[ j ].Language, OUString(), OUString() );
                                if ( aLocale == aTmpLocale )
                                {
                                    OUString aTmpLocaleString( ImplGetLocaleString( aTmpLocale ) );
                                    if ( aTmpLocaleString.getLength() )
                                        maSecondaryNativeLocaleMap[ aTmpLocaleString ] = aString;
                                }
                            }
                        }

                       if ( bAddLocale )
                            pLocaleArray[ nItemsAdded++ ] = aLocale;
                    }
                }
            }

            aSuppLocales.realloc( nItemsAdded );
        }

        // Fix 2686 by finding partial matches to application locales
        sal_Int32 nStart = aSuppLocales.getLength();
        aSuppLocales.realloc( nStart + aAppLocalesList.size() );
        CFIndex nItemsAdded = nStart;
        Locale *pLocaleArray = aSuppLocales.getArray();
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
                        ::std::map< OUString, CFStringRef >::const_iterator nlit = maPrimaryNativeLocaleMap.find( aTmpLocaleString );
                        if ( nlit != maPrimaryNativeLocaleMap.end() )
                        {
                            bFound = true;
                        }
                        else
                        {
                            nlit = maSecondaryNativeLocaleMap.find( aTmpLocaleString );
                            if ( nlit != maSecondaryNativeLocaleMap.end() )
                                bFound = true;
                        }

                        if ( bFound )
                        {
                            maSecondaryNativeLocaleMap[ aLocaleString ] = nlit->second;
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
                        ::std::map< OUString, CFStringRef >::const_iterator nlit = maPrimaryNativeLocaleMap.find( aTmpLocaleString );
                        if ( nlit != maPrimaryNativeLocaleMap.end() )
                        {
                            bFound = true;
                        }
                        else
                        {
                            nlit = maSecondaryNativeLocaleMap.find( aTmpLocaleString );
                            if ( nlit != maSecondaryNativeLocaleMap.end() )
                                bFound = true;
                        }

                        if ( bFound )
                        {
                            maSecondaryNativeLocaleMap[ aLocaleString ] = nlit->second;
                            bAddLocale = true;
                            break;
                        }
                    }
                }
            }

            if ( bAddLocale )
                pLocaleArray[ nItemsAdded++ ] = *it;
        }

        aSuppLocales.realloc( nItemsAdded );

        if ( aAppLocales )
            CFRelease( aAppLocales );
#endif	// USE_JAVA && MACOSX
    }

    return aSuppLocales;
}

sal_Bool SAL_CALL SpellChecker::hasLocale(const Locale& rLocale)
        throw(RuntimeException, std::exception)
{
    MutexGuard  aGuard( GetLinguMutex() );

    bool bRes = false;
    if (!aSuppLocales.getLength())
        getLocales();

    const Locale *pLocale = aSuppLocales.getConstArray();
    sal_Int32 nLen = aSuppLocales.getLength();
#if defined USE_JAVA && defined MACOSX
    // Check native locales first 
    OUString aLocaleString( ImplGetLocaleString( rLocale ) );
    if ( aLocaleString.getLength() )
    {
        ::std::map< OUString, CFStringRef >::const_iterator it = maPrimaryNativeLocaleMap.find( aLocaleString );
        if ( it != maPrimaryNativeLocaleMap.end() )
            return sal_True;

        it = maSecondaryNativeLocaleMap.find( aLocaleString );
        if ( it != maSecondaryNativeLocaleMap.end() )
            return sal_True;

        // Fix bug 2513 by checking for approximate matches in the native locales
        if ( rLocale.Variant.getLength() )
        {
            bool bFound = false;
            Locale aTmpLocale( rLocale.Language, rLocale.Country, OUString() );
            OUString aTmpLocaleString = ImplGetLocaleString( aTmpLocale );
            it = maPrimaryNativeLocaleMap.find( aTmpLocaleString );
            if ( it != maPrimaryNativeLocaleMap.end() )
            {
                bFound = true;
            }
            else
            {
                it = maSecondaryNativeLocaleMap.find( aTmpLocaleString );
                if ( it != maSecondaryNativeLocaleMap.end() )
                    bFound = true;
            }

            if ( bFound )
            {
                aSuppLocales.realloc( ++nLen );
                Locale *pLocaleArray = aSuppLocales.getArray();
                pLocaleArray[ nLen - 1 ] = rLocale;
                maSecondaryNativeLocaleMap[ aLocaleString ] = it->second;
                return sal_True;
            }
        }

        if ( rLocale.Country.getLength() )
        {
            bool bFound = false;
            Locale aTmpLocale( rLocale.Language, OUString(), OUString() );
            OUString aTmpLocaleString = ImplGetLocaleString( aTmpLocale );
            it = maPrimaryNativeLocaleMap.find( aTmpLocaleString );
            if ( it != maPrimaryNativeLocaleMap.end() )
            {
                bFound = true;
            }
            else
            {
                it = maSecondaryNativeLocaleMap.find( aTmpLocaleString );
                if ( it != maSecondaryNativeLocaleMap.end() )
                    bFound = true;
            }

            if ( bFound )
            {
                aSuppLocales.realloc( ++nLen );
                Locale *pLocaleArray = aSuppLocales.getArray();
                pLocaleArray[ nLen - 1 ] = rLocale;
                maSecondaryNativeLocaleMap[ aLocaleString ] = it->second;
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

sal_Int16 SpellChecker::GetSpellFailure( const OUString &rWord, const Locale &rLocale )
{
    Hunspell * pMS = NULL;
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
            rBuf[ix] = (sal_Unicode)0x0022;
        else if ((c == 0x2018) || (c == 0x2019))
            rBuf[ix] = (sal_Unicode)0x0027;

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
        ::std::map< OUString, CFStringRef >::const_iterator it = maPrimaryNativeLocaleMap.find( aLocaleString );
        if ( it != maPrimaryNativeLocaleMap.end() )
        {
            bFound = true;
        }
        else
        {
#endif	// USE_JAVA && MACOSX
        for (sal_Int32 i = 0; i < numdict; ++i)
        {
            pMS = NULL;
            eEnc = RTL_TEXTENCODING_DONTKNOW;

            if (rLocale == aDLocs[i])
            {
                if (!aDicts[i])
                {
                    OUString dicpath = aDNames[i] + ".dic";
                    OUString affpath = aDNames[i] + ".aff";
                    OUString dict;
                    OUString aff;
                    osl::FileBase::getSystemPathFromFileURL(dicpath,dict);
                    osl::FileBase::getSystemPathFromFileURL(affpath,aff);
#if defined(WNT)
                    // workaround for Windows specific problem that the
                    // path length in calls to 'fopen' is limted to somewhat
                    // about 120+ characters which will usually be exceed when
                    // using dictionaries as extensions. (Hunspell waits UTF-8 encoded
                    // path with \\?\ long path prefix.)
                    OString aTmpaff = Win_AddLongPathPrefix(OUStringToOString(aff, RTL_TEXTENCODING_UTF8));
                    OString aTmpdict = Win_AddLongPathPrefix(OUStringToOString(dict, RTL_TEXTENCODING_UTF8));
#else
                    OString aTmpaff(OU2ENC(aff,osl_getThreadTextEncoding()));
                    OString aTmpdict(OU2ENC(dict,osl_getThreadTextEncoding()));
#endif

                    aDicts[i] = new Hunspell(aTmpaff.getStr(),aTmpdict.getStr());
                    aDEncs[i] = RTL_TEXTENCODING_DONTKNOW;
                    if (aDicts[i])
                        aDEncs[i] = getTextEncodingFromCharset(aDicts[i]->get_dic_encoding());
                }
                pMS = aDicts[i];
                eEnc = aDEncs[i];
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
                int rVal = pMS->spell((char*)aWrd.getStr());
                if (rVal != 1) {
                    if (extrachar && (eEnc != RTL_TEXTENCODING_UTF8)) {
                        OUStringBuffer mBuf(nWord);
                        n = mBuf.getLength();
                        for (sal_Int32 ix=n-1; ix >= 0; ix--)
                        {
                          switch (mBuf[ix]) {
                            case 0xFB00: mBuf.remove(ix, 1); mBuf.insert(ix, "ff"); break;
                            case 0xFB01: mBuf.remove(ix, 1); mBuf.insert(ix, "fi"); break;
                            case 0xFB02: mBuf.remove(ix, 1); mBuf.insert(ix, "fl"); break;
                            case 0xFB03: mBuf.remove(ix, 1); mBuf.insert(ix, "ffi"); break;
                            case 0xFB04: mBuf.remove(ix, 1); mBuf.insert(ix, "ffl"); break;
                            case 0x200C:
                            case 0x200D: mBuf.remove(ix, 1); break;
                          }
                        }
                        OUString mWord(mBuf.makeStringAndClear());
                        OString bWrd(OU2ENC(mWord, eEnc));
                        rVal = pMS->spell((char*)bWrd.getStr());
                        if (rVal == 1) return -1;
                    }
                    nRes = SpellFailure::SPELLING_ERROR;
                } else {
                    return -1;
                }
                pMS = NULL;
            }
        }
#if defined USE_JAVA && defined MACOSX
        }
 
        if ( !bHandled )
        {
            if ( !bFound )
            {
                it = maSecondaryNativeLocaleMap.find( aLocaleString );
                if ( it != maSecondaryNativeLocaleMap.end() )
                    bFound = true;
            }

            if ( bFound )
            {
                CFStringRef aString = CFStringCreateWithCharactersNoCopy( kCFAllocatorDefault, rWord.getStr(), rWord.getLength(), kCFAllocatorNull );
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
        throw(IllegalArgumentException, RuntimeException, std::exception)
{
    MutexGuard  aGuard( GetLinguMutex() );

#ifdef USE_JAVA
    // If the locale is empty, spellcheck using the current locale
    if ( rWord.isEmpty() )
#else	// USE_JAVA
     if (rLocale == Locale()  ||  rWord.isEmpty())
#endif	// USE_JAVA
        return sal_True;

    if (!hasLocale( rLocale ))
        return sal_True;

    // return sal_False to process SPELLML requests (they are longer than the header)
    if (rWord.match(SPELLML_HEADER, 0) && (rWord.getLength() > 10)) return sal_False;

    // Get property values to be used.
    // These are be the default values set in the SN_LINGU_PROPERTIES
    // PropertySet which are overridden by the supplied ones from the
    // last argument.
    // You'll probably like to use a simpler solution than the provided
    // one using the PropertyHelper_Spell.
    PropertyHelper_Spelling& rHelper = GetPropHelper();
    rHelper.SetTmpPropVals( rProperties );

    sal_Int16 nFailure = GetSpellFailure( rWord, rLocale );
    if (nFailure != -1 && !rWord.match(SPELLML_HEADER, 0))
    {
        sal_Int16 nLang = LinguLocaleToLanguage( rLocale );
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

    Hunspell* pMS = NULL;
    rtl_TextEncoding eEnc = RTL_TEXTENCODING_DONTKNOW;

    // first handle smart quotes (single and double)
    OUStringBuffer rBuf(rWord);
    sal_Int32 n = rBuf.getLength();
    sal_Unicode c;
    for (sal_Int32 ix=0; ix < n; ix++)
    {
        c = rBuf[ix];
        if ((c == 0x201C) || (c == 0x201D))
            rBuf[ix] = (sal_Unicode)0x0022;
        if ((c == 0x2018) || (c == 0x2019))
            rBuf[ix] = (sal_Unicode)0x0027;
    }
    OUString nWord(rBuf.makeStringAndClear());

    if (n)
    {
        sal_Int16 nLang = LinguLocaleToLanguage( rLocale );
        int numsug = 0;

        Sequence< OUString > aStr( 0 );
#if defined USE_JAVA && defined MACOSX
        bool bHandled = false;
        bool bFound = false;
        OUString aLocaleString( ImplGetLocaleString( rLocale ) );
        ::std::map< OUString, CFStringRef >::const_iterator it = maPrimaryNativeLocaleMap.find( aLocaleString );
        if ( it != maPrimaryNativeLocaleMap.end() )
        {
            bFound = true;
        }
        else
        {
#endif	// USE_JAVA && MACOSX
        for (int i = 0; i < numdict; i++)
        {
            pMS = NULL;
            eEnc = RTL_TEXTENCODING_DONTKNOW;

            if (rLocale == aDLocs[i])
            {
                pMS = aDicts[i];
                eEnc = aDEncs[i];
            }

            if (pMS)
            {
#if defined USE_JAVA && defined MACOSX
                bHandled = true;
#endif	// USE_JAVA && MACOSX
                char ** suglst = NULL;
                OString aWrd(OU2ENC(nWord,eEnc));
                int count = pMS->suggest(&suglst, (const char *) aWrd.getStr());

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
            }
        }
#if defined USE_JAVA && defined MACOSX
       }

       if ( !bHandled )
       {
           if ( !bFound )
           {
               it = maSecondaryNativeLocaleMap.find( aLocaleString );
               if ( it != maSecondaryNativeLocaleMap.end() )
                   bFound = true;
           }

           if ( bFound )
           {
               CFStringRef aString = CFStringCreateWithCharactersNoCopy( kCFAllocatorDefault, rWord.getStr(), rWord.getLength(), kCFAllocatorNull );
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
                            CFStringRef aGuessString = (CFStringRef)CFArrayGetValueAtIndex( aGuesses, i );
                            if ( aGuessString )
                            {
                                CFIndex nLen = CFStringGetLength( aGuessString );
                                CFRange aRange = CFRangeMake( 0, nLen );
                                sal_Unicode pBuffer[ nLen + 1 ];
                                CFStringGetCharacters( aGuessString, aRange, pBuffer );
                                pBuffer[ nLen ] = 0;
                                OUString aItem( pBuffer );

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
        OUString aTmp(rWord);
        xRes = SpellAlternatives::CreateSpellAlternatives( aTmp, nLang, SpellFailure::SPELLING_ERROR, aStr );
        return xRes;
    }
    return xRes;
}

Reference< XSpellAlternatives > SAL_CALL SpellChecker::spell(
        const OUString& rWord, const Locale& rLocale,
        const PropertyValues& rProperties )
        throw(IllegalArgumentException, RuntimeException, std::exception)
{
    MutexGuard  aGuard( GetLinguMutex() );

#ifdef USE_JAVA
    // If the locale is empty, spellcheck using the current locale
    if ( rWord.isEmpty() )
#else	// USE_JAVA
     if (rLocale == Locale()  ||  rWord.isEmpty())
#endif	// USE_JAVA
        return NULL;

    if (!hasLocale( rLocale ))
        return NULL;

    Reference< XSpellAlternatives > xAlt;
    if (!isValid( rWord, rLocale, rProperties ))
    {
        xAlt =  GetProposals( rWord, rLocale );
    }
    return xAlt;
}

Reference< XInterface > SAL_CALL SpellChecker_CreateInstance(
        const Reference< XMultiServiceFactory > & /*rSMgr*/ )
        throw(Exception)
{

    Reference< XInterface > xService = (cppu::OWeakObject*) new SpellChecker;
    return xService;
}

sal_Bool SAL_CALL SpellChecker::addLinguServiceEventListener(
        const Reference< XLinguServiceEventListener >& rxLstnr )
        throw(RuntimeException, std::exception)
{
    MutexGuard  aGuard( GetLinguMutex() );

    bool bRes = false;
    if (!bDisposing && rxLstnr.is())
    {
        bRes = GetPropHelper().addLinguServiceEventListener( rxLstnr );
    }
    return bRes;
}

sal_Bool SAL_CALL SpellChecker::removeLinguServiceEventListener(
        const Reference< XLinguServiceEventListener >& rxLstnr )
        throw(RuntimeException, std::exception)
{
    MutexGuard  aGuard( GetLinguMutex() );

    bool bRes = false;
    if (!bDisposing && rxLstnr.is())
    {
        bRes = GetPropHelper().removeLinguServiceEventListener( rxLstnr );
    }
    return bRes;
}

OUString SAL_CALL SpellChecker::getServiceDisplayName( const Locale& /*rLocale*/ )
        throw(RuntimeException, std::exception)
{
    MutexGuard  aGuard( GetLinguMutex() );
#if defined USE_JAVA && defined MACOSX && defined PRODUCT_NAME
    return OUString( PRODUCT_NAME " Mac Spellchecker + Grammarchecker" );
#else	// USE_JAVA && MACOSX && PRODUCT_NAME
    return OUString( "Hunspell SpellChecker" );
#endif	// USE_JAVA && MACOSX && PRODUCT_NAME
}

void SAL_CALL SpellChecker::initialize( const Sequence< Any >& rArguments )
        throw(Exception, RuntimeException, std::exception)
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (!pPropHelper)
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
            //! when the object is not longer used.
            pPropHelper = new PropertyHelper_Spelling( (XSpellChecker *) this, xPropSet );
            pPropHelper->AddAsPropListener();   //! after a reference is established
        }
        else {
            OSL_FAIL( "wrong number of arguments in sequence" );
        }
    }
}

void SAL_CALL SpellChecker::dispose()
        throw(RuntimeException, std::exception)
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (!bDisposing)
    {
        bDisposing = true;
        EventObject aEvtObj( (XSpellChecker *) this );
        aEvtListeners.disposeAndClear( aEvtObj );
        if (pPropHelper)
        {
            pPropHelper->RemoveAsPropListener();
            delete pPropHelper;
            pPropHelper = NULL;
        }
    }
}

void SAL_CALL SpellChecker::addEventListener( const Reference< XEventListener >& rxListener )
        throw(RuntimeException, std::exception)
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (!bDisposing && rxListener.is())
        aEvtListeners.addInterface( rxListener );
}

void SAL_CALL SpellChecker::removeEventListener( const Reference< XEventListener >& rxListener )
        throw(RuntimeException, std::exception)
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (!bDisposing && rxListener.is())
        aEvtListeners.removeInterface( rxListener );
}

// Service specific part
OUString SAL_CALL SpellChecker::getImplementationName()
        throw(RuntimeException, std::exception)
{
    MutexGuard  aGuard( GetLinguMutex() );

    return getImplementationName_Static();
}

sal_Bool SAL_CALL SpellChecker::supportsService( const OUString& ServiceName )
        throw(RuntimeException, std::exception)
{
    return cppu::supportsService(this, ServiceName);
}

Sequence< OUString > SAL_CALL SpellChecker::getSupportedServiceNames()
        throw(RuntimeException, std::exception)
{
    MutexGuard  aGuard( GetLinguMutex() );

    return getSupportedServiceNames_Static();
}

#if defined USE_JAVA && defined MACOSX

sal_Bool SpellChecker::isSpellChecker()
		throw(RuntimeException)
{
	return sal_True;
}


ProofreadingResult SpellChecker::doProofreading( const OUString& aDocumentIdentifier, const OUString& aText, const Locale& aLocale, sal_Int32 nStartOfSentencePosition, sal_Int32 nSuggestedBehindEndOfSentencePosition, const Sequence< PropertyValue >& /* aProperties */ )
		throw (IllegalArgumentException, RuntimeException)
{
	MutexGuard	aGuard( GetLinguMutex() );

	ProofreadingResult aRet;
	aRet.aDocumentIdentifier = aDocumentIdentifier;
	aRet.aText = aText;
	aRet.aLocale = aLocale;
	aRet.nStartOfSentencePosition = nStartOfSentencePosition;
	aRet.nBehindEndOfSentencePosition = nSuggestedBehindEndOfSentencePosition;
	aRet.xProofreader = this;

	bool bFound = false;
	OUString aLocaleString( ImplGetLocaleString( aLocale ) );
	::std::map< OUString, CFStringRef >::const_iterator it = maPrimaryNativeLocaleMap.find( aLocaleString );
	if ( it != maPrimaryNativeLocaleMap.end() )
	{
		bFound = true;
	}
	else
	{
		it = maSecondaryNativeLocaleMap.find( aLocaleString );
		if ( it != maSecondaryNativeLocaleMap.end() )
			bFound = true;
	}

	if ( bFound )
		NSSpellChecker_checkGrammarOfString( &aRet, it->second );

	return aRet;
}

void SpellChecker::ignoreRule( const OUString& /* aRuleIdentifier */, const Locale& /* aLocale */ )
		throw (IllegalArgumentException, RuntimeException)
{
#ifdef DEBUG
	fprintf( stderr, "SpellChecker::resetIgnoreRules not implemented\n" );
#endif
}

void SpellChecker::resetIgnoreRules()
		throw(RuntimeException)
{
#ifdef DEBUG
	fprintf( stderr, "SpellChecker::resetIgnoreRules not implemented\n" );
#endif
}

#endif	// USE_JAVA && MACOSX

Sequence< OUString > SpellChecker::getSupportedServiceNames_Static()
        throw()
{
    MutexGuard  aGuard( GetLinguMutex() );

#if defined USE_JAVA && defined MACOSX
    Sequence< OUString > aSNS( 2 );
#else	// USE_JAVA && MACOSX
    Sequence< OUString > aSNS( 1 ); // more than 1 service is possible, too
#endif	// USE_JAVA && MACOSX
    aSNS.getArray()[0] = SN_SPELLCHECKER;
#if defined USE_JAVA && defined MACOSX
    aSNS.getArray()[1] = SN_GRAMMARCHECKER;
#endif	// USE_JAVA && MACOSX
    return aSNS;
}

void * SAL_CALL SpellChecker_getFactory( const sal_Char * pImplName,
            XMultiServiceFactory * pServiceManager, void *  )
{
    void * pRet = 0;
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
