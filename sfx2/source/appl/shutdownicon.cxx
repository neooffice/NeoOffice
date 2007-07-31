/*************************************************************************
 *
 *  $RCSfile$
 *
 *  $Revision$
 *
 *  last change: $Author$ $Date$
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU General Public License Version 2.1.
 *
 *
 *    GNU General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 *    Modified December 2005 by Patrick Luby. NeoOffice is distributed under
 *    GPL only under modification term 3 of the LGPL.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sfx2.hxx"

#include <shutdownicon.hxx>
#include <app.hrc>
#include <app.hxx>
#include <vos/mutex.hxx>
#include <svtools/imagemgr.hxx>
// #include <cmdlineargs.hxx>

#ifndef _COM_SUN_STAR_TASK_XINTERACTIONHANDLER_HPP_
#include <com/sun/star/task/XInteractionHandler.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCHRESULTLISTENER_HPP_
#include <com/sun/star/frame/XDispatchResultListener.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XNOTIFYINGDISPATCH_HPP_
#include <com/sun/star/frame/XNotifyingDispatch.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XFRAMESSUPPLIER_HPP_
#include <com/sun/star/frame/XFramesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XCOMPONENTLOADER_HPP_
#include <com/sun/star/frame/XComponentLoader.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XURLTRANSFORMER_HPP_
#include <com/sun/star/util/XURLTransformer.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XFRAMESSUPPLIER_HPP_
#include <com/sun/star/frame/XFramesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_UI_DIALOGS_XFILEPICKERCONTROLACCESS_HPP_
#include <com/sun/star/ui/dialogs/XFilePickerControlAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_UI_DIALOGS_XFILTERMANAGER_HPP_
#include <com/sun/star/ui/dialogs/XFilterManager.hpp>
#endif
#ifndef _COM_SUN_STAR_UI_DIALOGS_EXTENDEDFILEPICKERELEMENTIDS_HPP_
#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>
#endif
#ifndef _COM_SUN_STAR_UI_DIALOGS_COMMONFILEPICKERELEMENTIDS_HPP_
#include <com/sun/star/ui/dialogs/CommonFilePickerElementIds.hpp>
#endif
#ifndef _COM_SUN_STAR_UI_DIALOGS_CONTROLACTIONS_HPP_
#include <com/sun/star/ui/dialogs/ControlActions.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_MACROEXECMODE_HPP_
#include <com/sun/star/document/MacroExecMode.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_UPDATEDOCMODE_HPP_
#include <com/sun/star/document/UpdateDocMode.hpp>
#endif
#ifndef _FILEDLGHELPER_HXX
#include <filedlghelper.hxx>
#endif
#ifndef _SFX_FCONTNR_HXX
#include "fcontnr.hxx"
#endif
#ifndef _UNOTOOLS_PROCESSFACTORY_HXX
#include <comphelper/processfactory.hxx>
#endif
#ifndef _CPPUHELPER_COMPBASE1_HXX_
#include <cppuhelper/compbase1.hxx>
#endif
#include "dispatch.hxx"
#include <comphelper/extract.hxx>
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _OSL_SECURITY_HXX_
#include <osl/security.hxx>
#endif
#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif
#ifndef _UTL_BOOTSTRAP_HXX
#include <unotools/bootstrap.hxx>
#endif
#include <tools/link.hxx>
#ifdef UNX // need symlink
#include <unistd.h>
#endif

#include "sfxresid.hxx"

#ifdef USE_JAVA

#include <set>

#ifndef _SFXX11PRODUCTCHECK_HXX
#include "X11productcheck.hxx"
#endif

#include <svtools/dynamicmenuoptions.hxx> 
#include "shutdownicon_cocoa.h"

#ifndef _COMPHELPER_SEQUENCEASHASHMAP_HXX_
#include <comphelper/sequenceashashmap.hxx>
#endif 
#ifndef INCLUDED_SVTOOLS_MODULEOPTIONS_HXX
#include <svtools/moduleoptions.hxx>
#endif
 
#define WRITER_URL			"private:factory/swriter"
#define CALC_URL			"private:factory/scalc"
#define IMPRESS_URL			"private:factory/simpress"
#define IMPRESS_WIZARD_URL	"private:factory/simpress?slot=6686"
#define DRAW_URL			"private:factory/sdraw"
#define MATH_URL			"private:factory/smath"
#define BASE_URL			"private:factory/sdatabase?Interactive"

#define WRITER_FALLBACK_DESC			"Text Document"
#define CALC_FALLBACK_DESC				"Spreadsheet"
#define IMPRESS_WIZARD_FALLBACK_DESC	"Presentation"
#define DRAW_FALLBACK_DESC				"Drawing"
#define BASE_FALLBACK_DESC				"Database"

#endif	// USE_JAVA

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::vos;
using namespace ::rtl;
using namespace ::sfx2;

#ifdef USE_JAVA
class ShutdownIconEvent
{
	MenuCommand			mnCommand;
public:
						ShutdownIconEvent( MenuCommand nCommand ) : mnCommand( nCommand ) {}
						~ShutdownIconEvent() {}
						DECL_LINK( DispatchEvent, void* );
};

IMPL_LINK( ShutdownIconEvent, DispatchEvent, void*, pData )
{
	switch ( mnCommand )
	{
		case WRITER_COMMAND_ID:
			ShutdownIcon::OpenURL( OUString::createFromAscii( WRITER_URL ), OUString::createFromAscii( "_default" ) );
			break;
		case CALC_COMMAND_ID:
			ShutdownIcon::OpenURL( OUString::createFromAscii( CALC_URL ), OUString::createFromAscii( "_default" ) );
			break;
		case IMPRESS_COMMAND_ID:
			ShutdownIcon::OpenURL( OUString::createFromAscii( IMPRESS_WIZARD_URL ), OUString::createFromAscii( "_default" ) );
			break;
		case DRAW_COMMAND_ID:
			ShutdownIcon::OpenURL( OUString::createFromAscii( DRAW_URL ), OUString::createFromAscii( "_default" ) );
			break;
		case MATH_COMMAND_ID:
			ShutdownIcon::OpenURL( OUString::createFromAscii( MATH_URL ), OUString::createFromAscii( "_default" ) );
			break;
		case BASE_COMMAND_ID:
			ShutdownIcon::OpenURL( OUString::createFromAscii( BASE_URL ), OUString::createFromAscii( "_default" ) );
			break;
		case FROMTEMPLATE_COMMAND_ID:
			ShutdownIcon::FromTemplate();
			break;
		case FILEOPEN_COMMAND_ID:
			ShutdownIcon::FileOpen();
			break;
		default:
			break;
	}

	delete this;

	return 0;
}

void ProcessShutdownIconCommand( MenuCommand nCommand )
{
	if ( !Application::IsShutDown() )
	{
		switch ( nCommand )
		{
			case WRITER_COMMAND_ID:
			case CALC_COMMAND_ID:
			case IMPRESS_COMMAND_ID:
			case DRAW_COMMAND_ID:
			case MATH_COMMAND_ID:
			case BASE_COMMAND_ID:
			case FROMTEMPLATE_COMMAND_ID:
			case FILEOPEN_COMMAND_ID:
			{
				ShutdownIconEvent *pEvent = new ShutdownIconEvent( nCommand );
				Application::PostUserEvent( LINK( pEvent, ShutdownIconEvent, DispatchEvent ) );
				break;
			}
			default:
			{
				break;
			}
		}
	}
}

#endif	// USE_JAVA

class SfxNotificationListener_Impl : public cppu::WeakImplHelper1< XDispatchResultListener >
{
public:
    virtual void SAL_CALL dispatchFinished( const DispatchResultEvent& aEvent ) throw( RuntimeException );
    virtual void SAL_CALL disposing( const EventObject& aEvent ) throw( RuntimeException );
};

void SAL_CALL SfxNotificationListener_Impl::dispatchFinished( const DispatchResultEvent& ) throw( RuntimeException )
{
	ShutdownIcon::LeaveModalMode();
}

void SAL_CALL SfxNotificationListener_Impl::disposing( const EventObject& ) throw( RuntimeException )
{
}

SFX_IMPL_XSERVICEINFO( ShutdownIcon, "com.sun.star.office.Quickstart", "com.sun.star.comp.desktop.QuickstartWrapper" )	\
SFX_IMPL_ONEINSTANCEFACTORY( ShutdownIcon );

bool ShutdownIcon::bModalMode = false;
ShutdownIcon* ShutdownIcon::pShutdownIcon = NULL;

// To remove conditionals
extern "C" {
	static void disabled_initSystray() { }
	static void disabled_deInitSystray() { }
}
#define DOSTRING( x )			   			#x
#define STRING( x )				   			DOSTRING( x )

bool ShutdownIcon::LoadModule( osl::Module **pModule,
							   oslGenericFunction *pInit,
							   oslGenericFunction *pDeInit )
{
	if ( pModule )
	{
		OSL_ASSERT ( pInit && pDeInit );
		*pInit = *pDeInit = NULL;
		*pModule = NULL;
	}

#ifdef ENABLE_QUICKSTART_APPLET
#  ifdef WIN32
	if ( pModule )
	{
		*pInit = win32_init_sys_tray;
		*pDeInit = win32_shutdown_sys_tray;
	}
	return true;
#  else // UNX
	osl::Module *pPlugin;
	pPlugin = new osl::Module();

	oslGenericFunction pTmpInit = NULL;
	oslGenericFunction pTmpDeInit = NULL;
	if ( pPlugin->load( OUString (RTL_CONSTASCII_USTRINGPARAM( STRING( PLUGIN_NAME ) ) ) ) )
	{
		pTmpInit = pPlugin->getFunctionSymbol(
			OUString( RTL_CONSTASCII_USTRINGPARAM( "plugin_init_sys_tray" ) ) );
		pTmpDeInit = pPlugin->getFunctionSymbol(
			OUString( RTL_CONSTASCII_USTRINGPARAM( "plugin_shutdown_sys_tray" ) ) );
	}
	if ( !pTmpInit || !pTmpDeInit )
	{
		delete pPlugin;
		pPlugin = NULL;
	}
	if ( pModule )
	{
		*pModule = pPlugin;
		*pInit = pTmpInit;
		*pDeInit = pTmpDeInit;
	}
	else
	{
		bool bRet = pPlugin != NULL;
		delete pPlugin;
		return bRet;
	}
#  endif // UNX
#endif // ENABLE_QUICKSTART_APPLET
	if ( pModule )
	{
		if ( !*pInit )
			*pInit = disabled_initSystray;
		if ( !*pDeInit )
			*pDeInit = disabled_deInitSystray;
	}

	return true;
}

void ShutdownIcon::initSystray()
{
	if (m_bInitialized)
		return;
	m_bInitialized = true;

	(void) LoadModule( &m_pPlugin, &m_pInitSystray, &m_pDeInitSystray );
	m_bVeto = true;
	m_pInitSystray();
}

void ShutdownIcon::deInitSystray()
{
	if (!m_bInitialized)
		return;
    if (m_pDeInitSystray)
		m_pDeInitSystray();

	m_bVeto = false;
	m_pInitSystray = 0;
	m_pDeInitSystray = 0;
	if (m_pPlugin)
		delete m_pPlugin;
	m_pPlugin = 0;
    delete m_pFileDlg;
	m_pFileDlg = NULL;
	m_bInitialized = false;
}


ShutdownIcon::ShutdownIcon( Reference< XMultiServiceFactory > aSMgr ) :
	ShutdownIconServiceBase( m_aMutex ),
	m_bVeto ( false ),
	m_pResMgr( NULL ),
    m_pFileDlg( NULL ),
	m_xServiceManager( aSMgr ),
	m_pInitSystray( 0 ),
	m_pDeInitSystray( 0 ),
	m_pPlugin( 0 ),
	m_bInitialized( false )
{
}

ShutdownIcon::~ShutdownIcon()
{
	deInitSystray();
}

// ---------------------------------------------------------------------------

void ShutdownIcon::OpenURL( const ::rtl::OUString& aURL, const ::rtl::OUString& rTarget, const Sequence< PropertyValue >& aArgs )
{
    if ( getInstance() && getInstance()->m_xDesktop.is() )
    {
        Reference < XDispatchProvider > xDispatchProvider( getInstance()->m_xDesktop, UNO_QUERY );
        if ( xDispatchProvider.is() )
        {
            com::sun::star::util::URL aDispatchURL;
            aDispatchURL.Complete = aURL;

            Reference < com::sun::star::util::XURLTransformer > xURLTransformer(
                ::comphelper::getProcessServiceFactory()->createInstance( OUString::createFromAscii("com.sun.star.util.URLTransformer") ),
                com::sun::star::uno::UNO_QUERY );
            if ( xURLTransformer.is() )
            {
                try
                {
                    Reference< com::sun::star::frame::XDispatch > xDispatch;

                    xURLTransformer->parseStrict( aDispatchURL );
                    xDispatch = xDispatchProvider->queryDispatch( aDispatchURL, rTarget, 0 );
                    if ( xDispatch.is() )
                        xDispatch->dispatch( aDispatchURL, aArgs );
                }
                catch ( com::sun::star::uno::RuntimeException& )
                {
                    throw;
                }
                catch ( com::sun::star::uno::Exception& )
                {
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------

void ShutdownIcon::FileOpen()
{
    if ( getInstance() && getInstance()->m_xDesktop.is() )
    {
        ::vos::OGuard aGuard( Application::GetSolarMutex() );
		EnterModalMode();
        getInstance()->StartFileDialog();
    }
}

// ---------------------------------------------------------------------------

void ShutdownIcon::FromTemplate()
{
    if ( getInstance() && getInstance()->m_xDesktop.is() )
    {
        Reference < ::com::sun::star::frame::XFramesSupplier > xDesktop ( getInstance()->m_xDesktop, UNO_QUERY);
        Reference < ::com::sun::star::frame::XFrame > xFrame( xDesktop->getActiveFrame() );
        if ( !xFrame.is() )
            xFrame = Reference < ::com::sun::star::frame::XFrame >( xDesktop, UNO_QUERY );

        URL aTargetURL;
        aTargetURL.Complete = OUString( RTL_CONSTASCII_USTRINGPARAM( "slot:5500" ) );
        Reference < XURLTransformer > xTrans( ::comphelper::getProcessServiceFactory()->createInstance( rtl::OUString::createFromAscii("com.sun.star.util.URLTransformer" )), UNO_QUERY );
        xTrans->parseStrict( aTargetURL );

        Reference < ::com::sun::star::frame::XDispatchProvider > xProv( xFrame, UNO_QUERY );
        Reference < ::com::sun::star::frame::XDispatch > xDisp;
	    if ( xProv.is() )
            if ( aTargetURL.Protocol.compareToAscii("slot:") == COMPARE_EQUAL )
                xDisp = xProv->queryDispatch( aTargetURL, ::rtl::OUString(), 0 );
            else
                xDisp = xProv->queryDispatch( aTargetURL, ::rtl::OUString::createFromAscii("_blank"), 0 );
        if ( xDisp.is() )
	    {
		    Sequence<PropertyValue> aArgs(1);
		    PropertyValue* pArg = aArgs.getArray();
		    pArg[0].Name = rtl::OUString::createFromAscii("Referer");
            pArg[0].Value <<= ::rtl::OUString::createFromAscii("private:user");
            Reference< ::com::sun::star::frame::XNotifyingDispatch > xNotifyer( xDisp, UNO_QUERY );
            if ( xNotifyer.is() )
			{
				EnterModalMode();
                xNotifyer->dispatchWithNotification( aTargetURL, aArgs, new SfxNotificationListener_Impl() );
			}
            else
                xDisp->dispatch( aTargetURL, aArgs );
	    }
    }
}

// ---------------------------------------------------------------------------
#include <tools/rcid.h>
OUString ShutdownIcon::GetResString( int id )
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );

    if( ! m_pResMgr )
        m_pResMgr = SfxResId::GetResMgr();
	ResId aResId( id, m_pResMgr );
	aResId.SetRT( RSC_STRING );
	if( !m_pResMgr || !m_pResMgr->IsAvailable( aResId ) )
        return OUString();

    UniString aRes( ResId(id, m_pResMgr) );
    return OUString( aRes );
}

// ---------------------------------------------------------------------------

OUString ShutdownIcon::GetUrlDescription( const OUString& aUrl )
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );

    return OUString( SvFileInformationManager::GetDescription( INetURLObject( aUrl ) ) );
}

// ---------------------------------------------------------------------------

void ShutdownIcon::StartFileDialog()
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );

    if ( !m_pFileDlg )
        m_pFileDlg = new FileDialogHelper( WB_OPEN | SFXWB_MULTISELECTION, String() );
    m_pFileDlg->StartExecuteModal( STATIC_LINK( this, ShutdownIcon, DialogClosedHdl_Impl ) );
}

// ---------------------------------------------------------------------------

IMPL_STATIC_LINK( ShutdownIcon, DialogClosedHdl_Impl, FileDialogHelper*, EMPTYARG )
{
    DBG_ASSERT( pThis->m_pFileDlg, "ShutdownIcon, DialogClosedHdl_Impl(): no file dialog" );

    // use ctor for filling up filters automatically! #89169#
    if ( ERRCODE_NONE == pThis->m_pFileDlg->GetError() )
    {
        Reference< XFilePicker >    xPicker = pThis->m_pFileDlg->GetFilePicker();

        try
        {

            if ( xPicker.is() )
            {

                Reference < XFilePickerControlAccess > xPickerControls ( xPicker, UNO_QUERY );
                Reference < XFilterManager > xFilterManager ( xPicker, UNO_QUERY );

                Sequence< OUString >        sFiles = xPicker->getFiles();
                int                         nFiles = sFiles.getLength();

                int                         nArgs=3;
                Sequence< PropertyValue >   aArgs(3);

                Reference < com::sun::star::task::XInteractionHandler > xInteraction(
                    ::comphelper::getProcessServiceFactory()->createInstance( OUString::createFromAscii("com.sun.star.task.InteractionHandler") ),
                    com::sun::star::uno::UNO_QUERY );

                aArgs[0].Name = OUString::createFromAscii( "InteractionHandler" );
                aArgs[0].Value <<= xInteraction;

                sal_Int16 nMacroExecMode = ::com::sun::star::document::MacroExecMode::USE_CONFIG;
                aArgs[1].Name = OUString::createFromAscii( "MacroExecutionMode" );
                aArgs[1].Value <<= nMacroExecMode;

                sal_Int16 nUpdateDoc = ::com::sun::star::document::UpdateDocMode::ACCORDING_TO_CONFIG;
                aArgs[2].Name = OUString::createFromAscii( "UpdateDocMode" );
                aArgs[2].Value <<= nUpdateDoc;

                // pb: #102643# use the filedlghelper to get the current filter name,
                // because it removes the extensions before you get the filter name.
                OUString aFilterName( pThis->m_pFileDlg->GetCurrentFilter() );

                if ( xPickerControls.is() )
                {

                    // Set readonly flag

                    sal_Bool    bReadOnly = sal_False;


                    xPickerControls->getValue( ExtendedFilePickerElementIds::CHECKBOX_READONLY, 0 ) >>= bReadOnly;

                    // #95239#: Only set porperty if readonly is set to TRUE

                    if ( bReadOnly )
                    {
                        aArgs.realloc( ++nArgs );
                        aArgs[nArgs-1].Name  = OUString::createFromAscii( "ReadOnly" );
                        aArgs[nArgs-1].Value <<= bReadOnly;
                    }

                    // Get version string

                    sal_Int32   iVersion = -1;

                    xPickerControls->getValue( ExtendedFilePickerElementIds::LISTBOX_VERSION, ControlActions::GET_SELECTED_ITEM_INDEX ) >>= iVersion;

                    if ( iVersion >= 0 )
                    {
                        sal_Int16   uVersion = (sal_Int16)iVersion;

                        aArgs.realloc( ++nArgs );
                        aArgs[nArgs-1].Name  = OUString::createFromAscii( "Version" );
                        aArgs[nArgs-1].Value <<= uVersion;
                    }

                    // Retrieve the current filter

                    if ( !aFilterName.getLength() )
                        xPickerControls->getValue( CommonFilePickerElementIds::LISTBOX_FILTER, ControlActions::GET_SELECTED_ITEM ) >>= aFilterName;

                }


                // Convert UI filter name to internal filter name

                if ( aFilterName.getLength() )
                {
                    const SfxFilter* pFilter = SFX_APP()->GetFilterMatcher().GetFilter4UIName( aFilterName, 0, SFX_FILTER_NOTINFILEDLG );

                    if ( pFilter )
                    {
                        aFilterName = pFilter->GetFilterName();

                        if ( aFilterName.getLength() )
                        {
                            aArgs.realloc( ++nArgs );
                            aArgs[nArgs-1].Name  = OUString::createFromAscii( "FilterName" );
                            aArgs[nArgs-1].Value <<= aFilterName;
                        }
                    }
                }

                if ( 1 == nFiles )
                    OpenURL( sFiles[0], OUString( RTL_CONSTASCII_USTRINGPARAM( "_default" ) ), aArgs );
                else
                {
                    OUString    aBaseDirURL = sFiles[0];
                    if ( aBaseDirURL.getLength() > 0 && aBaseDirURL[aBaseDirURL.getLength()-1] != '/' )
                        aBaseDirURL += OUString::createFromAscii("/");

                    int iFiles;
                    for ( iFiles = 1; iFiles < nFiles; iFiles++ )
                    {
                        OUString    aURL = aBaseDirURL;
                        aURL += sFiles[iFiles];
                        OpenURL( aURL, OUString( RTL_CONSTASCII_USTRINGPARAM( "_default" ) ), aArgs );
                    }
                }
            }
        }
        catch ( ... )
        {
        }
    }

#ifdef WNT
        LeaveModalMode();
#endif
    return 0;
}

// ---------------------------------------------------------------------------

void ShutdownIcon::addTerminateListener()
{
	if ( getInstance() && getInstance()->m_xDesktop.is() )
		getInstance()->m_xDesktop->addTerminateListener( getInstance() );
}

// ---------------------------------------------------------------------------

void ShutdownIcon::terminateDesktop()
{
    if ( getInstance() && getInstance()->m_xDesktop.is() )
    {
        // always remove ourselves as listener
        getInstance()->m_xDesktop->removeTerminateListener( getInstance() );


        // terminate desktop only if no tasks exist
        Reference < XFramesSupplier > xSupplier( getInstance()->m_xDesktop, UNO_QUERY );
        if( xSupplier.is() )
        {
            Reference < XIndexAccess > xTasks ( xSupplier->getFrames(), UNO_QUERY );
            if( xTasks.is() )
            {
                if( xTasks->getCount()<1 )
                    getInstance()->m_xDesktop->terminate();
            }
        }

        // remove the instance pointer
        ShutdownIcon::pShutdownIcon = 0;
    }
}

// ---------------------------------------------------------------------------

ShutdownIcon* ShutdownIcon::getInstance()
{
	OSL_ASSERT( pShutdownIcon );
	return pShutdownIcon;
}

// ---------------------------------------------------------------------------

ShutdownIcon* ShutdownIcon::createInstance()
{
	if (pShutdownIcon)
        return pShutdownIcon;

	ShutdownIcon *pIcon = NULL;
	try {
		Reference< XMultiServiceFactory > xSMgr( comphelper::getProcessServiceFactory() );
		pIcon = new ShutdownIcon( xSMgr );
		pIcon->init ();
		pShutdownIcon = pIcon;
	} catch (...) {
		delete pIcon;
	}

	return pShutdownIcon;
}

void ShutdownIcon::init() throw( ::com::sun::star::uno::Exception )
{
	// access resource system and sfx only protected by solarmutex
	vos::OGuard aSolarGuard( Application::GetSolarMutex() );
	ResMgr *pResMgr = SfxResId::GetResMgr();

	::osl::ResettableMutexGuard	aGuard(	m_aMutex );
	m_pResMgr = pResMgr;
	aGuard.clear();
	Reference < XDesktop > xDesktop( m_xServiceManager->createInstance(
											 DEFINE_CONST_UNICODE( "com.sun.star.frame.Desktop" )),
									 UNO_QUERY );
	aGuard.reset();
	m_xDesktop = xDesktop;
}

// ---------------------------------------------------------------------------

void SAL_CALL ShutdownIcon::disposing()
{
	m_xServiceManager = Reference< XMultiServiceFactory >();
	m_xDesktop = Reference< XDesktop >();
}

// ---------------------------------------------------------------------------

// XEventListener
void SAL_CALL ShutdownIcon::disposing( const ::com::sun::star::lang::EventObject& )
	throw(::com::sun::star::uno::RuntimeException)
{
}

// ---------------------------------------------------------------------------

// XTerminateListener
void SAL_CALL ShutdownIcon::queryTermination( const ::com::sun::star::lang::EventObject& )
throw(::com::sun::star::frame::TerminationVetoException, ::com::sun::star::uno::RuntimeException)
{
	::osl::ClearableMutexGuard	aGuard(	m_aMutex );

	if ( m_bVeto )
		throw ::com::sun::star::frame::TerminationVetoException();
}


// ---------------------------------------------------------------------------

void SAL_CALL ShutdownIcon::notifyTermination( const ::com::sun::star::lang::EventObject& )
throw(::com::sun::star::uno::RuntimeException)
{
}


// ---------------------------------------------------------------------------

void SAL_CALL ShutdownIcon::initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any>& aArguments )
	throw( ::com::sun::star::uno::Exception )
{
	::osl::ResettableMutexGuard	aGuard(	m_aMutex );

    // third argument only sets veto, everything else will be ignored!
    if (aArguments.getLength() > 2)
    {
        sal_Bool bVeto = sal_True;
        bVeto = ::cppu::any2bool(aArguments[2]);
        m_bVeto = bVeto;
        return;
    }

	if ( aArguments.getLength() > 0 )
	{
		if ( !ShutdownIcon::pShutdownIcon )
		{
			try
			{
				sal_Bool bQuickstart = sal_False;
				bQuickstart = ::cppu::any2bool( aArguments[0] );
				if( !bQuickstart && !GetAutostart() )
					return;
                aGuard.clear();
				init ();
                aGuard.reset();
				if ( !m_xDesktop.is() )
					return;

				/* Create a sub-classed instance - foo */
				ShutdownIcon::pShutdownIcon = this;
#ifdef WNT
				initSystray();
#elif defined USE_JAVA
				if ( !::sfx2::IsX11Product() )
				{
				    // collect the URLs of the entries in the File/New menu
					::std::set< ::rtl::OUString > aFileNewAppsAvailable;
					SvtDynamicMenuOptions aOpt;
					Sequence < Sequence < PropertyValue > > aNewMenu = aOpt.GetMenu( E_NEWMENU );
					const ::rtl::OUString sURLKey( RTL_CONSTASCII_USTRINGPARAM( "URL" ) );

					const Sequence< PropertyValue >* pNewMenu = aNewMenu.getConstArray();
					const Sequence< PropertyValue >* pNewMenuEnd = aNewMenu.getConstArray() + aNewMenu.getLength();
					for ( ; pNewMenu != pNewMenuEnd; ++pNewMenu )
					{
						::comphelper::SequenceAsHashMap aEntryItems( *pNewMenu );
						::rtl::OUString sURL( aEntryItems.getUnpackedValueOrDefault( sURLKey, ::rtl::OUString() ) );
						if ( sURL.getLength() )
							aFileNewAppsAvailable.insert( sURL );
					}

					// describe the menu entries for launching the applications
					struct MenuEntryDescriptor
					{
						SvtModuleOptions::EModule	eModuleIdentifier;
						MenuCommand					nMenuItemID;
						const char*					pAsciiURLDescription;
						const char*					pFallbackDescription;
					} aMenuItems[] =
					{
						{ SvtModuleOptions::E_SWRITER, WRITER_COMMAND_ID, WRITER_URL, WRITER_FALLBACK_DESC },
						{ SvtModuleOptions::E_SCALC, CALC_COMMAND_ID, CALC_URL, CALC_FALLBACK_DESC },
						{ SvtModuleOptions::E_SIMPRESS, IMPRESS_COMMAND_ID, IMPRESS_WIZARD_URL, IMPRESS_WIZARD_FALLBACK_DESC },
						{ SvtModuleOptions::E_SDRAW, DRAW_COMMAND_ID, DRAW_URL, DRAW_FALLBACK_DESC },
						{ SvtModuleOptions::E_SDATABASE, BASE_COMMAND_ID, BASE_URL, BASE_FALLBACK_DESC }
					};

					// Disable shutdown
					SetVeto( true );
					addTerminateListener();

					// insert the menu entries for launching the applications
					int nItems = 0;
					MenuCommand aIDs[ 8 ];
					CFStringRef aStrings[ 8 ];
					XubString aDesc;
					SvtModuleOptions aModuleOptions;
					for ( size_t i = 0; i < sizeof( aMenuItems ) / sizeof( MenuEntryDescriptor ); ++i )
					{
						// the complete application is not even installed
						if ( !aModuleOptions.IsModuleInstalled( aMenuItems[i].eModuleIdentifier ) )
							continue;

						::rtl::OUString sURL( ::rtl::OUString::createFromAscii( aMenuItems[i].pAsciiURLDescription ) );

						// the application is installed, but the entry has been
						// configured to *not* appear in the File/New menu =>
						//  also let not appear it in the quickstarter
						if ( aFileNewAppsAvailable.find( sURL ) == aFileNewAppsAvailable.end() )
							continue;

						aIDs[ nItems ] = aMenuItems[i].nMenuItemID;
						aDesc = XubString( GetUrlDescription( sURL ) );
						aDesc.EraseAllChars( '~' );
						// Fix bug 2206 by putting in some default text if the
						// description is an empty string
						if ( !aDesc.Len() )
						{
							aDesc = XubString( ::rtl::OUString::createFromAscii( aMenuItems[i].pFallbackDescription ) );
							aDesc.EraseAllChars( '~' );
						}
						aStrings[ nItems++ ] = CFStringCreateWithCharacters( NULL, aDesc.GetBuffer(), aDesc.Len() );
					}

					// insert the remaining menu entries
					aIDs[ nItems ] = FROMTEMPLATE_COMMAND_ID;
					aDesc = XubString( GetResString( STR_QUICKSTART_FROMTEMPLATE ) );
					aDesc.EraseAllChars( '~' );
					aStrings[ nItems++ ] = CFStringCreateWithCharacters( NULL, aDesc.GetBuffer(), aDesc.Len() );
					aIDs[ nItems ] = FILEOPEN_COMMAND_ID;
					aDesc = XubString( GetResString( STR_QUICKSTART_FILEOPEN ) );
					aDesc.EraseAllChars( '~' );
					aStrings[ nItems++ ] = CFStringCreateWithCharacters( NULL, aDesc.GetBuffer(), aDesc.Len() );

					ULONG nCount = Application::ReleaseSolarMutex();
					AddQuickstartMenuItems( nItems, aIDs, aStrings );
					Application::AcquireSolarMutex( nCount );

					for ( int i = 0; i < nItems; i++ )
						CFRelease( aStrings[ i ] );
				}
#endif
			}
			catch(const ::com::sun::star::lang::IllegalArgumentException&)
			{
			}
		}
	}
    if ( aArguments.getLength() > 1 )
    {
			sal_Bool bAutostart = sal_False;
			bAutostart = ::cppu::any2bool( aArguments[1] );
            if (bAutostart && !GetAutostart())
                SetAutostart( sal_True );
            if (!bAutostart && GetAutostart())
                SetAutostart( sal_False );
    }

}

// -------------------------------

void ShutdownIcon::EnterModalMode()
{
	bModalMode = TRUE;
}

// -------------------------------

void ShutdownIcon::LeaveModalMode()
{
	bModalMode = FALSE;
}

#ifdef WNT
// defined in shutdowniconw32.cxx
#else
bool ShutdownIcon::IsQuickstarterInstalled()
{
#ifndef ENABLE_QUICKSTART_APPLET
	return false;
#else // !ENABLE_QUICKSTART_APPLET
#ifdef UNX
	return LoadModule( NULL, NULL, NULL);
#endif // UNX
#endif // !ENABLE_QUICKSTART_APPLET
}
#endif // !WNT

// ---------------------------------------------------------------------------

#if defined (ENABLE_QUICKSTART_APPLET) && defined (UNX)
static OUString getDotAutostart( bool bCreate = false )
{
	OUString aShortcut;
	const char *pConfigHome;
	if( (pConfigHome = getenv("XDG_CONFIG_HOME") ) )
		aShortcut = OStringToOUString( OString( pConfigHome ), RTL_TEXTENCODING_UTF8 );
	else
	{
		OUString aHomeURL;
		osl::Security().getHomeDir( aHomeURL );
		::osl::File::getSystemPathFromFileURL( aHomeURL, aShortcut );
		aShortcut += OUString( RTL_CONSTASCII_USTRINGPARAM( "/.config" ) );
	}
	aShortcut += OUString( RTL_CONSTASCII_USTRINGPARAM( "/autostart" ) );
	if (bCreate)
	{
		OUString aShortcutUrl;
		osl::File::getFileURLFromSystemPath( aShortcut, aShortcutUrl );
		osl::Directory::createPath( aShortcutUrl );
	}
	return aShortcut;
}
#endif

rtl::OUString ShutdownIcon::getShortcutName()
{
#ifndef ENABLE_QUICKSTART_APPLET
	return OUString();
#else

    OUString aShortcutName( RTL_CONSTASCII_USTRINGPARAM( "StarOffice 6.0" ) );
	ResMgr* pMgr = SfxResId::GetResMgr();
    if( pMgr )
    {
        ::vos::OGuard aGuard( Application::GetSolarMutex() );
        UniString aRes( SfxResId( STR_QUICKSTART_LNKNAME ) );
        aShortcutName = OUString( aRes );
    }
#ifdef WNT
    aShortcutName += OUString( RTL_CONSTASCII_USTRINGPARAM( ".lnk" ) );

	OUString aShortcut(GetAutostartFolderNameW32());
	aShortcut += OUString( RTL_CONSTASCII_USTRINGPARAM( "\\" ) );
	aShortcut += aShortcutName;
#else // UNX
	OUString aShortcut = getDotAutostart();
	aShortcut += OUString( RTL_CONSTASCII_USTRINGPARAM( "/qstart.desktop" ) );
#endif // UNX
	return aShortcut;
#endif // ENABLE_QUICKSTART_APPLET
}

bool ShutdownIcon::GetAutostart( )
{
	bool bRet = false;
#ifdef ENABLE_QUICKSTART_APPLET
	OUString aShortcut( getShortcutName() );
	OUString aShortcutUrl;
	osl::File::getFileURLFromSystemPath( aShortcut, aShortcutUrl );
	osl::File f( aShortcutUrl );
	osl::File::RC error = f.open( OpenFlag_Read );
	if( error == osl::File::E_None )
	{
		f.close();
		bRet = true;
	}
#endif // ENABLE_QUICKSTART_APPLET
    return bRet;
}

void ShutdownIcon::SetAutostart( bool bActivate )
{
#ifdef ENABLE_QUICKSTART_APPLET
	OUString aShortcut( getShortcutName() );

    if( bActivate && IsQuickstarterInstalled() )
    {
#ifdef WNT
		EnableAutostartW32( aShortcut );
#else // UNX
		getDotAutostart( true );

		OUString aPath;
		::utl::Bootstrap::locateBaseInstallation(aPath);

		OUString aDesktopFile;
		::osl::File::getSystemPathFromFileURL( aPath, aDesktopFile );
		aDesktopFile += OUString( RTL_CONSTASCII_USTRINGPARAM( "/share/xdg/qstart.desktop" ) );

		OString aDesktopFileUnx = OUStringToOString( aDesktopFile,
													 osl_getThreadTextEncoding() );
		OString aShortcutUnx = OUStringToOString( aShortcut,
												  osl_getThreadTextEncoding() );
		symlink( aDesktopFileUnx, aShortcutUnx );
#endif // UNX
		ShutdownIcon *pIcon = ShutdownIcon::createInstance();
		if( pIcon )
			pIcon->initSystray();
    }
    else
    {
        OUString aShortcutUrl;
        ::osl::File::getFileURLFromSystemPath( aShortcut, aShortcutUrl );
        ::osl::File::remove( aShortcutUrl );
		ShutdownIcon *pIcon = getInstance();
		if( pIcon )
			pIcon->deInitSystray();
    }
#else
    (void)bActivate; // unused variable
#endif // ENABLE_QUICKSTART_APPLET
}
