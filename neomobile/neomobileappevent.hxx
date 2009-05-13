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
 *  Copyright 2008 by Planamesa Inc.
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

#ifndef _NEOMOBILEAPPEVENT_HXX
#define _NEOMOBILEAPPEVENT_HXX

#include <premac.h>
#include <Foundation/Foundation.h>
#include <postmac.h>

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

class NeoMobileExportFileAppEvent
{
	int						mnErrorCode;
	NSFileManager*			mpFileManager;
	bool					mbFinished;
	NSMutableData*			mpPostBody;
	::rtl::OUString			maSaveUUID;
	bool					mbCanceled;
	bool					mbUnsupportedComponentType;

public:
							NeoMobileExportFileAppEvent( ::rtl::OUString aSaveUUID, NSFileManager *pFileManager, NSMutableData *pPostBody );
	virtual					~NeoMobileExportFileAppEvent() {};
							DECL_LINK( ExportFile, void* );
	int						GetErrorCode() { return mnErrorCode; }
	bool					IsFinished() { return mbFinished; }
	NSData*					GetPostBody() { return mpPostBody; }
	void					Cancel() { mbCanceled=true; }
	bool					IsCanceled() { return(mbCanceled); }
	bool					IsUnsupportedComponentType() { return(mbUnsupportedComponentType); }
};

#endif	// _NEOMOBILEAPPEVENT_HXX
