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
 *  Patrick Luby, June 2007
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

#ifndef _SFXX11PRODUCTCHECK_HXX
#include <X11productcheck.hxx>
#endif
#ifndef _OSL_MODULE_HXX_
#include <osl/module.hxx>
#endif
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef DLLPOSTFIX
#error DLLPOSTFIX must be defined in makefile.mk
#endif

#ifndef _OSL_MODULE_HXX_
#include <osl/module.hxx>
#endif

#define DOSTRING( x )			#x
#define STRING( x )				DOSTRING( x )

static ::osl::Module aVCLModule;

bool IsX11Product()
{
    if ( !aVCLModule.is() )
    {
        ::rtl::OUString aLibName = ::rtl::OUString::createFromAscii( "libvcl" );
        aLibName += ::rtl::OUString::valueOf( (sal_Int32)SUPD, 10 );
        aLibName += ::rtl::OUString::createFromAscii( STRING( DLLPOSTFIX ) );
        aLibName += ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".dylib" ) );
		aVCLModule.load( aLibName );
    }
    if ( aVCLModule.is() && aVCLModule.getSymbol( ::rtl::OUString::createFromAscii( "XOpenDisplay" ) ) )
        return true;
    else
        return false;
}
