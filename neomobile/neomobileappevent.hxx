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

#ifndef _NEOMOBILEAPPEVENT_HXX
#define _NEOMOBILEAPPEVENT_HXX

#import <tools/link.hxx>

#include <premac.h>
#import <Foundation/Foundation.h>
#include <postmac.h>

// Redefine Cocoa YES and NO defines types for convenience
#ifdef YES
#undef YES
#define YES (MacOSBOOL)1
#endif
#ifdef NO
#undef NO
#define NO (MacOSBOOL)0
#endif

@interface RunPasswordProtectionAlertOnMainThread : NSObject
{
	MacOSBOOL mcancelled;
}
- (id)init;
- (void)runModal:(id)arg;
- (MacOSBOOL)cancelled;
@end

@interface DoFileManagerOnMainThread : NSObject
{
	NSString *mpath;
}
- (id)init;
- (void)dealloc;
- (void)makeBasePath:(id)arg;
- (NSString *)filePath;
- (void)createDir:(NSString *)path;
- (void)removeItem:(NSString *)path;
@end

class NeoMobileExportFileAppEvent
{
	int						mnErrorCode;
	NSFileManager*			mpFileManager;
	bool					mbFinished;
	NSMutableData*			mpPostBody;
	::rtl::OUString			maSaveUUID;
	NSArray*				mpMimeTypes;
	bool					mbCanceled;
	bool					mbUnsupportedComponentType;

public:
							NeoMobileExportFileAppEvent( ::rtl::OUString aSaveUUID, NSFileManager *pFileManager, NSMutableData *pPostBody, NSArray *pMimeTypes );
	virtual					~NeoMobileExportFileAppEvent() {};
							DECL_LINK( ExportFile, void* );
	void					Cancel() { mbCanceled=true; }
	void					Execute();
	int						GetErrorCode() { return mnErrorCode; }
	NSData*					GetPostBody() { return mpPostBody; }
	bool					IsCanceled() { return(mbCanceled); }
	bool					IsFinished() { return mbFinished; }
	bool					IsUnsupportedComponentType() { return(mbUnsupportedComponentType); }
};

#endif	// _NEOMOBILEAPPEVENT_HXX
