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
 * Modified August 2008 by Patrick Luby. NeoOffice is distributed under
 * GPL only under modification term 2 of the LGPL.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_desktop.hxx"

#include "app.hxx"

#include <rtl/logfile.hxx>
#include <tools/extendapplicationenvironment.hxx>

#if defined USE_JAVA && defined MACOSX
#include "sal/main.h"
#endif	// USE_JAVA && MACOSX

BOOL SVMain();

// -=-= main() -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

#if defined USE_JAVA && defined MACOSX
// All references to main() need to be redefined to soffice_main()
#define main soffice_main
extern "C"
{
SAL_IMPLEMENT_MAIN_WITH_ARGS( argc, argv )
#undef main
#else	// USE_JAVA && MACOSX
extern "C" int soffice_main()
#endif	// USE_JAVA && MACOSX
{
    tools::extendApplicationEnvironment();

	RTL_LOGFILE_PRODUCT_TRACE( "PERFORMANCE - enter Main()" );

	desktop::Desktop aDesktop;
    // This string is used during initialization of the Gtk+ VCL module
    aDesktop.SetAppName( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("soffice")) );
    SVMain();

#if defined USE_JAVA && defined MACOSX
    // Force exit since some JVMs won't shutdown when only exit() is invoked
    _exit( 0 );
#else	// USE_JAVA && MACOSX
    return 0;
#endif	// USE_JAVA && MACOSX
}
#if defined USE_JAVA && defined MACOSX
}
#endif	// USE_JAVA && MACOSX
