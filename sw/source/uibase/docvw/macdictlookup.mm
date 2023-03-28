/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <osl/objcutils.h>
#include <sfx2/app.hxx>
#include <tools/rcid.h>

#include "macdictlookup.hxx"

static ResMgr *pSwResMgr = NULL;

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
			[pPasteboard declareTypes:[NSArray arrayWithObject:NSPasteboardTypeString] owner:nil];
			[pPasteboard setString:pString forType:NSPasteboardTypeString];
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
			SWPerformMacDictService *pSWPerformMacDictService = [SWPerformMacDictService create];
			osl_performSelectorOnMainThread( pSWPerformMacDictService, @selector(lookupInMacDict:), pString, YES );
		}
	}

	[pPool release];
}
