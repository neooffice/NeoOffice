/*************************************************************************
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU General Public License Version 2.1
 *
 *  Edward Peterlin, April 2008
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2008 Planamesa Inc.
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
 */

// Derived from iMBPicturesFolder class
// Copyright 2005-2007 by Karelia Software et al.

#import "iMBGalleryPicturesFolder.h"
#import "iMediaConfiguration.h"
#import "iMBLibraryNode.h"
#import "iMedia.h"

@implementation iMBGalleryPicturesFolder

+ (void)load
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	
	[iMediaConfiguration registerParser:[self class] forMediaType:@"photos"];
	
	[pool release];
}


- (id)init
{
	if (self = [super initWithContentsOfFile:[[[NSBundle mainBundle] bundlePath] stringByAppendingPathComponent:@"Contents/share/gallery"]])
	{
		
	}
	return self;
}

- (long)recursivelyParse:(NSString *)path withNode:(iMBLibraryNode *)root
{
	NSFileManager *fm = [NSFileManager defaultManager];
	NSWorkspace *ws = [NSWorkspace sharedWorkspace];
	NSArray *contents = [fm directoryContentsAtPath:path];
	NSEnumerator *e = [contents objectEnumerator];
	NSString *cur;
	BOOL isDir;
	NSMutableArray *images = [NSMutableArray array];
   NSArray * excludedFolders = [[iMediaConfiguration sharedConfiguration] excludedFolders];
	long numItems=0;
	
	while (cur = [e nextObject])
	{
		NSString *filePath = [path stringByAppendingPathComponent: cur];
		
		if ([excludedFolders containsObject:filePath]) continue;
      
		if ([fm fileExistsAtPath:filePath isDirectory:&isDir] && ![fm isPathHidden:filePath] && ![ws isFilePackageAtPath:filePath] )
		{
			if (isDir)
			{
				iMBLibraryNode *folder = [[iMBLibraryNode alloc] init];
				[folder setIconName:@"folder"];
				[folder setName:[fm displayNameAtPath:filePath]];
				numItems=[self recursivelyParse:filePath withNode:folder];
				
				if(numItems)
				{
					[root addItem:folder];
					numItems++;
				}
				
				[folder release];
			}
			else
			{
				NSString *UTI = [NSString UTIForFileAtPath:filePath];
				if ([NSString UTI:UTI conformsToUTI:(NSString *)kUTTypeImage])
				{
					NSMutableDictionary *newPicture = [NSMutableDictionary dictionary]; 
					if (filePath)
					{
						[newPicture setObject:filePath forKey:@"ImagePath"];
						[newPicture setObject:[[fm displayNameAtPath:filePath] stringByDeletingPathExtension] forKey:@"Caption"];
						//[newPicture setObject:filePath forKey:@"ThumbPath"];
					}
					NSDictionary *fileAttribs = [fm fileAttributesAtPath:filePath traverseLink:YES];
					NSDate* modDate = [fileAttribs fileModificationDate];
					if (modDate)
					{
						[newPicture setObject:[NSNumber numberWithDouble:[modDate timeIntervalSinceReferenceDate]]
																  forKey:@"DateAsTimerInterval"];
					}
					[images addObject:newPicture];
					
					numItems++;
				}
			}
		}
	}
	[root setAttribute:images forKey:@"Images"];
	
	return(numItems);
}

- (iMBLibraryNode *)parseDatabase
{
	iMBLibraryNode *root = [[iMBLibraryNode alloc] init];
	[root setName:LocalizedStringInIMedia(@"NeoOffice Gallery", @"Photos from the NeoOffice gallery")];
	[root setIconName:@"gallery"];
		
	if (![[NSFileManager defaultManager] fileExistsAtPath:myDatabase])
	{
		[root release];
		return nil;
	}
	
	[self recursivelyParse:myDatabase
				  withNode:root];

	return [root autorelease];
}

@end
