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
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): Edward Peterlin, 2004.  Native Menu Framework derived from
 *  NeoOffice, copyright Dan Williams 2004.
 *
 *
 ************************************************************************/

#ifndef _SV_SALFRAME_HXX
#define _SV_SALFRAME_HXX

#ifndef _SV_SV_H
#include <sv.h>
#endif

#ifdef __cplusplus

#ifndef _SV_PTRSTYLE_HXX
#include <ptrstyle.hxx>
#endif
#ifndef _SV_SNDSTYLE_HXX
#include <sndstyle.hxx>
#endif

#endif // __cplusplus

#ifndef _SV_SALWTYPE_HXX
#include <salwtype.hxx>
#endif
#ifndef _SV_SALFRAME_H
#include <salframe.h>
#endif
#ifndef _SV_SALGEOM_HXX
#include <salgeom.hxx>
#endif

#ifndef _SV_GEN_HXX
#include <tools/gen.hxx>
#endif

#ifdef __cplusplus

class AllSettings;
class SalGraphics;
class SalBitmap;
class SalMenu;

#else

#define AllSettings void
#define SalGraphics void

#endif // __cplusplus


struct SalFrameState;
struct SalInputContext;
struct SystemEnvData;

// -----------------
// - SalFrameTypes -
// -----------------

#define SAL_FRAME_TOTOP_RESTOREWHENMIN      ((USHORT)0x0001)
#define SAL_FRAME_TOTOP_FOREGROUNDTASK      ((USHORT)0x0002)
#define SAL_FRAME_TOTOP_GRABFOCUS           ((USHORT)0x0004)
#define SAL_FRAME_TOTOP_GRABFOCUS_ONLY		 ((USHORT)0x0008)

#define SAL_FRAME_ENDEXTTEXTINPUT_COMPLETE  ((USHORT)0x0001)
#define SAL_FRAME_ENDEXTTEXTINPUT_CANCEL    ((USHORT)0x0002)


// -----------------
// - SalFrameStyle -
// -----------------

#define SAL_FRAME_STYLE_DEFAULT             ((ULONG)0x00000001)
#define SAL_FRAME_STYLE_MOVEABLE            ((ULONG)0x00000002)
#define SAL_FRAME_STYLE_SIZEABLE            ((ULONG)0x00000004)
#define SAL_FRAME_STYLE_CLOSEABLE           ((ULONG)0x00000008)

#define SAL_FRAME_STYLE_NOSHADOW            ((ULONG)0x00000010)
#define SAL_FRAME_STYLE_TOOLTIP			 ((ULONG)0x00000020)
#define SAL_FRAME_STYLE_CHILD               ((ULONG)0x10000000)
#define SAL_FRAME_STYLE_FLOAT               ((ULONG)0x20000000)
#define SAL_FRAME_STYLE_TOOLWINDOW          ((ULONG)0x40000000)
#define SAL_FRAME_STYLE_INTRO               ((ULONG)0x80000000)

/*
#define SAL_FRAME_STYLE_MINABLE             ((ULONG)0x00000008)
#define SAL_FRAME_STYLE_MAXABLE             ((ULONG)0x00000010)
#define SAL_FRAME_STYLE_BORDER              ((ULONG)0x00000040)
#define SAL_FRAME_STYLE_DOC                 ((ULONG)0x00004000)
#define SAL_FRAME_STYLE_DIALOG              ((ULONG)0x00008000)
#define SAL_FRAME_STYLE_TOOL                ((ULONG)0x00010000)
#define SAL_FRAME_STYLE_FULLSIZE            ((ULONG)0x00020000)
*/


// ------------------------
// - Flags for SetPosSize -
// ------------------------

#define SAL_FRAME_POSSIZE_X                 ((USHORT)0x0001)
#define SAL_FRAME_POSSIZE_Y                 ((USHORT)0x0002)
#define SAL_FRAME_POSSIZE_WIDTH             ((USHORT)0x0004)
#define SAL_FRAME_POSSIZE_HEIGHT            ((USHORT)0x0008)

#ifdef __cplusplus

// ------------
// - SalFrame -
// ------------

class SalFrame
{
    friend class SalInstance;

public:                     // public for Sal Implementation
                            SalFrame();
                            ~SalFrame();

public:                     // public for Sal Implementation
    SalFrameData            maFrameData;
    SalFrameGeometry		maGeometry;
    static BOOL             mbInReparent;

#ifdef _INCL_SAL_SALFRAME_IMP
#include <salframe.imp>
#endif

public:
    // SalGraphics or NULL, but two Graphics for all SalFrames
    // must be returned
    SalGraphics*            GetGraphics();
    void                    ReleaseGraphics( SalGraphics* pGraphics );

    // Event must be destroyed, when Frame is destroyed
    // When Event is called, SalInstance::Yield() must be returned
    BOOL                    PostEvent( void* pData );

    void                    SetTitle( const XubString& rTitle );
    void                    SetIcon( USHORT nIcon );
    void                    SetMenu( SalMenu *pSalMenu );
    void                    DrawMenuBar();

                            // Befor the window is visible, a resize event
                            // must be sent with the correct size
    void                    Show( BOOL bVisible, BOOL bNoActivate = FALSE );
    void                    Enable( BOOL bEnable );
                            // Set ClientSize and Center the Window to the desktop
                            // and send/post a resize message
    void                    SetMinClientSize( long nWidth, long nHeight );
    void                    SetPosSize( long nX, long nY, long nWidth, long nHeight, USHORT nFlags );
    void                    GetClientSize( long& rWidth, long& rHeight );
    void                    GetWorkArea( Rectangle& rRect );
    SalFrame*               GetParent() const;
    // Note: x will be mirrored at parent if UI mirroring is active
    SalFrameGeometry        GetGeometry();
    const SalFrameGeometry& GetUnmirroredGeometry() const { return maGeometry; }
    void                    SetWindowState( const SalFrameState* pState );
    BOOL                    GetWindowState( SalFrameState* pState );
    void                    ShowFullScreen( BOOL bFullScreen );
                            // Enable/Disable ScreenSaver, SystemAgents, ...
    void                    StartPresentation( BOOL bStart );
                            // Show Window over all other Windows
    void                    SetAlwaysOnTop( BOOL bOnTop );

                            // Window to top and grab focus
    void                    ToTop( USHORT nFlags );

                            // this function can call with the same
                            // pointer style
    void                    SetPointer( PointerStyle ePointerStyle );
    void                    CaptureMouse( BOOL bMouse );
    void                    SetPointerPos( long nX, long nY );

                            // Alle noch anstehenden Ausgaben sofort
                            // durchfuehren
    void                    Flush();
                            // Dummy-Syncronen Aufruf zum Client/Display
                            // machen, damit man sicher sein kann, das
                            // Ausgaben nicht den Client/das Display
                            // ueberrennen
    void                    Sync();

    void                    SetInputContext( SalInputContext* pContext );
    void                    EndExtTextInput( USHORT nFlags );

    XubString               GetKeyName( USHORT nKeyCode );
    XubString               GetSymbolKeyName( const XubString& rFontName, USHORT nKeyCode );

                            // returns the input language used for the last key stroke
                            // may be LANGUAGE_DONTKNOW if not supported by the OS
    LanguageType            GetInputLanguage();

    SalBitmap*              SnapShot();

    void                    UpdateSettings( AllSettings& rSettings );

    void                    Beep( SoundType eSoundType );

    // Liefert die SystemDaten zurueck
    const SystemEnvData*    GetSystemData() const;

    // Callbacks (indepen in \sv\source\app\svframe.cxx)
    // for default message handling return 0
    void                    SetCallback( void* pInst, SALFRAMEPROC pProc );

    // get current modifier and button mask
    ULONG					GetCurrentModButtons();

    // set new parent window
    void					SetParent( SalFrame* pNewParent );
    // reparent window to act as a plugin; implementation
    // may choose to use a new system window inetrnally
    // return false to indicate failure
    bool					SetPluginParent( SystemParentData* pNewParent );
};



#endif // __cplusplus

#endif // _SV_SALFRAME_HXX
