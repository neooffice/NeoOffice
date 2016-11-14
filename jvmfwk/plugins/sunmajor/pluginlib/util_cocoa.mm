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
 *  Patrick Luby, July 2015
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2015 Planamesa Inc.
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

#import "util_cocoa.hxx"

sal_Bool JvmfwkUtil_isLoadableJVM( OUString aURL )
{
	sal_Bool bRet = sal_False;

	if ( aURL.getLength() )
	{
		NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

		NSString *pString = [NSString stringWithCharacters:aURL.getStr() length:aURL.getLength()];
		if ( pString )
		{
			NSURL *pURL = nil;

			// Ignore all but Oracle's JDK as loading Apple's Java and Oracle's
			// JRE will cause OS X's JavaVM framework to display a dialog and
			// invoke exit() when loaded via JNI on OS X 10.10
			NSURL *pTmpURL = [NSURL URLWithString:pString];
			if ( pTmpURL )
				pTmpURL = [pTmpURL filePathURL];
			if ( pTmpURL )
				pTmpURL = [pTmpURL URLByStandardizingPath];
			if ( pTmpURL )
				pTmpURL = [pTmpURL URLByResolvingSymlinksInPath];
			if ( pTmpURL )
			{
				NSURL *pJVMsDirURL = [NSURL URLWithString:@"file:///Library/Java/JavaVirtualMachines/"];
				if ( pJVMsDirURL )
					pJVMsDirURL= [pJVMsDirURL filePathURL];
				if ( pJVMsDirURL )
					pJVMsDirURL = [pJVMsDirURL URLByStandardizingPath];
				// The JVM directory must not contain softlinks or the JavaVM
				// framework bug will occur so don't resolve softlinks in the
				// JVM directory
				if ( pJVMsDirURL )
				{
					NSString *pTmpURLString = [pTmpURL absoluteString];
					NSString *pJVMsDirURLString = [pJVMsDirURL absoluteString];
					if ( pTmpURLString && pJVMsDirURLString && [pJVMsDirURLString length] )
					{
						NSRange aJVMsDirURLRange = [pTmpURLString rangeOfString:pJVMsDirURLString];
						if ( !aJVMsDirURLRange.location && aJVMsDirURLRange.length )
							pURL = pTmpURL;
					}
				}
			}

			while ( pURL )
			{
				// Check if this is a valid bundle
				NSNumber *pDir = nil;
				NSURL *pContentsURL = [pURL URLByAppendingPathComponent:@"Contents"];
				if ( pContentsURL && [pContentsURL getResourceValue:&pDir forKey:NSURLIsDirectoryKey error:nil] && pDir && [pDir boolValue] )
				{
					NSBundle *pBundle = [NSBundle bundleWithURL:pURL];
					if ( pBundle )
					{
						// Make sure that this bundle's Info.plist has the
						// proper JVM keys to supports loading via JNI. If
						// this bundle is a valid JVM and these keys
						// are missing, loading the JVM will cause OS X's
						// JavaVM framework to display a dialog and invoke
						// exit() when loaded via JNI on OS X 10.10.
						NSDictionary *pInfo = [pBundle infoDictionary];
						if ( pInfo )
						{
							NSDictionary *pJavaVM = [pInfo objectForKey:@"JavaVM"];
							if ( pJavaVM && [pJavaVM isKindOfClass:[NSDictionary class]] )
							{
								NSArray *pJVMCapabilities = [pJavaVM objectForKey:@"JVMCapabilities"];
								if ( pJVMCapabilities )
								{
									if ( [pJVMCapabilities indexOfObjectIdenticalTo:@"JNI"] == NSNotFound )
									{
										if ( [pJVMCapabilities isKindOfClass:[NSMutableArray class]] )
										{
											[(NSMutableArray *)pJVMCapabilities addObject:@"JNI"];
											bRet = sal_True;
										}
										else if ( [pJavaVM isKindOfClass:[NSMutableDictionary class]] )
										{
											NSMutableArray *pNewJVMCapabilities = [NSMutableArray arrayWithCapacity:[pJVMCapabilities count] + 1];
											if ( pNewJVMCapabilities )
											{
												[pNewJVMCapabilities addObject:@"JNI"];
												[(NSMutableDictionary *)pJavaVM setObject:pNewJVMCapabilities forKey:@"JVMCapabilities"];
												bRet = sal_True;
											}
										}
									}
									else
									{
										bRet = sal_True;
									}
								}
							}
						}
					}
				}

				NSURL *pOldURL = pURL;
				pURL = [pURL URLByDeletingLastPathComponent];
				if ( pURL )
				{
					pURL = [pURL URLByStandardizingPath];
					if ( pURL )
					{
						pURL = [pURL URLByResolvingSymlinksInPath];
						if ( pURL && [pURL isEqual:pOldURL] )
							pURL = nil;
					}
				}
			}
		}

		[pPool release];
	}

	return bRet;
}
