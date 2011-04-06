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
 *  Patrick Luby, August 2010
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2010 Planamesa Inc.
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

#include <dlfcn.h>
#include <list>

#include <saldata.hxx>
#include <salframe.h>
#include <com/sun/star/vcl/VCLEvent.hxx>
#include <rtl/ustring.hxx>
#include <vcl/svapp.hxx>
#include <vos/mutex.hxx>

#include <premac.h>
#import <Cocoa/Cocoa.h>
#include <postmac.h>

#include "VCLApplicationDelegate_cocoa.h"
#import "../app/salinst_cocoa.h"

typedef void KeyScript_Type( short nCode );

struct ImplPendingOpenPrintFileRequest
{
	::rtl::OString		maPath;
	sal_Bool			mbPrint;

						ImplPendingOpenPrintFileRequest( const ::rtl::OString &rPath, sal_Bool bPrint ) : maPath( rPath ), mbPrint( bPrint ) {}
						~ImplPendingOpenPrintFileRequest() {};
};

static NSString *pApplicationDelegateString = @"ApplicationDelegate";
static std::list< ImplPendingOpenPrintFileRequest* > aPendingOpenPrintFileRequests;

using namespace rtl;
using namespace vcl;
using namespace vos;

static void HandleAboutRequest()
{
	if ( !Application::IsShutDown() )
	{
		// If no Java event queue exists yet, ignore event as we are likely to
		// deadlock
		SalData *pSalData = GetSalData();
		if ( pSalData && pSalData->mpEventQueue )
		{
			IMutex& rSolarMutex = Application::GetSolarMutex();
			rSolarMutex.acquire();

			if ( !Application::IsShutDown() )
			{
				com_sun_star_vcl_VCLEvent aEvent( SALEVENT_ABOUT, NULL, NULL);
				pSalData->mpEventQueue->postCachedEvent( &aEvent );
			}

			rSolarMutex.release();
		}
	}
}

static void HandleOpenPrintFileRequest( const OString &rPath, sal_Bool bPrint )
{
	if ( rPath.getLength() && !Application::IsShutDown() )
	{
		// If no Java event queue exists yet, queue event as we are likely to
		// deadlock
		SalData *pSalData = GetSalData();
		if ( !pSalData || !pSalData->mpEventQueue )
		{
			ImplPendingOpenPrintFileRequest *pRequest = new ImplPendingOpenPrintFileRequest( rPath, bPrint );
			if ( pRequest )
				aPendingOpenPrintFileRequests.push_back( pRequest );
			return;
		}

		IMutex& rSolarMutex = Application::GetSolarMutex();
		rSolarMutex.acquire();

		if ( !Application::IsShutDown() )
		{
			com_sun_star_vcl_VCLEvent aEvent( bPrint ? SALEVENT_PRINTDOCUMENT : SALEVENT_OPENDOCUMENT, NULL, NULL, rPath );
			pSalData->mpEventQueue->postCachedEvent( &aEvent );
		}

		rSolarMutex.release();
	}
}

static void HandlePreferencesRequest()
{
	if ( !Application::IsShutDown() )
	{
		// If no Java event queue exists yet, ignore event as we are likely to
		// deadlock
		SalData *pSalData = GetSalData();
		if ( pSalData && pSalData->mpEventQueue )
		{
			IMutex& rSolarMutex = Application::GetSolarMutex();
			rSolarMutex.acquire();

			if ( !Application::IsShutDown() )
			{
				com_sun_star_vcl_VCLEvent aEvent( SALEVENT_PREFS, NULL, NULL);
				pSalData->mpEventQueue->postCachedEvent( &aEvent );
			}

			rSolarMutex.release();
		}
	}
}

static NSApplicationTerminateReply HandleTerminationRequest()
{
	NSApplicationTerminateReply nRet = NSTerminateCancel;

	if ( !Application::IsShutDown() )
	{
		// If no Java event queue exists yet, ignore event as we are likely to
		// deadlock
		SalData *pSalData = GetSalData();
		if ( pSalData && pSalData->mpEventQueue )
		{
			IMutex& rSolarMutex = Application::GetSolarMutex();
			rSolarMutex.acquire();

			if ( !Application::IsShutDown() && !pSalData->mpEventQueue->isShutdownDisabled() )
			{
				com_sun_star_vcl_VCLEvent aEvent( SALEVENT_SHUTDOWN, NULL, NULL );
				pSalData->mpEventQueue->postCachedEvent( &aEvent );
				while ( !Application::IsShutDown() && !aEvent.isShutdownCancelled() && !pSalData->mpEventQueue->isShutdownDisabled() )
					Application::Yield();
			}

			rSolarMutex.release();
		}
	}

	if ( Application::IsShutDown() )
		nRet = NSTerminateLater;

	return nRet;
}

static void HandleDidChangeScreenParametersRequest()
{
	if ( !Application::IsShutDown() )
	{
		// If no Java event queue exists yet, ignore event as we are likely to
		// deadlock
		SalData *pSalData = GetSalData();
		if ( pSalData && pSalData->mpEventQueue )
		{
			IMutex& rSolarMutex = Application::GetSolarMutex();
			rSolarMutex.acquire();

			if ( !Application::IsShutDown() )
			{
				for ( std::list< JavaSalFrame* >::const_iterator it = pSalData->maFrameList.begin(); it != pSalData->maFrameList.end(); ++it )
				{
					if ( (*it)->mbVisible )
						(*it)->SetPosSize( 0, 0, 0, 0, 0 );
				}
			}

			rSolarMutex.release();
		}
	}
}

static VCLApplicationDelegate *pSharedAppDelegate = nil;

@implementation VCLApplicationDelegate

+ (VCLApplicationDelegate *)sharedDelegate
{
	// Do not retain as invoking alloc disables autorelease
	if ( !pSharedAppDelegate )
		pSharedAppDelegate = [[VCLApplicationDelegate alloc] init];

	return pSharedAppDelegate;
}

- (void)addMenuBarItem:(NSNotification *)pNotification
{
	if ( pNotification )
	{
		NSApplication *pApp = [NSApplication sharedApplication];
		NSMenu *pObject = [pNotification object];
		if ( pApp && pObject && pObject == [pApp mainMenu] )
		{
			NSUInteger i = 0;
			NSUInteger nCount = [pObject numberOfItems];
			for ( ; i < nCount; i++ )
			{
				NSMenuItem *pItem = [pObject itemAtIndex:i];
				if ( pItem )
				{
					NSMenu *pSubmenu = [pItem submenu];
					if ( pSubmenu )
						[pSubmenu setDelegate:self];
				}
			}
		}
	}
}

- (BOOL)application:(NSApplication *)pApplication openFile:(NSString *)pFilename
{
	if ( mbInTermination || !pFilename )
		return NO;

	NSFileManager *pFileManager = [NSFileManager defaultManager];
	if ( pFileManager )
	{
		MacOSBOOL bDir = NO;
		if ( [pFileManager fileExistsAtPath:pFilename isDirectory:&bDir] && !bDir )
			HandleOpenPrintFileRequest( [pFilename UTF8String], sal_False );
	}

	return YES;
}

- (BOOL)application:(NSApplication *)pApplication printFile:(NSString *)pFilename
{
	if ( mbInTermination || !pFilename )
		return NO;

	NSFileManager *pFileManager = [NSFileManager defaultManager];
	if ( pFileManager )
	{
		MacOSBOOL bDir = NO;
		if ( [pFileManager fileExistsAtPath:pFilename isDirectory:&bDir] && !bDir )
			HandleOpenPrintFileRequest( [pFilename UTF8String], sal_True );
	}

	return YES;
}

- (void)applicationDidBecomeActive:(NSNotification *)pNotification
{
	// Fix bug 221 by explicitly reenabling all keyboards
	void *pLib = dlopen( NULL, RTLD_LAZY | RTLD_LOCAL );
	if ( pLib )
	{
		KeyScript_Type *pKeyScript = (KeyScript_Type *)dlsym( pLib, "KeyScript" );
		if ( pKeyScript )
			pKeyScript( smKeyEnableKybds );

		dlclose( pLib );
	}
}

- (void)applicationDidChangeScreenParameters:(NSNotification *)pNotification
{
	if ( mpDelegate && [mpDelegate respondsToSelector:@selector(applicationDidChangeScreenParameters:)] )
		[mpDelegate applicationDidChangeScreenParameters:pNotification];

	// Fix bug 3559 by making sure that the frame fits in the work area
	// if the screen size has changed
	HandleDidChangeScreenParametersRequest();
}

- (BOOL)applicationShouldHandleReopen:(NSApplication *)pApplication hasVisibleWindows:(BOOL)bFlag
{
	return NO;
}

- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)pApplication
{
	if ( mbInTermination || ( pApplication && [pApplication modalWindow] ) )
		return NSTerminateCancel;

	mbInTermination = YES;
	return HandleTerminationRequest();
}

- (void)applicationWillFinishLaunching:(NSNotification *)pNotification
{
	if ( mpDelegate && [mpDelegate respondsToSelector:@selector(applicationWillFinishLaunching:)] )
		[mpDelegate applicationWillFinishLaunching:pNotification];
}

- (void)cancelTermination
{
	mbInTermination = NO;

	// Dequeue any pending events
	std::list< ImplPendingOpenPrintFileRequest* > aRequests( aPendingOpenPrintFileRequests );
	aPendingOpenPrintFileRequests.clear();
	while ( aRequests.size() )
	{
		ImplPendingOpenPrintFileRequest *pRequest = aRequests.front();
		if ( pRequest )
		{
			HandleOpenPrintFileRequest( pRequest->maPath, pRequest->mbPrint );
			delete pRequest;
		}
		aRequests.pop_front();
	}
}

- (void)dealloc
{
	NSNotificationCenter *pNotificationCenter = [NSNotificationCenter defaultCenter];
	if ( pNotificationCenter )
	{
		[pNotificationCenter removeObserver:self name:NSMenuDidAddItemNotification object:nil];
		[pNotificationCenter removeObserver:self name:NSMenuDidBeginTrackingNotification object:nil];
		[pNotificationCenter removeObserver:self name:NSMenuDidEndTrackingNotification object:nil];
	}

	if ( mpDelegate )
		[mpDelegate release];

	[super dealloc];
}

- (id)init
{
	[super init];

	mpDelegate = nil;
	mbInTermination = NO;
	mbInTracking = NO;
	mbUpdateMenus = NO;

	// Set the application delegate as the delegate for the application menu so
	// that the Java menu item target and selector can be replaced with our own
	NSApplication *pApp = [NSApplication sharedApplication];
	if ( pApp )
	{
		NSMenu *pMainMenu = [pApp mainMenu];
		if ( pMainMenu )
		{
			if ( [pMainMenu numberOfItems] > 0 )
			{
				NSMenuItem *pItem = [pMainMenu itemAtIndex:0];
				if ( pItem )
				{
					NSMenu *pAppMenu = [pItem submenu];
					if ( pAppMenu )
						[pAppMenu setDelegate:self];
				}
			}
		
			NSNotificationCenter *pNotificationCenter = [NSNotificationCenter defaultCenter];
			if ( pNotificationCenter )
			{
				[pNotificationCenter addObserver:self selector:@selector(addMenuBarItem:) name:NSMenuDidAddItemNotification object:nil];
				[pNotificationCenter addObserver:self selector:@selector(trackMenuBar:) name:NSMenuDidBeginTrackingNotification object:nil];
				[pNotificationCenter addObserver:self selector:@selector(trackMenuBar:) name:NSMenuDidEndTrackingNotification object:nil];
			}
		}
	}

	return self;
}

- (void)menuNeedsUpdate:(NSMenu *)pMenu
{
	if ( !pMenu )
		return;

	NSMenu *pSupermenu = [pMenu supermenu];
	if ( !pSupermenu )
		return;

	NSApplication *pApp = [NSApplication sharedApplication];
	if ( !pApp )
		return;

	NSMenu *pMainMenu = [pApp mainMenu];
	if ( !pMainMenu || ![pMainMenu numberOfItems] || pSupermenu != pMainMenu )
		return;

	if ( mbInTracking && mbUpdateMenus )
	{
		mbUpdateMenus = NO;
		if ( !VCLInstance_updateNativeMenus() )
		{
			[pMainMenu cancelTracking];
			return;
		}
	}

	NSMenuItem *pAppItem = [pMainMenu itemAtIndex:0];
	if ( !pAppItem )
		return;

	NSMenu *pSubmenu = [pAppItem submenu];
	if ( !pSubmenu || pMenu != pSubmenu )
		return;

	int nItems = [pMenu numberOfItems];
	int i = 0;
	for ( ; i < nItems; i++ )
	{
		NSMenuItem *pItem = [pMenu itemAtIndex:i];
		if ( pItem )
		{
			NSObject *pTarget = [pItem target];
			if ( pTarget && [[pTarget className] isEqualToString:pApplicationDelegateString] )
			{
				NSString *pAction = NSStringFromSelector( [pItem action] );
				if ( pAction )
				{
					NSRange aRange = [pAction rangeOfString:@"about" options:NSCaseInsensitiveSearch];
					if ( aRange.location != NSNotFound && aRange.length > 0 )
					{
						[pItem setTarget:self];
						[pItem setAction:@selector(showAbout)];
					}
					else
					{
						aRange = [pAction rangeOfString:@"preferences" options:NSCaseInsensitiveSearch];
						if ( aRange.location != NSNotFound && aRange.length > 0 )
						{
							[pItem setTarget:self];
							[pItem setAction:@selector(showPreferences)];
						}
					}
				}
			}
		}
	}
}

- (void)setDelegate:(id)pDelegate
{
	if ( mpDelegate )
	{
    	[mpDelegate release];
    	mpDelegate = nil;
	}

	if ( pDelegate )
	{
    	mpDelegate = pDelegate;
    	[mpDelegate retain];
	}
}

- (void)showAbout
{
	if ( !mbInTermination )
		HandleAboutRequest();
}

- (void)showPreferences
{
	if ( !mbInTermination )
		HandlePreferencesRequest();
}

- (void)trackMenuBar:(NSNotification *)pNotification
{
	if ( pNotification )
	{
		NSApplication *pApp = [NSApplication sharedApplication];
		NSMenu *pObject = [pNotification object];
		if ( pApp && pObject && pObject == [pApp mainMenu] )
		{
			NSString *pName = [pNotification name];
			if ( [NSMenuDidBeginTrackingNotification isEqualToString:pName] )
			{
				mbInTracking = YES;
				mbUpdateMenus = YES;
			}
			else if ( [NSMenuDidEndTrackingNotification isEqualToString:pName] )
			{
				mbInTracking = NO;
				mbUpdateMenus = NO;
			}
		}
	}
}
- (BOOL)validateMenuItem:(NSMenuItem *)pMenuItem
{
	return !mbInTermination;
}

@end
