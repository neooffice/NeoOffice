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

// Derived from iMBMusicFolder class
// Copyright 2005-2007 by Karelia Software et al.


#import <Cocoa/Cocoa.h>
#import "iMBAbstractParser.h"

@interface iMBGalleryMusicFolder : iMBAbstractParser
{
    // note: these fields are only used during parsing; so there are no accessor functions for now.
	NSString *  myMusicFolderName;      // the name of the music folder as it appears to the user
	NSString *  myUnknownArtistName;    // artist name to be displayed when artist cannot be read from metadata
	NSString *  myIconName;             // the icon identifier for this folder
	BOOL        myParseMetadata;        // whether to parse metadata or not
}

// designated initializer
- (id)initWithContentsOfFile:(NSString *)file musicFolderName:(NSString *)musicFolderName unknownArtistName:(NSString *)unknownArtistName iconName:(NSString *)iconName parseMetadata:(BOOL)parseMetadata;

// default values
- (id)initWithContentsOfFile:(NSString *)file;

- (long)recursivelyParse:(NSString *)path withNode:(iMBLibraryNode *)root movieTypes:(NSArray *)movieTypes;

@end
