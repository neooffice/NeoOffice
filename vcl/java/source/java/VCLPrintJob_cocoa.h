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
 *  Patrick Luby, September 2005
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2005 Planamesa Inc.
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

#ifndef __VCLPRINTJOB_COCOA_H__
#define __VCLPRINTJOB_COCOA_H__

#include <com/sun/star/vcl/VCLGraphics.hxx>

#ifndef __OBJC__
typedef void* id;
#endif	// !__OBJC__

#ifdef __cplusplus
BEGIN_C
#endif
SAL_DLLPRIVATE sal_Bool NSPrintInfo_pageRange( id pNSPrintInfo, int *nFirst, int *nLast );
SAL_DLLPRIVATE float NSPrintInfo_scale( id pNSPrintInfo );
SAL_DLLPRIVATE id NSPrintInfo_showPrintDialog( id pNSPrintInfo, id pNSWindow, CFStringRef aJobName );
SAL_DLLPRIVATE void NSPrintPanel_abortJob( id pDialog );
SAL_DLLPRIVATE void NSPrintPanel_endJob( id pDialog );
SAL_DLLPRIVATE void NSPrintPanel_endPage( id pDialog );
SAL_DLLPRIVATE sal_Bool NSPrintPanel_finished( id pDialog );
SAL_DLLPRIVATE id NSPrintPanel_printOperation( id pDialog );
SAL_DLLPRIVATE void NSPrintPanel_release( id pDialog );
SAL_DLLPRIVATE ::vcl::com_sun_star_vcl_VCLGraphics *NSPrintPanel_startPage( id pDialog );
#ifdef __cplusplus
END_C
#endif

#endif
