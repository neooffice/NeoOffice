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
 *  Copyright 2012 by Planamesa Inc.
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

#ifndef _UPDATE_JAVA_HXX
#define _UPDATE_JAVA_HXX

#ifdef __OBJC__
#import <set>
#import <rtl/ustring.hxx>
#else	// __OBJC__
#include <set>
#include <rtl/ustring.hxx>
#endif	// __OBJC__

SAL_DLLPRIVATE void UpdateAddInstallerPackage(rtl::OUString aName, rtl::OUString aDownloadPath, rtl::OUString aPackagePath);
SAL_DLLPRIVATE void UpdateInstallNextBatchOfInstallerPackagePaths();
SAL_DLLPRIVATE void UpdateShutdownApp();

#endif	// _UPDATE_JAVA_HXX
