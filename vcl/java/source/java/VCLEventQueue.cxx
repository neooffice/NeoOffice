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
 *  Patrick Luby, June 2003
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2003 Planamesa Inc.
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

#import <dlfcn.h>

#include <saldata.hxx>
#include <salframe.h>
#include <salinst.h>
#include <vcl/edit.hxx>
#include <vcl/svapp.hxx>
#include <vcl/unohelp.hxx>
#include <vcl/window.h>
#include <vcl/window.hxx>
#include <comphelper/processfactory.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <vos/mutex.hxx>
#include <com/sun/star/datatransfer/clipboard/XClipboard.hpp>
#include <com/sun/star/frame/XDispatchHelper.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XFramesSupplier.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <premac.h>
#include <ApplicationServices/ApplicationServices.h>
#include <postmac.h>

#include "VCLEventQueue_cocoa.h"

typedef void Application_setPrivateClipboard_Type( ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboard > *pClipboard, sal_Bool bSystemClipboard );

static Application_setPrivateClipboard_Type *pApplication_setPrivateClipboard = NULL;

using namespace com::sun::star::awt;
using namespace com::sun::star::beans;
using namespace com::sun::star::container;
using namespace com::sun::star::datatransfer;
using namespace com::sun::star::datatransfer::clipboard;
using namespace com::sun::star::frame;
using namespace com::sun::star::uno;
using namespace vcl;
using namespace vos;

// ============================================================================

void VCLEventQueue_getTextSelection( void *pNSWindow, CFStringRef *pTextSelection, CFDataRef *pRTFSelection )
{
	if ( !pTextSelection && !pRTFSelection )
		return;

	if ( pTextSelection && *pTextSelection )
	{
		CFRelease( *pTextSelection );
		*pTextSelection = NULL;
	}

	if ( pRTFSelection && *pRTFSelection )
	{
		CFRelease( *pRTFSelection );
		*pRTFSelection = NULL;
	}

	if ( pNSWindow && !Application::IsShutDown() )
	{
		IMutex& rSolarMutex = Application::GetSolarMutex();
		rSolarMutex.acquire();

		if ( !Application::IsShutDown() )
		{
			JavaSalFrame *pFrame = NULL;
			SalData *pSalData = GetSalData();
			for ( ::std::list< JavaSalFrame* >::const_iterator it = pSalData->maFrameList.begin(); it != pSalData->maFrameList.end(); ++it )
			{
				if ( (*it)->mbVisible && (*it)->GetNativeWindow() == pNSWindow )
				{
					pFrame = *it;
					break;
				}
			}

			if ( pFrame )
			{
				Window *pWindow = Application::GetFirstTopLevelWindow();
				while ( pWindow && pWindow->ImplGetFrame() != pFrame )
					pWindow = Application::GetNextTopLevelWindow( pWindow );

				if ( pWindow )
				{
					// Try to copy current selection to system clipboard
					Reference< XClipboard > xClipboard = pWindow->GetClipboard();
					if ( !pApplication_setPrivateClipboard )
						pApplication_setPrivateClipboard = (Application_setPrivateClipboard_Type *)dlsym( RTLD_DEFAULT, "Application_setPrivateClipboard" );
					if ( xClipboard.is() && pApplication_setPrivateClipboard )
					{
						pApplication_setPrivateClipboard( &xClipboard, sal_True );
						try
						{
							xClipboard->setContents( Reference< XTransferable >(), Reference< XClipboardOwner >() );

							Reference< XFramesSupplier > xFramesSupplier( ::comphelper::getProcessServiceFactory()->createInstance( OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.frame.Desktop" ) ) ), UNO_QUERY );
							if ( xFramesSupplier.is() )
							{
								Reference< XIndexAccess > xList( xFramesSupplier->getFrames(), UNO_QUERY );
								if ( xList.is() )
								{
									sal_Int32 nCount = xList->getCount();
									for ( sal_Int32 i = 0; i < nCount; i++ )
									{
										Reference< XFrame > xFrame;
										xList->getByIndex( i ) >>= xFrame;
										if ( xFrame.is() )
										{
											Reference< XWindow > xWindow = xFrame->getComponentWindow();
											if ( xWindow.is() )
											{
												Window *pCurrentWindow = VCLUnoHelper::GetWindow( xWindow );
												while ( pCurrentWindow && pCurrentWindow != pWindow && pCurrentWindow->GetParent() )
													pCurrentWindow = pCurrentWindow->GetParent();
												if ( pCurrentWindow == pWindow )
												
												{
													Reference< XDispatchProvider > xDispatchProvider( xFrame, UNO_QUERY );
													if ( xDispatchProvider.is() )
													{
														Reference< XDispatchHelper > xDispatchHelper( ::comphelper::getProcessServiceFactory()->createInstance( OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.frame.DispatchHelper" ) ) ), UNO_QUERY );
														if ( xDispatchHelper.is() )
															xDispatchHelper->executeDispatch( xDispatchProvider, OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:Copy" ) ), OUString( RTL_CONSTASCII_USTRINGPARAM( "_self" ) ), 0, Sequence< PropertyValue >() );
													}

													break;
												}
											}
										}
									}
								}
							}

							// If an edit window has focus, use its text
							Edit *pEditWindow = dynamic_cast< Edit* >( pWindow->ImplGetWindowImpl()->mpFrameData->mpFocusWin );
							if ( pEditWindow )
								pEditWindow->Copy();

							Reference< XTransferable > xTransferable = xClipboard->getContents();
							if ( xTransferable.is() )
							{
								DataFlavor aFlavor;

								// Handle string selection
								if ( pTextSelection )
								{
									Type aType( getCppuType( ( OUString* )0 ) );
									aFlavor.MimeType = OUString( RTL_CONSTASCII_USTRINGPARAM( "text/plain;charset=utf-16" ) );
									aFlavor.DataType = aType;
									if ( xTransferable->isDataFlavorSupported( aFlavor ) )
									{
										Any aValue = xTransferable->getTransferData( aFlavor );
										if ( aValue.getValueType().equals( aType ) )
										{
											OUString aText;
											aValue >>= aText;
											if ( aText.getLength() )
												*pTextSelection = CFStringCreateWithCharacters( NULL, aText.getStr(), aText.getLength() );
										}
									}
								}

								// Handle RTF selection
								if ( pRTFSelection )
								{
									Type aType( getCppuType( ( ::com::sun::star::uno::Sequence< sal_Int8 >* )0 ) );
									aFlavor.MimeType = OUString( RTL_CONSTASCII_USTRINGPARAM( "text/richtext" ) );
									aFlavor.DataType = aType;
									if ( xTransferable->isDataFlavorSupported( aFlavor ) )
									{
										Any aValue = xTransferable->getTransferData( aFlavor );
										if ( aValue.getValueType().equals( aType ) )
										{
											Sequence< sal_Int8 > aData;
											aValue >>= aData;
											if ( aData.getLength() )
												*pRTFSelection = CFDataCreate( NULL, (const UInt8 *)aData.getArray(), aData.getLength() );
										}
									}
								}
							}
						}
						catch( ... )
						{
						}

						pApplication_setPrivateClipboard( &xClipboard, sal_False );
					}
				}
			}
		}

		rSolarMutex.release();
	}
}

// ----------------------------------------------------------------------------

BOOL VCLEventQueue_paste( void *pNSWindow )
{
	BOOL bRet = FALSE;

	if ( !Application::IsShutDown() )
	{
		IMutex& rSolarMutex = Application::GetSolarMutex();
		rSolarMutex.acquire();

		if ( !Application::IsShutDown() )
		{
			JavaSalFrame *pFrame = NULL;
			SalData *pSalData = GetSalData();
			for ( ::std::list< JavaSalFrame* >::const_iterator it = pSalData->maFrameList.begin(); it != pSalData->maFrameList.end(); ++it )
			{
				if ( (*it)->mbVisible && (*it)->GetNativeWindow() == pNSWindow )
				{
					pFrame = *it;
					break;
				}
			}

			if ( pFrame )
			{
				Window *pWindow = Application::GetFirstTopLevelWindow();
				while ( pWindow && pWindow->ImplGetFrame() != pFrame )
					pWindow = Application::GetNextTopLevelWindow( pWindow );

				if ( pWindow )
				{
					Reference< XFramesSupplier > xFramesSupplier( ::comphelper::getProcessServiceFactory()->createInstance( OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.frame.Desktop" ) ) ), UNO_QUERY );
					if ( xFramesSupplier.is() )
					{
						Reference< XIndexAccess > xList( xFramesSupplier->getFrames(), UNO_QUERY );
						if ( xList.is() )
						{
							sal_Int32 nCount = xList->getCount();
							for ( sal_Int32 i = 0; i < nCount; i++ )
							{
								Reference< XFrame > xFrame;
								xList->getByIndex( i ) >>= xFrame;
								if ( xFrame.is() )
								{
									Reference< XWindow > xWindow = xFrame->getComponentWindow();
									if ( xWindow.is() )
									{
										Window *pCurrentWindow = VCLUnoHelper::GetWindow( xWindow );
										while ( pCurrentWindow && pCurrentWindow != pWindow && pCurrentWindow->GetParent() )
											pCurrentWindow = pCurrentWindow->GetParent();
										if ( pCurrentWindow == pWindow )
										
										{
											Reference< XDispatchProvider > xDispatchProvider( xFrame, UNO_QUERY );
											if ( xDispatchProvider.is() )
											{
												Reference< XDispatchHelper > xDispatchHelper( ::comphelper::getProcessServiceFactory()->createInstance( OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.frame.DispatchHelper" ) ) ), UNO_QUERY );
												if ( xDispatchHelper.is() )
												{
													Any aRet = xDispatchHelper->executeDispatch( xDispatchProvider, OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:Paste" ) ), OUString( RTL_CONSTASCII_USTRINGPARAM( "_self" ) ), 0, Sequence< PropertyValue >() );
													bRet = ( aRet.getValue() ? TRUE : FALSE );
												}
											}

											break;
										}
									}
								}
							}
						}
					}
				}
			}
		}

		rSolarMutex.release();
	}

	return bRet;
}

// ----------------------------------------------------------------------------

void VCLEventQueue_removeCachedEvents()
{
	if ( !Application::IsShutDown() )
	{
		IMutex& rSolarMutex = Application::GetSolarMutex();
		rSolarMutex.acquire();

		if ( !Application::IsShutDown() )
		{
			// Yield to give Java event dispatch thread a chance to finish
			OThread::yield();

			SalData *pSalData = GetSalData();
			for ( ::std::list< JavaSalFrame* >::const_iterator it = pSalData->maFrameList.begin(); it != pSalData->maFrameList.end(); ++it )
			{
				if ( (*it)->mbVisible )
					JavaSalEventQueue::removeCachedEvents( *it );
			}
		}

		rSolarMutex.release();
	}
}
