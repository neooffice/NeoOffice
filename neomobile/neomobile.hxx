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
 *  Copyright 2009 by Planamesa Inc.
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

#ifndef _NEOMOBILE_HXX
#define _NEOMOBILE_HXX

#import <rtl/ustring.hxx>

#include "premac.h"
#import <Cocoa/Cocoa.h>
#include "postmac.h"

extern const NSString *kAboutURL;
extern const NSString *kLoginURI;
extern const NSString *kNeoMobileLastURLPref;
extern const NSString *kNeoMobileXPosPref;
extern const NSString *kNeoMobileYPosPref;
extern const NSString *kNeoMobileWidthPref;
extern const NSString *kNeoMobileHeightPref;
extern const NSString *kNeoMobileVisiblePref;
extern const NSString *kNeoMobileServerTypePref;
extern const NSString *kOpenURI;

@interface CreateWebViewImpl : NSObject
{
	const NSString*				mpURI;
	const NSString*				mpUserAgent;
}
+ (id)createWithURI:(const NSString *)pURI userAgent:(const NSString *)pUserAgent;
- (id)initWithURI:(const NSString *)pURI userAgent:(const NSString *)pUserAgent;
- (void)showWebView:(id)obj;
- (void)showWebViewOnlyIfVisible:(id)obj;
@end

NSArray *GetPerformSelectorOnMainThreadModes();
NSString *GetUserAgent();
::rtl::OUString NSStringToOUString( NSString *pString );
::sal_Bool IsSupportedMacOSXVersion();
::sal_Bool ZipDirectory( const rtl::OUString& dirPath, const rtl::OUString& zipFilePath );

#endif	// _NEOMOBILE_HXX
