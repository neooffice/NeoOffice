/*************************************************************************
 *
 *  $RCSfile$
 *
 *  $Revision$
 *
 *  last change: $Author$ $Date$
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU General Public License Version 2.1
 *
 *  Edward Peterlin, January 2005
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2005 by Edward Peterlin (OPENSTEP@neooffice.org)
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 ************************************************************************/

#ifdef PRECOMPILED
#include "ofapch.hxx"
#endif

#pragma hdrstop

#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#include <vcl/config.hxx>
#ifndef _SFXFLAGITEM_HXX
#include <svtools/flagitem.hxx>
#endif
#ifndef _SFX_WHITER_HXX
#include <svtools/whiter.hxx>
#endif
#ifndef _SFXDISPATCH_HXX
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SFXREQUEST_HXX
#include <sfx2/request.hxx>
#endif
#ifndef _SFXVIEWSH_HXX
#include <sfx2/viewsh.hxx>
#endif
#ifndef _SFXMODULE_HXX
#include <sfx2/module.hxx>
#endif
#ifndef _SFX_PRINTOPT_HXX
#include <sfx2/printopt.hxx>
#endif
#ifndef _SVX_ADRITEM_HXX
#include <svx/adritem.hxx>
#endif
#ifndef _SVTOOLS_LANGUAGEOPTIONS_HXX
#include <svtools/languageoptions.hxx>
#endif

#ifndef _SFXGENLINK_HXX //autogen
#include <sfx2/genlink.hxx>
#endif
#ifndef _SVX_OPTGENRL_HXX
#include <svx/optgenrl.hxx>
#endif
#ifndef _SVX_FONT_SUBSTITUTION_HXX //autogen
#include <fontsubs.hxx>
#endif
#ifndef _SBXCORE_HXX //autogen
#include <svtools/sbxcore.hxx>
#endif

#ifndef _SFXSIDS_HRC
#include <sfx2/sfxsids.hrc>
#endif
#ifndef _SVX_OPTITEMS_HXX
#define ITEMID_SPELLCHECK   SID_ATTR_SPELL
#define ITEMID_HYPHENREGION SID_ATTR_HYPHENREGION
#include <svx/optitems.hxx>
#endif
#ifndef _SVX_HYZNITEM_HXX
#define  ITEMID_HYPHENZONE SID_ATTR_PARA_HYPHENZONE
#include <svx/hyznitem.hxx>
#endif
#ifndef _SVX_LANGITEM_HXX
#define	ITEMID_LANGUAGE SID_ATTR_CHAR_LANGUAGE
#include <svx/langitem.hxx>
#endif
#ifndef _SVX_OPTCOLOR_HXX
#include <svx/optcolor.hxx>
#endif
#ifndef _SVX_OPTCTL_HXX
#include <svx/optctl.hxx>
#endif

#ifndef _XTABLE_HXX
#include <svx/xtable.hxx>
#endif
#ifndef _SFXINTITEM_HXX //autogen
#include <svtools/intitem.hxx>
#endif
#ifndef _PVER_HXX //autogen
#include <svtools/pver.hxx>
#endif
#ifndef _SVX_DIALOGS_HRC
#include <svx/dialogs.hrc>
#endif
#include <sfx2/misccfg.hxx>

#ifndef _SVX_OPTSAVE_HXX //autogen
#include <svx/optsave.hxx>
#endif
#ifndef _SVX_OPTPATH_HXX //autogen
#include <svx/optpath.hxx>
#endif
#ifndef _SVX_OPTGENRL_HXX //autogen
#include <svx/optgenrl.hxx>
#endif
#ifndef _SVX_OPTLINGU_HXX //autogen
#include <svx/optlingu.hxx>
#endif
#ifndef _SVX_TAB_AREA_HXX //autogen
#include <svx/tabarea.hxx>
#endif
#ifndef _SVX_OPTINET_HXX //autogen
#include <svx/optinet2.hxx>
#endif
#ifndef _SVX_OPTEXTBR_HXX //autogen
#include <svx/optextbr.hxx>
#endif
#ifndef _SVX_OPTASIAN_HXX
#include <svx/optasian.hxx>
#endif
#ifndef _SVX_OPTACCESSIBILITY_HXX
#include <svx/optaccessibility.hxx>
#endif
#ifndef _SVX_OPTJSEARCH_HXX_
#include <svx/optjsearch.hxx>
#endif
#ifndef _OFFAPP_CONNPOOLOPTIONS_HXX_
#include "connpooloptions.hxx"
#endif
#ifndef _OFFAPP_CONNPOOLCONFIG_HXX_
#include "connpoolconfig.hxx"
#endif

#ifndef _OFA_OPTHTML_HXX //autogen
#include <opthtml.hxx>
#endif
#ifndef _SV_WRKWIN_HXX //autogen
#include <vcl/wrkwin.hxx>
#endif
#ifndef _UTL_CONFIGMGR_HXX_
#include <unotools/configmgr.hxx>
#endif

#include <comphelper/processfactory.hxx>

#ifndef _SFX_SFXUNO_HXX
#include <sfx2/sfxuno.hxx>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XJOBEXECUTOR_HPP_
#include <com/sun/star/task/XJobExecutor.hpp>
#endif
#ifndef _UNO_LINGU_HXX
#include <svx/unolingu.hxx>
#endif

#ifndef _LINGUISTIC_LNGPROPS_HHX_
#include <linguistic/lngprops.hxx>
#endif
#ifndef _OSL_MODULE_H_
#include <osl/module.h>
#endif
#ifndef _OSL_PROCESS_H_
#include <osl/process.h>
#endif
#ifndef _RTL_BOOTSTRAP_HXX_
#include <rtl/bootstrap.hxx>
#endif

#include <sfx2/viewfrm.hxx>
#include <sfx2/docfac.hxx>

#define ITEMID_COLOR_TABLE      SID_COLOR_TABLE
#ifndef _SVX_DRAWITEM_HXX 
#include <svx/drawitem.hxx>
#endif

#include "resid.hxx"
#include "app.hxx"
#include "appimp.hxx"
#include "app.hrc"
#include "osplcfg.hxx"
#include "apearcfg.hxx"
#include "optgdlg.hxx"
#include "optmemory.hxx"
#include "treeopt.hxx"
#include "treeopt.hrc"
#include "splnote.hxx"
#include "optfltr.hxx"
#include "ofaids.hrc"
#include "ofaitem.hxx"
#include <tools/urlobj.hxx>
#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#include <svtools/pathoptions.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_MODULEOPTIONS_HXX
#include <svtools/moduleoptions.hxx>
#endif
#ifndef SVTOOLS_REGOPTIONS_HXX
#include <svtools/regoptions.hxx>
#endif

#define DEF_INCH	2540L
#define DEF_RELTWIP	1440L


using namespace rtl;
using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::linguistic2;
using namespace com::sun::star::task;

// ------------------------------------------------------------------------
static const ::rtl::OUString& getProductRegistrationServiceName( )
{
	static ::rtl::OUString s_sServiceName = ::rtl::OUString::createFromAscii( "com.sun.star.setup.ProductRegistration" );
	return s_sServiceName;
}

// ------------------------------------------------------------------------

void OfficeApplication::ExecuteApp_Impl( SfxRequest &rReq )
{
	sal_uInt16 nSlot = rReq.GetSlot();
	sal_Bool bIgnore = sal_False;

	switch ( nSlot )
	{
		case SID_OPTPAGE_PROXY:
		case SID_OPTPAGE_USERDATA:
		case SID_OPTIONS_TREEDIALOG:
		{
			ExecuteGeneralOptionsDialog(nSlot);
		}
		break;

		case SID_ONLINE_REGISTRATION:
		{
			try
			{
				// create the ProductRegistration component
				Reference< XMultiServiceFactory > xORB( ::comphelper::getProcessServiceFactory() );
				Reference< XJobExecutor > xProductRegistration;
				if ( xORB.is() )
					xProductRegistration = xProductRegistration.query( xORB->createInstance( getProductRegistrationServiceName() ) );
				DBG_ASSERT( xProductRegistration.is(), "OfficeApplication::ExecuteApp_Impl: could not create the service!" );

				// tell it that the user wants to register
				if ( xProductRegistration.is() )
				{
					xProductRegistration->trigger( ::rtl::OUString::createFromAscii( "RegistrationRequired" ) );
				}
			}
			catch( const ::com::sun::star::uno::Exception& )
			{
				DBG_ERROR( "OfficeApplication::ExecuteApp_Impl(SID_ONLINE_REGISTRATION): caught an exception!" );
			}
		}
		break;

		case SID_BASICCHOOSER:
		{
			bIgnore = sal_True;
			const SfxItemSet* pArgs = rReq.GetArgs();
			const SfxPoolItem* pItem;
            BOOL bChooseOnly = FALSE, bExecute = TRUE;
            if(pArgs && SFX_ITEM_SET == pArgs->GetItemState(SID_RECORDMACRO, sal_False, &pItem) )
            {
                BOOL bRecord = ((SfxBoolItem*)pItem)->GetValue();
                if ( bRecord )
                {
                    // !Hack
                    bChooseOnly = FALSE;
                    bExecute = FALSE;
                }
            }

            rReq.SetReturnValue( SfxStringItem( nSlot, OfficeApplication::ChooseMacro( bExecute, bChooseOnly ) ) );
			rReq.Done();
		}
		break;

		case SID_SEARCH_ITEM:
		break;
		case SID_ATTR_ADDRESS:
		{
			const SfxItemSet* pArgs = rReq.GetArgs();
			const SfxPoolItem* pItem;
			if(pArgs && SFX_ITEM_SET == pArgs->GetItemState(nSlot, sal_False, &pItem))
			{
					((SvxAddressItem*)pItem)->Store();
			}
		}
		break;
		case SID_APP_ENTERWAIT:
			Application::EnterWait();
			break;
		case SID_APP_LEAVEWAIT:
			Application::LeaveWait();
			break;
		case SID_APP_ISWAIT:
			{
				rReq.SetReturnValue( SfxBoolItem( SID_APP_ISWAIT,
												Application::IsWait() ));
			}
			break;

		case SID_OFFICE_CHECK_PLZ:
		{
			sal_Bool bRet = sal_False;
			SFX_REQUEST_ARG(rReq, pStringItem, SfxStringItem, nSlot, sal_False);

			if ( pStringItem )
			{
				String aPLZ = pStringItem->GetValue();
				bRet = TRUE /*!!!SfxIniManager::CheckPLZ( aPLZ )*/;
			}
			else
				SbxBase::SetError( SbxERR_WRONG_ARGS );
			rReq.SetReturnValue( SfxBoolItem( nSlot, bRet ) );
			break;
		}
	}

	if ( !bIgnore )
		rReq.Done();
}

// ------------------------------------------------------------------------

void OfficeApplication::GetStateApp_Impl( SfxItemSet &rSet )
{
	SfxWhichIter aIter( rSet );
	sal_uInt16 nWhich = aIter.FirstWhich();

	while ( nWhich )
	{
		switch ( nWhich )
		{
			case SID_ATTR_ADDRESS:
			{
				SvxAddressItem aAddress;
				aAddress.SetWhich(nWhich);
				rSet.Put(aAddress);
			}
			break;
			case SID_ONLINE_REGISTRATION:
			{
				::svt::RegOptions aOptions;
				if ( !aOptions.allowMenu() )
					rSet.DisableItem( SID_ONLINE_REGISTRATION );
			}
			break;
		}
		nWhich = aIter.NextWhich();
	}
}

// ------------------------------------------------------------------------
// Overloaded Application::Sys...
// Check whether systems-settings (from the os) should overwrite our defaults
// (CP & TH)

void
OfficeApplication::SystemSettingsChanging( AllSettings& rSettings, Window* pFrame )
{
	if ( !OfaTabAppearanceCfg::IsInitialized () )
		return ;

#   define DRAGFULL_OPTION_ALL \
	     ( DRAGFULL_OPTION_WINDOWMOVE | DRAGFULL_OPTION_WINDOWSIZE  \
		 | DRAGFULL_OPTION_OBJECTMOVE  | DRAGFULL_OPTION_OBJECTSIZE \
		 | DRAGFULL_OPTION_DOCKING     | DRAGFULL_OPTION_SPLIT      \
		 | DRAGFULL_OPTION_SCROLL )
#   define DRAGFULL_OPTION_NONE ((sal_uInt32)~DRAGFULL_OPTION_ALL)

	StyleSettings hStyleSettings   = rSettings.GetStyleSettings();
	MouseSettings hMouseSettings = rSettings.GetMouseSettings();

	sal_uInt32         nDragFullOptions = hStyleSettings.GetDragFullOptions();

	OfaTabAppearanceCfg* pAppearanceCfg = GetTabAppearanceConfig();
	sal_uInt16 nGet = pAppearanceCfg->GetDragMode();

	switch ( nGet )
	{
	case DragFullWindow:
		nDragFullOptions |= DRAGFULL_OPTION_ALL;
		break;
	case DragFrame:
		nDragFullOptions &= DRAGFULL_OPTION_NONE;
		break;
	case DragSystemDep:
	default:
		break;
	}

    sal_uInt32 nFollow = hMouseSettings.GetFollow();
    hMouseSettings.SetFollow(pAppearanceCfg->IsMenuMouseFollow() ?
            (nFollow|MOUSE_FOLLOW_MENU) : (nFollow&~MOUSE_FOLLOW_MENU));
    rSettings.SetMouseSettings(hMouseSettings);

	sal_uInt16 nTabStyle = hStyleSettings.GetTabControlStyle();
    nTabStyle &= ~STYLE_TABCONTROL_SINGLELINE;
    if(pAppearanceCfg->IsSingleLineTabCtrl())
        nTabStyle |=STYLE_TABCONTROL_SINGLELINE;

    nTabStyle &= ~STYLE_TABCONTROL_COLOR;
    if(pAppearanceCfg->IsColoredTabCtrl())
        nTabStyle |= STYLE_TABCONTROL_COLOR;

    hStyleSettings.SetTabControlStyle(nTabStyle);

	hStyleSettings.SetDragFullOptions( nDragFullOptions );
	rSettings.SetStyleSettings ( hStyleSettings );

	MiscSettings aMiscSettings( rSettings.GetMiscSettings() );
	aMiscSettings.SetTwoDigitYearStart( (USHORT)GetMiscConfig()->GetYear2000() );
	rSettings.SetMiscSettings( aMiscSettings );
}

// ------------------------------------------------------------------------
/*
BasicIDE* OfficeApplication::GetBasicIDE()
{
	if(!pDataImpl->pBasicIDE)
		pDataImpl->pBasicIDE = new BasicIDE;
	return pDataImpl->pBasicIDE;
}
*/
/* -----------------11.02.99 13:57-------------------
 *
 * --------------------------------------------------*/
SfxItemSet*	OfficeApplication::CreateItemSet( sal_uInt16 nId )
{
	Reference< XPropertySet >  xProp( SvxGetLinguPropertySet() );

	SfxItemSet*	pRet = 0;
	switch(nId)
	{
        case SID_GENERAL_OPTIONS:
		{
			pRet = new SfxItemSet(
				GetPool(),
				SID_BASIC_ENABLED, SID_BASIC_ENABLED,
//SID_OPTIONS_START - ..END
				SID_OPTIONS_START, SID_INET_PROXY_PORT,
                SID_INET_SMTPSERVER, SID_INET_SMTPSERVER,
				SID_INET_NOPROXY, SID_INET_SOCKS_PROXY_PORT,
				SID_INET_DNS_AUTO, SID_INET_DNS_SERVER,
                SID_INET_EXE_PLUGIN, SID_INET_EXE_PLUGIN,
				SID_ATTR_BUTTON_OUTSTYLE3D, SID_ATTR_BUTTON_BIGSIZE,
				SID_ATTR_AUTO_STYLE_UPDATE, SID_AUTOHELPAGENT_RESET,
				SID_ATTR_QUICKLAUNCHER, SID_APPEAR_COLORED_TABCTRL,
				SID_ATTR_ALLOWFOLDERWEBVIEW, SID_HELPAGENT_TIMEOUT,
				SID_PRINTER_NOTFOUND_WARN, SID_PRINTER_NOTFOUND_WARN,
				SID_PRINTER_CHANGESTODOC, SID_PRINTER_CHANGESTODOC,
				SID_SECURE_URL, SID_SECURE_URL,
#if defined( UNX ) || defined ( FS_PRIV_DEBUG )
				SID_OPT_FONT_ANTIALIASING_ENABLED, SID_OPT_FONT_ANTIALIASING_MINPIXELS,
#endif
                SID_OPT_MIDDLE_MOUSE, SID_OPT_MIDDLE_MOUSE,
                SID_MACRO_WARNING,  SID_MACRO_CONFIRMATION,
                SID_HELP_STYLESHEET, SID_HELP_STYLESHEET,
                0 );

			GetOptions(*pRet);
			pRet->Put(SvxAddressItem(SID_ATTR_ADDRESS));

			SfxMiscCfg*	pMisc = GetMiscConfig();
			const SfxPoolItem* pItem;
			SfxPoolItem* pClone;
            SfxViewFrame* pViewFrame = SfxViewFrame::Current();
            if ( pViewFrame )
            {
                SfxDispatcher* pDispatch = pViewFrame->GetDispatcher();

                sal_Bool bAppUndo = sal_False;
    //          UndoCount fuer den Writer extra
                if(SFX_ITEM_AVAILABLE <= pDispatch->QueryState( SID_ATTR_UNDO_COUNT, pItem) )
                {
                    pClone = pItem->Clone();
                    pRet->Put(*pClone);
                    delete pClone;
                    bAppUndo = sal_True;
                }

                // Sonstiges - Year2000
                if( SFX_ITEM_AVAILABLE <= pDispatch->QueryState( SID_ATTR_YEAR2000, pItem ) )
                    pRet->Put( SfxUInt16Item( SID_ATTR_YEAR2000, ((const SfxUInt16Item*)pItem)->GetValue() ) );
                else
                    pRet->Put( SfxUInt16Item( SID_ATTR_YEAR2000, (USHORT)pMisc->GetYear2000() ) );
            }
            else
                pRet->Put( SfxUInt16Item( SID_ATTR_YEAR2000, (USHORT)pMisc->GetYear2000() ) );


			// Sonstiges - Tabulator
            pRet->Put(SfxBoolItem(SID_PRINTER_NOTFOUND_WARN, pMisc->IsNotFoundWarning()));

			sal_uInt16 nFlag = pMisc->IsPaperSizeWarning() ? SFX_PRINTER_CHG_SIZE : 0;
            nFlag  |= pMisc->IsPaperOrientationWarning()  ? SFX_PRINTER_CHG_ORIENTATION : 0;
			pRet->Put( SfxFlagItem( SID_PRINTER_CHANGESTODOC, nFlag ));

			// Optionen Allgemein -- Darstellung
			OfaTabAppearanceCfg* pAppearanceCfg = GetTabAppearanceConfig();
			sal_uInt16 nSet;

			nSet = pAppearanceCfg->GetLookNFeel();
			pRet->Put(SfxUInt16Item (SID_OPT_SYSTEMLOOK,    nSet) );

			nSet = pAppearanceCfg->GetScaleFactor();
			pRet->Put(SfxUInt16Item (SID_OPT_SCREENSCALING, nSet) );

			nSet = pAppearanceCfg->GetDragMode();
			pRet->Put(SfxUInt16Item (SID_OPT_DRAGMODE,      nSet) );

            pRet->Put(SfxInt16Item (SID_OPT_SNAPTYPE, pAppearanceCfg->GetSnapMode()) );
            pRet->Put(SfxInt16Item (SID_OPT_MIDDLE_MOUSE, pAppearanceCfg->GetMiddleMouseButton()));
#if defined( UNX ) || defined ( FS_PRIV_DEBUG )
			pRet->Put( SfxBoolItem( SID_OPT_FONT_ANTIALIASING_ENABLED, pAppearanceCfg->IsFontAntiAliasing() ) );
			pRet->Put( SfxUInt16Item( SID_OPT_FONT_ANTIALIASING_MINPIXELS, pAppearanceCfg->GetFontAntialiasingMinPixelHeight() ) );
#endif

			AllSettings   hAppSettings = Application::GetSettings();
			MouseSettings hMouseSettings = hAppSettings.GetMouseSettings();
			sal_uInt32 nFollow = hMouseSettings.GetFollow();

			sal_uInt16 nTabStyle = hAppSettings.GetStyleSettings().GetTabControlStyle();

			pRet->Put(SfxBoolItem(SID_APPEAR_MENUE_MOUSE_FOLLOW,
									0 != (nFollow&MOUSE_FOLLOW_MENU)));
			pRet->Put(SfxBoolItem(SID_APPEAR_SINGLE_LINE_TABCTRL,
						0 !=(nTabStyle&STYLE_TABCONTROL_SINGLELINE)));
			pRet->Put(SfxBoolItem(SID_APPEAR_COLORED_TABCTRL,
						0 !=(nTabStyle&STYLE_TABCONTROL_COLOR)));
		}
		break;
        case SID_LANGUAGE_OPTIONS:
        {
            pRet = new SfxItemSet(
				GetPool(),
                SID_ATTR_LANGUAGE, SID_AUTOSPELL_MARKOFF,
                SID_ATTR_CHAR_CJK_LANGUAGE, SID_ATTR_CHAR_CTL_LANGUAGE,
                SID_OPT_LOCALE_CHANGED, SID_OPT_LOCALE_CHANGED,
				0 );

			// fuer die Linguistik

			Reference< XSpellChecker1 >  xSpell = SvxGetSpellChecker();
			pRet->Put(SfxSpellCheckItem( xSpell, SID_ATTR_SPELL ));
			SfxHyphenRegionItem aHyphen( SID_ATTR_HYPHENREGION );

			sal_Int16 	nMinLead  = 2,
						nMinTrail = 2;
			if (xProp.is())
			{
				xProp->getPropertyValue( String::CreateFromAscii(
						UPN_HYPH_MIN_LEADING) ) >>= nMinLead;
				xProp->getPropertyValue( String::CreateFromAscii(
						UPN_HYPH_MIN_TRAILING) ) >>= nMinTrail;
			}
			aHyphen.GetMinLead()  = (sal_uInt8)nMinLead;
			aHyphen.GetMinTrail() = (sal_uInt8)nMinTrail;

			const SfxPoolItem* pItem;
			SfxPoolItem* pClone;
            SfxViewFrame* pViewFrame = SfxViewFrame::Current();
            if ( pViewFrame )
            {
                SfxDispatcher* pDispatch = pViewFrame->GetDispatcher();
                if(SFX_ITEM_AVAILABLE <= pDispatch->QueryState(SID_ATTR_LANGUAGE, pItem))
                    pRet->Put(SfxUInt16Item(SID_ATTR_LANGUAGE, ((const SvxLanguageItem*)pItem)->GetLanguage()));
                if(SFX_ITEM_AVAILABLE <= pDispatch->QueryState(SID_ATTR_CHAR_CJK_LANGUAGE, pItem))
                    pRet->Put(SfxUInt16Item(SID_ATTR_CHAR_CJK_LANGUAGE, ((const SvxLanguageItem*)pItem)->GetLanguage()));
                if(SFX_ITEM_AVAILABLE <= pDispatch->QueryState(SID_ATTR_CHAR_CTL_LANGUAGE, pItem))
                    pRet->Put(SfxUInt16Item(SID_ATTR_CHAR_CTL_LANGUAGE, ((const SvxLanguageItem*)pItem)->GetLanguage()));

				pRet->Put(aHyphen);
                if(SFX_ITEM_AVAILABLE <= pDispatch->QueryState(SID_AUTOSPELL_CHECK, pItem))
                {
                    pClone = pItem->Clone();
                    pRet->Put(*pClone);
                    delete pClone;
                }
                else
                {
                        sal_Bool bVal = sal_False;
                        if (xProp.is())
                        {
                            xProp->getPropertyValue( String::CreateFromAscii( UPN_IS_SPELL_AUTO) ) >>= bVal;
                        }

                        pRet->Put(SfxBoolItem(SID_AUTOSPELL_CHECK, bVal));
                }

                if(SFX_ITEM_AVAILABLE <= pDispatch->QueryState(SID_AUTOSPELL_MARKOFF, pItem))
                {
                    pClone = pItem->Clone();
                    pRet->Put(*pClone);
                    delete pClone;
                }
                else
                {
                    sal_Bool bVal = sal_False;
                    if (xProp.is())
                    {
                        xProp->getPropertyValue( String::CreateFromAscii( UPN_IS_SPELL_HIDE) ) >>= bVal;
                    }
                    pRet->Put(SfxBoolItem(SID_AUTOSPELL_MARKOFF, bVal));
                }
            }
        }
        break;
        case SID_INET_DLG :
				pRet = new SfxItemSet(GetPool(),
								SID_BASIC_ENABLED, SID_BASIC_ENABLED,
				//SID_OPTIONS_START - ..END
								SID_OPTIONS_START, SID_INET_PROXY_PORT,
								SID_SAVEREL_INET, SID_SAVEREL_FSYS,
								SID_INET_SMTPSERVER, SID_INET_SMTPSERVER,
								SID_INET_NOPROXY, SID_INET_SOCKS_PROXY_PORT,
								SID_INET_DNS_AUTO, SID_INET_DNS_SERVER,
								SID_SECURE_URL, SID_SECURE_URL,
								0L );
				GetOptions(*pRet);
		break;
		case SID_FILTER_DLG:
            pRet = new SfxItemSet( GetPool(),
            SID_ATTR_DOCINFO, SID_ATTR_AUTOSAVEMINUTE,
            SID_SAVEREL_INET, SID_SAVEREL_FSYS,
			SID_ATTR_PRETTYPRINTING, SID_ATTR_PRETTYPRINTING,
            0 );
            GetOptions(*pRet);
            break;

		case SID_SB_STARBASEOPTIONS:
            pRet = new SfxItemSet( GetPool(),
            SID_SB_POOLING_ENABLED, SID_SB_DRIVER_TIMEOUTS,
            0 );
			::offapp::ConnectionPoolConfig::GetOptions(*pRet);
			break;
	}
	return pRet;
}
/* -----------------11.02.99 13:57-------------------
 *
 * --------------------------------------------------*/
void	OfficeApplication::ApplyItemSet( sal_uInt16 nId, const SfxItemSet& rSet )
{
	switch(nId)
	{
		case SID_GENERAL_OPTIONS:
		{
			SfxMiscCfg*	pMisc = GetMiscConfig();
			const SfxPoolItem* pItem;
			SetOptions( rSet );
			// Dispatcher neu holen, weil SetOptions() ggf. den Dispatcher zerst"ort hat
            SfxViewFrame *pViewFrame = SfxViewFrame::Current();
// -------------------------------------------------------------------------
//							Adresse setzen
// -------------------------------------------------------------------------
			if ( SFX_ITEM_SET == rSet.GetItemState( SID_ATTR_ADDRESS, sal_False, &pItem ) )
			{
				( (SvxAddressItem*)pItem )->Store();
			}

// -------------------------------------------------------------------------
//			Year2000 auswerten
// -------------------------------------------------------------------------

            USHORT nY2K = USHRT_MAX;
            if( SFX_ITEM_SET == rSet.GetItemState( SID_ATTR_YEAR2000, sal_False, &pItem ) )
                nY2K = ((const SfxUInt16Item*)pItem)->GetValue();
            if ( pViewFrame )
            {
                SfxDispatcher* pDispatch = pViewFrame->GetDispatcher();
                if( USHRT_MAX != nY2K)
                {
                    pDispatch->Execute( SID_ATTR_YEAR2000, SFX_CALLMODE_ASYNCHRON, pItem, 0L);
                }

// -------------------------------------------------------------------------
//			UndoCount fuer den Writer extra
// -------------------------------------------------------------------------

                sal_Bool bAppUndo = sal_False;
    //          UndoCount fuer den Writer extra
                if(SFX_ITEM_AVAILABLE <= pDispatch->QueryState( SID_ATTR_UNDO_COUNT, pItem)
                    && SFX_ITEM_SET == rSet.GetItemState( SID_ATTR_UNDO_COUNT, sal_False, &pItem))
                {
                    pDispatch->Execute(SID_ATTR_UNDO_COUNT, SFX_CALLMODE_ASYNCHRON|SFX_CALLMODE_RECORD, pItem, 0L);
                }
            }
            if( USHRT_MAX != nY2K)
            {
                pMisc->SetYear2000( nY2K );
                // an die Settings fuer VCL-Fields und NumberFormatter-Default propagieren
                AllSettings aAllSettings( Application::GetSettings() );
                MiscSettings aMiscSettings( aAllSettings.GetMiscSettings() );
                aMiscSettings.SetTwoDigitYearStart( (USHORT)pMisc->GetYear2000() );
                aAllSettings.SetMiscSettings( aMiscSettings );
                Application::SetSettings( aAllSettings );
            }

// -------------------------------------------------------------------------
//			Drucken auswerten
// -------------------------------------------------------------------------

			if(SFX_ITEM_SET == rSet.GetItemState(SID_PRINTER_NOTFOUND_WARN, sal_False, &pItem))
				pMisc->SetNotFoundWarning(((const SfxBoolItem*)pItem)->GetValue());

			if(SFX_ITEM_SET == rSet.GetItemState(SID_PRINTER_CHANGESTODOC, sal_False, &pItem))
			{
				const SfxFlagItem* pFlag = (const SfxFlagItem*)pItem;
				pMisc->SetPaperSizeWarning(0 != (pFlag->GetValue() &  SFX_PRINTER_CHG_SIZE ));
                pMisc->SetPaperOrientationWarning(0 !=  (pFlag->GetValue() & SFX_PRINTER_CHG_ORIENTATION ));
			}

// -------------------------------------------------------------------------
//			Appearance auswerten
// -------------------------------------------------------------------------

			AllSettings   hAppSettings = Application::GetSettings();
			StyleSettings hAppStyle    = hAppSettings.GetStyleSettings();
			sal_Bool  bAppChanged = sal_False;

			OfaTabAppearanceCfg* pAppearanceCfg = GetTabAppearanceConfig();
			// Look & Feel
			if(SFX_ITEM_SET == rSet.GetItemState(SID_OPT_SYSTEMLOOK, sal_False, &pItem))
			{
				pAppearanceCfg->SetLookNFeel( ((const SfxUInt16Item*)pItem)->GetValue() );
				bAppChanged = sal_True;
			}

			// Screen and ScreenFont Scaling
			if(SFX_ITEM_SET == rSet.GetItemState(SID_OPT_SCREENSCALING, sal_False, &pItem))
			{
				pAppearanceCfg->SetScaleFactor( ((const SfxUInt16Item*)pItem)->GetValue() );
				bAppChanged = sal_True;
			}

			// Mouse Snap
			if(SFX_ITEM_SET == rSet.GetItemState(SID_OPT_SNAPTYPE, sal_False, &pItem))
			{
                pAppearanceCfg->SetSnapMode( ((const SfxInt16Item*)pItem)->GetValue() );
				bAppChanged = sal_True;
			}
            // Middle mouse button
            if(SFX_ITEM_SET == rSet.GetItemState(SID_OPT_MIDDLE_MOUSE, sal_False, &pItem))
			{
                pAppearanceCfg->SetMiddleMouseButton( ((const SfxInt16Item*)pItem)->GetValue() );
				bAppChanged = sal_True;
			}

#if defined( UNX ) || defined ( FS_PRIV_DEBUG )
			// Font Antialising - enabled
			if (SFX_ITEM_SET == rSet.GetItemState(SID_OPT_FONT_ANTIALIASING_ENABLED, sal_False, &pItem) )
			{
                pAppearanceCfg->SetFontAntiAliasing( static_cast< const SfxBoolItem* >( pItem )->GetValue() );
				bAppChanged = sal_True;
			}

			// Font Antialising - min pixel height
			if (SFX_ITEM_SET == rSet.GetItemState(SID_OPT_FONT_ANTIALIASING_MINPIXELS, sal_False, &pItem) )
			{
                pAppearanceCfg->SetFontAntialiasingMinPixelHeight( static_cast< const SfxUInt16Item* >( pItem )->GetValue() );
				bAppChanged = sal_True;
			}
#endif

			// Show Full Window while dragging?
			if(SFX_ITEM_SET == rSet.GetItemState(SID_OPT_DRAGMODE, sal_False, &pItem))
			{
				pAppearanceCfg->SetDragMode( ((const SfxUInt16Item*)pItem)->GetValue() );
				bAppChanged = sal_True;
			}

			if(SFX_ITEM_SET == rSet.GetItemState(SID_APPEAR_MENUE_MOUSE_FOLLOW, sal_False, &pItem))
			{
				pAppearanceCfg->SetMenuMouseFollow( ((const SfxBoolItem*)pItem)->GetValue() );
				bAppChanged = sal_True;
			}

			if(SFX_ITEM_SET == rSet.GetItemState(SID_APPEAR_SINGLE_LINE_TABCTRL, sal_False, &pItem))
			{
				pAppearanceCfg->SetSingleLineTabCtrl( ((const SfxBoolItem*)pItem)->GetValue() );
				bAppChanged = sal_True;
			}

			if(SFX_ITEM_SET == rSet.GetItemState(SID_APPEAR_COLORED_TABCTRL, sal_False, &pItem))
			{
				pAppearanceCfg->SetColoredTabCtrl(((const SfxBoolItem*)pItem)->GetValue() );
				bAppChanged = sal_True;
			}

			// Appearance has changed ? publish it to Application
			if ( bAppChanged )
			{
				pAppearanceCfg->SetApplicationDefaults ( GetpApp() );
			}
		}

		break;
        case SID_LANGUAGE_OPTIONS :
        {
            sal_Bool bSaveSpellCheck = sal_False;
            const SfxPoolItem* pItem;
			if ( SFX_ITEM_SET == rSet.GetItemState( SID_SPELL_MODIFIED, sal_False, &pItem ) )
			{
				bSaveSpellCheck = ( (const SfxBoolItem*)pItem )->GetValue();
			}
            Reference< XMultiServiceFactory >  xMgr( ::comphelper::getProcessServiceFactory() );
			Reference< XPropertySet >  xProp(
					xMgr->createInstance( OUString::createFromAscii(
							"com.sun.star.linguistic2.LinguProperties") ),
					UNO_QUERY );
            if ( SFX_ITEM_SET == rSet.GetItemState(SID_ATTR_HYPHENREGION, sal_False, &pItem ) )
			{
				const SfxHyphenRegionItem* pHyphenItem = (const SfxHyphenRegionItem*)pItem;

				if (xProp.is())
				{
					xProp->setPropertyValue(
							String::CreateFromAscii(UPN_HYPH_MIN_LEADING),
							makeAny((sal_Int16) pHyphenItem->GetMinLead()) );
					xProp->setPropertyValue(
							String::CreateFromAscii(UPN_HYPH_MIN_TRAILING),
							makeAny((sal_Int16) pHyphenItem->GetMinTrail()) );
				}
				bSaveSpellCheck = sal_True;
			}

            SfxViewFrame *pViewFrame = SfxViewFrame::Current();
            if ( pViewFrame )
            {
                SfxDispatcher* pDispatch = pViewFrame->GetDispatcher();
				pItem = 0;
				if(SFX_ITEM_SET == rSet.GetItemState( SID_ATTR_LANGUAGE, sal_False, &pItem ))
				{
                	pDispatch->Execute(pItem->Which(),    SFX_CALLMODE_ASYNCHRON, pItem, 0L);
                    bSaveSpellCheck = sal_True;
				}
				if(SFX_ITEM_SET == rSet.GetItemState( SID_ATTR_CHAR_CTL_LANGUAGE, sal_False, &pItem ))
				{
                	pDispatch->Execute(pItem->Which(),    SFX_CALLMODE_ASYNCHRON, pItem, 0L);
                    bSaveSpellCheck = sal_True;
				}
				if(SFX_ITEM_SET == rSet.GetItemState( SID_ATTR_CHAR_CJK_LANGUAGE, sal_False, &pItem ))
				{
                	pDispatch->Execute(pItem->Which(),    SFX_CALLMODE_ASYNCHRON, pItem, 0L);
                    bSaveSpellCheck = sal_True;
				}

				if( SFX_ITEM_SET == rSet.GetItemState(SID_AUTOSPELL_CHECK, sal_False, &pItem ))
                {
                    sal_Bool bOnlineSpelling = ((const SfxBoolItem*)pItem)->GetValue();
                    pDispatch->Execute(SID_AUTOSPELL_CHECK,
                        SFX_CALLMODE_ASYNCHRON|SFX_CALLMODE_RECORD, pItem, 0L);

                    if (xProp.is())
                    {
                        xProp->setPropertyValue(
                                String::CreateFromAscii(UPN_IS_SPELL_AUTO),
                                makeAny(bOnlineSpelling) );
                    }
                }

                if( SFX_ITEM_SET == rSet.GetItemState(SID_AUTOSPELL_MARKOFF, sal_False, &pItem ))
                {
                    sal_Bool bHideSpell = ((const SfxBoolItem*)pItem)->GetValue();
                    pDispatch->Execute(SID_AUTOSPELL_MARKOFF, SFX_CALLMODE_ASYNCHRON|SFX_CALLMODE_RECORD, pItem, 0L);

                    if (xProp.is())
                    {
                        xProp->setPropertyValue(
                                String::CreateFromAscii(UPN_IS_SPELL_HIDE),
                                makeAny(bHideSpell) );
                    }
                }

                if( bSaveSpellCheck )
                {
                    //! the config item has changed since we modified the
                    //! property set it uses
                    pDispatch->Execute(SID_SPELLCHECKER_CHANGED, SFX_CALLMODE_ASYNCHRON);
                }
            }

            if( SFX_ITEM_SET == rSet.GetItemState(SID_OPT_LOCALE_CHANGED, sal_False, &pItem ))
            {
                SfxViewFrame* pViewFrame = SfxViewFrame::GetFirst();
                while ( pViewFrame )
                {
                    pViewFrame->GetDispatcher()->Execute(pItem->Which(),    SFX_CALLMODE_ASYNCHRON, pItem, 0L);
                    pViewFrame = SfxViewFrame::GetNext( *pViewFrame );
                }
            }
        }
        break;
		case SID_INET_DLG :
        case SID_FILTER_DLG:
			SetOptions( rSet );

		case SID_SB_STARBASEOPTIONS:
			::offapp::ConnectionPoolConfig::SetOptions( rSet );
			break;
		break;
	}

}
/* -----------------11.02.99 13:57-------------------
 *
 * --------------------------------------------------*/

typedef SfxTabPage* (*FNCreateTabPage)( Window *pParent, const SfxItemSet &rAttrSet );

SfxTabPage*	OfficeApplication::CreateTabPage( sal_uInt16 nId, Window* pParent, const SfxItemSet& rSet )
{
	FNCreateTabPage fnCreate = 0;
	switch(nId)
	{
		case RID_SFXPAGE_SAVE:						fnCreate = &SvxSaveTabPage::Create; break;
		case RID_SFXPAGE_PATH:						fnCreate = &SvxPathTabPage::Create; break;
		case RID_SFXPAGE_GENERAL:					fnCreate = &SvxGeneralTabPage::Create; break;
		case RID_SFXPAGE_PRINTOPTIONS:				fnCreate = &SfxCommonPrintOptionsTabPage::Create; break;
		case OFA_TP_HELPERPROG:						fnCreate = &OfaHelperProgramsTabPage::Create; break;
		case OFA_TP_LANGUAGES:						fnCreate = &OfaLanguagesTabPage::Create; break;
		case RID_SFXPAGE_LINGU:						fnCreate = &SvxLinguTabPage::Create; break;
		case RID_SVXPAGE_COLOR:						fnCreate = &SvxColorTabPage::Create; break;
		case OFA_TP_VIEW:							fnCreate = &OfaViewTabPage::Create; break;
		case OFA_TP_MISC:							fnCreate = &OfaMiscTabPage::Create; break;
		case OFA_TP_MEMORY:							fnCreate = &OfaMemoryOptionsPage::Create; break;
		case RID_SVXPAGE_ASIAN_LAYOUT:				fnCreate = &SvxAsianLayoutPage::Create; break;
		case RID_SVX_FONT_SUBSTITUTION:				fnCreate = &SvxFontSubstTabPage::Create; break;
		case RID_SVXPAGE_INET_PROXY:				fnCreate = &SvxProxyTabPage::Create; break;
		case RID_SVXPAGE_INET_SEARCH:				fnCreate = &SvxSearchTabPage::Create; break;
		case RID_SVXPAGE_INET_SCRIPTING:			fnCreate = &SvxScriptingTabPage::Create; break;
		case RID_SVXPAGE_COLORCONFIG:				fnCreate = &SvxColorOptionsTabPage::Create; break;
		case RID_OFAPAGE_HTMLOPT:					fnCreate = &OfaHtmlTabPage::Create; break;
		case SID_OPTFILTER_MSOFFICE:				fnCreate = &OfaMSFilterTabPage::Create; break;
		case RID_OFAPAGE_MSFILTEROPT2:				fnCreate = &OfaMSFilterTabPage2::Create; break;
		case RID_SVXPAGE_JSEARCH_OPTIONS:			fnCreate = &SvxJSearchOptionsPage::Create ; break;
		case SID_SB_CONNECTIONPOOLING:				fnCreate = &::offapp::ConnectionPoolOptionsPage::Create; break;
		case RID_SVXPAGE_ACCESSIBILITYCONFIG:		fnCreate = &SvxAccessibilityOptionsTabPage::Create; break;
		case RID_SVXPAGE_SSO:						fnCreate = ( FNCreateTabPage )GetSSOCreator(); break;
		case RID_SVXPAGE_OPTIONS_CTL:				fnCreate = &SvxCTLOptionsPage::Create ; break;
	}

	SfxTabPage*	pRet = fnCreate ? (*fnCreate)( pParent, rSet ) : NULL;
	return pRet;
}

/* -----------------11.02.99 13:57-------------------
 *
 * --------------------------------------------------*/
void	OfficeApplication::ExecuteGeneralOptionsDialog(sal_uInt16 nSlot)
{
#ifdef MACOSX
	// [ed] 2/7/05 Insert static guard so we only create one options dialog
	// at a time.  The Apple menu handlers do not dim appropriately.
	
	static BOOL optionsDialogActive = FALSE;
	
	if ( optionsDialogActive )
		return;
	
	optionsDialogActive = TRUE;
#endif
	//SID_OPTPAGE_USERDATA, SID_OPTPAGE_PROXY
	OfaTreeOptionsDialog* pDlg = new OfaTreeOptionsDialog( NULL );

	OfaPageResource aDlgResource;
	sal_uInt16 nGroup = 0;
	if(nSlot != SID_OPTPAGE_PROXY)
	{
		BOOL isSSOEnabled = EnableSSO();

		ResStringArray& rGeneralArray = aDlgResource.GetGeneralArray();
		nGroup = pDlg->AddGroup(rGeneralArray.GetString(0), this, 0, SID_GENERAL_OPTIONS );
		sal_uInt16 nEnd = nSlot == SID_OPTIONS_TREEDIALOG ? rGeneralArray.Count() : 2;

        sal_uInt16 i;
        for(i = 1; i < nEnd; i++)
		{
			sal_uInt16 nPageId = (sal_uInt16)rGeneralArray.GetValue(i);
			if ( nPageId != RID_SVXPAGE_SSO || isSSOEnabled )
			{
				pDlg->AddTabPage( nPageId, rGeneralArray.GetString(i), nGroup );
			}
		}

        //load/save
        ResStringArray& rFilterArray = aDlgResource.GetFilterArray();
        nGroup = pDlg->AddGroup( rFilterArray.GetString(0), this, 0,
                                    SID_FILTER_DLG );
        for(i = 1; i < rFilterArray.Count(); ++i )
            pDlg->AddTabPage( (sal_uInt16)rFilterArray.GetValue(i),
                                rFilterArray.GetString(i), nGroup );
    }

    SvtLanguageOptions aLanguageOptions;

    if ( nSlot == SID_OPTIONS_TREEDIALOG )
	{
        // language options
        ResStringArray& rLangArray = aDlgResource.GetLangArray();
        nGroup = pDlg->AddGroup( rLangArray.GetString(0), this, 0, SID_LANGUAGE_OPTIONS );
        for ( USHORT i = 1; i < rLangArray.Count(); ++i )
        {
            sal_uInt16 nValue = (sal_uInt16)rLangArray.GetValue(i);
            if ( ( RID_SVXPAGE_JSEARCH_OPTIONS != nValue || aLanguageOptions.IsJapaneseFindEnabled() ) &&
                 ( RID_SVXPAGE_ASIAN_LAYOUT != nValue    || aLanguageOptions.IsAsianTypographyEnabled() ) &&
				 ( RID_SVXPAGE_OPTIONS_CTL != nValue 	 || aLanguageOptions.IsCTLFontEnabled() ) )
                pDlg->AddTabPage( nValue, rLangArray.GetString(i), nGroup );
        }
    }
    if ( nSlot != SID_OPTPAGE_USERDATA )
	{
		// Internet
		// f"ur SID_OPTPAGE_PROXY wird der komplett INet-Dlg angezeigt
		ResStringArray& rInetArray = aDlgResource.GetInetArray();
		nGroup = pDlg->AddGroup(rInetArray.GetString(0), this, 0, SID_INET_DLG );
		//falls doch nur dir Proxy-Page gewuenscht wird, dann diese Zeile
//		sal_uInt16 nEnd = nSlot == SID_OPTPAGE_PROXY ? 2 : rInetArray.Count();
		sal_uInt16 nEnd = rInetArray.Count();

		for ( sal_uInt16 i = 1; i < nEnd; i++ )
		{
			sal_uInt16 nPageId = (sal_uInt16)rInetArray.GetValue(i);
			pDlg->AddTabPage( nPageId, rInetArray.GetString(i), nGroup );
		}
		if ( nSlot == SID_OPTPAGE_PROXY )
			pDlg->ActivatePage(	RID_SVXPAGE_INET_PROXY );
	}

	if ( nSlot == SID_OPTIONS_TREEDIALOG )
	{
        sal_Bool bHasAnyFilter = sal_False;
		SvtModuleOptions aModuleOpt;
		if ( aModuleOpt.IsWriter() )
		{
			// Textdokument
			bHasAnyFilter = sal_True;
			ResStringArray& rTextArray = aDlgResource.GetTextArray();
			SfxModule *pSwMod = (*(SfxModule**) GetAppData(SHL_WRITER));
			nGroup = pDlg->AddGroup(rTextArray.GetString(0), pSwMod, pSwMod, SID_SW_EDITOPTIONS );
			for(USHORT i = 1; i < rTextArray.Count(); i++)
            {
                sal_uInt16 nValue = (sal_uInt16)rTextArray.GetValue(i);
                if((RID_SW_TP_STD_FONT_CJK != nValue || aLanguageOptions.IsCJKFontEnabled())&&
                    (RID_SW_TP_STD_FONT_CTL != nValue || aLanguageOptions.IsCTLFontEnabled()))
                    pDlg->AddTabPage( nValue, rTextArray.GetString(i), nGroup);
            }
#ifndef PRODUCT
			pDlg->AddTabPage( RID_SW_TP_OPTTEST_PAGE, String::CreateFromAscii("Interner Test"), nGroup);
#endif
			// HTML-Dokument
			ResStringArray& rHTMLArray = aDlgResource.GetHTMLArray();
			nGroup = pDlg->AddGroup(rHTMLArray.GetString(0), pSwMod, pSwMod, SID_SW_ONLINEOPTIONS );
			for(i = 1; i < rHTMLArray.Count(); i++)
				pDlg->AddTabPage( (sal_uInt16)rHTMLArray.GetValue(i), rHTMLArray.GetString(i), nGroup);
#ifndef PRODUCT
			pDlg->AddTabPage( RID_SW_TP_OPTTEST_PAGE, String::CreateFromAscii("Interner Test"), nGroup);
#endif
		}

		if ( aModuleOpt.IsCalc() )
		{
			// StarCalc-Dialog
			bHasAnyFilter = sal_True;
			SfxModule*		pScMod = ( *( SfxModule** ) GetAppData( SHL_CALC ) );
			ResStringArray&	rCalcArray = aDlgResource.GetCalcArray();
			nGroup = pDlg->AddGroup( rCalcArray.GetString( 0 ), pScMod, pScMod, SID_SC_EDITOPTIONS );
			const sal_Bool	bCTL = aLanguageOptions.IsCTLFontEnabled();
			sal_uInt16		nId;
			const USHORT	nCount = rCalcArray.Count();
			for( USHORT i = 1 ; i < nCount ; ++i )
			{
				nId = ( sal_uInt16 ) rCalcArray.GetValue( i );
//				if( bCTL || nId != RID_OFA_TP_INTERNATIONAL )
//				#103755# if an international tabpage is need one day, this should be used again... ;-)
				if( nId != RID_OFA_TP_INTERNATIONAL )
					pDlg->AddTabPage( nId, rCalcArray.GetString( i ), nGroup );
			}
		}

		if ( aModuleOpt.IsImpress() )
		{
			//Praesentation
			bHasAnyFilter = sal_True;
			SfxModule*		pSdMod = ( *( SfxModule** ) GetAppData( SHL_DRAW ) );
			ResStringArray&	rImpressArray = aDlgResource.GetImpressArray();
			nGroup = pDlg->AddGroup( rImpressArray.GetString( 0 ), pSdMod, pSdMod, SID_SD_EDITOPTIONS );
			const sal_Bool	bCTL = aLanguageOptions.IsCTLFontEnabled();
			sal_uInt16		nId;
			const USHORT	nCount = rImpressArray.Count();
			for( USHORT i = 1 ; i < nCount ; ++i )
			{
				nId = ( sal_uInt16 ) rImpressArray.GetValue( i );
				if( bCTL || nId != RID_OFA_TP_INTERNATIONAL_IMPR )
					pDlg->AddTabPage( nId, rImpressArray.GetString( i ), nGroup );
			}
		}

		if ( aModuleOpt.IsDraw() )
		{
			//Zeichnung
			SfxModule*		pSdMod = ( *( SfxModule** ) GetAppData( SHL_DRAW ) );
			ResStringArray&	rDrawArray = aDlgResource.GetDrawArray();
			nGroup = pDlg->AddGroup( rDrawArray.GetString( 0 ), pSdMod, pSdMod, SID_SD_GRAPHIC_OPTIONS );
			const sal_Bool	bCTL = aLanguageOptions.IsCTLFontEnabled();
			sal_uInt16		nId;
			const USHORT	nCount = rDrawArray.Count();
			for( USHORT i = 1 ; i < nCount ; ++i )
			{
				nId = ( sal_uInt16 ) rDrawArray.GetValue( i );
				if( bCTL || nId != RID_OFA_TP_INTERNATIONAL_SD )
					pDlg->AddTabPage( nId, rDrawArray.GetString( i ), nGroup );
			}
		}

		if ( aModuleOpt.IsMath() )
		{
			// StarMath-Dialog
			SfxModule *pSmMod = (*(SfxModule**) GetAppData(SHL_SM));
			ResStringArray& rStarMathArray = aDlgResource.GetStarMathArray();
			nGroup = pDlg->AddGroup(rStarMathArray.GetString(0), pSmMod, pSmMod, SID_SM_EDITOPTIONS );
			for(USHORT i = 1; i < rStarMathArray.Count(); i++)
				pDlg->AddTabPage( (sal_uInt16)rStarMathArray.GetValue(i), rStarMathArray.GetString(i), nGroup);
		}

		if ( aModuleOpt.IsChart() )
		{
			//Diagramm
			SfxModule *pSchMod = (*(SfxModule**) GetAppData(SHL_SCH));
			ResStringArray& rChartArray = aDlgResource.GetChartArray();
			nGroup = pDlg->AddGroup(rChartArray.GetString(0), pSchMod, pSchMod, SID_SCH_EDITOPTIONS );
			for(USHORT i = 1; i < rChartArray.Count(); i++)
				pDlg->AddTabPage( (sal_uInt16)rChartArray.GetValue(i), rChartArray.GetString(i), nGroup);
		}

		if (sal_True)
		{	// Data access (always installed)
			ResStringArray& rDSArray = aDlgResource.GetDatasourcesArray();
			nGroup = pDlg->AddGroup(rDSArray.GetString(0), this, NULL, SID_SB_STARBASEOPTIONS );
			for(USHORT i = 1; i < rDSArray.Count(); i++)
				pDlg->AddTabPage( (sal_uInt16)rDSArray.GetValue(i), rDSArray.GetString(i), nGroup);
		}

        pDlg->ActivateLastSelection();
        if ( nSlot == SID_OPTPAGE_USERDATA )
			pDlg->ActivatePage(	RID_SFXPAGE_GENERAL );
    }

	short nRet;
	{
		// collect all DictionaryList Events while the dialog is executed
        Reference<com::sun::star::linguistic2::XDictionaryList> xDictionaryList(SvxGetDictionaryList());
		SvxDicListChgClamp aClamp( xDictionaryList );
		nRet = pDlg->Execute();
	}

	if(RET_OK == nRet)
	{
		pDlg->ApplyItemSets();

		if( pDlg->GetColorTable() )
		{
			pDlg->GetColorTable()->Save();

			// notify current viewframe it it uses the same color table
			if ( SfxViewFrame::Current() && SfxViewFrame::Current()->GetDispatcher() )
			{
				const OfaPtrItem* pPtr = (const OfaPtrItem*)SfxViewFrame::Current()->GetDispatcher()->Execute( SID_GET_COLORTABLE, SFX_CALLMODE_SYNCHRON );
				if( pPtr )
				{
					XColorTable* pColorTab = (XColorTable*)pPtr->GetValue();

					if( pColorTab && 
						pColorTab->GetPath() == pDlg->GetColorTable()->GetPath() &&
						pColorTab->GetName() == pDlg->GetColorTable()->GetName() )
						SfxObjectShell::Current()->PutItem( SvxColorTableItem( pDlg->GetColorTable() ) );
				}
			}
		}


        utl::ConfigManager::GetConfigManager()->StoreConfigItems();
    }
	delete pDlg;

#ifdef MACOSX
	// [ed] 2/7/05 Insert static guard so we only create one options dialog
	// at a time.  The Apple menu handlers do not dim appropriately.
	
	optionsDialogActive = FALSE;
#endif
}

BOOL OfficeApplication::EnableSSO( void )
{
	// SSO must be enabled if the configuration manager bootstrap settings
	// are configured as follows ...
	//  CFG_Offline=false
	//  CFG_ServerType=uno ( or unspecified )
	//  CFG_BackendService=
	//   com.sun.star.comp.configuration.backend.LdapSingleBackend

	OUString theIniFile;
	osl_getExecutableFile( &theIniFile.pData );
	theIniFile = theIniFile.copy( 0, theIniFile.lastIndexOf( '/' ) + 1 ) +
				 OUString::createFromAscii( SAL_CONFIGFILE( "configmgr" ) );
	::rtl::Bootstrap theBootstrap( theIniFile );

	OUString theOfflineValue;
	OUString theDefaultOfflineValue = OUString::createFromAscii( "false" );
	theBootstrap.getFrom( OUString::createFromAscii( "CFG_Offline" ),
						  theOfflineValue,
						  theDefaultOfflineValue );

	OUString theServerTypeValue;
	theBootstrap.getFrom( OUString::createFromAscii( "CFG_ServerType" ),
						  theServerTypeValue );

	OUString theBackendServiceTypeValue;
	theBootstrap.getFrom( OUString::createFromAscii( "CFG_BackendService" ),
						  theBackendServiceTypeValue );

	BOOL bSSOEnabled = 
		( theOfflineValue == theDefaultOfflineValue						&&
		  ( theServerTypeValue.getLength() == 0 ||
		  theServerTypeValue == OUString::createFromAscii( "uno" ) )	&&
		  theBackendServiceTypeValue ==
			OUString::createFromAscii(
				"com.sun.star.comp.configuration.backend.LdapSingleBackend" ) );
	if ( bSSOEnabled && GetSSOCreator() == 0 )
	{
		bSSOEnabled = FALSE;
	}
	return bSSOEnabled;
}

void * OfficeApplication::GetSSOCreator( void )
{
	static void * theSymbol = 0;
	if ( theSymbol == 0 )
	{
		OUString	theModuleName	=
			OUString::createFromAscii( SVLIBRARY( "ssoopt" ) );
		oslModule	theModule		=
			osl_loadModule( theModuleName.pData, SAL_LOADMODULE_DEFAULT );
		if ( theModule != 0 )
		{
			OUString theSymbolName =
				OUString::createFromAscii( "CreateSSOTabPage" );
			theSymbol = osl_getSymbol( theModule, theSymbolName.pData );
		}
	}
	return theSymbol;		
}
