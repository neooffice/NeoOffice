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
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
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
 *  =================================================
 *  Modified January 2005 by Edward Peterlin. SISSL Removed. NeoOffice is
 *  distributed under GPL only under modification term 3 of the LGPL.
 *
 *  Contributor(s): _______________________________________
 *
 ************************************************************************/

#ifdef PRECOMPILED
#include "ofapch.hxx"
#endif

#pragma hdrstop
#ifndef _TOOLS_SIMPLERESMGR_HXX_
#include <tools/simplerm.hxx>
#endif

#ifndef _COM_SUN_STAR_DRAWING_MEASURETEXTVERTPOS_HPP_
#include <com/sun/star/drawing/MeasureTextVertPos.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_TEXTANIMATIONDIRECTION_HPP_
#include <com/sun/star/drawing/TextAnimationDirection.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_CONNECTIONTYPE_HPP_
#include <com/sun/star/drawing/ConnectionType.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_LINESTYLE_HPP_
#include <com/sun/star/drawing/LineStyle.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_TEXTUREMODE_HPP_
#include <com/sun/star/drawing/TextureMode.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_ARRANGEMENT_HPP_
#include <com/sun/star/drawing/Arrangement.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_TEXTANIMATIONKIND_HPP_
#include <com/sun/star/drawing/TextAnimationKind.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_DASHSTYLE_HPP_
#include <com/sun/star/drawing/DashStyle.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_TEXTADJUST_HPP_
#include <com/sun/star/drawing/TextAdjust.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_PROJECTIONMODE_HPP_
#include <com/sun/star/drawing/ProjectionMode.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_LINEENDTYPE_HPP_
#include <com/sun/star/drawing/LineEndType.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_RECTANGLEPOINT_HPP_
#include <com/sun/star/drawing/RectanglePoint.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_MEASURETEXTHORZPOS_HPP_
#include <com/sun/star/drawing/MeasureTextHorzPos.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_CONNECTORTYPE_HPP_
#include <com/sun/star/drawing/ConnectorType.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_MIRRORAXIS_HPP_
#include <com/sun/star/drawing/MirrorAxis.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_TEXTUREKIND_HPP_
#include <com/sun/star/drawing/TextureKind.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_NORMALSKIND_HPP_
#include <com/sun/star/drawing/NormalsKind.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_ALIGNMENT_HPP_
#include <com/sun/star/drawing/Alignment.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_SHADEMODE_HPP_
#include <com/sun/star/drawing/ShadeMode.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_MEASUREKIND_HPP_
#include <com/sun/star/drawing/MeasureKind.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_LAYERTYPE_HPP_
#include <com/sun/star/drawing/LayerType.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_SNAPOBJECTTYPE_HPP_
#include <com/sun/star/drawing/SnapObjectType.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_FILLSTYLE_HPP_
#include <com/sun/star/drawing/FillStyle.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_CIRCLEKIND_HPP_
#include <com/sun/star/drawing/CircleKind.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_HORIZONTALDIMENSIONING_HPP_
#include <com/sun/star/drawing/HorizontalDimensioning.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_HATCHSTYLE_HPP_
#include <com/sun/star/drawing/HatchStyle.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_VERTICALDIMENSIONING_HPP_
#include <com/sun/star/drawing/VerticalDimensioning.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_TEXTFITTOSIZETYPE_HPP_
#include <com/sun/star/drawing/TextFitToSizeType.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_POLYGONKIND_HPP_
#include <com/sun/star/drawing/PolygonKind.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_TEXTUREPROJECTIONMODE_HPP_
#include <com/sun/star/drawing/TextureProjectionMode.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_POLYGONFLAGS_HPP_
#include <com/sun/star/drawing/PolygonFlags.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_DRAWVIEWMODE_HPP_
#include <com/sun/star/drawing/DrawViewMode.hpp>
#endif
#ifndef _COM_SUN_STAR_LOADER_XIMPLEMENTATIONLOADER_HPP_
#include <com/sun/star/loader/XImplementationLoader.hpp>
#endif
#ifndef _COM_SUN_STAR_LOADER_CANNOTACTIVATEFACTORYEXCEPTION_HPP_
#include <com/sun/star/loader/CannotActivateFactoryException.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XSET_HPP_
#include <com/sun/star/container/XSet.hpp>
#endif
#include <comphelper/processfactory.hxx>
#include <rtl/logfile.hxx>

#ifndef _SVX_UNOSHCOL_HXX

#include <vcl/config.hxx>

#ifndef _SVX_IMPGRF_HXX	//autogen
#include <svx/impgrf.hxx>
#endif
#ifndef _STRING_HXX //autogen
//#include <tools/string.hxx>
#endif
#ifndef _URLOBJ_HXX //autogen
#include <tools/urlobj.hxx>
#endif
#ifndef _SFXPTITEM_HXX
#include <svtools/ptitem.hxx>
#endif
#ifndef _SFX_FCONTNR_HXX //autogen
#include <sfx2/fcontnr.hxx>
#endif
#ifndef _SFXREQUEST_HXX
#include <sfx2/request.hxx>
#endif
#ifndef _FM_FMOBJFAC_HXX
#include <svx/fmobjfac.hxx>
#endif
#ifndef _SVX_SIIMPORT_HXX
#include <svx/siimport.hxx>
#endif
#ifndef _SVX_TBCONTRL_HXX //autogen
#include <svx/tbcontrl.hxx>
#endif
#ifndef _SVX_SRCHDLG_HXX //autogen
#include <svx/srchdlg.hxx>
#endif
#ifndef _SVX_THESDLG_HXX //autogen
#include <svx/thesdlg.hxx>
#endif
#ifndef _SVX_ADRITEM_HXX //autogen
#include <svx/adritem.hxx>
#endif
#ifndef _OUTLINER_HXX //autogen
#include <svx/outliner.hxx>
#endif
#ifndef _SV_WRKWIN_HXX //autogen
#include <vcl/wrkwin.hxx>
#endif
#ifndef _IMAPDLG_HXX_ //autogen
#include <svx/imapdlg.hxx>
#endif
#ifndef _CPPUHELPER_FACTORY_HXX_
#include <cppuhelper/factory.hxx>
#endif

#ifndef _SFXREQUEST_HXX
#include <sfx2/request.hxx>
#endif

#ifndef _SFXITEMSET_HXX
#include <svtools/itemset.hxx>
#endif
#ifndef _OFA_FONTSUBSTCONFIG_HXX
#include <fontsubstconfig.hxx>
#endif

// include ---------------------------------------------------------------

#ifndef _OFF_APP_HXX
#include "app.hxx"
#endif
#ifndef _OFF_APPIMP_HXX
#include "appimp.hxx"
#endif
#ifndef _OFF_RESID_HXX
#include "resid.hxx"
#endif
#ifndef _OFF_APP_HRC
#include "app.hrc"
#endif

#ifndef _SFX_DBCOLL_HXX
#include "dbcoll.hxx"
#endif
#ifndef _SVX_DLG_HYPERLINK_HXX
#include "hyprlink.hxx"
#endif
#ifndef _SFX_INETOPT_HXX
#include "optuno.hxx"
#endif

#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef _SFXDISPATCH_HXX
#include <sfx2/dispatch.hxx>
#endif
#ifndef	_SFXMSG_HXX
#include <sfx2/msg.hxx>
#endif
#ifndef _SFXOBJFACE_HXX
#include <sfx2/objface.hxx>
#endif
#define ITEMID_FIELD 0
#ifndef _SVX_FLDITEM_HXX
#include <svx/flditem.hxx>
#endif
#ifndef _SVDFIELD_HXX
#include <svx/svdfield.hxx>
#endif
#ifndef _OBJFAC3D_HXX
#include <svx/objfac3d.hxx>
#endif
#ifndef _SVX_SIZEITEM_HXX
#define ITEMID_SIZE 0
#include <svx/sizeitem.hxx>
#endif
#ifndef _SVX_ZOOMITEM_HXX
#include <svx/zoomitem.hxx>
#endif
#ifndef _SVX_LANGITEM_HXX
#define ITEMID_LANGUAGE 0
#include <svx/langitem.hxx>
#endif
#ifndef _SVX_UNOSHGRP_HXX
#include <svx/unoshcol.hxx>
#endif
#endif
#ifndef _SVX_TAB_HYPERLINK_HXX
#include <svx/hyperdlg.hxx>
#endif
#ifndef _SVX_HLNKITEM_HXX
#include <svx/hlnkitem.hxx>
#endif
#ifndef _NUMUNO_HXX
#include <svtools/numuno.hxx>
#endif
#ifndef _SB_SBSTAR_HXX
#include <basic/sbstar.hxx>
#endif

#include <svtools/moduleoptions.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/viewfrm.hxx>

#include "calwin.hxx"
#ifndef _OFA_APEARCFG_HXX
#include "apearcfg.hxx"
#endif

#ifndef _SBASLTID_HRC
#include "sbasltid.hrc"
#endif

#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif

#include <osl/module.hxx>

// [ed] 1/25/05 Includes for appleevent handlers.  Bug #396
#ifdef MACOSX
#include <premac.h>
#include <Carbon/Carbon.h>
#include <postmac.h>
#endif

using namespace ::com::sun::star::uno;

//=========================================================================

#define SFX_TYPEMAP
#define Address
#define _ExecAddress  ExecuteApp_Impl
#define _StateAddress GetStateApp_Impl
#define OfficeApplication
#include "ofaslots.hxx"

TYPEINIT1(OfficeApplication,SfxApplication);

#define IS_AVAILABLE(WhichId,ppItem) \
    (pArgs->GetItemState((WhichId), sal_True, ppItem ) == SFX_ITEM_SET)

#define DOSTRING( x )			   			#x
#define STRING( x )				   			DOSTRING( x )

//=========================================================================

typedef	rtl_uString* (SAL_CALL *basicide_choose_macro)(BOOL, BOOL, rtl_uString*);

typedef	long (SAL_CALL *basicide_handle_basic_error)(void*);

//=========================================================================

#ifdef MACOSX
// [ed] 1/26/05 AppleEvent handler for preferences events.
static OSErr DoAEPref( const AppleEvent *message, AppleEvent *reply, long refcon )
{
    OfficeApplication *theApp=(OfficeApplication *)refcon;
    if ( theApp )
        theApp->ExecuteGeneralOptionsDialog( SID_OPTIONS_TREEDIALOG );
	return noErr;
}
#endif

//=========================================================================

SFX_IMPL_INTERFACE(OfficeApplication,SfxApplication,OffResId(RID_DESKTOP))
{
	SFX_CHILDWINDOW_REGISTRATION( SID_HYPERLINK_INSERT );
//  SFX_CHILDWINDOW_REGISTRATION( SID_HYPERLINK_DIALOG );
//	SFX_CHILDWINDOW_REGISTRATION( SID_SHOW_BROWSER );
}

// ------------------------------------------------------------------------
// hack: this must be implemented somewhere else, hack for 554 update

Reference< XInterface > SAL_CALL SvxShapeCollection_CreateInstance( const Reference< ::com::sun::star::lang::XMultiServiceFactory >& rSMgr ) throw( Exception )
{
	return *( new SvxShapeCollection() );
}

void OfficeApplication::Init()
{
	RTL_LOGFILE_CONTEXT( aLog, "offmgr (cd100003) ::OfficeApplication::Init" );

    SfxApplication::Init();

	SvxIMapDlgChildWindow::RegisterChildWindow();

	// Handler fuer Grafikfilter setzen; im Handler wird ggf.
	// der Graphicfilter init., wenn das erste Mal auf einen
	// Filter zugegriffen wird (KA 04.08.98)
	Application::SetFilterHdl( LINK( this, OfficeApplication, ImplInitFilterHdl ) );

    // set basic error handler
	StarBASIC::SetGlobalErrorHdl( LINK( this, OfficeApplication, GlobalBasicErrorHdl ) );

	CreateDataImpl();
	UseFontSubst();

	OfficeApplication::RegisterInterface();

	SvxHyperlinkDlgWrapper::RegisterChildWindow();
	SvxSearchDialogWrapper::RegisterChildWindow();
	SvxHlinkDlgWrapper::RegisterChildWindow ();

	//OfficeApplication::RegisterInterface();
    SvxReloadControllerItem::RegisterControl( SID_RELOAD );

    // SvxURLField registrieren
	SvClassManager& rClassManager = SvxFieldItem::GetClassManager();
	rClassManager.SV_CLASS_REGISTER( SvxFieldData );
	rClassManager.SV_CLASS_REGISTER( SvxURLField );
	rClassManager.SV_CLASS_REGISTER( SvxDateField );
	rClassManager.SV_CLASS_REGISTER( SvxPageField );
	rClassManager.SV_CLASS_REGISTER( SvxTimeField );
	rClassManager.SV_CLASS_REGISTER( SvxExtTimeField );
	rClassManager.SV_CLASS_REGISTER( SvxExtFileField );
	rClassManager.SV_CLASS_REGISTER( SvxAuthorField );

    // SvDraw-Felder registrieren
    SdrRegisterFieldClasses();

    // 3D-Objekt-Factory eintragen
    E3dObjFactory();

    // ::com::sun::star::form::component::Form-Objekt-Factory eintragen
    FmFormObjFactory();

    // factory for dummy import of old si-controls in 3.1 documents
    SiImportFactory();

	// Servies etc. registrieren
	RTL_LOGFILE_CONTEXT_TRACE( aLog, "{ register services: com.sun.star.drawing.ShapeCollection/com.sun.star.util.NumberFormatter" );
	Reference< ::com::sun::star::lang::XMultiServiceFactory >  xSMgr = ::comphelper::getProcessServiceFactory();
	Reference< ::com::sun::star::container::XSet >  xSet( xSMgr, UNO_QUERY );
	Sequence< ::rtl::OUString >			aName( 1 );
	Reference< ::com::sun::star::lang::XSingleServiceFactory > 	xFact;

	aName.getArray()[0] = ::rtl::OUString::createFromAscii("com.sun.star.drawing.ShapeCollection");
	xFact = ::cppu::createSingleFactory( xSMgr, ::rtl::OUString::createFromAscii("ShapeCollection"), SvxShapeCollection_CreateInstance, aName );
	xSet->insert( makeAny(xFact) );

	aName.getArray()[0] = ::rtl::OUString::createFromAscii("com.sun.star.util.NumberFormatter");
	xFact = ::cppu::createSingleFactory( xSMgr, ::rtl::OUString::createFromAscii("NumberFormatter"), SvNumberFormatterServiceObj_NewInstance, aName );
	xSet->insert( makeAny( xFact ) );

	xSet->insert( makeAny( SfxSettingsContainer::impl_createFactory(xSMgr) ) );
	RTL_LOGFILE_CONTEXT_TRACE( aLog, "} register services: com.sun.star.drawing.ShapeCollection/com.sun.star.util.NumberFormatter" );

	// options - general - appearance
    OfaTabAppearanceCfg* pAppearanceCfg = GetTabAppearanceConfig();
	pAppearanceCfg->SetInitialized();
	pAppearanceCfg->SetApplicationDefaults( GetpApp() );

    pDataImpl->SetVCLSettings();

#ifdef MACOSX
// [ed] 1/26/05 Install preferenceshandler.
	AEInstallEventHandler( kCoreEventClass, 'mPRF', NewAEEventHandlerUPP( DoAEPref ), (long)this, FALSE );
#endif
}

// ------------------------------------------------------------------------
void OfficeApplication::Exit()
{
	if ( pDataImpl->pWordDeInitFct )
		pDataImpl->pWordDeInitFct();
}

//------------------------------------------------------------------------------


void OfficeApplication::UseFontSubst()
{
	OutputDevice::BeginFontSubstitution();
	// Alte Substitution entfernen
	sal_uInt16 nOldCount = OutputDevice::GetFontSubstituteCount();

	while (nOldCount)
		OutputDevice::RemoveFontSubstitute(--nOldCount);

	// Neue Substitution einlesen
	OfaFontSubstConfig aFontConfig;
    sal_Int32 nCount = aFontConfig.IsEnabled() ? aFontConfig.SubstitutionCount() : 0;

	for (sal_Int32  i = 0; i < nCount; i++)
	{
	    sal_uInt16 nFlags = 0;
		const SubstitutionStruct* pSubs = aFontConfig.GetSubstitution(i);
		if(pSubs->bReplaceAlways)
			nFlags |= FONT_SUBSTITUTE_ALWAYS;
		if(pSubs->bReplaceOnScreenOnly)
			nFlags |= FONT_SUBSTITUTE_SCREENONLY;
		OutputDevice::AddFontSubstitute( String(pSubs->sFont), String(pSubs->sReplaceBy), nFlags );
    }
	OutputDevice::EndFontSubstitution();
}

// ------------------------------------------------------------------------

ResMgr* OfficeApplication::GetOffResManager()
{
	if ( !pImpl->pResMgr )
		pImpl->pResMgr = CreateResManager( "ofa");
	return pImpl->pResMgr;
}

// ------------------------------------------------------------------------

SimpleResMgr* OfficeApplication::GetOffSimpleResManager()
{
	if ( !pImpl->m_pThreadSafeRessources )
	{
        LanguageType nType = Application::GetSettings().GetUILanguage();
		ByteString sMgrName("ofs");
		sMgrName += ByteString::CreateFromInt32( SOLARUPD );
		pImpl->m_pThreadSafeRessources = SimpleResMgr::Create( sMgrName.GetBuffer(), nType );
	}
	return pImpl->m_pThreadSafeRessources;
}

// ------------------------------------------------------------------------

void OfficeApplication::SetSbxCreatedLink( const Link &rLink )
// nur bis GetSbxObject virtual ist
{
	aSbaCreatedLink = rLink;
}

// ------------------------------------------------------------------------

void OfficeApplication::DrawExec_Impl( SfxRequest &rReq )
{
/*
Slots with the following id's are executed in this function
SID_AUTOPILOT
SID_OUTLINE_TO_IMPRESS
*/
	SvtModuleOptions aModuleOpt;

	// The special slots SID_AUTOPILOT/SID_OUTLINE_TO_IMPRESS are only used for impress.
	// Because impress uses the drawing library we have to ask for these special slots.
	if ( !aModuleOpt.IsImpress() &&
		 (( rReq.GetSlot() == SID_AUTOPILOT			 ) ||
		  ( rReq.GetSlot() == SID_OUTLINE_TO_IMPRESS ))	  )
	{
		vos::OGuard aGuard( Application::GetSolarMutex() );
		ErrorBox( 0, ResId( RID_ERRBOX_MODULENOTINSTALLED, GetOffResManager() )).Execute();
		return;
	}

	// We have to be sure that the drawing module is installed before trying to load draw library.
	if ( aModuleOpt.IsDraw() || aModuleOpt.IsImpress() )
	{
		SfxModule *pMod = (*(SfxModule**) GetAppData(SHL_DRAW))->Load();
		if(pMod)
		{
			pMod->ExecuteSlot( rReq );
			pMod->Free();
		}
	}
	else
	{
		vos::OGuard aGuard( Application::GetSolarMutex() );
		ErrorBox( 0, ResId( RID_ERRBOX_MODULENOTINSTALLED, GetOffResManager() )).Execute();
	}
}

// ------------------------------------------------------------------------

void OfficeApplication::ModuleState_Impl( SfxItemSet &rSet )
{
	// f"ur die Statusabfrage darf das Modul NICHT geladen werden

#if OSL_DEBUG_LEVEL > 1
// lass mal alle Features aus .ini sehen
	sal_Bool bf;
	for ( sal_uInt32 j=1; j; j <<= 1 )
	{
		if ( HasFeature( j ) )
			bf = sal_True;
		else
			bf = sal_False;
	}
#endif

	SvtModuleOptions aModuleOpt;

	if ( !aModuleOpt.IsCalc() )
		rSet.DisableItem( SID_SC_EDITOPTIONS );

	if ( !aModuleOpt.IsMath() )
		rSet.DisableItem( SID_SM_EDITOPTIONS );

	if ( !aModuleOpt.IsImpress() )
		rSet.DisableItem( SID_SD_EDITOPTIONS );

	if ( !aModuleOpt.IsDraw() )
		rSet.DisableItem( SID_SD_GRAPHIC_OPTIONS );

    if( !aModuleOpt.IsWriter())
    {
        rSet.DisableItem( SID_SW_AGENDA_WIZZARD );
        rSet.DisableItem( SID_SW_FAX_WIZZARD );
        rSet.DisableItem( SID_SW_LETTER_WIZZARD );
        rSet.DisableItem( SID_SW_MEMO_WIZZARD );
        rSet.DisableItem( SID_SW_DOCMAN_PATH );
    }
}


// ------------------------------------------------------------------------

void OfficeApplication::WriterExec_Impl( SfxRequest &rReq )
{
/*
Hier werden Executes fuer folgende Slots weitergeleitet
SID_SW_AGENDA_WIZZARD
SID_SW_FAX_WIZZARD
SID_SW_LETTER_WIZZARD
SID_SW_MEMO_WIZZARD
SID_SW_EDITOPTIONS
SID_SW_DOCMAN_PATH
*/
	SvtModuleOptions aModuleOpt;

	if ( aModuleOpt.IsWriter() )
	{
		SfxModule *pMod = (*(SfxModule**) GetAppData(SHL_WRITER))->Load();
		if(pMod)
		{
			pMod->ExecuteSlot( rReq );
			pMod->Free();
		}
	}
	else
	{
		vos::OGuard aGuard( Application::GetSolarMutex() );
		ErrorBox( 0, ResId( RID_ERRBOX_MODULENOTINSTALLED, GetOffResManager() )).Execute();
	}
}

// ------------------------------------------------------------------------

void OfficeApplication::CalcExec_Impl( SfxRequest &rReq )
{
/*
Hier werden Executes fuer folgende Slots weitergeleitet
*/

	SvtModuleOptions aModuleOpt;

	if ( aModuleOpt.IsCalc() )
	{
		SfxModule *pMod = (*(SfxModule**) GetAppData(SHL_CALC))->Load();
		if(pMod)
		{
			pMod->ExecuteSlot( rReq );
			pMod->Free();
		}
	}
	else
	{
		vos::OGuard aGuard( Application::GetSolarMutex() );
		ErrorBox( 0, ResId( RID_ERRBOX_MODULENOTINSTALLED, GetOffResManager() )).Execute();
	}
}


// ------------------------------------------------------------------------

IMPL_LINK( OfficeApplication, ImplInitFilterHdl, ConvertData*, pData )
{
	return GetGrfFilter()->GetFilterCallback().Call( pData );
}

// ------------------------------------------------------------------------

::rtl::OUString OfficeApplication::ChooseMacro( BOOL bExecute, BOOL bChooseOnly, const ::rtl::OUString& rMacroDesc )
{
    // get basctl dllname
    String sLibName = String::CreateFromAscii( STRING( DLL_NAME ) );
	sLibName.SearchAndReplace( String( RTL_CONSTASCII_USTRINGPARAM( "ofa" ) ), String( RTL_CONSTASCII_USTRINGPARAM( "basctl" ) ) );
	::rtl::OUString aLibName( sLibName );

    // load module
	oslModule handleMod = osl_loadModule( aLibName.pData, 0 );

    // get symbol
    ::rtl::OUString aSymbol( RTL_CONSTASCII_USTRINGPARAM( "basicide_choose_macro" ) );
    basicide_choose_macro pSymbol = (basicide_choose_macro) osl_getSymbol( handleMod, aSymbol.pData );

    // call basicide_choose_macro in basctl
    rtl_uString* pScriptURL = pSymbol( bExecute, bChooseOnly, rMacroDesc.pData );

    ::rtl::OUString aScriptURL( pScriptURL );
    rtl_uString_release( pScriptURL );

	return aScriptURL;
}

// ------------------------------------------------------------------------

IMPL_LINK( OfficeApplication, GlobalBasicErrorHdl, StarBASIC*, pBasic )
{
    // get basctl dllname
    String sLibName = String::CreateFromAscii( STRING( DLL_NAME ) );
	sLibName.SearchAndReplace( String( RTL_CONSTASCII_USTRINGPARAM( "ofa" ) ), String( RTL_CONSTASCII_USTRINGPARAM( "basctl" ) ) );
	::rtl::OUString aLibName( sLibName );

    // load module
	oslModule handleMod = osl_loadModule( aLibName.pData, 0 );

    // get symbol
    ::rtl::OUString aSymbol( RTL_CONSTASCII_USTRINGPARAM( "basicide_handle_basic_error" ) );
    basicide_handle_basic_error pSymbol = (basicide_handle_basic_error) osl_getSymbol( handleMod, aSymbol.pData );

    // call basicide_handle_basic_error in basctl
    long nRet = pSymbol( pBasic );

	return nRet;
}

// ------------------------------------------------------------------------
