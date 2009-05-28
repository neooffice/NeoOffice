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

#include "premac.h"
#import <Cocoa/Cocoa.h>
#include "postmac.h"

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

extern const NSString *kNeoMobileXPosPref;
extern const NSString *kNeoMobileYPosPref;
extern const NSString *kNeoMobileVisiblePref;

::rtl::OUString NSStringToOUString( NSString *pString );

#endif	// _NEOMOBILE_HXX
