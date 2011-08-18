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
 * Modified October 2008 by Patrick Luby. NeoOffice is distributed under
 * GPL only under modification term 2 of the LGPL.
 *
 ************************************************************************/

#ifndef INCLUDED_DESKTOP_SOURCE_PKGCHK_UNOPKG_UNOPKG_MAIN_H
#define INCLUDED_DESKTOP_SOURCE_PKGCHK_UNOPKG_UNOPKG_MAIN_H

#include "sal/config.h"

#if defined __cplusplus
extern "C" {
#endif

#if defined USE_JAVA && defined MACOSX
int unopkg_main( int argc, char **argv );
#else	// USE_JAVA && MACOSX
int unopkg_main(void);
#endif	// USE_JAVA && MACOSX

#if defined __cplusplus
}
#endif

#endif
