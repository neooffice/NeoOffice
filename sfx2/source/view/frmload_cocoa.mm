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
 *  Patrick Luby, December 2013
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2013 Planamesa Inc.
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
		pDocTypeLaunchOptions = [NSDictionary dictionaryWithObjectsAndKeys:@"-writer", @"com.sun.star.text.TextDocument", @"-calc", @"com.sun.star.sheet.SpreadsheetDocument", @"-impress", @"com.sun.star.presentation.PresentationDocument", @"-draw", @"com.sun.star.drawing.DrawingDocument", @"-math", @"com.sun.star.formula.FormulaProperties", nil];
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
