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
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
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
 *  =================================================
 *  Modified June 2004 by Patrick Luby. SISSL Removed. NeoOffice is
 *  distributed under GPL only under modification term 3 of the LGPL.
 *
 *  Contributor(s): _______________________________________
 *
 ************************************************************************/

 #ifndef _OSL_UUNXAPI_H_
 #include "uunxapi.h"
 #endif 
 
 #ifndef _LIMITS_H
 #include <limits.h>
 #endif
  
 #ifndef _RTL_USTRING_HXX_
 #include <rtl/ustring.hxx>
 #endif
 
 #ifndef _OSL_THREAD_H_
 #include <osl/thread.h>
 #endif

 #ifdef MACOSX
 /* All Mac OS X paths are UTF-8 */
 #define osl_getThreadTextEncoding() RTL_TEXTENCODING_UTF8
 #endif 
 
 /***********************************
  access_u 
  **********************************/
  
 int access_u(const rtl_uString* pustrPath, int mode)
 {
 	rtl::OString p = rtl::OUStringToOString(
		rtl::OUString(const_cast<rtl_uString*>(pustrPath)),
		osl_getThreadTextEncoding());
		
	return access(p.getStr(), mode);
 }
 
 /***********************************
  realpath_u
  **********************************/
  
 sal_Bool realpath_u(const rtl_uString* pustrFileName, rtl_uString** ppustrResolvedName)
 {
 	rtl::OString fn = rtl::OUStringToOString(
		rtl::OUString(const_cast<rtl_uString*>(pustrFileName)),
		osl_getThreadTextEncoding());
		
	char  rp[PATH_MAX];
	bool  bRet = realpath(fn.getStr(), rp); 
	
	if (bRet)
	{
		rtl::OUString resolved = rtl::OStringToOUString(
			rtl::OString(static_cast<sal_Char*>(rp)),
			osl_getThreadTextEncoding());
			
		rtl_uString_assign(ppustrResolvedName, resolved.pData);
	}
	return bRet;
 }
 
 /***********************************
  lstat_u
  **********************************/
  
 int lstat_u(const rtl_uString* pustrPath, struct stat* buf)
 {
 	rtl::OString p = rtl::OUStringToOString(
		rtl::OUString(const_cast<rtl_uString*>(pustrPath)),
		osl_getThreadTextEncoding());
	
	return lstat(p.getStr(), buf);
 } 
