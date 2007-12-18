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
 *  Patrick Luby, December 2007
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2007 Planamesa Inc.
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_extensions.hxx"

#include <plugin/impl.hxx>

using namespace rtl;
using namespace com::sun::star::uno;
using namespace com::sun::star::plugin;

// ============================================================================

Sequence< PluginDescription > XPluginManager_Impl::getPluginDescriptions() throw()
{
	static Sequence< PluginDescription > aDescs;

	if ( !aDescs.getLength() )
	{
		// Since we don't need mime types to load WebKit plugins, add a generic
		// mime type that includes the list of mime type substrings in the
		// SvxPluginFileDlg::IsAvailable method in the
		// svx/source/dialog/pfiledlg.cxx file
		aDescs.realloc( 1 );
		aDescs[ 0 ].PluginName = OUString( RTL_CONSTASCII_USTRINGPARAM( "Webkit" ) );
		aDescs[ 0 ].Mimetype = OUString( RTL_CONSTASCII_USTRINGPARAM( "application/x-webkit-audio-video" ) );
	}

	return aDescs;
}
