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

#define _SV_SALTIMER_CXX

#ifndef _SV_SALTIMER_HXX
#include <saltimer.hxx>
#endif
#ifndef _SV_SALDATA_HXX
#include <saldata.hxx>
#endif
#ifndef _SV_SALFRAME_HXX
#include <salframe.hxx>
#endif

// =======================================================================

void SalTimer::SetCallback( SALTIMERPROC pProc )
{
	GetSalData()->mpTimerProc = pProc;
}

// -----------------------------------------------------------------------

void SalTimer::Start( ULONG nMS )
{
	SalData *pSalData = GetSalData();
	gettimeofday( &pSalData->maTimeout, NULL );
	pSalData->maTimeout += nMS;
	pSalData->mnTimerInterval = nMS;

	// Wakeup the event queue by sending it a dummy event
	SalFrame *pFrame = pSalData->mpFirstFrame;
	if ( pFrame )
		pFrame->PostEvent( NULL );
} 

// -----------------------------------------------------------------------
  
void SalTimer::Stop()
{
	SalData *pSalData = GetSalData();
	pSalData->mnTimerInterval = 0;
	pSalData->maTimeout.tv_sec = 0;
	pSalData->maTimeout.tv_usec = 0;
}
