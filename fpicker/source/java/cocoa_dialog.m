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

static NSString *pBlankItem = @" ";

@interface ShowFileDialog : NSObject
{
	BOOL					mbChooseFiles;
	NSMutableDictionary*	mpControls;
	NSString*				mpDefaultName;
	NSSavePanel*			mpFilePanel;
	NSMutableDictionary*	mpFilters;
	int						mnResult;
	NSMutableDictionary*	mpTextFields;
	BOOL					mbUseFileOpenDialog;
}
- (void)addFilter:(NSString *)pItem filter:(NSString *)pFilter;
- (void)addItem:(int)nID item:(NSString *)pItem;
- (void)cancel;
- (void)dealloc;
- (void)deleteItem:(int)nID item:(NSString *)pItem;
- (NSString *)directory;
- (NSArray *)filenames;
- (NSArray *)items:(int)nID;
- (id)initWithOptions:(BOOL)bUseFileOpenDialog chooseFiles:(BOOL)bChooseFiles showAutoExtension:(BOOL)bShowAutoExtension showFilterOptions:(BOOL)bShowFilterOptions showImageTemplate:(BOOL)bShowImageTemplate showLink:(BOOL)bShowLink showPassword:(BOOL)bShowPassword showReadOnly:(BOOL)bShowReadOnly showSelction:(BOOL)bShowSelection showTemplate:(BOOL)bShowTemplate showVersion:(BOOL)bShowVersion;
- (BOOL)isChecked:(int)nID;
- (NSString *)label:(int)nID;
- (int)result;
- (NSString *)selectedItem:(int)nID;
- (int)selectedItemIndex:(int)nID;
- (NSString *)selectedFilter;
- (void)setChecked:(int)nID checked:(BOOL)bChecked;
- (void)setDefaultName:(NSString *)pName;
- (void)setDirectory:(NSString *)pDirectory;
- (void)setEnabled:(int)nID enabled:(BOOL)bEnabled;
- (void)setLabel:(int)nID label:(NSString *)pLabel;
- (void)setMultiSelectionMode:(BOOL)bMultiSelectionMode;
- (void)setSelectedFilter:(NSString *)pItem;
- (void)setSelectedItem:(int)nID item:(int)nItem;
- (void)setTitle:(NSString *)pTitle;
- (void)showFileDialog:(id)pObject;
@end

@implementation ShowFileDialog

- (void)addFilter:(NSString *)pItem filter:(NSString *)pFilter
{
	if ( !pItem || !pFilter )
		return;

	NSArray *pFilters = [pFilter componentsSeparatedByString:@";"];
	if ( pFilters && [pFilters count] )
	{
		NSMutableArray *pArray = [NSMutableArray arrayWithArray:pFilters];
		if ( pArray )
		{
			int nCount = [pArray count];
			BOOL bAllowAll = NO;
			int i = 0;
			for ( ; i < nCount; i++ )
			{
				NSString *pCurrentFilter = [pArray objectAtIndex:i];
				NSRange aRange = [pCurrentFilter rangeOfString:@"." options:NSLiteralSearch | NSBackwardsSearch];
				if ( aRange.location != NSNotFound )
				{
					pCurrentFilter = [pCurrentFilter substringFromIndex:aRange.location + 1];
					[pArray insertObject:pCurrentFilter atIndex:i];
				}

				if ( [pCurrentFilter isEqualToString:@"*"] )
				{
					bAllowAll = YES;
					break;
				}
			}

			if ( !bAllowAll )
				[mpFilters setValue:pFilters forKey:pFilter];
		}
	}

	NSPopUpButton *pPopup = (NSPopUpButton *)[mpControls objectForKey:[[NSNumber numberWithInt:COCOA_CONTROL_ID_FILETYPE] stringValue]];
	if ( pPopup )
	{
		if ( [pPopup numberOfItems] == 1 && [pBlankItem isEqualToString:[pPopup itemTitleAtIndex:0]] )
			[pPopup removeAllItems];
		[pPopup addItemWithTitle:pItem];
	}
}

- (void)addItem:(int)nID item:(NSString *)pItem
{
	if ( !pItem || nID == COCOA_CONTROL_ID_FILETYPE )
		return;

	if ( NSFileDialog_controlType( nID ) == COCOA_CONTROL_TYPE_POPUP )
	{
		NSPopUpButton *pPopup = (NSPopUpButton *)[mpControls objectForKey:[[NSNumber numberWithInt:nID] stringValue]];
		if ( pPopup )
		{
			if ( [pPopup numberOfItems] == 1 && [pBlankItem isEqualToString:[pPopup itemTitleAtIndex:0]] )
				[pPopup removeAllItems];
			[pPopup addItemWithTitle:pItem];
		}
	}
}

- (void)cancel
{
	[mpFilePanel cancel:self];
}

- (void)dealloc
{
	if ( mpControls )
		[mpControls release];

	if ( mpDefaultName )
		[mpDefaultName release];

	if ( mpFilePanel )
		[mpFilePanel release];

	if ( mpFilters )
		[mpFilters release];

	if ( mpTextFields )
		[mpTextFields release];

	[super dealloc];
}

- (void)deleteItem:(int)nID item:(NSString *)pItem
{
	if ( !pItem || nID == COCOA_CONTROL_ID_FILETYPE )
		return;

	if ( NSFileDialog_controlType( nID ) == COCOA_CONTROL_TYPE_POPUP )
	{
		NSPopUpButton *pPopup = (NSPopUpButton *)[mpControls objectForKey:[[NSNumber numberWithInt:nID] stringValue]];
		if ( pPopup )
		{
			[pPopup removeItemWithTitle:pItem];
			if ( ![pPopup numberOfItems] )
				[pPopup addItemWithTitle:pBlankItem];
		}
	}
}

- (NSString *)directory
{
	return [mpFilePanel directory];
}

- (NSArray *)filenames
{
	NSArray *pRet = nil;

	if ( mbUseFileOpenDialog )
	{
		NSArray *pArray = [(NSOpenPanel *)mpFilePanel filenames];
		if ( pArray )
			pRet = [NSArray arrayWithArray:pArray];
	}
	else
	{
		NSString *pFileName = [mpFilePanel filename];
		if ( pFileName )
			pRet = [NSArray arrayWithObject:pFileName];
	}

	return pRet;
}

- (NSArray *)items:(int)nID
{
	NSArray *pRet = nil;

	if ( NSFileDialog_controlType( nID ) == COCOA_CONTROL_TYPE_CHECKBOX )
	{
		NSPopUpButton *pPopup = (NSPopUpButton *)[mpControls objectForKey:[[NSNumber numberWithInt:nID] stringValue]];
		if ( pPopup )
			pRet = [pPopup itemArray];
	}

	return pRet;
}

- (id)initWithOptions:(BOOL)bUseFileOpenDialog chooseFiles:(BOOL)bChooseFiles showAutoExtension:(BOOL)bShowAutoExtension showFilterOptions:(BOOL)bShowFilterOptions showImageTemplate:(BOOL)bShowImageTemplate showLink:(BOOL)bShowLink showPassword:(BOOL)bShowPassword showReadOnly:(BOOL)bShowReadOnly showSelction:(BOOL)bShowSelection showTemplate:(BOOL)bShowTemplate showVersion:(BOOL)bShowVersion
{
	[super init];

	mbChooseFiles = bChooseFiles;
	mpDefaultName = nil;
	mnResult = NSCancelButton;
	mbUseFileOpenDialog = bUseFileOpenDialog;

	mpControls = [[NSMutableDictionary alloc] init];
	if ( mpControls )
		[mpControls retain];

	mpFilters = [[NSMutableDictionary alloc] init];
	if ( mpFilters )
		[mpFilters retain];

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
		[mpFilePanel setCanSelectHiddenExtension:bShowAutoExtension];

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

	// Create file type popup
	if ( mbChooseFiles )
	{
		NSPopUpButton *pPopup = [[NSPopUpButton alloc] initWithFrame:NSMakeRect( 0, 0, 0, 0 ) pullsDown:NO];
		if ( pPopup )
		{
			[pPopup addItemWithTitle:pBlankItem];
			[mpControls setValue:pPopup forKey:[[NSNumber numberWithInt:COCOA_CONTROL_ID_FILETYPE] stringValue]];
		}

		NSTextField *pTextField = [[NSTextField alloc] initWithFrame:NSMakeRect( 0, 0, 0, 0 )];
		if ( pTextField )
		{
			[pTextField setBordered:NO];
			[pTextField setDrawsBackground:NO];
			[pTextField setEditable:NO];
			[pTextField setSelectable:NO];
			[mpTextFields setValue:pTextField forKey:[[NSNumber numberWithInt:COCOA_CONTROL_ID_FILETYPE] stringValue]];
		}
	}

	// Create filter options checkbox
	if ( bShowFilterOptions )
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
	if ( bShowImageTemplate )
	{
		NSPopUpButton *pPopup = [[NSPopUpButton alloc] initWithFrame:NSMakeRect( 0, 0, 0, 0 ) pullsDown:NO];
		if ( pPopup )
		{
			[pPopup addItemWithTitle:pBlankItem];
			[mpControls setValue:pPopup forKey:[[NSNumber numberWithInt:COCOA_CONTROL_ID_IMAGE_TEMPLATE] stringValue]];
		}

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
	if ( bShowLink )
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
	if ( bShowPassword )
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
	if ( bShowReadOnly )
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
	if ( bShowSelection )
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
	if ( bShowTemplate )
	{
		NSPopUpButton *pPopup = [[NSPopUpButton alloc] initWithFrame:NSMakeRect( 0, 0, 0, 0 ) pullsDown:NO];
		if ( pPopup )
		{
			[pPopup addItemWithTitle:pBlankItem];
			[mpControls setValue:pPopup forKey:[[NSNumber numberWithInt:COCOA_CONTROL_ID_TEMPLATE] stringValue]];
		}

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
	if ( bShowVersion )
	{
		NSPopUpButton *pPopup = [[NSPopUpButton alloc] initWithFrame:NSMakeRect( 0, 0, 0, 0 ) pullsDown:NO];
		if ( pPopup )
		{
			[pPopup addItemWithTitle:pBlankItem];
			[mpControls setValue:pPopup forKey:[[NSNumber numberWithInt:COCOA_CONTROL_ID_VERSION] stringValue]];
		}

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

- (NSString *)selectedItem:(int)nID
{
	NSString *pRet = nil;

	if ( NSFileDialog_controlType( nID ) == COCOA_CONTROL_TYPE_CHECKBOX )
	{
		NSPopUpButton *pPopup = (NSPopUpButton *)[mpControls objectForKey:[[NSNumber numberWithInt:nID] stringValue]];
		if ( pPopup )
			pRet = [pPopup titleOfSelectedItem];
	}

	return pRet;
}

- (int)selectedItemIndex:(int)nID
{
	int nRet = 0;

	if ( NSFileDialog_controlType( nID ) == COCOA_CONTROL_TYPE_CHECKBOX )
	{
		NSPopUpButton *pPopup = (NSPopUpButton *)[mpControls objectForKey:[[NSNumber numberWithInt:nID] stringValue]];
		if ( pPopup )
			nRet = [pPopup indexOfSelectedItem];
	}

	return nRet;
}

- (NSString *)selectedFilter
{
	NSString *pRet = nil;

	NSPopUpButton *pPopup = (NSPopUpButton *)[mpControls objectForKey:[[NSNumber numberWithInt:COCOA_CONTROL_ID_FILETYPE] stringValue]];
	if ( pPopup )
		pRet = [pPopup titleOfSelectedItem];

	return pRet;
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

- (void)setDefaultName:(NSString *)pName
{
	if ( mpDefaultName )
	{
		[mpDefaultName release];
		mpDefaultName = nil;
	}

	if ( pName )
	{
		[pName retain];
		mpDefaultName = pName;
	}
}

- (void)setDirectory:(NSString *)pDirectory
{
	[mpFilePanel setDirectory:pDirectory];
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

- (void)setSelectedFilter:(NSString *)pItem
{
	[mpFilePanel setAllowedFileTypes:(NSArray *)[mpFilters objectForKey:pItem]];

	NSPopUpButton *pPopup = (NSPopUpButton *)[mpControls objectForKey:[[NSNumber numberWithInt:COCOA_CONTROL_ID_FILETYPE] stringValue]];
	if ( pPopup )
		[pPopup selectItemWithTitle:pItem];
}

- (void)setSelectedItem:(int)nID item:(int)nItem
{
	if ( nID == COCOA_CONTROL_ID_FILETYPE )
		return;

	if ( NSFileDialog_controlType( nID ) == COCOA_CONTROL_TYPE_POPUP )
	{
		NSPopUpButton *pPopup = (NSPopUpButton *)[mpControls objectForKey:[[NSNumber numberWithInt:nID] stringValue]];
		if ( pPopup )
			[pPopup selectItemAtIndex:nItem];
	}
}

- (void)setTitle:(NSString *)pTitle
{
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
			mnResult = [pOpenPanel runModalForDirectory:[pOpenPanel directory] file:mpDefaultName types:[pOpenPanel allowedFileTypes]];
		}
		else
		{
			mnResult = [mpFilePanel runModalForDirectory:[mpFilePanel directory] file:mpDefaultName];
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
		case COCOA_CONTROL_ID_FILETYPE:
			nRet = COCOA_CONTROL_TYPE_POPUP;
			break;
	}

	return nRet;
}

void NSFileDialog_addFilter( id pDialog, CFStringRef aItem, CFStringRef aFilter )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pDialog )
		[(ShowFileDialog *)pDialog addFilter:(NSString *)aItem filter:(NSString *)aFilter];

	[pPool release];
}

void NSFileDialog_addItem( id pDialog, int nID, CFStringRef aItem )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pDialog )
		[(ShowFileDialog *)pDialog addItem:nID item:(NSString *)aItem];

	[pPool release];
}

void NSFileDialog_cancel( id pDialog )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pDialog )
		[(ShowFileDialog *)pDialog cancel];

	[pPool release];
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

void NSFileDialog_deleteItem( id pDialog, int nID, CFStringRef aItem )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pDialog )
		[(ShowFileDialog *)pDialog deleteItem:nID item:(NSString *)aItem];

	[pPool release];
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

CFStringRef *NSFileDialog_items( id pDialog, int nID )
{
	CFStringRef *pRet = nil;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pDialog )
	{
		NSArray *pItems = [(ShowFileDialog *)pDialog items:nID];
		if ( pItems )
		{
			unsigned nCount = [pItems count];
			if ( nCount )
			{
				pRet = (CFStringRef *)malloc( ( nCount + 1 ) * sizeof( CFStringRef ) );
				if ( pRet )
				{
					unsigned nIndex = 0;
					unsigned i = 0;
					for ( ; i < nCount; i++ )
					{
						NSString *pCurrentItem = (NSString *)[pItems objectAtIndex:i];
						if ( pCurrentItem )
						{
							[pCurrentItem retain];
							pRet[ nIndex++ ] = (CFStringRef)pCurrentItem;
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

void NSFileManager_releaseFileNames( CFStringRef *pFileNames )
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

void NSFileManager_releaseItems( CFStringRef *pItems )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pItems )
	{
		unsigned nIndex = 0;
		for ( ; pItems [ nIndex ]; nIndex++ )
			CFRelease( pItems[ nIndex ] );
		free( pItems );
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

CFStringRef NSFileDialog_selectedFilter( id pDialog )
{
	CFStringRef aRet = nil;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pDialog )
	{
		NSString *pItem = [(ShowFileDialog *)pDialog selectedFilter];
		if ( pItem )
		{
			[pItem retain];
			aRet = (CFStringRef)pItem;
		}
	}

	[pPool release];

	return aRet;
}

CFStringRef NSFileDialog_selectedItem( id pDialog, int nID )
{
	CFStringRef aRet = nil;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pDialog )
	{
		NSString *pItem = [(ShowFileDialog *)pDialog selectedItem:nID];
		if ( pItem )
		{
			[pItem retain];
			aRet = (CFStringRef)pItem;
		}
	}

	[pPool release];

	return aRet;
}

int NSFileDialog_selectedItemIndex( id pDialog, int nID )
{
	int nRet = 0;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pDialog )
		nRet = [(ShowFileDialog *)pDialog selectedItemIndex:nID];

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

void NSFileDialog_setDefaultName( id pDialog, CFStringRef aName )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pDialog )
		[(ShowFileDialog *)pDialog setDefaultName:(NSString *)aName];

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

void NSFileDialog_setSelectedFilter( id pDialog, CFStringRef aItem )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pDialog )
		[(ShowFileDialog *)pDialog setSelectedFilter:(NSString *)aItem];

	[pPool release];
}

void NSFileDialog_setSelectedItem( id pDialog, int nID, int nItem )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pDialog )
		[(ShowFileDialog *)pDialog setSelectedItem:nID item:nItem];

	[pPool release];
}

void NSFileDialog_setTitle( id pDialog, CFStringRef aTitle )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pDialog )
		[(ShowFileDialog *)pDialog setTitle:(NSString *)aTitle];

	[pPool release];
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
