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

#include <list>

#include <premac.h>
#import <objc/objc-runtime.h>
#import <apple_remote/RemoteControl.h>
#include <postmac.h>

#include <osl/objcutils.h>
#include <rtl/ustring.hxx>
#include <vcl/svapp.hxx>

#include "svids.hrc"
#include "java/saldata.hxx"
#include "java/salframe.h"

#include "VCLApplicationDelegate_cocoa.h"
#include "VCLEventQueue_cocoa.h"
#include "../app/salinst_cocoa.h"

// Comment out the following line to disable native resume support
#define USE_NATIVE_RESUME

struct ImplPendingOpenPrintFileRequest
{
	OString				maPath;
	sal_Bool			mbPrint;

						ImplPendingOpenPrintFileRequest( const OString &rPath, sal_Bool bPrint ) : maPath( rPath ), mbPrint( bPrint ) {}
						~ImplPendingOpenPrintFileRequest() {};
};

static std::list< ImplPendingOpenPrintFileRequest* > aPendingOpenPrintFileRequests;
static NSString *pSFXDocument = @"SFXDocument";
static NSString *pSFXDocumentRevision = @"SFXDocumentRevision";

using namespace vcl;

#ifdef USE_NATIVE_RESUME

static BOOL IsResumeEnabled()
{
	BOOL bRet = YES;

	CFPropertyListRef aPref = CFPreferencesCopyAppValue( CFSTR( "DisableResume" ), kCFPreferencesCurrentApplication );
	if ( aPref )
	{
		if ( CFGetTypeID( aPref ) == CFBooleanGetTypeID() && (CFBooleanRef)aPref == kCFBooleanTrue )
			bRet = NO;
		CFRelease( aPref );
	}

	return bRet;
}

#endif	// USE_NATIVE_RESUME

static void HandleAboutRequest()
{
	// If no application mutex exists yet, ignore event as we are likely to
	// crash
	if ( ImplApplicationIsRunning() )
	{
		JavaSalEvent *pEvent = new JavaSalEvent( SALEVENT_ABOUT, NULL, NULL);
		JavaSalEventQueue::postCachedEvent( pEvent );
		pEvent->release();
	}
}

static void HandleOpenPrintFileRequest( const OString &rPath, sal_Bool bPrint )
{
	if ( rPath.getLength() && !Application::IsShutDown() )
	{
		// If no application mutex exists yet, ignore event as we are likely to
		// crash
		if ( ImplApplicationIsRunning() )
		{
			JavaSalEvent *pEvent = new JavaSalEvent( bPrint ? SALEVENT_PRINTDOCUMENT : SALEVENT_OPENDOCUMENT, NULL, NULL, rPath );
			JavaSalEventQueue::postCachedEvent( pEvent );
			pEvent->release();
		}
		else
		{
			ImplPendingOpenPrintFileRequest *pRequest = new ImplPendingOpenPrintFileRequest( rPath, bPrint );
			if ( pRequest )
				aPendingOpenPrintFileRequests.push_back( pRequest );
		}
	}
}

static void HandlePreferencesRequest()
{
	// If no application mutex exists yet, ignore event as we are likely to
	// crash
	if ( ImplApplicationIsRunning() )
	{
		JavaSalEvent *pEvent = new JavaSalEvent( SALEVENT_PREFS, NULL, NULL);
		JavaSalEventQueue::postCachedEvent( pEvent );
		pEvent->release();
	}
}

static NSApplicationTerminateReply HandleTerminationRequest()
{
	NSApplicationTerminateReply nRet = NSTerminateCancel;

	// If no application mutex exists yet, ignore event as we are likely to
	// crash
	if ( ImplApplicationIsRunning() )
	{
		// Try to fix deadlocks in the framework module by not acquiring the
		// application mutex on the main thread
		JavaSalEvent *pEvent = new JavaSalEvent( SALEVENT_SHUTDOWN, NULL, NULL );
		JavaSalEventQueue::postCachedEvent( pEvent );
		while ( ImplApplicationIsRunning() && !pEvent->isShutdownCancelled() && !JavaSalEventQueue::isShutdownDisabled() )
		{
			// Prioritize execution of any run on main thread calls that may
			// be blocking shutdown
			CFRunLoopRunInMode( JAVA_AWT_RUNLOOPMODE, 0, false );
			NSApplication_dispatchPendingEvents( NO, YES );
		}
		pEvent->release();
	}

	return nRet;
}

static void HandleDidChangeScreenParametersRequest()
{
	// If no application mutex exists yet, ignore event as we are likely to
	// crash
	if ( ImplApplicationIsRunning() )
	{
		JavaSalEvent *pEvent = new JavaSalEvent( SALEVENT_SCREENPARAMSCHANGED, NULL, NULL);
		JavaSalEventQueue::postCachedEvent( pEvent );
		pEvent->release();
	}
}

static VCLApplicationDelegate *pSharedAppDelegate = nil;

@interface NSObject (SFXDocument)
+ (BOOL)isInVersionBrowser;
@end

@interface VCLDocument : NSDocument
+ (BOOL)autosavesInPlace;
- (BOOL)hasUnautosavedChanges;
- (BOOL)isDocumentEdited;
- (void)makeWindowControllers;
- (BOOL)readFromURL:(NSURL *)pURL ofType:(NSString *)pTypeName error:(NSError **)ppError;
- (void)restoreStateWithCoder:(NSCoder *)pCoder;
@end

@implementation VCLDocument

+ (BOOL)autosavesInPlace
{
	return YES;
}

- (BOOL)hasUnautosavedChanges
{
	// Don't allow NSDocument to do autosaving
	return NO;
}

- (BOOL)isDocumentEdited
{
	return NO;
}

- (void)makeWindowControllers
{
	// Close document so that there are no dangling managed object references.
	// Note that we have to queue the closing as invoking close now will not
	// actually close the document.
	[self performSelector:@selector(close) withObject:nil afterDelay:0];
}

- (BOOL)readFromURL:(NSURL *)pURL ofType:(NSString *)pTypeName error:(NSError **)ppError
{
	(void)pURL;
	(void)pTypeName;

	if ( ppError )
		*ppError = nil;

	return YES;
}

- (void)restoreStateWithCoder:(NSCoder *)pCoder
{
	(void)pCoder;
	// Don't allow NSDocument to do the restoration
}

@end

@interface NSDocumentController (VCLDocumentController)
- (void)_docController:(NSDocumentController *)pDocController shouldTerminate:(BOOL)bShouldTerminate;
@end

@interface VCLDocumentController : NSDocumentController
{
	NSOpenPanel*			mpOpenPanel;
}
- (void)_closeAllDocumentsWithDelegate:(id)pDelegate shouldTerminateSelector:(SEL)aShouldTerminateSelector;
- (void)beginOpenPanel:(NSOpenPanel *)pOpenPanel forTypes:(NSArray *)pTypes completionHandler:(void (^)(NSInteger result))aCompletionHandler;
- (Class)documentClassForType:(NSString *)pDocumentTypeName;
- (id)init;
- (id)makeDocumentWithContentsOfURL:(NSURL *)pAbsoluteURL ofType:(NSString *)pTypeName error:(NSError **)ppError;
- (void)newDocument:(id)pSender;
- (NSOpenPanel *)openPanel;
- (void)reopenDocumentForURL:(NSURL *)pURL withContentsOfURL:(NSURL *)pContentsURL display:(BOOL)bDisplayDocument completionHandler:(void (^)(NSDocument *pDocument, BOOL bDocumentWasAlreadyOpen, NSError *error))aCompletionHandler;
@end

@implementation VCLDocumentController

- (void)_closeAllDocumentsWithDelegate:(id)pDelegate shouldTerminateSelector:(SEL)aShouldTerminateSelector
{
	if ( pDelegate && [pDelegate respondsToSelector:aShouldTerminateSelector] && sel_isEqual( aShouldTerminateSelector, @selector(_docController:shouldTerminate:) ) )
		[pDelegate _docController:self shouldTerminate:YES];
}

- (void)beginOpenPanel:(NSOpenPanel *)pOpenPanel forTypes:(NSArray *)pTypes completionHandler:(void (^)(NSInteger result))aCompletionHandler
{
	(void)pTypes;

	mpOpenPanel = pOpenPanel;

	if ( aCompletionHandler )
		aCompletionHandler( NSModalResponseCancel );
}

- (Class)documentClassForType:(NSString *)pDocumentTypeName
{
	(void)pDocumentTypeName;

	// Always return our custom class for rendering in the version browser
	return NSClassFromString( pSFXDocumentRevision );
}

- (id)init
{
	[super init];

	mpOpenPanel = nil;

	return self;
}

- (id)makeDocumentWithContentsOfURL:(NSURL *)pAbsoluteURL ofType:(NSString *)pTypeName error:(NSError **)ppError
{
	// Handle call normally if we are in the version browser
	Class aSFXDocumentClass = NSClassFromString( pSFXDocument );
	if ( aSFXDocumentClass && class_getClassMethod( aSFXDocumentClass, @selector(isInVersionBrowser) ) && [aSFXDocumentClass isInVersionBrowser] )
		return [super makeDocumentWithContentsOfURL:pAbsoluteURL ofType:pTypeName error:ppError];

	if ( ppError )
		*ppError = nil;

	Application_cacheSecurityScopedURL( pAbsoluteURL );

#ifdef USE_NATIVE_RESUME
	if ( pSharedAppDelegate && pAbsoluteURL && [pAbsoluteURL isFileURL] )
	{
		NSApplication *pApp = [NSApplication sharedApplication];
		NSString *pPath = [pAbsoluteURL path];
		if ( pApp && pPath && IsResumeEnabled() )
			[pSharedAppDelegate application:pApp openFile:pPath];
	}
#endif	// USE_NATIVE_RESUME

	VCLDocument *pDoc = [[VCLDocument alloc] init];
	[pDoc autorelease];
	return pDoc;
}

- (void)newDocument:(id)pSender
{
	(void)pSender;

	NSApplication *pApp = [NSApplication sharedApplication];
	if ( pApp )
	{
		NSMenu *pMainMenu = [pApp mainMenu];
		if ( pMainMenu )
		{
			NSEvent *pEvent = [NSEvent keyEventWithType:NSEventTypeKeyDown location:NSMakePoint( 0, 0 ) modifierFlags:NSEventModifierFlagCommand timestamp:JavaSalEventQueue::getLastNativeEventTime() windowNumber:0 context:nil characters:@"n" charactersIgnoringModifiers:@"n" isARepeat:NO keyCode:0];
			if ( pEvent )
				[pMainMenu performKeyEquivalent:pEvent];
		}
	}
}

- (NSOpenPanel *)openPanel
{
	mpOpenPanel = nil;

	[self beginOpenPanelWithCompletionHandler:^(NSArray *pResult) {
		(void)pResult;
	}];

	return mpOpenPanel;
}

- (void)reopenDocumentForURL:(NSURL *)pURL withContentsOfURL:(NSURL *)pContentsURL display:(BOOL)bDisplayDocument completionHandler:(void (^)(NSDocument *pDocument, BOOL bDocumentWasAlreadyOpen, NSError *error))aCompletionHandler
{
	(void)pURL;
	(void)bDisplayDocument;

	VCLDocument *pDoc = nil;
	if ( pContentsURL )
	{
		Application_cacheSecurityScopedURL( pContentsURL );

		pContentsURL = [pContentsURL filePathURL];
		if ( pContentsURL )
		{
			NSString *pPath = [pContentsURL path];
			if ( pPath && [pPath length] )
			{
#ifdef USE_NATIVE_RESUME
				if ( IsResumeEnabled() )
					HandleOpenPrintFileRequest( [pPath UTF8String], sal_False );
#endif	// USE_NATIVE_RESUME

				pDoc = [[VCLDocument alloc] init];
				[pDoc autorelease];
			}
		}
	}

	if ( aCompletionHandler )
		aCompletionHandler( pDoc, NO, nil );
}

@end

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
	(void)pApplication;

	if ( pFilename )
		Application_cacheSecurityScopedURL( [NSURL fileURLWithPath:pFilename] );

	if ( mbInTermination || !pFilename )
		return NO;

	NSFileManager *pFileManager = [NSFileManager defaultManager];
	if ( pFileManager )
	{
		BOOL bDir = NO;
		if ( [pFileManager fileExistsAtPath:pFilename isDirectory:&bDir] && !bDir )
			HandleOpenPrintFileRequest( [pFilename UTF8String], sal_False );
	}

	return YES;
}

- (BOOL)application:(NSApplication *)pApplication printFile:(NSString *)pFilename
{
	(void)pApplication;

	if ( pFilename )
		Application_cacheSecurityScopedURL( [NSURL fileURLWithPath:pFilename] );

	if ( mbInTermination || !pFilename )
		return NO;

	NSFileManager *pFileManager = [NSFileManager defaultManager];
	if ( pFileManager )
	{
		BOOL bDir = NO;
		if ( [pFileManager fileExistsAtPath:pFilename isDirectory:&bDir] && !bDir )
			HandleOpenPrintFileRequest( [pFilename UTF8String], sal_True );
	}

	return YES;
}

- (void)applicationDidBecomeActive:(NSNotification *)pNotification
{
	(void)pNotification;

	// There seems to be a lag on macOS 10.15 before the effective
	// appearance changes when the application is not active so reset the
	// appearance when the application becomes active
	VCLUpdateSystemAppearance_handleAppearanceChange();
}

- (void)applicationDidChangeScreenParameters:(NSNotification *)pNotification
{
	if ( mpDelegate && [mpDelegate respondsToSelector:@selector(applicationDidChangeScreenParameters:)] )
		[mpDelegate applicationDidChangeScreenParameters:pNotification];

	// Fix bug 3559 by making sure that the frame fits in the work area
	// if the screen size has changed
	HandleDidChangeScreenParametersRequest();
}

- (NSMenu *)applicationDockMenu:(NSApplication *)pApplication
{
	(void)pApplication;

	return mpDockMenu;
}

- (BOOL)applicationShouldHandleReopen:(NSApplication *)pApplication hasVisibleWindows:(BOOL)bFlag
{
	(void)pApplication;

	// Fix bug reported in the following NeoOffice forum topic by
	// returning true if there is visible windows:
	// http://trinity.neooffice.org/modules.php?name=Forums&file=viewtopic&t=8478
	return bFlag;
}

- (BOOL)applicationShouldOpenUntitledFile:(NSApplication *)pSender
{
	(void)pSender;

	return NO;
}

- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)pApplication
{
	if ( mbInTermination || ( pApplication && [pApplication modalWindow] ) )
		return NSTerminateCancel;

	mbInTermination = YES;
	return HandleTerminationRequest();
}

- (void)applicationWillBecomeActive:(NSNotification *)pNotification
{
	(void)pNotification;

	if ( mpAppleRemoteMainController )
	{
		RemoteControl *pRemoteControl = [mpAppleRemoteMainController remoteControl];
		if ( pRemoteControl )
			[pRemoteControl startListening:self];
	}
}

- (void)applicationWillFinishLaunching:(NSNotification *)pNotification
{
	// Make our NSDocumentController subclass the shared controller by creating
	// an instance of our subclass before AppKit does
	[[VCLDocumentController alloc] init];

	if ( mpDelegate && [mpDelegate respondsToSelector:@selector(applicationWillFinishLaunching:)] )
		[mpDelegate applicationWillFinishLaunching:pNotification];
}

- (void)applicationWillResignActive:(NSNotification *)pNotification
{
	(void)pNotification;

	if ( mpAppleRemoteMainController )
	{
		RemoteControl *pRemoteControl = [mpAppleRemoteMainController remoteControl];
		if ( pRemoteControl )
			[pRemoteControl stopListening:self];
	}
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

	if ( mpDockMenu )
		[mpDockMenu release];

	if ( mpAppleRemoteMainController )
		[mpAppleRemoteMainController release];

	[super dealloc];
}

- (id)init
{
	[super init];

	mbAppMenuInitialized = NO;
	mbAwaitingTracking = NO;
	mbCancelTracking = NO;
	mpDelegate = nil;
	mpDockMenu = [[NSMenu alloc] initWithTitle:@""];
	mbInPerformKeyEquivalent = NO;
	mbInTermination = NO;
	mbInTracking = NO;
	mpAppleRemoteMainController = [[AppleRemoteMainController alloc] init];

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

	// Set the application delegate as the delegate for the dock menu
	if ( mpDockMenu )
		[mpDockMenu setDelegate:self];

	return self;
}

- (BOOL)isInPerformKeyEquivalent
{
	return mbInPerformKeyEquivalent;
}

- (BOOL)isInTracking
{
	// Attempt to fix Mac App Store crash when tracking menubar by counting
	// "awaiting tracking" as "in tracking"
	return ( mbAwaitingTracking || mbInTracking );
}

- (void)menuNeedsUpdate:(NSMenu *)pMenu
{
	if ( !mbAppMenuInitialized )
	{
		NSApplication *pApp = [NSApplication sharedApplication];
		if ( pApp )
		{
			NSMenu *pMainMenu = [pApp mainMenu];
			if ( pMainMenu )
			{
				NSMenuItem *pItem = [pMainMenu itemAtIndex:0];
				if ( pItem )
				{
					NSMenu *pAppMenu = [pItem submenu];
					if ( pAppMenu )
					{
						if ( ImplApplicationIsRunning() )
						{
								ACQUIRE_SOLARMUTEX
								mbAppMenuInitialized = YES;

								NSString *pAbout = nil;
								NSString *pPreferences = nil;
								NSString *pServices = nil;
								NSString *pHide = nil;
								NSString *pHideOthers = nil;
								NSString *pShowAll = nil;
								NSString *pQuit = nil;

								ResMgr *pResMgr = ImplGetResMgr();
								if ( pResMgr )
								{
									OUString aAbout( ResId( SV_STDTEXT_ABOUT, *pResMgr ) );
									if ( aAbout.getLength() )
										pAbout = [NSString stringWithCharacters:aAbout.getStr() length:aAbout.getLength()];

									OUString aPreferences( ResId( SV_STDTEXT_PREFERENCES, *pResMgr ) );
									if ( aPreferences.getLength() )
										pPreferences = [NSString stringWithCharacters:aPreferences.getStr() length:aPreferences.getLength()];

									OUString aServices( ResId( SV_MENU_MAC_SERVICES, *pResMgr ) );
									if ( aServices.getLength() )
										pServices = [NSString stringWithCharacters:aServices.getStr() length:aServices.getLength()];

									OUString aHide( ResId( SV_MENU_MAC_HIDEAPP, *pResMgr ) );
									if ( aHide.getLength() )
										pHide = [NSString stringWithCharacters:aHide.getStr() length:aHide.getLength()];

									OUString aHideOthers( ResId( SV_MENU_MAC_HIDEALL, *pResMgr ) );
									if ( aHideOthers.getLength() )
										pHideOthers = [NSString stringWithCharacters:aHideOthers.getStr() length:aHideOthers.getLength()];

									OUString aShowAll( ResId( SV_MENU_MAC_SHOWALL, *pResMgr ) );
									if ( aShowAll.getLength() )
										pShowAll = [NSString stringWithCharacters:aShowAll.getStr() length:aShowAll.getLength()];

									OUString aQuit( ResId( SV_MENU_MAC_QUITAPP, *pResMgr ) );
									if ( aQuit.getLength() )
										pQuit = [NSString stringWithCharacters:aQuit.getStr() length:aQuit.getLength()];
								}

								NSUInteger nItems = [pAppMenu numberOfItems];
								NSUInteger i = 0;
								for ( ; i < nItems; i++ )
								{
									NSMenuItem *pItem = [pAppMenu itemAtIndex:i];
									if ( pItem )
									{
										NSString *pTitle = [pItem title];
										if ( pTitle )
										{
											if ( pAbout && [pTitle isEqualToString:@"About"] )
											{
												[pItem setTarget:self];
												[pItem setAction:@selector(showAbout)];
												[pItem setTitle:pAbout];
											}
											else if ( pPreferences && [pTitle isEqualToString:@"Preferences…"] )
											{
												[pItem setTarget:self];
												[pItem setAction:@selector(showPreferences)];
												[pItem setTitle:pPreferences];
											}
											else if ( pServices && [pTitle isEqualToString:@"Services"] )
											{
												[pItem setTitle:pServices];
											}
											else if ( pHide && [pTitle isEqualToString:@"Hide"] )
											{
												[pItem setTitle:pHide];
											}
											else if ( pHideOthers && [pTitle isEqualToString:@"Hide Others"] )
											{
												[pItem setTitle:pHideOthers];
											}
											else if ( pShowAll && [pTitle isEqualToString:@"Show All"] )
											{
												[pItem setTitle:pShowAll];
											}
											else if ( pQuit && [pTitle isEqualToString:@"Quit"] )
											{
												[pItem setTitle:pQuit];
											}
										}
									}
								}
								RELEASE_SOLARMUTEX
						}
					}
				}
			}
		}
	}

	// Attempt to fix Mac App Store crash by ensuring that the menu is not
	// released by the VCLInstance_updateNativeMenus() function
	if ( pMenu )
		[pMenu retain];

	// Fix failure to display menubar submenus on macOS 12 after start of
	// tracking until mouse is moved by delaying updating of menus until the
	// first call to menuNeedsUpdate: after tracking has started
	if ( mbAwaitingTracking && !mbInTracking && !mbCancelTracking )
	{
		// Attempt to fix Mac App Store crash by immediately changing status
		// to in tracking so that out code will not change the native menubar's
		// menu items while updating the menus
		mbInTracking = YES;
		mbAwaitingTracking = NO;

		if ( VCLInstance_updateNativeMenus() )
		{
			// Fix bug reported in the following NeoOffice forum
			// topic by forcing any pending menu changes to be done
			// before any menus are displayed:
			// http://trinity.neooffice.org/modules.php?name=Forums&file=viewtopic&t=8532
			[VCLMainMenuDidEndTracking mainMenuDidEndTracking:YES];
		}
		else
		{
			mbInTracking = NO;
			mbCancelTracking = YES;
		}
	}

	if ( pMenu )
	{
		if ( !mbInTracking || mbCancelTracking )
			[pMenu cancelTracking];
		[pMenu release];
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

- (void)setInPerformKeyEquivalent:(BOOL)bInPerformKeyEquivalent
{
	mbInPerformKeyEquivalent = bInPerformKeyEquivalent;
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
		if ( pApp )
		{
			NSMenu *pMainMenu = [pApp mainMenu];
			if ( pObject && pObject == pMainMenu )
			{
				NSString *pName = [pNotification name];
				if ( [NSMenuDidBeginTrackingNotification isEqualToString:pName] )
				{
					mbAwaitingTracking = YES;
					mbCancelTracking = NO;
					mbInTracking = NO;
				}
				else if ( [NSMenuDidEndTrackingNotification isEqualToString:pName] )
				{
					mbAwaitingTracking = NO;
					mbCancelTracking = YES;
					mbInTracking = NO;
				}
			}
		}
	}
}

- (BOOL)validateMenuItem:(NSMenuItem *)pMenuItem
{
	return ( !mbInTermination && pMenuItem );
}

@end
