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
 *  Patrick Luby, July 2006
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2006 by Patrick Luby (patrick.luby@planamesa.com)
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

#import <Cocoa/Cocoa.h>

#ifndef _COCOA_FILEDIALOG_H_
#import "cocoa_dialog.h"
#endif

@interface ShowFileDialog : NSObject
{
	BOOL					mbChooseFiles;
	NSMutableDictionary*	mpControls;
	NSSavePanel*			mpFilePanel;
	int						mnResult;
	BOOL					mbShowAutoExtension;
	BOOL					mbShowFilterOptions;
	BOOL					mbShowImageTemplate;
	BOOL					mbShowLink;
	BOOL					mbShowPassword;
	BOOL					mbShowReadOnly;
	BOOL					mbShowSelection;
	BOOL					mbShowTemplate;
	BOOL					mbShowVersion;
	NSMutableDictionary*	mpTextFields;
	BOOL					mbUseFileOpenDialog;
}
- (void)dealloc;
- (NSString *)directory;
- (NSArray *)filenames;
- (id)initWithOptions:(BOOL)bUseFileOpenDialog chooseFiles:(BOOL)bChooseFiles showAutoExtension:(BOOL)bShowAutoExtension showFilterOptions:(BOOL)bShowFilterOptions showImageTemplate:(BOOL)bShowImageTemplate showLink:(BOOL)bShowLink showPassword:(BOOL)bShowPassword showReadOnly:(BOOL)bShowReadOnly showSelction:(BOOL)bShowSelection showTemplate:(BOOL)bShowTemplate showVersion:(BOOL)bShowVersion;
- (BOOL)isChecked:(int)nID;
- (NSString *)label:(int)nID;
- (int)result;
- (void)showFileDialog:(id)pObject;
- (void)setChecked:(int)nID checked:(BOOL)bChecked;
- (void)setDirectory:(NSString *)pDirectory;
- (void)setEnabled:(int)nID enabled:(BOOL)bEnabled;
- (void)setLabel:(int)nID label:(NSString *)pLabel;
- (void)setMultiSelectionMode:(BOOL)bMultiSelectionMode;
- (void)setTitle:(NSString *)pTitle;
@end

@implementation ShowFileDialog

- (void)dealloc
{
	if ( mpControls )
		[mpControls release];

	if ( mpFilePanel )
		[mpFilePanel release];

	if ( mpTextFields )
		[mpTextFields release];

	[super dealloc];
}

- (NSString *)directory
{
	return [mpFilePanel directory];
}

- (NSArray *)filenames
{
	if ( mnResult == NSOKButton )
	{
		if ( mbUseFileOpenDialog )
		{
			NSArray *pArray = [(NSOpenPanel *)mpFilePanel filenames];
			if ( pArray )
				return [NSArray arrayWithArray:pArray];
		}
		else
		{
			NSString *pFileName = [mpFilePanel filename];
			if ( pFileName )
				return [NSArray arrayWithObject:pFileName];
		}
	}
}

- (id)initWithOptions:(BOOL)bUseFileOpenDialog chooseFiles:(BOOL)bChooseFiles showAutoExtension:(BOOL)bShowAutoExtension showFilterOptions:(BOOL)bShowFilterOptions showImageTemplate:(BOOL)bShowImageTemplate showLink:(BOOL)bShowLink showPassword:(BOOL)bShowPassword showReadOnly:(BOOL)bShowReadOnly showSelction:(BOOL)bShowSelection showTemplate:(BOOL)bShowTemplate showVersion:(BOOL)bShowVersion
{
	[super init];

	mbChooseFiles = bChooseFiles;
	mnResult = NSCancelButton;
	mbShowAutoExtension = bShowAutoExtension;
	mbShowFilterOptions = bShowFilterOptions;
	mbShowImageTemplate = bShowImageTemplate;
	mbShowLink = bShowLink;
	mbShowPassword = bShowPassword;
	mbShowReadOnly = bShowReadOnly;
	mbShowSelection = bShowSelection;
	mbShowTemplate = bShowTemplate;
	mbShowVersion = bShowVersion;
	mbUseFileOpenDialog = bUseFileOpenDialog;

	mpControls = [[NSMutableDictionary alloc] init];
	if ( mpControls )
		[mpControls retain];

	mpTextFields = [[NSMutableDictionary alloc] init];
	if ( mpTextFields )
		[mpTextFields retain];

	if ( mbUseFileOpenDialog )
		mpFilePanel = (NSSavePanel *)[NSOpenPanel openPanel];
	else
		mpFilePanel = [NSSavePanel savePanel];

	if ( mpFilePanel )
	{
		[mpFilePanel retain];

		[mpFilePanel setCanCreateDirectories:YES];
		[mpFilePanel setCanSelectHiddenExtension:mbShowAutoExtension];

		if ( mbUseFileOpenDialog )
		{
			NSOpenPanel *pOpenPanel = (NSOpenPanel *)mpFilePanel;

			[pOpenPanel setCanChooseFiles:mbChooseFiles];
			[pOpenPanel setResolvesAliases:YES];

			if ( mbChooseFiles )
			{
				[pOpenPanel setCanChooseDirectories:NO];
				[pOpenPanel setTreatsFilePackagesAsDirectories:YES];
			}
			else
			{
				[pOpenPanel setCanChooseDirectories:YES];
				[pOpenPanel setTreatsFilePackagesAsDirectories:NO];
			}
		}
		else
		{
			[mpFilePanel setTreatsFilePackagesAsDirectories:NO];
		}
	}

	// Create filter options checkbox
	if ( mbShowFilterOptions )
	{
		NSButton *pButton = [[NSButton alloc] initWithFrame:NSMakeRect( 0, 0, 0, 0 )];
		if ( pButton )
		{
			[pButton setButtonType:NSSwitchButton];
			[pButton setState:NSOffState];
			[pButton setTitle:@""];
			[mpControls setValue:pButton forKey:[[NSNumber numberWithInt:COCOA_CONTROL_ID_FILTEROPTIONS] stringValue]];
		}
	}

	// Create image template popup
	if ( mbShowImageTemplate )
	{
		NSPopUpButton *pPopup = [[NSPopUpButton alloc] initWithFrame:NSMakeRect( 0, 0, 0, 0 )];
		if ( pPopup )
			[mpControls setValue:pPopup forKey:[[NSNumber numberWithInt:COCOA_CONTROL_ID_IMAGE_TEMPLATE] stringValue]];

		NSTextField *pTextField = [[NSTextField alloc] initWithFrame:NSMakeRect( 0, 0, 0, 0 )];
		if ( pTextField )
		{
			[pTextField setBordered:NO];
			[pTextField setDrawsBackground:NO];
			[pTextField setEditable:NO];
			[pTextField setSelectable:NO];
			[mpTextFields setValue:pTextField forKey:[[NSNumber numberWithInt:COCOA_CONTROL_ID_IMAGE_TEMPLATE] stringValue]];
		}
	}

	// Create link checkbox
	if ( mbShowLink )
	{
		NSButton *pButton = [[NSButton alloc] initWithFrame:NSMakeRect( 0, 0, 0, 0 )];
		if ( pButton )
		{
			[pButton setButtonType:NSSwitchButton];
			[pButton setState:NSOffState];
			[pButton setTitle:@""];
			[mpControls setValue:pButton forKey:[[NSNumber numberWithInt:COCOA_CONTROL_ID_LINK] stringValue]];
		}
	}

	// Create password checkbox
	if ( mbShowPassword )
	{
		NSButton *pButton = [[NSButton alloc] initWithFrame:NSMakeRect( 0, 0, 0, 0 )];
		if ( pButton )
		{
			[pButton setButtonType:NSSwitchButton];
			[pButton setState:NSOffState];
			[pButton setTitle:@""];
			[mpControls setValue:pButton forKey:[[NSNumber numberWithInt:COCOA_CONTROL_ID_PASSWORD] stringValue]];
		}
	}

	// Create read only checkbox
	if ( mbShowReadOnly )
	{
		NSButton *pButton = [[NSButton alloc] initWithFrame:NSMakeRect( 0, 0, 0, 0 )];
		if ( pButton )
		{
			[pButton setButtonType:NSSwitchButton];
			[pButton setState:NSOffState];
			[pButton setTitle:@""];
			[mpControls setValue:pButton forKey:[[NSNumber numberWithInt:COCOA_CONTROL_ID_READONLY] stringValue]];
		}
	}

	// Create selection checkbox
	if ( mbShowSelection )
	{
		NSButton *pButton = [[NSButton alloc] initWithFrame:NSMakeRect( 0, 0, 0, 0 )];
		if ( pButton )
		{
			[pButton setButtonType:NSSwitchButton];
			[pButton setState:NSOffState];
			[pButton setTitle:@""];
			[mpControls setValue:pButton forKey:[[NSNumber numberWithInt:COCOA_CONTROL_ID_SELECTION] stringValue]];
		}
	}

	// Create template popup
	if ( mbShowTemplate )
	{
		NSPopUpButton *pPopup = [[NSPopUpButton alloc] initWithFrame:NSMakeRect( 0, 0, 0, 0 )];
		if ( pPopup )
			[mpControls setValue:pPopup forKey:[[NSNumber numberWithInt:COCOA_CONTROL_ID_TEMPLATE] stringValue]];

		NSTextField *pTextField = [[NSTextField alloc] initWithFrame:NSMakeRect( 0, 0, 1000, 0 )];
		if ( pTextField )
		{
			[pTextField setBordered:NO];
			[pTextField setDrawsBackground:NO];
			[pTextField setEditable:NO];
			[pTextField setSelectable:NO];
			[mpTextFields setValue:pTextField forKey:[[NSNumber numberWithInt:COCOA_CONTROL_ID_TEMPLATE] stringValue]];
		}
	}

	// Create template popup
	if ( mbShowVersion )
	{
		NSPopUpButton *pPopup = [[NSPopUpButton alloc] initWithFrame:NSMakeRect( 0, 0, 0, 0 )];
		if ( pPopup )
			[mpControls setValue:pPopup forKey:[[NSNumber numberWithInt:COCOA_CONTROL_ID_VERSION] stringValue]];

		NSTextField *pTextField = [[NSTextField alloc] initWithFrame:NSMakeRect( 0, 0, 1000, 0 )];
		if ( pTextField )
		{
			[pTextField setBordered:NO];
			[pTextField setDrawsBackground:NO];
			[pTextField setEditable:NO];
			[pTextField setSelectable:NO];
			[mpTextFields setValue:pTextField forKey:[[NSNumber numberWithInt:COCOA_CONTROL_ID_VERSION] stringValue]];
		}
	}

	return self;
}

- (BOOL)isChecked:(int)nID
{
	BOOL bRet = NO;

	int nCocoaControlType = NSFileDialog_controlType( nID );
	if ( nID == COCOA_CONTROL_ID_AUTOEXTENSION )
	{
		bRet = [mpFilePanel isExtensionHidden];
	}
	else if ( nCocoaControlType == COCOA_CONTROL_TYPE_CHECKBOX )
	{
		NSButton *pButton = (NSButton *)[mpControls objectForKey:[[NSNumber numberWithInt:nID] stringValue]];
		if ( pButton )
			bRet = ( [pButton state] == NSOnState );
	}

	return bRet;
}

- (NSString *)label:(int)nID
{
	NSString *pRet = nil;

	int nCocoaControlType = NSFileDialog_controlType( nID );
	if ( nCocoaControlType == COCOA_CONTROL_TYPE_BUTTON || nCocoaControlType == COCOA_CONTROL_TYPE_CHECKBOX )
	{
		NSButton *pButton = (NSButton *)[mpControls objectForKey:[[NSNumber numberWithInt:nID] stringValue]];
		if ( pButton )
			pRet = [pButton title];
	}
	else if ( nCocoaControlType == COCOA_CONTROL_TYPE_POPUP )
	{
		NSTextField *pTextField = (NSTextField *)[mpTextFields objectForKey:[[NSNumber numberWithInt:nID] stringValue]];
		if ( pTextField )
			pRet = [pTextField stringValue];
	}


	return pRet;
}

- (int)result;
{
	return mnResult;
}

- (void)showFileDialog:(id)pObject;
{
	mnResult = NSCancelButton;

	// Create accessory view
	NSView *pAccessoryView = [[NSView alloc] initWithFrame:NSMakeRect(0, 0, 0, 0)];
	if ( pAccessoryView )
	{
		NSView *pOldAccessoryView = [mpFilePanel accessoryView];

		float nCurrentY = 0;
		float nCurrentWidth = 0;

		int i = 0;
		for ( ; i < MAX_COCOA_CONTROL_ID; i++ )
		{
			NSControl *pControl = (NSControl *)[mpControls objectForKey:[[NSNumber numberWithInt:i] stringValue]];
			if ( pControl )
			{
				float nTextWidth = 0;
				float nTextHeight = 0;
				NSTextField *pTextField = nil;
				if ( NSFileDialog_controlType( i ) == COCOA_CONTROL_TYPE_POPUP )
				{
					pTextField = (NSTextField *)[mpTextFields objectForKey:[[NSNumber numberWithInt:i] stringValue]];
					if ( pTextField )
					{
						[pTextField setFrameOrigin:NSMakePoint( 0, nCurrentY )];
						[pTextField sizeToFit];
						nTextWidth = [pTextField bounds].size.width;
						nTextHeight = [pTextField bounds].size.height;
					}
				}

				[pControl setFrameOrigin:NSMakePoint( nTextWidth, nCurrentY )];
				[pControl sizeToFit];
				float nWidth = nTextWidth + [pControl bounds].size.width;
				if ( nCurrentWidth < nWidth )
					nCurrentWidth = nWidth;

				// Adjust text label vertically to match popup position
				float nHeight = [pControl bounds].size.height;
				if ( pTextField )
				{
					if ( nHeight < nTextHeight )
						[pControl setFrameOrigin:NSMakePoint( 0, nCurrentY + ( ( nTextHeight - nHeight ) / 2 ) )];
					else
						[pTextField setFrameOrigin:NSMakePoint( 0, nCurrentY + ( ( nHeight - nTextHeight ) / 2 ) )];
					[pAccessoryView addSubview:pTextField];
				}

				if ( nHeight < nTextHeight )
					nCurrentY += nTextHeight;
				else
					nCurrentY += nHeight;

				[pAccessoryView addSubview:pControl];
					
			}
		}

		NSArray *pSubviews = [pAccessoryView subviews];
		if ( pSubviews && [pSubviews count] )
		{
			[pAccessoryView setFrameSize:NSMakeSize( nCurrentWidth, nCurrentY )];
			[mpFilePanel setAccessoryView:pAccessoryView];
		}

		if ( mbUseFileOpenDialog )
		{
			NSOpenPanel *pOpenPanel = (NSOpenPanel *)mpFilePanel;
			mnResult = [pOpenPanel runModalForDirectory:nil file:nil types:nil];
		}
		else
		{
			mnResult = [mpFilePanel runModalForDirectory:nil file:nil];
		}

		[mpFilePanel setAccessoryView:pOldAccessoryView];

		for ( i = 0; i < MAX_COCOA_CONTROL_ID; i++ )
		{
			NSControl *pControl = (NSControl *)[mpControls objectForKey:[[NSNumber numberWithInt:i] stringValue]];
			if ( pControl )
				[pControl removeFromSuperview];
		}
	}
}

- (void)setDirectory:(NSString *)pDirectory
{
	[mpFilePanel setDirectory:pDirectory];
}

- (void)setChecked:(int)nID checked:(BOOL)bChecked
{
	int nCocoaControlType = NSFileDialog_controlType( nID );
	if ( nID == COCOA_CONTROL_ID_AUTOEXTENSION )
	{
		[mpFilePanel setExtensionHidden:bChecked];
	}
	else if ( nCocoaControlType == COCOA_CONTROL_TYPE_CHECKBOX )
	{
		NSButton *pButton = (NSButton *)[mpControls objectForKey:[[NSNumber numberWithInt:nID] stringValue]];
		if ( pButton )
			[pButton setState:( bChecked ? NSOnState : NSOffState )];
	}
}

- (void)setEnabled:(int)nID enabled:(BOOL)bEnabled
{
	NSControl *pControl = (NSControl *)[mpControls objectForKey:[[NSNumber numberWithInt:nID] stringValue]];
	if ( pControl )
		[pControl setEnabled:bEnabled];
}

- (void)setLabel:(int)nID label:(NSString *)pLabel
{
	if ( !pLabel )
		pLabel = @"";

	int nCocoaControlType = NSFileDialog_controlType( nID );
	if ( nCocoaControlType == COCOA_CONTROL_TYPE_CHECKBOX )
	{
		NSButton *pButton = (NSButton *)[mpControls objectForKey:[[NSNumber numberWithInt:nID] stringValue]];
		if ( pButton )
			[pButton setTitle:pLabel];
	}
	else if ( nCocoaControlType == COCOA_CONTROL_TYPE_POPUP )
	{
		NSTextField *pTextField = (NSTextField *)[mpTextFields objectForKey:[[NSNumber numberWithInt:nID] stringValue]];
		if ( pTextField )
			[pTextField setStringValue:pLabel];
	}
}

- (void)setMultiSelectionMode:(BOOL)bMultiSelectionMode
{
	if ( mbUseFileOpenDialog )
		[(NSOpenPanel *)mpFilePanel setAllowsMultipleSelection:bMultiSelectionMode];
}

- (void)setTitle:(NSString *)pTitle
{
}

@end

int NSFileDialog_controlType( int nID )
{
	int nRet = MAX_COCOA_CONTROL_TYPE;

	switch ( nID )
	{
		case COCOA_CONTROL_ID_PLAY:
			nRet = COCOA_CONTROL_TYPE_BUTTON;
			break;
		case COCOA_CONTROL_ID_AUTOEXTENSION:
		case COCOA_CONTROL_ID_FILTEROPTIONS:
		case COCOA_CONTROL_ID_LINK:
		case COCOA_CONTROL_ID_PASSWORD:
		case COCOA_CONTROL_ID_PREVIEW:
		case COCOA_CONTROL_ID_READONLY:
		case COCOA_CONTROL_ID_SELECTION:
			nRet = COCOA_CONTROL_TYPE_CHECKBOX;
			break;
		case COCOA_CONTROL_ID_IMAGE_TEMPLATE:
		case COCOA_CONTROL_ID_TEMPLATE:
		case COCOA_CONTROL_ID_VERSION:
			nRet = COCOA_CONTROL_TYPE_POPUP;
			break;
	}

	return nRet;
}

id NSFileDialog_create( BOOL bUseFileOpenDialog, BOOL bChooseFiles, BOOL bShowAutoExtension, BOOL bShowFilterOptions, BOOL bShowImageTemplate, BOOL bShowLink, BOOL bShowPassword, BOOL bShowReadOnly, BOOL bShowSelection, BOOL bShowTemplate, BOOL bShowVersion )
{
	ShowFileDialog *pRet = nil;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	pRet = [[ShowFileDialog alloc] initWithOptions:bUseFileOpenDialog chooseFiles:bChooseFiles showAutoExtension:bShowAutoExtension showFilterOptions:bShowFilterOptions showImageTemplate:bShowImageTemplate showLink:bShowLink showPassword:bShowPassword showReadOnly:bShowReadOnly showSelction:bShowSelection showTemplate:bShowTemplate showVersion:bShowVersion];
	if ( pRet )
		[pRet retain];

	[pPool release];

	return pRet;
}

CFStringRef NSFileDialog_directory( id pDialog )
{
	CFStringRef aRet = nil;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pDialog )
	{
		NSString *pDirectory = [(ShowFileDialog *)pDialog directory];
		if ( pDirectory )
		{
			[pDirectory retain];
			aRet = (CFStringRef)pDirectory;
		}
	}

	[pPool release];

	return aRet;
}

CFStringRef *NSFileDialog_fileNames( id pDialog )
{
	CFStringRef *pRet = nil;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pDialog )
	{
		NSArray *pFileNames = [(ShowFileDialog *)pDialog filenames];
		if ( pFileNames )
		{
			unsigned nCount = [pFileNames count];
			if ( nCount )
			{
				pRet = (CFStringRef *)malloc( ( nCount + 1 ) * sizeof( CFStringRef ) );
				if ( pRet )
				{
					unsigned nIndex = 0;
					unsigned i = 0;
					for ( ; i < nCount; i++ )
					{
						NSString *pCurrentName = (NSString *)[pFileNames objectAtIndex:i];
						if ( pCurrentName )
						{
							[pCurrentName retain];
							pRet[ nIndex++ ] = (CFStringRef)pCurrentName;
						}
					}

					pRet[ nIndex ] = nil;
				}
			}
		}
	}

	[pPool release];

	return pRet;
}

BOOL NSFileDialog_isChecked( id pDialog, int nID )
{
	BOOL bRet = NO;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pDialog )
		bRet = [(ShowFileDialog *)pDialog isChecked:nID];

	[pPool release];

	return bRet;
}

CFStringRef NSFileDialog_label( id pDialog, int nID )
{
	CFStringRef aRet = nil;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pDialog )
	{
		NSString *pLabel = [(ShowFileDialog *)pDialog label:nID];
		if ( pLabel )
		{
			[pLabel retain];
			aRet = (CFStringRef)pLabel;
		}
	}

	[pPool release];

	return aRet;
}

void NSFileDialog_release( id pDialog )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pDialog )
		[(ShowFileDialog *)pDialog release];

	[pPool release];
}

void NSFontManager_releaseFileNames( CFStringRef *pFileNames )
{           
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pFileNames )
	{
		unsigned nIndex = 0;
		for ( ; pFileNames [ nIndex ]; nIndex++ )
			CFRelease( pFileNames[ nIndex ] );
		free( pFileNames );
	}

	[pPool release];
}

int NSFileDialog_result( id pDialog )
{
	int nRet = NO;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pDialog )
	{
		nRet = [(ShowFileDialog *)pDialog result];
		[(ShowFileDialog *)pDialog release];
	}

	[pPool release];

	return nRet;
}

int NSFileDialog_showFileDialog( id pDialog )
{
	int nRet = NO;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pDialog )
	{
		[(ShowFileDialog *)pDialog performSelectorOnMainThread:@selector(showFileDialog:) withObject:pDialog waitUntilDone:YES];
		nRet = [(ShowFileDialog *)pDialog result];
	}

	[pPool release];

	return nRet;
}

void NSFileDialog_setChecked( id pDialog, int nID, BOOL bChecked )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pDialog )
		[(ShowFileDialog *)pDialog setChecked:nID checked:bChecked];

	[pPool release];
}

void NSFileDialog_setDirectory( id pDialog, CFStringRef aDirectory )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pDialog )
		[(ShowFileDialog *)pDialog setDirectory:(NSString *)aDirectory ];

	[pPool release];
}

void NSFileDialog_setEnabled( id pDialog, int nID, BOOL bEnabled )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pDialog )
		[(ShowFileDialog *)pDialog setEnabled:nID enabled:bEnabled ];

	[pPool release];
}

void NSFileDialog_setLabel( id pDialog, int nID, CFStringRef aLabel )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pDialog )
		[(ShowFileDialog *)pDialog setLabel:nID label:(NSString *)aLabel];

	[pPool release];
}

void NSFileDialog_setMultiSelectionMode( id pDialog, BOOL bMultiSelectionMode )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pDialog )
		[(ShowFileDialog *)pDialog setMultiSelectionMode:bMultiSelectionMode];

	[pPool release];
}

void NSFileDialog_setTitle( id pDialog, CFStringRef aTitle )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pDialog )
		[(ShowFileDialog *)pDialog setTitle:(NSString *)aTitle];

	[pPool release];
}
