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
 *  Patrick Luby, August 2022
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2022 Planamesa Inc.
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

#ifndef INCLUDED_VCL_A11Y_H
#define INCLUDED_VCL_A11Y_H

#if defined USE_JAVA && MACOSX

// Uncomment the following define to only create AquaA11yWrapper instances on
// the main thread when AquaA11yWrapper is a subclass of NSView.
// Note: if you change this, you will need to rebuild the following custom
// modules as they include this file:
//   accessibility
//   sw
//#define USE_ONLY_MAIN_THREAD_TO_CREATE_AQUAA11YWRAPPERS

#endif	// USE_JAVA && MACOSX

#endif	// INCLUDED_VCL_A11Y_H
