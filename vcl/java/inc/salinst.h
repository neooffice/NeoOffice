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

#include <salframe.h>
#include <vcl/salinst.hxx>
#include <vcl/sv.h>
#include <vcl/svapp.hxx>
#include <osl/conditn.hxx>
#include <osl/interlck.h>
#include <vos/mutex.hxx>
#include <vos/thread.hxx>

// Custom event types
#define SALEVENT_OPENDOCUMENT		((USHORT)100)
#define SALEVENT_PRINTDOCUMENT		((USHORT)101)
#define SALEVENT_DEMINIMIZED		((USHORT)102)
#define SALEVENT_MINIMIZED			((USHORT)103)
#define SALEVENT_FULLSCREENENTERED	((USHORT)104)
#define SALEVENT_FULLSCREENEXITED	((USHORT)105)
#define SALEVENT_SCREENPARAMSCHANGED	((USHORT)106)
#define SALEVENT_SYSTEMCOLORSCHANGED	((USHORT)107)
#define SALEVENT_ABOUT				((USHORT)130)
#define SALEVENT_PREFS				((USHORT)140)

// -----------------
// - SalYieldMutex -
// -----------------

class SalYieldMutex : public ::vos::OMutex
{
	ULONG					mnCount;
	::osl::Condition		maMainThreadCondition;
	::osl::Condition		maReacquireThreadCondition;
	::vos::OThread::TThreadIdentifier	mnThreadId;
	::vos::OThread::TThreadIdentifier	mnReacquireThreadId;

public:
							SalYieldMutex();
	virtual void			acquire();
	virtual void			release();
	virtual sal_Bool		tryToAcquire();
	::vos::OThread::TThreadIdentifier	GetThreadId() { return mnThreadId; }
	::vos::OThread::TThreadIdentifier	GetReacquireThreadId() { return mnReacquireThreadId; }
	ULONG					ReleaseAcquireCount();
	void					WaitForReacquireThread();
};

// -------------------
// - JavaSalInstance -
// -------------------

class JavaSalInstance : public SalInstance
{
	SalYieldMutex*			mpSalYieldMutex;

public:
							JavaSalInstance();
	virtual					~JavaSalInstance();

	virtual SalFrame*		CreateChildFrame( SystemParentData* pParent, ULONG nStyle );
	virtual SalFrame*		CreateFrame( SalFrame* pParent, ULONG nStyle );
	virtual void			DestroyFrame( SalFrame* pFrame );
	virtual SalObject*		CreateObject( SalFrame* pParent, SystemWindowData* pWindowData, BOOL bShow = TRUE );
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
	virtual SalTimer*		CreateSalTimer();
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

// ----------------
// - JavaSalEvent -
// ----------------

class JavaSalEvent
{
	USHORT					mnID;
	JavaSalFrame*			mpFrame;
	::rtl::OUString			maPath;
	bool					mbNative;
	sal_Bool				mbShutdownCancelled;
	::std::list< JavaSalEvent* >	maOriginalKeyEvents;
	ULONG					mnCommittedCharacters;
	ULONG					mnCursorPosition;
	void*					mpData;
	mutable oslInterlockedCount	mnRefCount;

public:
							JavaSalEvent( USHORT nID, JavaSalFrame *pFrame, void *pData, const ::rtl::OString& rPath = ::rtl::OString(), ULONG nCommittedCharacters = 0, ULONG nCursorPosition = 0 );

protected:
	virtual					~JavaSalEvent();

public:
	void					addRepeatCount( USHORT nCount );
	void					addOriginalKeyEvent( JavaSalEvent *pEvent );
	void					addUpdateRect( const Rectangle& rRect );
	bool					addWheelRotationAndScrollLines( long nRotation, ULONG nScrollLines, sal_Bool bHorizontal );
	void					cancelShutdown();
	void					dispatch();
	ULONG					getCommittedCharacterCount();
	ULONG					getCursorPosition();
	JavaSalFrame*			getFrame();
	USHORT					getKeyChar();
	USHORT					getKeyCode();
	USHORT					getID();
	USHORT					getModifiers();
	JavaSalEvent*			getNextOriginalKeyEvent();
	::rtl::OUString			getPath();
	USHORT					getRepeatCount();
	XubString				getText();
	const USHORT*			getTextAttributes();
	const Rectangle			getUpdateRect();
	ULONG					getWhen();
	long					getX();
	long					getY();
	short					getMenuID();
	void*					getMenuCookie();
	long					getScrollAmount();
	long					getWheelRotation();
	sal_Bool				isHorizontal();
	bool					isNative() { return mbNative; }
	sal_Bool				isShutdownCancelled();
	void					reference() const;
	void					release() const;
};

// -------------------------
// - JavaSalEventQueueItem -
// -------------------------

class SAL_DLLPRIVATE JavaSalEventQueueItem
{
	JavaSalEvent*			mpEvent;
	const ::std::list< JavaSalEventQueueItem* >*	mpEventQueue;
	bool					mbRemove;
	USHORT					mnType;

public:
							JavaSalEventQueueItem( JavaSalEvent *pEvent, const ::std::list< JavaSalEventQueueItem* > *pEventQueue );
	virtual					~JavaSalEventQueueItem();

	JavaSalEvent*			getEvent() { return mpEvent; }
	const ::std::list< JavaSalEventQueueItem* >*	getEventQueue() { return mpEventQueue; }
	USHORT					getType() { return mnType; }
	bool					isRemove() { return mbRemove; }
	void					remove() { mbRemove = true; }
};

// ---------------------
// - JavaSalEventQueue -
// ---------------------

class JavaSalEventQueue
{
	static ::osl::Mutex		maMutex;
	static ::osl::Condition	maCondition;
	static ::std::list< JavaSalEventQueueItem* >	maNativeEventQueue;
	static ::std::list< JavaSalEventQueueItem* >	maNonNativeEventQueue;
	static JavaSalEventQueueItem*	mpKeyInputItem;
	static double			mfLastNativeEventTime;
	static JavaSalEventQueueItem*	mpMoveResizeItem;
	static JavaSalEventQueueItem*	mpPaintItem;
	static sal_Bool			mbShutdownDisabled;

public:
	static void				purgeRemovedEventsFromFront( ::std::list< JavaSalEventQueueItem* > *pEventQueue );
	static sal_Bool			anyCachedEvent( USHORT nType );
	static void				dispatchNextEvent();
	static double			getLastNativeEventTime();
	static JavaSalEvent*	getNextCachedEvent( ULONG nTimeout, sal_Bool bNativeEvents );
	static sal_Bool			isShutdownDisabled();
	static void				postCachedEvent( JavaSalEvent *pEvent );
	static void				removeCachedEvents( const JavaSalFrame *pFrame );
	static void				setLastNativeEventTime( double nEventTime );
	static void				setShutdownDisabled( sal_Bool bShutdownDisabled );
};

#endif // _SV_SALINST_H
