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
 *  Patrick Luby, July 2005
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2005 by Patrick Luby (patrick.luby@planamesa.com)
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

#import <Cocoa/Cocoa.h>
#import "VCLFrame_cocoa.h"

static NSWindow *GetNSWindow( void *pCWindow )
{
	SEL aSelector = @selector(getNSWindow);
	if ( pCWindow && [(NSObject *)pCWindow respondsToSelector:aSelector] )
		return (NSWindow *)[(NSObject *)pCWindow performSelector:aSelector];
	else
		return NULL;
}

void CWindow_setVisible( void *pCWindow, BOOL bVisible, BOOL bEnable )
{
	NSWindow *pNSWindow = GetNSWindow( pCWindow );
	if ( pNSWindow )
	{
		if ( bVisible )
			[pNSWindow orderFront:pNSWindow];
		else
			[pNSWindow orderOut:pNSWindow];
	}
}

void CWindow_toFront( void *pCWindow )
{
	NSWindow *pNSWindow = GetNSWindow( pCWindow );
	if ( pNSWindow )
		[pNSWindow orderFront:pNSWindow];
}

WindowRef CWindow_windowRef( void *pCWindow )
{
	WindowRef aWindow = NULL;

	NSWindow *pNSWindow = GetNSWindow( pCWindow );
	if ( pNSWindow )
		aWindow = [pNSWindow windowRef];

	return aWindow;
}
