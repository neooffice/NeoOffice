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

#include <hintids.hxx>

#include <svx/dialogs.hrc>
#include <i18nlangtag/mslangid.hxx>
#include <sot/storinfo.hxx>
#include <sot/storage.hxx>
#include <svl/zforlist.hxx>
#include <svtools/ctrltool.hxx>
#include <unotools/lingucfg.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/sfxmodelfactory.hxx>
#include <sfx2/printer.hxx>
#include <sfx2/bindings.hxx>
#include <svl/asiancfg.hxx>
#include <editeng/unolingu.hxx>
#include <sfx2/request.hxx>
#include <svl/intitem.hxx>
#include <editeng/adjustitem.hxx>
#include <editeng/autokernitem.hxx>
#include <linguistic/lngprops.hxx>
#include <com/sun/star/document/UpdateDocMode.hpp>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <sfx2/docfilt.hxx>
#include <svx/xtable.hxx>
#include <svx/drawitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/flstitem.hxx>
#include <editeng/tstpitem.hxx>
#include <editeng/langitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/hyphenzoneitem.hxx>
#include <editeng/svxacorr.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <view.hxx>
#include <prtopt.hxx>
#include <fmtcol.hxx>
#include <docsh.hxx>
#include <wdocsh.hxx>
#include <swmodule.hxx>
#include <doc.hxx>
#include <IDocumentSettingAccess.hxx>
#include <IDocumentDeviceAccess.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <IDocumentStylePoolAccess.hxx>
#include <IDocumentChartDataProviderAccess.hxx>
#include <IDocumentState.hxx>
#include <docfac.hxx>
#include <docstyle.hxx>
#include <shellio.hxx>
#include <tox.hxx>
#include <swdtflvr.hxx>
#include <dbmgr.hxx>
#include <usrpref.hxx>
#include <fontcfg.hxx>
#include <poolfmt.hxx>
#include <modcfg.hxx>
#include <globdoc.hxx>
#include <ndole.hxx>
#include <mdiexp.hxx>
#include <unotxdoc.hxx>
#include <linkenum.hxx>
#include <swwait.hxx>
#include <wrtsh.hxx>
#include <swerror.h>
#include <globals.hrc>
#include <unochart.hxx>

// text grid
#include <tgrditem.hxx>
#include <boost/scoped_ptr.hpp>

using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;

// Load Document
bool SwDocShell::InitNew( const uno::Reference < embed::XStorage >& xStor )
{
    bool bRet = SfxObjectShell::InitNew( xStor );
    OSL_ENSURE( GetMapUnit() == MAP_TWIP, "map unit is not twip!" );
    bool bHTMLTemplSet = false;
    if( bRet )
    {
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        AddLink();      // create mpDoc / pIo if applicable
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        AddLink();      // create m_xDoc / pIo if applicable
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX

        bool bWeb = ISA( SwWebDocShell );
        if ( bWeb )
            bHTMLTemplSet = SetHTMLTemplate( *GetDoc() );// Styles from HTML.vor
        else if( ISA( SwGlobalDocShell ) )
            GetDoc()->getIDocumentSettingAccess().set(IDocumentSettingAccess::GLOBAL_DOCUMENT, true);       // Globaldokument

        if ( GetCreateMode() ==  SFX_CREATE_MODE_EMBEDDED )
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
            SwTransferable::InitOle( this, *mpDoc );
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
            SwTransferable::InitOle( this, *m_xDoc );
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX

        // set forbidden characters if necessary
        SvxAsianConfig aAsian;
        Sequence<lang::Locale> aLocales =  aAsian.GetStartEndCharLocales();
        if(aLocales.getLength())
        {
            const lang::Locale* pLocales = aLocales.getConstArray();
            for(sal_Int32 i = 0; i < aLocales.getLength(); i++)
            {
                ForbiddenCharacters aForbidden;
                aAsian.GetStartEndChars( pLocales[i], aForbidden.beginLine, aForbidden.endLine);
                LanguageType  eLang = LanguageTag::convertToLanguageType(pLocales[i]);
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                mpDoc->getIDocumentSettingAccess().setForbiddenCharacters( eLang, aForbidden);
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                    m_xDoc->getIDocumentSettingAccess().setForbiddenCharacters( eLang, aForbidden);
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
            }
        }
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        mpDoc->getIDocumentSettingAccess().set(IDocumentSettingAccess::KERN_ASIAN_PUNCTUATION,
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
            m_xDoc->getIDocumentSettingAccess().set(IDocumentSettingAccess::KERN_ASIAN_PUNCTUATION,
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                  !aAsian.IsKerningWesternTextOnly());
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        mpDoc->getIDocumentSettingAccess().setCharacterCompressionType(static_cast<SwCharCompressType>(aAsian.GetCharDistanceCompression()));
        mpDoc->getIDocumentDeviceAccess().setPrintData(*SW_MOD()->GetPrtOptions(bWeb));
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
            m_xDoc->getIDocumentSettingAccess().setCharacterCompressionType(static_cast<SwCharCompressType>(aAsian.GetCharDistanceCompression()));
            m_xDoc->getIDocumentDeviceAccess().setPrintData(*SW_MOD()->GetPrtOptions(bWeb));
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX

        SubInitNew();

        // for all

        SwStdFontConfig* pStdFont = SW_MOD()->GetStdFontConfig();
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        SfxPrinter* pPrt = mpDoc->getIDocumentDeviceAccess().getPrinter( false );
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        SfxPrinter* pPrt = m_xDoc->getIDocumentDeviceAccess().getPrinter( false );
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX

        OUString sEntry;
        sal_uInt16 aFontWhich[] =
        {   RES_CHRATR_FONT,
            RES_CHRATR_CJK_FONT,
            RES_CHRATR_CTL_FONT
        };
        sal_uInt16 aFontHeightWhich[] =
        {
            RES_CHRATR_FONTSIZE,
            RES_CHRATR_CJK_FONTSIZE,
            RES_CHRATR_CTL_FONTSIZE
        };
        sal_uInt16 aFontIds[] =
        {
            FONT_STANDARD,
            FONT_STANDARD_CJK,
            FONT_STANDARD_CTL
        };
        sal_uInt16 nFontTypes[] =
        {
            DEFAULTFONT_LATIN_TEXT,
            DEFAULTFONT_CJK_TEXT,
            DEFAULTFONT_CTL_TEXT
        };
        sal_uInt16 aLangTypes[] =
        {
            RES_CHRATR_LANGUAGE,
            RES_CHRATR_CJK_LANGUAGE,
            RES_CHRATR_CTL_LANGUAGE
        };

        for(sal_uInt8 i = 0; i < 3; i++)
        {
            sal_uInt16 nFontWhich = aFontWhich[i];
            sal_uInt16 nFontId = aFontIds[i];
            boost::scoped_ptr<SvxFontItem> pFontItem;
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
            const SvxLanguageItem& rLang = (const SvxLanguageItem&)mpDoc->GetDefault( aLangTypes[i] );
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
            const SvxLanguageItem& rLang = static_cast<const SvxLanguageItem&>(m_xDoc->GetDefault( aLangTypes[i] ));
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
            LanguageType eLanguage = rLang.GetLanguage();
            if(!pStdFont->IsFontDefault(nFontId))
            {
                sEntry = pStdFont->GetFontFor(nFontId);

                vcl::Font aFont( sEntry, Size( 0, 10 ) );
                if( pPrt )
                {
                    aFont = pPrt->GetFontMetric( aFont );
                }

                pFontItem.reset(new SvxFontItem(aFont.GetFamily(), aFont.GetName(),
                                                aEmptyOUStr, aFont.GetPitch(), aFont.GetCharSet(), nFontWhich));
            }
            else
            {
                // #107782# OJ use korean language if latin was used
                if ( i == 0 )
                {
                        LanguageType eUiLanguage = Application::GetSettings().GetUILanguageTag().getLanguageType();
                    if (MsLangId::isKorean(eUiLanguage))
                        eLanguage = eUiLanguage;
                }

                vcl::Font aLangDefFont = OutputDevice::GetDefaultFont(
                    nFontTypes[i],
                    eLanguage,
                    DEFAULTFONT_FLAGS_ONLYONE );
                pFontItem.reset(new SvxFontItem(aLangDefFont.GetFamily(), aLangDefFont.GetName(),
                                                aEmptyOUStr, aLangDefFont.GetPitch(), aLangDefFont.GetCharSet(), nFontWhich));
            }
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
            mpDoc->SetDefault(*pFontItem);
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
            m_xDoc->SetDefault(*pFontItem);
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
            if( !bHTMLTemplSet )
            {
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                SwTxtFmtColl *pColl = mpDoc->getIDocumentStylePoolAccess().GetTxtCollFromPool(RES_POOLCOLL_STANDARD);
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                SwTxtFmtColl *pColl = m_xDoc->getIDocumentStylePoolAccess().GetTxtCollFromPool(RES_POOLCOLL_STANDARD);
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                pColl->ResetFmtAttr(nFontWhich);
            }
            pFontItem.reset();
            sal_Int32 nFontHeight = pStdFont->GetFontHeight( FONT_STANDARD, i, eLanguage );
            if(nFontHeight <= 0)
                nFontHeight = SwStdFontConfig::GetDefaultHeightFor( nFontId, eLanguage );
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
            mpDoc->SetDefault(SvxFontHeightItem( nFontHeight, 100, aFontHeightWhich[i] ));
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
            m_xDoc->SetDefault(SvxFontHeightItem( nFontHeight, 100, aFontHeightWhich[i] ));
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
            if( !bHTMLTemplSet )
            {
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                SwTxtFmtColl *pColl = mpDoc->getIDocumentStylePoolAccess().GetTxtCollFromPool(RES_POOLCOLL_STANDARD);
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                SwTxtFmtColl *pColl = m_xDoc->getIDocumentStylePoolAccess().GetTxtCollFromPool(RES_POOLCOLL_STANDARD);
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                pColl->ResetFmtAttr(aFontHeightWhich[i]);
            }

        }
        sal_uInt16 aFontIdPoolId[] =
        {
            FONT_OUTLINE,       RES_POOLCOLL_HEADLINE_BASE,
            FONT_LIST,          RES_POOLCOLL_NUMBUL_BASE,
            FONT_CAPTION,       RES_POOLCOLL_LABEL,
            FONT_INDEX,         RES_POOLCOLL_REGISTER_BASE,
            FONT_OUTLINE_CJK,   RES_POOLCOLL_HEADLINE_BASE,
            FONT_LIST_CJK,      RES_POOLCOLL_NUMBUL_BASE,
            FONT_CAPTION_CJK,   RES_POOLCOLL_LABEL,
            FONT_INDEX_CJK,     RES_POOLCOLL_REGISTER_BASE,
            FONT_OUTLINE_CTL,   RES_POOLCOLL_HEADLINE_BASE,
            FONT_LIST_CTL,      RES_POOLCOLL_NUMBUL_BASE,
            FONT_CAPTION_CTL,   RES_POOLCOLL_LABEL,
            FONT_INDEX_CTL,     RES_POOLCOLL_REGISTER_BASE
        };

        sal_uInt16 nFontWhich = RES_CHRATR_FONT;
        sal_uInt16 nFontHeightWhich = RES_CHRATR_FONTSIZE;
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        LanguageType eLanguage = static_cast<const SvxLanguageItem&>(mpDoc->GetDefault( RES_CHRATR_LANGUAGE )).GetLanguage();
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        LanguageType eLanguage = static_cast<const SvxLanguageItem&>(m_xDoc->GetDefault( RES_CHRATR_LANGUAGE )).GetLanguage();
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        for(sal_uInt8 nIdx = 0; nIdx < 24; nIdx += 2)
        {
            if(nIdx == 8)
            {
                nFontWhich = RES_CHRATR_CJK_FONT;
                nFontHeightWhich = RES_CHRATR_CJK_FONTSIZE;
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                eLanguage = static_cast<const SvxLanguageItem&>(mpDoc->GetDefault( RES_CHRATR_CJK_LANGUAGE )).GetLanguage();
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                eLanguage = static_cast<const SvxLanguageItem&>(m_xDoc->GetDefault( RES_CHRATR_CJK_LANGUAGE )).GetLanguage();
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
            }
            else if(nIdx == 16)
            {
                nFontWhich = RES_CHRATR_CTL_FONT;
                nFontHeightWhich = RES_CHRATR_CTL_FONTSIZE;
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                eLanguage = static_cast<const SvxLanguageItem&>(mpDoc->GetDefault( RES_CHRATR_CTL_LANGUAGE )).GetLanguage();
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                eLanguage = static_cast<const SvxLanguageItem&>(m_xDoc->GetDefault( RES_CHRATR_CTL_LANGUAGE )).GetLanguage();
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
            }
            SwTxtFmtColl *pColl = 0;
            if(!pStdFont->IsFontDefault(aFontIdPoolId[nIdx]))
            {
                sEntry = pStdFont->GetFontFor(aFontIdPoolId[nIdx]);

                vcl::Font aFont( sEntry, Size( 0, 10 ) );
                if( pPrt )
                    aFont = pPrt->GetFontMetric( aFont );

#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                pColl = mpDoc->getIDocumentStylePoolAccess().GetTxtCollFromPool(aFontIdPoolId[nIdx + 1]);
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                pColl = m_xDoc->getIDocumentStylePoolAccess().GetTxtCollFromPool(aFontIdPoolId[nIdx + 1]);
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                if( !bHTMLTemplSet ||
                    SfxItemState::SET != pColl->GetAttrSet().GetItemState(
                                                    nFontWhich, false ) )
                {
                    pColl->SetFmtAttr(SvxFontItem(aFont.GetFamily(), aFont.GetName(),
                                                  aEmptyOUStr, aFont.GetPitch(), aFont.GetCharSet(), nFontWhich));
                }
            }
            sal_Int32 nFontHeight = pStdFont->GetFontHeight( static_cast< sal_Int8 >(aFontIdPoolId[nIdx]), 0, eLanguage );
            if(nFontHeight <= 0)
                nFontHeight = SwStdFontConfig::GetDefaultHeightFor( aFontIdPoolId[nIdx], eLanguage );
            if(!pColl)
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                pColl = mpDoc->getIDocumentStylePoolAccess().GetTxtCollFromPool(aFontIdPoolId[nIdx + 1]);
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                pColl = m_xDoc->getIDocumentStylePoolAccess().GetTxtCollFromPool(aFontIdPoolId[nIdx + 1]);
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
            SvxFontHeightItem aFontHeight( (const SvxFontHeightItem&)pColl->GetFmtAttr( nFontHeightWhich, true ));
            if(aFontHeight.GetHeight() != sal::static_int_cast<sal_uInt32, sal_Int32>(nFontHeight))
            {
                aFontHeight.SetHeight(nFontHeight);
                pColl->SetFmtAttr( aFontHeight );
            }
        }

        // the default for documents created via 'File/New' should be 'on'
        // (old documents, where this property was not yet implemented, will get the
        // value 'false' in the SwDoc c-tor)
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        mpDoc->getIDocumentSettingAccess().set( IDocumentSettingAccess::MATH_BASELINE_ALIGNMENT,
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        m_xDoc->getIDocumentSettingAccess().set( IDocumentSettingAccess::MATH_BASELINE_ALIGNMENT,
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                SW_MOD()->GetUsrPref( bWeb )->IsAlignMathObjectsToBaseline() );
    }

    /* #106748# If the default frame direction of a document is RTL
        the default adjusment is to the right. */
    if( !bHTMLTemplSet &&
        FRMDIR_HORI_RIGHT_TOP == GetDefaultFrameDirection(GetAppLanguage()) )
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        mpDoc->SetDefault( SvxAdjustItem(SVX_ADJUST_RIGHT, RES_PARATR_ADJUST ) );
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        m_xDoc->SetDefault( SvxAdjustItem(SVX_ADJUST_RIGHT, RES_PARATR_ADJUST ) );
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX

// #i29550#
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    mpDoc->SetDefault( SfxBoolItem( RES_COLLAPSING_BORDERS, true ) );
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    m_xDoc->SetDefault( SfxBoolItem( RES_COLLAPSING_BORDERS, true ) );
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
// <-- collapsing

    //#i16874# AutoKerning as default for new documents
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    mpDoc->SetDefault( SvxAutoKernItem( true, RES_CHRATR_AUTOKERN ) );
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    m_xDoc->SetDefault( SvxAutoKernItem( true, RES_CHRATR_AUTOKERN ) );
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX

    // #i42080# - Due to the several calls of method <SetDefault(..)>
    // at the document instance, the document is modified. Thus, reset this
    // status here. Note: In method <SubInitNew()> this is also done.
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    mpDoc->getIDocumentState().ResetModified();
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    m_xDoc->getIDocumentState().ResetModified();
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX

    return bRet;
}

// Ctor with SfxCreateMode ?????
SwDocShell::SwDocShell( SfxObjectCreateMode eMode ) :
    SfxObjectShell ( eMode ),
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    mpDoc(0),
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    mpFontList(0),
    mbInUpdateFontList(false),
    mpView( 0 ),
    mpWrtShell( 0 ),
    mpOLEChildList( 0 ),
    mnUpdateDocMode(document::UpdateDocMode::ACCORDING_TO_CONFIG),
    bIsATemplate(false)
{
    Init_Impl();
}

// Ctor / Dtor
SwDocShell::SwDocShell( const sal_uInt64 i_nSfxCreationFlags ) :
    SfxObjectShell ( i_nSfxCreationFlags ),
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    mpDoc(0),
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    mpFontList(0),
    mbInUpdateFontList(false),
    mpView( 0 ),
    mpWrtShell( 0 ),
    mpOLEChildList( 0 ),
    mnUpdateDocMode(document::UpdateDocMode::ACCORDING_TO_CONFIG),
    bIsATemplate(false)
{
    Init_Impl();
}

// Ctor / Dtor
SwDocShell::SwDocShell( SwDoc *pD, SfxObjectCreateMode eMode ):
    SfxObjectShell ( eMode ),
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    mpDoc(pD),
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    m_xDoc(pD),
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    mpFontList(0),
    mbInUpdateFontList(false),
    mpView( 0 ),
    mpWrtShell( 0 ),
    mpOLEChildList( 0 ),
    mnUpdateDocMode(document::UpdateDocMode::ACCORDING_TO_CONFIG),
    bIsATemplate(false)
{
    Init_Impl();
}

// Dtor
SwDocShell::~SwDocShell()
{
    // disable chart related objects now because in ~SwDoc it may be to late for this
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    if( mpDoc )
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    if (m_xDoc.get())
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    {
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        mpDoc->getIDocumentChartDataProviderAccess().GetChartControllerHelper().Disconnect();
        SwChartDataProvider *pPCD = mpDoc->getIDocumentChartDataProviderAccess().GetChartDataProvider();
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        m_xDoc->getIDocumentChartDataProviderAccess().GetChartControllerHelper().Disconnect();
        SwChartDataProvider *pPCD = m_xDoc->getIDocumentChartDataProviderAccess().GetChartDataProvider();
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        if (pPCD)
            pPCD->dispose();
    }

    RemoveLink();
    delete mpFontList;

    // we, as BroadCaster also become our own Listener
    // (for DocInfo/FileNames/....)
    EndListening( *this );

    delete mpOLEChildList;
}

void  SwDocShell::Init_Impl()
{
    SetPool(&SW_MOD()->GetPool());
    SetBaseModel(new SwXTextDocument(this));
    // we, as BroadCaster also become our own Listener
    // (for DocInfo/FileNames/....)
    StartListening( *this );
    //position of the "Automatic" style filter for the stylist (app.src)
    SetAutoStyleFilterIndex(3);

    // set map unit to twip
    SetMapUnit( MAP_TWIP );
}

void SwDocShell::AddLink()
{
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    if( !mpDoc )
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    if (!m_xDoc.get())
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    {
        SwDocFac aFactory;
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        mpDoc = aFactory.GetDoc();
        mpDoc->acquire();
        mpDoc->getIDocumentSettingAccess().set(IDocumentSettingAccess::HTML_MODE, ISA(SwWebDocShell) );
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        m_xDoc = aFactory.GetDoc();
        m_xDoc->getIDocumentSettingAccess().set(IDocumentSettingAccess::HTML_MODE, dynamic_cast< const SwWebDocShell *>( this ) !=  nullptr );
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    }
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    else
        mpDoc->acquire();
    mpDoc->SetDocShell( this );      // set the DocShell-Pointer for Doc
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    m_xDoc->SetDocShell( this );      // set the DocShell-Pointer for Doc
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    uno::Reference< text::XTextDocument >  xDoc(GetBaseModel(), uno::UNO_QUERY);
    ((SwXTextDocument*)xDoc.get())->Reactivate(this);

#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    SetPool(&mpDoc->GetAttrPool());
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    SetPool(&m_xDoc->GetAttrPool());
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX

    // most suitably not until a sdbcx::View is created!!!
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    mpDoc->SetOle2Link(LINK(this, SwDocShell, Ole2ModifiedHdl));
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    m_xDoc->SetOle2Link(LINK(this, SwDocShell, Ole2ModifiedHdl));
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
}

// create new FontList Change Printer
void SwDocShell::UpdateFontList()
{
    if(!mbInUpdateFontList)
    {
        mbInUpdateFontList = true;
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        OSL_ENSURE(mpDoc, "No Doc no FontList");
        if( mpDoc )
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        OSL_ENSURE(m_xDoc.get(), "No Doc no FontList");
        if (m_xDoc.get())
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        {
            delete mpFontList;
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
            mpFontList = new FontList( mpDoc->getIDocumentDeviceAccess().getReferenceDevice( true ) );
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
            mpFontList = new FontList( m_xDoc->getIDocumentDeviceAccess().getReferenceDevice(true) );
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
            PutItem( SvxFontListItem( mpFontList, SID_ATTR_CHAR_FONTLIST ) );
        }
        mbInUpdateFontList = false;
    }
}

void SwDocShell::RemoveLink()
{
    // disconnect Uno-Object
    uno::Reference< text::XTextDocument >  xDoc(GetBaseModel(), uno::UNO_QUERY);
    ((SwXTextDocument*)xDoc.get())->Invalidate();
    aFinishedTimer.Stop();
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    if (mpDoc)
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    if (m_xDoc.get())
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    {
        if( mxBasePool.is() )
        {
            static_cast<SwDocStyleSheetPool*>(mxBasePool.get())->dispose();
            mxBasePool.clear();
        }
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        sal_Int8 nRefCt = static_cast< sal_Int8 >(mpDoc->release());
        mpDoc->SetOle2Link(Link());
        mpDoc->SetDocShell( 0 );
        if( !nRefCt )
            delete mpDoc;
        mpDoc = 0;       // we don't have the Doc anymore!!
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        m_xDoc->SetOle2Link(Link());
        m_xDoc->SetDocShell( nullptr );
        m_xDoc.clear();       // we don't have the Doc anymore!!
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    }
}
void SwDocShell::InvalidateModel()
{
    // disconnect Uno-Object
    uno::Reference< text::XTextDocument >  xDoc(GetBaseModel(), uno::UNO_QUERY);
    ((SwXTextDocument*)xDoc.get())->Invalidate();
}
void SwDocShell::ReactivateModel()
{
    // disconnect Uno-Object
    uno::Reference< text::XTextDocument >  xDoc(GetBaseModel(), uno::UNO_QUERY);
    ((SwXTextDocument*)xDoc.get())->Reactivate(this);
}

// Load, Default-Format
bool  SwDocShell::Load( SfxMedium& rMedium )
{
    bool bRet = false;
    if( SfxObjectShell::Load( rMedium ))
    {
        comphelper::EmbeddedObjectContainer& rEmbeddedObjectContainer = getEmbeddedObjectContainer();
        rEmbeddedObjectContainer.setUserAllowsLinkUpdate(false);

        SAL_INFO( "sw.ui", "after SfxInPlaceObject::Load" );
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        if (mpDoc)              // for last version!!
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        if (m_xDoc.get())       // for last version!!
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
            RemoveLink();       // release the existing

        AddLink();      // set Link and update Data!!

        // Loading
        // for MD
        OSL_ENSURE( !mxBasePool.is(), "who hasn't destroyed their Pool?" );
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        mxBasePool = new SwDocStyleSheetPool( *mpDoc, SFX_CREATE_MODE_ORGANIZER == GetCreateMode() );
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        mxBasePool = new SwDocStyleSheetPool( *m_xDoc, SFX_CREATE_MODE_ORGANIZER == GetCreateMode() );
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        if(GetCreateMode() != SFX_CREATE_MODE_ORGANIZER)
        {
            SFX_ITEMSET_ARG( rMedium.GetItemSet(), pUpdateDocItem, SfxUInt16Item, SID_UPDATEDOCMODE, false);
            mnUpdateDocMode = pUpdateDocItem ? pUpdateDocItem->GetValue() : document::UpdateDocMode::NO_UPDATE;
        }

        SwWait aWait( *this, true );
        sal_uInt32 nErr = ERR_SWG_READ_ERROR;
        switch( GetCreateMode() )
        {
            case SFX_CREATE_MODE_ORGANIZER:
                {
                    if( ReadXML )
                    {
                        ReadXML->SetOrganizerMode( true );
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                        SwReader aRdr( rMedium, aEmptyOUStr, mpDoc );
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                        SwReader aRdr( rMedium, aEmptyOUStr, m_xDoc.get() );
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                        nErr = aRdr.Read( *ReadXML );
                        ReadXML->SetOrganizerMode( false );
                    }
                }
                break;

            case SFX_CREATE_MODE_INTERNAL:
            case SFX_CREATE_MODE_EMBEDDED:
                {
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                    SwTransferable::InitOle( this, *mpDoc );
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                    SwTransferable::InitOle( this, *m_xDoc );
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                }
                // suppress SfxProgress, when we are Embedded
                SW_MOD()->SetEmbeddedLoadSave( true );
                // no break;

            case SFX_CREATE_MODE_STANDARD:
            case SFX_CREATE_MODE_PREVIEW:
                {
                    Reader *pReader = ReadXML;
                    if( pReader )
                    {
                        // set Doc's DocInfo at DocShell-Medium
                        SAL_INFO( "sw.ui", "before ReadDocInfo" );
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                        SwReader aRdr( rMedium, aEmptyOUStr, mpDoc );
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                        SwReader aRdr( rMedium, aEmptyOUStr, m_xDoc.get() );
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                        SAL_INFO( "sw.ui", "before Read" );
                        nErr = aRdr.Read( *pReader );
                        SAL_INFO( "sw.ui", "after Read" );
                        // If a XML document is loaded, the global doc/web doc
                        // flags have to be set, because they aren't loaded
                        // by this formats.
                        if( ISA( SwWebDocShell ) )
                        {
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                            if( !mpDoc->getIDocumentSettingAccess().get(IDocumentSettingAccess::HTML_MODE) )
                                mpDoc->getIDocumentSettingAccess().set(IDocumentSettingAccess::HTML_MODE, true);
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                            if (!m_xDoc->getIDocumentSettingAccess().get(IDocumentSettingAccess::HTML_MODE))
                                m_xDoc->getIDocumentSettingAccess().set(IDocumentSettingAccess::HTML_MODE, true);
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                        }
                        if( ISA( SwGlobalDocShell ) )
                        {
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                            if( !mpDoc->getIDocumentSettingAccess().get(IDocumentSettingAccess::GLOBAL_DOCUMENT) )
                                mpDoc->getIDocumentSettingAccess().set(IDocumentSettingAccess::GLOBAL_DOCUMENT, true);
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                            if (!m_xDoc->getIDocumentSettingAccess().get(IDocumentSettingAccess::GLOBAL_DOCUMENT))
                                m_xDoc->getIDocumentSettingAccess().set(IDocumentSettingAccess::GLOBAL_DOCUMENT, true);
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                        }
                    }
                }
                break;

            default:
                OSL_ENSURE( false, "Load: new CreateMode?" );
        }

        UpdateFontList();
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        InitDrawModelAndDocShell(this, mpDoc ? mpDoc->getIDocumentDrawModelAccess().GetDrawModel() : 0);
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        InitDrawModelAndDocShell(this, m_xDoc.get() ? m_xDoc->getIDocumentDrawModelAccess().GetDrawModel() : nullptr);
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX

        SetError( nErr, OUString( OSL_LOG_PREFIX ) );
        bRet = !IsError( nErr );

#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        if ( bRet && !mpDoc->IsInLoadAsynchron() &&
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        if (bRet && !m_xDoc->IsInLoadAsynchron() &&
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
             GetCreateMode() == SFX_CREATE_MODE_STANDARD )
        {
            LoadingFinished();
        }

        // suppress SfxProgress, when we are Embedded
        SW_MOD()->SetEmbeddedLoadSave( false );
    }

    return bRet;
}

bool  SwDocShell::LoadFrom( SfxMedium& rMedium )
{
    bool bRet = false;
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    if( mpDoc )
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    if (m_xDoc.get())
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        RemoveLink();

    AddLink();      // set Link and update Data!!

    do {        // middle check loop
        sal_uInt32 nErr = ERR_SWG_READ_ERROR;
        OUString aStreamName = "styles.xml";
        uno::Reference < container::XNameAccess > xAccess( rMedium.GetStorage(), uno::UNO_QUERY );
        if ( xAccess->hasByName( aStreamName ) && rMedium.GetStorage()->isStreamElement( aStreamName ) )
        {
            // Loading
            SwWait aWait( *this, true );
            {
                OSL_ENSURE( !mxBasePool.is(), "who hasn't destroyed their Pool?" );
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                mxBasePool = new SwDocStyleSheetPool( *mpDoc, SFX_CREATE_MODE_ORGANIZER == GetCreateMode() );
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                mxBasePool = new SwDocStyleSheetPool( *m_xDoc, SFX_CREATE_MODE_ORGANIZER == GetCreateMode() );
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                if( ReadXML )
                {
                    ReadXML->SetOrganizerMode( true );
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                    SwReader aRdr( rMedium, aEmptyOUStr, mpDoc );
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                    SwReader aRdr( rMedium, aEmptyOUStr, m_xDoc.get() );
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                    nErr = aRdr.Read( *ReadXML );
                    ReadXML->SetOrganizerMode( false );
                }
            }
        }
        else
        {
            OSL_FAIL("Code removed!");
        }

        SetError( nErr, OUString( OSL_LOG_PREFIX ) );
        bRet = !IsError( nErr );

    } while( false );

    SfxObjectShell::LoadFrom( rMedium );
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    mpDoc->getIDocumentState().ResetModified();
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    m_xDoc->getIDocumentState().ResetModified();
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    return bRet;
}

void SwDocShell::SubInitNew()
{
    OSL_ENSURE( !mxBasePool.is(), "who hasn't destroyed their Pool?" );
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    mxBasePool = new SwDocStyleSheetPool( *mpDoc, SFX_CREATE_MODE_ORGANIZER == GetCreateMode() );
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    mxBasePool = new SwDocStyleSheetPool( *m_xDoc, SFX_CREATE_MODE_ORGANIZER == GetCreateMode() );
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    UpdateFontList();
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    InitDrawModelAndDocShell(this, mpDoc ? mpDoc->getIDocumentDrawModelAccess().GetDrawModel() : 0);
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    InitDrawModelAndDocShell(this, m_xDoc.is() ? m_xDoc->getIDocumentDrawModelAccess().GetDrawModel() : nullptr);
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX

#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    mpDoc->getIDocumentSettingAccess().setLinkUpdateMode( GLOBALSETTING );
    mpDoc->getIDocumentSettingAccess().setFieldUpdateFlags( AUTOUPD_GLOBALSETTING );
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    m_xDoc->getIDocumentSettingAccess().setLinkUpdateMode( GLOBALSETTING );
    m_xDoc->getIDocumentSettingAccess().setFieldUpdateFlags( AUTOUPD_GLOBALSETTING );
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX

    bool bWeb = ISA(SwWebDocShell);

    sal_uInt16 nRange[] =   {
        RES_PARATR_ADJUST, RES_PARATR_ADJUST,
        RES_CHRATR_COLOR, RES_CHRATR_COLOR,
        RES_CHRATR_LANGUAGE, RES_CHRATR_LANGUAGE,
        RES_CHRATR_CJK_LANGUAGE, RES_CHRATR_CJK_LANGUAGE,
        RES_CHRATR_CTL_LANGUAGE, RES_CHRATR_CTL_LANGUAGE,
        0, 0, 0  };
    if(!bWeb)
    {
        nRange[ (sizeof(nRange)/sizeof(nRange[0])) - 3 ] = RES_PARATR_TABSTOP;
        nRange[ (sizeof(nRange)/sizeof(nRange[0])) - 2 ] = RES_PARATR_HYPHENZONE;
    }
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    SfxItemSet aDfltSet( mpDoc->GetAttrPool(), nRange );
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    SfxItemSet aDfltSet( m_xDoc->GetAttrPool(), nRange );
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX

    //! get lingu options without loading lingu DLL
    SvtLinguOptions aLinguOpt;

    SvtLinguConfig().GetOptions( aLinguOpt );

    sal_Int16   nVal = MsLangId::resolveSystemLanguageByScriptType(aLinguOpt.nDefaultLanguage, ::com::sun::star::i18n::ScriptType::LATIN),
                eCJK = MsLangId::resolveSystemLanguageByScriptType(aLinguOpt.nDefaultLanguage_CJK, ::com::sun::star::i18n::ScriptType::ASIAN),
                eCTL = MsLangId::resolveSystemLanguageByScriptType(aLinguOpt.nDefaultLanguage_CTL, ::com::sun::star::i18n::ScriptType::COMPLEX);
    aDfltSet.Put( SvxLanguageItem( nVal, RES_CHRATR_LANGUAGE ) );
    aDfltSet.Put( SvxLanguageItem( eCJK, RES_CHRATR_CJK_LANGUAGE ) );
    aDfltSet.Put( SvxLanguageItem( eCTL, RES_CHRATR_CTL_LANGUAGE ) );

    if(!bWeb)
    {
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        SvxHyphenZoneItem aHyp( (SvxHyphenZoneItem&) mpDoc->GetDefault(
                                                        RES_PARATR_HYPHENZONE) );
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        SvxHyphenZoneItem aHyp( static_cast<const SvxHyphenZoneItem&>( m_xDoc->GetDefault(
                                                         RES_PARATR_HYPHENZONE)  ) );
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        aHyp.GetMinLead()   = static_cast< sal_uInt8 >(aLinguOpt.nHyphMinLeading);
        aHyp.GetMinTrail()  = static_cast< sal_uInt8 >(aLinguOpt.nHyphMinTrailing);

        aDfltSet.Put( aHyp );

        sal_uInt16 nNewPos = static_cast< sal_uInt16 >(SW_MOD()->GetUsrPref(false)->GetDefTab());
        if( nNewPos )
            aDfltSet.Put( SvxTabStopItem( 1, nNewPos,
                                          SVX_TAB_ADJUST_DEFAULT, RES_PARATR_TABSTOP ) );
    }
    aDfltSet.Put( SvxColorItem( Color( COL_AUTO ), RES_CHRATR_COLOR ) );

#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    mpDoc->SetDefault( aDfltSet );
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    m_xDoc->SetDefault( aDfltSet );
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX

    //default page mode for text grid
    if(!bWeb)
    {
        bool bSquaredPageMode = SW_MOD()->GetUsrPref(false)->IsSquaredPageMode();
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        mpDoc->SetDefaultPageMode( bSquaredPageMode );
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        m_xDoc->SetDefaultPageMode( bSquaredPageMode );
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    }

#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    mpDoc->getIDocumentState().ResetModified();
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    m_xDoc->getIDocumentState().ResetModified();
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
}

/*
 * Document Interface Access
 */
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
IDocumentDeviceAccess* SwDocShell::getIDocumentDeviceAccess() { return &mpDoc->getIDocumentDeviceAccess(); }
const IDocumentSettingAccess* SwDocShell::getIDocumentSettingAccess() const { return &mpDoc->getIDocumentSettingAccess(); }
IDocumentChartDataProviderAccess* SwDocShell::getIDocumentChartDataProviderAccess() { return &mpDoc->getIDocumentChartDataProviderAccess(); }
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
IDocumentDeviceAccess* SwDocShell::getIDocumentDeviceAccess() { return &m_xDoc->getIDocumentDeviceAccess(); }
const IDocumentSettingAccess* SwDocShell::getIDocumentSettingAccess() const { return &m_xDoc->getIDocumentSettingAccess(); }
IDocumentChartDataProviderAccess* SwDocShell::getIDocumentChartDataProviderAccess() { return &m_xDoc->getIDocumentChartDataProviderAccess(); }
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
