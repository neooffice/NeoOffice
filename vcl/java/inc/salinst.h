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

#ifndef _SV_SALINST_H
#define _SV_SALINST_H

#ifndef _SV_SALINST_HXX
#include <salinst.hxx>
#endif
#ifndef _SV_SV_H
#include <sv.h>
#endif
#ifndef _SV_SVAPP_HXX
#include <svapp.hxx>
#endif
#ifndef _OSL_CONDITN_HXX
#include <osl/conditn.hxx>
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
	ULONG					mnCount;
	::osl::Condition		maMainThreadCondition;
	::vos::OThread::TThreadIdentifier	mnThreadId;

public:
							SalYieldMutex();
	virtual void			acquire();
	virtual void			release();
	virtual sal_Bool		tryToAcquire();
	ULONG					GetAcquireCount() { return mnCount; }
	::vos::OThread::TThreadIdentifier	GetThreadId() { return mnThreadId; }
};

// -------------------
// - JavaSalInstance -
// -------------------

class JavaSalInstance : public SalInstance
{
public:
	SalYieldMutex*			mpSalYieldMutex;

							JavaSalInstance();
	virtual					~JavaSalInstance();

	virtual SalFrame*		CreateChildFrame( SystemParentData* pParent, ULONG nStyle );
	virtual SalFrame*		CreateFrame( SalFrame* pParent, ULONG nStyle );
	virtual void			DestroyFrame( SalFrame* pFrame );
	virtual SalObject*		CreateObject( SalFrame* pParent, SystemWindowData* pWindowData );
	virtual void			DestroyObject( SalObject* pObject );
	virtual SalVirtualDevice*	CreateVirtualDevice( SalGraphics* pGraphics, long nDX, long nDY, USHORT nBitCount, const SystemGraphicsData *pData = NULL );
	virtual void			DestroyVirtualDevice( SalVirtualDevice* pDevice );
	virtual SalInfoPrinter* CreateInfoPrinter( SalPrinterQueueInfo* pQueueInfo, ImplJobSetup* pSetupData );
	virtual void			DestroyInfoPrinter( SalInfoPrinter* pPrinter );
	virtual SalPrinter*		CreatePrinter( SalInfoPrinter* pInfoPrinter );
	virtual void			DestroyPrinter( SalPrinter* pPrinter );

	virtual void			GetPrinterQueueInfo( ImplPrnQueueList* pList );
	virtual void			GetPrinterQueueState( SalPrinterQueueInfo* pInfo );
	virtual void			DeletePrinterQueueInfo( SalPrinterQueueInfo* pInfo );
	virtual String			GetDefaultPrinter();
	virtual SalSound*		CreateSalSound();
	virtual SalTimer*		CreateSalTimer();
	virtual SalOpenGL*		CreateSalOpenGL( SalGraphics* pGraphics );
	virtual SalI18NImeStatus*	CreateI18NImeStatus();
	virtual SalSystem*		CreateSalSystem();
	virtual SalBitmap*		CreateSalBitmap();
	virtual vos::IMutex*	GetYieldMutex();
	virtual ULONG			ReleaseYieldMutex();
	virtual void			AcquireYieldMutex( ULONG nCount );
	virtual void			Yield( bool bWait, bool bHandleAllCurrentEvents );
	virtual bool			AnyInput( USHORT nType );
	virtual SalMenu*		CreateMenu( BOOL bMenuBar, Menu* pVCLMenu );
	virtual void			DestroyMenu( SalMenu* pMenu);
	virtual SalMenuItem*	CreateMenuItem( const SalItemParams* pItemData );
	virtual void			DestroyMenuItem( SalMenuItem* pItem );
	virtual SalSession*		CreateSalSession();
	virtual void*			GetConnectionIdentifier( ConnectionIdentifierType& rReturnedType, int& rReturnedBytes );
};

void InitJavaAWT();

#endif // _SV_SALINST_H
