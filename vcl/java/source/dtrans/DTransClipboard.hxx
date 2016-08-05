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
 *  Patrick Luby, June 2003
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2003 Planamesa Inc.
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

#ifndef _DTRANSCLIPBOARD_HXX
#define	_DTRANSCLIPBOARD_HXX

#ifndef _DTRANSTRANSFERABLE_HXX
#include "DTransTransferable.hxx"
#endif

namespace java {

class DTransClipboard
{
public:
	static DTransTransferable*	getContents();
	static DTransTransferable*	setContents( const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable > &xTransferable );

						DTransClipboard() {};
};

} // namespace java

#endif // _DTRANSCLIPBOARD_HXX
