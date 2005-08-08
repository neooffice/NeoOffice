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

#ifndef _SV_SALFRAME_HXX
#include <salframe.hxx>
#endif
#ifndef _SV_SALGDI_HXX
#include <salgdi.hxx>
#endif
#ifndef _SV_SALDATA_HXX
#include <saldata.hxx>
#endif
#ifndef _SV_SALMENU_HXX
#include <salmenu.hxx>
#endif
#ifndef _SV_SALSYS_HXX
#include <salsys.hxx>
#endif
#ifndef _SV_SETTINGS_HXX
#include <settings.hxx>
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

static EventHandlerUPP pEventHandlerUPP = NULL;
static EventLoopTimerUPP pSetSystemUIModeTimerUPP = NULL;

using namespace rtl;
using namespace vcl;

// =======================================================================

long ImplSalCallbackDummy( void*, SalFrame*, USHORT, const void* )
{
	return 0;
}

// -----------------------------------------------------------------------

bool GetSalSystemDisplayInfo( System::DisplayInfo& rInfo )
{
#ifdef DEBUG
	fprintf( stderr, "GetSalSystemDisplayInfo not implemented\n" );
#endif
	return false;
}

// -----------------------------------------------------------------------

int ImplShowNativeMessageBox( const String& rTitle, const String& rMessage,
                              int nButtonCombination, int nDefaultButton)
{
#ifdef DEBUG
	fprintf( stderr, "ImplShowNativeMessageBox not implemented\n" );
#endif
	return 0;
}

// =======================================================================

SalFrame::SalFrame()
{
	memset( &maGeometry, 0, sizeof( maGeometry ) );
	maFrameData.mpGraphics->maGraphicsData.mpFrame = this;
}

// -----------------------------------------------------------------------

SalFrame::~SalFrame()
{
	SalData *pSalData = GetSalData();

	if ( pSalData->mpFocusFrame == this )
		pSalData->mpFocusFrame = NULL;

	if ( pSalData->mpPresentationFrame == this )
		pSalData->mpPresentationFrame = NULL;

	pSalData->maAlwaysOnTopFrameList.remove( this );
}

// -----------------------------------------------------------------------

SalGraphics* SalFrame::GetGraphics()
{
	if ( maFrameData.mbGraphics )
		return NULL;

	maFrameData.mpGraphics->maGraphicsData.mpVCLGraphics = maFrameData.mpVCLFrame->getGraphics();
	maFrameData.mbGraphics = TRUE;

	return maFrameData.mpGraphics;
}

// -----------------------------------------------------------------------

void SalFrame::ReleaseGraphics( SalGraphics* pGraphics )
{
	if ( pGraphics != maFrameData.mpGraphics )
		return;

	if ( maFrameData.mpGraphics->maGraphicsData.mpVCLGraphics )
		delete maFrameData.mpGraphics->maGraphicsData.mpVCLGraphics;
	maFrameData.mpGraphics->maGraphicsData.mpVCLGraphics = NULL;
	maFrameData.mbGraphics = FALSE;
}

// -----------------------------------------------------------------------

BOOL SalFrame::PostEvent( void *pData )
{
	com_sun_star_vcl_VCLEvent aEvent( SALEVENT_USEREVENT, this, pData );
	GetSalData()->mpEventQueue->postCachedEvent( &aEvent );
	return TRUE;
}

// -----------------------------------------------------------------------

void SalFrame::SetTitle( const XubString& rTitle )
{
	maFrameData.mpVCLFrame->setTitle( rTitle );
}

// -----------------------------------------------------------------------

void SalFrame::SetIcon( USHORT nIcon )
{
#ifdef DEBUG
	fprintf( stderr, "SalFrame::SetIcon not implemented\n" );
#endif
}

// -----------------------------------------------------------------------

void SalFrame::Show( BOOL bVisible, BOOL bNoActivate )
{
	if ( bVisible == maFrameData.mbVisible )
		return;

	SalData *pSalData = GetSalData();

	maFrameData.mbVisible = bVisible;

	maFrameData.mpVCLFrame->setVisible( maFrameData.mbVisible );

	// Reset graphics
	com_sun_star_vcl_VCLGraphics *pVCLGraphics = maFrameData.mpVCLFrame->getGraphics();
	if ( pVCLGraphics )
	{
		pVCLGraphics->resetGraphics();
		delete pVCLGraphics;
	}

	if ( maFrameData.mbVisible )
	{
		// Make a pass through the native menus to speed up later updates
		UpdateMenusForFrame( this, NULL );
	}
	else
	{
		if ( pSalData->mpFocusFrame == this )
			pSalData->mpFocusFrame = NULL;

		if ( maFrameData.mpParent )
			maFrameData.mpParent->ToTop( SAL_FRAME_TOTOP_GRABFOCUS );
	}
}

// -----------------------------------------------------------------------

void SalFrame::Enable( BOOL bEnable )
{
#ifdef DEBUG
	fprintf( stderr, "SalFrame::Enable not implemented\n" );
#endif
}

// -----------------------------------------------------------------------

void SalFrame::SetMinClientSize( long nWidth, long nHeight )
{
	maFrameData.mpVCLFrame->setMinClientSize( nWidth, nHeight );
}

// -----------------------------------------------------------------------

void SalFrame::SetPosSize( long nX, long nY, long nWidth, long nHeight,
							USHORT nFlags )
{
	if ( maFrameData.mnStyle & SAL_FRAME_STYLE_CHILD )
		return;

	Rectangle aPosSize( Point( maGeometry.nX - maGeometry.nLeftDecoration, maGeometry.nY - maGeometry.nTopDecoration ), Size( maGeometry.nWidth, maGeometry.nHeight ) );
	aPosSize.Justify();

	if ( ! ( nFlags & SAL_FRAME_POSSIZE_X ) )
		nX = aPosSize.nLeft;
	if ( ! ( nFlags & SAL_FRAME_POSSIZE_Y ) )
		nY = aPosSize.nTop;
	if ( ! ( nFlags & SAL_FRAME_POSSIZE_WIDTH ) )
		nWidth = aPosSize.GetWidth();
	if ( ! ( nFlags & SAL_FRAME_POSSIZE_HEIGHT ) )
		nHeight = aPosSize.GetHeight();

	// Adjust position for RTL layout
	if ( maFrameData.mpParent && nFlags & ( SAL_FRAME_POSSIZE_X | SAL_FRAME_POSSIZE_Y ) && Application::GetSettings().GetLayoutRTL() )
	{
		Rectangle aParentPosSize( Point( maFrameData.mpParent->maGeometry.nX - maFrameData.mpParent->maGeometry.nLeftDecoration, maFrameData.mpParent->maGeometry.nY - maFrameData.mpParent->maGeometry.nTopDecoration ), Size( maFrameData.mpParent->maGeometry.nWidth, maFrameData.mpParent->maGeometry.nHeight ) );
		nX = aParentPosSize.GetWidth() - nWidth - nX - 1;
	}

	Rectangle aWorkArea;

	if ( maFrameData.mbCenter && ! ( nFlags & ( SAL_FRAME_POSSIZE_X | SAL_FRAME_POSSIZE_Y ) ) )
	{
		if ( maFrameData.mpParent )
			maFrameData.mpParent->GetWorkArea( aWorkArea );
		else
			GetWorkArea( aWorkArea );
		if ( maFrameData.mpParent && maFrameData.mpParent->maGeometry.nWidth >= nWidth && maFrameData.mpParent->maGeometry.nHeight > nHeight)
		{
			nX = maFrameData.mpParent->maGeometry.nX + ( ( maFrameData.mpParent->maGeometry.nWidth - nWidth ) / 2 );
			nY = maFrameData.mpParent->maGeometry.nY + ( ( maFrameData.mpParent->maGeometry.nHeight - nHeight ) / 2 );
		}
		else
		{
			nX = aWorkArea.nLeft + ( ( aWorkArea.GetWidth() - nWidth ) / 2 );
			nY = aWorkArea.nTop + ( ( aWorkArea.GetHeight() - nHeight ) / 2 );
		}

		maFrameData.mbCenter = FALSE;
	}
	else if ( maFrameData.mpParent )
	{
		if ( nFlags & SAL_FRAME_POSSIZE_X )
			nX += maFrameData.mpParent->maGeometry.nX;
		if ( nFlags & SAL_FRAME_POSSIZE_Y )
			nY += maFrameData.mpParent->maGeometry.nY;

		// If this is a popup window, we need to put the window on the correct
		// screen when the parent window straddles more than one screen
		Rectangle aBounds( Point( nX, nY ), Size( nWidth + maGeometry.nLeftDecoration + maGeometry.nRightDecoration, nHeight + maGeometry.nTopDecoration + maGeometry.nBottomDecoration ) );
		maFrameData.mpVCLFrame->setBounds( aBounds.nLeft, aBounds.nTop, aBounds.GetWidth(), aBounds.GetHeight() );
		GetWorkArea( aWorkArea );
		if ( aBounds.Intersection( aWorkArea ).IsEmpty() )
			maFrameData.mpParent->GetWorkArea( aWorkArea );
	}
	else
	{
		GetWorkArea( aWorkArea );
	}

	// Make sure window does not spill off of the screen
	long nMinX = aWorkArea.nLeft;
	long nMinY = aWorkArea.nTop;
	if ( maFrameData.mbPresentation )
	{
		nMinX -= 1;
		nMinY -= 1;
	}
	nWidth = nWidth + maGeometry.nLeftDecoration + maGeometry.nRightDecoration;
	nHeight = nHeight + maGeometry.nTopDecoration + maGeometry.nBottomDecoration;
	if ( nMinX + nWidth > aWorkArea.nLeft + aWorkArea.GetWidth() )
		nWidth = aWorkArea.nLeft + aWorkArea.GetWidth() - nMinX;
	if ( nMinY + nHeight > aWorkArea.nTop + aWorkArea.GetHeight() )
		nHeight = aWorkArea.nTop + aWorkArea.GetHeight() - nMinY;
	if ( nX < nMinX )
		nX = nMinX;
	if ( nY < nMinY )
		nY = nMinY;
	if ( nX + nWidth > aWorkArea.nLeft + aWorkArea.GetWidth() )
		nX = aWorkArea.nLeft + aWorkArea.GetWidth() - nWidth;
	if ( nY + nHeight > aWorkArea.nTop + aWorkArea.GetHeight() )
		nY = aWorkArea.nTop + aWorkArea.GetHeight() - nHeight;

	maFrameData.mpVCLFrame->setBounds( nX, nY, nWidth, nHeight );

	// Update the cached position
	Rectangle *pBounds = new Rectangle( maFrameData.mpVCLFrame->getBounds() );
	com_sun_star_vcl_VCLEvent aEvent( SALEVENT_MOVERESIZE, this, (void *)pBounds );
	aEvent.dispatch();
}

// -----------------------------------------------------------------------

void SalFrame::GetWorkArea( Rectangle &rRect )
{
	rRect = com_sun_star_vcl_VCLScreen::getScreenBounds( maFrameData.mpVCLFrame );

	// Adjust for system menu bar when not in presentation mode
	if ( !maFrameData.mbPresentation && !rRect.nTop )
	{
		const Rectangle& rFrameInsets( maFrameData.mpVCLFrame->getInsets() );
		rRect.nTop += rFrameInsets.nTop;
	}
}

// -----------------------------------------------------------------------

void SalFrame::GetClientSize( long& rWidth, long& rHeight )
{
	rWidth = maGeometry.nWidth;
	rHeight = maGeometry.nHeight;
}

// -----------------------------------------------------------------------

void SalFrame::SetWindowState( const SalFrameState* pState )
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
		Rectangle aPosSize( Point( pState->mnX, pState->mnY ), Size( pState->mnWidth, pState->mnHeight ) );
		if ( maFrameData.mpParent )
			aPosSize.Move( -maFrameData.mpParent->maGeometry.nX, -maFrameData.mpParent->maGeometry.nY );
		SetPosSize( aPosSize.nLeft, aPosSize.nTop, aPosSize.GetWidth(), aPosSize.GetHeight(), nFlags );
	}

	if ( pState->mnMask & SAL_FRAMESTATE_MASK_STATE )
	{
		if ( pState->mnState & SAL_FRAMESTATE_MINIMIZED )
			maFrameData.mpVCLFrame->setState( SAL_FRAMESTATE_MINIMIZED );
		else
			maFrameData.mpVCLFrame->setState( SAL_FRAMESTATE_NORMAL );
	}
}

// -----------------------------------------------------------------------

BOOL SalFrame::GetWindowState( SalFrameState* pState )
{
	pState->mnMask = SAL_FRAME_POSSIZE_X | SAL_FRAME_POSSIZE_Y | SAL_FRAME_POSSIZE_WIDTH | SAL_FRAME_POSSIZE_HEIGHT | SAL_FRAMESTATE_MASK_STATE;
	pState->mnX = maGeometry.nX - maGeometry.nLeftDecoration;
	pState->mnY = maGeometry.nY - maGeometry.nTopDecoration;
	pState->mnWidth = maGeometry.nWidth;
	pState->mnHeight = maGeometry.nHeight;
	pState->mnState = maFrameData.mpVCLFrame->getState();

	return TRUE;
}

// -----------------------------------------------------------------------

void SalFrame::ShowFullScreen( BOOL bFullScreen )
{
	if ( bFullScreen == maFrameData.mbFullScreen )
		return;

	USHORT nFlags = SAL_FRAME_POSSIZE_X | SAL_FRAME_POSSIZE_Y | SAL_FRAME_POSSIZE_WIDTH | SAL_FRAME_POSSIZE_HEIGHT;

	if ( bFullScreen )
	{
		SalData *pSalData = GetSalData();
		memcpy( &maFrameData.maOriginalGeometry, &maGeometry, sizeof( SalFrameGeometry ) );
		Rectangle aWorkArea;
		// If a window does not have a parent, who knows which screen the full
		// screen window will appear on so we place it on the same screen as
		// the focus window
		if ( maFrameData.mpParent )
			maFrameData.mpParent->GetWorkArea( aWorkArea );
		else if ( pSalData->mpFocusFrame )
			pSalData->mpFocusFrame->GetWorkArea( aWorkArea );
		else
			GetWorkArea( aWorkArea );
		SetPosSize( aWorkArea.nLeft, aWorkArea.nTop, aWorkArea.GetWidth() - maGeometry.nLeftDecoration - maGeometry.nRightDecoration, aWorkArea.GetHeight() - maGeometry.nTopDecoration - maGeometry.nBottomDecoration, nFlags );
	}
	else
	{
		SetPosSize( maFrameData.maOriginalGeometry.nX - maFrameData.maOriginalGeometry.nLeftDecoration, maFrameData.maOriginalGeometry.nY - maFrameData.maOriginalGeometry.nTopDecoration, maFrameData.maOriginalGeometry.nWidth, maFrameData.maOriginalGeometry.nHeight, nFlags );
		memset( &maFrameData.maOriginalGeometry, 0, sizeof( SalFrameGeometry ) );
	}

	maFrameData.mpVCLFrame->setFullScreenMode( bFullScreen );
	maFrameData.mbFullScreen = bFullScreen;
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

void SalFrame::StartPresentation( BOOL bStart )
{
	if ( bStart == maFrameData.mbPresentation )
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
			InstallEventLoopTimer( GetMainEventLoop(), 0, 0, pSetSystemUIModeTimerUPP, (void *)( bStart ? true : false ), NULL );
		else
			SetSystemUIModeTimerCallback( NULL, (void *)( bStart ? true : false ) );
	}

	maFrameData.mbPresentation = bStart;
	pSalData->mpPresentationFrame = this;

	// Adjust window size if in full screen mode
	if ( maFrameData.mbFullScreen )
	{
		USHORT nFlags = SAL_FRAME_POSSIZE_X | SAL_FRAME_POSSIZE_Y | SAL_FRAME_POSSIZE_WIDTH | SAL_FRAME_POSSIZE_HEIGHT;

		Rectangle aWorkArea;
		GetWorkArea( aWorkArea );

		SetPosSize( aWorkArea.nLeft, aWorkArea.nTop, aWorkArea.GetWidth() - maGeometry.nLeftDecoration - maGeometry.nRightDecoration, aWorkArea.GetHeight() - maGeometry.nTopDecoration - maGeometry.nBottomDecoration, nFlags );
	}
}

// -----------------------------------------------------------------------

void SalFrame::SetAlwaysOnTop( BOOL bOnTop )
{
	SalData *pSalData = GetSalData();
	if ( bOnTop )
		pSalData->maAlwaysOnTopFrameList.push_back( this );
	else
		pSalData->maAlwaysOnTopFrameList.remove( this );
}

// -----------------------------------------------------------------------

void SalFrame::ToTop( USHORT nFlags )
{
	if ( maFrameData.mpVCLFrame->getState() == SAL_FRAMESTATE_MINIMIZED && ! ( nFlags & SAL_FRAME_TOTOP_RESTOREWHENMIN ) )
		return;

	maFrameData.mpVCLFrame->toFront();
}

// -----------------------------------------------------------------------

void SalFrame::SetPointer( PointerStyle ePointerStyle )
{
	maFrameData.mpVCLFrame->setPointer( ePointerStyle );
}

// -----------------------------------------------------------------------

void SalFrame::CaptureMouse( BOOL bCapture )
{
#ifdef DEBUG
	fprintf( stderr, "SalFrame::CaptureMouse not implemented\n" );
#endif
}

// -----------------------------------------------------------------------

void SalFrame::SetPointerPos( long nX, long nY )
{
#ifdef DEBUG
	fprintf( stderr, "SalFrame::SetPointerPos not implemented\n" );
#endif
}

// -----------------------------------------------------------------------

void SalFrame::Flush()
{
	maFrameData.mpVCLFrame->flush();
}

// -----------------------------------------------------------------------

void SalFrame::Sync()
{
	maFrameData.mpVCLFrame->flush();
}

// -----------------------------------------------------------------------

void SalFrame::SetInputContext( SalInputContext* pContext )
{
#ifdef DEBUG
	fprintf( stderr, "SalFrame::SetInputContext not implemented\n" );
#endif
}

// -----------------------------------------------------------------------

void SalFrame::EndExtTextInput( USHORT nFlags )
{
	maFrameData.mpVCLFrame->endComposition();
}

// -----------------------------------------------------------------------

XubString SalFrame::GetKeyName( USHORT nKeyCode )
{
	return maFrameData.mpVCLFrame->getKeyName( nKeyCode );
}

// -----------------------------------------------------------------------

XubString SalFrame::GetSymbolKeyName( const XubString&, USHORT nKeyCode )
{
	return GetKeyName( nKeyCode );
}

// -----------------------------------------------------------------------

void SalFrame::UpdateSettings( AllSettings& rSettings )
{
	MouseSettings aMouseSettings = rSettings.GetMouseSettings();
	ULONG nDblTime = (ULONG)GetDblTime();
	if ( nDblTime < 25 )
		nDblTime = 25;
	aMouseSettings.SetDoubleClickTime( nDblTime * 1000 / CLK_TCK );
	aMouseSettings.SetStartDragWidth( 1 );
	aMouseSettings.SetStartDragHeight( 1 );
	rSettings.SetMouseSettings( aMouseSettings );

	StyleSettings aStyleSettings( rSettings.GetStyleSettings() );

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

SalBitmap* SalFrame::SnapShot()
{
#ifdef DEBUG
	fprintf( stderr, "SalFrame::Snapshot not implemented\n" );
#endif
	return NULL;
}

// -----------------------------------------------------------------------

const SystemEnvData* SalFrame::GetSystemData() const
{
	return &maFrameData.maSysData;
}

// -----------------------------------------------------------------------

void SalFrame::Beep( SoundType eSoundType )
{
	com_sun_star_vcl_VCLGraphics::beep();
}

// -----------------------------------------------------------------------

void SalFrame::SetCallback( void* pInst, SALFRAMEPROC pProc )
{
	maFrameData.mpInst = pInst;
	if ( pProc )
		maFrameData.mpProc = pProc;
	else
		maFrameData.mpProc = ImplSalCallbackDummy;
}

// -----------------------------------------------------------------------

SalFrame* SalFrame::GetParent() const
{
	return maFrameData.mpParent;
}

// -----------------------------------------------------------------------

LanguageType SalFrame::GetInputLanguage()
{
#ifdef DEBUG
	fprintf( stderr, "SalFrame::GetInputLanguage not implemented\n" );
#endif
	return LANGUAGE_DONTKNOW;
}

// -----------------------------------------------------------------------

ULONG SalFrame::GetCurrentModButtons()
{
	return maFrameData.mpVCLFrame->getCurrentModButtons();
}

// -----------------------------------------------------------------------

void SalFrame::SetParent( SalFrame* pNewParent )
{
	if ( maFrameData.mpParent )
	{
		maFrameData.mpParent->maFrameData.maChildren.remove( this );
		maFrameData.mpParent->maFrameData.mpVCLFrame->removeChild( this );
	}
	maFrameData.mpParent = pNewParent;
	if ( maFrameData.mpParent )
	{
		maFrameData.mpParent->maFrameData.maChildren.push_back( this );
		maFrameData.mpParent->maFrameData.mpVCLFrame->addChild( this );
	}
}

// -----------------------------------------------------------------------

bool SalFrame::SetPluginParent( SystemParentData* pNewParent )
{
#ifdef DEBUG
	fprintf( stderr, "SalFrame::SetPluginParent not implemented\n" );
#endif
	return false;
}

// -----------------------------------------------------------------------

void SalFrame::SetMenu( SalMenu* pSalMenu )
{
}

// -----------------------------------------------------------------------

void SalFrame::DrawMenuBar()
{
}

// =======================================================================

SalFrameData::SalFrameData()
{
	mpVCLFrame = NULL;
	mpGraphics = new SalGraphicsLayout();
	mnStyle = 0;
	mpParent = NULL;
	mbGraphics = FALSE;
	mbVisible = FALSE;
	mpInst = NULL;
	mpProc = ImplSalCallbackDummy;
	memset( &maSysData, 0, sizeof( SystemEnvData ) );
	maSysData.nSize = sizeof( SystemEnvData );
	mbCenter = TRUE;
	memset( &maOriginalGeometry, 0, sizeof( maOriginalGeometry ) );
	mbFullScreen = FALSE;
	mbPresentation = FALSE;
	mpMenuBar = NULL;
}

// -----------------------------------------------------------------------

SalFrameData::~SalFrameData()
{
	if ( mpVCLFrame )
	{
		mpVCLFrame->dispose();
		delete mpVCLFrame;
	}

	delete mpGraphics;
}
