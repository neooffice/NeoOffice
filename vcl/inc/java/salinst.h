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

// -----------------
// - SalYieldMutex -
// -----------------

class SalYieldMutex : public comphelper::SolarMutex
{
	::osl::Mutex			maMutex;
	sal_uLong				mnCount;
	::osl::Condition		maMainThreadCondition;
	::osl::Condition		maReacquireThreadCondition;
	oslThreadIdentifier		mnThreadId;
	oslThreadIdentifier		mnReacquireThreadId;

public:
							SalYieldMutex();
	virtual void			acquire() override;
	virtual void			release() override;
	virtual bool			tryToAcquire() override;
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

	virtual SalFrame*		CreateChildFrame( SystemParentData* pParent, SalFrameStyleFlags nStyle ) override;
	virtual SalFrame*		CreateFrame( SalFrame* pParent, SalFrameStyleFlags nStyle ) override;
	virtual void			DestroyFrame( SalFrame* pFrame ) override;
	virtual SalObject*		CreateObject( SalFrame* pParent, SystemWindowData* pWindowData, bool bShow = true ) override;
	virtual void			DestroyObject( SalObject* pObject ) override;
	virtual SalVirtualDevice*	CreateVirtualDevice( SalGraphics* pGraphics, long& rDX, long& rDY, DeviceFormat eFormat, const SystemGraphicsData *pData = NULL ) override;
	virtual SalInfoPrinter* CreateInfoPrinter( SalPrinterQueueInfo* pQueueInfo, ImplJobSetup* pSetupData ) override;
	virtual void			DestroyInfoPrinter( SalInfoPrinter* pPrinter ) override;
	virtual SalPrinter*		CreatePrinter( SalInfoPrinter* pInfoPrinter ) override;
	virtual void			DestroyPrinter( SalPrinter* pPrinter ) override;

	virtual void			GetPrinterQueueInfo( ImplPrnQueueList* pList ) override;
	virtual void			GetPrinterQueueState( SalPrinterQueueInfo* pInfo ) override;
	virtual void			DeletePrinterQueueInfo( SalPrinterQueueInfo* pInfo ) override;
	virtual OUString		GetDefaultPrinter() override;
	virtual SalTimer*		CreateSalTimer() override;
	virtual SalI18NImeStatus*	CreateI18NImeStatus() override;
	virtual SalSystem*		CreateSalSystem() override;
	virtual SalBitmap*		CreateSalBitmap() override;
	virtual comphelper::SolarMutex*	GetYieldMutex() override;
	virtual sal_uLong		ReleaseYieldMutex() override;
	virtual void			AcquireYieldMutex( sal_uLong nCount ) override;
	virtual bool			CheckYieldMutex() override;
	virtual SalYieldResult	DoYield( bool bWait, bool bHandleAllCurrentEvents, sal_uLong nReleased ) override;
	virtual bool			AnyInput( VclInputFlags nType ) override;
	virtual SalMenu*		CreateMenu( bool bMenuBar, Menu* pVCLMenu ) override;
	virtual void			DestroyMenu( SalMenu* pMenu ) override;
	virtual SalMenuItem*	CreateMenuItem( const SalItemParams* pItemData ) override;
	virtual void			DestroyMenuItem( SalMenuItem* pItem ) override;
	virtual SalSession*		CreateSalSession() override;
	virtual OUString		GetConnectionIdentifier() override;
    virtual css::uno::Reference< css::uno::XInterface >	CreateClipboard( const css::uno::Sequence< css::uno::Any >& rArguments ) override;
    virtual css::uno::Reference< css::uno::XInterface >	CreateDragSource() override;
    virtual css::uno::Reference< css::uno::XInterface >	CreateDropTarget() override;
	virtual void			AddToRecentDocumentList( const OUString& rFileUrl, const OUString& rMimeType, const OUString& rDocumentService ) override;
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
	void					addUpdateRect( const tools::Rectangle& rRect );
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
	const tools::Rectangle	getUpdateRect();
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

#endif // _SV_SALINST_H
