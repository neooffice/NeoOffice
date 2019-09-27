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
#include <sfx2/app.hxx>
#include <sfx2/sfxresid.hxx>
#include <tools/link.hxx>
#include <tools/rcid.h>
#include <unotools/dynamicmenuoptions.hxx>
#include <unotools/moduleoptions.hxx>
#include <vcl/svapp.hxx>

#define USE_APP_SHORTCUTS
#include "app.hrc"
#include "../dialog/dialog.hrc"
#include "shutdowniconjava.hrc"
#include "shutdownicon.hxx"

#include <premac.h>
#import <Cocoa/Cocoa.h>
#include <postmac.h>

#include "../view/topfrm_cocoa.hxx"

// LibreOffice headers are found relative to LibreOffice include directory
#import "../desktop/source/app/desktop.hrc"
#import "../vcl/inc/svids.hrc"

#define DEFAULT_URL						"_default"

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

#define DEFAULT_LAUNCH_OPTIONS_KEY		CFSTR( "DefaultLaunchOptions" )

typedef void VCLOpenPrintFileHandler_Type( const char *pPath, sal_Bool bPrint );
typedef void VCLRequestShutdownHandler_Type();

static bool bIsRunningHighSierraOrLowerInitizalized  = false;
static bool bIsRunningHighSierraOrLower = false;

static const NSString *kMenuItemPrefNameKey = @"MenuItemPrefName";
static const NSString *kMenuItemPrefBooleanValueKey = @"MenuItemPrefBooleanValue";
static const NSString *kMenuItemPrefStringValueKey = @"MenuItemPrefStringValue";
static const NSString *kMenuItemValueIsDefaultForPrefKey = @"MenuItemValueIsDefaultForPref";
static const NSString *kMenuItemForceDefaultIfUnsetPrefKey = @"MenuItemForceDefaultIfUnsetPref";
static ResMgr *pDktResMgr = NULL;
static ResMgr *pVclResMgr = NULL;
static FSEventStreamRef aFSEventStream = NULL;
static NSString *pRestartMessageText = nil;
static CFStringRef aExecutablePath = NULL;

using namespace com::sun::star::beans;
using namespace com::sun::star::uno;

static bool IsRunningHighSierraOrLower()
{
	if ( !bIsRunningHighSierraOrLowerInitizalized )
	{
		NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

		NSProcessInfo *pProcessInfo = [NSProcessInfo processInfo];
		if ( pProcessInfo )
		{
			NSOperatingSystemVersion aVersion = pProcessInfo.operatingSystemVersion;
			if ( aVersion.majorVersion <= 10 && aVersion.minorVersion <= 13 )
				bIsRunningHighSierraOrLower = true;
		}

		bIsRunningHighSierraOrLowerInitizalized = true;

		[pPool release];
	}

	return bIsRunningHighSierraOrLower;
}

static OUString GetDktResString( int nId )
{
	if ( !pDktResMgr )
	{
		pDktResMgr = ResMgr::CreateResMgr( "dkt" );
		if ( !pDktResMgr )
			return "";
	}

	ResId aResId( nId, *pDktResMgr );
	aResId.SetRT( RSC_STRING );
	if ( !pDktResMgr->IsAvailable( aResId ) )
		return "";
 
	return OUString( ResId( nId, *pDktResMgr ) );
}

static OUString GetVclResString( int nId )
{
	if ( !pVclResMgr )
	{
		pVclResMgr = ResMgr::CreateResMgr( "vcl" );
		if ( !pVclResMgr )
			return "";
	}

	ResId aResId( nId, *pVclResMgr );
	aResId.SetRT( RSC_STRING );
	if ( !pVclResMgr->IsAvailable( aResId ) )
		return "";
 
	return OUString( ResId( nId, *pVclResMgr ) );
}

static void ExecutableFSEventStreamCallback( ConstFSEventStreamRef aStreamRef, void *pClientCallBackInfo, size_t nNumEvents, void *pEventPaths, const FSEventStreamEventFlags *pEventFlags, const FSEventStreamEventId *pEventIds )
{
	(void)aStreamRef;
	(void)pClientCallBackInfo;
	(void)pEventIds;

	if ( !aExecutablePath || !pEventPaths || CFGetTypeID( pEventPaths ) != CFArrayGetTypeID() )
		return;

	for ( size_t i = 0; i < nNumEvents; i++ )
	{
		CFStringRef aPath = (CFStringRef)CFArrayGetValueAtIndex( (CFArrayRef)pEventPaths, i );
		if ( pEventFlags && pEventFlags[ i ] == kFSEventStreamEventFlagRootChanged && aPath && CFGetTypeID( aPath ) == CFStringGetTypeID() && CFStringCompare( aExecutablePath, aPath, kCFCompareCaseInsensitive ) == kCFCompareEqualTo )
		{
			NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

			NSAlert *pRestartAlert = nil;
			if ( pRestartMessageText )
			{
				pRestartAlert = [[NSAlert alloc] init];
				if ( pRestartAlert )
					pRestartAlert.messageText = pRestartMessageText;
			}

			if ( pRestartAlert )
				[pRestartAlert runModal];

			NSApplication *pApp = [NSApplication sharedApplication];
			if ( pApp )
				[pApp terminate:pApp];

			[pPool release];

			break;
		}
	}
}

@interface NSObject (ShutdownIconDelegate)
- (BOOL)application:(NSApplication *)pApplication openFile:(NSString *)pFilename;
- (BOOL)application:(NSApplication *)pApplication printFile:(NSString *)pFilename;
- (void)applicationDidBecomeActive:(NSNotification *)pNotification;
- (void)applicationDidChangeScreenParameters:(NSNotification *)pNotification;
- (NSMenu *)applicationDockMenu:(NSApplication *)pApplication;
- (BOOL)applicationShouldHandleReopen:(NSApplication *)pApplication hasVisibleWindows:(BOOL)bFlag;
- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)pApplication;
- (void)applicationWillBecomeActive:(NSNotification *)pNotification;
- (void)applicationWillFinishLaunching:(NSNotification *)pNotification;
- (void)applicationWillResignActive:(NSNotification *)pNotification;
@end

/*
 * Create a class that is a facade for the application delegate set by the JVM.
 * Note that this class only implement the delegate methods in the JVM's
 * ApplicationDelegate and AWTApplicationDelegate classes.
 */
@interface ShutdownIconDelegate : NSObject <NSApplicationDelegate, NSMenuDelegate>
{
	id					mpDelegate;
}
- (BOOL)application:(NSApplication *)pApplication openFile:(NSString *)pFilename;
- (BOOL)application:(NSApplication *)pApplication printFile:(NSString *)pFilename;
- (void)applicationDidBecomeActive:(NSNotification *)pNotification;
- (void)applicationDidChangeScreenParameters:(NSNotification *)pNotification;
- (NSMenu *)applicationDockMenu:(NSApplication *)pApplication;
- (BOOL)applicationShouldHandleReopen:(NSApplication *)pApplication hasVisibleWindows:(BOOL)bFlag;
- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)pApplication;
- (void)applicationWillBecomeActive:(NSNotification *)pNotification;
- (void)applicationWillFinishLaunching:(NSNotification *)pNotification;
- (void)applicationWillResignActive:(NSNotification *)pNotification;
- (void)dealloc;
- (id)init;
- (void)handleCalcCommand:(id)pObject;
- (void)handleDrawCommand:(id)pObject;
- (void)handleFileOpenCommand:(id)pObject;
- (void)handleFromTemplateCommand:(id)pObject;
- (void)handleImpressCommand:(id)pObject;
- (void)handleMathCommand:(id)pObject;
- (void)handlePreferenceChangeCommand:(id)pObject;
- (void)handleWriterCommand:(id)pObject;
- (void)setDelegate:(id)pDelegate;
- (void)menuNeedsUpdate:(NSMenu *)pMenu;
- (BOOL)validateMenuItem:(NSMenuItem *)pMenuItem;
@end


class QuickstartMenuItemDescriptor
{
	SEL							maSelector;
	OUString					maText;
	::std::vector< QuickstartMenuItemDescriptor >	maItems;
	CFStringRef					maPrefName;
	CFPropertyListRef			maCheckedPrefValue;
	BOOL						mbValueIsDefaultForPref;
	BOOL						mbForceDefaultIfUnsetPref;

public:
								QuickstartMenuItemDescriptor( SEL aSelector, OUString aText, CFStringRef aPrefName = NULL, CFPropertyListRef aCheckedPrefValue = NULL, BOOL bValueIsDefaultForPref = NO, BOOL bForceDefaultIfUnsetPref = NO ) : maSelector( aSelector ), maText( aText ), maPrefName( aPrefName ), maCheckedPrefValue( aCheckedPrefValue ), mbValueIsDefaultForPref( bValueIsDefaultForPref ), mbForceDefaultIfUnsetPref( bForceDefaultIfUnsetPref ) {}
								QuickstartMenuItemDescriptor( ::std::vector< QuickstartMenuItemDescriptor > &rItems, OUString aText ) : maSelector( NULL ), maText( aText ), maItems( rItems ), maPrefName( NULL ), maCheckedPrefValue( NULL ), mbValueIsDefaultForPref( NO ), mbForceDefaultIfUnsetPref( NO ) {}
								~QuickstartMenuItemDescriptor() {};
	NSMenuItem*					CreateMenuItem( const ShutdownIconDelegate *pDelegate ) const;
};

NSMenuItem *QuickstartMenuItemDescriptor::QuickstartMenuItemDescriptor::CreateMenuItem( const ShutdownIconDelegate *pDelegate ) const
{
	NSMenuItem *pRet = nil;

	if ( maText.getLength() )
	{
		NSString *pTitle = [NSString stringWithCharacters:maText.getStr() length:maText.getLength()];
		if ( pTitle )
		{
			pRet = [[NSMenuItem alloc] initWithTitle:pTitle action:maSelector keyEquivalent:@""];
			if ( pRet )
			{
				if ( pDelegate )
					[pRet setTarget:pDelegate];

				if ( pRet && maItems.size() )
				{
					NSMenu *pMenu = [[NSMenu alloc] initWithTitle:pTitle];
					if ( pMenu )
					{
						if ( pDelegate )
							[pMenu setDelegate:pDelegate];

						for ( ::std::vector< QuickstartMenuItemDescriptor >::const_iterator it = maItems.begin(); it != maItems.end(); ++it )
						{
							NSMenuItem *pSubmenuItem = it->CreateMenuItem( pDelegate );
							if ( pSubmenuItem )
								[pMenu addItem:pSubmenuItem];

						}

						[pRet setSubmenu:pMenu];
					}
				}
				else if ( maPrefName && maCheckedPrefValue )
				{
					const NSString *pPrefKey = nil;
					NSObject *pPrefValue = nil;
					if ( CFGetTypeID( maCheckedPrefValue ) == CFBooleanGetTypeID() )
					{
						pPrefKey = kMenuItemPrefBooleanValueKey;
						pPrefValue = [NSNumber numberWithBool:(CFBooleanRef)maCheckedPrefValue == kCFBooleanTrue ? YES : NO];
					}
					else if ( CFGetTypeID( maCheckedPrefValue ) == CFStringGetTypeID() )
					{
						pPrefKey = kMenuItemPrefStringValueKey;
						pPrefValue = (NSString *)maCheckedPrefValue;
					}

					NSObject *pValueIsDefaultForPref = [NSNumber numberWithBool:mbValueIsDefaultForPref];
					NSObject *pForceDefaultIfUnsetPref = [NSNumber numberWithBool:mbForceDefaultIfUnsetPref];
					if ( pPrefKey && pPrefValue && pValueIsDefaultForPref && pForceDefaultIfUnsetPref )
					{
						NSDictionary *pDict = [NSDictionary dictionaryWithObjectsAndKeys:(NSString *)maPrefName, kMenuItemPrefNameKey, pPrefValue, pPrefKey, pValueIsDefaultForPref, kMenuItemValueIsDefaultForPrefKey, pForceDefaultIfUnsetPref, kMenuItemForceDefaultIfUnsetPrefKey, nil];
						if ( pDict )
							[pRet setRepresentedObject:pDict];
					}
				}
			}
		}
	}

	return pRet;
}

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
	(void)pData;

	switch ( mnCommand )
	{
		case WRITER_COMMAND_ID:
			ShutdownIcon::OpenURL( WRITER_URL, DEFAULT_URL );
			break;
		case CALC_COMMAND_ID:
			ShutdownIcon::OpenURL( CALC_URL, DEFAULT_URL );
			break;
		case IMPRESS_COMMAND_ID:
			ShutdownIcon::OpenURL( IMPRESS_WIZARD_URL, DEFAULT_URL );
			break;
		case DRAW_COMMAND_ID:
			ShutdownIcon::OpenURL( DRAW_URL, DEFAULT_URL );
			break;
		case MATH_COMMAND_ID:
			ShutdownIcon::OpenURL( MATH_URL, DEFAULT_URL );
			break;
		case BASE_COMMAND_ID:
			ShutdownIcon::OpenURL( BASE_URL, DEFAULT_URL );
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

@implementation ShutdownIconDelegate

- (BOOL)application:(NSApplication *)pApplication openFile:(NSString *)pFilename
{
	if ( mpDelegate && [mpDelegate respondsToSelector:@selector(application:openFile:)] )
		return [mpDelegate application:pApplication openFile:pFilename];
	else
		return NO;
}

- (BOOL)application:(NSApplication *)pApplication printFile:(NSString *)pFilename
{
	if ( mpDelegate && [mpDelegate respondsToSelector:@selector(application:printFile:)] )
		return [mpDelegate application:pApplication printFile:pFilename];
	else
		return NO;
}

- (NSMenu *)applicationDockMenu:(NSApplication *)pApplication
{
	if ( mpDelegate && [mpDelegate respondsToSelector:@selector(applicationDockMenu:)] )
		return [mpDelegate applicationDockMenu:pApplication];
	else
		return nil;
} 

- (void)applicationDidBecomeActive:(NSNotification *)pNotification
{
	if ( mpDelegate && [mpDelegate respondsToSelector:@selector(applicationDidBecomeActive:)] )
		[mpDelegate applicationDidBecomeActive:pNotification];
}

- (void)applicationDidChangeScreenParameters:(NSNotification *)pNotification
{
	if ( mpDelegate && [mpDelegate respondsToSelector:@selector(applicationDidChangeScreenParameters:)] )
		[mpDelegate applicationDidChangeScreenParameters:pNotification];
}

- (BOOL)applicationShouldHandleReopen:(NSApplication *)pApplication hasVisibleWindows:(BOOL)bFlag
{
	if ( mpDelegate && [mpDelegate respondsToSelector:@selector(applicationShouldHandleReopen:hasVisibleWindows:)] )
		return [mpDelegate applicationShouldHandleReopen:pApplication hasVisibleWindows:bFlag];
	else
		return NO;
}

- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)pApplication
{
	if ( mpDelegate && [mpDelegate respondsToSelector:@selector(applicationShouldTerminate:)] )
		return [mpDelegate applicationShouldTerminate:pApplication];
	else
		return NSTerminateCancel;
}

- (void)applicationWillBecomeActive:(NSNotification *)pNotification
{
	if ( mpDelegate && [mpDelegate respondsToSelector:@selector(applicationWillBecomeActive:)] )
		[mpDelegate applicationWillBecomeActive:pNotification];
}

- (void)applicationWillFinishLaunching:(NSNotification *)pNotification
{
	if ( mpDelegate && [mpDelegate respondsToSelector:@selector(applicationWillFinishLaunching:)] )
		[mpDelegate applicationWillFinishLaunching:pNotification];
}

- (void)applicationWillResignActive:(NSNotification *)pNotification
{
	if ( mpDelegate && [mpDelegate respondsToSelector:@selector(applicationWillResignActive:)] )
		[mpDelegate applicationWillResignActive:pNotification];
}

- (void)dealloc
{
	if ( mpDelegate )
		[mpDelegate release];

	[super dealloc];
}

- (id)init
{
	[super init];

	mpDelegate = nil;

	return self;
}

- (void)handleCalcCommand:(id)pObject
{
	(void)pObject;

	ProcessShutdownIconCommand( CALC_COMMAND_ID );
}

- (void)handleDrawCommand:(id)pObject
{
	(void)pObject;

	ProcessShutdownIconCommand( DRAW_COMMAND_ID );
}

- (void)handleFileOpenCommand:(id)pObject
{
	(void)pObject;

	ProcessShutdownIconCommand( FILEOPEN_COMMAND_ID );
}

- (void)handleFromTemplateCommand:(id)pObject
{
	(void)pObject;

	ProcessShutdownIconCommand( FROMTEMPLATE_COMMAND_ID );
}

- (void)handleImpressCommand:(id)pObject
{
	(void)pObject;

	ProcessShutdownIconCommand( IMPRESS_COMMAND_ID );
}

- (void)handleMathCommand:(id)pObject
{
	(void)pObject;

	ProcessShutdownIconCommand( MATH_COMMAND_ID );
}

- (void)handlePreferenceChangeCommand:(id)pObject
{
	if ( pObject && [pObject isKindOfClass:[NSMenuItem class]] )
	{
		NSMenuItem *pMenuItem = (NSMenuItem *)pObject;
		NSUserDefaults *pDefaults = [NSUserDefaults standardUserDefaults];
		NSObject *pPrefs = [pMenuItem representedObject];
		if ( pDefaults && pPrefs && [pPrefs isKindOfClass:[NSDictionary class]] )
		{
			NSDictionary *pDict = (NSDictionary *)pPrefs;
			NSString *pPrefName = (NSString *)[pDict objectForKey:kMenuItemPrefNameKey];
			if ( pPrefName )
			{
				NSNumber *pPrefBooleanValue = (NSNumber *)[pDict objectForKey:kMenuItemPrefBooleanValueKey];
				NSString *pPrefStringValue = (NSString *)[pDict objectForKey:kMenuItemPrefStringValueKey];
				if ( pPrefBooleanValue )
				{
					BOOL bValue = [pPrefBooleanValue boolValue];
					[pDefaults setBool:( [pMenuItem state] == NSOffState ? bValue : !bValue ) forKey:pPrefName];
					[pDefaults synchronize];
				}
				else if ( pPrefStringValue )
				{
					[pDefaults setObject:pPrefStringValue forKey:pPrefName];
					[pDefaults synchronize];
				}
			}
		}
	}
}

- (void)handleWriterCommand:(id)pObject
{
	(void)pObject;

	ProcessShutdownIconCommand( WRITER_COMMAND_ID );
}

- (void)handleBaseCommand:(id)pObject
{
	(void)pObject;

	ProcessShutdownIconCommand( BASE_COMMAND_ID );
}

- (void)setDelegate:(id)pDelegate
{
	if ( pDelegate != mpDelegate )
	{
		if ( mpDelegate )
			[mpDelegate release];
		mpDelegate = pDelegate;
		if ( mpDelegate )
			[mpDelegate retain];
	}
}

- (void)menuNeedsUpdate:(NSMenu *)pMenu
{
	if ( pMenu )
	{
		NSUserDefaults *pDefaults = [NSUserDefaults standardUserDefaults];
		NSMutableDictionary *pCheckedMenuItems = [NSMutableDictionary dictionaryWithCapacity:1];

		unsigned int nCount = [pMenu numberOfItems];
		unsigned i = 0;
		for ( ; i < nCount; i++ )
		{
			NSMenuItem *pMenuItem = [pMenu itemAtIndex:i];
			if ( pMenuItem )
			{
				NSObject *pPrefs = [pMenuItem representedObject];
				if ( pPrefs && [pPrefs isKindOfClass:[NSDictionary class]] )
				{
					[pMenuItem setState:NSOffState];

					NSDictionary *pDict = (NSDictionary *)pPrefs;
					NSString *pPrefName = (NSString *)[pDict objectForKey:kMenuItemPrefNameKey];
					if ( pPrefName )
					{
						NSNumber *pPrefBooleanValue = (NSNumber *)[pDict objectForKey:kMenuItemPrefBooleanValueKey];
						NSString *pPrefStringValue = (NSString *)[pDict objectForKey:kMenuItemPrefStringValueKey];
						NSNumber *pValueIsDefaultForPref = (NSNumber *)[pDict objectForKey:kMenuItemValueIsDefaultForPrefKey];
						NSNumber *pForceDefaultIfUnsetPref = (NSNumber *)[pDict objectForKey:kMenuItemForceDefaultIfUnsetPrefKey];
						if ( pPrefBooleanValue )
						{
							if ( pDefaults )
							{
								NSNumber *pValue = (NSNumber *)[pDefaults objectForKey:pPrefName];
								if ( pValue && [pValue boolValue] == [pPrefBooleanValue boolValue] )
									[pCheckedMenuItems setObject:pMenuItem forKey:pPrefName];
								else if ( !pValue && pForceDefaultIfUnsetPref && [pForceDefaultIfUnsetPref boolValue] )
									[pCheckedMenuItems setObject:pMenuItem forKey:pPrefName];
							}
						}
						else if ( pPrefStringValue )
						{
							if ( pDefaults )
							{
								NSString *pValue = [pDefaults stringForKey:pPrefName];
								if ( pValue && [pValue isEqualToString:pPrefStringValue] )
								{
									[pCheckedMenuItems setObject:pMenuItem forKey:pPrefName];
								}
								// Handle OpenOffice 3.x style launch options
								else if ( [pPrefName isEqualToString:(NSString *)DEFAULT_LAUNCH_OPTIONS_KEY] && pValue && [pValue length] > 1 && [pValue characterAtIndex:0] == '-' && [pValue characterAtIndex:1] != '-' )
								{
									pValue = [@"-" stringByAppendingString:pValue];
									if ( pValue && [pValue isEqualToString:pPrefStringValue] )
										[pCheckedMenuItems setObject:pMenuItem forKey:pPrefName];
								}
								else if ( !pValue && pForceDefaultIfUnsetPref && [pForceDefaultIfUnsetPref boolValue] )
									[pCheckedMenuItems setObject:pMenuItem forKey:pPrefName];
							}
						}

						// If no checked item is set for this key and the menu
						// item is marked as the default checked item, make it
						// the checked item until another menu item becomes the
						// checked item
						if ( pValueIsDefaultForPref && [pValueIsDefaultForPref boolValue] && ( !pForceDefaultIfUnsetPref || ![pForceDefaultIfUnsetPref boolValue] ) && pCheckedMenuItems && ![pCheckedMenuItems objectForKey:pPrefName] )
							[pCheckedMenuItems setObject:pMenuItem forKey:pPrefName];
					}
				}
			}
		}

		if ( pCheckedMenuItems )
		{
			NSArray *pArray = [pCheckedMenuItems allValues];
			if ( pArray )
			{
				nCount = [pArray count];
				i = 0;
				for ( ; i < nCount; i++ )
				{
					NSMenuItem *pMenuItem = (NSMenuItem *)[pArray objectAtIndex:i];
					if ( pMenuItem )
						[pMenuItem setState:NSOnState];
				}
			}
		}
	}
}

- (BOOL)validateMenuItem:(NSMenuItem *)pMenuItem
{
	if ( mpDelegate && [mpDelegate respondsToSelector:@selector(validateMenuItem:)] )
		return [mpDelegate validateMenuItem:pMenuItem];
	else
		return NO;
}

@end

@interface QuickstartMenuItems : NSObject
{
	const ::std::vector< QuickstartMenuItemDescriptor >*	mpItems;
}
+ (id)createWithItemDescriptors:(const ::std::vector< QuickstartMenuItemDescriptor >*)pItems;
- (void)addMenuItems:(id)pObject;
- (id)initWithItemDescriptors:(const ::std::vector< QuickstartMenuItemDescriptor >*)pItems;
@end

@implementation QuickstartMenuItems

+ (id)createWithItemDescriptors:(const ::std::vector< QuickstartMenuItemDescriptor >*)pItems
{
	QuickstartMenuItems *pRet = [[QuickstartMenuItems alloc] initWithItemDescriptors:pItems];
	[pRet autorelease];
	return pRet;
}

- (void)addMenuItems:(id)pObject
{
	(void)pObject;

	NSApplication *pApp = [NSApplication sharedApplication];
	if ( pApp && mpItems && mpItems->size() )
	{
		NSMenu *pAppMenu = nil;
		NSMenu *pMainMenu = [pApp mainMenu];
		if ( pMainMenu && [pMainMenu numberOfItems] > 0 )
		{
			NSMenuItem *pItem = [pMainMenu itemAtIndex:0];
			if ( pItem )
				pAppMenu = [pItem submenu];
		}

		id pDelegate = [pApp delegate];
		if ( !pDelegate || ![pDelegate isKindOfClass:[ShutdownIconDelegate class]] )
		{
			// Do not retain as invoking alloc disables autorelease
			ShutdownIconDelegate *pNewDelegate = [[ShutdownIconDelegate alloc] init];
			if ( pDelegate )
				[pNewDelegate setDelegate:pDelegate];
			// NSApplication does not retain delegates so don't release it
			[pApp setDelegate:pNewDelegate];
			pDelegate = pNewDelegate;
		}

		if ( pAppMenu && pDelegate && [pDelegate isKindOfClass:[ShutdownIconDelegate class]] )
		{
			NSMenu *pDockMenu = [(ShutdownIconDelegate *)pDelegate applicationDockMenu:pApp];
			if ( pDockMenu )
			{
				// Insert a separator menu item (only in the application menu)
				[pAppMenu insertItem:[NSMenuItem separatorItem] atIndex:2];
	
				// Work the list of menu items is reverse order
				for ( ::std::vector< QuickstartMenuItemDescriptor >::const_reverse_iterator it = mpItems->rbegin(); it != mpItems->rend(); ++it )
				{
					NSMenuItem *pAppMenuItem = it->CreateMenuItem( (ShutdownIconDelegate *)pDelegate );
					NSMenuItem *pDockMenuItem = it->CreateMenuItem( (ShutdownIconDelegate *)pDelegate );
					if ( pAppMenuItem )
						[pAppMenu insertItem:pAppMenuItem atIndex:2];
					if ( pDockMenuItem )
						[pDockMenu insertItem:pDockMenuItem atIndex:0];
				}

				mpItems = nil;
			}
		}
	}
}

- (id)initWithItemDescriptors:(const ::std::vector< QuickstartMenuItemDescriptor >*)pItems
{
	[super init];

	mpItems = pItems;

	return self;
}

@end

extern "C" void java_init_systray()
{
	ShutdownIcon *pShutdownIcon = ShutdownIcon::getInstance();
	if ( !pShutdownIcon )
		return;

	// Collect the URLs of the entries in the File/New menu
	::std::set< OUString > aFileNewAppsAvailable;
	SvtDynamicMenuOptions aOpt;
	Sequence < Sequence < PropertyValue > > aNewMenu = aOpt.GetMenu( E_NEWMENU );
	const OUString sURLKey( "URL" );

	const Sequence< PropertyValue >* pNewMenu = aNewMenu.getConstArray();
	const Sequence< PropertyValue >* pNewMenuEnd = aNewMenu.getConstArray() + aNewMenu.getLength();
	for ( ; pNewMenu != pNewMenuEnd; ++pNewMenu )
	{
		::comphelper::SequenceAsHashMap aEntryItems( *pNewMenu );
		OUString sURL( aEntryItems.getUnpackedValueOrDefault( sURLKey, OUString() ) );
		if ( sURL.getLength() )
			aFileNewAppsAvailable.insert( sURL );
	}

	// Describe the menu entries for launching the applications
	struct MenuEntryDescriptor
	{
		SvtModuleOptions::EModule	eModuleIdentifier;
		const char*					pAsciiURLDescription;
		const char*					pFallbackDescription;
		SEL							aNewSelector;
		const CFStringRef			aCheckedPrefValue;
		BOOL						bValueIsDefaultForPref;
	} aMenuItems[] =
	{
		{ SvtModuleOptions::E_SWRITER, WRITER_URL, WRITER_FALLBACK_DESC, @selector(handleWriterCommand:), CFSTR( "--writer" ), NO },
		// Make Calc the preferred default document
		{ SvtModuleOptions::E_SCALC, CALC_URL, CALC_FALLBACK_DESC, @selector(handleCalcCommand:), CFSTR( "--calc" ), YES },
		{ SvtModuleOptions::E_SIMPRESS, IMPRESS_WIZARD_URL, IMPRESS_WIZARD_FALLBACK_DESC, @selector(handleImpressCommand:), CFSTR( "--impress" ), NO },
		{ SvtModuleOptions::E_SDRAW, DRAW_URL, DRAW_FALLBACK_DESC, @selector(handleDrawCommand:), CFSTR( "--draw" ), NO },
		{ SvtModuleOptions::E_SDATABASE, BASE_URL, BASE_FALLBACK_DESC, @selector(handleBaseCommand:), CFSTR( "--base" ), NO },
		{ SvtModuleOptions::E_SMATH, MATH_URL, MATH_FALLBACK_DESC, @selector(handleMathCommand:), CFSTR( "--math" ), NO }
	};

	// Disable shutdown
	pShutdownIcon->SetVeto( true );
	pShutdownIcon->addTerminateListener();

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	::std::vector< QuickstartMenuItemDescriptor > aAppMenuItems;
	OUString aDesc;

	// Watch for changes to the executable file
	if ( !aFSEventStream )
	{
		if ( !pRestartMessageText )
		{
			aDesc = GetDktResString( STR_LO_MUST_BE_RESTARTED );
			if ( !aDesc.isEmpty() )
			{
				pRestartMessageText = [NSString stringWithCharacters:aDesc.getStr() length:aDesc.getLength()];
				if ( pRestartMessageText )
					[pRestartMessageText retain];
			}
		}

		if ( !aExecutablePath )
		{
			NSBundle *pBundle = [NSBundle mainBundle];
			if ( pBundle )
			{
				NSURL *pURL = [pBundle executableURL];
				if ( pURL && [pURL isKindOfClass:[NSURL class]] && [pURL isFileURL] )
				{
					NSFileManager *pFileManager = [NSFileManager defaultManager];
					if ( pFileManager && [pFileManager fileExistsAtPath:[pURL path]] )
					{
						aExecutablePath = (CFStringRef)[pURL path];
						if ( aExecutablePath )
							CFRetain( aExecutablePath );
					}
				}
			}
		}

		if ( aExecutablePath )
		{
			CFArrayRef aPaths = CFArrayCreate( kCFAllocatorDefault, (const void **)&aExecutablePath, 1, NULL );
			if ( aPaths )
			{
				aFSEventStream = FSEventStreamCreate( kCFAllocatorDefault, ExecutableFSEventStreamCallback, NULL, aPaths, kFSEventStreamEventIdSinceNow, 0.1, kFSEventStreamCreateFlagUseCFTypes | kFSEventStreamCreateFlagWatchRoot );
				if ( aFSEventStream )
				{
					FSEventStreamScheduleWithRunLoop( aFSEventStream, CFRunLoopGetMain(), kCFRunLoopDefaultMode );
					FSEventStreamStart( aFSEventStream );
				}

				CFRelease( aPaths );
			}
		}
	}

	// Insert the new document and default launch submenu entries
	::std::vector< QuickstartMenuItemDescriptor > aNewSubmenuItems;
	::std::vector< QuickstartMenuItemDescriptor > aOpenAtLaunchSubmenuItems;

	// None menu item is only used in default launch submenu
	aDesc = pShutdownIcon->GetResString( STR_NONE );
	aDesc = aDesc.replaceAll( "~", "" );
	aOpenAtLaunchSubmenuItems.push_back( QuickstartMenuItemDescriptor( @selector(handlePreferenceChangeCommand:), aDesc, DEFAULT_LAUNCH_OPTIONS_KEY, CFSTR( "--nodefault" ), NO ) );

	SvtModuleOptions aModuleOptions;
	for ( size_t i = 0; i < sizeof( aMenuItems ) / sizeof( MenuEntryDescriptor ); ++i )
	{
		// the complete application is not even installed
		if ( !aModuleOptions.IsModuleInstalled( aMenuItems[i].eModuleIdentifier ) )
			continue;

		OUString sURL = OUString::createFromAscii( aMenuItems[i].pAsciiURLDescription );

		// the application is installed, but the entry has been
		// configured to *not* appear in the File/New menu =>
		//  also let not appear it in the quickstarter
		if ( aFileNewAppsAvailable.find( sURL ) == aFileNewAppsAvailable.end() )
			continue;

		aDesc = pShutdownIcon->GetUrlDescription( sURL );
		aDesc = aDesc.replaceAll( "~", "" );
		// Fix bug 2206 by putting in some default text if the
		// description is an empty string
		if ( !aDesc.getLength() )
		{
			aDesc = OUString::createFromAscii( aMenuItems[i].pFallbackDescription );
			aDesc = aDesc.replaceAll( "~", "" );
		}
		aNewSubmenuItems.push_back( QuickstartMenuItemDescriptor( aMenuItems[i].aNewSelector, aDesc ) );

		// Add module name to open at launch submenu
		aDesc = aModuleOptions.GetModuleName( aMenuItems[i].eModuleIdentifier );
		aDesc = aDesc.replaceAll( "~", "" );
		if ( aDesc.getLength() )
			aOpenAtLaunchSubmenuItems.push_back( QuickstartMenuItemDescriptor( @selector(handlePreferenceChangeCommand:), aDesc, DEFAULT_LAUNCH_OPTIONS_KEY, aMenuItems[i].aCheckedPrefValue, aMenuItems[i].bValueIsDefaultForPref ) );
	}

	// Open template menu item is only used in new document submenu
	aDesc = pShutdownIcon->GetResString( STR_QUICKSTART_FROMTEMPLATE );
	aDesc = aDesc.replaceAll( "~", "" );
	aNewSubmenuItems.push_back( QuickstartMenuItemDescriptor( @selector(handleFromTemplateCommand:), aDesc ) );

	// Insert the new document submenu
	aDesc = GetVclResString( SV_BUTTONTEXT_NEW );
	aDesc = aDesc.replaceAll( "~", "" );
	aAppMenuItems.push_back( QuickstartMenuItemDescriptor( aNewSubmenuItems, aDesc ) );

	// Insert the open document menu item into the application menu
	aDesc = pShutdownIcon->GetResString( STR_QUICKSTART_FILEOPEN );
	aDesc = aDesc.replaceAll( "~", "" );
	aAppMenuItems.push_back( QuickstartMenuItemDescriptor( @selector(handleFileOpenCommand:), aDesc ) );

	// Insert the open at launch submenu
	aDesc = SfxResId( STR_OPENATLAUNCH );
	aDesc = aDesc.replaceAll( "~", "" );
	aAppMenuItems.push_back( QuickstartMenuItemDescriptor( aOpenAtLaunchSubmenuItems, aDesc ) );

	// Insert the Mac OS X submenu entries
	::std::vector< QuickstartMenuItemDescriptor > aMacOSXSubmenuItems;

	aDesc = SfxResId( STR_IGNORETRACKPADGESTURES );
	aDesc = aDesc.replaceAll( "~", "" );
	aMacOSXSubmenuItems.push_back( QuickstartMenuItemDescriptor( @selector(handlePreferenceChangeCommand:), aDesc, CFSTR( "IgnoreTrackpadGestures" ), kCFBooleanTrue, NO ) );

	aDesc = SfxResId( STR_DISABLEMACOSXSERVICESMENU );
	aDesc = aDesc.replaceAll( "~", "" );
	aMacOSXSubmenuItems.push_back( QuickstartMenuItemDescriptor( @selector(handlePreferenceChangeCommand:), aDesc, CFSTR( "DisableServicesMenu" ), kCFBooleanTrue, NO ) );

	aDesc = SfxResId( STR_DISABLEMACOSXTEXTHIGHLIGHTING );
	aDesc = aDesc.replaceAll( "~", "" );
	aMacOSXSubmenuItems.push_back( QuickstartMenuItemDescriptor( @selector(handlePreferenceChangeCommand:), aDesc, CFSTR( "UseNativeHighlightColor" ), kCFBooleanFalse, NO ) );

	// Insert the Quick Look submenu entries
	::std::vector< QuickstartMenuItemDescriptor > aQuickLookSubmenuItems;

	aDesc = SfxResId( STR_QUICKLOOKDISABLED );
	aDesc = aDesc.replaceAll( "~", "" );
	aQuickLookSubmenuItems.push_back( QuickstartMenuItemDescriptor( @selector(handlePreferenceChangeCommand:), aDesc, CFSTR( "DisablePDFThumbnailSupport" ), kCFBooleanTrue, NO ) );

	aDesc = SfxResId( STR_QUICKLOOKFIRSTPAGEONLY );
	aDesc = aDesc.replaceAll( "~", "" );
	aQuickLookSubmenuItems.push_back( QuickstartMenuItemDescriptor( @selector(handlePreferenceChangeCommand:), aDesc, CFSTR( "DisablePDFThumbnailSupport" ), kCFBooleanFalse, YES ) );

	aDesc = SfxResId( STR_QUICKLOOKALLPAGES );
	aDesc = aDesc.replaceAll( "~", "" );
	aQuickLookSubmenuItems.push_back( QuickstartMenuItemDescriptor( @selector(handlePreferenceChangeCommand:), aDesc, CFSTR( "DisablePDFThumbnailSupport" ), CFSTR( "All" ), YES ) );

	aDesc = SfxResId( STR_QUICKLOOKSUPPORT );
	aDesc = aDesc.replaceAll( "~", "" );
	aMacOSXSubmenuItems.push_back( QuickstartMenuItemDescriptor( aQuickLookSubmenuItems, aDesc ) );

	if ( NSDocument_versionsSupported() )
	{
		aDesc = SfxResId( STR_DISABLEVERSIONSSUPPORT );
		aDesc = aDesc.replaceAll( "~", "" );
		aMacOSXSubmenuItems.push_back( QuickstartMenuItemDescriptor( @selector(handlePreferenceChangeCommand:), aDesc, CFSTR( "DisableVersions" ), kCFBooleanTrue, NO ) );

		aDesc = SfxResId( STR_DISABLERESUMESUPPORT );
		aDesc = aDesc.replaceAll( "~", "" );
		aMacOSXSubmenuItems.push_back( QuickstartMenuItemDescriptor( @selector(handlePreferenceChangeCommand:), aDesc, CFSTR( "DisableResume" ), kCFBooleanTrue, NO ) );

		if ( !IsRunningHighSierraOrLower() )
		{
			aDesc = SfxResId( STR_DISABLEDARKMODE );
			aDesc = aDesc.replaceAll( "~", "" );
			aMacOSXSubmenuItems.push_back( QuickstartMenuItemDescriptor( @selector(handlePreferenceChangeCommand:), aDesc, CFSTR( "DisableDarkMode" ), kCFBooleanTrue, NO ) );
		}
	}

	// Insert the Mac OS X submenu
	aDesc = SfxResId( STR_MACOSXOPTIONS );
	aDesc = aDesc.replaceAll( "~", "" );
	aAppMenuItems.push_back( QuickstartMenuItemDescriptor( aMacOSXSubmenuItems, aDesc ) );

	sal_uLong nCount = Application::ReleaseSolarMutex();

	QuickstartMenuItems *pItems = [QuickstartMenuItems createWithItemDescriptors:&aAppMenuItems];
	NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
	[pItems performSelectorOnMainThread:@selector(addMenuItems:) withObject:pItems waitUntilDone:YES modes:pModes];

	Application::AcquireSolarMutex( nCount );

	[pPool release];
}

extern "C" void java_shutdown_systray()
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if (aFSEventStream)
	{
		FSEventStreamStop(aFSEventStream);
		FSEventStreamInvalidate(aFSEventStream);
		FSEventStreamRelease(aFSEventStream);
		aFSEventStream = NULL;
	}

	if ( pRestartMessageText )
	{
		[pRestartMessageText release];
		pRestartMessageText = nil;
	}

	if ( aExecutablePath )
	{
		CFRelease( aExecutablePath );
		aExecutablePath = NULL;
	}

	[pPool release];
}

bool ShutdownIcon::IsQuickstarterInstalled()
{
	return true;
}
