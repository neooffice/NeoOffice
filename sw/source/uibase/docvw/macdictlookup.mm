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
 *  Patrick Luby, February 2010
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2010 Planamesa Inc.
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

#include <sfx2/app.hxx>
#include <tools/rcid.h>

#include <premac.h>
#import <Cocoa/Cocoa.h>
#include <postmac.h>

#include "macdictlookup.hxx"

static ResMgr *pSwResMgr = nullptr;

OUString GetMacDictLoookupResString( int nId )
{
    if ( !pSwResMgr )
    {
        pSwResMgr = ResMgr::CreateResMgr( "sw" );
        if ( !pSwResMgr )
            return "";
    }

    ResId aResId( nId, *pSwResMgr );
    aResId.SetRT( RSC_STRING );
    if ( !pSwResMgr->IsAvailable( aResId ) )
        return "";
 
    return OUString( ResId( nId, *pSwResMgr ) );
}

@interface SWPerformMacDictService : NSObject
+ (id)create;
- (void)lookupInMacDict:(NSString *)pString;
@end

@implementation SWPerformMacDictService

+ (id)create
{
	SWPerformMacDictService *pRet = [[SWPerformMacDictService alloc] init];
	[pRet autorelease];
	return pRet;
}

- (void)lookupInMacDict:(NSString *)pString
{
	if ( pString )
	{
		NSPasteboard *pPasteboard = [NSPasteboard pasteboardWithUniqueName];
		if ( pPasteboard )
		{
			[pPasteboard declareTypes:[NSArray arrayWithObject:NSStringPboardType] owner:nil];
			[pPasteboard setString:pString forType:NSStringPboardType];
			NSPerformService( @"Look Up in Dictionary", pPasteboard );
		}
	}
}

@end

void LookupInMacDict( const OUString &aString )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( aString.getLength() )
	{
		NSString *pString = [NSString stringWithCharacters:aString.getStr() length:aString.getLength()];
		if ( pString )
		{
			NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
			SWPerformMacDictService *pSWPerformMacDictService = [SWPerformMacDictService create];
			[pSWPerformMacDictService performSelectorOnMainThread:@selector(lookupInMacDict:) withObject:pString waitUntilDone:YES modes:pModes];
		}
	}

	[pPool release];
}
