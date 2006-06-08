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

#define _SV_SALFRAME_CXX

#ifndef _SV_SALFRAME_H
#include <salframe.h>
#endif
#ifndef _SV_SALGDI_H
#include <salgdi.h>
#endif
#ifndef _SV_SALDATA_HXX
#include <saldata.hxx>
#endif
#ifndef _SV_SALMENU_H
#include <salmenu.h>
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

#include <premac.h>
#include <Carbon/Carbon.h>
#include <postmac.h>
#undef check

#include "salframe_cocoa.h"

static EventLoopTimerUPP pSetSystemUIModeTimerUPP = NULL;

using namespace rtl;
using namespace vcl;

// =======================================================================

long ImplSalCallbackDummy( void*, SalFrame*, USHORT, const void* )
{
	return 0;
}

// -----------------------------------------------------------------------

void VCLScreen_getScreenBounds( long *nX, long *nY, long *nWidth, long *nHeight, BOOL bFullScreenMode, BOOL bUseMainScreenOnly )
{
	Rectangle aRect = com_sun_star_vcl_VCLScreen::getScreenBounds( *nX, *nY, *nWidth, *nHeight, bFullScreenMode, bUseMainScreenOnly );
	if ( !aRect.IsEmpty() )
	{
		*nX = aRect.nLeft;
		*nY = aRect.nTop;
		*nWidth = aRect.GetWidth();
		*nHeight = aRect.GetHeight();
	}
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
	mbUseMainScreenOnly = TRUE;
	mbInSetPosSize = FALSE;
	mbInShow = FALSE;
}

// -----------------------------------------------------------------------

JavaSalFrame::~JavaSalFrame()
{
	Show( FALSE );
	StartPresentation( FALSE );

	if ( mpVCLFrame )
	{
		mpVCLFrame->dispose();
		delete mpVCLFrame;
	}

	delete mpGraphics;
}

// -----------------------------------------------------------------------

bool JavaSalFrame::IsFloatingFrame()
{
	return ( ! ( mnStyle & ( SAL_FRAME_STYLE_DEFAULT | SAL_FRAME_STYLE_MOVEABLE | SAL_FRAME_STYLE_SIZEABLE ) ) && this != GetSalData()->mpPresentationFrame );
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

	SalData *pSalData = GetSalData();

	mbVisible = bVisible;

	// Make sure there is a graphics available to avoid crashing when the OOo
	// code tries to draw while updating the menus
	if ( !mpGraphics->mpVCLGraphics )
		mpGraphics->mpVCLGraphics = mpVCLFrame->getGraphics();

	// Do some of the updating before the window is shown
	if ( mbVisible )
		UpdateMenusForFrame( this, NULL );

	mpVCLFrame->setVisible( mbVisible );

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

		// Reattach child frames
		for ( ::std::list< JavaSalFrame* >::const_iterator it = maChildren.begin(); it != maChildren.end(); ++it )
		{
			if ( (*it)->mbVisible )
			{
				mpVCLFrame->removeChild( *it );
				(*it)->Show( FALSE );

				// Fix bug 1310 by creating a new native window with the new
				// parent
				if ( (*it)->mpGraphics->mpVCLGraphics )
					(*it)->mpGraphics->mpVCLGraphics;
				if ( (*it)->mpVCLFrame )
				{
					(*it)->mpVCLFrame->dispose();
					delete (*it)->mpVCLFrame;
				}
				(*it)->mpVCLFrame = new com_sun_star_vcl_VCLFrame( (*it)->mnStyle, *it, this );
				(*it)->mpGraphics->mpVCLGraphics = (*it)->mpVCLFrame->getGraphics();
				(*it)->maSysData.aWindow = 0;
				(*it)->SetPosSize( (*it)->maGeometry.nX - (*it)->maGeometry.nLeftDecoration - maGeometry.nX, (*it)->maGeometry.nY - (*it)->maGeometry.nTopDecoration - maGeometry.nY, (*it)->maGeometry.nWidth, (*it)->maGeometry.nHeight, SAL_FRAME_POSSIZE_X | SAL_FRAME_POSSIZE_Y | SAL_FRAME_POSSIZE_WIDTH | SAL_FRAME_POSSIZE_HEIGHT );

				(*it)->Show( TRUE, FALSE );
				mpVCLFrame->addChild( *it );
			}
		}

		// Explicitly set focus to this frame since Java may set the focus
		// to the child frame
		if ( !bNoActivate )
			ToTop( SAL_FRAME_TOTOP_RESTOREWHENMIN | SAL_FRAME_TOTOP_GRABFOCUS );
		else if ( pSalData->mpFocusFrame )
			pSalData->mpFocusFrame->ToTop( SAL_FRAME_TOTOP_RESTOREWHENMIN | SAL_FRAME_TOTOP_GRABFOCUS );

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

			// Make sure frame is a top-level window
			JavaSalFrame *pFocusFrame = this;
			while ( pFocusFrame->mpParent && pFocusFrame->mpParent->mbVisible )
				pFocusFrame = pFocusFrame->mpParent;
	
			if ( pFocusFrame != this )
				pFocusFrame->ToTop( SAL_FRAME_TOTOP_RESTOREWHENMIN | SAL_FRAME_TOTOP_GRABFOCUS );
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
	if ( mpParent )
	{
		if ( nFlags & ( SAL_FRAME_POSSIZE_X | SAL_FRAME_POSSIZE_Y ) && Application::GetSettings().GetLayoutRTL() )
			nX = mpParent->maGeometry.nWidth - nWidth - nX - 1;

		if ( nFlags & SAL_FRAME_POSSIZE_X )
			nX += mpParent->maGeometry.nX;
		if ( nFlags & SAL_FRAME_POSSIZE_Y )
			nY += mpParent->maGeometry.nY;
	}

	Rectangle aWorkArea;
	if ( mbCenter && ! ( nFlags & ( SAL_FRAME_POSSIZE_X | SAL_FRAME_POSSIZE_Y ) ) )
	{
		if ( mpParent && mpParent->maGeometry.nWidth >= nWidth && mpParent->maGeometry.nHeight > nHeight)
		{
			nX = mpParent->maGeometry.nX + ( mpParent->maGeometry.nWidth - nWidth ) / 2;
			nY = mpParent->maGeometry.nY + ( mpParent->maGeometry.nHeight - nHeight ) / 2;

			aWorkArea = Rectangle( Point( nX, nX ), Size( nWidth, nHeight ) );
			GetWorkArea( aWorkArea );
		}
		else
		{
			aWorkArea = Rectangle( Point( nX, nX ), Size( nWidth, nHeight ) );
			GetWorkArea( aWorkArea );

			nX = aWorkArea.nLeft + ( ( aWorkArea.GetWidth() - nWidth ) / 2 );
			nY = aWorkArea.nTop + ( ( aWorkArea.GetHeight() - nHeight ) / 2 );
		}

		mbCenter = FALSE;
	}
	else
	{
		aWorkArea = Rectangle( Point( nX, nX ), Size( nWidth, nHeight ) );
		GetWorkArea( aWorkArea );
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
	Rectangle *pRect = new Rectangle( Point( nX, nY ), Size( nWidth, nHeight ) );
	com_sun_star_vcl_VCLEvent aEvent( SALEVENT_MOVERESIZE, this, (void *)pRect );
	aEvent.dispatch();

	mbInSetPosSize = FALSE;
}

// -----------------------------------------------------------------------

void JavaSalFrame::GetWorkArea( Rectangle &rRect )
{
	if ( rRect.IsEmpty() )
		rRect = mpVCLFrame->getBounds();

	SalData *pSalData = GetSalData();
	BOOL bFullScreenMode = ( pSalData->mpPresentationFrame || ( this == pSalData->mpLastDragFrame ) );
	NSScreen_getScreenBounds( &rRect.nLeft, &rRect.nTop, &rRect.nRight, &rRect.nBottom, bFullScreenMode, mbUseMainScreenOnly );
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
		mbUseMainScreenOnly = FALSE;

		Rectangle aPosSize( Point( pState->mnX, pState->mnY ), Size( pState->mnWidth, pState->mnHeight ) );
		if ( mpParent )
			aPosSize.Move( -mpParent->maGeometry.nX, -mpParent->maGeometry.nY );
		SetPosSize( aPosSize.nLeft, aPosSize.nTop, aPosSize.GetWidth(), aPosSize.GetHeight(), nFlags );
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

void JavaSalFrame::ShowFullScreen( BOOL bFullScreen )
{
	if ( bFullScreen == mbFullScreen )
		return;

	USHORT nFlags = SAL_FRAME_POSSIZE_X | SAL_FRAME_POSSIZE_Y | SAL_FRAME_POSSIZE_WIDTH | SAL_FRAME_POSSIZE_HEIGHT;

	if ( bFullScreen )
	{
		SalData *pSalData = GetSalData();
		memcpy( &maOriginalGeometry, &maGeometry, sizeof( SalFrameGeometry ) );
		Rectangle aWorkArea;
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

	// [ed] 2/15/05 Change the SystemUIMode via timers so we can trigger
	// it on the main runloop thread.  Bug 484
	if ( !pSetSystemUIModeTimerUPP )
		pSetSystemUIModeTimerUPP = NewEventLoopTimerUPP( SetSystemUIModeTimerCallback );
	if ( pSetSystemUIModeTimerUPP )
	{
		if ( GetCurrentEventLoop() != GetMainEventLoop() )
			InstallEventLoopTimer( GetMainEventLoop(), 0.001, kEventDurationForever, pSetSystemUIModeTimerUPP, (void *)( bStart ? true : false ), NULL );
		else
			SetSystemUIModeTimerCallback( NULL, (void *)( bStart ? true : false ) );
	}

	mbPresentation = bStart;

	if ( mbPresentation )
		pSalData->mpPresentationFrame = this;
	else
		pSalData->mpPresentationFrame = NULL;

	// Adjust window size if in full screen mode
	if ( mbFullScreen )
	{
		USHORT nFlags = SAL_FRAME_POSSIZE_X | SAL_FRAME_POSSIZE_Y | SAL_FRAME_POSSIZE_WIDTH | SAL_FRAME_POSSIZE_HEIGHT;

		Rectangle aWorkArea;
		GetWorkArea( aWorkArea );

		SetPosSize( aWorkArea.nLeft, aWorkArea.nTop, aWorkArea.GetWidth() - maGeometry.nLeftDecoration - maGeometry.nRightDecoration, aWorkArea.GetHeight() - maGeometry.nTopDecoration - maGeometry.nBottomDecoration, nFlags );
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
#ifdef DEBUG
	fprintf( stderr, "JavaSalFrame::CaptureMouse not implemented\n" );
#endif
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
	mpVCLFrame->endComposition();
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

	aStyleSettings.SetCursorBlinkTime( 500 );

	SalColor nTextTextColor = com_sun_star_vcl_VCLScreen::getTextTextColor();
	Color aTextColor( SALCOLOR_RED( nTextTextColor ), SALCOLOR_GREEN( nTextTextColor ), SALCOLOR_BLUE( nTextTextColor ) );
	aStyleSettings.SetDialogTextColor( aTextColor );
	aStyleSettings.SetMenuTextColor( aTextColor );
	aStyleSettings.SetButtonTextColor( aTextColor );
	aStyleSettings.SetRadioCheckTextColor( aTextColor );
	aStyleSettings.SetGroupTextColor( aTextColor );
	aStyleSettings.SetLabelTextColor( aTextColor );
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

	SalColor nControlColor = com_sun_star_vcl_VCLScreen::getControlColor();
	Color aBackColor( SALCOLOR_RED( nControlColor ), SALCOLOR_GREEN( nControlColor ), SALCOLOR_BLUE( nControlColor ) );
	aStyleSettings.Set3DColors( aBackColor );
	aStyleSettings.SetDeactiveBorderColor( aBackColor );
	aStyleSettings.SetDeactiveColor( aBackColor );
	aStyleSettings.SetDeactiveTextColor( aBackColor );
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
	if ( pNewParent != mpParent )
	{
		if ( mpParent )
		{
			mpParent->maChildren.remove( this );
			mpParent->mpVCLFrame->removeChild( this );
		}

		JavaSalFrame *mpOldParent = mpParent;
		mpParent = (JavaSalFrame *)pNewParent;

		if ( mpParent )
			mbUseMainScreenOnly = FALSE;

		if ( ( mpOldParent && mpOldParent->mbVisible ) || ( mpParent && mpParent->mbVisible ) )
		{
			// Fix bug 1310 by creating a new native window with the new parent
			if ( mpGraphics->mpVCLGraphics )
				delete mpGraphics->mpVCLGraphics;
			if ( mpVCLFrame )
			{
				mpVCLFrame->dispose();
				delete mpVCLFrame;
			}
			mpVCLFrame = new com_sun_star_vcl_VCLFrame( mnStyle, this, mpParent );
			mpGraphics->mpVCLGraphics = mpVCLFrame->getGraphics();
			maSysData.aWindow = 0;
			SetPosSize( maGeometry.nX - maGeometry.nLeftDecoration, maGeometry.nY - maGeometry.nTopDecoration, maGeometry.nWidth, maGeometry.nHeight, SAL_FRAME_POSSIZE_X | SAL_FRAME_POSSIZE_Y | SAL_FRAME_POSSIZE_WIDTH | SAL_FRAME_POSSIZE_HEIGHT );
		}

		if ( mpParent )
		{
			mpParent->mpVCLFrame->addChild( this );
			mpParent->maChildren.push_back( this );
		}
	}
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
		// to update the new menus
		if ( mbVisible )
		{
			UpdateMenusForFrame( this, NULL );
 
			// Explicitly set the focus so that Java will repaint the menubar
			if ( this == GetSalData()->mpFocusFrame )
				mpVCLFrame->requestFocus();
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
