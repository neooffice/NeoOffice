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

#include <osl/conditn.hxx>
#include <osl/interlck.h>
#include <osl/mutex.hxx>
#include <osl/thread.hxx>
#include <vcl/svapp.hxx>

#include "salinst.hxx"
#include "svdata.hxx"
#include "java/salframe.h"

// Custom event types
#define SALEVENT_OPENDOCUMENT		((sal_uInt16)100)
#define SALEVENT_PRINTDOCUMENT		((sal_uInt16)101)
#define SALEVENT_DEMINIMIZED		((sal_uInt16)102)
#define SALEVENT_MINIMIZED			((sal_uInt16)103)
#define SALEVENT_FULLSCREENENTERED	((sal_uInt16)104)
#define SALEVENT_FULLSCREENEXITED	((sal_uInt16)105)
#define SALEVENT_SCREENPARAMSCHANGED	((sal_uInt16)106)
#define SALEVENT_SYSTEMCOLORSCHANGED	((sal_uInt16)107)
#define SALEVENT_COMMANDMEDIADATA	((sal_uInt16)120)
#define SALEVENT_ABOUT				((sal_uInt16)130)
#define SALEVENT_PREFS				((sal_uInt16)140)
#define SALEVENT_WAKEUP				((sal_uInt16)150)
#define SALEVENT_REGISTERA11YFRAME	((sal_uInt16)161)
#define SALEVENT_REVOKEA11YFRAME	((sal_uInt16)162)

// -----------------
// - SalYieldMutex -
// -----------------

class SalYieldMutex : public comphelper::SolarMutex
{
	::osl::Mutex			maMutex;
	sal_uLong				mnCount;
	::osl::Condition		maMainThreadCondition;
	::osl::Condition		maMainThreadWaitingCondition;
	::osl::Condition		maReacquireThreadCondition;
	oslThreadIdentifier		mnThreadId;
	oslThreadIdentifier		mnReacquireThreadId;

public:
							SalYieldMutex();
	virtual void			acquire() SAL_OVERRIDE;
	virtual void			release() SAL_OVERRIDE;
	virtual bool			tryToAcquire() SAL_OVERRIDE;
	oslThreadIdentifier		GetThreadId() { return mnThreadId; }
	oslThreadIdentifier		GetReacquireThreadId() { return mnReacquireThreadId; }
	sal_uLong				ReleaseAcquireCount();
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

	virtual SalFrame*		CreateChildFrame( SystemParentData* pParent, sal_uLong nStyle ) SAL_OVERRIDE;
	virtual SalFrame*		CreateFrame( SalFrame* pParent, sal_uLong nStyle ) SAL_OVERRIDE;
	virtual void			DestroyFrame( SalFrame* pFrame ) SAL_OVERRIDE;
	virtual SalObject*		CreateObject( SalFrame* pParent, SystemWindowData* pWindowData, bool bShow = true ) SAL_OVERRIDE;
	virtual void			DestroyObject( SalObject* pObject ) SAL_OVERRIDE;
	virtual SalVirtualDevice*	CreateVirtualDevice( SalGraphics* pGraphics, long& rDX, long& rDY, sal_uInt16 nBitCount, const SystemGraphicsData *pData = NULL ) SAL_OVERRIDE;
	virtual SalInfoPrinter* CreateInfoPrinter( SalPrinterQueueInfo* pQueueInfo, ImplJobSetup* pSetupData ) SAL_OVERRIDE;
	virtual void			DestroyInfoPrinter( SalInfoPrinter* pPrinter ) SAL_OVERRIDE;
	virtual SalPrinter*		CreatePrinter( SalInfoPrinter* pInfoPrinter ) SAL_OVERRIDE;
	virtual void			DestroyPrinter( SalPrinter* pPrinter ) SAL_OVERRIDE;

	virtual void			GetPrinterQueueInfo( ImplPrnQueueList* pList ) SAL_OVERRIDE;
	virtual void			GetPrinterQueueState( SalPrinterQueueInfo* pInfo ) SAL_OVERRIDE;
	virtual void			DeletePrinterQueueInfo( SalPrinterQueueInfo* pInfo ) SAL_OVERRIDE;
	virtual OUString		GetDefaultPrinter() SAL_OVERRIDE;
	virtual SalTimer*		CreateSalTimer() SAL_OVERRIDE;
	virtual SalI18NImeStatus*	CreateI18NImeStatus() SAL_OVERRIDE;
	virtual SalSystem*		CreateSalSystem() SAL_OVERRIDE;
	virtual SalBitmap*		CreateSalBitmap() SAL_OVERRIDE;
	virtual comphelper::SolarMutex*	GetYieldMutex() SAL_OVERRIDE;
	virtual sal_uLong		ReleaseYieldMutex() SAL_OVERRIDE;
	virtual void			AcquireYieldMutex( sal_uLong nCount ) SAL_OVERRIDE;
	virtual bool			CheckYieldMutex() SAL_OVERRIDE;
	virtual void			Yield( bool bWait, bool bHandleAllCurrentEvents ) SAL_OVERRIDE;
	virtual bool			AnyInput( sal_uInt16 nType ) SAL_OVERRIDE;
	virtual SalMenu*		CreateMenu( bool bMenuBar, Menu* pVCLMenu ) SAL_OVERRIDE;
	virtual void			DestroyMenu( SalMenu* pMenu ) SAL_OVERRIDE;
	virtual SalMenuItem*	CreateMenuItem( const SalItemParams* pItemData ) SAL_OVERRIDE;
	virtual void			DestroyMenuItem( SalMenuItem* pItem ) SAL_OVERRIDE;
	virtual SalSession*		CreateSalSession() SAL_OVERRIDE;
	virtual void*			GetConnectionIdentifier( ConnectionIdentifierType& rReturnedType, int& rReturnedBytes ) SAL_OVERRIDE;
    virtual css::uno::Reference< css::uno::XInterface >	CreateClipboard( const css::uno::Sequence< css::uno::Any >& rArguments ) SAL_OVERRIDE;
    virtual css::uno::Reference< css::uno::XInterface >	CreateDragSource() SAL_OVERRIDE;
    virtual css::uno::Reference< css::uno::XInterface >	CreateDropTarget() SAL_OVERRIDE;
	virtual void			AddToRecentDocumentList( const OUString& rFileUrl, const OUString& rMimeType, const OUString& rDocumentService ) SAL_OVERRIDE;
};

// ----------------
// - JavaSalEvent -
// ----------------

class JavaSalEvent
{
	sal_uInt16				mnID;
	JavaSalFrame*			mpFrame;
	OUString				maPath;
	bool					mbNative;
	sal_Bool				mbShutdownCancelled;
	::std::list< JavaSalEvent* >	maOriginalKeyEvents;
	sal_uLong				mnCommittedCharacters;
	sal_uLong				mnCursorPosition;
	void*					mpData;
	mutable oslInterlockedCount	mnRefCount;

public:
							JavaSalEvent( sal_uInt16 nID, JavaSalFrame *pFrame, void *pData, const OString& rPath = OString(), sal_uLong nCommittedCharacters = 0, sal_uLong nCursorPosition = 0 );

protected:
	virtual					~JavaSalEvent();

public:
	void					addRepeatCount( sal_uInt16 nCount );
	void					addOriginalKeyEvent( JavaSalEvent *pEvent );
	void					addUpdateRect( const Rectangle& rRect );
	bool					addWheelRotationAndScrollLines( long nRotation, sal_uLong nScrollLines, sal_Bool bHorizontal );
	void					cancelShutdown();
	void					dispatch();
	sal_uLong				getCommittedCharacterCount();
	sal_uLong				getCursorPosition();
	JavaSalFrame*			getFrame();
	sal_uInt16				getKeyChar();
	sal_uInt16				getKeyCode();
	sal_uInt16				getID();
	sal_uInt16				getModifiers();
	JavaSalEvent*			getNextOriginalKeyEvent();
	OUString				getPath();
	sal_uInt16				getRepeatCount();
	OUString				getText();
	const sal_uInt16*		getTextAttributes();
	const Rectangle			getUpdateRect();
	sal_uLong				getWhen();
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
	sal_uInt16				mnType;

public:
							JavaSalEventQueueItem( JavaSalEvent *pEvent, const ::std::list< JavaSalEventQueueItem* > *pEventQueue );
	virtual					~JavaSalEventQueueItem();

	JavaSalEvent*			getEvent() { return mpEvent; }
	const ::std::list< JavaSalEventQueueItem* >*	getEventQueue() { return mpEventQueue; }
	sal_uInt16				getType() { return mnType; }
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
	static sal_Bool			anyCachedEvent( sal_uInt16 nType );
	static void				dispatchNextEvent();
	static double			getLastNativeEventTime();
	static JavaSalEvent*	getNextCachedEvent( sal_uLong nTimeout, sal_Bool bNativeEvents );
	static sal_Bool			isShutdownDisabled();
	static void				postCachedEvent( JavaSalEvent *pEvent );
	static void				removeCachedEvents( const JavaSalFrame *pFrame );
	static void				setLastNativeEventTime( double nEventTime );
	static void				setShutdownDisabled( sal_Bool bShutdownDisabled );
};

extern "C" inline sal_Bool ImplSalInstanceExists() { return ( ImplGetSVData() && ImplGetSVData()->mpDefInst ); }

// Check if ImplSVData exists first since Application::IsShutDown() uses it
extern "C" inline sal_Bool ImplApplicationIsRunning() { return ( ImplGetSVData() && ImplGetSVData()->mpDefInst && !Application::IsShutDown() ); }

#endif // _SV_SALINST_H
