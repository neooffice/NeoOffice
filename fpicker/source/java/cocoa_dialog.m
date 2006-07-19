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
	BOOL					mbShowPreview;
	BOOL					mbShowReadOnly;
	BOOL					mbShowSelection;
	BOOL					mbShowTemplate;
	BOOL					mbShowVersion;
	BOOL					mbUseFileOpenDialog;
}
- (void)dealloc;
- (NSString *)directory;
- (NSArray *)filenames;
- (id)initWithOptions:(BOOL)bUseFileOpenDialog chooseFiles:(BOOL)bChooseFiles showAutoExtension:(BOOL)bShowAutoExtension showFilterOptions:(BOOL)bShowFilterOptions showImageTemplate:(BOOL)bShowImageTemplate showLink:(BOOL)bShowLink showPassword:(BOOL)bShowPassword showPreview:(BOOL)bShowPreview showReadOnly:(BOOL)bShowReadOnly showSelction:(BOOL)bShowSelection showTemplate:(BOOL)bShowTemplate showVersion:(BOOL)bShowVersion;
- (BOOL)isChecked:(int)nID;
- (NSString *)label:(int)nID;
- (int)result;
- (void)showFileDialog:(id)pObject;
- (void)setChecked:(int)nID checked:(BOOL)bChecked;
- (void)setDirectory:(NSString *)pDirectory;
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

- (id)initWithOptions:(BOOL)bUseFileOpenDialog chooseFiles:(BOOL)bChooseFiles showAutoExtension:(BOOL)bShowAutoExtension showFilterOptions:(BOOL)bShowFilterOptions showImageTemplate:(BOOL)bShowImageTemplate showLink:(BOOL)bShowLink showPassword:(BOOL)bShowPassword showPreview:(BOOL)bShowPreview showReadOnly:(BOOL)bShowReadOnly showSelction:(BOOL)bShowSelection showTemplate:(BOOL)bShowTemplate showVersion:(BOOL)bShowVersion
{
	[super init];

	mbChooseFiles = bChooseFiles;
	mnResult = NSCancelButton;
	mbShowAutoExtension = bShowAutoExtension;
	mbShowFilterOptions = bShowFilterOptions;
	mbShowImageTemplate = bShowImageTemplate;
	mbShowLink = bShowLink;
	mbShowPassword = bShowPassword;
	mbShowPreview = bShowPreview;
	mbShowReadOnly = bShowReadOnly;
	mbShowSelection = bShowSelection;
	mbShowTemplate = bShowTemplate;
	mbShowVersion = bShowVersion;
	mbUseFileOpenDialog = bUseFileOpenDialog;

	mpControls = [[NSMutableDictionary alloc] init];
	if ( mpControls )
		[mpControls retain];

	if ( mbUseFileOpenDialog )
		mpFilePanel = (NSSavePanel *)[NSOpenPanel openPanel];
	else
		mpFilePanel = [NSSavePanel savePanel];

	if ( mpFilePanel )
		[mpFilePanel retain];

	// Create read only checkbox
	if ( mbShowReadOnly )
	{
		NSButton *pReadOnlyButton = [[NSButton alloc] initWithFrame:NSMakeRect( 0, 0, 0, 0 )];
		if ( pReadOnlyButton )
		{
			[pReadOnlyButton setButtonType:NSSwitchButton];
			[pReadOnlyButton setState:NSOffState];
			[pReadOnlyButton setTitle:@""];
			[mpControls setValue:pReadOnlyButton forKey:[[NSNumber numberWithInt:COCOA_CONTROL_ID_READONLY] stringValue]];
		}
	}

	return self;
}

- (BOOL)isChecked:(int)nID
{
	BOOL bRet = NO;

	switch ( nID )
	{
		case COCOA_CONTROL_ID_AUTOEXTENSION:
			bRet = [mpFilePanel isExtensionHidden];
			break;
		case COCOA_CONTROL_ID_READONLY:
			{
				NSButton *pButton = (NSButton *)[mpControls objectForKey:[[NSNumber numberWithInt:nID] stringValue]];
				if ( pButton )
					bRet = ( [pButton state] == NSOnState );
			}
			break;
	}

	return bRet;
}

- (NSString *)label:(int)nID
{
	NSControl *pControl = (NSControl *)[mpControls objectForKey:[[NSNumber numberWithInt:nID] stringValue]];
	if ( pControl )
	{
		switch ( nID )
		{
			case COCOA_CONTROL_ID_READONLY:
				return [(NSButton *)pControl title];
		}
	}

	return nil;
}

- (int)result;
{
	return mnResult;
}

- (void)showFileDialog:(id)pObject;
{
	mnResult = NSCancelButton;

	[mpFilePanel setCanCreateDirectories:YES];
	[mpFilePanel setCanSelectHiddenExtension:mbShowAutoExtension];

	// Create accessory view
	NSView *pAccessoryView = [[NSView alloc] initWithFrame:NSMakeRect(0, 0, 0, 0)];
	if ( pAccessoryView )
	{
		NSView *pOldAccessoryView = [mpFilePanel accessoryView];

		float nCurrentY = 0;
		float nCurrentWidth = 0;

		NSButton *pReadOnlyButton = (NSButton *)[mpControls objectForKey:[[NSNumber numberWithInt:COCOA_CONTROL_ID_READONLY] stringValue]];
		if ( pReadOnlyButton )
		{
			[pReadOnlyButton setFrameOrigin:NSMakePoint( 0, nCurrentY )];
			[pReadOnlyButton sizeToFit];
			nCurrentY += [pReadOnlyButton bounds].size.height;
			float nWidth = [pReadOnlyButton bounds].size.width;
			if ( nWidth > nCurrentWidth )
				nCurrentWidth = nWidth;
			[pAccessoryView addSubview:pReadOnlyButton];
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

			mnResult = [pOpenPanel runModalForDirectory:nil file:nil types:nil];
		}
		else
		{
			[mpFilePanel setTreatsFilePackagesAsDirectories:NO];

			mnResult = [mpFilePanel runModalForDirectory:nil file:nil];
		}

		[mpFilePanel setAccessoryView:pOldAccessoryView];

		if ( pSubviews && [pSubviews count] )
		{
			int nCount = [pSubviews count];
			int i = 0;
			for ( ; i < nCount; i++ )
				[[pSubviews objectAtIndex:i] removeFromSuperview];
		}
	}
}

- (void)setDirectory:(NSString *)pDirectory
{
	[mpFilePanel setDirectory:pDirectory];
}

- (void)setChecked:(int)nID checked:(BOOL)bChecked
{
	switch ( nID )
	{
		case COCOA_CONTROL_ID_AUTOEXTENSION:
			[mpFilePanel setExtensionHidden:bChecked];
			break;
		case COCOA_CONTROL_ID_READONLY:
			{
				NSButton *pButton = (NSButton *)[mpControls objectForKey:[[NSNumber numberWithInt:nID] stringValue]];
				if ( pButton )
					[pButton setState:( bChecked ? NSOnState : NSOffState )];
			}
			break;
	}
}

- (void)setLabel:(int)nID label:(NSString *)pLabel;
{
	if ( !pLabel )
		pLabel = @"";

	NSControl *pControl = (NSControl *)[mpControls objectForKey:[[NSNumber numberWithInt:nID] stringValue]];
	if ( pControl )
	{
		switch ( nID )
		{
			case COCOA_CONTROL_ID_READONLY:
				return [(NSButton *)pControl setTitle:pLabel];
		}
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

id NSFileDialog_create( BOOL bUseFileOpenDialog, BOOL bChooseFiles, BOOL bShowAutoExtension, BOOL bShowFilterOptions, BOOL bShowImageTemplate, BOOL bShowLink, BOOL bShowPassword, BOOL bShowPreview, BOOL bShowReadOnly, BOOL bShowSelection, BOOL bShowTemplate, BOOL bShowVersion )
{
	ShowFileDialog *pRet = nil;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	pRet = [[ShowFileDialog alloc] initWithOptions:bUseFileOpenDialog chooseFiles:bChooseFiles showAutoExtension:bShowAutoExtension showFilterOptions:bShowFilterOptions showImageTemplate:bShowImageTemplate showLink:bShowLink showPassword:bShowPassword showPreview:bShowPreview showReadOnly:bShowReadOnly showSelction:bShowSelection showTemplate:bShowTemplate showVersion:bShowVersion];
	if ( pRet )
		[pRet retain];

	[pPool release];

	return pRet;
}

CFStringRef NSFileDialog_directory( void *pDialog )
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

CFStringRef *NSFileDialog_fileNames( void *pDialog )
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

BOOL NSFileDialog_isChecked( void *pDialog, int nID )
{
	BOOL bRet = NO;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pDialog )
		bRet = [(ShowFileDialog *)pDialog isChecked:nID];

	[pPool release];

	return bRet;
}

CFStringRef NSFileDialog_label( void *pDialog, int nID )
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

void NSFileDialog_release( void *pDialog )
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

void NSFileDialog_setChecked( void *pDialog, int nID, BOOL bChecked )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pDialog )
		[(ShowFileDialog *)pDialog setChecked:nID checked:bChecked];

	[pPool release];
}

void NSFileDialog_setDirectory( void *pDialog, CFStringRef aDirectory )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pDialog )
		[(ShowFileDialog *)pDialog setDirectory:(NSString *)aDirectory ];

	[pPool release];
}

void NSFileDialog_setLabel( void *pDialog, int nID, CFStringRef aLabel )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pDialog )
		[(ShowFileDialog *)pDialog setLabel:nID label:(NSString *)aLabel];

	[pPool release];
}

void NSFileDialog_setMultiSelectionMode( void *pDialog, BOOL bMultiSelectionMode )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pDialog )
		[(ShowFileDialog *)pDialog setMultiSelectionMode:bMultiSelectionMode];

	[pPool release];
}

void NSFileDialog_setTitle( void *pDialog, CFStringRef aTitle )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pDialog )
		[(ShowFileDialog *)pDialog setTitle:(NSString *)aTitle];

	[pPool release];
}
