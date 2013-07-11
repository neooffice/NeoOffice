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

#import <dlfcn.h>

#import <Cocoa/Cocoa.h>

#ifndef _COCOA_FILEDIALOG_H_
#import "cocoa_dialog.h"
#endif

typedef void Application_cacheSecurityScopedURL_Type( id pURL );

static Application_cacheSecurityScopedURL_Type *pApplication_cacheSecurityScopedURL = NULL;

static NSString *pBlankItem = @" ";

@interface ShowFileDialogArgs : NSObject
{
	NSArray*				mpArgs;
	NSObject*				mpResult;
}
+ (id)argsWithArgs:(NSArray *)pArgs;
- (NSArray *)args;
- (void)dealloc;
- (id)initWithArgs:(NSArray *)pArgs;
- (NSObject *)result;
- (void)setResult:(NSObject *)pResult;
@end

@implementation ShowFileDialogArgs

+ (id)argsWithArgs:(NSArray *)pArgs
{
	ShowFileDialogArgs *pRet = [[ShowFileDialogArgs alloc] initWithArgs:(NSArray *)pArgs];
	[pRet autorelease];
	return pRet;
}

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
	BOOL					mbInShowFileDialog;
	void*					mpPicker;
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
- (void)destroy:(id)pObject;
- (NSURL *)directory:(ShowFileDialogArgs *)pArgs;
- (NSArray *)URLs:(ShowFileDialogArgs *)pArgs;
- (NSArray *)items:(ShowFileDialogArgs *)pArgs;
- (id)initWithPicker:(void *)pPicker useFileOpenDialog:(BOOL)bUseFileOpenDialog chooseFiles:(BOOL)bChooseFiles showAutoExtension:(BOOL)bShowAutoExtension showFilterOptions:(BOOL)bShowFilterOptions showImageTemplate:(BOOL)bShowImageTemplate showLink:(BOOL)bShowLink showPassword:(BOOL)bShowPassword showReadOnly:(BOOL)bShowReadOnly showSelection:(BOOL)bShowSelection showTemplate:(BOOL)bShowTemplate showVersion:(BOOL)bShowVersion;
- (void)initialize:(id)pObject;
- (BOOL)isChecked:(ShowFileDialogArgs *)pArgs;
- (BOOL)isInShowFileDialog;
- (NSString *)label:(ShowFileDialogArgs *)pArgs;
- (NSSavePanel *)panel;
- (void)panel:(id)pObject didChangeToDirectoryURL:(NSURL *)pURL;
- (BOOL)panel:(id)pObject shouldEnableURL:(NSURL *)pURL;
- (void *)picker;
- (void)release:(id)pObject;
- (NSString *)selectedItem:(ShowFileDialogArgs *)pArgs;
- (int)selectedItemIndex:(ShowFileDialogArgs *)pArgs;
- (NSString *)selectedFilter:(ShowFileDialogArgs *)pArgs;
- (void)setChecked:(ShowFileDialogArgs *)pArgs;
- (void)setDefaultName:(ShowFileDialogArgs *)pArgs;
- (void)setDirectory:(ShowFileDialogArgs *)pArgs;
- (void)setEnabled:(ShowFileDialogArgs *)pArgs;
- (void)setLabel:(ShowFileDialogArgs *)pArgs;
- (void)setMultiSelectionMode:(ShowFileDialogArgs *)pArgs;
- (void)setSelectedFilter:(ShowFileDialogArgs *)pArgs;
- (void)setSelectedItem:(ShowFileDialogArgs *)pArgs;
- (void)setTitle:(ShowFileDialogArgs *)pArgs;
- (int)showFileDialog:(ShowFileDialogArgs *)pArgs;
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

	NSString *pFilter = (NSString *)[pArgArray objectAtIndex:1];
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
	[self destroy:self];

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

- (void)destroy:(id)pObject
{
	if ( mpControls )
	{
		[mpControls release];
		mpControls = nil;
	}

	if ( mpDefaultName )
	{
		[mpDefaultName release];
		mpDefaultName = nil;
	}

	if ( mpFilePanel )
	{
		[mpFilePanel setDelegate:nil];
		[mpFilePanel release];
		mpFilePanel = nil;
	}

	if ( mpFilters )
	{
		[mpFilters release];
		mpFilters = nil;
	}

	if ( mpTextFields )
	{
		[mpTextFields release];
		mpTextFields = nil;
	}
}

- (NSURL *)directory:(ShowFileDialogArgs *)pArgs
{
	NSURL *pRet = [mpFilePanel directoryURL];

	if ( pArgs )
		[pArgs setResult:pRet];

	return pRet;
}

- (NSArray *)URLs:(ShowFileDialogArgs *)pArgs
{
	NSArray *pRet = nil;

	if ( mbUseFileOpenDialog )
	{
		NSArray *pArray = [(NSOpenPanel *)mpFilePanel URLs];
		if ( pArray )
			pRet = [NSArray arrayWithArray:pArray];
	}
	else
	{
		NSURL *pURL = [mpFilePanel URL];
		// Fix bug 3662 by ensuring that the save panel does not append a
		// trailing "/" character
		if ( pURL && [pURL isFileURL] )
			pURL = [pURL URLByStandardizingPath];
		if ( pURL )
			pRet = [NSArray arrayWithObject:pURL];
	}

	if ( pArgs )
		[pArgs setResult:pRet];

	return pRet;
}

- (NSArray *)items:(ShowFileDialogArgs *)pArgs
{
	NSMutableArray *pRet = nil;

	NSArray *pArgArray = [pArgs args];
	if ( !pArgArray || [pArgArray count] < 1 )
		return pRet;

	NSNumber *pID = (NSNumber *)[pArgArray objectAtIndex:0];
	if ( !pID )
		return pRet;

	int nID = [pID intValue];

	if ( NSFileDialog_controlType( nID ) == COCOA_CONTROL_TYPE_POPUP )
	{
		NSPopUpButton *pPopup = (NSPopUpButton *)[mpControls objectForKey:[[NSNumber numberWithInt:nID] stringValue]];
		if ( pPopup )
		{
			int nCount = [pPopup numberOfItems];
			if ( nCount )
			{
				pRet = [NSMutableArray arrayWithCapacity:nCount];
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
	mbInShowFileDialog = NO;
	mpPicker = pPicker;
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

	mpControls = [NSMutableDictionary dictionary];
	if ( mpControls )
		[mpControls retain];

	mpFilters = [NSMutableDictionary dictionary];
	if ( mpFilters )
		[mpFilters retain];

	mpTextFields = [NSMutableDictionary dictionary];
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
			// Add to autorelease pool as invoking alloc disables autorelease
			[pPopup autorelease];

			// Swap in our own custom cell instance to handle selection changes
			ShowDialogPopUpButtonCell *pCell = [[ShowDialogPopUpButtonCell alloc] initWithShowFileDialog:self control:COCOA_CONTROL_ID_FILETYPE];
			if ( pCell )
			{
				// Add to autorelease pool as invoking alloc disables autorelease
				[pCell autorelease];
				[pPopup setCell:pCell];
			}

			[pPopup addItemWithTitle:pBlankItem];
			[mpControls setValue:pPopup forKey:[[NSNumber numberWithInt:COCOA_CONTROL_ID_FILETYPE] stringValue]];
		}

		NSTextField *pTextField = [[NSTextField alloc] initWithFrame:NSMakeRect( 0, 0, 1000, 0 )];
		if ( pTextField )
		{
			// Add to autorelease pool as invoking alloc disables autorelease
			[pTextField autorelease];

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
			// Add to autorelease pool as invoking alloc disables autorelease
			[pButton autorelease];

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
			// Add to autorelease pool as invoking alloc disables autorelease
			[pPopup autorelease];

			[pPopup addItemWithTitle:pBlankItem];
			[mpControls setValue:pPopup forKey:[[NSNumber numberWithInt:COCOA_CONTROL_ID_IMAGE_TEMPLATE] stringValue]];
		}

		NSTextField *pTextField = [[NSTextField alloc] initWithFrame:NSMakeRect( 0, 0, 1000, 0 )];
		if ( pTextField )
		{
			// Add to autorelease pool as invoking alloc disables autorelease
			[pTextField autorelease];

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
			// Add to autorelease pool as invoking alloc disables autorelease
			[pButton autorelease];

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
			// Add to autorelease pool as invoking alloc disables autorelease
			[pButton autorelease];

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
			// Add to autorelease pool as invoking alloc disables autorelease
			[pButton autorelease];

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
			// Add to autorelease pool as invoking alloc disables autorelease
			[pButton autorelease];

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
			// Add to autorelease pool as invoking alloc disables autorelease
			[pPopup autorelease];

			[pPopup addItemWithTitle:pBlankItem];
			[mpControls setValue:pPopup forKey:[[NSNumber numberWithInt:COCOA_CONTROL_ID_TEMPLATE] stringValue]];
		}

		NSTextField *pTextField = [[NSTextField alloc] initWithFrame:NSMakeRect( 0, 0, 1000, 0 )];
		if ( pTextField )
		{
			// Add to autorelease pool as invoking alloc disables autorelease
			[pTextField autorelease];

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
			// Add to autorelease pool as invoking alloc disables autorelease
			[pPopup autorelease];

			[pPopup addItemWithTitle:pBlankItem];
			[mpControls setValue:pPopup forKey:[[NSNumber numberWithInt:COCOA_CONTROL_ID_VERSION] stringValue]];
		}

		NSTextField *pTextField = [[NSTextField alloc] initWithFrame:NSMakeRect( 0, 0, 1000, 0 )];
		if ( pTextField )
		{
			// Add to autorelease pool as invoking alloc disables autorelease
			[pTextField autorelease];

			[pTextField setBordered:NO];
			[pTextField setDrawsBackground:NO];
			[pTextField setEditable:NO];
			[pTextField setSelectable:NO];
			[mpTextFields setValue:pTextField forKey:[[NSNumber numberWithInt:COCOA_CONTROL_ID_VERSION] stringValue]];
		}
	}
}

- (BOOL)isChecked:(ShowFileDialogArgs *)pArgs
{
	BOOL bRet = NO;

	NSArray *pArgArray = [pArgs args];
	if ( !pArgArray || [pArgArray count] < 1 )
		return bRet;

	NSNumber *pID = (NSNumber *)[pArgArray objectAtIndex:0];
	if ( !pID )
		return bRet;

	int nID = [pID intValue];

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

	[pArgs setResult:[NSNumber numberWithBool:bRet]];

	return bRet;
}

- (BOOL)isInShowFileDialog
{
	return mbInShowFileDialog;
}

- (NSString *)label:(ShowFileDialogArgs *)pArgs
{
	NSString *pRet = nil;

	NSArray *pArgArray = [pArgs args];
	if ( !pArgArray || [pArgArray count] < 1 )
		return pRet;

	NSNumber *pID = (NSNumber *)[pArgArray objectAtIndex:0];
	if ( !pID )
		return pRet;

	int nID = [pID intValue];

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

	[pArgs setResult:pRet];

	return pRet;
}

- (NSSavePanel *)panel
{
	return mpFilePanel;
}

- (void)panel:(id)pObject didChangeToDirectoryURL:(NSURL *)pURL
{
	// Fix bug 3568 by forcefully setting the directory when it has been
	// changed by the user. Note that we only do this if the file URL is
	// different than the panel's directory URL or else on Mac OS X 10.7 the
	// file list will go into a infinite repainting loop.
	if ( pURL && [pURL isFileURL] )
	{
		NSURL *pCurrentURL = [mpFilePanel directoryURL];
		if ( !pCurrentURL || ![pURL isEqual:pCurrentURL] )
			[mpFilePanel setDirectoryURL:pURL];
	}
}

- (BOOL)panel:(id)pObject shouldEnableURL:(NSURL *)pURL
{
	BOOL bRet = NO;

	// Fix bug 1622 by checking for nil argument
	if ( !pURL )
		return bRet;

	if ( [pURL isFileURL] )
	{
		NSNumber *pAlias = nil;
		while ( pURL && [pURL checkResourceIsReachableAndReturnError:nil] && [pURL getResourceValue:&pAlias forKey:NSURLIsAliasFileKey error:nil] && pAlias && [pAlias boolValue] )
		{
			// Resolve alias
			NSData *pData = [NSURL bookmarkDataWithContentsOfURL:pURL error:nil];
			pURL = nil;
			if ( pData )
			{
				BOOL bStale = NO;
				NSURL *pResolvedURL = [NSURL URLByResolvingBookmarkData:pData options:NSURLBookmarkResolutionWithoutUI | NSURLBookmarkResolutionWithoutMounting relativeToURL:nil bookmarkDataIsStale:&bStale error:nil];
				if ( !bStale && pResolvedURL )
					pURL = pResolvedURL;
			}
		}

		if ( pURL && [pURL isFileURL] )
		{
			NSNumber *pDir = nil;
			if ( [pURL getResourceValue:&pDir forKey:NSURLIsDirectoryKey error:nil] && pDir && [pDir boolValue] )
			{
				bRet = YES;
			}
			else if ( mbChooseFiles )
			{
				NSString *pResolvedPath = [pURL path];
				if ( pResolvedPath )
				{
					NSString *pItem = [self selectedFilter:nil];
					if ( pItem )
					{
						NSArray *pArray = (NSArray *)[mpFilters objectForKey:pItem];
						if ( pArray )
						{
							NSString *pExt = [pResolvedPath pathExtension];
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
	else
	{
		bRet = YES;
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

- (NSString *)selectedItem:(ShowFileDialogArgs *)pArgs
{
	NSString *pRet = nil;

	NSArray *pArgArray = [pArgs args];
	if ( !pArgArray || [pArgArray count] < 1 )
		return pRet;

	NSNumber *pID = (NSNumber *)[pArgArray objectAtIndex:0];
	if ( !pID )
		return pRet;

	int nID = [pID intValue];

	if ( NSFileDialog_controlType( nID ) == COCOA_CONTROL_TYPE_POPUP )
	{
		NSPopUpButton *pPopup = (NSPopUpButton *)[mpControls objectForKey:[[NSNumber numberWithInt:nID] stringValue]];
		if ( pPopup )
			pRet = [pPopup titleOfSelectedItem];
	}

	[pArgs setResult:pRet];

	return pRet;
}

- (int)selectedItemIndex:(ShowFileDialogArgs *)pArgs
{
	int nRet = 0;

	NSArray *pArgArray = [pArgs args];
	if ( !pArgArray || [pArgArray count] < 1 )
		return nRet;

	NSNumber *pID = (NSNumber *)[pArgArray objectAtIndex:0];
	if ( !pID )
		return nRet;

	int nID = [pID intValue];

	if ( NSFileDialog_controlType( nID ) == COCOA_CONTROL_TYPE_POPUP )
	{
		NSPopUpButton *pPopup = (NSPopUpButton *)[mpControls objectForKey:[[NSNumber numberWithInt:nID] stringValue]];
		if ( pPopup )
			nRet = [pPopup indexOfSelectedItem];
	}

	[pArgs setResult:[NSNumber numberWithInt:nRet]];

	return nRet;
}

- (NSString *)selectedFilter:(ShowFileDialogArgs *)pArgs
{
	NSString *pRet = nil;

	NSPopUpButton *pPopup = (NSPopUpButton *)[mpControls objectForKey:[[NSNumber numberWithInt:COCOA_CONTROL_ID_FILETYPE] stringValue]];
	if ( pPopup )
		pRet = [pPopup titleOfSelectedItem];

	if ( pArgs )
		[pArgs setResult:pRet];

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
	NSString *pOldDefaultName;
	if ( mpDefaultName )
	{
		pOldDefaultName = [NSString stringWithString:mpDefaultName];
		[mpDefaultName release];
		mpDefaultName = nil;
	}
	else
	{
		pOldDefaultName = nil;
	}

	NSArray *pArgArray = [pArgs args];
	if ( !pArgArray || [pArgArray count] < 1 )
		return;

	NSString *pName = (NSString *)[pArgArray objectAtIndex:0];
	if ( !pName )
		return;

	// Fix bug 2954 by only applying the fix for bug 1607 only if the path
	// extension has not been already stripped off by the OOo code
	if ( !mbUseFileOpenDialog && ( !pOldDefaultName || ![pName isEqualToString:pOldDefaultName] ) )
	{
		// Fix bug 1607 by stripping of any path extension
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

	NSURL *pURL = [NSURL URLWithString:pDirectory];
	if ( !pURL )
		return;

	// Fix bug 3568 by forcefully setting the directory when it has been
	// changed by the user
	if ( pURL && [pURL isFileURL] )
		[mpFilePanel setDirectoryURL:pURL];
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

- (int)showFileDialog:(ShowFileDialogArgs *)pArgs
{
	int nRet = NSCancelButton;

	if ( mbInShowFileDialog )
		return nRet;

	// When sandboxed the NSSavePanel does not have the isVisible selector so
    // use our internal object variable instead
	mbInShowFileDialog = YES;
    
	// Create accessory view
	NSView *pAccessoryView = [[NSView alloc] initWithFrame:NSMakeRect(0, 0, 0, 0)];
	if ( pAccessoryView )
	{
		// Add to autorelease pool as invoking alloc disables autorelease
		[pAccessoryView autorelease];

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
		NSString *pFilter = [self selectedFilter:nil];
		if ( pFilter )
		{
			ShowFileDialogArgs *pSelectedFilterArgs = [ShowFileDialogArgs argsWithArgs:[NSArray arrayWithObject:pFilter]];
			[self setSelectedFilter:pSelectedFilterArgs];
		}

		if ( mpDefaultName )
			[mpFilePanel setNameFieldStringValue:mpDefaultName];
		nRet = [mpFilePanel runModal];
		if ( nRet == NSFileHandlingPanelOKButton )
		{
			NSArray *pURLs = nil;
			if ( mbUseFileOpenDialog )
			{
				pURLs = [(NSOpenPanel *)mpFilePanel URLs];
			}
			else
			{
				NSURL *pSaveURL = [mpFilePanel URL];
				if ( pSaveURL )
					pURLs = [NSArray arrayWithObject:pSaveURL];
			}

			if ( pURLs )
			{
				NSUInteger nCount = [pURLs count];
				NSUInteger i = 0;
				for ( ; i < nCount; i++ )
				{
					if ( !pApplication_cacheSecurityScopedURL )
						pApplication_cacheSecurityScopedURL = (Application_cacheSecurityScopedURL_Type *)dlsym( RTLD_DEFAULT, "Application_cacheSecurityScopedURL" );
					if ( pApplication_cacheSecurityScopedURL )
						pApplication_cacheSecurityScopedURL( [pURLs objectAtIndex:i] );
				}
			}
		}


		[mpFilePanel setAccessoryView:pOldAccessoryView];

		for ( i = 0; i < MAX_COCOA_CONTROL_ID; i++ )
		{
			NSControl *pControl = (NSControl *)[mpControls objectForKey:[[NSNumber numberWithInt:i] stringValue]];
			if ( pControl )
				[pControl removeFromSuperview];
		}
	}

	if ( pArgs )
		[pArgs setResult:[NSNumber numberWithInt:nRet]];

	mbInShowFileDialog = NO;
    
	return nRet;
}

@end

@implementation ShowDialogPopUpButtonCell

- (void)dismissPopUp
{
	[super dismissPopUp];

	if ( mpDialog && [mpDialog isInShowFileDialog] )
	{
		void *pPicker = [mpDialog picker];
		if ( pPicker )
			JavaFilePicker_controlStateChanged( mnID, pPicker );

		// Update filtering
		NSString *pFilter = [mpDialog selectedFilter:nil];
		if ( pFilter )
		{
			ShowFileDialogArgs *pSelectedFilterArgs = [ShowFileDialogArgs argsWithArgs:[NSArray arrayWithObject:pFilter]];
			[mpDialog setSelectedFilter:pSelectedFilterArgs];
		}
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
		ShowFileDialogArgs *pArgs = [ShowFileDialogArgs argsWithArgs:[NSArray arrayWithObjects:(NSString *)aItem, (NSString *)aFilter, nil]];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[(ShowFileDialog *)pDialog performSelectorOnMainThread:@selector(addFilter:) withObject:pArgs waitUntilDone:YES modes:pModes];
	}

	[pPool release];
}

void NSFileDialog_addItem( id pDialog, int nID, CFStringRef aItem )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pDialog && aItem )
	{
		ShowFileDialogArgs *pArgs = [ShowFileDialogArgs argsWithArgs:[NSArray arrayWithObjects:[NSNumber numberWithInt:nID], (NSString *)aItem, nil]];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[(ShowFileDialog *)pDialog performSelectorOnMainThread:@selector(addItem:) withObject:pArgs waitUntilDone:YES modes:pModes];
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

	// Do not retain as invoking alloc disables autorelease
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
		ShowFileDialogArgs *pArgs = [ShowFileDialogArgs argsWithArgs:[NSArray arrayWithObjects:[NSNumber numberWithInt:nID], (NSString *)aItem, nil]];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[(ShowFileDialog *)pDialog performSelectorOnMainThread:@selector(deleteItem:) withObject:pArgs waitUntilDone:YES modes:pModes];
	}

	[pPool release];
}

CFStringRef NSFileDialog_directory( id pDialog )
{
	CFStringRef aRet = nil;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pDialog )
	{
		ShowFileDialogArgs *pArgs = [ShowFileDialogArgs argsWithArgs:nil];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[(ShowFileDialog *)pDialog performSelectorOnMainThread:@selector(directory:) withObject:pArgs waitUntilDone:YES modes:pModes];
		NSURL *pURL = (NSURL *)[pArgs result];
		if ( pURL )
		{
			NSString *pDirectory = [pURL absoluteString];
			if ( pDirectory )
			{
				[pDirectory retain];
				aRet = (CFStringRef)pDirectory;
			}
		}
	}

	[pPool release];

	return aRet;
}

CFStringRef *NSFileDialog_URLs( id pDialog )
{
	CFStringRef *pRet = nil;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pDialog )
	{
		ShowFileDialogArgs *pArgs = [ShowFileDialogArgs argsWithArgs:nil];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[(ShowFileDialog *)pDialog performSelectorOnMainThread:@selector(URLs:) withObject:pArgs waitUntilDone:YES modes:pModes];
		NSArray *pURLs = (NSArray *)[pArgs result];
		if ( pURLs )
		{
			unsigned nCount = [pURLs count];
			if ( nCount )
			{
				pRet = (CFStringRef *)malloc( ( nCount + 1 ) * sizeof( CFStringRef ) );
				if ( pRet )
				{
					unsigned nIndex = 0;
					unsigned i = 0;
					for ( ; i < nCount; i++ )
					{
						NSURL *pCurrentURL = (NSURL *)[pURLs objectAtIndex:i];
						if ( pCurrentURL )
						{
							NSString *pCurrentName = [pCurrentURL absoluteString];
							if ( pCurrentName )
							{
								[pCurrentName retain];
								pRet[ nIndex++ ] = (CFStringRef)pCurrentName;
							}
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
		ShowFileDialogArgs *pArgs = [ShowFileDialogArgs argsWithArgs:[NSArray arrayWithObject:[NSNumber numberWithInt:nID]]];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[(ShowFileDialog *)pDialog performSelectorOnMainThread:@selector(items:) withObject:pArgs waitUntilDone:YES modes:pModes];
		NSArray *pItems = (NSArray *)[pArgs result];
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
	{
		ShowFileDialogArgs *pArgs = [ShowFileDialogArgs argsWithArgs:[NSArray arrayWithObject:[NSNumber numberWithInt:nID]]];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[(ShowFileDialog *)pDialog performSelectorOnMainThread:@selector(isChecked:) withObject:pArgs waitUntilDone:YES modes:pModes];
		NSNumber *pRet = (NSNumber *)[pArgs result];
		if ( pRet )
			bRet = [pRet boolValue];
	}

	[pPool release];

	return bRet;
}

CFStringRef NSFileDialog_label( id pDialog, int nID )
{
	CFStringRef aRet = nil;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pDialog )
	{
		ShowFileDialogArgs *pArgs = [ShowFileDialogArgs argsWithArgs:[NSArray arrayWithObject:[NSNumber numberWithInt:nID]]];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[(ShowFileDialog *)pDialog performSelectorOnMainThread:@selector(label:) withObject:pArgs waitUntilDone:YES modes:pModes];
		NSString *pLabel = (NSString *)[pArgs result];
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
		[(ShowFileDialog *)pDialog performSelectorOnMainThread:@selector(destroy:) withObject:pDialog waitUntilDone:YES modes:pModes];
		[(ShowFileDialog *)pDialog release];
	}

	[pPool release];
}

void NSFileManager_releaseURLs( CFStringRef *pURLs )
{           
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pURLs )
	{
		unsigned nIndex = 0;
		for ( ; pURLs[ nIndex ]; nIndex++ )
			CFRelease( pURLs[ nIndex ] );
		free( pURLs );
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

CFStringRef NSFileDialog_selectedFilter( id pDialog )
{
	CFStringRef aRet = nil;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pDialog )
	{
		ShowFileDialogArgs *pArgs = [ShowFileDialogArgs argsWithArgs:nil];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[(ShowFileDialog *)pDialog performSelectorOnMainThread:@selector(selectedFilter:) withObject:pArgs waitUntilDone:YES modes:pModes];
		NSString *pItem = (NSString *)[pArgs result];
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
		ShowFileDialogArgs *pArgs = [ShowFileDialogArgs argsWithArgs:[NSArray arrayWithObject:[NSNumber numberWithInt:nID]]];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[(ShowFileDialog *)pDialog performSelectorOnMainThread:@selector(selectedItem:) withObject:pArgs waitUntilDone:YES modes:pModes];
		NSString *pItem = (NSString *)[pArgs result];
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
	{
		ShowFileDialogArgs *pArgs = [ShowFileDialogArgs argsWithArgs:[NSArray arrayWithObject:[NSNumber numberWithInt:nID]]];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[(ShowFileDialog *)pDialog performSelectorOnMainThread:@selector(selectedItemIndex:) withObject:pArgs waitUntilDone:YES modes:pModes];
		NSNumber *pRet = (NSNumber *)[pArgs result];
		if ( pRet )
			nRet = [pRet intValue];
	}

	[pPool release];

	return nRet;
}

void NSFileDialog_setChecked( id pDialog, int nID, BOOL bChecked )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pDialog )
	{
		ShowFileDialogArgs *pArgs = [ShowFileDialogArgs argsWithArgs:[NSArray arrayWithObjects:[NSNumber numberWithInt:nID], [NSNumber numberWithBool:bChecked], nil]];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[(ShowFileDialog *)pDialog performSelectorOnMainThread:@selector(setChecked:) withObject:pArgs waitUntilDone:YES modes:pModes];
	}

	[pPool release];
}

void NSFileDialog_setDefaultName( id pDialog, CFStringRef aName )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pDialog && aName )
	{
		ShowFileDialogArgs *pArgs = [ShowFileDialogArgs argsWithArgs:[NSArray arrayWithObject:(NSString *)aName]];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[(ShowFileDialog *)pDialog performSelectorOnMainThread:@selector(setDefaultName:) withObject:pArgs waitUntilDone:YES modes:pModes];
	}

	[pPool release];
}

void NSFileDialog_setDirectory( id pDialog, CFStringRef aDirectory )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pDialog && aDirectory )
	{
		ShowFileDialogArgs *pArgs = [ShowFileDialogArgs argsWithArgs:[NSArray arrayWithObject:(NSString *)aDirectory]];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[(ShowFileDialog *)pDialog performSelectorOnMainThread:@selector(setDirectory:) withObject:pArgs waitUntilDone:YES modes:pModes];
	}

	[pPool release];
}

void NSFileDialog_setEnabled( id pDialog, int nID, BOOL bEnabled )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pDialog )
	{
		ShowFileDialogArgs *pArgs = [ShowFileDialogArgs argsWithArgs:[NSArray arrayWithObjects:[NSNumber numberWithInt:nID], [NSNumber numberWithBool:bEnabled], nil]];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[(ShowFileDialog *)pDialog performSelectorOnMainThread:@selector(setEnabled:) withObject:pArgs waitUntilDone:YES modes:pModes];
	}

	[pPool release];
}

void NSFileDialog_setLabel( id pDialog, int nID, CFStringRef aLabel )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pDialog && aLabel )
	{
		ShowFileDialogArgs *pArgs = [ShowFileDialogArgs argsWithArgs:[NSArray arrayWithObjects:[NSNumber numberWithInt:nID], (NSString *)aLabel, nil]];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[(ShowFileDialog *)pDialog performSelectorOnMainThread:@selector(setLabel:) withObject:pArgs waitUntilDone:YES modes:pModes];
	}

	[pPool release];
}

void NSFileDialog_setMultiSelectionMode( id pDialog, BOOL bMultiSelectionMode )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pDialog )
	{
		ShowFileDialogArgs *pArgs = [ShowFileDialogArgs argsWithArgs:[NSArray arrayWithObject:[NSNumber numberWithBool:bMultiSelectionMode]]];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[(ShowFileDialog *)pDialog performSelectorOnMainThread:@selector(setMultiSelectionMode:) withObject:pArgs waitUntilDone:YES modes:pModes];
	}

	[pPool release];
}

void NSFileDialog_setSelectedFilter( id pDialog, CFStringRef aItem )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pDialog && aItem )
	{
		ShowFileDialogArgs *pArgs = [ShowFileDialogArgs argsWithArgs:[NSArray arrayWithObject:(NSString *)aItem]];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[(ShowFileDialog *)pDialog performSelectorOnMainThread:@selector(setSelectedFilter:) withObject:pArgs waitUntilDone:YES modes:pModes];
	}

	[pPool release];
}

void NSFileDialog_setSelectedItem( id pDialog, int nID, int nItem )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pDialog )
	{
		ShowFileDialogArgs *pArgs = [ShowFileDialogArgs argsWithArgs:[NSArray arrayWithObjects:[NSNumber numberWithInt:nID], [NSNumber numberWithInt:nItem], nil]];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[(ShowFileDialog *)pDialog performSelectorOnMainThread:@selector(setSelectedItem:) withObject:pArgs waitUntilDone:YES modes:pModes];
	}

	[pPool release];
}

void NSFileDialog_setTitle( id pDialog, CFStringRef aTitle )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pDialog && aTitle )
	{
		ShowFileDialogArgs *pArgs = [ShowFileDialogArgs argsWithArgs:[NSArray arrayWithObject:(NSString *)aTitle]];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[(ShowFileDialog *)pDialog performSelectorOnMainThread:@selector(setTitle:) withObject:pArgs waitUntilDone:YES modes:pModes];
	}

	[pPool release];
}

int NSFileDialog_showFileDialog( id pDialog )
{
	int nRet = NSCancelButton;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pDialog )
	{
		ShowFileDialogArgs *pArgs = [ShowFileDialogArgs argsWithArgs:nil];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[(ShowFileDialog *)pDialog performSelectorOnMainThread:@selector(showFileDialog:) withObject:pArgs waitUntilDone:YES modes:pModes];
		NSNumber *pRet = (NSNumber *)[pArgs result];
		if ( pRet )
			nRet = [pRet intValue];
	}

	[pPool release];

	return nRet;
}
