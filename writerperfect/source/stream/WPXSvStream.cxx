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
 *  Copyright 2004-2005 Michael Meeks (mmeeks@novell.com)
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

#include "WPXSvStream.h"

#include <tools/stream.hxx>
#include <unotools/streamwrap.hxx>
#include <unotools/ucbstreamhelper.hxx>

#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_H_
#include <com/sun/star/io/XSeekable.hpp>
#endif

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::io;

WPXSvInputStream::WPXSvInputStream( Reference< XInputStream > xStream ) :
		WPXInputStream(true),
		mxChildStorage(),
		mxChildStream(),
		mxStream(xStream),
		mnOffset(0)
{
	Reference < XSeekable> xSeekable = Reference < XSeekable > (xStream, UNO_QUERY);
	if (!xSeekable.is())
		mnLength = 0;
	else
		mnLength = xSeekable->getLength(); // exception
}

WPXSvInputStream::~WPXSvInputStream()
{
}

const uint8_t * WPXSvInputStream::read(size_t numBytes, size_t &numBytesRead)
{
	// FIXME: assume no short reads (?)
	sal_Int64 oldMnOffset = mnOffset;
	mnOffset += mxStream->readBytes (maData, numBytes);
	numBytesRead = mnOffset - oldMnOffset;
	return (const uint8_t *)maData.getConstArray();
}

int WPXSvInputStream::seek(long offset, WPX_SEEK_TYPE seekType) 
{
	if (seekType == WPX_SEEK_CUR && offset >= 0)
	{
			mxStream->skipBytes (offset); // exception ?
			mnOffset += offset;
			return FALSE;
	}
	Reference < XSeekable> xSeekable = Reference < XSeekable >(mxStream, UNO_QUERY);

	if (!xSeekable.is())
			return TRUE;
	
	if (seekType == WPX_SEEK_CUR)
			mnOffset += offset;
	else
			mnOffset = offset;

	xSeekable->seek(mnOffset); // FIXME: catch exception!
	
	return FALSE;
}

long WPXSvInputStream::tell()
{
	return mnOffset;
}

bool WPXSvInputStream::atEOS()
{
	return mnOffset >= mnLength;
}

bool WPXSvInputStream::isOLEStream()
{
	bool bAns;

	SvStream *pStream = utl::UcbStreamHelper::CreateStream( mxStream );
	bAns = pStream && SotStorage::IsOLEStorage( pStream );
	delete pStream;

	seek (0, WPX_SEEK_SET);

	return bAns;
}

WPXInputStream * WPXSvInputStream::getDocumentOLEStream()
{
	SvStream *pStream = utl::UcbStreamHelper::CreateStream( mxStream );

	mxChildStorage = new SotStorage( pStream, TRUE );

	mxChildStream = mxChildStorage->OpenSotStream(
			rtl::OUString::createFromAscii( "PerfectOffice_MAIN" ),
			STREAM_STD_READ );

	Reference < XInputStream > xContents = new utl::OSeekableInputStreamWrapper( mxChildStream );
	if (xContents.is())
		return new WPXSvInputStream( xContents );
	else
		return NULL;
}
