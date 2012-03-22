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

#include <com/sun/star/vcl/VCLEventQueue.hxx>
#include <com/sun/star/vcl/VCLEvent.hxx>
#include <com/sun/star/vcl/VCLFrame.hxx>
#include <saldata.hxx>
#include <salframe.h>
#include <vcl/svapp.hxx>
#include <vcl/unohelp.hxx>
#include <vcl/window.hxx>
#include <comphelper/processfactory.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <vos/mutex.hxx>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/datatransfer/XTransferable.hpp>
#include <com/sun/star/datatransfer/clipboard/XClipboard.hpp>
#include <com/sun/star/frame/XDispatchHelper.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XFramesSupplier.hpp>

#include <premac.h>
#include <ApplicationServices/ApplicationServices.h>
#include <postmac.h>

#include "VCLEventQueue_cocoa.h"

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

JNIEXPORT jboolean JNICALL Java_com_sun_star_vcl_VCLEventQueue_isApplicationActive( JNIEnv *pEnv, jobject object )
{
	return ( NSApplication_isActive() ? JNI_TRUE : JNI_FALSE );
}

// ----------------------------------------------------------------------------

JNIEXPORT jboolean JNICALL Java_com_sun_star_vcl_VCLEventQueue_isApplicationMainThread( JNIEnv *pEnv, jobject object )
{
	jboolean bRet = JNI_FALSE;

	if ( CFRunLoopGetCurrent() == CFRunLoopGetMain() )
	{
		bRet = JNI_TRUE;
		CFRunLoopRunInMode( CFSTR( "AWTRunLoopMode" ), 0, false );
	}

	return bRet;
}

// ----------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_com_sun_star_vcl_VCLEventQueue_runApplicationMainThreadTimers( JNIEnv *pEnv, jobject object )
{
	if ( CFRunLoopGetCurrent() == CFRunLoopGetMain() )
		CFRunLoopRunInMode( CFSTR( "AWTRunLoopMode" ), 0, false );
}

// ============================================================================

void VCLEventQueue_fullScreen( void *pNSWindow, BOOL bFullScreen )
{
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
				if ( (*it)->mbVisible && (*it)->mpVCLFrame->getNativeWindow() == pNSWindow )
				{
					pFrame = *it;
					break;
				}
			}

			if ( pFrame && !pFrame->mbInShowFullScreen && bFullScreen != pFrame->mbFullScreen )
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
													pFrame->mbInWindowDidExitFullScreen = !bFullScreen;
													pFrame->mbInWindowWillEnterFullScreen = bFullScreen;

													Any aRet = xDispatchHelper->executeDispatch( xDispatchProvider, OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:FullScreen" ) ), OUString( RTL_CONSTASCII_USTRINGPARAM( "_self" ) ), 0, Sequence< PropertyValue >() );

													pFrame->mbInWindowDidExitFullScreen = FALSE;
													pFrame->mbInWindowWillEnterFullScreen = FALSE;
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
}

// ----------------------------------------------------------------------------

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
				if ( (*it)->mbVisible && (*it)->mpVCLFrame->getNativeWindow() == pNSWindow )
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
					Reference< XClipboard > xClipboard = pWindow->GetPrimarySelection();
					if ( xClipboard.is() )
					{
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
										{
											*pRTFSelection = CFDataCreate( NULL, (const UInt8 *)aData.getArray(), aData.getLength() );
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
				if ( (*it)->mbVisible && (*it)->mpVCLFrame->getNativeWindow() == pNSWindow )
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

BOOL VCLEventQueue_postCommandEvent( jobject aPeer, short nKey, short nModifiers, jchar nOriginalKeyChar, short nOriginalModifiers )
{
	BOOL bRet = FALSE;

	if ( aPeer )
		bRet = com_sun_star_vcl_VCLEventQueue::postCommandEvent( aPeer, nKey, nModifiers & KEY_SHIFT ? sal_True : sal_False, nModifiers & KEY_MOD1 ? sal_True : sal_False, nModifiers & KEY_MOD2 ? sal_True : sal_False, nModifiers & KEY_MOD3 ? sal_True : sal_False, nOriginalKeyChar, nOriginalModifiers & KEY_SHIFT ? sal_True : sal_False, nOriginalModifiers & KEY_MOD1 ? sal_True : sal_False, nOriginalModifiers & KEY_MOD2 ? sal_True : sal_False, nOriginalModifiers & KEY_MOD3 ? sal_True : sal_False );

	return bRet;
}

// ----------------------------------------------------------------------------

void VCLEventQueue_postMouseWheelEvent( jobject aPeer, long nX, long nY, long nRotationX, long nRotationY, BOOL bShiftDown, BOOL bMetaDown, BOOL bAltDown, BOOL bControlDown )
{
	if ( aPeer )
		com_sun_star_vcl_VCLEventQueue::postMouseWheelEvent( aPeer, nX, nY, nRotationX, nRotationY, bShiftDown, bMetaDown, bAltDown, bControlDown );
}

// ----------------------------------------------------------------------------

void VCLEventQueue_postWindowMoveSessionEvent( jobject aPeer, long nX, long nY, BOOL bStartSession )
{
	if ( aPeer )
		com_sun_star_vcl_VCLEventQueue::postWindowMoveSessionEvent( aPeer, nX, nY, bStartSession );
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
					pSalData->mpEventQueue->removeCachedEvents( *it );
			}
		}

		rSolarMutex.release();
	}
}

// ============================================================================

jclass com_sun_star_vcl_VCLEventQueue::theClass = NULL;

// ----------------------------------------------------------------------------

jclass com_sun_star_vcl_VCLEventQueue::getMyClass()
{
	if ( !theClass )
	{
		VCLThreadAttach t;
		if ( !t.pEnv ) return (jclass)NULL;

		// Fix bugs 1390 and 1619 by inserting our own Cocoa class in place of
		// the NSView class. We need to do this because the JVM does not
		// properly handle key events where a single key press generates more
		// than one Unicode character.
		VCLEventQueue_installVCLEventQueueClasses();

		jclass tempClass = t.pEnv->FindClass( "com/sun/star/vcl/VCLEventQueue" );
		OSL_ENSURE( tempClass, "Java : FindClass not found!" );

		if ( tempClass )
		{
			// Register the native methods for our class
			JNINativeMethod pMethods[3]; 
			pMethods[0].name = "isApplicationActive";
			pMethods[0].signature = "()Z";
			pMethods[0].fnPtr = (void *)Java_com_sun_star_vcl_VCLEventQueue_isApplicationActive;
			pMethods[1].name = "isApplicationMainThread";
			pMethods[1].signature = "()Z";
			pMethods[1].fnPtr = (void *)Java_com_sun_star_vcl_VCLEventQueue_isApplicationMainThread;
			pMethods[2].name = "runApplicationMainThreadTimers";
			pMethods[2].signature = "()V";
			pMethods[2].fnPtr = (void *)Java_com_sun_star_vcl_VCLEventQueue_runApplicationMainThreadTimers;
			t.pEnv->RegisterNatives( tempClass, pMethods, 3 );
		}

		// Apple periodically deprecates methods in the com.apple.eawt package
		// so try to dynamically initialize the that class in case any
		// deprecated methods invoked in the com.sun.star.vcl.VCLEventQueue
		// class' static initizer no longer work
		jclass cAppMenuBarHandlerClass = t.pEnv->FindClass( "com/apple/eawt/_AppMenuBarHandler" );
		if ( cAppMenuBarHandlerClass )
		{
			jmethodID mIDGetInstance = NULL;
			jmethodID mIDSetPrefsMenu = NULL;
			if ( !mIDGetInstance )
			{
				char *cSignature = "()Lcom/apple/eawt/_AppMenuBarHandler;";
				mIDGetInstance = t.pEnv->GetStaticMethodID( cAppMenuBarHandlerClass, "getInstance", cSignature );
			}
			OSL_ENSURE( mIDGetInstance, "Unknown method id!" );
			if ( !mIDSetPrefsMenu )
			{
				char *cSignature = "(Z)V";
				mIDSetPrefsMenu = t.pEnv->GetMethodID( cAppMenuBarHandlerClass, "setPreferencesMenuItemVisible", cSignature );
			}
			OSL_ENSURE( mIDSetPrefsMenu , "Unknown method id!" );
			if ( mIDGetInstance && mIDSetPrefsMenu )
			{
				jobject tempObj = t.pEnv->CallStaticObjectMethod( cAppMenuBarHandlerClass, mIDGetInstance );
				if ( tempObj )
				{
					jvalue args[1];
					args[0].z = JNI_TRUE;
					t.pEnv->CallNonvirtualVoidMethodA( tempObj, cAppMenuBarHandlerClass, mIDSetPrefsMenu, args );
				}
			}
		}

		// Handle Java versions that do not have a newer version of the
		// com.apple.eawt package
		if ( t.pEnv->ExceptionCheck() )
			t.pEnv->ExceptionClear();

		theClass = (jclass)t.pEnv->NewGlobalRef( tempClass );
	}
	return theClass;
}

// ----------------------------------------------------------------------------

sal_Bool com_sun_star_vcl_VCLEventQueue::postCommandEvent( jobject _par0, short _par1, sal_Bool _par2, sal_Bool _par3, sal_Bool _par4, sal_Bool _par5, jchar _par6, sal_Bool _par7, sal_Bool _par8, sal_Bool _par9, sal_Bool _par10 )
{
	static jmethodID mID = NULL;
	sal_Bool out = sal_False;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "(Ljava/lang/Object;IZZZZCZZZZ)Z";
			mID = t.pEnv->GetStaticMethodID( getMyClass(), "postCommandEvent", cSignature );	
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			jvalue args[11];
			args[0].l = _par0;
			args[1].i = jint( _par1 );
			args[2].z = jboolean( _par2 );
			args[3].z = jboolean( _par3 );
			args[4].z = jboolean( _par4 );
			args[5].z = jboolean( _par5 );
			args[6].c = _par6;
			args[7].z = jboolean( _par7 );
			args[8].z = jboolean( _par8 );
			args[9].z = jboolean( _par9 );
			args[10].z = jboolean( _par10 );
			out = (sal_Bool)t.pEnv->CallStaticBooleanMethodA( getMyClass(), mID, args );
		}
	}

	return out;
}

// ----------------------------------------------------------------------------

void com_sun_star_vcl_VCLEventQueue::postMouseWheelEvent( jobject _par0, long _par1, long _par2, long _par3, long _par4, sal_Bool _par5, sal_Bool _par6, sal_Bool _par7, sal_Bool _par8 )
{
	static jmethodID mID = NULL;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "(Ljava/lang/Object;IIIIZZZZ)V";
			mID = t.pEnv->GetStaticMethodID( getMyClass(), "postMouseWheelEvent", cSignature );	
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			jvalue args[9];
			args[0].l = _par0;
			args[1].i = jint( _par1 );
			args[2].i = jint( _par2 );
			args[3].i = jint( _par3 );
			args[4].i = jint( _par4 );
			args[5].z = jboolean( _par5 );
			args[6].z = jboolean( _par6 );
			args[7].z = jboolean( _par7 );
			args[8].z = jboolean( _par8 );
			t.pEnv->CallStaticVoidMethodA( getMyClass(), mID, args );
		}
	}
}

// ----------------------------------------------------------------------------

void com_sun_star_vcl_VCLEventQueue::postWindowMoveSessionEvent( jobject _par0, long _par1, long _par2, sal_Bool _par3 )
{
	static jmethodID mID = NULL;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "(Ljava/lang/Object;IIZ)V";
			mID = t.pEnv->GetStaticMethodID( getMyClass(), "postWindowMoveSessionEvent", cSignature );	
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			jvalue args[4];
			args[0].l = _par0;
			args[1].i = jint( _par1 );
			args[2].i = jint( _par2 );
			args[3].z = jboolean( _par3 );
			t.pEnv->CallStaticVoidMethodA( getMyClass(), mID, args );
		}
	}
}

// ----------------------------------------------------------------------------

com_sun_star_vcl_VCLEventQueue::com_sun_star_vcl_VCLEventQueue( jobject myObj ) : java_lang_Object( (jobject)NULL )
{
	static jmethodID mID = NULL;
	VCLThreadAttach t;
	if ( !t.pEnv )
		return;
	if ( !mID )
	{
		char *cSignature = "()V";
		mID = t.pEnv->GetMethodID( getMyClass(), "<init>", cSignature );
	}
	OSL_ENSURE( mID, "Unknown method id!" );

	jobject tempObj = t.pEnv->NewObject( getMyClass(), mID );
	saveRef( tempObj );
}

// ----------------------------------------------------------------------------

sal_Bool com_sun_star_vcl_VCLEventQueue::anyCachedEvent( USHORT _par0 )
{
	static jmethodID mID = NULL;
	sal_Bool out = sal_False;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "(I)Z";
			mID = t.pEnv->GetMethodID( getMyClass(), "anyCachedEvent", cSignature );	
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			jvalue args[1];
			args[0].i = jint( _par0 );
			out = (sal_Bool)t.pEnv->CallNonvirtualBooleanMethodA( object, getMyClass(), mID, args );
		}
	}
	return out;
}

// ----------------------------------------------------------------------------

void com_sun_star_vcl_VCLEventQueue::dispatchNextEvent()
{
	static jmethodID mID = NULL;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "()V";
			mID = t.pEnv->GetMethodID( getMyClass(), "dispatchNextEvent", cSignature );	
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
			t.pEnv->CallNonvirtualVoidMethod( object, getMyClass(), mID );
	}
}

// ----------------------------------------------------------------------------

com_sun_star_vcl_VCLEvent *com_sun_star_vcl_VCLEventQueue::getNextCachedEvent( ULONG _par0, sal_Bool _par1 )
{
	static jmethodID mID = NULL;
	com_sun_star_vcl_VCLEvent *out = NULL;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "(JZ)Lcom/sun/star/vcl/VCLEvent;";
			mID = t.pEnv->GetMethodID( getMyClass(), "getNextCachedEvent", cSignature );	
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			jvalue args[2];
			args[0].j = jlong( _par0 );
			args[1].z = jboolean( _par1 );
			jobject tempObj;
			tempObj = t.pEnv->CallNonvirtualObjectMethodA( object, getMyClass(), mID, args );
			if ( tempObj )
				out = new com_sun_star_vcl_VCLEvent( tempObj );
		}
	}
	return out;
}

// ----------------------------------------------------------------------------

sal_Bool com_sun_star_vcl_VCLEventQueue::isShutdownDisabled()
{
	static jmethodID mID = NULL;
	sal_Bool out = sal_False;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "()Z";
			mID = t.pEnv->GetMethodID( getMyClass(), "isShutdownDisabled", cSignature );	
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
			out = (sal_Bool)t.pEnv->CallNonvirtualBooleanMethod( object, getMyClass(), mID );
	}
	return out;
}

// ----------------------------------------------------------------------------

void com_sun_star_vcl_VCLEventQueue::postCachedEvent( const com_sun_star_vcl_VCLEvent *_par0 )
{
	static jmethodID mID = NULL;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "(Lcom/sun/star/vcl/VCLEvent;)V";
			mID = t.pEnv->GetMethodID( getMyClass(), "postCachedEvent", cSignature );	
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			jvalue args[1];
			args[0].l = _par0->getJavaObject();
			t.pEnv->CallNonvirtualVoidMethodA( object, getMyClass(), mID, args );
		}
	}
}

// ----------------------------------------------------------------------------

void com_sun_star_vcl_VCLEventQueue::removeCachedEvents( const JavaSalFrame *_par0 )
{
	static jmethodID mID = NULL;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "(J)V";
			mID = t.pEnv->GetMethodID( getMyClass(), "removeCachedEvents", cSignature );	
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			jvalue args[1];
			args[0].j = jlong( _par0 );
			t.pEnv->CallNonvirtualVoidMethodA( object, getMyClass(), mID, args );
		}
	}
}

// ----------------------------------------------------------------------------

void com_sun_star_vcl_VCLEventQueue::setShutdownDisabled( sal_Bool _par0 )
{
	// When no shutdown is no longer disabled, allow native termination and
	// dequeue any pending native open or print events
	if ( !_par0 )
	{
		ULONG nCount = Application::ReleaseSolarMutex();
		VCLEventQueue_cancelTermination();
		Application::AcquireSolarMutex( nCount );
	}

	static jmethodID mID = NULL;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "(Z)V";
			mID = t.pEnv->GetMethodID( getMyClass(), "setShutdownDisabled", cSignature );	
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			jvalue args[1];
			args[0].z = jboolean( _par0 );
			t.pEnv->CallNonvirtualVoidMethodA( object, getMyClass(), mID, args );
		}
	}
}
