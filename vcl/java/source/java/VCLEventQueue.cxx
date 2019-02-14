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

#include <comphelper/processfactory.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/edit.hxx>
#include <vcl/svapp.hxx>
#include <vcl/unohelp.hxx>
#include <vcl/window.hxx>
#include <com/sun/star/datatransfer/clipboard/XClipboard.hpp>
#include <com/sun/star/frame/XDispatchHelper.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XFramesSupplier.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <premac.h>
#include <ApplicationServices/ApplicationServices.h>
#include <Cocoa/Cocoa.h>
#include <postmac.h>
#undef check

#include "window.h"
#include "java/saldata.hxx"
#include "java/salframe.h"
#include "java/salinst.h"

#include "VCLEventQueue_cocoa.h"
#include "../dtrans/java_clipboard.hxx"

using namespace com::sun::star::awt;
using namespace com::sun::star::beans;
using namespace com::sun::star;
using namespace osl;
using namespace vcl;

// ============================================================================

void VCLEventQueue_getTextSelection( void *pNSWindow, CFStringRef *pTextSelection, CFDataRef *pRTFSelection )
{
	if ( !pTextSelection && !pRTFSelection )
		return;

	if ( pTextSelection && *pTextSelection )
	{
		CFRelease( *pTextSelection );
		*pTextSelection = nullptr;
	}

	if ( pRTFSelection && *pRTFSelection )
	{
		CFRelease( *pRTFSelection );
		*pRTFSelection = nullptr;
	}

	if ( pNSWindow && !Application::IsShutDown() )
	{
		comphelper::SolarMutex& rSolarMutex = Application::GetSolarMutex();
		rSolarMutex.acquire();

		if ( !Application::IsShutDown() )
		{
			JavaSalFrame *pFrame = nullptr;
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
					uno::Reference< datatransfer::clipboard::XClipboard > xClipboard = pWindow->GetClipboard();
					if ( xClipboard.is() )
					{
						JavaClipboard *pJavaClipboard = dynamic_cast< JavaClipboard* >( xClipboard.get() );
						if ( pJavaClipboard )
							pJavaClipboard->setPrivateClipboard( sal_True );

						try
						{
							xClipboard->setContents( uno::Reference< datatransfer::XTransferable >(), uno::Reference< datatransfer::clipboard::XClipboardOwner >() );

							uno::Reference< frame::XFramesSupplier > xFramesSupplier( ::comphelper::getProcessServiceFactory()->createInstance( "com.sun.star.frame.Desktop" ), uno::UNO_QUERY );
							if ( xFramesSupplier.is() )
							{
								uno::Reference< container::XIndexAccess > xList( xFramesSupplier->getFrames(), uno::UNO_QUERY );
								if ( xList.is() )
								{
									sal_Int32 nCount = xList->getCount();
									for ( sal_Int32 i = 0; i < nCount; i++ )
									{
										uno::Reference< frame::XFrame > xFrame;
										xList->getByIndex( i ) >>= xFrame;
										if ( xFrame.is() )
										{
											uno::Reference< XWindow > xWindow = xFrame->getComponentWindow();
											if ( xWindow.is() )
											{
												Window *pCurrentWindow = VCLUnoHelper::GetWindow( xWindow );
												while ( pCurrentWindow && pCurrentWindow != pWindow && pCurrentWindow->GetParent() )
													pCurrentWindow = pCurrentWindow->GetParent();
												if ( pCurrentWindow == pWindow )
												
												{
													uno::Reference< frame::XDispatchProvider > xDispatchProvider( xFrame, uno::UNO_QUERY );
													if ( xDispatchProvider.is() )
													{
														uno::Reference< frame::XDispatchHelper > xDispatchHelper( ::comphelper::getProcessServiceFactory()->createInstance( "com.sun.star.frame.DispatchHelper" ), uno::UNO_QUERY );
														if ( xDispatchHelper.is() )
															xDispatchHelper->executeDispatch( xDispatchProvider, ".uno:Copy", "_self", 0, uno::Sequence< PropertyValue >() );
													}

													break;
												}
											}
										}
									}
								}
							}

							// If an edit window has focus, use its text
							vcl::Window *pFocusWindow = pWindow->ImplGetWindowImpl()->mpFrameData->mpFocusWin;
							if ( pFocusWindow )
							{
								Edit *pEditWindow = dynamic_cast< Edit* >( pFocusWindow );
								if ( pEditWindow )
									pEditWindow->Copy();
							}

							uno::Reference< datatransfer::XTransferable > xTransferable = xClipboard->getContents();
							if ( xTransferable.is() )
							{
								datatransfer::DataFlavor aFlavor;

								// Handle string selection
								if ( pTextSelection )
								{
									aFlavor.MimeType = "text/plain;charset=utf-16";
									aFlavor.DataType = cppu::UnoType< OUString >::get();
									if ( xTransferable->isDataFlavorSupported( aFlavor ) )
									{
										uno::Any aValue = xTransferable->getTransferData( aFlavor );
										if ( aValue.getValueType().equals( aFlavor.DataType ) )
										{
											OUString aText;
											aValue >>= aText;
											if ( aText.getLength() )
												*pTextSelection = CFStringCreateWithCharacters( nullptr, reinterpret_cast< const UniChar* >( aText.getStr() ), aText.getLength() );
										}
									}
								}

								// Handle RTF selection
								if ( pRTFSelection )
								{
									aFlavor.MimeType = "text/richtext";
									aFlavor.DataType = cppu::UnoType< uno::Sequence< sal_Int8 > >::get();
									if ( xTransferable->isDataFlavorSupported( aFlavor ) )
									{
										uno::Any aValue = xTransferable->getTransferData( aFlavor );
										if ( aValue.getValueType().equals( aFlavor.DataType ) )
										{
											uno::Sequence< sal_Int8 > aData;
											aValue >>= aData;
											if ( aData.getLength() )
												*pRTFSelection = CFDataCreate( nullptr, reinterpret_cast< const UInt8* >( aData.getArray() ), aData.getLength() );
										}
									}
								}
							}
						}
						catch( ... )
						{
						}

						if ( pJavaClipboard )
							pJavaClipboard->setPrivateClipboard( sal_False );
					}
				}
			}
		}

		rSolarMutex.release();
	}
}

// ----------------------------------------------------------------------------

sal_Bool VCLEventQueue_paste( void *pNSWindow )
{
	sal_Bool bRet = sal_False;

	if ( !Application::IsShutDown() )
	{
		comphelper::SolarMutex& rSolarMutex = Application::GetSolarMutex();
		rSolarMutex.acquire();

		if ( !Application::IsShutDown() )
		{
			JavaSalFrame *pFrame = nullptr;
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
					uno::Reference< frame::XFramesSupplier > xFramesSupplier( ::comphelper::getProcessServiceFactory()->createInstance( "com.sun.star.frame.Desktop" ), uno::UNO_QUERY );
					if ( xFramesSupplier.is() )
					{
						uno::Reference< container::XIndexAccess > xList( xFramesSupplier->getFrames(), uno::UNO_QUERY );
						if ( xList.is() )
						{
							sal_Int32 nCount = xList->getCount();
							for ( sal_Int32 i = 0; i < nCount; i++ )
							{
								uno::Reference< frame::XFrame > xFrame;
								xList->getByIndex( i ) >>= xFrame;
								if ( xFrame.is() )
								{
									uno::Reference< XWindow > xWindow = xFrame->getComponentWindow();
									if ( xWindow.is() )
									{
										Window *pCurrentWindow = VCLUnoHelper::GetWindow( xWindow );
										while ( pCurrentWindow && pCurrentWindow != pWindow && pCurrentWindow->GetParent() )
											pCurrentWindow = pCurrentWindow->GetParent();
										if ( pCurrentWindow == pWindow )
										
										{
											uno::Reference< frame::XDispatchProvider > xDispatchProvider( xFrame, uno::UNO_QUERY );
											if ( xDispatchProvider.is() )
											{
												uno::Reference< frame::XDispatchHelper > xDispatchHelper( ::comphelper::getProcessServiceFactory()->createInstance( "com.sun.star.frame.DispatchHelper" ), uno::UNO_QUERY );
												if ( xDispatchHelper.is() )
												{
													uno::Any aRet = xDispatchHelper->executeDispatch( xDispatchProvider, ".uno:Paste", "_self", 0, uno::Sequence< PropertyValue >() );
													bRet = ( aRet.getValue() ? sal_True : sal_False );
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
		comphelper::SolarMutex& rSolarMutex = Application::GetSolarMutex();
		rSolarMutex.acquire();

		if ( !Application::IsShutDown() )
		{
			// Yield to give Java event dispatch thread a chance to finish
			Thread::yield();

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
