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

#ifdef MACOSX

#ifndef _VOS_MODULE_HXX_
#include <vos/module.hxx>
#endif

#include <premac.h>
#include <Carbon/Carbon.h>
#include <postmac.h>
typedef OSStatus SetSystemUIMode_Type( SystemUIMode, SystemUIOptions );

using namespace rtl;
using namespace vos;

#endif	// MACOSX

using namespace vcl;

// =======================================================================

long ImplSalCallbackDummy( void*, SalFrame*, USHORT, const void* )
{
	return 0;
}

// -----------------------------------------------------------------------

SalFrame::SalFrame()
{
	maFrameData.mpGraphics->maGraphicsData.mpFrame = this;
}

// -----------------------------------------------------------------------

SalFrame::~SalFrame()
{
	SalData *pSalData = GetSalData();

	if ( pSalData->mpPresentationFrame == this )
		pSalData->mpPresentationFrame = NULL;
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

void SalFrame::Show( BOOL bVisible )
{
	maFrameData.mpVCLFrame->setVisible( bVisible );

	if ( bVisible )
	{
		// Update the cached position
		Rectangle *pBounds = new Rectangle( maFrameData.mpVCLFrame->getBounds() );
		com_sun_star_vcl_VCLEvent aEvent( SALEVENT_MOVERESIZE, this, (void *)pBounds );
		aEvent.dispatch();

		// Post a paint event
		SalPaintEvent *pPaintEvent = new SalPaintEvent();
		pPaintEvent->mnBoundX = 0;
		pPaintEvent->mnBoundY = 0;
		pPaintEvent->mnBoundWidth = maGeometry.nWidth + maGeometry.nLeftDecoration;
		pPaintEvent->mnBoundHeight = maGeometry.nHeight + maGeometry.nTopDecoration;
		com_sun_star_vcl_VCLEvent aVCLPaintEvent( SALEVENT_PAINT, this, (void *)pPaintEvent );
		aVCLPaintEvent.dispatch();
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
#ifdef DEBUG
	fprintf( stderr, "SalFrame::SetMinClientSize not implemented\n" );
#endif
}

// -----------------------------------------------------------------------

void SalFrame::SetPosSize( long nX, long nY, long nWidth, long nHeight,
							USHORT nFlags )
{
	if ( maFrameData.mnStyle & SAL_FRAME_STYLE_CHILD )
		return;

	Rectangle aPosSize( Point( maGeometry.nX, maGeometry.nY ), Size( maGeometry.nWidth, maGeometry.nHeight ) );
	aPosSize.Justify();

	// If there is a parent frame, add back its X and Y coordinates as the
	// new X and Y coordinates are relative to the parent frame
	if ( maFrameData.mpParent )
		nX += maFrameData.mpParent->maGeometry.nX;
	if ( maFrameData.mpParent )
		nY += maFrameData.mpParent->maGeometry.nY;

	if ( ! ( nFlags & SAL_FRAME_POSSIZE_X ) )
		nX = aPosSize.nLeft;
	if ( ! ( nFlags & SAL_FRAME_POSSIZE_Y ) )
		nY = aPosSize.nTop;
	if ( ! ( nFlags & SAL_FRAME_POSSIZE_WIDTH ) )
		nWidth = aPosSize.GetWidth();
	if ( ! ( nFlags & SAL_FRAME_POSSIZE_HEIGHT ) )
		nHeight = aPosSize.GetHeight();

	if ( maFrameData.mbCenter && ! ( nFlags & SAL_FRAME_POSSIZE_X ) && ! ( nFlags & SAL_FRAME_POSSIZE_Y ) && !maFrameData.mbVisible )
	{
		Rectangle aWorkArea;
		GetWorkArea( aWorkArea );
		nX = ( aWorkArea.GetWidth() - nWidth ) / 2;
		nY = ( aWorkArea.GetHeight() - nHeight ) / 2;
	}

	maFrameData.mpVCLFrame->setBounds( nX, nY, nWidth + maGeometry.nLeftDecoration + maGeometry.nRightDecoration, nHeight + maGeometry.nTopDecoration + maGeometry.nBottomDecoration );

	// Update the cached position
	Rectangle *pBounds = new Rectangle( maFrameData.mpVCLFrame->getBounds() );
	com_sun_star_vcl_VCLEvent aEvent( SALEVENT_MOVERESIZE, this, (void *)pBounds );
	aEvent.dispatch();
}

// -----------------------------------------------------------------------

void SalFrame::GetWorkArea( Rectangle &rRect )
{
	const Size& rScreenSize( com_sun_star_vcl_VCLScreen::getScreenSize() );
	rRect.nLeft = 0;
	rRect.nTop = 0;
	rRect.nRight = rScreenSize.Width() - 1;
	rRect.nBottom = rScreenSize.Height() - 1;

#ifdef MACOSX
	// Adjust for system menu bar when in presentation mode
	if ( maFrameData.mbPresentation )
	{
		const Rectangle& rFrameInsets( com_sun_star_vcl_VCLScreen::getFrameInsets() );
		rRect.nBottom += rFrameInsets.nTop;
	}
#endif	// MACOSX
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
#ifdef DEBUG
	fprintf( stderr, "SalFrame::SetWindowState not implemented\n" );
#endif
}

// -----------------------------------------------------------------------

BOOL SalFrame::GetWindowState( SalFrameState* pState )
{
#ifdef DEBUG
	fprintf( stderr, "SalFrame::GetWindowState not implemented\n" );
#endif
	return FALSE;
}

// -----------------------------------------------------------------------

void SalFrame::ShowFullScreen( BOOL bFullScreen )
{
	if ( bFullScreen == maFrameData.mbFullScreen )
		return;

	USHORT nFlags = SAL_FRAME_POSSIZE_X | SAL_FRAME_POSSIZE_Y | SAL_FRAME_POSSIZE_WIDTH | SAL_FRAME_POSSIZE_HEIGHT;

	if ( bFullScreen )
	{
		memcpy( &maFrameData.maOriginalGeometry, &maGeometry, sizeof( SalFrameGeometry ) );
		Rectangle aWorkArea;
		GetWorkArea( aWorkArea );
		SetPosSize( aWorkArea.nLeft + maGeometry.nLeftDecoration, aWorkArea.nTop + maGeometry.nTopDecoration, aWorkArea.GetWidth() - maGeometry.nLeftDecoration - maGeometry.nRightDecoration, aWorkArea.GetHeight() - maGeometry.nTopDecoration - maGeometry.nBottomDecoration, nFlags );
	}
	else
	{
		SetPosSize( maFrameData.maOriginalGeometry.nX, maFrameData.maOriginalGeometry.nY, maFrameData.maOriginalGeometry.nWidth, maFrameData.maOriginalGeometry.nHeight, nFlags );
		memset( &maFrameData.maOriginalGeometry, 0, sizeof( SalFrameGeometry ) );
	}

	maFrameData.mpVCLFrame->setFullScreenMode( bFullScreen );
	maFrameData.mbFullScreen = bFullScreen;
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

#ifdef MACOSX
	OModule aModule;
	if ( aModule.load( OUString::createFromAscii( "/System/Library/Frameworks/Carbon.framework/Carbon" ) ) )
	{
		SetSystemUIMode_Type *pSetSystemUIMode = (SetSystemUIMode_Type *)aModule.getSymbol( OUString::createFromAscii( "SetSystemUIMode" ) );

		if ( pSetSystemUIMode )
		{
			if ( bStart )
				pSetSystemUIMode( kUIModeAllHidden, kUIOptionDisableAppleMenu | kUIOptionDisableProcessSwitch );
			else
				pSetSystemUIMode( kUIModeNormal, 0 );

		}
		aModule.unload();
	}

	maFrameData.mbPresentation = bStart;
	pSalData->mpPresentationFrame = this;

	// Adjust window size if in full screen mode
	if ( maFrameData.mbFullScreen )
	{
		USHORT nFlags = SAL_FRAME_POSSIZE_X | SAL_FRAME_POSSIZE_Y | SAL_FRAME_POSSIZE_WIDTH | SAL_FRAME_POSSIZE_HEIGHT;

		Rectangle aWorkArea;
		GetWorkArea( aWorkArea );

		// The system menu bar is a dead zone of coordinates so we need to set
		// the top of the window at -1 and make the cached insets larger to
		// make the window appear at the top of the screen
		unsigned int nRealTopDecoration = 0;
		if ( bStart )
		{
			aWorkArea.nTop -= 1;
			aWorkArea.nBottom += 1;
			nRealTopDecoration = maGeometry.nTopDecoration;
			maGeometry.nTopDecoration += 1;
		}
		else
		{
			maGeometry.nTopDecoration -= 1;
			nRealTopDecoration = maGeometry.nTopDecoration;
		}
		SetPosSize( aWorkArea.nLeft + maGeometry.nLeftDecoration, aWorkArea.nTop + nRealTopDecoration, aWorkArea.GetWidth() - maGeometry.nLeftDecoration - maGeometry.nRightDecoration, aWorkArea.GetHeight() - nRealTopDecoration - maGeometry.nBottomDecoration, nFlags );
	}
#else	// MACOSX
#ifdef DEBUG
	fprintf( stderr, "SalFrame::StartPresentation not implemented\n" );
#endif
#endif	// MACOSX
}

// -----------------------------------------------------------------------

void SalFrame::SetAlwaysOnTop( BOOL bOnTop )
{
#ifdef DEBUG
	fprintf( stderr, "SalFrame::SetAlwaysOnTop not implemented\n" );
#endif
}

// -----------------------------------------------------------------------

void SalFrame::ToTop( USHORT nFlags )
{
	maFrameData.mpVCLFrame->toFront();
	for( ::std::list< SalFrame* >::const_iterator it = maFrameData.maChildren.begin(); it != maFrameData.maChildren.end(); ++it )
		(*it)->ToTop( nFlags );
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
#ifdef DEBUG
	fprintf( stderr, "SalFrame::EndExtTextInput not implemented\n" );
#endif
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
#ifdef DEBUG
	fprintf( stderr, "SalFrame::UpdateSettings not implemented\n" );
#endif
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

// =======================================================================

SalFrameData::SalFrameData()
{
	mpVCLFrame = NULL;
	mpGraphics = new SalGraphics();
	mpNextFrame = NULL;
	mnStyle = 0;
	mpParent = NULL;
	mbGraphics = FALSE;
	mbVisible = FALSE;
	mpInst = NULL;
	mpProc = ImplSalCallbackDummy;
	maSysData.nSize = sizeof( SystemEnvData );
	maSysData.pVCLFrame = NULL;
	mbCenter = TRUE;
	memset( &maOriginalGeometry, 0, sizeof( maOriginalGeometry ) );
	mbFullScreen = FALSE;
	mbPresentation = FALSE;
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
