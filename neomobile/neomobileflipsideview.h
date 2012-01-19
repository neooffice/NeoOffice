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
 *  Copyright 2011 by Planamesa Inc.
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

#import "neomobilewebview.h"

@interface NonRecursiveResponderFlipsidePanel : NonRecursiveResponderPanel
{
	NeoMobileNonRecursiveResponderWebPanel*	mpWebPanel;
	NSButton*				mpaboutButton;
	NSView*					mpcontentView;
	NSButton*				mpcreateAccountButton;
	NSButton*				mpforgotPasswordButton;
	NSButton*				mploginButton;
	NSSecureTextField*		mppasswordEdit;
	NSText*					mppasswordLabel;
	NSButton*				mpsavePasswordButton;
	NSText*					mptitleLabel;
	NSView*					mptitleView;
	NSTextField*			mpusernameEdit;
	NSText*					mpusernameLabel;
}
- (void)dealloc;
- (IBAction)doAbout;
- (IBAction)doCreateAccount;
- (IBAction)doForgotPassword;
- (IBAction)doLogin;
- (id)initWithWebPanel:(NeoMobileNonRecursiveResponderWebPanel *)pWebPanel;
- (void)orderWindow:(NSWindowOrderingMode)nOrderingMode relativeTo:(int)nOtherWindowNumber;
- (IBAction)storeLoginInfo;
@end

