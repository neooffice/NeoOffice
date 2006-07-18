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
	NSString*				mpDirectory;
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
- (id)initWithOptions:(BOOL)bUseFileOpenDialog showAutoExtension:(BOOL)bShowAutoExtension showFilterOptions:(BOOL)bShowFilterOptions showImageTemplate:(BOOL)bImageTemplate showLink:(BOOL)bShowLink showPassword:(BOOL)bShowPassword showPreview:(BOOL)bShowPreview showReadOnly:(BOOL)bShowReadOnly showSelction:(BOOL)bShowSelection showTemplate:(BOOL)bShowTemplate showVersion:(BOOL)bShowVersion;
- (int)result;
- (void)showFileDialog:(id)pObject;
- (void)setDirectory:(NSString *)pDirectory;
- (void)setMultiSelectionMode:(BOOL)bMultiSelectionMode;
- (void)setTitle:(NSString *)pTitle;
@end

@implementation ShowFileDialog

- (void)dealloc
{
	if ( mpTitle )
		[mpTitle release];

	[super dealloc];
}

- (id)initWithOptions:(BOOL)bUseFileOpenDialog showAutoExtension:(BOOL)bShowAutoExtension showFilterOptions:(BOOL)bShowFilterOptions showImageTemplate:(BOOL)bShowImageTemplate showLink:(BOOL)bShowLink showPassword:(BOOL)bShowPassword showPreview:(BOOL)bShowPreview showReadOnly:(BOOL)bShowReadOnly showSelction:(BOOL)bShowSelection showTemplate:(BOOL)bShowTemplate showVersion:(BOOL)bShowVersion
{
	[super init];

	mpDirectory = nil;
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

- (int)result;
{
	return mnResult;
}

- (void)showFileDialog:(id)pObject;
{
	NSSavePanel *pFilePanel;
	if ( mbUseFileOpenDialog )
		pFilePanel = (NSSavePanel *)[NSOpenPanel openPanel];
	else
		pFilePanel = [NSSavePanel savePanel];

	if ( mpTitle )
		[pFilePanel setTitle:mpTitle];

	if ( pFilePanel )
	{
		if ( mbUseFileOpenDialog )
		{
			NSOpenPanel *pOpenPanel = (NSOpenPanel *)pFilePanel;
			[pOpenPanel setAllowsMultipleSelection:mbMultiSelectionMode];

			mnResult = [pOpenPanel runModalForDirectory:mpDirectory file:nil types:nil];
		}
		else
		{
			mnResult = [pFilePanel runModalForDirectory:mpDirectory file:nil];
		}
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

id NSFileDialog_create( BOOL bUseFileOpenDialog, BOOL bShowAutoExtension, BOOL bShowFilterOptions, BOOL bShowImageTemplate, BOOL bShowLink, BOOL bShowPassword, BOOL bShowPreview, BOOL bShowReadOnly, BOOL bShowSelection, BOOL bShowTemplate, BOOL bShowVersion )
{
	ShowFileDialog *pRet = nil;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	pRet = [[ShowFileDialog alloc] initWithOptions:bUseFileOpenDialog showAutoExtension:bShowAutoExtension showFilterOptions:bShowFilterOptions showImageTemplate:bShowImageTemplate showLink:bShowLink showPassword:bShowPassword showPreview:bShowPreview showReadOnly:bShowReadOnly showSelction:bShowSelection showTemplate:bShowTemplate showVersion:bShowVersion];
	if ( pRet )
		[pRet retain];

	[pPool release];

	return pRet;
}

void NSFileDialog_release( void *pDialog )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pDialog )
		[(ShowFileDialog *)pDialog release];

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
