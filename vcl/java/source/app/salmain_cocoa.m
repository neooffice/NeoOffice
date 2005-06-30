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
 *  Patrick Luby, June 2003
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2003 by Patrick Luby (patrick.luby@planamesa.com)
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

#define _SV_SALMAIN_COCOA_M

#import <salmain_cocoa.h>
#import <AppKit/AppKit.h>

// ============================================================================

void InitCocoa()
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	[NSApplication sharedApplication];

	[pPool release];
}

// ----------------------------------------------------------------------------

void RunCocoaEventLoop()
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	[NSApp run];

	[pPool release];
}

// ----------------------------------------------------------------------------

void StopCocoaEventLoop()
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( [NSApp isRunning] )
	{
		[NSApp stop:NSApp];

		// Post an event to wake up the event thread
		NSPoint aPoint;
		aPoint.x = 0;
		aPoint.y = 0;
		NSEvent *pEvent = [NSEvent otherEventWithType:NSApplicationDefined location:aPoint modifierFlags:0 timestamp:[[NSDate date] timeIntervalSince1970] windowNumber:0 context:nil subtype:0 data1:0 data2:0];
		if ( pEvent )
			[NSApp postEvent:pEvent atStart:YES];
	}

	[pPool release];
}
