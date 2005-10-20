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

#ifndef __VCLPAGEFORMAT_COCOA_H__
#define __VCLPAGEFORMAT_COCOA_H__

#ifdef __cplusplus
typedef void* id;
#endif

#ifdef __cplusplus
BEGIN_C
#endif
BOOL NSPageLayout_finished( id pDialog );
BOOL NSPageLayout_result( id pDialog );
id NSPrintInfo_create();
void NSPrintInfo_installVCLPrintInfo();
void NSPrintInfo_setInDialog( BOOL bIn );
void NSPrintInfo_setSharedPrintInfo( id pNSPrintInfo );
id NSPrintInfo_showPageLayoutDialog( id pNSPrintInfo, id pNSWindow, BOOL bLandscape );
#ifdef __cplusplus
END_C
#endif

#endif
