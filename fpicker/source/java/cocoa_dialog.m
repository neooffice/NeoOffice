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
	NSString*				mpDirectory;
	BOOL					mbExtensionHidden;
	NSArray*				mpFileNames;
	BOOL					mbMultiSelectionMode;
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
	NSString*				mpTitle;
	BOOL					mbUseFileOpenDialog;
}
- (void)dealloc;
- (NSString *)directory;
- (NSArray *)filenames;
- (id)initWithOptions:(BOOL)bUseFileOpenDialog chooseFiles:(BOOL)bChooseFiles showAutoExtension:(BOOL)bShowAutoExtension showFilterOptions:(BOOL)bShowFilterOptions showImageTemplate:(BOOL)bShowImageTemplate showLink:(BOOL)bShowLink showPassword:(BOOL)bShowPassword showPreview:(BOOL)bShowPreview showReadOnly:(BOOL)bShowReadOnly showSelction:(BOOL)bShowSelection showTemplate:(BOOL)bShowTemplate showVersion:(BOOL)bShowVersion;
- (BOOL)isExtensionHidden;
- (int)result;
- (void)showFileDialog:(id)pObject;
- (void)setDirectory:(NSString *)pDirectory;
- (void)setExtensionHidden:(BOOL)bExtensionHidden;
- (void)setMultiSelectionMode:(BOOL)bMultiSelectionMode;
- (void)setTitle:(NSString *)pTitle;
@end

@implementation ShowFileDialog

- (void)dealloc
{
	if ( mpFileNames )
		[mpFileNames release];

	if ( mpTitle )
		[mpTitle release];

	[super dealloc];
}

- (NSString *)directory
{
	return mpDirectory;
}

- (NSArray *)filenames
{
	return mpFileNames;
}

- (id)initWithOptions:(BOOL)bUseFileOpenDialog chooseFiles:(BOOL)bChooseFiles showAutoExtension:(BOOL)bShowAutoExtension showFilterOptions:(BOOL)bShowFilterOptions showImageTemplate:(BOOL)bShowImageTemplate showLink:(BOOL)bShowLink showPassword:(BOOL)bShowPassword showPreview:(BOOL)bShowPreview showReadOnly:(BOOL)bShowReadOnly showSelction:(BOOL)bShowSelection showTemplate:(BOOL)bShowTemplate showVersion:(BOOL)bShowVersion
{
	[super init];

	mpDirectory = nil;
	mbChooseFiles = bChooseFiles;
	mbExtensionHidden = NO;
	mpFileNames = nil;
	mbMultiSelectionMode = NO;
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
	mpTitle = nil;
	mbUseFileOpenDialog = bUseFileOpenDialog;

	return self;
}

- (BOOL)isExtensionHidden
{
	return mbExtensionHidden;
}

- (int)result;
{
	return mnResult;
}

- (void)showFileDialog:(id)pObject;
{
	if ( mpFileNames )
	{
		[mpFileNames release];
		mpFileNames = nil;
	}

	mnResult = NSCancelButton;

	NSSavePanel *pFilePanel;
	if ( mbUseFileOpenDialog )
		pFilePanel = (NSSavePanel *)[NSOpenPanel openPanel];
	else
		pFilePanel = [NSSavePanel savePanel];

	if ( pFilePanel )
	{
		if ( mpTitle )
			[pFilePanel setTitle:mpTitle];

		[pFilePanel setCanCreateDirectories:YES];
		[pFilePanel setCanSelectHiddenExtension:mbShowAutoExtension];
		[pFilePanel setExtensionHidden:mbExtensionHidden];

		if ( mbUseFileOpenDialog )
		{
			NSOpenPanel *pOpenPanel = (NSOpenPanel *)pFilePanel;

			[pOpenPanel setAllowsMultipleSelection:mbMultiSelectionMode];
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

			mnResult = [pOpenPanel runModalForDirectory:mpDirectory file:nil types:nil];
			if ( mnResult == NSOKButton )
			{
				NSArray *pArray = [pOpenPanel filenames];
				if ( pArray )
				{
					mpFileNames = [NSArray arrayWithArray:pArray];
					if ( mpFileNames )
						[mpFileNames retain];
				}
			}
		}
		else
		{
			[pFilePanel setTreatsFilePackagesAsDirectories:NO];

			mnResult = [pFilePanel runModalForDirectory:mpDirectory file:nil];
			if ( mnResult == NSOKButton )
			{
				NSString *pFileName = [pFilePanel filename];
				if ( pFileName )
				{
					mpFileNames = [NSArray arrayWithObject:pFileName];
					if ( mpFileNames )
						[mpFileNames retain];
				}
			}
		}

		mbExtensionHidden = [pFilePanel isExtensionHidden];
		[self setDirectory:[pFilePanel directory]];
	}
}

- (void)setDirectory:(NSString *)pDirectory
{
	if ( mpDirectory )
	{
		[mpDirectory release];
		mpDirectory = nil;
	}

	if ( pDirectory )
	{
		[pDirectory retain];
		mpDirectory = pDirectory;
	}
}

- (void)setExtensionHidden:(BOOL)bExtensionHidden
{
	mbExtensionHidden = bExtensionHidden;
}

- (void)setMultiSelectionMode:(BOOL)bMultiSelectionMode
{
	mbMultiSelectionMode = bMultiSelectionMode;
}

- (void)setTitle:(NSString *)pTitle
{
	if ( mpTitle )
	{
		[mpTitle release];
		mpTitle = nil;
	}

	if ( pTitle )
	{
		[pTitle retain];
		mpTitle = pTitle;
	}
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

BOOL NSFileDialog_isExtensionHidden( void *pDialog )
{
	BOOL bRet = NO;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pDialog )
		bRet = [(ShowFileDialog *)pDialog isExtensionHidden];

	[pPool release];

	return bRet;
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

void NSFileDialog_setDirectory( void *pDialog, CFStringRef aDirectory )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pDialog )
		[(ShowFileDialog *)pDialog setDirectory:(NSString *)aDirectory ];

	[pPool release];
}

void NSFileDialog_setExtensionHidden( void *pDialog, BOOL bExtensionHidden )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pDialog )
		[(ShowFileDialog *)pDialog setExtensionHidden:bExtensionHidden];

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
