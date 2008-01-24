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

#define _SV_SALFRAME_CXX

#ifndef _SV_SALFRAME_H
#include <salframe.h>
#endif
#ifndef _SV_SALGDI_H
#include <salgdi.h>
#endif
#ifndef _SV_SALOBJ_H
#include <salobj.h>
#endif
#ifndef _SV_SALDATA_HXX
#include <saldata.hxx>
#endif
#ifndef _SV_SALMENU_H
#include <salmenu.h>
#endif
#ifndef _SV_SALSYS_H
#include <salsys.h>
#endif
#ifndef _SV_SETTINGS_HXX
#include <settings.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <svapp.hxx>
#endif
#ifndef _SV_COM_SUN_STAR_VCL_VCLEVENT_HXX
#include <com/sun/star/vcl/VCLEvent.hxx>
#endif
#ifndef _SV_COM_SUN_STAR_VCL_VCLFRAME_HXX
#include <com/sun/star/vcl/VCLFrame.hxx>
#endif
#ifndef _SV_COM_SUN_STAR_VCL_VCLGRAPHICS_HXX
#include <com/sun/star/vcl/VCLGraphics.hxx>
#endif
#ifndef _SV_COM_SUN_STAR_VCL_VCLSCREEN_HXX
#include <com/sun/star/vcl/VCLScreen.hxx>
#endif
#ifndef _SV_DIALOG_HXX
#include <dialog.hxx>
#endif

#include <premac.h>
#include <Carbon/Carbon.h>
#include <postmac.h>
#undef check

static EventLoopTimerUPP pSetSystemUIModeTimerUPP = NULL;

using namespace rtl;
using namespace vcl;

// =======================================================================

static inline Color RGBColorToColor( RGBColor *theColor )
{
	return Color( (unsigned char)( ( (double)theColor->red / (double)USHRT_MAX ) * (double)0xFF ),
		(unsigned char)( ( (double)theColor->green / (double)USHRT_MAX ) * (double)0xFF ),
		(unsigned char)( ( (double)theColor->blue / (double)USHRT_MAX ) * (double)0xFF ) );
}

// =======================================================================

long ImplSalCallbackDummy( void*, SalFrame*, USHORT, const void* )
{
	return 0;
}

// =======================================================================

JavaSalFrame::JavaSalFrame()
{
	memset( &maGeometry, 0, sizeof( maGeometry ) );
	mpVCLFrame = NULL;
	mpGraphics = new JavaSalGraphics();
	mpGraphics->mpFrame = this;
	mnStyle = 0;
	mpParent = NULL;
	mbGraphics = FALSE;
	mbVisible = FALSE;
	memset( &maSysData, 0, sizeof( SystemEnvData ) );
	maSysData.nSize = sizeof( SystemEnvData );
	mbCenter = TRUE;
	memset( &maOriginalGeometry, 0, sizeof( maOriginalGeometry ) );
	mbFullScreen = FALSE;
	mbPresentation = FALSE;
	mpMenuBar = NULL;
	mbInSetPosSize = FALSE;
	mbInShow = FALSE;
}

// -----------------------------------------------------------------------

JavaSalFrame::~JavaSalFrame()
{
	Show( FALSE );
	StartPresentation( FALSE );
	CaptureMouse( FALSE );

	// Detach child objects
	::std::list< JavaSalObject* > aObjects( maObjects );
	for ( ::std::list< JavaSalObject* >::const_iterator it = aObjects.begin(); it != aObjects.end(); ++it )
		(*it)->Destroy();

	// Detach child windows
	::std::list< JavaSalFrame* > aChildren( maChildren );
	for ( ::std::list< JavaSalFrame* >::const_iterator it = aChildren.begin(); it != aChildren.end(); ++it )
		(*it)->SetParent( NULL );

	// Detach from parent
	SetParent( NULL );

	if ( mpVCLFrame )
	{
		mpVCLFrame->dispose();
		delete mpVCLFrame;
	}

	delete mpGraphics;
}

// -----------------------------------------------------------------------

void JavaSalFrame::AddObject( JavaSalObject *pObject )
{
	if ( pObject )
		maObjects.push_back( pObject );
}

// -----------------------------------------------------------------------

bool JavaSalFrame::IsFloatingFrame()
{
	return ( ! ( mnStyle & ( SAL_FRAME_STYLE_DEFAULT | SAL_FRAME_STYLE_MOVEABLE | SAL_FRAME_STYLE_SIZEABLE ) ) && this != GetSalData()->mpPresentationFrame );
}

// -----------------------------------------------------------------------

void JavaSalFrame::RemoveObject( JavaSalObject *pObject )
{
	if ( pObject )
		maObjects.remove( pObject );
}

// -----------------------------------------------------------------------

void JavaSalFrame::FlushAllObjects()
{
	if ( mbVisible )
	{
		::std::list< JavaSalObject* > aObjects( maObjects );
		for ( ::std::list< JavaSalObject* >::const_iterator it = aObjects.begin(); it != aObjects.end(); ++it )
			(*it)->Flush();
	}
}

// -----------------------------------------------------------------------

SalGraphics* JavaSalFrame::GetGraphics()
{
	if ( mbGraphics )
		return NULL;

	if ( !mpGraphics->mpVCLGraphics )
		mpGraphics->mpVCLGraphics = mpVCLFrame->getGraphics();
	mbGraphics = TRUE;

	return mpGraphics;
}

// -----------------------------------------------------------------------

void JavaSalFrame::ReleaseGraphics( SalGraphics* pGraphics )
{
	if ( pGraphics != mpGraphics )
		return;

	mbGraphics = FALSE;
}

// -----------------------------------------------------------------------

BOOL JavaSalFrame::PostEvent( void *pData )
{
	com_sun_star_vcl_VCLEvent aEvent( SALEVENT_USEREVENT, this, pData );
	GetSalData()->mpEventQueue->postCachedEvent( &aEvent );
	return TRUE;
}

// -----------------------------------------------------------------------

void JavaSalFrame::SetTitle( const XubString& rTitle )
{
	maTitle = rTitle;
	mpVCLFrame->setTitle( rTitle );
}

// -----------------------------------------------------------------------

void JavaSalFrame::SetIcon( USHORT nIcon )
{
#ifdef DEBUG
	fprintf( stderr, "JavaSalFrame::SetIcon not implemented\n" );
#endif
}

// -----------------------------------------------------------------------

void JavaSalFrame::Show( BOOL bVisible, BOOL bNoActivate )
{
	if ( bVisible == mbVisible )
		return;

	// Fix bug 2501 by closing any dialogs that are child windows of this
	// window. This is necessary because in a few cases, the OOo code closes
	// a dialog's parent and expects the dialog to still remain showing!
	// Java, on the other hand, will forcefully close the window leaving the
	// the OOo code in an irrecoverable state.
	if ( !bVisible )
	{
		// Close any attached objects
		::std::list< JavaSalObject* > aObjects( maObjects );
		for ( ::std::list< JavaSalObject* >::const_iterator it = aObjects.begin(); it != aObjects.end(); ++it )
			(*it)->Show( FALSE );

		Window *pWindow = Application::GetFirstTopLevelWindow();
		while ( pWindow && pWindow->ImplGetFrame() != this )
			pWindow = Application::GetNextTopLevelWindow( pWindow );

		if ( pWindow )
			Dialog::EndAllDialogs( pWindow );
	}

	SalData *pSalData = GetSalData();

	mbVisible = bVisible;

	// Make sure there is a graphics available to avoid crashing when the OOo
	// code tries to draw while updating the menus
	if ( !mpGraphics->mpVCLGraphics )
		mpGraphics->mpVCLGraphics = mpVCLFrame->getGraphics();

	mpVCLFrame->setVisible( mbVisible, bNoActivate );

	// Reset graphics
	mpGraphics->mpVCLGraphics->resetGraphics();

	if ( mbVisible )
	{
		mbInShow = TRUE;

		// Get native window since it won't be created until first shown
		maSysData.aWindow = (long)mpVCLFrame->getNativeWindowRef();
		mbCenter = FALSE;

		com_sun_star_vcl_VCLEvent aEvent( SALEVENT_MOVERESIZE, this, NULL );
		aEvent.dispatch();

		UpdateMenusForFrame( this, NULL );

		// Reattach floating children
		::std::list< JavaSalFrame* > aChildren( maChildren );
		for ( ::std::list< JavaSalFrame* >::const_iterator it = aChildren.begin(); it != aChildren.end(); ++it )
		{
			if ( (*it)->mbVisible )
				(*it)->SetParent( this );
		}

		// Reattach visible objects
		::std::list< JavaSalObject* > aReshowObjects( maObjects );
		for ( ::std::list< JavaSalObject* >::const_iterator it = aReshowObjects.begin(); it != aReshowObjects.end(); ++it )
			(*it)->Show( TRUE );

		// Explicitly set focus to this frame since Java may set the focus
		// to the child frame
		if ( !bNoActivate )
			ToTop( SAL_FRAME_TOTOP_RESTOREWHENMIN | SAL_FRAME_TOTOP_GRABFOCUS );

		mbInShow = FALSE;
	}
	else
	{
		// End composition
		com_sun_star_vcl_VCLEvent aEvent( SALEVENT_ENDEXTTEXTINPUT, this, NULL );
		aEvent.dispatch();

		// Remove the native window since it is destroyed when hidden
		maSysData.aWindow = 0;

		// Fix bug 1106 by ensuring that some frame has focus if we close
		// the focus frame
		if ( pSalData->mpFocusFrame == this )
		{
			pSalData->mpFocusFrame = NULL;

			// Set focus to parent frame
			if ( mpParent && mpParent->mbVisible )
				mpParent->ToTop( SAL_FRAME_TOTOP_RESTOREWHENMIN | SAL_FRAME_TOTOP_GRABFOCUS );
		}

		if ( pSalData->mpLastDragFrame == this )
			pSalData->mpLastDragFrame = NULL;
	}
}

// -----------------------------------------------------------------------

void JavaSalFrame::Enable( BOOL bEnable )
{
#ifdef DEBUG
	fprintf( stderr, "JavaSalFrame::Enable not implemented\n" );
#endif
}

// -----------------------------------------------------------------------

void JavaSalFrame::SetMinClientSize( long nWidth, long nHeight )
{
	mpVCLFrame->setMinClientSize( nWidth, nHeight );
}

// -----------------------------------------------------------------------

void JavaSalFrame::SetPosSize( long nX, long nY, long nWidth, long nHeight,
							USHORT nFlags )
{
	if ( mnStyle & SAL_FRAME_STYLE_CHILD )
		return;

	mbInSetPosSize = TRUE;

	Rectangle aPosSize( Point( maGeometry.nX - maGeometry.nLeftDecoration, maGeometry.nY - maGeometry.nTopDecoration ), Size( maGeometry.nWidth, maGeometry.nHeight ) );

	if ( ! ( nFlags & SAL_FRAME_POSSIZE_X ) )
		nX = aPosSize.nLeft;
	if ( ! ( nFlags & SAL_FRAME_POSSIZE_Y ) )
		nY = aPosSize.nTop;
	if ( ! ( nFlags & SAL_FRAME_POSSIZE_WIDTH ) )
		nWidth = aPosSize.GetWidth();
	if ( ! ( nFlags & SAL_FRAME_POSSIZE_HEIGHT ) )
		nHeight = aPosSize.GetHeight();

	// Adjust position for RTL layout
	long nParentX = 0;
	long nParentY = 0;
	if ( mpParent )
	{
		Rectangle aParentBounds( mpParent->mpVCLFrame->getBounds() );
		nParentX = aParentBounds.nLeft + mpParent->maGeometry.nLeftDecoration;
		nParentY = aParentBounds.nTop + mpParent->maGeometry.nTopDecoration;

		if ( nFlags & ( SAL_FRAME_POSSIZE_X | SAL_FRAME_POSSIZE_Y ) && Application::GetSettings().GetLayoutRTL() )
			nX = mpParent->maGeometry.nWidth - nWidth - nX - 1;

		if ( nFlags & SAL_FRAME_POSSIZE_X )
			nX += nParentX;
		if ( nFlags & SAL_FRAME_POSSIZE_Y )
			nY += nParentY;
	}

	Rectangle aWorkArea;
	if ( mbCenter && ! ( nFlags & ( SAL_FRAME_POSSIZE_X | SAL_FRAME_POSSIZE_Y ) ) )
	{
		if ( mpParent && mpParent->maGeometry.nWidth >= nWidth && mpParent->maGeometry.nHeight > nHeight)
		{
			nX = nParentX + ( mpParent->maGeometry.nWidth - nWidth ) / 2;
			nY = nParentY + ( mpParent->maGeometry.nHeight - nHeight ) / 2;

			aWorkArea = Rectangle( Point( nX, nY ), Size( nWidth, nHeight ) );
			GetWorkArea( aWorkArea );
		}
		else
		{
			aWorkArea = Rectangle( Point( nX, nY ), Size( nWidth, nHeight ) );
			GetWorkArea( aWorkArea );

			nX = aWorkArea.nLeft + ( ( aWorkArea.GetWidth() - nWidth ) / 2 );
			nY = aWorkArea.nTop + ( ( aWorkArea.GetHeight() - nHeight ) / 2 );
		}

		mbCenter = FALSE;
	}
	else
	{
		aWorkArea = Rectangle( Point( nX, nY ), Size( nWidth, nHeight ) );
		GetWorkArea( aWorkArea );

		// Make sure that the work area intersects with the parent frame
		// so that dialogs don't show on a different monitor than the parent
		if ( mpParent )
		{
			Rectangle aParentBounds( mpParent->mpVCLFrame->getBounds() );
			if ( aWorkArea.GetIntersection( aParentBounds ).IsEmpty() )
			{
				aWorkArea = aParentBounds;
				GetWorkArea( aWorkArea );
			}
		}
	}

	// Make sure window does not spill off of the screen
	long nMinX = aWorkArea.nLeft;
	long nMinY = aWorkArea.nTop;
	if ( mbPresentation )
	{
		nMinX -= 1;
		nMinY -= 1;
	}
	nWidth += maGeometry.nLeftDecoration + maGeometry.nRightDecoration;
	nHeight += maGeometry.nTopDecoration + maGeometry.nBottomDecoration;
	if ( nMinX + nWidth > aWorkArea.nLeft + aWorkArea.GetWidth() )
		nWidth = aWorkArea.nLeft + aWorkArea.GetWidth() - nMinX;
	if ( nMinY + nHeight > aWorkArea.nTop + aWorkArea.GetHeight() )
		nHeight = aWorkArea.nTop + aWorkArea.GetHeight() - nMinY;
	if ( nX < nMinX )
		nX = nMinX;
	if ( nY < nMinY )
		nY = nMinY;

	// Fix bug 1420 by not restricting width or height to work area for current
	// drag frame
	if ( this != GetSalData()->mpLastDragFrame )
	{
		if ( nX + nWidth > aWorkArea.nLeft + aWorkArea.GetWidth() )
			nX = aWorkArea.nLeft + aWorkArea.GetWidth() - nWidth;
		if ( nY + nHeight > aWorkArea.nTop + aWorkArea.GetHeight() )
			nY = aWorkArea.nTop + aWorkArea.GetHeight() - nHeight;
	}

	mpVCLFrame->setBounds( nX, nY, nWidth, nHeight );

	// Update the cached position immediately
	com_sun_star_vcl_VCLEvent aEvent( SALEVENT_MOVERESIZE, this, NULL );
	aEvent.dispatch();

	mbInSetPosSize = FALSE;
}

// -----------------------------------------------------------------------

void JavaSalFrame::GetWorkArea( Rectangle &rRect )
{
	SalData *pSalData = GetSalData();
	sal_Bool bFullScreenMode = ( pSalData->mpPresentationFrame || ( this == pSalData->mpLastDragFrame ) );
	long nX = rRect.nLeft;
	long nY = rRect.nTop;
	long nWidth = rRect.GetWidth();
	long nHeight = rRect.GetHeight();
    
	// If the input rectangle is empty, we are being called by the platform
	// independent VCL code and so we need to use the parent window's bounds
	// if there is one
	if ( mpParent )
	{
		nX = mpParent->maGeometry.nX - maGeometry.nLeftDecoration;
		nY = mpParent->maGeometry.nY - maGeometry.nTopDecoration;
		nWidth  = mpParent->maGeometry.nWidth + maGeometry.nLeftDecoration + maGeometry.nRightDecoration;
		nHeight  = mpParent->maGeometry.nHeight + maGeometry.nTopDecoration + maGeometry.nBottomDecoration;
	}

	Rectangle aRect( com_sun_star_vcl_VCLScreen::getScreenBounds( nX, nY, nWidth, nHeight, bFullScreenMode ) );
	if ( aRect.GetWidth() > 0 && aRect.GetHeight() > 0 )
		rRect = aRect;
}

// -----------------------------------------------------------------------

void JavaSalFrame::GetClientSize( long& rWidth, long& rHeight )
{
	rWidth = maGeometry.nWidth;
	rHeight = maGeometry.nHeight;
}

// -----------------------------------------------------------------------

void JavaSalFrame::SetWindowState( const SalFrameState* pState )
{
	USHORT nFlags = 0;
	if ( pState->mnMask & SAL_FRAMESTATE_MASK_X )
		nFlags |= SAL_FRAME_POSSIZE_X;
	if ( pState->mnMask & SAL_FRAMESTATE_MASK_Y )
		nFlags |= SAL_FRAME_POSSIZE_Y;
	if ( pState->mnMask & SAL_FRAMESTATE_MASK_WIDTH )
		nFlags |= SAL_FRAME_POSSIZE_WIDTH;
	if ( pState->mnMask & SAL_FRAMESTATE_MASK_HEIGHT )
		nFlags |= SAL_FRAME_POSSIZE_HEIGHT;
	if ( nFlags )
	{
		JavaSalFrame *pParent = mpParent;
		mpParent = NULL;
		SetPosSize( pState->mnX, pState->mnY, pState->mnWidth, pState->mnHeight, nFlags );
		mpParent = pParent;
	}

	if ( pState->mnMask & SAL_FRAMESTATE_MASK_STATE )
	{
		if ( pState->mnState & SAL_FRAMESTATE_MINIMIZED )
			mpVCLFrame->setState( SAL_FRAMESTATE_MINIMIZED );
		else
			mpVCLFrame->setState( SAL_FRAMESTATE_NORMAL );
	}
}

// -----------------------------------------------------------------------

BOOL JavaSalFrame::GetWindowState( SalFrameState* pState )
{
	pState->mnMask = SAL_FRAME_POSSIZE_X | SAL_FRAME_POSSIZE_Y | SAL_FRAME_POSSIZE_WIDTH | SAL_FRAME_POSSIZE_HEIGHT | SAL_FRAMESTATE_MASK_STATE;
	pState->mnX = maGeometry.nX - maGeometry.nLeftDecoration;
	pState->mnY = maGeometry.nY - maGeometry.nTopDecoration;
	pState->mnWidth = maGeometry.nWidth;
	pState->mnHeight = maGeometry.nHeight;
	pState->mnState = mpVCLFrame->getState();

	return TRUE;
}

// -----------------------------------------------------------------------

void JavaSalFrame::ShowFullScreen( BOOL bFullScreen, sal_Int32 nDisplay )
{
	if ( bFullScreen == mbFullScreen )
		return;

	USHORT nFlags = SAL_FRAME_POSSIZE_X | SAL_FRAME_POSSIZE_Y | SAL_FRAME_POSSIZE_WIDTH | SAL_FRAME_POSSIZE_HEIGHT;
	if ( bFullScreen )
	{
		memcpy( &maOriginalGeometry, &maGeometry, sizeof( SalFrameGeometry ) );

		JavaSalSystem *pSalSystem = (JavaSalSystem *)ImplGetSalSystem();
		Rectangle aWorkArea;
		if ( pSalSystem )
			aWorkArea = pSalSystem->GetDisplayWorkAreaPosSizePixel( nDisplay );
		if ( aWorkArea.IsEmpty() )
			aWorkArea = Rectangle( Point( maGeometry.nX - maGeometry.nLeftDecoration, maGeometry.nY - maGeometry.nTopDecoration ), Size( maGeometry.nWidth, maGeometry.nHeight ) );
		GetWorkArea( aWorkArea );
		SetPosSize( aWorkArea.nLeft, aWorkArea.nTop, aWorkArea.GetWidth() - maGeometry.nLeftDecoration - maGeometry.nRightDecoration, aWorkArea.GetHeight() - maGeometry.nTopDecoration - maGeometry.nBottomDecoration, nFlags );
	}
	else
	{
		SetPosSize( maOriginalGeometry.nX - maOriginalGeometry.nLeftDecoration, maOriginalGeometry.nY - maOriginalGeometry.nTopDecoration, maOriginalGeometry.nWidth, maOriginalGeometry.nHeight, nFlags );
		memset( &maOriginalGeometry, 0, sizeof( SalFrameGeometry ) );
	}

	mpVCLFrame->setFullScreenMode( bFullScreen );
	mbFullScreen = bFullScreen;
}

// -----------------------------------------------------------------------

/**
 * Timer routine to toggle the system user interface mode on the
 * thread that is hosting our main runloop.  Starting with 10.3.8, it appears
 * that there is some type of contention that is causing SetSystemUIMode
 * to crash if it is invoked from a thread that is not the carbon runloop.
 *
 * @param aTimer	timer reference structure
 * @param pData		cookie passed to timer;  treated as a boolean value
 *			set to true if we're entering fullscreen presentation
 *			mode, false if we're returning to normal usage mode.
 */
static void SetSystemUIModeTimerCallback( EventLoopTimerRef aTimer, void *pData )
{
	bool enterFullscreen = (bool)pData;
	
	if ( enterFullscreen )
		SetSystemUIMode( kUIModeAllHidden, kUIOptionDisableAppleMenu | kUIOptionDisableProcessSwitch );
	else
		SetSystemUIMode( kUIModeNormal, 0 );
}

// -----------------------------------------------------------------------

void JavaSalFrame::StartPresentation( BOOL bStart )
{
	if ( bStart == mbPresentation )
		return;

	SalData *pSalData = GetSalData();

	// Only allow one frame to be in presentation mode at any one time
	if ( bStart && pSalData->mpPresentationFrame )
		return;
	else if ( !bStart && pSalData->mpPresentationFrame != this )
		return;

	mbPresentation = bStart;

	if ( mbPresentation )
		pSalData->mpPresentationFrame = this;
	else
		pSalData->mpPresentationFrame = NULL;

	// Adjust window size if in full screen mode
	bool bRunTimer = true;
	if ( mbFullScreen )
	{
		USHORT nFlags = SAL_FRAME_POSSIZE_X | SAL_FRAME_POSSIZE_Y | SAL_FRAME_POSSIZE_WIDTH | SAL_FRAME_POSSIZE_HEIGHT;

		Rectangle aWorkArea( Point( maGeometry.nX - maGeometry.nLeftDecoration, maGeometry.nY - maGeometry.nTopDecoration ), Size( maGeometry.nWidth, maGeometry.nHeight ) );
		GetWorkArea( aWorkArea );

		SetPosSize( aWorkArea.nLeft, aWorkArea.nTop, aWorkArea.GetWidth() - maGeometry.nLeftDecoration - maGeometry.nRightDecoration, aWorkArea.GetHeight() - maGeometry.nTopDecoration - maGeometry.nBottomDecoration, nFlags );

		JavaSalSystem *pSalSystem = (JavaSalSystem *)ImplGetSalSystem();
		if ( pSalSystem )
		{
			Rectangle aBounds( mpVCLFrame->getBounds() );
			Rectangle aScreenBounds( pSalSystem->GetDisplayScreenPosSizePixel( 0 ) );
			if ( !aBounds.IsEmpty() && !aScreenBounds.IsEmpty() && aBounds.Intersection( aScreenBounds ).IsEmpty() )
				bRunTimer = false;
		}
	}

	// [ed] 2/15/05 Change the SystemUIMode via timers so we can trigger
	// it on the main runloop thread.  Bug 484
	if ( bRunTimer )
	{
		if ( !pSetSystemUIModeTimerUPP )
			pSetSystemUIModeTimerUPP = NewEventLoopTimerUPP( SetSystemUIModeTimerCallback );
		if ( pSetSystemUIModeTimerUPP )
		{
			if ( GetCurrentEventLoop() != GetMainEventLoop() )
				InstallEventLoopTimer( GetMainEventLoop(), 0.001, kEventDurationForever, pSetSystemUIModeTimerUPP, (void *)( mbPresentation ? true : false ), NULL );
			else
				SetSystemUIModeTimerCallback( NULL, (void *)( mbPresentation ? true : false ) );
		}
	}
}

// -----------------------------------------------------------------------

void JavaSalFrame::SetAlwaysOnTop( BOOL bOnTop )
{
}

// -----------------------------------------------------------------------

void JavaSalFrame::ToTop( USHORT nFlags )
{
	// Make sure frame is a top-level window
	JavaSalFrame *pFrame = this;
	while ( pFrame && pFrame->IsFloatingFrame() && pFrame->mpParent && pFrame->mpParent->mbVisible )
		pFrame = pFrame->mpParent;

	if ( !pFrame || pFrame->IsFloatingFrame() || !pFrame->mbVisible )
		return;
	
	bool bSuccess;
	if ( nFlags & SAL_FRAME_TOTOP_GRABFOCUS )
		bSuccess = pFrame->mpVCLFrame->toFront();
	else if ( nFlags & SAL_FRAME_TOTOP_GRABFOCUS_ONLY )
		bSuccess = pFrame->mpVCLFrame->requestFocus();
	else
		bSuccess = false;

	// If Java has set the focus, update it now in the OOo code as it may
	// take a while before the Java event shows up in the queue. Fix bug
	// 1203 by not doing this update if we are in the Show() method.
	if ( bSuccess && !mbInShow )
	{
		com_sun_star_vcl_VCLEvent aEvent( SALEVENT_GETFOCUS, pFrame, NULL );
		aEvent.dispatch();
	}
}

// -----------------------------------------------------------------------

void JavaSalFrame::SetPointer( PointerStyle ePointerStyle )
{
	mpVCLFrame->setPointer( ePointerStyle );
}

// -----------------------------------------------------------------------

void JavaSalFrame::CaptureMouse( BOOL bCapture )
{
	SalData *pSalData = GetSalData();
	if ( bCapture )
		pSalData->mpCaptureFrame = this;
	else
		pSalData->mpCaptureFrame = NULL;
}

// -----------------------------------------------------------------------

void JavaSalFrame::SetPointerPos( long nX, long nY )
{
#ifdef DEBUG
	fprintf( stderr, "JavaSalFrame::SetPointerPos not implemented\n" );
#endif
}

// -----------------------------------------------------------------------

void JavaSalFrame::Flush()
{
	mpVCLFrame->sync();
}

// -----------------------------------------------------------------------

void JavaSalFrame::Sync()
{
	mpVCLFrame->sync();
}

// -----------------------------------------------------------------------

void JavaSalFrame::SetInputContext( SalInputContext* pContext )
{
#ifdef DEBUG
	fprintf( stderr, "JavaSalFrame::SetInputContext not implemented\n" );
#endif
}

// -----------------------------------------------------------------------

void JavaSalFrame::EndExtTextInput( USHORT nFlags )
{
#ifdef DEBUG
	fprintf( stderr, "JavaSalFrame::EndExtTextInput not implemented\n" );
#endif
}

// -----------------------------------------------------------------------

XubString JavaSalFrame::GetKeyName( USHORT nKeyCode )
{
	return mpVCLFrame->getKeyName( nKeyCode );
}

// -----------------------------------------------------------------------

XubString JavaSalFrame::GetSymbolKeyName( const XubString&, USHORT nKeyCode )
{
	return GetKeyName( nKeyCode );
}

// -----------------------------------------------------------------------

void JavaSalFrame::UpdateSettings( AllSettings& rSettings )
{
	MouseSettings aMouseSettings = rSettings.GetMouseSettings();
	ULONG nDblTime = (ULONG)GetDblTime();
	if ( nDblTime < 25 )
		nDblTime = 25;
	aMouseSettings.SetDoubleClickTime( nDblTime * 1000 / CLK_TCK );
	aMouseSettings.SetStartDragWidth( 6 );
	aMouseSettings.SetStartDragHeight( 6 );
	rSettings.SetMouseSettings( aMouseSettings );

	StyleSettings aStyleSettings( rSettings.GetStyleSettings() );

	long nBlinkRate = 500;
	CFPropertyListRef aInsertionPointBlinkPref = CFPreferencesCopyValue( CFSTR( "NSTextInsertionPointBlinkPeriod" ), kCFPreferencesAnyApplication, kCFPreferencesCurrentUser, kCFPreferencesAnyHost );
	if ( aInsertionPointBlinkPref )
	{
		if ( CFGetTypeID( aInsertionPointBlinkPref ) == CFNumberGetTypeID() && CFNumberGetValue( (CFNumberRef)aInsertionPointBlinkPref, kCFNumberLongType, &nBlinkRate ) && nBlinkRate < 500 )
			nBlinkRate = 500;
		CFRelease( aInsertionPointBlinkPref );
	}
	aStyleSettings.SetCursorBlinkTime( nBlinkRate );
	
	RGBColor theColor;
	
	BOOL useThemeDialogColor = FALSE;
	Color themeDialogColor;
	if( GetThemeTextColor( kThemeTextColorPushButtonActive /* used for text of all controls */, 32, true, &theColor ) == noErr )
	{
		themeDialogColor = RGBColorToColor( &theColor );
		useThemeDialogColor = TRUE;
	}
	
	SalColor nTextTextColor = com_sun_star_vcl_VCLScreen::getTextTextColor();
	Color aTextColor( SALCOLOR_RED( nTextTextColor ), SALCOLOR_GREEN( nTextTextColor ), SALCOLOR_BLUE( nTextTextColor ) );
	aStyleSettings.SetDialogTextColor( ( useThemeDialogColor ) ? themeDialogColor : aTextColor );
	aStyleSettings.SetMenuTextColor( aTextColor );
	aStyleSettings.SetButtonTextColor( ( useThemeDialogColor) ? themeDialogColor : aTextColor );
	aStyleSettings.SetRadioCheckTextColor( ( useThemeDialogColor ) ? themeDialogColor : aTextColor );
	aStyleSettings.SetGroupTextColor( ( useThemeDialogColor ) ? themeDialogColor : aTextColor );
	aStyleSettings.SetLabelTextColor( ( useThemeDialogColor ) ? themeDialogColor : aTextColor );
	aStyleSettings.SetInfoTextColor( aTextColor );
	aStyleSettings.SetWindowTextColor( aTextColor );
	aStyleSettings.SetFieldTextColor( aTextColor );

	SalColor nTextHighlightColor = com_sun_star_vcl_VCLScreen::getTextHighlightColor();
	Color aHighlightColor( SALCOLOR_RED( nTextHighlightColor ), SALCOLOR_GREEN( nTextHighlightColor ), SALCOLOR_BLUE( nTextHighlightColor ) );
	aStyleSettings.SetActiveBorderColor( aHighlightColor );
	aStyleSettings.SetActiveColor( aHighlightColor );
	aStyleSettings.SetActiveTextColor( aHighlightColor );
	aStyleSettings.SetHighlightColor( aHighlightColor );
	aStyleSettings.SetMenuHighlightColor( aHighlightColor );

	SalColor nTextHighlightTextColor = com_sun_star_vcl_VCLScreen::getTextHighlightTextColor();
	Color aHighlightTextColor( SALCOLOR_RED( nTextHighlightTextColor ), SALCOLOR_GREEN( nTextHighlightTextColor ), SALCOLOR_BLUE( nTextHighlightTextColor ) );
	aStyleSettings.SetHighlightTextColor( aHighlightTextColor );
	aStyleSettings.SetMenuHighlightTextColor( aHighlightTextColor );
	
	useThemeDialogColor = FALSE;
	if( GetThemeTextColor( kThemeTextColorPushButtonInactive /* used for text of all disabled controls */, 32, true, &theColor ) == noErr )
	{
		themeDialogColor = RGBColorToColor( &theColor );
		useThemeDialogColor = TRUE;
	}
	
	SalColor nControlColor = com_sun_star_vcl_VCLScreen::getControlColor();
	Color aBackColor( SALCOLOR_RED( nControlColor ), SALCOLOR_GREEN( nControlColor ), SALCOLOR_BLUE( nControlColor ) );
	aStyleSettings.Set3DColors( aBackColor );
	aStyleSettings.SetDeactiveBorderColor( aBackColor );
	aStyleSettings.SetDeactiveColor( aBackColor );
	aStyleSettings.SetDeactiveTextColor( ( useThemeDialogColor ) ? themeDialogColor : aBackColor );
	aStyleSettings.SetDialogColor( aBackColor );
	aStyleSettings.SetDisableColor( aBackColor );
	aStyleSettings.SetFaceColor( aBackColor );
	aStyleSettings.SetMenuColor( aBackColor );
	aStyleSettings.SetMenuBarColor( aBackColor );
	if( aBackColor == COL_LIGHTGRAY )
	{
		aStyleSettings.SetCheckedColor( Color( 0xCC, 0xCC, 0xCC ) );
	}
	else
	{
		Color aColor2 = aStyleSettings.GetLightColor();
		aStyleSettings.SetCheckedColor( Color( (BYTE)( ( (USHORT)aBackColor.GetRed() + (USHORT)aColor2.GetRed() ) / 2 ), (BYTE)( ( (USHORT)aBackColor.GetGreen() + (USHORT)aColor2.GetGreen() ) / 2 ), (BYTE)( ( (USHORT)aBackColor.GetBlue() + (USHORT)aColor2.GetBlue() ) / 2 ) ) );
	}

	rSettings.SetStyleSettings( aStyleSettings );
}

// -----------------------------------------------------------------------

SalBitmap* JavaSalFrame::SnapShot()
{
#ifdef DEBUG
	fprintf( stderr, "JavaSalFrame::Snapshot not implemented\n" );
#endif
	return NULL;
}

// -----------------------------------------------------------------------

const SystemEnvData* JavaSalFrame::GetSystemData() const
{
	return &maSysData;
}

// -----------------------------------------------------------------------

void JavaSalFrame::Beep( SoundType eSoundType )
{
	com_sun_star_vcl_VCLGraphics::beep();
}

// -----------------------------------------------------------------------

SalFrame* JavaSalFrame::GetParent() const
{
	return mpParent;
}

// -----------------------------------------------------------------------

LanguageType JavaSalFrame::GetInputLanguage()
{
#ifdef DEBUG
	fprintf( stderr, "JavaSalFrame::GetInputLanguage not implemented\n" );
#endif
	return LANGUAGE_DONTKNOW;
}

// -----------------------------------------------------------------------

void JavaSalFrame::SetParent( SalFrame* pNewParent )
{
	if ( mpParent )
	{
		mpParent->maChildren.remove( this );
		mpParent->mpVCLFrame->removeChild( this );
	}

	mpParent = (JavaSalFrame *)pNewParent;

	::std::list< JavaSalObject* > aReshowObjects( maObjects );
	bool bReshow = mbVisible;
	if ( bReshow )
		Show( FALSE );

	// Fix bug 1310 by creating a new native window with the new parent
	if ( mpGraphics->mpVCLGraphics )
		delete mpGraphics->mpVCLGraphics;
	if ( mpVCLFrame )
	{
		mpVCLFrame->dispose();
		delete mpVCLFrame;
	}
	mpVCLFrame = new com_sun_star_vcl_VCLFrame( mnStyle, this, mpParent );
	if ( mpVCLFrame )
	{
		mpVCLFrame->setTitle( maTitle );
		mpGraphics->mpVCLGraphics = mpVCLFrame->getGraphics();
	}
	maSysData.aWindow = 0;
	if ( mpParent )
		SetPosSize( maGeometry.nX - mpParent->maGeometry.nX - mpParent->maGeometry.nLeftDecoration, maGeometry.nY - mpParent->maGeometry.nY - mpParent->maGeometry.nTopDecoration, maGeometry.nWidth, maGeometry.nHeight, SAL_FRAME_POSSIZE_X | SAL_FRAME_POSSIZE_Y | SAL_FRAME_POSSIZE_WIDTH | SAL_FRAME_POSSIZE_HEIGHT );
	else
		SetPosSize( maGeometry.nX - maGeometry.nLeftDecoration, maGeometry.nY - maGeometry.nTopDecoration, maGeometry.nWidth, maGeometry.nHeight, SAL_FRAME_POSSIZE_X | SAL_FRAME_POSSIZE_Y | SAL_FRAME_POSSIZE_WIDTH | SAL_FRAME_POSSIZE_HEIGHT );

	if ( mpParent )
	{
		mpParent->mpVCLFrame->addChild( this );
		mpParent->maChildren.push_back( this );
	}

	if ( bReshow )
	{
		Show( TRUE, FALSE );
		for ( ::std::list< JavaSalObject* >::const_iterator it = aReshowObjects.begin(); it != aReshowObjects.end(); ++it )
			(*it)->Show( TRUE );
	}

	// Reattach floating children
	::std::list< JavaSalFrame* >aChildren( maChildren );
	for ( ::std::list< JavaSalFrame* >::const_iterator it = aChildren.begin(); it != aChildren.end(); ++it )
		(*it)->SetParent( this );
}

// -----------------------------------------------------------------------

bool JavaSalFrame::SetPluginParent( SystemParentData* pNewParent )
{
#ifdef DEBUG
	fprintf( stderr, "JavaSalFrame::SetPluginParent not implemented\n" );
#endif
	return false;
}

// -----------------------------------------------------------------------

void JavaSalFrame::SetMenu( SalMenu* pSalMenu )
{
	JavaSalMenu *pJavaSalMenu = (JavaSalMenu *)pSalMenu;
	if ( pJavaSalMenu && pJavaSalMenu->mbIsMenuBarMenu )
	{
		mpMenuBar = pJavaSalMenu;

		// If the menu is being set while the window is showing, we need
		// to update the new menus. Fix bug 2577 by only updating the menubar
		// and not its submenus.
		if ( mbVisible && mpMenuBar->mpParentVCLMenu )
		{
			Menu *pVCLMenu = mpMenuBar->mpParentVCLMenu;

			// Post the SALEVENT_MENUACTIVATE event
			SalMenuEvent *pActivateEvent = new SalMenuEvent();
			pActivateEvent->mnId = 0;
			pActivateEvent->mpMenu = pVCLMenu;
			com_sun_star_vcl_VCLEvent aActivateEvent( SALEVENT_MENUACTIVATE, this, pActivateEvent );
			aActivateEvent.dispatch();

			// Post the SALEVENT_MENUDEACTIVATE event
			SalMenuEvent *pDeactivateEvent = new SalMenuEvent();
			pDeactivateEvent->mnId = 0;
			pDeactivateEvent->mpMenu = pVCLMenu;
			com_sun_star_vcl_VCLEvent aDeactivateEvent( SALEVENT_MENUDEACTIVATE, this, pDeactivateEvent );
			aDeactivateEvent.dispatch();
 
			// Explicitly set the focus so that Java will repaint the menubar
			if ( this == GetSalData()->mpFocusFrame )
				mpVCLFrame->requestFocus();

			SalData *pSalData = GetSalData();

			// Post events to update menus asynchronously
			USHORT nCount = pVCLMenu->GetItemCount();
			for( USHORT i = 0; i < nCount; i++ )
			{
				JavaSalMenuItem *pSalMenuItem = (JavaSalMenuItem *)pVCLMenu->GetItemSalItem( i );
				if ( pSalMenuItem && pSalMenuItem->mpSalSubmenu )
				{
					Menu *pVCLSubMenu = pSalMenuItem->mpSalSubmenu->mpParentVCLMenu;

					// Post the SALEVENT_MENUACTIVATE event
					SalMenuEvent *pSubActivateEvent = new SalMenuEvent();
					pSubActivateEvent->mnId = 0;
					pSubActivateEvent->mpMenu = pVCLSubMenu;
					com_sun_star_vcl_VCLEvent aSubActivateEvent( SALEVENT_MENUACTIVATE, this, pSubActivateEvent );
					pSalData->mpEventQueue->postCachedEvent( &aSubActivateEvent );

					// Post the SALEVENT_MENUDEACTIVATE event
					SalMenuEvent *pSubDeactivateEvent = new SalMenuEvent();
					pSubDeactivateEvent->mnId = 0;
					pSubDeactivateEvent->mpMenu = pVCLSubMenu;
					com_sun_star_vcl_VCLEvent aSubDeactivateEvent( SALEVENT_MENUDEACTIVATE, this, pSubDeactivateEvent );
					pSalData->mpEventQueue->postCachedEvent( &aSubDeactivateEvent );
				}
			}
		}
	}
	else
	{
		mpMenuBar = NULL;
	}
}

// -----------------------------------------------------------------------

void JavaSalFrame::DrawMenuBar()
{
}

// -----------------------------------------------------------------------

SalFrame::SalPointerState JavaSalFrame::GetPointerState()
{
	SalData *pSalData = GetSalData();

	SalPointerState aState;
	aState.mnState = pSalData->maLastPointerState.mnState;
	aState.maPos = Point( pSalData->maLastPointerState.maPos.X() - maGeometry.nX, pSalData->maLastPointerState.maPos.Y() - maGeometry.nY );

	return aState;
}

// -----------------------------------------------------------------------

BOOL JavaSalFrame::MapUnicodeToKeyCode( sal_Unicode aUnicode, LanguageType aLangType, KeyCode& rKeyCode )
{
	return FALSE;
}

// -----------------------------------------------------------------------

void JavaSalFrame::SetExtendedFrameStyle( SalExtStyle nExtStyle )
{
}

// -----------------------------------------------------------------------

void JavaSalFrame::SetBackgroundBitmap( SalBitmap* )
{
}

// -----------------------------------------------------------------------

void JavaSalFrame::SetMaxClientSize( long nWidth, long nHeight )
{
#ifdef DEBUG
	fprintf( stderr, "JavaSalFrame::SetMaxClientSize not implemented\n" );
#endif
}

// -----------------------------------------------------------------------

void JavaSalFrame::ResetClipRegion()
{
	if ( !mpGraphics->mpVCLGraphics )
		mpGraphics->mpVCLGraphics = mpVCLFrame->getGraphics();
	mpGraphics->mpVCLGraphics->resetClipRegion( sal_True );
}

// -----------------------------------------------------------------------

void JavaSalFrame::BeginSetClipRegion( ULONG nRects )
{
	if ( !mpGraphics->mpVCLGraphics )
		mpGraphics->mpVCLGraphics = mpVCLFrame->getGraphics();
	mpGraphics->mpVCLGraphics->beginSetClipRegion( sal_True );
}

// -----------------------------------------------------------------------

void JavaSalFrame::UnionClipRegion( long nX, long nY, long nWidth, long nHeight )
{
	if ( !mpGraphics->mpVCLGraphics )
		mpGraphics->mpVCLGraphics = mpVCLFrame->getGraphics();
	mpGraphics->mpVCLGraphics->unionClipRegion( nX, nY, nWidth, nHeight, sal_True );
}

// -----------------------------------------------------------------------

void JavaSalFrame::EndSetClipRegion()
{
	if ( !mpGraphics->mpVCLGraphics )
		mpGraphics->mpVCLGraphics = mpVCLFrame->getGraphics();
	mpGraphics->mpVCLGraphics->endSetClipRegion( sal_True );
}
