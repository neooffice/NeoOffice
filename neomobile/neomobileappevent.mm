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

#import <crt_externs.h>
#import "neomobile.hxx"
#import "neomobileappevent.hxx"
#import "neomobilei18n.hxx"

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

	NSString *basePath = nil;

	NSFileManager *fileManager = [NSFileManager defaultManager];
	if (fileManager)
	{
		// Use NSCachesDirectory to stay within FileVault encryption
		NSArray *cachePaths = NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES);
		if (cachePaths)
		{
			NSNumber *perms = [NSNumber numberWithUnsignedLong:(S_IRUSR | S_IWUSR | S_IXUSR)];
			NSDictionary *dict = (perms ? [NSDictionary dictionaryWithObject:perms forKey:NSFilePosixPermissions] : nil);

 			unsigned int dirCount = [cachePaths count];
 			unsigned int i = 0;
			for (; i < dirCount && !basePath; i++)
			{
				MacOSBOOL isDir = NO;
				NSString *cachePath = (NSString *)[cachePaths objectAtIndex:i];
				if (([fileManager fileExistsAtPath:cachePath isDirectory:&isDir] && isDir) || [fileManager createDirectoryAtPath:cachePath attributes:dict])
				{
					// Append program name to cache path
					char **progName = _NSGetProgname();
					if (progName && *progName)
					{
						cachePath = [cachePath stringByAppendingPathComponent:[NSString stringWithUTF8String:(const char *)*progName]];
						isDir = NO;
						if (([fileManager fileExistsAtPath:cachePath isDirectory:&isDir] && isDir) || [fileManager createDirectoryAtPath:cachePath attributes:dict])
						{
							basePath = cachePath;
							break;
						}
					}
				}
			}
		}
	}

	if (!basePath)
	{
		basePath = NSTemporaryDirectory();
		if (!basePath)
			basePath = @"/tmp";
	}

	NSString *filePath = [basePath stringByAppendingPathComponent:@"_nm_export"];
	if (fileManager)
	{
		while ([fileManager fileExistsAtPath:filePath])
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
	NSNumber *perms = [NSNumber numberWithUnsignedLong:(S_IRUSR | S_IWUSR | S_IXUSR)];
	NSDictionary *dict = (perms ? [NSDictionary dictionaryWithObject:perms forKey:NSFilePosixPermissions] : nil);
	[[NSFileManager defaultManager] createDirectoryAtPath:path attributes:dict];
}

- (void)removeItem:(NSString *)path
{
	[[NSFileManager defaultManager] removeFileAtPath:path handler:nil];
}

@end

NeoMobileExportFileAppEvent::NeoMobileExportFileAppEvent( OUString aSaveUUID, NSFileManager *pFileManager, NSMutableData *pPostBody, NSArray *pMimeTypes ) :
	mnErrorCode( 0 ),
	mpFileManager( pFileManager ),
	mbFinished( false ),
	mpPostBody( pPostBody ),
	maSaveUUID( aSaveUUID ),
	mpMimeTypes( pMimeTypes ),
	mbCanceled( false ),
	mbUnsupportedComponentType( false )
{
}
