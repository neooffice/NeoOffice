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
 *  Patrick Luby, July 2005
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2005 Planamesa Inc.
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

#import <set>

#include <comphelper/sequenceashashmap.hxx>
#include <svtools/dynamicmenuoptions.hxx>
#include <svtools/moduleoptions.hxx>
#include <tools/link.hxx>
#include <vcl/svapp.hxx>

#define USE_APP_SHORTCUTS
#include "app.hrc"
#include "shutdownicon.hxx"

#include <premac.h>
#import <Carbon/Carbon.h>
#import <Cocoa/Cocoa.h>
#include <postmac.h>

#define WRITER_COMMAND_ID				'SDI1'
#define CALC_COMMAND_ID					'SDI2'
#define IMPRESS_COMMAND_ID				'SDI3'
#define DRAW_COMMAND_ID					'SDI4'
#define MATH_COMMAND_ID					'SDI5'
#define BASE_COMMAND_ID					'SDI8'
#define FROMTEMPLATE_COMMAND_ID			'SDI6'
#define FILEOPEN_COMMAND_ID				'SDI7'

#define WRITER_FALLBACK_DESC			"Text Document"
#define CALC_FALLBACK_DESC				"Spreadsheet"
#define IMPRESS_WIZARD_FALLBACK_DESC	"Presentation"
#define DRAW_FALLBACK_DESC				"Drawing"
#define BASE_FALLBACK_DESC				"Database"
#define MATH_FALLBACK_DESC				"Formula"

using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace ::rtl;

class ShutdownIconEvent
{
	int					mnCommand;
public:
						ShutdownIconEvent( int nCommand ) : mnCommand( nCommand ) {}
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

void ProcessShutdownIconCommand( int nCommand )
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

@interface NSObject (ShutdownIconDelegate)
- (BOOL)application:(NSApplication *)pApplication openFile:(NSString *)pFilename;
- (BOOL)application:(NSApplication *)pApplication printFile:(NSString *)pFilename;
- (void)applicationDidChangeScreenParameters:(NSNotification *)pNotification;
- (BOOL)applicationShouldHandleReopen:(NSApplication *)pApplication hasVisibleWindows:(BOOL)bFlag;
- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)pApplication;
- (void)applicationWillFinishLaunching:(NSNotification *)pNotification;
- (void)cancelTermination;
@end

/*
 * Create a class that is a facade for the application delegate set by the JVM.
 * Note that this class only implement the delegate methods in the JVM's
 * ApplicationDelegate and AWTApplicationDelegate classes.
 */
@interface ShutdownIconDelegate : NSObject
{
	id					mpDelegate;
	NSMenu*				mpDockMenu;
	BOOL				mbInTermination;
}
- (BOOL)application:(NSApplication *)pApplication openFile:(NSString *)pFilename;
- (BOOL)application:(NSApplication *)pApplication printFile:(NSString *)pFilename;
- (void)applicationDidBecomeActive:(NSNotification *)pNotification;
- (void)applicationDidChangeScreenParameters:(NSNotification *)pNotification;
- (NSMenu *)applicationDockMenu:(NSApplication *)pApplication;
- (BOOL)applicationShouldHandleReopen:(NSApplication *)pApplication hasVisibleWindows:(BOOL)bFlag;
- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)pApplication;
- (void)applicationWillFinishLaunching:(NSNotification *)pNotification;
- (void)cancelTermination;
- (void)dealloc;
- (id)init;
- (void)handleCalcCommand:(id)pObject;
- (void)handleDrawCommand:(id)pObject;
- (void)handleFileOpenCommand:(id)pObject;
- (void)handleFromTemplateCommand:(id)pObject;
- (void)handleImpressCommand:(id)pObject;
- (void)handleMathCommand:(id)pObject;
- (void)handleWriterCommand:(id)pObject;
- (void)setDelegate:(id)pDelegate;
- (BOOL)validateMenuItem:(NSMenuItem *)pMenuItem;
@end

@implementation ShutdownIconDelegate

- (BOOL)application:(NSApplication *)pApplication openFile:(NSString *)pFilename
{
	if ( !mbInTermination && mpDelegate && [mpDelegate respondsToSelector:@selector(application:openFile:)] )
		return [mpDelegate application:pApplication openFile:pFilename];
	else
		return NO;
}

- (BOOL)application:(NSApplication *)pApplication printFile:(NSString *)pFilename
{
	if ( !mbInTermination && mpDelegate && [mpDelegate respondsToSelector:@selector(application:printFile:)] )
		return [mpDelegate application:pApplication printFile:pFilename];
	else
		return NO;
}

- (NSMenu *)applicationDockMenu:(NSApplication *)pApplication
{   
    return mpDockMenu;
} 

- (void)applicationDidBecomeActive:(NSNotification *)pNotification
{
	// Fix bug 221 by explicitly reenabling all keyboards
	KeyScript( smKeyEnableKybds );
}

- (void)applicationDidChangeScreenParameters:(NSNotification *)pNotification
{
	if ( mpDelegate && [mpDelegate respondsToSelector:@selector(applicationDidChangeScreenParameters:)] )
		[mpDelegate applicationDidChangeScreenParameters:pNotification];
}

- (BOOL)applicationShouldHandleReopen:(NSApplication *)pApplication hasVisibleWindows:(BOOL)bFlag
{
	if ( !mbInTermination && mpDelegate && [mpDelegate respondsToSelector:@selector(applicationShouldHandleReopen:hasVisibleWindows:)] )
		return [mpDelegate applicationShouldHandleReopen:pApplication hasVisibleWindows:bFlag];
	else
		return NO;
}

- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)pApplication
{
	if ( mbInTermination )
		return NSTerminateCancel;

	mbInTermination = YES;

	if ( mpDelegate && [mpDelegate respondsToSelector:@selector(applicationShouldTerminate:)] )
		return [mpDelegate applicationShouldTerminate:pApplication];
	else
		return NSTerminateCancel;
}

- (void)applicationWillFinishLaunching:(NSNotification *)pNotification
{
	if ( mpDelegate && [mpDelegate respondsToSelector:@selector(applicationWillFinishLaunching:)] )
		[mpDelegate applicationWillFinishLaunching:pNotification];
}

- (void)cancelTermination
{
	mbInTermination = NO;
}

- (void)dealloc
{
	if ( mpDelegate )
		[mpDelegate release];

	if ( mpDockMenu )
		[mpDockMenu release];

	[super dealloc];
}

- (id)init
{
	[super init];

	mpDelegate = nil;
	mpDockMenu = [[NSMenu alloc] initWithTitle:@""];
	mbInTermination = NO;

	return self;
}

- (void)handleCalcCommand:(id)pObject
{
	ProcessShutdownIconCommand( CALC_COMMAND_ID );
}

- (void)handleDrawCommand:(id)pObject
{
	ProcessShutdownIconCommand( DRAW_COMMAND_ID );
}

- (void)handleFileOpenCommand:(id)pObject
{
	ProcessShutdownIconCommand( FILEOPEN_COMMAND_ID );
}

- (void)handleFromTemplateCommand:(id)pObject
{
	ProcessShutdownIconCommand( FROMTEMPLATE_COMMAND_ID );
}

- (void)handleImpressCommand:(id)pObject
{
	ProcessShutdownIconCommand( IMPRESS_COMMAND_ID );
}

- (void)handleMathCommand:(id)pObject
{
	ProcessShutdownIconCommand( MATH_COMMAND_ID );
}

- (void)handleWriterCommand:(id)pObject
{
	ProcessShutdownIconCommand( WRITER_COMMAND_ID );
}

- (void)handleBaseCommand:(id)pObject
{
	ProcessShutdownIconCommand( BASE_COMMAND_ID );
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

- (BOOL)validateMenuItem:(NSMenuItem *)pMenuItem
{
	return !mbInTermination;
}

@end

@interface CancelTermination : NSObject
+ (id)create;
- (void)cancelTermination:(id)pObject;
@end

@implementation CancelTermination

+ (id)create
{
	CancelTermination *pRet = [[CancelTermination alloc] init];
	[pRet autorelease];
	return pRet;
}

- (void)cancelTermination:(id)pObject;
{
	NSApplication *pApp = [NSApplication sharedApplication];
	if ( pApp )
	{
		NSObject *pDelegate = [pApp delegate];
		if ( pDelegate && [pDelegate respondsToSelector:@selector(cancelTermination)] )
			[pDelegate cancelTermination];
	}
}
@end

@interface QuickstartMenuItems : NSObject
{
	int					mnItems;
	int*				mpIDs;
	CFStringRef*		mpStrings;
}
+ (id)createWithItems:(int)nItems menuCommands:(int *)pIDs strings:(CFStringRef *)pStrings;
- (void)addMenuItems:(id)pObject;
- (id)initWithItems:(int)nItems menuCommands:(int *)pIDs strings:(CFStringRef *)pStrings;
@end

@implementation QuickstartMenuItems

+ (id)createWithItems:(int)nItems menuCommands:(int *)pIDs strings:(CFStringRef *)pStrings
{
	QuickstartMenuItems *pRet = [[QuickstartMenuItems alloc] initWithItems:nItems menuCommands:pIDs strings:pStrings];
	[pRet autorelease];
	return pRet;
}

- (void)addMenuItems:(id)pObject
{
	NSApplication *pApp = [NSApplication sharedApplication];
	if ( pApp )
	{
		NSMenu *pAppMenu = nil;
		NSMenu *pDockMenu = nil;

		NSMenu *pMainMenu = [pApp mainMenu];
		if ( pMainMenu && [pMainMenu numberOfItems] > 0 )
		{
			NSMenuItem *pItem = [pMainMenu itemAtIndex:0];
			if ( pItem )
				pAppMenu = [pItem submenu];
		}

		NSObject *pDelegate = [pApp delegate];
		if ( pDelegate && [pDelegate respondsToSelector:@selector(applicationDockMenu:)] )
			pDockMenu = [pDelegate applicationDockMenu:pApp];

		if ( !pDockMenu )
		{
			// Do not retain as invoking alloc disables autorelease
			ShutdownIconDelegate *pNewDelegate = [[ShutdownIconDelegate alloc] init];
			if ( pDelegate )
				[pNewDelegate setDelegate:pDelegate];
			// NSApplication does not retain delegates so don't release it
			[pApp setDelegate:pNewDelegate];
			pDelegate = pNewDelegate;
			pDockMenu = [pNewDelegate applicationDockMenu:pApp];
		}

		if ( pAppMenu && pDockMenu )
		{
			// Insert a separator menu item (only in the application menu)
			[pAppMenu insertItem:[NSMenuItem separatorItem] atIndex:2];

			// Work the list of menu items is reverse order
			SEL aSelector;
			int i;
			for ( i = mnItems - 1; i >= 0; i-- )
			{
				switch ( mpIDs[ i ] )
				{
					case BASE_COMMAND_ID:
						aSelector = @selector(handleBaseCommand:);
						break;
					case CALC_COMMAND_ID:
						aSelector = @selector(handleCalcCommand:);
						break;
					case DRAW_COMMAND_ID:
						aSelector = @selector(handleDrawCommand:);
						break;
					case FILEOPEN_COMMAND_ID:
						aSelector = @selector(handleFileOpenCommand:);
						break;
					case FROMTEMPLATE_COMMAND_ID:
						aSelector = @selector(handleFromTemplateCommand:);
						break;
					case IMPRESS_COMMAND_ID:
						aSelector = @selector(handleImpressCommand:);
						break;
					case MATH_COMMAND_ID:
						aSelector = @selector(handleMathCommand:);
						break;
					case WRITER_COMMAND_ID:
						aSelector = @selector(handleWriterCommand:);
						break;
					default:
						aSelector = nil;
						break;
				}

				// Insert and release string
				if ( mpStrings[ i ] )
				{
					NSMenuItem *pAppMenuItem = [pAppMenu insertItemWithTitle:(NSString *)mpStrings[ i ] action:aSelector keyEquivalent:@"" atIndex:2];
					if ( pAppMenuItem )
						[pAppMenuItem setTarget:pDelegate];
					NSMenuItem *pDockMenuItem = [pDockMenu insertItemWithTitle:(NSString *)mpStrings[ i ] action:aSelector keyEquivalent:@"" atIndex:0];
					if ( pDockMenuItem )
						[pDockMenuItem setTarget:pDelegate];
				}
			}

			mpIDs = nil;
			mpStrings = nil;
		}
	}
}

- (id)initWithItems:(int)nItems menuCommands:(int *)pIDs strings:(CFStringRef *)pStrings;
{
	[super init];

	mnItems = nItems;
	mpIDs = pIDs;
	mpStrings = pStrings;

	return self;
}

@end

// Note: this must not be static as the symbol will be loaded by the vcl module
extern "C" __attribute__((visibility("default"))) void NativeShutdownCancelledHandler()
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	CancelTermination *pCancelTermination = [CancelTermination create];
	NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
	[pCancelTermination performSelectorOnMainThread:@selector(cancelTermination:) withObject:pCancelTermination waitUntilDone:YES modes:pModes];

	[pPool release];
}

extern "C" void java_init_systray()
{
	ShutdownIcon *pShutdownIcon = ShutdownIcon::getInstance();
	if ( !pShutdownIcon )
		return;

	// Collect the URLs of the entries in the File/New menu
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

	// Describe the menu entries for launching the applications
	struct MenuEntryDescriptor
	{
		SvtModuleOptions::EModule	eModuleIdentifier;
		int							nMenuItemID;
		const char*					pAsciiURLDescription;
		const char*					pFallbackDescription;
	} aMenuItems[] =
	{
		{ SvtModuleOptions::E_SWRITER, WRITER_COMMAND_ID, WRITER_URL, WRITER_FALLBACK_DESC },
		{ SvtModuleOptions::E_SCALC, CALC_COMMAND_ID, CALC_URL, CALC_FALLBACK_DESC },
		{ SvtModuleOptions::E_SIMPRESS, IMPRESS_COMMAND_ID, IMPRESS_WIZARD_URL, IMPRESS_WIZARD_FALLBACK_DESC },
		{ SvtModuleOptions::E_SDRAW, DRAW_COMMAND_ID, DRAW_URL, DRAW_FALLBACK_DESC },
		{ SvtModuleOptions::E_SDATABASE, BASE_COMMAND_ID, BASE_URL, BASE_FALLBACK_DESC },
		{ SvtModuleOptions::E_SMATH, MATH_COMMAND_ID, MATH_URL, MATH_FALLBACK_DESC }
	};

	// Disable shutdown
	pShutdownIcon->SetVeto( true );
	pShutdownIcon->addTerminateListener();

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	// insert the menu entries for launching the applications
	int nItems = 0;
	int aIDs[ 8 ];
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
		aDesc = XubString( pShutdownIcon->GetUrlDescription( sURL ) );
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
	aDesc = XubString( pShutdownIcon->GetResString( STR_QUICKSTART_FROMTEMPLATE ) );
	aDesc.EraseAllChars( '~' );
	aStrings[ nItems++ ] = CFStringCreateWithCharacters( NULL, aDesc.GetBuffer(), aDesc.Len() );
	aIDs[ nItems ] = FILEOPEN_COMMAND_ID;
	aDesc = XubString( pShutdownIcon->GetResString( STR_QUICKSTART_FILEOPEN ) );
	aDesc.EraseAllChars( '~' );
	aStrings[ nItems++ ] = CFStringCreateWithCharacters( NULL, aDesc.GetBuffer(), aDesc.Len() );

	ULONG nCount = Application::ReleaseSolarMutex();

	QuickstartMenuItems *pItems = [QuickstartMenuItems createWithItems:nItems menuCommands:aIDs strings:aStrings];
	NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
	[pItems performSelectorOnMainThread:@selector(addMenuItems:) withObject:pItems waitUntilDone:YES modes:pModes];

	Application::AcquireSolarMutex( nCount );

	for ( int i = 0; i < nItems; i++ )
		CFRelease( aStrings[ i ] );

	[pPool release];
}

extern "C" void java_destroy_systray()
{
}
