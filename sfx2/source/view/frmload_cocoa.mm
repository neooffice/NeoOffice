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

#include <rtl/ustring.hxx>

#include <premac.h>
#import <Foundation/Foundation.h>
#include <postmac.h>
#import "frmload_cocoa.h"

#define DEFAULT_LAUNCH_OPTIONS_KEY @"DefaultLaunchOptions"
#define DEFAULT_LAUNCH_OPTIONS_DOC_TYPES_OPENED_KEY @"DefaultLaunchOptionsDoctypesOpened"
#define MIN_NUMBER_DOCS_OPENED 15
#define MIN_FRACTION_DOCS_OPENED 0.67f

static NSDictionary *pDocTypeLaunchOptions = nil;

void SfxFrameLoader_openDocumentOfType( OUString aDocType )
{
	if ( !aDocType.getLength() )
		return;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( !pDocTypeLaunchOptions )
	{
		pDocTypeLaunchOptions = [NSDictionary dictionaryWithObjectsAndKeys:@"--writer", @"com.sun.star.text.TextDocument", @"--calc", @"com.sun.star.sheet.SpreadsheetDocument", @"--impress", @"com.sun.star.presentation.PresentationDocument", @"--draw", @"com.sun.star.drawing.DrawingDocument", @"--math", @"com.sun.star.formula.FormulaProperties", nil];
		if ( pDocTypeLaunchOptions )
			[pDocTypeLaunchOptions retain];
	}

	NSString *pDocTypeKey = [NSString stringWithCharacters:aDocType.getStr() length:aDocType.getLength()];
	if ( pDocTypeLaunchOptions && pDocTypeKey && [pDocTypeKey length] )
	{
		NSString *pLaunchOption = [pDocTypeLaunchOptions objectForKey:pDocTypeKey];
		NSUserDefaults *pDefaults = [NSUserDefaults standardUserDefaults];
		if ( pLaunchOption && pDefaults )
		{
			if ( ![pDefaults stringForKey:DEFAULT_LAUNCH_OPTIONS_KEY] && ![pDefaults arrayForKey:DEFAULT_LAUNCH_OPTIONS_KEY] )
			{
				NSMutableDictionary *pDict = [NSMutableDictionary dictionaryWithCapacity:[pDocTypeLaunchOptions count]];
				if ( pDict )
				{
					BOOL bSetDefaultLaunchOption = NO;
					NSDictionary *pDocTypesOpened = [pDefaults dictionaryForKey:DEFAULT_LAUNCH_OPTIONS_DOC_TYPES_OPENED_KEY];
					if ( pDocTypesOpened )
						[pDict setDictionary:pDocTypesOpened];

					// Get updated count for doc type
					NSInteger nDocTypeCount = 1;
					NSNumber *pDocTypeValue = [pDict objectForKey:pDocTypeKey];
					if ( pDocTypeValue && [pDocTypeValue isKindOfClass:[NSNumber class]] )
					{
						nDocTypeCount = [pDocTypeValue integerValue];
						if ( nDocTypeCount < 1 )
							nDocTypeCount = 1;
						else
							nDocTypeCount++;
					}
					pDocTypeValue = [NSNumber numberWithInteger:nDocTypeCount];
					if ( pDocTypeValue )
						[pDict setObject:pDocTypeValue forKey:pDocTypeKey];
					else
						[pDict removeObjectForKey:pDocTypeKey];

					// Check if any doc type is overwhelming larger than all
					// other doc types
					NSEnumerator *pEnum = [pDict objectEnumerator];
					if ( pEnum )
					{
						NSInteger nTotalDocTypeCount = 0;
						NSNumber *pValue = nil;
						while ( ( pValue = [pEnum nextObject] ) )
						{
							if ( [pValue isKindOfClass:[NSNumber class]] )
							{
								NSInteger nCount = [pValue integerValue];
								if ( nCount > 0 )
									nTotalDocTypeCount += nCount;
							}
						}

						if ( MIN_NUMBER_DOCS_OPENED > 0 && nTotalDocTypeCount > 0 && nTotalDocTypeCount > MIN_NUMBER_DOCS_OPENED )
						{
							float fDocTypeFraction = (float)nDocTypeCount / (float)nTotalDocTypeCount;
							if ( MIN_FRACTION_DOCS_OPENED > 0 && fDocTypeFraction > MIN_FRACTION_DOCS_OPENED )
								bSetDefaultLaunchOption = YES;
						}
					}

					if ( bSetDefaultLaunchOption )
					{
						[pDefaults setObject:pLaunchOption forKey:DEFAULT_LAUNCH_OPTIONS_KEY];
						[pDefaults removeObjectForKey:DEFAULT_LAUNCH_OPTIONS_DOC_TYPES_OPENED_KEY];
					}
					else
					{
						[pDefaults setObject:pDict forKey:DEFAULT_LAUNCH_OPTIONS_DOC_TYPES_OPENED_KEY];
					}
					[pDefaults synchronize];
				}
			}
			else
			{
				[pDefaults removeObjectForKey:DEFAULT_LAUNCH_OPTIONS_DOC_TYPES_OPENED_KEY];
				[pDefaults synchronize];
			}
		}
	}

	[pPool release];
}
