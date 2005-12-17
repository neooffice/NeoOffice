/*************************************************************************
 *
 *  $RCSfile$
 *
 *  $Revision$
 *
 *  last change: $Author$ $Date$
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU General Public License Version 2.1.
 *
 *
 *    GNU General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 *    Modified December 2005 by Patrick Luby. NeoOffice is distributed under
 *    GPL only under modification term 3 of the LGPL.
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
 #include "system.h"
 #endif
 
 //###########################
 inline rtl::OString OUStringToOString(const rtl_uString* s)
 {
    return rtl::OUStringToOString(
        rtl::OUString(const_cast<rtl_uString*>(s)),
        osl_getThreadTextEncoding());
 }
 
 //###########################
 //access_u     
 int access_u(const rtl_uString* pustrPath, int mode)
 { 		
 #ifdef MACOSX
	::rtl::OString p = OUStringToOString(pustrPath);
	sal_Char path[ PATH_MAX ];
	if ( p.getLength() < PATH_MAX )
	{
		strcpy( path, p.getStr() );
		macxp_resolveAlias( path, PATH_MAX, sal_False );
		p = rtl::OString( path );
	}
	return access(p.getStr(), mode);
 #else	/* MACOSX */
	return access(OUStringToOString(pustrPath).getStr(), mode);
 #endif	/* MACOSX */
 }
 
 //#########################
 //realpath_u  
 sal_Bool realpath_u(const rtl_uString* pustrFileName, rtl_uString** ppustrResolvedName)
 {
 	rtl::OString fn = rtl::OUStringToOString(
		rtl::OUString(const_cast<rtl_uString*>(pustrFileName)),
		osl_getThreadTextEncoding());
		
 #ifdef MACOSX
	sal_Char path[ PATH_MAX ];
	if ( fn.getLength() < PATH_MAX )
	{
		strcpy( path, fn.getStr() );
		macxp_resolveAlias( path, PATH_MAX, sal_False );
		fn = rtl::OString( path );
	}
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
 #ifdef MACOSX
	::rtl::OString p = OUStringToOString(pustrPath);
	sal_Char path[ PATH_MAX ];
	if ( p.getLength() < PATH_MAX )
	{
		strcpy( path, p.getStr() );
		macxp_resolveAlias( path, PATH_MAX, sal_False );
		p = rtl::OString( path );
	}
	return lstat(p.getStr(), buf);
 #else	/* MACOSX */
	return lstat(OUStringToOString(pustrPath).getStr(), buf);
 #endif	/* MACOSX */
 } 
 
 //#########################
 // @see mkdir
 int mkdir_u(const rtl_uString* path, mode_t mode)
 {    
 #ifdef MACOSX
	::rtl::OString p = OUStringToOString(path);
	sal_Char tmppath[ PATH_MAX ];
	if ( p.getLength() < PATH_MAX )
	{
		strcpy( tmppath, p.getStr() );
		macxp_resolveAlias( tmppath, PATH_MAX, sal_False );
		p = rtl::OString( tmppath );
	}
	return mkdir(p.getStr(), mode);
 #else	/* MACOSX */
    return mkdir(OUStringToOString(path).getStr(), mode);     
 #endif	/* MACOSX */
 }
 
