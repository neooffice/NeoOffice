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
 *  Edward Peterlin, April 2007
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
#include "objmisc_cocoa.h"

/**
 * Perform a SetWindowModified on a WindowRef that has been extracted from
 * a Cocoa window.
 */
void DoCocoaSetWindowModifiedBit( unsigned long winRef, bool isModified )
{
	if(winRef)
	{
		NSEnumerator *windowIter=[[NSApp windows] objectEnumerator];
		id winObject;
		
		while(winObject=[windowIter nextObject])
		{
			NSWindow *theWin=(NSWindow *)winObject;
			if([theWin windowRef]==(WindowRef)winRef)
			{
				[theWin setDocumentEdited: ((isModified) ? YES : NO)];
				break;
			}
		}
	}
}