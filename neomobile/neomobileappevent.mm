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
 *		 - GNU General Public License Version 2.1
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2008 by Planamesa Inc.
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
 *************************************************************************/

#include "neomobile.hxx"
#include "neomobileappevent.hxx"
#include "neomobilei18n.hxx"

using namespace ::rtl;

@implementation RunPasswordProtectionAlertOnMainThread

- (MacOSBOOL)cancelled
{
	return mcancelled;
}

- (id)init
{
	[super init];
	
	mcancelled=YES;
	return self;
}

- (void)runModal:(id)arg;
{
	NSAlert *alert = [NSAlert alertWithMessageText:GetLocalizedString(NEOMOBILEUPLOADPASSWORDPROTECTED) defaultButton:GetLocalizedString(NEOMOBILEUPLOAD) alternateButton:GetLocalizedString(NEOMOBILECANCEL) otherButton:nil informativeTextWithFormat:GetLocalizedString(NEOMOBILEUPLOADCONTINUE)];
	if(alert && [alert runModal] == NSAlertDefaultReturn)
		mcancelled = NO;
}

@end

@implementation DoFileManagerOnMainThread

- (id)init
{
	[super init];
	
	mpath=nil;
	return self;
}

- (void)dealloc
{
	if(mpath)
	{
		[mpath release];
		mpath=nil;
	}
	
	[super dealloc];
}

- (void)makeBasePath:(id)arg
{
#pragma unused(arg)

	NSString *basePath = NSTemporaryDirectory();
	NSString *filePath = [basePath stringByAppendingPathComponent:@"_nm_export"];
	while ([[NSFileManager defaultManager] fileExistsAtPath:filePath]) {
		filePath = [basePath stringByAppendingPathComponent:[NSString stringWithFormat:@"_nm_export_%d", rand()]];
	}
	[filePath retain];
	mpath=filePath;
}

- (NSString *)filePath
{
	return(mpath);
}

- (void)createDir:(NSString *)path
{
	[[NSFileManager defaultManager] createDirectoryAtPath: path attributes: nil];
}

- (void)removeItem:(NSString *)path
{
	[[NSFileManager defaultManager] removeFileAtPath:path handler:NULL];
}

@end

NeoMobileExportFileAppEvent::NeoMobileExportFileAppEvent( OUString aSaveUUID, NSFileManager *pFileManager, NSMutableData *pPostBody ) :
	mnErrorCode( 0 ),
	mpFileManager( pFileManager ),
	mbFinished( false ),
	mpPostBody( pPostBody ),
	maSaveUUID( aSaveUUID ),
	mbCanceled( false ),
	mbUnsupportedComponentType( false )
{
}
