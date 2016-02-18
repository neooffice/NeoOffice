/**************************************************************
 * 
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 * 
 * This file incorporates work covered by the following license notice:
 * 
 *   Modified February 2016 by Patrick Luby. NeoOffice is only distributed
 *   under the GNU General Public License, Version 3 as allowed by Section 4
 *   of the Apache License, Version 2.0.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * 
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_desktop.hxx"

#include "app.hxx"

#include <rtl/logfile.hxx>
#include <tools/extendapplicationenvironment.hxx>

#if defined USE_JAVA && defined MACOSX
#include "sal/main.h"
#endif	// USE_JAVA && MACOSX

sal_Bool SVMain();

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
