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
		*pTextSelection = NULL;
	}

	if ( pRTFSelection && *pRTFSelection )
	{
		CFRelease( *pRTFSelection );
		*pRTFSelection = NULL;
	}

	if ( pNSWindow && !Application::IsShutDown() )
	{
			ACQUIRE_SOLARMUTEX
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
							Edit *pEditWindow = dynamic_cast< Edit* >( pWindow->ImplGetWindowImpl()->mpFrameData->mpFocusWin );
							if ( pEditWindow )
								pEditWindow->Copy();

							uno::Reference< datatransfer::XTransferable > xTransferable = xClipboard->getContents();
							if ( xTransferable.is() )
							{
								datatransfer::DataFlavor aFlavor;

								// Handle string selection
								if ( pTextSelection )
								{
									uno::Type aType( getCppuType( ( OUString* )0 ) );
									aFlavor.MimeType = "text/plain;charset=utf-16";
									aFlavor.DataType = aType;
									if ( xTransferable->isDataFlavorSupported( aFlavor ) )
									{
										uno::Any aValue = xTransferable->getTransferData( aFlavor );
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
									uno::Type aType( getCppuType( ( uno::Sequence< sal_Int8 >* )0 ) );
									aFlavor.MimeType = "text/richtext";
									aFlavor.DataType = aType;
									if ( xTransferable->isDataFlavorSupported( aFlavor ) )
									{
										uno::Any aValue = xTransferable->getTransferData( aFlavor );
										if ( aValue.getValueType().equals( aType ) )
										{
											uno::Sequence< sal_Int8 > aData;
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

						if ( pJavaClipboard )
							pJavaClipboard->setPrivateClipboard( sal_False );
					}
				}
			}
			RELEASE_SOLARMUTEX
	}
}

// ----------------------------------------------------------------------------

sal_Bool VCLEventQueue_paste( void *pNSWindow )
{
	sal_Bool bRet = sal_False;

	if ( !Application::IsShutDown() )
	{
			ACQUIRE_SOLARMUTEX
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
			RELEASE_SOLARMUTEX
	}

	return bRet;
}

// ----------------------------------------------------------------------------

void VCLEventQueue_removeCachedEvents()
{
	// If no application mutex exists yet, ignore event as we are likely to
	// crash
	if ( ImplApplicationIsRunning() )
	{
			ACQUIRE_SOLARMUTEX

			// Yield to give Java event dispatch thread a chance to finish
			Thread::yield();

			SalData *pSalData = GetSalData();
			for ( ::std::list< JavaSalFrame* >::const_iterator it = pSalData->maFrameList.begin(); it != pSalData->maFrameList.end(); ++it )
			{
				if ( (*it)->mbVisible )
					JavaSalEventQueue::removeCachedEvents( *it );
			}
			RELEASE_SOLARMUTEX
	}
}
