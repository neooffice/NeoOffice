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
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): Edward Peterlin, 2004.  Native Menu Framework derived from
 *  NeoOffice, copright 2004 Dan Williams.
 *
 *
 ************************************************************************/

#ifndef _SV_SALINST_HXX
#define _SV_SALINST_HXX

#ifdef __cplusplus

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif

#endif // __cplusplus

#ifndef _SV_SV_H
#include <sv.h>
#endif
#ifndef _SV_SALINST_H
#include <salinst.h>
#endif

#ifdef __cplusplus

struct SystemParentData;
struct SalPrinterQueueInfo;
struct SalStatus;
struct ImplJobSetup;
class SalGraphics;
class SalFrame;
class SalObject;
class SalMenu;
class SalMenuItem;
class SalVirtualDevice;
class SalInfoPrinter;
class SalPrinter;
class ImplPrnQueueList;
class SalSession;
class Menu;
struct SalItemParams;

namespace vos { class IMutex; }

// ---------------
// - SalInstance -
// ---------------

class SalInstance
{
    friend SalInstance* CreateSalInstance();
    friend void DestroySalInstance( SalInstance* );

private:
                            SalInstance();
                            ~SalInstance();

public:                     // public for Sal Implementation
    SalInstanceData         maInstData;

#ifdef _INCL_SAL_SALINST_IMP
#include <salinst.imp>
#endif

public:
                            // Frame
                            // DisplayName for Unix ???
    SalFrame*               CreateChildFrame( SystemParentData* pParent, ULONG nStyle );
    SalFrame*               CreateFrame( SalFrame* pParent, ULONG nStyle );
    void                    DestroyFrame( SalFrame* pFrame );

                            // Object (System Child Window)
    SalObject*              CreateObject( SalFrame* pParent );
    void                    DestroyObject( SalObject* pObject );

                            // Menus
    SalMenu*                CreateMenu( BOOL bMenuBar, Menu* pVCLMenu );
    void                    DestroyMenu( SalMenu* pMenu );
    SalMenuItem*            CreateMenuItem( const SalItemParams* pItemData );
    void                    DestroyMenuItem( SalMenuItem* pItem );

                            // VirtualDevice
                            // nDX and nDY in Pixeln
                            // nBitCount: 0 == Default / 1 == Mono
    SalVirtualDevice*       CreateVirtualDevice( SalGraphics* pGraphics,
                                                 long nDX, long nDY,
                                                 USHORT nBitCount );
    void                    DestroyVirtualDevice( SalVirtualDevice* pDevice );

                            // Printer
                            // pSetupData->mpDriverData can be 0
                            // pSetupData must be updatet with the current
                            // JobSetup
    SalInfoPrinter*         CreateInfoPrinter( SalPrinterQueueInfo* pQueueInfo,
                                               ImplJobSetup* pSetupData );
    void                    DestroyInfoPrinter( SalInfoPrinter* pPrinter );
    SalPrinter*             CreatePrinter( SalInfoPrinter* pInfoPrinter );
    void                    DestroyPrinter( SalPrinter* pPrinter );

    void                    GetPrinterQueueInfo( ImplPrnQueueList* pList );
    void                    GetPrinterQueueState( SalPrinterQueueInfo* pInfo );
    void                    DeletePrinterQueueInfo( SalPrinterQueueInfo* pInfo );
    XubString               GetDefaultPrinter();

    // may return NULL to disable session management
    SalSession*				CreateSalSession();

                            // YieldMutex
    vos::IMutex*            GetYieldMutex();
    ULONG                   ReleaseYieldMutex();
    void                    AcquireYieldMutex( ULONG nCount );

                            // wait next event and dispatch
                            // must returned by UserEvent (SalFrame::PostEvent)
                            // and timer
    void                    Yield( BOOL bWait );
    static BOOL             AnyInput( USHORT nType );

    // methods for XDisplayConnection

    // the parameters for the callbacks are:
    //    void* pInst:          pInstance form the SetCallback call
    //    void* pEvent:         address of the system specific event structure
    //    int   nBytes:         length of the system specific event structure
    void                    SetEventCallback( void* pInstance, bool(*pCallback)(void*,void*,int));
    void                    SetErrorEventCallback( void* pInstance, bool(*pCallback)(void*,void*,int));

    enum ConnectionIdentifierType { AsciiCString, Blob };
    void*                   GetConnectionIdentifier( ConnectionIdentifierType& rReturnedType, int& rReturnedBytes );
};

// called from SVMain
SalInstance* CreateSalInstance();
void DestroySalInstance( SalInstance* pInst );

// -------------------------
// - SalInstance-Functions -
// -------------------------

void SalAbort( const XubString& rErrorText );

#endif // __cplusplus

// -----------
// - SalData -
// -----------

void InitSalData();                         // called from Application-Ctor
void DeInitSalData();                       // called from Application-Dtor
void SetFilterCallback( void* pCallback, void* pInst );

void InitSalMain();
void DeInitSalMain();

// ----------
// - SVMain -
// ----------

// Callbacks (indepen in \sv\source\app\svmain.cxx)
BOOL SVMain();

#endif // _SV_SALINST_HXX
