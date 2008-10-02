/*************************************************************************
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * $RCSfile$
 * $Revision$
 *
 * This file is part of NeoOffice.
 *
 * NeoOffice is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * NeoOffice is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU General Public License
 * version 3 along with NeoOffice.  If not, see
 * <http://www.gnu.org/licenses/gpl-3.0.txt>
 * for a copy of the GPLv3 License.
 *
 * Modified June 2004 by Patrick Luby. NeoOffice is distributed under
 * GPL only under modification term 2 of the LGPL.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sal.hxx"
 
 #ifndef _OSL_UUNXAPI_H_
 #include "uunxapi.h"
 #endif 

 #ifndef __OSL_SYSTEM_H__
 #include "system.h"
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
 
 //###########################
 inline rtl::OString OUStringToOString(const rtl_uString* s)
 {
    return rtl::OUStringToOString(
        rtl::OUString(const_cast<rtl_uString*>(s)),
        osl_getThreadTextEncoding());
 }
 
 //###########################
#ifdef MACOSX
/*
 * Helper function for resolving Mac native alias files (not the same as unix alias files)  
 * and to return the resolved alias as rtl::OString
 */
 inline rtl::OString macxp_resolveAliasAndConvert(const rtl_uString* s)
 {
  rtl::OString p = OUStringToOString(s);
  sal_Char path[PATH_MAX];
  if (p.getLength() < PATH_MAX)
    {
      strcpy(path, p.getStr());
#ifdef USE_JAVA
      macxp_resolveAlias(path, PATH_MAX, sal_False);
#else	// USE_JAVA
      macxp_resolveAlias(path, PATH_MAX);
#endif	// USE_JAVA
      p = rtl::OString(path);
    }
  return p;
 }
#endif /* MACOSX */
 
 //###########################
 //access_u     
 int access_u(const rtl_uString* pustrPath, int mode)
 {
#ifndef MACOSX // not MACOSX  
	return access(OUStringToOString(pustrPath).getStr(), mode);
#else
	return access(macxp_resolveAliasAndConvert(pustrPath).getStr(), mode);
#endif
 }
 
 //#########################
 //realpath_u  
 sal_Bool realpath_u(const rtl_uString* pustrFileName, rtl_uString** ppustrResolvedName)
 {
#ifndef MACOSX // not MACOSX  
        rtl::OString fn = OUStringToOString(pustrFileName);
#else
	rtl::OString fn = macxp_resolveAliasAndConvert(pustrFileName);
#endif
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
 
 //#########################
 //lstat_u 
  int lstat_u(const rtl_uString* pustrPath, struct stat* buf)
 { 	
#ifndef MACOSX  // not MACOSX  
	return lstat(OUStringToOString(pustrPath).getStr(), buf);
#else
	return lstat(macxp_resolveAliasAndConvert(pustrPath).getStr(), buf);
#endif
 } 
 
 //#########################
 // @see mkdir
 int mkdir_u(const rtl_uString* path, mode_t mode)
 {    
#ifdef USE_JAVA
    if ( !macxp_checkCreateDirectory( OUStringToOString( path ).getStr() ) )
    { 
        errno = EACCES;
        return -1;
    } 
	return mkdir(macxp_resolveAliasAndConvert(path).getStr(), mode);
#else	/* USE_JAVA */
    return mkdir(OUStringToOString(path).getStr(), mode);     
#endif  /* USE_JAVA */
 }
 
