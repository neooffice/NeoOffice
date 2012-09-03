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
 * Modified September 2012 by Patrick Luby. NeoOffice is distributed under
 * GPL only under modification term 2 of the LGPL.
 *
 ************************************************************************/

#ifndef _OSL_SOCKETIMPL_H_
#define _OSL_SOCKETIMPL_H_

#include <osl/pipe.h>
#include <osl/socket.h>
#include <osl/interlck.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void* (*oslCloseCallback) (void*);
	
struct oslSocketImpl {
    int					m_Socket;
    int                 m_nLastError;
	oslCloseCallback	m_CloseCallback;
	void*				m_CallbackArg;
	oslInterlockedCount m_nRefCount;
#if defined(LINUX)
    sal_Bool            m_bIsAccepting;
    sal_Bool            m_bIsInShutdown;
#endif
};

struct oslSocketAddrImpl
{
	sal_Int32 m_nRefCount;
	struct sockaddr m_sockaddr;
};

struct oslPipeImpl {
	int	 m_Socket;
	sal_Char m_Name[PATH_MAX + 1];
	oslInterlockedCount m_nRefCount;
	sal_Bool m_bClosed;
#if defined(LINUX)
    sal_Bool m_bIsAccepting;
    sal_Bool m_bIsInShutdown;
#endif
#ifdef USE_JAVA
	sal_Char m_RealName[PATH_MAX + 1];
#endif	/* USE_JAVA */
};
	
oslSocket __osl_createSocketImpl(int Socket);
void __osl_destroySocketImpl(oslSocket pImpl);

#ifdef __cplusplus
}
#endif

#endif 

