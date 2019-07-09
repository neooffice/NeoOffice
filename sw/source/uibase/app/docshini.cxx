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
#include <unotools/configmgr.hxx>
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
#include <editeng/orphitem.hxx>
#include <editeng/widwitem.hxx>
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
#include <drawdoc.hxx>

#include <svx/CommonStyleManager.hxx>

// text grid
#include <tgrditem.hxx>
#include <memory>

using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;

// Load Document
bool SwDocShell::InitNew( const uno::Reference < embed::XStorage >& xStor )
{
    bool bRet = SfxObjectShell::InitNew( xStor );
    OSL_ENSURE( GetMapUnit() == MapUnit::MapTwip, "map unit is not twip!" );
    bool bHTMLTemplSet = false;
    if( bRet )
    {
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        AddLink();      // create m_pDoc / pIo if applicable
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        AddLink();      // create m_xDoc / pIo if applicable
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX

        bool bWeb = dynamic_cast< const SwWebDocShell *>( this ) !=  nullptr;
        if ( bWeb )
            bHTMLTemplSet = SetHTMLTemplate( *GetDoc() );// Styles from HTML.vor
        else if( dynamic_cast< const SwGlobalDocShell *>( this ) !=  nullptr )
            GetDoc()->getIDocumentSettingAccess().set(DocumentSettingId::GLOBAL_DOCUMENT, true);       // Globaldokument

        if ( GetCreateMode() ==  SfxObjectCreateMode::EMBEDDED )
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
            SwTransferable::InitOle( this, *m_pDoc );
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
            SwTransferable::InitOle( this, *m_xDoc );
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX

        // set forbidden characters if necessary
        if (!utl::ConfigManager::IsAvoidConfig())
        {
            SvxAsianConfig aAsian;
            Sequence<lang::Locale> aLocales =  aAsian.GetStartEndCharLocales();
            if (aLocales.getLength())
            {
                const lang::Locale* pLocales = aLocales.getConstArray();
                for(sal_Int32 i = 0; i < aLocales.getLength(); i++)
                {
                    ForbiddenCharacters aForbidden;
                    aAsian.GetStartEndChars( pLocales[i], aForbidden.beginLine, aForbidden.endLine);
                    LanguageType  eLang = LanguageTag::convertToLanguageType(pLocales[i]);
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                    m_pDoc->getIDocumentSettingAccess().setForbiddenCharacters( eLang, aForbidden);
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                    m_xDoc->getIDocumentSettingAccess().setForbiddenCharacters( eLang, aForbidden);
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                }
            }
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
            m_pDoc->getIDocumentSettingAccess().set(DocumentSettingId::KERN_ASIAN_PUNCTUATION,
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
            m_xDoc->getIDocumentSettingAccess().set(DocumentSettingId::KERN_ASIAN_PUNCTUATION,
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                  !aAsian.IsKerningWesternTextOnly());
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
            m_pDoc->getIDocumentSettingAccess().setCharacterCompressionType(aAsian.GetCharDistanceCompression());
            m_pDoc->getIDocumentDeviceAccess().setPrintData(*SW_MOD()->GetPrtOptions(bWeb));
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
            m_xDoc->getIDocumentSettingAccess().setCharacterCompressionType(aAsian.GetCharDistanceCompression());
            m_xDoc->getIDocumentDeviceAccess().setPrintData(*SW_MOD()->GetPrtOptions(bWeb));
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        }

        SubInitNew();

        // for all

        SwStdFontConfig* pStdFont = SW_MOD()->GetStdFontConfig();
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        SfxPrinter* pPrt = m_pDoc->getIDocumentDeviceAccess().getPrinter( false );
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        SfxPrinter* pPrt = m_xDoc->getIDocumentDeviceAccess().getPrinter( false );
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX

        OUString sEntry;
        static const sal_uInt16 aFontWhich[] =
        {   RES_CHRATR_FONT,
            RES_CHRATR_CJK_FONT,
            RES_CHRATR_CTL_FONT
        };
        static const sal_uInt16 aFontHeightWhich[] =
        {
            RES_CHRATR_FONTSIZE,
            RES_CHRATR_CJK_FONTSIZE,
            RES_CHRATR_CTL_FONTSIZE
        };
        static const sal_uInt16 aFontIds[] =
        {
            FONT_STANDARD,
            FONT_STANDARD_CJK,
            FONT_STANDARD_CTL
        };
        static const DefaultFontType nFontTypes[] =
        {
            DefaultFontType::LATIN_TEXT,
            DefaultFontType::CJK_TEXT,
            DefaultFontType::CTL_TEXT
        };
        static const sal_uInt16 aLangTypes[] =
        {
            RES_CHRATR_LANGUAGE,
            RES_CHRATR_CJK_LANGUAGE,
            RES_CHRATR_CTL_LANGUAGE
        };

        for(sal_uInt8 i = 0; i < 3; i++)
        {
            sal_uInt16 nFontWhich = aFontWhich[i];
            sal_uInt16 nFontId = aFontIds[i];
            std::unique_ptr<SvxFontItem> pFontItem;
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
            const SvxLanguageItem& rLang = static_cast<const SvxLanguageItem&>(m_pDoc->GetDefault( aLangTypes[i] ));
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

                pFontItem.reset(new SvxFontItem(aFont.GetFamilyType(), aFont.GetFamilyName(),
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
                    GetDefaultFontFlags::OnlyOne );
                pFontItem.reset(new SvxFontItem(aLangDefFont.GetFamilyType(), aLangDefFont.GetFamilyName(),
                                                aEmptyOUStr, aLangDefFont.GetPitch(), aLangDefFont.GetCharSet(), nFontWhich));
            }
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
            m_pDoc->SetDefault(*pFontItem);
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
            m_xDoc->SetDefault(*pFontItem);
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
            if( !bHTMLTemplSet )
            {
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                SwTextFormatColl *pColl = m_pDoc->getIDocumentStylePoolAccess().GetTextCollFromPool(RES_POOLCOLL_STANDARD);
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                SwTextFormatColl *pColl = m_xDoc->getIDocumentStylePoolAccess().GetTextCollFromPool(RES_POOLCOLL_STANDARD);
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                pColl->ResetFormatAttr(nFontWhich);
            }
            pFontItem.reset();
            sal_Int32 nFontHeight = pStdFont->GetFontHeight( FONT_STANDARD, i, eLanguage );
            if(nFontHeight <= 0)
                nFontHeight = SwStdFontConfig::GetDefaultHeightFor( nFontId, eLanguage );
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
            m_pDoc->SetDefault(SvxFontHeightItem( nFontHeight, 100, aFontHeightWhich[i] ));
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
            m_xDoc->SetDefault(SvxFontHeightItem( nFontHeight, 100, aFontHeightWhich[i] ));
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
            if( !bHTMLTemplSet )
            {
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                SwTextFormatColl *pColl = m_pDoc->getIDocumentStylePoolAccess().GetTextCollFromPool(RES_POOLCOLL_STANDARD);
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                SwTextFormatColl *pColl = m_xDoc->getIDocumentStylePoolAccess().GetTextCollFromPool(RES_POOLCOLL_STANDARD);
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                pColl->ResetFormatAttr(aFontHeightWhich[i]);
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
        LanguageType eLanguage = static_cast<const SvxLanguageItem&>(m_pDoc->GetDefault( RES_CHRATR_LANGUAGE )).GetLanguage();
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
                eLanguage = static_cast<const SvxLanguageItem&>(m_pDoc->GetDefault( RES_CHRATR_CJK_LANGUAGE )).GetLanguage();
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                eLanguage = static_cast<const SvxLanguageItem&>(m_xDoc->GetDefault( RES_CHRATR_CJK_LANGUAGE )).GetLanguage();
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
            }
            else if(nIdx == 16)
            {
                nFontWhich = RES_CHRATR_CTL_FONT;
                nFontHeightWhich = RES_CHRATR_CTL_FONTSIZE;
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                eLanguage = static_cast<const SvxLanguageItem&>(m_pDoc->GetDefault( RES_CHRATR_CTL_LANGUAGE )).GetLanguage();
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                eLanguage = static_cast<const SvxLanguageItem&>(m_xDoc->GetDefault( RES_CHRATR_CTL_LANGUAGE )).GetLanguage();
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
            }
            SwTextFormatColl *pColl = nullptr;
            if(!pStdFont->IsFontDefault(aFontIdPoolId[nIdx]))
            {
                sEntry = pStdFont->GetFontFor(aFontIdPoolId[nIdx]);

                vcl::Font aFont( sEntry, Size( 0, 10 ) );
                if( pPrt )
                    aFont = pPrt->GetFontMetric( aFont );

#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                pColl = m_pDoc->getIDocumentStylePoolAccess().GetTextCollFromPool(aFontIdPoolId[nIdx + 1]);
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                pColl = m_xDoc->getIDocumentStylePoolAccess().GetTextCollFromPool(aFontIdPoolId[nIdx + 1]);
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                if( !bHTMLTemplSet ||
                    SfxItemState::SET != pColl->GetAttrSet().GetItemState(
                                                    nFontWhich, false ) )
                {
                    pColl->SetFormatAttr(SvxFontItem(aFont.GetFamilyType(), aFont.GetFamilyName(),
                                                  aEmptyOUStr, aFont.GetPitch(), aFont.GetCharSet(), nFontWhich));
                }
            }
            sal_Int32 nFontHeight = pStdFont->GetFontHeight( static_cast< sal_Int8 >(aFontIdPoolId[nIdx]), 0, eLanguage );
            if(nFontHeight <= 0)
                nFontHeight = SwStdFontConfig::GetDefaultHeightFor( aFontIdPoolId[nIdx], eLanguage );
            if(!pColl)
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                pColl = m_pDoc->getIDocumentStylePoolAccess().GetTextCollFromPool(aFontIdPoolId[nIdx + 1]);
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                pColl = m_xDoc->getIDocumentStylePoolAccess().GetTextCollFromPool(aFontIdPoolId[nIdx + 1]);
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
            SvxFontHeightItem aFontHeight( static_cast<const SvxFontHeightItem&>(pColl->GetFormatAttr( nFontHeightWhich )));
            if(aFontHeight.GetHeight() != sal::static_int_cast<sal_uInt32, sal_Int32>(nFontHeight))
            {
                aFontHeight.SetHeight(nFontHeight);
                pColl->SetFormatAttr( aFontHeight );
            }
        }

        // the default for documents created via 'File/New' should be 'on'
        // (old documents, where this property was not yet implemented, will get the
        // value 'false' in the SwDoc c-tor)
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        m_pDoc->getIDocumentSettingAccess().set( DocumentSettingId::MATH_BASELINE_ALIGNMENT,
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        m_xDoc->getIDocumentSettingAccess().set( DocumentSettingId::MATH_BASELINE_ALIGNMENT,
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                SW_MOD()->GetUsrPref( bWeb )->IsAlignMathObjectsToBaseline() );
    }

    /* #106748# If the default frame direction of a document is RTL
        the default adjustment is to the right. */
    if( !bHTMLTemplSet &&
        SvxFrameDirection::Horizontal_RL_TB == GetDefaultFrameDirection(GetAppLanguage()) )
    {
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        m_pDoc->SetDefault( SvxAdjustItem(SvxAdjust::Right, RES_PARATR_ADJUST ) );
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        m_xDoc->SetDefault( SvxAdjustItem(SvxAdjust::Right, RES_PARATR_ADJUST ) );
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    }

// #i29550#
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    m_pDoc->SetDefault( SfxBoolItem( RES_COLLAPSING_BORDERS, true ) );
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    m_xDoc->SetDefault( SfxBoolItem( RES_COLLAPSING_BORDERS, true ) );
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
// <-- collapsing

    //#i16874# AutoKerning as default for new documents
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    m_pDoc->SetDefault( SvxAutoKernItem( true, RES_CHRATR_AUTOKERN ) );
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    m_xDoc->SetDefault( SvxAutoKernItem( true, RES_CHRATR_AUTOKERN ) );
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX

    // #i42080# - Due to the several calls of method <SetDefault(..)>
    // at the document instance, the document is modified. Thus, reset this
    // status here. Note: In method <SubInitNew()> this is also done.
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    m_pDoc->getIDocumentState().ResetModified();
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    m_xDoc->getIDocumentState().ResetModified();
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX

    return bRet;
}

// Ctor with SfxCreateMode ?????
SwDocShell::SwDocShell( SfxObjectCreateMode const eMode )
    : SfxObjectShell(eMode)
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    , m_pDoc(nullptr)
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    , m_pFontList(nullptr)
    , m_IsInUpdateFontList(false)
    , m_pStyleManager(new svx::CommonStyleManager(*this))
    , m_pView(nullptr)
    , m_pWrtShell(nullptr)
    , m_pOLEChildList(nullptr)
    , m_nUpdateDocMode(document::UpdateDocMode::ACCORDING_TO_CONFIG)
    , m_IsATemplate(false)
    , m_IsRemovedInvisibleContent(false)
{
    Init_Impl();
}

// Ctor / Dtor
SwDocShell::SwDocShell( const SfxModelFlags i_nSfxCreationFlags )
    : SfxObjectShell ( i_nSfxCreationFlags )
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    , m_pDoc(nullptr)
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    , m_pFontList(nullptr)
    , m_IsInUpdateFontList(false)
    , m_pStyleManager(new svx::CommonStyleManager(*this))
    , m_pView(nullptr)
    , m_pWrtShell(nullptr)
    , m_pOLEChildList(nullptr)
    , m_nUpdateDocMode(document::UpdateDocMode::ACCORDING_TO_CONFIG)
    , m_IsATemplate(false)
    , m_IsRemovedInvisibleContent(false)
{
    Init_Impl();
}

// Ctor / Dtor
SwDocShell::SwDocShell( SwDoc *const pD, SfxObjectCreateMode const eMode )
    : SfxObjectShell(eMode)
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    , m_pDoc(pD)
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    , m_xDoc(pD)
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    , m_pFontList(nullptr)
    , m_IsInUpdateFontList(false)
    , m_pStyleManager(new svx::CommonStyleManager(*this))
    , m_pView(nullptr)
    , m_pWrtShell(nullptr)
    , m_pOLEChildList(nullptr)
    , m_nUpdateDocMode(document::UpdateDocMode::ACCORDING_TO_CONFIG)
    , m_IsATemplate(false)
    , m_IsRemovedInvisibleContent(false)
{
    Init_Impl();
}

// Dtor
SwDocShell::~SwDocShell()
{
    // disable chart related objects now because in ~SwDoc it may be to late for this
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    if (m_pDoc)
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    if (m_xDoc.get())
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    {
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        m_pDoc->getIDocumentChartDataProviderAccess().GetChartControllerHelper().Disconnect();
        SwChartDataProvider *pPCD = m_pDoc->getIDocumentChartDataProviderAccess().GetChartDataProvider();
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        m_xDoc->getIDocumentChartDataProviderAccess().GetChartControllerHelper().Disconnect();
        SwChartDataProvider *pPCD = m_xDoc->getIDocumentChartDataProviderAccess().GetChartDataProvider();
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        if (pPCD)
            pPCD->dispose();
    }

    RemoveLink();
    delete m_pFontList;

    // we, as BroadCaster also become our own Listener
    // (for DocInfo/FileNames/....)
    EndListening( *this );

    delete m_pOLEChildList;
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
    SetMapUnit( MapUnit::MapTwip );
}

void SwDocShell::AddLink()
{
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    if (!m_pDoc)
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    if (!m_xDoc.get())
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    {
        SwDocFac aFactory;
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        m_pDoc = aFactory.GetDoc();
        m_pDoc->acquire();
        m_pDoc->getIDocumentSettingAccess().set(DocumentSettingId::HTML_MODE, dynamic_cast< const SwWebDocShell *>( this ) !=  nullptr );
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        m_xDoc = aFactory.GetDoc();
        m_xDoc->getIDocumentSettingAccess().set(DocumentSettingId::HTML_MODE, dynamic_cast< const SwWebDocShell *>( this ) !=  nullptr );
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    }
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    else
        m_pDoc->acquire();
    m_pDoc->SetDocShell( this );      // set the DocShell-Pointer for Doc
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    m_xDoc->SetDocShell( this );      // set the DocShell-Pointer for Doc
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    uno::Reference< text::XTextDocument >  xDoc(GetBaseModel(), uno::UNO_QUERY);
    static_cast<SwXTextDocument*>(xDoc.get())->Reactivate(this);

#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    SetPool(&m_pDoc->GetAttrPool());
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    SetPool(&m_xDoc->GetAttrPool());
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX

    // most suitably not until a sdbcx::View is created!!!
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    m_pDoc->SetOle2Link(LINK(this, SwDocShell, Ole2ModifiedHdl));
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    m_xDoc->SetOle2Link(LINK(this, SwDocShell, Ole2ModifiedHdl));
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
}

// create new FontList Change Printer
void SwDocShell::UpdateFontList()
{
    if (!m_IsInUpdateFontList)
    {
        m_IsInUpdateFontList = true;
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        OSL_ENSURE(m_pDoc, "No Doc no FontList");
        if (m_pDoc)
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        OSL_ENSURE(m_xDoc.get(), "No Doc no FontList");
        if (m_xDoc.get())
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        {
            delete m_pFontList;
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
            m_pFontList = new FontList( m_pDoc->getIDocumentDeviceAccess().getReferenceDevice(true) );
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
            m_pFontList = new FontList( m_xDoc->getIDocumentDeviceAccess().getReferenceDevice(true) );
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
            PutItem( SvxFontListItem( m_pFontList, SID_ATTR_CHAR_FONTLIST ) );
        }
        m_IsInUpdateFontList = false;
    }
}

void SwDocShell::RemoveLink()
{
    // disconnect Uno-Object
    uno::Reference< text::XTextDocument >  xDoc(GetBaseModel(), uno::UNO_QUERY);
    static_cast<SwXTextDocument*>(xDoc.get())->Invalidate();
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    if (m_pDoc)
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    if (m_xDoc.get())
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    {
        if (m_xBasePool.is())
        {
            static_cast<SwDocStyleSheetPool*>(m_xBasePool.get())->dispose();
            m_xBasePool.clear();
        }
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        sal_Int8 nRefCt = static_cast< sal_Int8 >(m_pDoc->release());
        m_pDoc->SetOle2Link(Link<bool,void>());
        m_pDoc->SetDocShell( nullptr );
        if( !nRefCt )
            delete m_pDoc;
        m_pDoc = nullptr;       // we don't have the Doc anymore!!
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        m_xDoc->SetOle2Link(Link<bool,void>());
        m_xDoc->SetDocShell( nullptr );
        m_xDoc.clear();       // we don't have the Doc anymore!!
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    }
}
void SwDocShell::InvalidateModel()
{
    // disconnect Uno-Object
    uno::Reference< text::XTextDocument >  xDoc(GetBaseModel(), uno::UNO_QUERY);
    static_cast<SwXTextDocument*>(xDoc.get())->Invalidate();
}
void SwDocShell::ReactivateModel()
{
    // disconnect Uno-Object
    uno::Reference< text::XTextDocument >  xDoc(GetBaseModel(), uno::UNO_QUERY);
    static_cast<SwXTextDocument*>(xDoc.get())->Reactivate(this);
}

// Load, Default-Format
bool  SwDocShell::Load( SfxMedium& rMedium )
{
    bool bRet = false;

    // If this is an ODF file being loaded, then by default, use legacy processing
    // for tdf#99729 (if required, it will be overridden in *::ReadUserDataSequence())
    if (IsOwnStorageFormat(rMedium))
    {
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        if (m_pDoc && m_pDoc->getIDocumentDrawModelAccess().GetDrawModel())
            m_pDoc->getIDocumentDrawModelAccess().GetDrawModel()->SetAnchoredTextOverflowLegacy(true);
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        if (m_xDoc.get() && m_xDoc->getIDocumentDrawModelAccess().GetDrawModel())
            m_xDoc->getIDocumentDrawModelAccess().GetDrawModel()->SetAnchoredTextOverflowLegacy(true);
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    }

    if (SfxObjectShell::Load(rMedium))
    {
        comphelper::EmbeddedObjectContainer& rEmbeddedObjectContainer = getEmbeddedObjectContainer();
        rEmbeddedObjectContainer.setUserAllowsLinkUpdate(false);

        SAL_INFO( "sw.ui", "after SfxInPlaceObject::Load" );
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        if (m_pDoc)              // for last version!!
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        if (m_xDoc.get())       // for last version!!
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
            RemoveLink();       // release the existing

        AddLink();      // set Link and update Data!!

        // Loading
        // for MD
        OSL_ENSURE( !m_xBasePool.is(), "who hasn't destroyed their Pool?" );
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        m_xBasePool = new SwDocStyleSheetPool( *m_pDoc, SfxObjectCreateMode::ORGANIZER == GetCreateMode() );
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        m_xBasePool = new SwDocStyleSheetPool( *m_xDoc, SfxObjectCreateMode::ORGANIZER == GetCreateMode() );
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        if(GetCreateMode() != SfxObjectCreateMode::ORGANIZER)
        {
            const SfxUInt16Item* pUpdateDocItem = SfxItemSet::GetItem<SfxUInt16Item>(rMedium.GetItemSet(), SID_UPDATEDOCMODE, false);
            m_nUpdateDocMode = pUpdateDocItem ? pUpdateDocItem->GetValue() : document::UpdateDocMode::NO_UPDATE;
        }

        SwWait aWait( *this, true );
        sal_uInt32 nErr = ERR_SWG_READ_ERROR;
        switch( GetCreateMode() )
        {
            case SfxObjectCreateMode::ORGANIZER:
                {
                    if( ReadXML )
                    {
                        ReadXML->SetOrganizerMode( true );
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                        SwReader aRdr( rMedium, aEmptyOUStr, m_pDoc );
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                        SwReader aRdr( rMedium, aEmptyOUStr, m_xDoc.get() );
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                        nErr = aRdr.Read( *ReadXML );
                        ReadXML->SetOrganizerMode( false );
                    }
                }
                break;

            case SfxObjectCreateMode::INTERNAL:
            case SfxObjectCreateMode::EMBEDDED:
                {
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                    SwTransferable::InitOle( this, *m_pDoc );
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                    SwTransferable::InitOle( this, *m_xDoc );
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                }
                // suppress SfxProgress, when we are Embedded
                SW_MOD()->SetEmbeddedLoadSave( true );
                SAL_FALLTHROUGH;

            case SfxObjectCreateMode::STANDARD:
                {
                    Reader *pReader = ReadXML;
                    if( pReader )
                    {
                        // set Doc's DocInfo at DocShell-Medium
                        SAL_INFO( "sw.ui", "before ReadDocInfo" );
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                        SwReader aRdr( rMedium, aEmptyOUStr, m_pDoc );
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                        SwReader aRdr( rMedium, aEmptyOUStr, m_xDoc.get() );
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                        SAL_INFO( "sw.ui", "before Read" );
                        nErr = aRdr.Read( *pReader );
                        SAL_INFO( "sw.ui", "after Read" );
                        // If a XML document is loaded, the global doc/web doc
                        // flags have to be set, because they aren't loaded
                        // by this formats.
                        if( dynamic_cast< const SwWebDocShell *>( this ) !=  nullptr )
                        {
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                            if (!m_pDoc->getIDocumentSettingAccess().get(DocumentSettingId::HTML_MODE))
                                m_pDoc->getIDocumentSettingAccess().set(DocumentSettingId::HTML_MODE, true);
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                            if (!m_xDoc->getIDocumentSettingAccess().get(DocumentSettingId::HTML_MODE))
                                m_xDoc->getIDocumentSettingAccess().set(DocumentSettingId::HTML_MODE, true);
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                        }
                        if( dynamic_cast< const SwGlobalDocShell *>( this ) !=  nullptr )
                        {
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                            if (!m_pDoc->getIDocumentSettingAccess().get(DocumentSettingId::GLOBAL_DOCUMENT))
                                m_pDoc->getIDocumentSettingAccess().set(DocumentSettingId::GLOBAL_DOCUMENT, true);
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                            if (!m_xDoc->getIDocumentSettingAccess().get(DocumentSettingId::GLOBAL_DOCUMENT))
                                m_xDoc->getIDocumentSettingAccess().set(DocumentSettingId::GLOBAL_DOCUMENT, true);
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
        InitDrawModelAndDocShell(this, m_pDoc ? m_pDoc->getIDocumentDrawModelAccess().GetDrawModel() : nullptr);
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        InitDrawModelAndDocShell(this, m_xDoc.get() ? m_xDoc->getIDocumentDrawModelAccess().GetDrawModel() : nullptr);
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX

        SetError(nErr);
        bRet = !IsError( nErr );

#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        if (bRet && !m_pDoc->IsInLoadAsynchron() &&
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        if (bRet && !m_xDoc->IsInLoadAsynchron() &&
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
            GetCreateMode() == SfxObjectCreateMode::STANDARD)
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
    if (m_pDoc)
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
                OSL_ENSURE( !m_xBasePool.is(), "who hasn't destroyed their Pool?" );
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                m_xBasePool = new SwDocStyleSheetPool( *m_pDoc, SfxObjectCreateMode::ORGANIZER == GetCreateMode() );
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                m_xBasePool = new SwDocStyleSheetPool( *m_xDoc, SfxObjectCreateMode::ORGANIZER == GetCreateMode() );
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                if( ReadXML )
                {
                    ReadXML->SetOrganizerMode( true );
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                    SwReader aRdr( rMedium, aEmptyOUStr, m_pDoc );
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

        SetError(nErr);
        bRet = !IsError( nErr );

    } while( false );

    SfxObjectShell::LoadFrom( rMedium );
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    m_pDoc->getIDocumentState().ResetModified();
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    m_xDoc->getIDocumentState().ResetModified();
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    return bRet;
}

void SwDocShell::SubInitNew()
{
    OSL_ENSURE( !m_xBasePool.is(), "who hasn't destroyed their Pool?" );
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    m_xBasePool = new SwDocStyleSheetPool( *m_pDoc, SfxObjectCreateMode::ORGANIZER == GetCreateMode() );
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    m_xBasePool = new SwDocStyleSheetPool( *m_xDoc, SfxObjectCreateMode::ORGANIZER == GetCreateMode() );
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    UpdateFontList();
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    InitDrawModelAndDocShell(this, m_pDoc ? m_pDoc->getIDocumentDrawModelAccess().GetDrawModel() : nullptr);
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    InitDrawModelAndDocShell(this, m_xDoc ? m_xDoc->getIDocumentDrawModelAccess().GetDrawModel() : nullptr);
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX

#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    m_pDoc->getIDocumentSettingAccess().setLinkUpdateMode( GLOBALSETTING );
    m_pDoc->getIDocumentSettingAccess().setFieldUpdateFlags( AUTOUPD_GLOBALSETTING );
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    m_xDoc->getIDocumentSettingAccess().setLinkUpdateMode( GLOBALSETTING );
    m_xDoc->getIDocumentSettingAccess().setFieldUpdateFlags( AUTOUPD_GLOBALSETTING );
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX

    bool bWeb = dynamic_cast< const SwWebDocShell *>( this ) !=  nullptr;

    sal_uInt16 nRange[] =   {
        RES_PARATR_ADJUST, RES_PARATR_ADJUST,
        RES_CHRATR_COLOR, RES_CHRATR_COLOR,
        RES_CHRATR_LANGUAGE, RES_CHRATR_LANGUAGE,
        RES_CHRATR_CJK_LANGUAGE, RES_CHRATR_CJK_LANGUAGE,
        RES_CHRATR_CTL_LANGUAGE, RES_CHRATR_CTL_LANGUAGE,
        0, 0, 0  };
    if(!bWeb)
    {
        nRange[ SAL_N_ELEMENTS(nRange) - 3 ] = RES_PARATR_TABSTOP;
        nRange[ SAL_N_ELEMENTS(nRange) - 2 ] = RES_PARATR_HYPHENZONE;
    }
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    SfxItemSet aDfltSet( m_pDoc->GetAttrPool(), nRange );
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    SfxItemSet aDfltSet( m_xDoc->GetAttrPool(), nRange );
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX

    //! get lingu options without loading lingu DLL
    SvtLinguOptions aLinguOpt;

    if (!utl::ConfigManager::IsAvoidConfig())
        SvtLinguConfig().GetOptions(aLinguOpt);

    LanguageType nVal = MsLangId::resolveSystemLanguageByScriptType(aLinguOpt.nDefaultLanguage, css::i18n::ScriptType::LATIN),
                 eCJK = MsLangId::resolveSystemLanguageByScriptType(aLinguOpt.nDefaultLanguage_CJK, css::i18n::ScriptType::ASIAN),
                 eCTL = MsLangId::resolveSystemLanguageByScriptType(aLinguOpt.nDefaultLanguage_CTL, css::i18n::ScriptType::COMPLEX);
    aDfltSet.Put( SvxLanguageItem( nVal, RES_CHRATR_LANGUAGE ) );
    aDfltSet.Put( SvxLanguageItem( eCJK, RES_CHRATR_CJK_LANGUAGE ) );
    aDfltSet.Put( SvxLanguageItem( eCTL, RES_CHRATR_CTL_LANGUAGE ) );

    if(!bWeb)
    {
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        SvxHyphenZoneItem aHyp( static_cast<const SvxHyphenZoneItem&>( m_pDoc->GetDefault(
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        SvxHyphenZoneItem aHyp( static_cast<const SvxHyphenZoneItem&>( m_xDoc->GetDefault(
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                                                        RES_PARATR_HYPHENZONE)  ) );
        aHyp.GetMinLead()   = static_cast< sal_uInt8 >(aLinguOpt.nHyphMinLeading);
        aHyp.GetMinTrail()  = static_cast< sal_uInt8 >(aLinguOpt.nHyphMinTrailing);

        aDfltSet.Put( aHyp );

        sal_uInt16 nNewPos = static_cast< sal_uInt16 >(SW_MOD()->GetUsrPref(false)->GetDefTab());
        if( nNewPos )
            aDfltSet.Put( SvxTabStopItem( 1, nNewPos,
                                          SvxTabAdjust::Default, RES_PARATR_TABSTOP ) );
    }
    aDfltSet.Put( SvxColorItem( Color( COL_AUTO ), RES_CHRATR_COLOR ) );

#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    m_pDoc->SetDefault( aDfltSet );
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    m_xDoc->SetDefault( aDfltSet );
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX

    //default page mode for text grid
    if(!bWeb)
    {
        bool bSquaredPageMode = SW_MOD()->GetUsrPref(false)->IsSquaredPageMode();
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        m_pDoc->SetDefaultPageMode( bSquaredPageMode );
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        m_xDoc->SetDefaultPageMode( bSquaredPageMode );
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX

        // only set Widow/Orphan defaults on a new, non-web document - not an opened one
        if( GetMedium() && GetMedium()->GetOrigURL().isEmpty() )
        {
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
            m_pDoc->SetDefault( SvxWidowsItem(  (sal_uInt8) 2, RES_PARATR_WIDOWS)  );
            m_pDoc->SetDefault( SvxOrphansItem( (sal_uInt8) 2, RES_PARATR_ORPHANS) );
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
            m_xDoc->SetDefault( SvxWidowsItem(  (sal_uInt8) 2, RES_PARATR_WIDOWS)  );
            m_xDoc->SetDefault( SvxOrphansItem( (sal_uInt8) 2, RES_PARATR_ORPHANS) );
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        }
    }

#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    m_pDoc->getIDocumentState().ResetModified();
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    m_xDoc->getIDocumentState().ResetModified();
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
}

/*
 * Document Interface Access
 */
IDocumentDeviceAccess& SwDocShell::getIDocumentDeviceAccess()
{
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    return m_pDoc->getIDocumentDeviceAccess();
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    return m_xDoc->getIDocumentDeviceAccess();
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
}

IDocumentChartDataProviderAccess& SwDocShell::getIDocumentChartDataProviderAccess()
{
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
     return m_pDoc->getIDocumentChartDataProviderAccess();
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
     return m_xDoc->getIDocumentChartDataProviderAccess();
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
