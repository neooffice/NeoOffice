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
#include <vos/mutex.hxx>
#include <vos/thread.hxx>

#ifndef USE_NATIVE_EVENTS
#include <jni.h>
#endif	// !USE_NATIVE_EVENTS

// Custom event types
#define SALEVENT_OPENDOCUMENT		((USHORT)100)
#define SALEVENT_PRINTDOCUMENT		((USHORT)101)
#define SALEVENT_DEMINIMIZED		((USHORT)102)
#define SALEVENT_MINIMIZED			((USHORT)103)
#define SALEVENT_ABOUT				((USHORT)130)
#define SALEVENT_PREFS				((USHORT)140)

#define WHEEL_ROTATION_FACTOR 120

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

class SAL_DLLPRIVATE JavaSalEvent
{
#ifdef USE_NATIVE_EVENTS
	USHORT					mnID;
	JavaSalFrame*			mpFrame;
	::rtl::OUString			maPath;
	bool					mbNative;
	sal_Bool				mbShutdownCancelled;
#else	// USE_NATIVE_EVENTS
	::vcl::com_sun_star_vcl_VCLEvent*	mpVCLEvent;
#endif	// USE_NATIVE_EVENTS
	void*					mpData;
	mutable int				mnRefCount;

public:
							JavaSalEvent( USHORT nID, JavaSalFrame *pFrame, void *pData, const ::rtl::OString& rPath = ::rtl::OString() );
#ifndef USE_NATIVE_EVENTS
							JavaSalEvent( ::vcl::com_sun_star_vcl_VCLEvent *pVCLEvent );
#endif	// !USE_NATIVE_EVENTS
protected:
	virtual					~JavaSalEvent();

public:
#ifdef USE_NATIVE_EVENTS
	void					addRepeatCount( USHORT nCount );
	void					addUpdateRect( const Rectangle& rRect );
	void					addWheelRotation( long nRotation );
#endif	// USE_NATIVE_EVENTS
	void					cancelShutdown();
	void					dispatch();
	ULONG					getCommittedCharacterCount();
	ULONG					getCursorPosition();
#ifndef USE_NATIVE_EVENTS
	void*					getData();
#endif	// !USE_NATIVE_EVENTS
	JavaSalFrame*			getFrame();
	USHORT					getKeyChar();
	USHORT					getKeyCode();
	USHORT					getID();
	USHORT					getModifiers();
	JavaSalEvent*			getNextOriginalKeyEvent();
	::rtl::OUString			getPath();
	USHORT					getRepeatCount();
	::rtl::OUString			getText();
	USHORT*					getTextAttributes();
	const Rectangle			getUpdateRect();
#ifndef USE_NATIVE_EVENTS
	::vcl::com_sun_star_vcl_VCLEvent*	getVCLEvent() const { return mpVCLEvent; }
#endif	// !USE_NATIVE_EVENTS
	ULONG					getWhen();
	long					getX();
	long					getY();
	short					getMenuID();
	void*					getMenuCookie();
	long					getScrollAmount();
	long					getWheelRotation();
	sal_Bool				isHorizontal();
#ifdef USE_NATIVE_EVENTS
	bool					isNative() { return mbNative; }
#endif	// USE_NATIVE_EVENTS
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

class SAL_DLLPRIVATE JavaSalEventQueue
{
	static ::osl::Mutex		maMutex;
#ifdef USE_NATIVE_EVENTS
	static ::osl::Condition	maCondition;
	static ::std::list< JavaSalEventQueueItem* >	maNativeEventQueue;
	static ::std::list< JavaSalEventQueueItem* >	maNonNativeEventQueue;
	static JavaSalEventQueueItem*	mpKeyInputItem;
	static JavaSalEventQueueItem*	mpMoveResizeItem;
	static JavaSalEventQueueItem*	mpPaintItem;
	static sal_Bool			mbShutdownDisabled;
#else	// USE_NATIVE_EVENTS
	static ::vcl::com_sun_star_vcl_VCLEventQueue*	mpVCLEventQueue;
#endif	// USE_NATIVE_EVENTS

public:
#ifdef USE_NATIVE_EVENTS
	static void				purgeRemovedEventsFromFront( ::std::list< JavaSalEventQueueItem* > *pEventQueue );
#else	// USE_NATIVE_EVENTS
	static ::vcl::com_sun_star_vcl_VCLEventQueue*	getVCLEventQueue();
	static sal_Bool			postCommandEvent( jobject aObj, short nKeyCode, sal_Bool bShiftDown, sal_Bool bControlDown, sal_Bool bAltDown, sal_Bool bMetaDown, jchar nOriginalKeyChar, sal_Bool bOriginalShiftDown, sal_Bool bOriginalControlDown, sal_Bool bOriginalAltDown, sal_Bool bOriginalMetaDown );
	static void				postMouseWheelEvent( jobject aObj, long nX, long nY, long nRotationX, long nRotationY, sal_Bool bShiftDown, sal_Bool bMetaDown, sal_Bool bAltDown, sal_Bool bControlDown );
#ifdef USE_NATIVE_WINDOW
	static void				postMenuItemSelectedEvent( JavaSalFrame *pFrame, USHORT nID, Menu *pMenu );
#endif	// USE_NATIVE_WINDOW
	static void				postWindowMoveSessionEvent( jobject aObj, long nX, long nY, sal_Bool bStartSession );
#endif	// USE_NATIVE_EVENTS
	static sal_Bool			anyCachedEvent( USHORT nType );
	static void				dispatchNextEvent();
	static JavaSalEvent*	getNextCachedEvent( ULONG nTimeout, sal_Bool bNativeEvents );
	static sal_Bool			isInitialized();
	static sal_Bool			isShutdownDisabled();
	static void				postCachedEvent( JavaSalEvent *pEvent );
	static void				removeCachedEvents( const JavaSalFrame *pFrame );
	static void				setShutdownDisabled( sal_Bool bShutdownDisabled );
};

SAL_DLLPRIVATE bool IsRunningLeopard();
SAL_DLLPRIVATE bool IsRunningSnowLeopard();
SAL_DLLPRIVATE bool IsRunningLion();
SAL_DLLPRIVATE bool IsRunningMountainLion();
SAL_DLLPRIVATE bool IsFullKeyboardAccessEnabled();
#ifndef USE_NATIVE_EVENTS
SAL_DLLPRIVATE void InitJavaAWT();
#endif	// !USE_NATIVE_EVENTS

#endif // _SV_SALINST_H
