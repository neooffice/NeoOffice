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

long CWindow_getNSWindow( long aCWindow )
{
	long out = nil;

	SEL aSelector = @selector(getNSWindow);
	if ( aCWindow && [(NSObject *)aCWindow respondsToSelector:aSelector] )
		out = (long)[(NSObject *)aCWindow performSelector:aSelector];

	return out;
}

void CWindow_makeMainWindow( long aCWindow )
{
	NSWindow *pNSWindow = (NSWindow *)CWindow_getNSWindow( aCWindow );
	if ( pNSWindow )
		[pNSWindow makeMainWindow];
}

void CWindow_orderFront( long aCWindow )
{
	NSWindow *pNSWindow = (NSWindow *)CWindow_getNSWindow( aCWindow );
	if ( pNSWindow )
		[pNSWindow orderFront:pNSWindow];
}

WindowRef CWindow_windowRef( long aCWindow )
{
	WindowRef aWindow = NULL;

	NSWindow *pNSWindow = (NSWindow *)CWindow_getNSWindow( aCWindow );
	if ( pNSWindow )
		aWindow = [pNSWindow windowRef];

	return aWindow;
}
