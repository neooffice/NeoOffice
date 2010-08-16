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
 * Modified August 2010 by Patrick Luby. NeoOffice is distributed under
 * GPL only under modification term 2 of the LGPL.
 *
 ************************************************************************/

/* Information:
 * This class implements a mechanism to lock a users installation directory,
 * which is necessesary because instances of staroffice could be running on 
 * different hosts while using the same directory thus causing data 
 * inconsistency.
 * When an existing lock is detected, the user will be asked whether he wants
 * to continue anyway, thus removing the lock and replacing it with a new one
 *
 * ideas:
 * - store information about user and host and time in the lockfile and display 
 * these when asking whether to remove the lockfile.
 * - periodically check the lockfile and warn the user when it gets replaced
 *
 */

#include "sal/types.h"
#include "rtl/ustring.hxx"

class ByteString;

#define LOCKFILE_SUFFIX   rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "/.lock" ) )
#define LOCKFILE_GROUP    ByteString( "Lockdata" )
#define LOCKFILE_USERKEY  ByteString( "User" )
#define LOCKFILE_HOSTKEY  ByteString( "Host" )
#define LOCKFILE_STAMPKEY ByteString( "Stamp" )
#define LOCKFILE_TIMEKEY  ByteString( "Time" )
#define LOCKFILE_IPCKEY   ByteString( "IPCServer" )

namespace desktop {

	class Lockfile;
	bool Lockfile_execWarning( Lockfile * that );

	class Lockfile
	{
	public:

		// contructs a new lockfile onject
		Lockfile( bool bIPCserver = true );

        // separating GUI code:
        typedef bool (* fpExecWarning)( Lockfile * that );
        
		// checks the lockfile, asks user when lockfile is 
		// found (iff gui) and returns false when we may not continue
		sal_Bool check( fpExecWarning execWarning );

		// removes the lockfile. should only be called in exceptional situations
		void clean(void);

#ifdef USE_JAVA
		sal_Bool isLocked(void) const { return m_bIsLocked; }
#endif	// USE_JAVA

		// removes the lockfile
		~Lockfile(void);

	private:
		bool m_bIPCserver;
		// full qualified name (file://-url) of the lockfile
		rtl::OUString m_aLockname;
		// flag whether the d'tor should delete the lock
		sal_Bool m_bRemove;
		sal_Bool m_bIsLocked;
		// ID 
		rtl::OUString m_aId;
		rtl::OUString m_aDate;
		// access to data in file
		void syncToFile(void) const;
		sal_Bool isStale(void) const;
        friend bool Lockfile_execWarning( Lockfile * that );

	};

}
