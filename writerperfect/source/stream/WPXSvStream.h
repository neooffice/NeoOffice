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
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2002-2003 William Lachance (william.lachance@sympatico.ca)
 *  http://libwpd.sourceforge.net
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
 *  Modified November 2004 by Patrick Luby. SISSL Removed. NeoOffice is
 *  distributed under GPL only under modification term 3 of the LGPL.
 *
 *  Contributor(s): _______________________________________
 *
 ************************************************************************/

#ifndef WPXSVSTREAM_H
#define WPXSVSTREAM_H

#include <sot/storage.hxx>
#include <com/sun/star/io/XInputStream.hpp>

#include <libwpd/WPXStream.h>

class WPXSvInputStream : public WPXInputStream
{
public:
	WPXSvInputStream( ::com::sun::star::uno::Reference<
					  ::com::sun::star::io::XInputStream > xStream );
	virtual ~WPXSvInputStream();

	virtual bool isOLEStream();
	virtual WPXInputStream * getDocumentOLEStream();

	virtual const uint8_t *read(size_t numBytes);
	virtual int seek(long offset, WPX_SEEK_TYPE seekType);
	virtual long tell();
	virtual bool atEOS();

private:
	SotStorageRef       mxChildStorage;
	SotStorageStreamRef mxChildStream;
	::com::sun::star::uno::Reference<
			  ::com::sun::star::io::XInputStream > mxStream;
	::com::sun::star::uno::Sequence< sal_Int8 > maData;
	sal_Int64 mnOffset;
	sal_Int64 mnLength;
};

#endif
