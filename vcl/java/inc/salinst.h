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
 *  Copyright 2003 by Patrick Luby (patrick.luby@planamesa.com)
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

#ifndef _SV_SALINST_H
#define _SV_SALINST_H

#ifndef _SV_SV_H
#include <sv.h>
#endif
#ifndef _VOS_MUTEX_HXX
#include <vos/mutex.hxx>
#endif
#ifndef _VOS_THREAD_HXX
#include <vos/thread.hxx>
#endif

// -----------------
// - SalYieldMutex -
// -----------------

class SalYieldMutex : public ::vos::OMutex
{
	ULONG				mnCount;
	::vos::OThread::TThreadIdentifier	mnThreadId;

public: 
						SalYieldMutex();
	virtual void		acquire();
	virtual void		release();
	virtual sal_Bool	tryToAcquire();  
	ULONG				GetAcquireCount() { return mnCount; }
	::vos::OThread::TThreadIdentifier	GetThreadId() { return mnThreadId; }
};

// -------------------
// - SalInstanceData -
// -------------------

class SalInstanceData
{
public:
	SalYieldMutex*		mpSalYieldMutex;
	void*				mpFilterInst;
	void*				mpFilterCallback;

						SalInstanceData();
						~SalInstanceData();
};

#endif // _SV_SALINST_H
