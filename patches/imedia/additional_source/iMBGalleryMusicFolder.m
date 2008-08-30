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

// Derived from iMBGalleryMusicFolder class
// Copyright 2005-2007 by Karelia Software et al.


#import "iMBGalleryMusicFolder.h"
#import "iMedia.h"
#import "MetadataUtility.h"

static NSImage *sSongIcon = nil;
static NSImage *sDRMIcon = nil;

@implementation iMBGalleryMusicFolder

+ (void)initialize	// preferred over +load in most cases
{
	if ( self == [iMBGalleryMusicFolder class] ) 
	{
		// Only do some work when not called because one of our subclasses does not implement +initialize
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	
	NSBundle *bndl = [NSBundle bundleForClass:[self class]];
	NSString *iconPath = [bndl pathForResource:@"MBiTunes4Song" ofType:@"png"];
	sSongIcon = [[[NSWorkspace sharedWorkspace] iconForFileType:@"mp3"] retain];
	sDRMIcon = [[[NSWorkspace sharedWorkspace] iconForFileType:@"m4p"] retain];
	
	[pool release];
}
}

+ (void)load	// registration of this class
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    
	[iMediaConfiguration registerParser:[self class] forMediaType:@"music"];

	[pool release];
}

- (id)initWithContentsOfFile:(NSString *)file musicFolderName:(NSString *)musicFolderName unknownArtistName:(NSString *)unknownArtistName iconName:(NSString *)iconName parseMetadata:(BOOL)parseMetadata
{
    self = [super initWithContentsOfFile:file];
    if (self)
    {
        myMusicFolderName = [musicFolderName copy];
        myUnknownArtistName = [unknownArtistName copy];
        myIconName = [iconName copy];
        myParseMetadata = parseMetadata;
    }
    return self;
}

- (id)initWithContentsOfFile:(NSString *)file
{
    NSString *musicFolderName = LocalizedStringInIMedia(@"NeoOffice Gallery", @"Audio from the NeoOffice Gallery");
    NSString *unknownArtistName = LocalizedStringInIMedia(@"Unknown", @"Unknown music key");
    NSString *iconName = @"gallery";

	return [self initWithContentsOfFile:file musicFolderName:musicFolderName unknownArtistName:unknownArtistName iconName:iconName parseMetadata:YES];
}

- (id)init
{
	return [self initWithContentsOfFile:[[[NSBundle mainBundle] bundlePath] stringByAppendingPathComponent:@"Contents/share/gallery"]];
}

- (void)dealloc
{
    [myMusicFolderName release]; myMusicFolderName = nil;
    [myUnknownArtistName release]; myUnknownArtistName = nil;
    [myIconName release]; myIconName = nil;

    [super dealloc];
}

- (long)recursivelyParse:(NSString *)path withNode:(iMBLibraryNode *)root movieTypes:(NSArray *)movieTypes
{
	NSFileManager *fileManager = [NSFileManager defaultManager];
	NSWorkspace *workspace = [NSWorkspace sharedWorkspace];
	NSArray *contents = [fileManager directoryContentsAtPath:path];
	NSEnumerator *e = [contents objectEnumerator];
	NSMutableArray *tracks = [NSMutableArray array];
	long addedItems = 0;
   
	NSAutoreleasePool *innerPool = [[NSAutoreleasePool alloc] init];
	int poolRelease = 0;
	NSArray *excludedFolders = [[iMediaConfiguration sharedConfiguration] excludedFolders];
   
	NSString *currentFile;
	while (currentFile = [e nextObject])
	{
		NSString *filePath = [path stringByAppendingPathComponent:currentFile];

        // skip files that are likely to be included elsewhere or have been explicitly excluded
		if ([excludedFolders containsObject:filePath]) continue;
        
        BOOL isDirectory;
		if ( [fileManager fileExistsAtPath:filePath isDirectory:&isDirectory] &&
            ![fileManager isPathHidden:filePath] &&
            ![workspace isFilePackageAtPath:filePath])
		{
			if (isDirectory)
			{
				iMBLibraryNode *folder = [[[iMBLibraryNode alloc] init] autorelease];
				[folder setIconName:@"folder"];
				[folder setName:[fileManager displayNameAtPath:filePath]];
				addedItems=[self recursivelyParse:filePath withNode:folder movieTypes:movieTypes];
				
				if(addedItems)
				{
					// NOTE: It is not legal to add items on a thread; so we do it on the main thread.
					// [root addItem:folder];
					[root performSelectorOnMainThread:@selector(addItem:) withObject:folder waitUntilDone:YES];
					
					addedItems++;
				}
			}
			else
			{
					NSString *UTI = [NSString UTIForFileAtPath:filePath];
				if ([NSString UTI:UTI conformsToUTI:(NSString *)kUTTypeAudio])
				{
					if ([movieTypes indexOfObject:[[filePath lowercaseString] pathExtension]] != NSNotFound)
					{
						NSMutableDictionary *song = [NSMutableDictionary dictionary]; 
						
						NSDictionary *arguments = [[MetadataUtility sharedMetadataUtility] getMetadataForFile:filePath];
						
						if (arguments)
						{
							// handle the song duration
							NSNumber *duration = [arguments objectForKey:@"kMDItemDurationSeconds"];
							if ( duration != nil )
							{
								[song setObject:[NSNumber numberWithFloat:[duration floatValue]*1000] forKey:@"Total Time"];
							}
							else
							{
								[song setObject:[NSNumber numberWithInt:0] forKey:@"Total Time"];
							}
							
							// handle the song title
							NSString *title = [arguments objectForKey:@"kMDItemTitle"];
							if ( title != nil )
							{
								[song setObject:title forKey:@"Name"];
							}
							else
							{
								[song setObject:[[filePath lastPathComponent] stringByDeletingPathExtension] forKey:@"Name"];
							}
							
							// handle the song artist
							NSArray *authors = [arguments objectForKey:@"kMDItemAuthors"];
							if ( authors != nil && [authors count] > 0 )
							{
								[song setObject:[authors objectAtIndex:0] forKey:@"Artist"];
							}
							else
							{
								[song setObject:myUnknownArtistName forKey:@"Artist"];
							}
							
							// handle the song protected vs. unprotected icon
							NSString *kind = [arguments objectForKey:@"kMDItemKind"];
							if ( kind != nil && [kind rangeOfString:@"Protected"].location != NSNotFound )
							{
								[song setObject:sDRMIcon forKey:@"Icon"];
							}
							else
							{
								[song setObject:sSongIcon forKey:@"Icon"];
							}
							
							[song setObject:filePath forKey:@"Location"];
							[song setObject:filePath forKey:@"Preview"];
							
							[tracks addObject:song];
							
							addedItems++;
						}
					}
				}
			}
		}
		poolRelease++;
		if (poolRelease == 15)
		{
			poolRelease = 0;
			[innerPool release];	// don't use drain, maybe we retain 10.3 compatibility?
			innerPool = [[NSAutoreleasePool alloc] init];
		}
	}
	[innerPool release];
	[root setAttribute:tracks forKey:@"Tracks"];
	
	return(addedItems);
}

- (void)populateLibraryNode:(iMBLibraryNode *)root
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

	NSString *folder = [self databasePath];
	if ( [[NSFileManager defaultManager] fileExistsAtPath:folder] )
    {
        NSMutableArray *movieTypes = [NSMutableArray arrayWithArray:[QTMovie movieFileTypes:QTIncludeCommonTypes]];
        
		// TODO: Why is this type being removed? What is it? cmeyer 2007/08/07.
        [movieTypes removeObject:@"kar"];
        
        [self recursivelyParse:folder withNode:root movieTypes:movieTypes];
    }
    
    // the node is populated, so remove the 'loading' moniker. do this on the main thread to be friendly to bindings.
	[root performSelectorOnMainThread:@selector(setName:) withObject:myMusicFolderName waitUntilDone:NO];
	
	[pool release];
}

- (iMBLibraryNode *)parseDatabase
{
	NSString *folder = [self databasePath];
	if ( [[NSFileManager defaultManager] fileExistsAtPath:folder] )
    {
        iMBLibraryNode *root = [[[iMBLibraryNode alloc] init] autorelease];
        
        // the name will include 'loading' until it is populated.
        NSString *loadingString = LocalizedStringInIMedia(@"Loading...", @"Name extension to indicate it is loading.");
        [root setName:[myMusicFolderName stringByAppendingFormat:@" (%@)", loadingString]];
        [root setIconName:myIconName];
        
        // the node itself will be returned immediately. now launch _another_ thread to populate the node.
        [NSThread detachNewThreadSelector:@selector(populateLibraryNode:) toTarget:self withObject:root];
        
        return root;
    }
    else
    {
        return nil;
    }
}

@end
