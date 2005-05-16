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
 *  Modified May 2005 by Patrick Luby. SISSL Removed. NeoOffice is
 *  distributed under GPL only under modification term 3 of the LGPL.
 *
 *  Contributor(s): _______________________________________
 *
 ************************************************************************/

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

#if defined USE_JAVA && defined MACOSX

#include <premac.h>
#include <Carbon/Carbon.h>
#include <postmac.h>

#ifndef INCLUDED_SVTOOLS_MODULEOPTIONS_HXX
#include <svtools/moduleoptions.hxx>
#endif

#define WRITER_URL			"private:factory/swriter"
#define CALC_URL			"private:factory/scalc"
#define IMPRESS_URL			"private:factory/simpress"
#define IMPRESS_WIZARD_URL	"private:factory/simpress?slot=10425"
#define DRAW_URL			"private:factory/sdraw"
#define MATH_URL			"private:factory/smath"

#define WRITER_COMMAND_ID			'SDI1'
#define CALC_COMMAND_ID				'SDI2'
#define IMPRESS_COMMAND_ID			'SDI3'
#define DRAW_COMMAND_ID				'SDI4'
#define MATH_COMMAND_ID				'SDI5'
#define TEMPLATE_COMMAND_ID			'SDI6'
#define OPEN_COMMAND_ID				'SDI7'

#endif	// USE_JAVA && MACOSX

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

#if defined USE_JAVA && defined MACOSX

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
		default:
			break;
	}

	delete this;

	return 0;
}

static OSStatus CarbonEventHandler( EventHandlerCallRef aNextHandler, EventRef aEvent, void *pData )
{
	if ( !Application::IsShutDown() )
	{
		if ( GetEventClass( aEvent ) == kEventClassCommand && GetEventKind( aEvent ) == kEventCommandProcess )
		{
			HICommand aCommand;
			if ( GetEventParameter( aEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof( HICommand ), NULL, &aCommand ) == noErr )
			{
				MenuCommand nCommand = aCommand.commandID;
				switch ( nCommand )
				{
					case WRITER_COMMAND_ID:
					case CALC_COMMAND_ID:
					case IMPRESS_COMMAND_ID:
					case DRAW_COMMAND_ID:
					case MATH_COMMAND_ID:
					case TEMPLATE_COMMAND_ID:
					case OPEN_COMMAND_ID:
					{
						ShutdownIconEvent *pEvent = new ShutdownIconEvent( nCommand );
						Application::PostUserEvent( LINK( pEvent, ShutdownIconEvent, DispatchEvent ) );
						return noErr;
					}
					default:
					{
						break;
					}
				}
			}
		}
	}

	return CallNextEventHandler( aNextHandler, aEvent );
}

#endif	// USE_JAVA && MACOSX

class SfxNotificationListener_Impl : public cppu::WeakImplHelper1< XDispatchResultListener >
{
public:
    virtual void SAL_CALL dispatchFinished( const DispatchResultEvent& aEvent ) throw( RuntimeException );
    virtual void SAL_CALL disposing( const EventObject& aEvent ) throw( RuntimeException );
};

void SAL_CALL SfxNotificationListener_Impl::dispatchFinished( const DispatchResultEvent& aEvent ) throw( RuntimeException )
{
#ifdef WNT
	ShutdownIcon::LeaveModalMode();
#endif
}

void SAL_CALL SfxNotificationListener_Impl::disposing( const EventObject& aEvent ) throw( RuntimeException )
{
}

SFX_IMPL_XSERVICEINFO( ShutdownIcon, "com.sun.star.office.Quickstart", "com.sun.star.comp.desktop.QuickstartWrapper" )	\
SFX_IMPL_ONEINSTANCEFACTORY( ShutdownIcon );

ShutdownIcon* ShutdownIcon::pShutdownIcon = 0;

ShutdownIcon::ShutdownIcon( Reference< XMultiServiceFactory > aSMgr ) :
	ShutdownIconServiceBase( m_aMutex ),
	m_xServiceManager( aSMgr ),
	m_pResMgr( 0 ),
	m_bVeto ( false )
{
}

ShutdownIcon::~ShutdownIcon()
{
#ifdef WNT
    deInitSystray();
#endif
}

// ---------------------------------------------------------------------------

void ShutdownIcon::SetAutostart( bool bActivate )
{
#ifdef WNT
    OUString aShortcutName( RTL_CONSTASCII_USTRINGPARAM( "StarOffice 6.0" ) );
    if( SFX_APP() && SFX_APP()->GetSfxResManager() )
    {
        ::vos::OGuard aGuard( Application::GetSolarMutex() );
        UniString aRes( ResId( STR_QUICKSTART_LNKNAME, SFX_APP()->GetSfxResManager() ) );
        aShortcutName = OUString( aRes );
    }
    aShortcutName += OUString( RTL_CONSTASCII_USTRINGPARAM( ".lnk" ) );

    SetAutostartW32( aShortcutName, bActivate );
#endif
}

bool ShutdownIcon::GetAutostart( )
{
#ifdef WNT
    OUString aShortcutName( RTL_CONSTASCII_USTRINGPARAM( "StarOffice 6.0" ) );
    if( SFX_APP() && SFX_APP()->GetSfxResManager() )
    {
        ::vos::OGuard aGuard( Application::GetSolarMutex() );
        UniString aRes( ResId( STR_QUICKSTART_LNKNAME, SFX_APP()->GetSfxResManager() ) );
        aShortcutName = OUString( aRes );
    }
    aShortcutName += OUString( RTL_CONSTASCII_USTRINGPARAM( ".lnk" ) );

    return GetAutostartW32( aShortcutName );
#else
    return false;
#endif
}


// ---------------------------------------------------------------------------

void ShutdownIcon::OpenURL( const ::rtl::OUString& aURL, const ::rtl::OUString& rTarget, const Sequence< PropertyValue >& aArgs )
{
    if ( getInstance() && getInstance()->m_xDesktop.is() )
    {
        Reference < XComponentLoader > xLoader ( getInstance()->m_xDesktop, UNO_QUERY );
        if( xLoader.is() )
        {
            try
            {
                xLoader->loadComponentFromURL(
                    aURL,
                    rTarget,
                    0,
                    aArgs );
            }
            catch( IOException )
            {
                OSL_ENSURE( 0, "IOException in loadComponentFromURL!");
            }
            catch( IllegalArgumentException )
            {
                OSL_ENSURE( 0, "IllegalArgumentException in loadComponentFromURL!");
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
#ifdef WNT
		EnterModalMode();
#endif
		// use ctor for filling up filters automatically! #89169#
		FileDialogHelper dlg( WB_OPEN | SFXWB_MULTISELECTION, *(SfxObjectFactory*) NULL );
        if ( ERRCODE_NONE == dlg.Execute() )
		{
			Reference< XFilePicker >	xPicker = dlg.GetFilePicker();

			try
			{

				if ( xPicker.is() )
				{

					Reference < XFilePickerControlAccess > xPickerControls ( xPicker, UNO_QUERY );
					Reference < XFilterManager > xFilterManager ( xPicker, UNO_QUERY );

					Sequence< OUString >		sFiles = xPicker->getFiles();
					int							nFiles = sFiles.getLength();

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
					OUString aFilterName( dlg.GetCurrentFilter() );

					if ( xPickerControls.is() )
					{

						// Set readonly flag

						sal_Bool	bReadOnly = sal_False;


						xPickerControls->getValue( ExtendedFilePickerElementIds::CHECKBOX_READONLY, 0 ) >>= bReadOnly;

						// #95239#: Only set porperty if readonly is set to TRUE

						if ( bReadOnly )
						{
							aArgs.realloc( ++nArgs );
							aArgs[nArgs-1].Name  = OUString::createFromAscii( "ReadOnly" );
							aArgs[nArgs-1].Value <<= bReadOnly;
						}

						// Get version string

						sal_Int32	iVersion = -1;

						xPickerControls->getValue( ExtendedFilePickerElementIds::LISTBOX_VERSION, ControlActions::GET_SELECTED_ITEM_INDEX ) >>= iVersion;

						if ( iVersion >= 0 )
						{
							sal_Int16	uVersion = (sal_Int16)iVersion;

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
						OUString	aBaseDirURL = sFiles[0];
						if ( aBaseDirURL.getLength() > 0 && aBaseDirURL[aBaseDirURL.getLength()-1] != '/' )
							aBaseDirURL += OUString::createFromAscii("/");

						int	iFiles;
						for ( iFiles = 1; iFiles < nFiles; iFiles++ )
						{
							OUString	aURL = aBaseDirURL;
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
#ifdef WNT
				EnterModalMode();
#endif
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
    }
}

// ---------------------------------------------------------------------------

ShutdownIcon* ShutdownIcon::getInstance()
{
	OSL_ASSERT( pShutdownIcon );
	return pShutdownIcon;
}

// ---------------------------------------------------------------------------

void SAL_CALL ShutdownIcon::disposing()
{
	m_xServiceManager = Reference< XMultiServiceFactory >();
	m_xDesktop = Reference< XDesktop >();
}

// ---------------------------------------------------------------------------

// XEventListener
void SAL_CALL ShutdownIcon::disposing( const ::com::sun::star::lang::EventObject& Source )
	throw(::com::sun::star::uno::RuntimeException)
{
}

// ---------------------------------------------------------------------------

// XTerminateListener
void SAL_CALL ShutdownIcon::queryTermination( const ::com::sun::star::lang::EventObject& aEvent )
throw(::com::sun::star::frame::TerminationVetoException, ::com::sun::star::uno::RuntimeException)
{
	::osl::ClearableMutexGuard	aGuard(	m_aMutex );

	if ( m_bVeto )
		throw ::com::sun::star::frame::TerminationVetoException();
}


// ---------------------------------------------------------------------------

void SAL_CALL ShutdownIcon::notifyTermination( const ::com::sun::star::lang::EventObject& aEvent )
throw(::com::sun::star::uno::RuntimeException)
{
}


// ---------------------------------------------------------------------------

void SAL_CALL ShutdownIcon::initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any>& aArguments )
	throw( ::com::sun::star::uno::Exception )
{
	::osl::ResettableMutexGuard	aGuard(	m_aMutex );

	if ( aArguments.getLength() > 0 )
	{
		if ( !ShutdownIcon::pShutdownIcon )
		{
			try
			{
				sal_Bool bQuickstart = sal_False;
				bQuickstart = ::cppu::any2bool( aArguments[0] );
				{
					aGuard.clear();
					// access resource system and sfx only protected by solarmutex
					vos::OGuard aSolarGuard( Application::GetSolarMutex() );

					if( Application::IsRemoteServer() || ( !bQuickstart && !GetAutostart() ) )
						return;

					m_pResMgr = SFX_APP()->GetSfxResManager();
				}
				aGuard.reset();

				m_xDesktop = Reference < XDesktop >( m_xServiceManager->createInstance(
															DEFINE_CONST_UNICODE( "com.sun.star.frame.Desktop" )),
														UNO_QUERY );

				if ( !m_xDesktop.is() )
					return;

				ShutdownIcon::pShutdownIcon = this;
#ifdef WNT
				initSystray();
#elif defined USE_JAVA && defined MACOSX
				// Disable shutdown
				SetVeto( true );
				addTerminateListener();

				// Add Quickstart menu items
				SvtModuleOptions aModuleOptions;
				MenuRef aAppMenu;

				MenuRef aRootMenu = AcquireRootMenu();
				if ( aRootMenu && GetMenuItemHierarchicalMenu( aRootMenu, 1, &aAppMenu ) == noErr )
				{
					// Insert a spacing menu item
					OUString aDesc( OUString::createFromAscii( "-" ) );
					CFStringRef aString = CFStringCreateWithCharacters( NULL, aDesc.getStr(), aDesc.getLength() );
					if ( aString )
					{
						InsertMenuItemTextWithCFString( aAppMenu, aString, 0, 0, 0 );
						CFRelease( aString );
					}

					if ( aModuleOptions.IsMath() )
					{
						aDesc = GetUrlDescription( OUString::createFromAscii( MATH_URL ) );
						aString = CFStringCreateWithCharacters( NULL, aDesc.getStr(), aDesc.getLength() );
						if ( aString )
						{
							if ( CFStringGetLength( aString ) )
								InsertMenuItemTextWithCFString( aAppMenu, aString, 0, 0, MATH_COMMAND_ID );
							CFRelease( aString );
						}
					}

					if ( aModuleOptions.IsDraw() )
					{
						aDesc = GetUrlDescription( OUString::createFromAscii( DRAW_URL ) );
						aString = CFStringCreateWithCharacters( NULL, aDesc.getStr(), aDesc.getLength() );
						if ( aString )
						{
							if ( CFStringGetLength( aString ) )
								InsertMenuItemTextWithCFString( aAppMenu, aString, 0, 0, DRAW_COMMAND_ID );
							CFRelease( aString );
						}
					}

					if ( aModuleOptions.IsImpress() )
					{
						aDesc = GetUrlDescription( OUString::createFromAscii( IMPRESS_URL ) );
						aString = CFStringCreateWithCharacters( NULL, aDesc.getStr(), aDesc.getLength() );
						if ( aString )
						{
							if ( CFStringGetLength( aString ) )
								InsertMenuItemTextWithCFString( aAppMenu, aString, 0, 0, IMPRESS_COMMAND_ID );
							CFRelease( aString );
						}
					}

					if ( aModuleOptions.IsCalc() )
					{
						aDesc = GetUrlDescription( OUString::createFromAscii( CALC_URL ) );
						aString = CFStringCreateWithCharacters( NULL, aDesc.getStr(), aDesc.getLength() );
						if ( aString )
						{
							if ( CFStringGetLength( aString ) )
								InsertMenuItemTextWithCFString( aAppMenu, aString, 0, 0, CALC_COMMAND_ID );
							CFRelease( aString );
						}
					}

					if ( aModuleOptions.IsWriter() )
					{
						aDesc = GetUrlDescription( OUString::createFromAscii( WRITER_URL ) );
						aString = CFStringCreateWithCharacters( NULL, aDesc.getStr(), aDesc.getLength() );
						if ( aString )
						{
							if ( CFStringGetLength( aString ) )
								InsertMenuItemTextWithCFString( aAppMenu, aString, 0, 0, WRITER_COMMAND_ID );
							CFRelease( aString );
						}
					}

					EventHandlerUPP pEventHandlerUPP = NewEventHandlerUPP( CarbonEventHandler );
					if ( pEventHandlerUPP )
					{
						EventTypeSpec aType;
						aType.eventClass = kEventClassCommand;
						aType.eventKind = kEventCommandProcess;
						InstallApplicationEventHandler( pEventHandlerUPP, 1, &aType, NULL, NULL );
					}

					ReleaseMenu( aRootMenu );
				}
#endif
			}
			catch(const ::com::sun::star::lang::IllegalArgumentException&)
			{
			}
		}
	}
}
