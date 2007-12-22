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
 *  Copyright 2006 Planamesa Inc.
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

@interface ShowFileDialogArgs : NSObject
{
	NSArray*				mpArgs;
	NSObject*				mpResult;
}
- (NSArray *)args;
- (void)dealloc;
- (id)initWithArgs:(NSArray *)pArgs;
- (NSObject *)result;
- (void)setResult:(NSObject *)pResult;
@end

@implementation ShowFileDialogArgs

- (NSArray *)args
{
	return mpArgs;
}

- (void)dealloc
{
	if ( mpArgs )
		[mpArgs release];

	if ( mpResult )
		[mpResult release];

	[super dealloc];
}

- (id)initWithArgs:(NSArray *)pArgs
{
	[super init];

	mpResult = nil;
	mpArgs = pArgs;
	if ( mpArgs )
		[mpArgs retain];

	return self;
}

- (NSObject *)result
{
	return mpResult;
}

- (void)setResult:(NSObject *)pResult
{
	if ( mpResult )
		[mpResult release];

	mpResult = pResult;

	if ( mpResult )
		[mpResult retain];
}

@end

@interface ShowFileDialog : NSObject
{
	BOOL					mbChooseFiles;
	NSMutableDictionary*	mpControls;
	NSString*				mpDefaultName;
	NSSavePanel*			mpFilePanel;
	NSMutableDictionary*	mpFilters;
	void*					mpPicker;
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
- (void)addFilter:(ShowFileDialogArgs *)pArgs;
- (void)addItem:(ShowFileDialogArgs *)pArgs;
- (void)cancel:(id)pObject;
- (void)dealloc;
- (void)deleteItem:(ShowFileDialogArgs *)pArgs;
- (NSString *)directory;
- (NSArray *)filenames;
- (NSArray *)items:(int)nID;
- (id)initWithPicker:(void *)pPicker useFileOpenDialog:(BOOL)bUseFileOpenDialog chooseFiles:(BOOL)bChooseFiles showAutoExtension:(BOOL)bShowAutoExtension showFilterOptions:(BOOL)bShowFilterOptions showImageTemplate:(BOOL)bShowImageTemplate showLink:(BOOL)bShowLink showPassword:(BOOL)bShowPassword showReadOnly:(BOOL)bShowReadOnly showSelection:(BOOL)bShowSelection showTemplate:(BOOL)bShowTemplate showVersion:(BOOL)bShowVersion;
- (void)initialize:(id)pObject;
- (BOOL)isChecked:(int)nID;
- (NSString *)label:(int)nID;
- (NSSavePanel *)panel;
- (BOOL)panel:(id)pObject shouldShowFilename:(NSString *)pFilename;
- (void *)picker;
- (void)release:(id)pObject;
- (int)result;
- (NSString *)selectedItem:(int)nID;
- (int)selectedItemIndex:(int)nID;
- (NSString *)selectedFilter;
- (void)setChecked:(ShowFileDialogArgs *)pArgs;
- (void)setDefaultName:(ShowFileDialogArgs *)pArgs;
- (void)setDirectory:(ShowFileDialogArgs *)pArgs;
- (void)setEnabled:(ShowFileDialogArgs *)pArgs;
- (void)setLabel:(ShowFileDialogArgs *)pArgs;
- (void)setMultiSelectionMode:(ShowFileDialogArgs *)pArgs;
- (void)setSelectedFilter:(ShowFileDialogArgs *)pArgs;
- (void)setSelectedItem:(ShowFileDialogArgs *)pArgs;
- (void)setTitle:(ShowFileDialogArgs *)pArgs;
- (void)showFileDialog:(id)pObject;
@end

@interface ShowDialogPopUpButtonCell : NSPopUpButtonCell
{
	int						mnID;
	ShowFileDialog*			mpDialog;
}
- (void)dismissPopUp;
- (id)initWithShowFileDialog:(ShowFileDialog *)pDialog control:(int)nID;
@end

@implementation ShowFileDialog

- (void)addFilter:(ShowFileDialogArgs *)pArgs
{
	NSArray *pArgArray = [pArgs args];
	if ( !pArgArray || [pArgArray count] < 2 )
		return;

	NSString *pItem = (NSString *)[pArgArray objectAtIndex:0];
	if ( !pItem )
		return;

	NSString *pFilter = (NSString *)[pArgArray objectAtIndex:0];
	if ( !pFilter )
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
				NSString *pCurrentFilter = [(NSString *)[pArray objectAtIndex:i] pathExtension];
				if ( !pCurrentFilter || ![pCurrentFilter length] || [pCurrentFilter isEqualToString:@"*"] )
				{
					bAllowAll = YES;
					break;
				}

				[pArray replaceObjectAtIndex:i withObject:pCurrentFilter];
			}

			if ( !bAllowAll )
				[mpFilters setObject:pArray forKey:pItem];
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

- (void)addItem:(ShowFileDialogArgs *)pArgs
{
	NSArray *pArgArray = [pArgs args];
	if ( !pArgArray || [pArgArray count] < 2 )
		return;

	NSNumber *pID = (NSNumber *)[pArgArray objectAtIndex:0];
	if ( !pID )
		return;

	NSString *pItem = (NSString *)[pArgArray objectAtIndex:1];
	if ( !pItem )
		return;

	int nID = [pID intValue];

	if ( !pItem || nID == COCOA_CONTROL_ID_FILETYPE )
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

- (void)cancel:(id)pObject;
{
	[mpFilePanel cancel:pObject];
}

- (void)dealloc
{
	if ( mpControls )
		[mpControls release];

	if ( mpDefaultName )
		[mpDefaultName release];

	if ( mpFilePanel )
	{
		[mpFilePanel setDelegate:nil];
		[mpFilePanel release];
	}

	if ( mpFilters )
		[mpFilters release];

	if ( mpTextFields )
		[mpTextFields release];

	[super dealloc];
}

- (void)deleteItem:(ShowFileDialogArgs *)pArgs
{
	NSArray *pArgArray = [pArgs args];
	if ( !pArgArray || [pArgArray count] < 2 )
		return;

	NSNumber *pID = (NSNumber *)[pArgArray objectAtIndex:0];
	if ( !pID )
		return;

	NSString *pItem = (NSString *)[pArgArray objectAtIndex:1];
	if ( !pItem )
		return;

	int nID = [pID intValue];

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
	NSMutableArray *pRet = nil;

	if ( NSFileDialog_controlType( nID ) == COCOA_CONTROL_TYPE_CHECKBOX )
	{
		NSPopUpButton *pPopup = (NSPopUpButton *)[mpControls objectForKey:[[NSNumber numberWithInt:nID] stringValue]];
		if ( pPopup )
		{
			int nCount = [pPopup numberOfItems];
			if ( nCount )
			{
				pRet = [[NSMutableArray alloc] initWithCapacity:nCount];
				if ( pRet )
				{
					int i = 0;
					for ( ; i < nCount; i++ )
					{
						NSString *pTitle = (NSString *)[pPopup itemTitleAtIndex:i];
						if ( pTitle )
							[pRet addObject:[pPopup itemTitleAtIndex:i]];
					}
				}
			}
		}
	}

	return pRet;
}

- (id)initWithPicker:(void *)pPicker useFileOpenDialog:(BOOL)bUseFileOpenDialog chooseFiles:(BOOL)bChooseFiles showAutoExtension:(BOOL)bShowAutoExtension showFilterOptions:(BOOL)bShowFilterOptions showImageTemplate:(BOOL)bShowImageTemplate showLink:(BOOL)bShowLink showPassword:(BOOL)bShowPassword showReadOnly:(BOOL)bShowReadOnly showSelection:(BOOL)bShowSelection showTemplate:(BOOL)bShowTemplate showVersion:(BOOL)bShowVersion
{
	[super init];

	mbChooseFiles = bChooseFiles;
	mpDefaultName = nil;
	mpPicker = pPicker;
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

	mpFilters = [[NSMutableDictionary alloc] init];
	if ( mpFilters )
		[mpFilters retain];

	mpTextFields = [[NSMutableDictionary alloc] init];
	if ( mpTextFields )
		[mpTextFields retain];

	return self;
}

- (void)initialize:(id)pObject
{
	if ( mbUseFileOpenDialog )
		mpFilePanel = (NSSavePanel *)[NSOpenPanel openPanel];
	else
		mpFilePanel = [NSSavePanel savePanel];

	if ( !mpFilePanel )
		return;

	[mpFilePanel retain];

	[mpFilePanel setDelegate:self];
	[mpFilePanel setCanCreateDirectories:YES];
	[mpFilePanel setCanSelectHiddenExtension:mbShowAutoExtension];

	if ( mbUseFileOpenDialog )
	{
		NSOpenPanel *pOpenPanel = (NSOpenPanel *)mpFilePanel;

		[pOpenPanel setCanChooseFiles:mbChooseFiles];
		[pOpenPanel setTreatsFilePackagesAsDirectories:NO];

		if ( mbChooseFiles )
			[pOpenPanel setCanChooseDirectories:NO];
		else
			[pOpenPanel setCanChooseDirectories:YES];
	}
	else
	{
		[mpFilePanel setTreatsFilePackagesAsDirectories:NO];
	}

	// Create file type popup
	if ( mbChooseFiles )
	{
		NSPopUpButton *pPopup = [[NSPopUpButton alloc] initWithFrame:NSMakeRect( 0, 0, 0, 0 ) pullsDown:NO];
		if ( pPopup )
		{
			// Swap in our own custom cell instance to handle selection changes
			ShowDialogPopUpButtonCell *pCell = [[ShowDialogPopUpButtonCell alloc] initWithShowFileDialog:self control:COCOA_CONTROL_ID_FILETYPE];
			if ( pCell )
				[pPopup setCell:pCell];

			[pPopup addItemWithTitle:pBlankItem];
			[mpControls setValue:pPopup forKey:[[NSNumber numberWithInt:COCOA_CONTROL_ID_FILETYPE] stringValue]];
		}

		NSTextField *pTextField = [[NSTextField alloc] initWithFrame:NSMakeRect( 0, 0, 1000, 0 )];
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
		NSPopUpButton *pPopup = [[NSPopUpButton alloc] initWithFrame:NSMakeRect( 0, 0, 0, 0 ) pullsDown:NO];
		if ( pPopup )
		{
			[pPopup addItemWithTitle:pBlankItem];
			[mpControls setValue:pPopup forKey:[[NSNumber numberWithInt:COCOA_CONTROL_ID_IMAGE_TEMPLATE] stringValue]];
		}

		NSTextField *pTextField = [[NSTextField alloc] initWithFrame:NSMakeRect( 0, 0, 1000, 0 )];
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
	if ( mbShowVersion )
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

- (NSSavePanel *)panel
{
	return mpFilePanel;
}

- (BOOL)panel:(id)pObject shouldShowFilename:(NSString *)pFilename
{
	BOOL bRet = NO;

	// Fix bug 1622 by checking for nil argument
	if ( !pFilename )
		return bRet;

	CFURLRef aURL = CFURLCreateWithFileSystemPath( NULL, (CFStringRef)pFilename, kCFURLPOSIXPathStyle, NO );
	if ( aURL )
	{
		FSRef aFSRef;
		if ( CFURLGetFSRef( aURL, &aFSRef ) )
		{
			Boolean bFolder;
			Boolean bAliased;
			if ( FSResolveAliasFileWithMountFlags( &aFSRef, YES, &bFolder, &bAliased, kResolveAliasFileNoUI ) == noErr )
			{
				if ( bFolder )
				{
					bRet = YES;
				}
				else if ( mbChooseFiles )
				{
					CFURLRef aResolvedURL = CFURLCreateFromFSRef( NULL, &aFSRef );
					if ( aResolvedURL )
					{
						NSString *pResolvedPath = (NSString *)CFURLCopyFileSystemPath( aResolvedURL, kCFURLPOSIXPathStyle );
						if ( pResolvedPath )
						{
							NSString *pItem = [self selectedFilter];
							if ( pItem )
							{
								NSArray *pArray = (NSArray *)[mpFilters objectForKey:pItem];
								if ( pArray )
								{
									NSString *pExt = (NSString *)[pResolvedPath pathExtension];
									if ( pExt )
									{
										int nCount = [pArray count];
										int i = 0;
										for ( ; i < nCount; i++ )
										{
											NSString *pCurrentType = (NSString *)[pArray objectAtIndex:i];
											if ( pCurrentType && ( [pCurrentType isEqualToString:@"*"] || [pCurrentType caseInsensitiveCompare:pExt] == NSOrderedSame ) )
											{
												bRet = YES;
												break;
											}
										}
									}
								}
								else
								{
									bRet = YES;
								}
							}
							else
							{
								bRet = YES;
							}
						}
					}
				}
			}
		}
	}

	return bRet;
}

- (void *)picker
{
	return mpPicker;
}

- (void)release:(id)pObject
{
	[self release];
}

- (int)result;
{
	return mnResult;
}

- (NSString *)selectedItem:(int)nID
{
	NSString *pRet = nil;

	if ( NSFileDialog_controlType( nID ) == COCOA_CONTROL_TYPE_POPUP )
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

	if ( NSFileDialog_controlType( nID ) == COCOA_CONTROL_TYPE_POPUP )
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

- (void)setChecked:(ShowFileDialogArgs *)pArgs
{
	NSArray *pArgArray = [pArgs args];
	if ( !pArgArray || [pArgArray count] < 2 )
		return;

	NSNumber *pID = (NSNumber *)[pArgArray objectAtIndex:0];
	if ( !pID )
		return;

	NSNumber *pChecked = (NSNumber *)[pArgArray objectAtIndex:1];
	if ( !pChecked )
		return;

	int nID = [pID intValue];
	int bChecked = [pChecked boolValue];

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

- (void)setDefaultName:(ShowFileDialogArgs *)pArgs
{
	if ( mpDefaultName )
	{
		[mpDefaultName release];
		mpDefaultName = nil;
	}

	NSArray *pArgArray = [pArgs args];
	if ( !pArgArray || [pArgArray count] < 1 )
		return;

	NSString *pName = (NSString *)[pArgArray objectAtIndex:0];
	if ( !pName )
		return;

	if ( !mbUseFileOpenDialog )
	{
		NSString *pExt = [pName pathExtension];
		if ( pExt && [pExt length] )
			pName = [pName substringToIndex:[pName length] - [pExt length] - 1];
	}

	[pName retain];
	mpDefaultName = pName;
}

- (void)setDirectory:(ShowFileDialogArgs *)pArgs
{
	NSArray *pArgArray = [pArgs args];
	if ( !pArgArray || [pArgArray count] < 1 )
		return;

	NSString *pDirectory = (NSString *)[pArgArray objectAtIndex:0];
	if ( !pDirectory )
		return;

	[mpFilePanel setDirectory:pDirectory];
}

- (void)setEnabled:(ShowFileDialogArgs *)pArgs
{
	NSArray *pArgArray = [pArgs args];
	if ( !pArgArray || [pArgArray count] < 2 )
		return;

	NSNumber *pID = (NSNumber *)[pArgArray objectAtIndex:0];
	if ( !pID )
		return;

	NSNumber *pEnabled = (NSNumber *)[pArgArray objectAtIndex:1];
	if ( !pEnabled )
		return;

	int nID = [pID intValue];
	int bEnabled = [pEnabled boolValue];

	NSControl *pControl = (NSControl *)[mpControls objectForKey:[[NSNumber numberWithInt:nID] stringValue]];
	if ( pControl )
		[pControl setEnabled:bEnabled];
}

- (void)setLabel:(ShowFileDialogArgs *)pArgs
{
	NSArray *pArgArray = [pArgs args];
	if ( !pArgArray || [pArgArray count] < 2 )
		return;

	NSNumber *pID = (NSNumber *)[pArgArray objectAtIndex:0];
	if ( !pID )
		return;

	NSString *pLabel = (NSString *)[pArgArray objectAtIndex:1];
	if ( !pLabel )
		return;

	int nID = [pID intValue];

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

- (void)setMultiSelectionMode:(ShowFileDialogArgs *)pArgs
{
	if ( !mbUseFileOpenDialog )
		return;

	NSArray *pArgArray = [pArgs args];
	if ( !pArgArray || [pArgArray count] < 1 )
		return;

	NSNumber *pMode = (NSNumber *)[pArgArray objectAtIndex:0];
	if ( !pMode )
		return;
	
	[(NSOpenPanel *)mpFilePanel setAllowsMultipleSelection:[pMode boolValue]];
}

- (void)setSelectedFilter:(ShowFileDialogArgs *)pArgs
{
	NSArray *pArgArray = [pArgs args];
	if ( !pArgArray || [pArgArray count] < 1 )
		return;

	NSString *pItem = (NSString *)[pArgArray objectAtIndex:0];
	if ( !pItem )
		return;

	if ( !mbUseFileOpenDialog )
		[mpFilePanel setAllowedFileTypes:(NSArray *)[mpFilters objectForKey:pItem]];

	[mpFilePanel validateVisibleColumns];

	NSPopUpButton *pPopup = (NSPopUpButton *)[mpControls objectForKey:[[NSNumber numberWithInt:COCOA_CONTROL_ID_FILETYPE] stringValue]];
	if ( pPopup )
	{
		[pPopup selectItemWithTitle:pItem];

		// OOo sometimes passes substrings of the full title
		if ( pItem && ![pPopup titleOfSelectedItem] )
		{
			int nCount = [pPopup numberOfItems];
			int i = 0;
			for ( ; i < nCount; i++ )
			{
				NSString *pTitle = [pPopup itemTitleAtIndex:i];
				if ( pTitle )
				{
					NSRange aRange = [pTitle rangeOfString:pItem];
					if ( aRange.location == 0 )
					{
						[pPopup selectItemAtIndex:i];
						break;
					}
				}
			}
		}
	}
}

- (void)setSelectedItem:(ShowFileDialogArgs *)pArgs
{
	NSArray *pArgArray = [pArgs args];
	if ( !pArgArray || [pArgArray count] < 2 )
		return;

	NSNumber *pID = (NSNumber *)[pArgArray objectAtIndex:0];
	if ( !pID )
		return;

	NSNumber *pItem = (NSNumber *)[pArgArray objectAtIndex:1];
	if ( !pItem )
		return;

	int nID = [pID intValue];
	int nItem = [pItem intValue];

	if ( nID == COCOA_CONTROL_ID_FILETYPE )
		return;

	if ( NSFileDialog_controlType( nID ) == COCOA_CONTROL_TYPE_POPUP )
	{
		NSPopUpButton *pPopup = (NSPopUpButton *)[mpControls objectForKey:[[NSNumber numberWithInt:nID] stringValue]];
		if ( pPopup )
			[pPopup selectItemAtIndex:nItem];
	}
}

- (void)setTitle:(ShowFileDialogArgs *)pArgs
{
}

- (void)showFileDialog:(id)pObject;
{
	if ( [mpFilePanel isVisible] )
		return;

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
				int nControlType = NSFileDialog_controlType( i );
				if ( nControlType == COCOA_CONTROL_TYPE_POPUP )
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
				if ( nControlType == COCOA_CONTROL_TYPE_POPUP )
					[pControl setFrameSize:NSMakeSize( 200, [pControl frame].size.height )];
					
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
				}

				if ( nHeight < nTextHeight )
					nCurrentY += nTextHeight;
				else
					nCurrentY += nHeight;
			}
		}

		// Center controls in view
		for ( i = 0; i < MAX_COCOA_CONTROL_ID; i++ )
		{
			NSControl *pControl = (NSControl *)[mpControls objectForKey:[[NSNumber numberWithInt:i] stringValue]];
			if ( pControl )
			{
				float nTextWidth = 0;
				NSTextField *pTextField = nil;
				if ( NSFileDialog_controlType( i ) == COCOA_CONTROL_TYPE_POPUP )
				{
					pTextField = (NSTextField *)[mpTextFields objectForKey:[[NSNumber numberWithInt:i] stringValue]];
					if ( pTextField )
						nTextWidth = [pTextField bounds].size.width;
				}

				float nAdjustWidth = ( nCurrentWidth - nTextWidth - [pControl bounds].size.width ) / 2;

				NSRect aBounds;
				if ( pTextField )
				{
					aBounds = [pTextField frame];
					[pTextField setFrameOrigin:NSMakePoint( aBounds.origin.x + nAdjustWidth, aBounds.origin.y )];
					[pAccessoryView addSubview:pTextField];
				}

				aBounds = [pControl frame];
				[pControl setFrameOrigin:NSMakePoint( aBounds.origin.x + nAdjustWidth, aBounds.origin.y )];
				[pAccessoryView addSubview:pControl];
			}
		}

		NSArray *pSubviews = [pAccessoryView subviews];
		if ( pSubviews && [pSubviews count] )
		{
			[pAccessoryView setFrameSize:NSMakeSize( nCurrentWidth, nCurrentY )];
			[mpFilePanel setAccessoryView:pAccessoryView];
		}

		// Fix bug 2302 by updating filtering
		ShowFileDialogArgs *pArgs = [[ShowFileDialogArgs alloc] initWithArgs:[NSArray arrayWithObject:[self selectedFilter]]];
		[self setSelectedFilter:pArgs];

		if ( mbUseFileOpenDialog )
		{
			NSOpenPanel *pOpenPanel = (NSOpenPanel *)mpFilePanel;
			mnResult = [pOpenPanel runModalForDirectory:[pOpenPanel directory] file:mpDefaultName types:nil];
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

@implementation ShowDialogPopUpButtonCell

- (void)dismissPopUp
{
	[super dismissPopUp];

	if ( mpDialog )
	{
		void *pPicker = [mpDialog picker];
		if ( pPicker )
			JavaFilePicker_controlStateChanged( mnID, pPicker );

		// Update filtering
		ShowFileDialogArgs *pArgs = [[ShowFileDialogArgs alloc] initWithArgs:[NSArray arrayWithObject:[mpDialog selectedFilter]]];
		[mpDialog setSelectedFilter:pArgs];
	}
}

- (id)initWithShowFileDialog:(ShowFileDialog *)pDialog control:(int)nID
{
	[super initTextCell:pBlankItem pullsDown:NO];

	mnID = nID;
	mpDialog = pDialog;

	return self;
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

	if ( pDialog && aItem && aFilter )
	{
		ShowFileDialogArgs *pArgs = [[ShowFileDialogArgs alloc] initWithArgs:[NSArray arrayWithObjects:(NSString *)aItem, (NSString *)aFilter, nil]];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[(ShowFileDialog *)pDialog performSelectorOnMainThread:@selector(addFilter:) withObject:pArgs waitUntilDone:NO modes:pModes];
	}

	[pPool release];
}

void NSFileDialog_addItem( id pDialog, int nID, CFStringRef aItem )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pDialog && aItem )
	{
		ShowFileDialogArgs *pArgs = [[ShowFileDialogArgs alloc] initWithArgs:[NSArray arrayWithObjects:[NSNumber numberWithInt:nID], (NSString *)aItem, nil]];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[(ShowFileDialog *)pDialog performSelectorOnMainThread:@selector(addItem:) withObject:pArgs waitUntilDone:NO modes:pModes];
	}

	[pPool release];
}

void NSFileDialog_cancel( id pDialog )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pDialog )
	{
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[(ShowFileDialog *)pDialog performSelectorOnMainThread:@selector(cancel:) withObject:pDialog waitUntilDone:YES modes:pModes];
	}

	[pPool release];
}

id NSFileDialog_create( void *pPicker, BOOL bUseFileOpenDialog, BOOL bChooseFiles, BOOL bShowAutoExtension, BOOL bShowFilterOptions, BOOL bShowImageTemplate, BOOL bShowLink, BOOL bShowPassword, BOOL bShowReadOnly, BOOL bShowSelection, BOOL bShowTemplate, BOOL bShowVersion )
{
	ShowFileDialog *pRet = nil;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	pRet = [[ShowFileDialog alloc] initWithPicker:pPicker useFileOpenDialog:bUseFileOpenDialog chooseFiles:bChooseFiles showAutoExtension:bShowAutoExtension showFilterOptions:bShowFilterOptions showImageTemplate:bShowImageTemplate showLink:bShowLink showPassword:bShowPassword showReadOnly:bShowReadOnly showSelection:bShowSelection showTemplate:bShowTemplate showVersion:bShowVersion];
	if ( pRet )
	{
		// Fix bug 1601 by ensuring the first save and open panels are created
		// on the main thread
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[(ShowFileDialog *)pRet performSelectorOnMainThread:@selector(initialize:) withObject:pRet waitUntilDone:YES modes:pModes];
	}

	[pPool release];

	return pRet;
}

void NSFileDialog_deleteItem( id pDialog, int nID, CFStringRef aItem )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pDialog && aItem )
	{
		ShowFileDialogArgs *pArgs = [[ShowFileDialogArgs alloc] initWithArgs:[NSArray arrayWithObjects:[NSNumber numberWithInt:nID], (NSString *)aItem, nil]];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[(ShowFileDialog *)pDialog performSelectorOnMainThread:@selector(deleteItem:) withObject:pArgs waitUntilDone:NO modes:pModes];
	}

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
	{
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[(ShowFileDialog *)pDialog performSelectorOnMainThread:@selector(release:) withObject:pDialog waitUntilDone:NO modes:pModes];
	}

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
	{
		ShowFileDialogArgs *pArgs = [[ShowFileDialogArgs alloc] initWithArgs:[NSArray arrayWithObjects:[NSNumber numberWithInt:nID], [NSNumber numberWithBool:bChecked], nil]];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[(ShowFileDialog *)pDialog performSelectorOnMainThread:@selector(setChecked:) withObject:pArgs waitUntilDone:NO modes:pModes];
	}

	[pPool release];
}

void NSFileDialog_setDefaultName( id pDialog, CFStringRef aName )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pDialog && aName )
	{
		ShowFileDialogArgs *pArgs = [[ShowFileDialogArgs alloc] initWithArgs:[NSArray arrayWithObject:(NSString *)aName]];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[(ShowFileDialog *)pDialog performSelectorOnMainThread:@selector(setDefaultName:) withObject:pArgs waitUntilDone:NO modes:pModes];
	}

	[pPool release];
}

void NSFileDialog_setDirectory( id pDialog, CFStringRef aDirectory )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pDialog && aDirectory )
	{
		ShowFileDialogArgs *pArgs = [[ShowFileDialogArgs alloc] initWithArgs:[NSArray arrayWithObject:(NSString *)aDirectory]];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[(ShowFileDialog *)pDialog performSelectorOnMainThread:@selector(setDirectory:) withObject:pArgs waitUntilDone:NO modes:pModes];
	}

	[pPool release];
}

void NSFileDialog_setEnabled( id pDialog, int nID, BOOL bEnabled )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pDialog )
	{
		ShowFileDialogArgs *pArgs = [[ShowFileDialogArgs alloc] initWithArgs:[NSArray arrayWithObjects:[NSNumber numberWithInt:nID], [NSNumber numberWithBool:bEnabled], nil]];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[(ShowFileDialog *)pDialog performSelectorOnMainThread:@selector(setEnabled:) withObject:pArgs waitUntilDone:NO modes:pModes];
	}

	[pPool release];
}

void NSFileDialog_setLabel( id pDialog, int nID, CFStringRef aLabel )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pDialog && aLabel )
	{
		ShowFileDialogArgs *pArgs = [[ShowFileDialogArgs alloc] initWithArgs:[NSArray arrayWithObjects:[NSNumber numberWithInt:nID], (NSString *)aLabel, nil]];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[(ShowFileDialog *)pDialog performSelectorOnMainThread:@selector(setLabel:) withObject:pArgs waitUntilDone:NO modes:pModes];
	}

	[pPool release];
}

void NSFileDialog_setMultiSelectionMode( id pDialog, BOOL bMultiSelectionMode )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pDialog )
	{
		ShowFileDialogArgs *pArgs = [[ShowFileDialogArgs alloc] initWithArgs:[NSArray arrayWithObject:[NSNumber numberWithBool:bMultiSelectionMode]]];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[(ShowFileDialog *)pDialog performSelectorOnMainThread:@selector(setMultiSelectionMode:) withObject:pArgs waitUntilDone:NO modes:pModes];
	}

	[pPool release];
}

void NSFileDialog_setSelectedFilter( id pDialog, CFStringRef aItem )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pDialog && aItem )
	{
		ShowFileDialogArgs *pArgs = [[ShowFileDialogArgs alloc] initWithArgs:[NSArray arrayWithObject:(NSString *)aItem]];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[(ShowFileDialog *)pDialog performSelectorOnMainThread:@selector(setSelectedFilter:) withObject:pArgs waitUntilDone:NO modes:pModes];
	}

	[pPool release];
}

void NSFileDialog_setSelectedItem( id pDialog, int nID, int nItem )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pDialog )
	{
		ShowFileDialogArgs *pArgs = [[ShowFileDialogArgs alloc] initWithArgs:[NSArray arrayWithObjects:[NSNumber numberWithInt:nID], [NSNumber numberWithInt:nItem], nil]];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[(ShowFileDialog *)pDialog performSelectorOnMainThread:@selector(setSelectedItem:) withObject:pArgs waitUntilDone:NO modes:pModes];
	}

	[pPool release];
}

void NSFileDialog_setTitle( id pDialog, CFStringRef aTitle )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pDialog && aTitle )
	{
		ShowFileDialogArgs *pArgs = [[ShowFileDialogArgs alloc] initWithArgs:[NSArray arrayWithObject:(NSString *)aTitle]];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[(ShowFileDialog *)pDialog performSelectorOnMainThread:@selector(setTitle:) withObject:pArgs waitUntilDone:NO modes:pModes];
	}

	[pPool release];
}

int NSFileDialog_showFileDialog( id pDialog )
{
	int nRet = NSCancelButton;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pDialog )
	{
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[(ShowFileDialog *)pDialog performSelectorOnMainThread:@selector(showFileDialog:) withObject:pDialog waitUntilDone:YES modes:pModes];
		nRet = [(ShowFileDialog *)pDialog result];
	}

	[pPool release];

	return nRet;
}
