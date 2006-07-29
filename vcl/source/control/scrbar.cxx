/*************************************************************************
 *
 *  $RCSfile$
 *
 *  $Revision$
 *
 *  last change: $Author$ $Date$
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU General Public License Version 2.1.
 *
 *
 *    GNU General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 *    Modified May 2006 by Edward Peterlin. NeoOffice is distributed under
 *    GPL only under modification term 3 of the LGPL.
 *
 ************************************************************************/

#ifndef _SV_EVENT_HXX
#include <event.hxx>
#endif
#ifndef _SV_SOUND_HXX
#include <sound.hxx>
#endif
#ifndef _SV_DECOVIEW_HXX
#include <decoview.hxx>
#endif
#ifndef _SV_SCRBAR_HXX
#include <scrbar.hxx>
#endif
#ifndef _SV_TIMER_HXX
#include <timer.hxx>
#endif

#ifndef _RTL_STRING_HXX_
#include <rtl/string.hxx>
#endif

#ifndef _SV_RC_H
#include <tools/rc.h>
#endif



#ifdef USE_JAVA

#include <list>

#ifndef _SV_SVAPP_HXX
#include <svapp.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif

#include <premac.h>
#include <Carbon/Carbon.h>
#include <postmac.h>

static ::std::list< ScrollBar* > gScrollBars;
static EventHandlerUPP pRelayoutScrollBarHandler = NULL;

using namespace vos;

#endif	// USE_JAVA

using namespace rtl;

// =======================================================================

#ifdef USE_JAVA

static OSStatus RelayoutScrollBars(  EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void * inUserData )
{
	// We need to let any pending timers run so that we don't deadlock
	TimeValue aDelay;
	aDelay.Seconds = 0;
	aDelay.Nanosec = 10;
	IMutex& rSolarMutex = Application::GetSolarMutex();
	bool bAcquired = false;
	while ( !Application::IsShutDown() )
	{
		if ( rSolarMutex.tryToAcquire() )
		{
			if ( !Application::IsShutDown() )
				bAcquired = true; 
			else
				rSolarMutex.release();
			break;
		}
		ReceiveNextEvent( 0, NULL, 0, false, NULL );
		OThread::wait( aDelay );
	}

	if ( bAcquired )
	{
		for ( ::std::list< ScrollBar* >::const_iterator iter = gScrollBars.begin(); iter != gScrollBars.end(); iter++ )
		{
			(*iter)->Resize();
			(*iter)->Invalidate();
		}

		rSolarMutex.release();
	}

	return noErr;
}

// =======================================================================

static void BeginTrackingScrollBar( ScrollBar *toTrack )
{
	if ( !pRelayoutScrollBarHandler )
	{
		pRelayoutScrollBarHandler = NewEventHandlerUPP( RelayoutScrollBars );
		if ( pRelayoutScrollBarHandler )
		{
			EventHandlerRef myRef;
			EventTypeSpec appearanceScrollEvent = { kEventClassAppearance, kEventAppearanceScrollBarVariantChanged };
			InstallEventHandler( GetApplicationEventTarget(), pRelayoutScrollBarHandler, 1, &appearanceScrollEvent, NULL, &myRef );
		}
	}

	gScrollBars.push_back( toTrack );
}

// =======================================================================

static void EndTrackingScrollBar( ScrollBar *toTrack )
{
	gScrollBars.remove( toTrack );
}

// =======================================================================

#endif	// USE_JAVA
static long ImplMulDiv( long nNumber, long nNumerator, long nDenominator )
{
    double n = ((double)nNumber * (double)nNumerator) / (double)nDenominator;
    return (long)n;
}

// =======================================================================

#define SCRBAR_DRAW_BTN1            ((USHORT)0x0001)
#define SCRBAR_DRAW_BTN2            ((USHORT)0x0002)
#define SCRBAR_DRAW_PAGE1           ((USHORT)0x0004)
#define SCRBAR_DRAW_PAGE2           ((USHORT)0x0008)
#define SCRBAR_DRAW_THUMB           ((USHORT)0x0010)
#define SCRBAR_DRAW_BACKGROUND      ((USHORT)0x0020)
#define SCRBAR_DRAW_ALL             (SCRBAR_DRAW_BTN1 | SCRBAR_DRAW_BTN2 |  \
                                     SCRBAR_DRAW_PAGE1 | SCRBAR_DRAW_PAGE2 |\
                                     SCRBAR_DRAW_THUMB | SCRBAR_DRAW_BACKGROUND )

#define SCRBAR_STATE_BTN1_DOWN      ((USHORT)0x0001)
#define SCRBAR_STATE_BTN1_DISABLE   ((USHORT)0x0002)
#define SCRBAR_STATE_BTN2_DOWN      ((USHORT)0x0004)
#define SCRBAR_STATE_BTN2_DISABLE   ((USHORT)0x0008)
#define SCRBAR_STATE_PAGE1_DOWN     ((USHORT)0x0010)
#define SCRBAR_STATE_PAGE2_DOWN     ((USHORT)0x0020)
#define SCRBAR_STATE_THUMB_DOWN     ((USHORT)0x0040)

#define SCRBAR_VIEW_STYLE           (WB_3DLOOK | WB_HORZ | WB_VERT)

struct ImplScrollBarData
{
	AutoTimer		maTimer;			// Timer
    BOOL            mbHide;
#ifdef USE_JAVA
    BOOL            mbHasEntireControlRect;
    Rectangle       maEntireControlRect;
#endif	// USE_JAVA
};

// =======================================================================

void ScrollBar::ImplInit( Window* pParent, WinBits nStyle )
{
    mpData              = NULL;
    mnThumbPixRange     = 0;
    mnThumbPixPos       = 0;
    mnThumbPixSize      = 0;
    mnMinRange          = 0;
    mnMaxRange          = 100;
    mnThumbPos          = 0;
    mnVisibleSize       = 0;
    mnLineSize          = 1;
    mnPageSize          = 1;
    mnDelta             = 0;
    mnDragDraw          = 0;
    mnStateFlags        = 0;
    meScrollType        = SCROLL_DONTKNOW;
    meDDScrollType      = SCROLL_DONTKNOW;
    mbCalcSize          = TRUE;
    mbFullDrag          = 0;

    ImplInitStyle( nStyle );
    Control::ImplInit( pParent, nStyle, NULL );

    long nScrollSize = GetSettings().GetStyleSettings().GetScrollBarSize();
    SetSizePixel( Size( nScrollSize, nScrollSize ) );
    SetBackground();
#ifdef USE_JAVA
	BeginTrackingScrollBar( this );
#endif	// USE_JAVA
}

// -----------------------------------------------------------------------

void ScrollBar::ImplInitStyle( WinBits nStyle )
{
    if ( nStyle & WB_DRAG )
        mbFullDrag = TRUE;
    else
        mbFullDrag = (GetSettings().GetStyleSettings().GetDragFullOptions() & DRAGFULL_OPTION_SCROLL) != 0;
}

// -----------------------------------------------------------------------

ScrollBar::ScrollBar( Window* pParent, WinBits nStyle ) :
    Control( WINDOW_SCROLLBAR )
{
    ImplInit( pParent, nStyle );
}

// -----------------------------------------------------------------------

ScrollBar::ScrollBar( Window* pParent, const ResId& rResId ) :
    Control( WINDOW_SCROLLBAR )
{
    rResId.SetRT( RSC_SCROLLBAR );
    WinBits nStyle = ImplInitRes( rResId );
    ImplInit( pParent, nStyle );
    ImplLoadRes( rResId );

    if ( !(nStyle & WB_HIDE) )
        Show();
}

// -----------------------------------------------------------------------

ScrollBar::~ScrollBar()
{
    if( mpData )
        delete mpData;
#ifdef USE_JAVA
	EndTrackingScrollBar( this );
#endif	// USE_JAVA
}

// -----------------------------------------------------------------------

void ScrollBar::ImplLoadRes( const ResId& rResId )
{
    Control::ImplLoadRes( rResId );

    INT16 nMin          = ReadShortRes();
    INT16 nMax          = ReadShortRes();
    INT16 nThumbPos     = ReadShortRes();
    INT16 nPage         = ReadShortRes();
    INT16 nStep         = ReadShortRes();
    INT16 nVisibleSize  = ReadShortRes();

    SetRange( Range( nMin, nMax ) );
    SetLineSize( nStep );
    SetPageSize( nPage );
    SetVisibleSize( nVisibleSize );
    SetThumbPos( nThumbPos );
}

// -----------------------------------------------------------------------

BOOL ScrollBar::ImplUpdateThumbRect( const Rectangle& rOldRect )
{
/* !!! Wegen ueberlappenden Fenstern ... !!!
    Size aThumbRectSize  = rOldRect.GetSize();
    if ( aThumbRectSize == maThumbRect.GetSize() )
    {
        DrawOutDev( maThumbRect.TopLeft(), aThumbRectSize,
                    rOldRect.TopLeft(), aThumbRectSize );
        return TRUE;
    }
    else
*/
        return FALSE;
}

// -----------------------------------------------------------------------

void ScrollBar::ImplUpdateRects( BOOL bUpdate )
{
#ifdef USE_JAVA
	if( IsNativeControlSupported( CTRL_SCROLLBAR, PART_ENTIRE_CONTROL ) )
	{
		ImplUpdateRectsNative( bUpdate );
		return;
	}
#endif
    USHORT      nOldStateFlags  = mnStateFlags;
    Rectangle   aOldPage1Rect = maPage1Rect;
    Rectangle   aOldPage2Rect = maPage2Rect;
    Rectangle   aOldThumbRect = maThumbRect;

    mnStateFlags  &= ~SCRBAR_STATE_BTN1_DISABLE;
    mnStateFlags  &= ~SCRBAR_STATE_BTN2_DISABLE;

    if ( mnThumbPixRange )
    {
        if ( GetStyle() & WB_HORZ )
        {
            maThumbRect.Left()      = maBtn1Rect.Right()+1+mnThumbPixPos;
            maThumbRect.Right()     = maThumbRect.Left()+mnThumbPixSize-1;
            if ( !mnThumbPixPos )
                maPage1Rect.Right()     = RECT_EMPTY;
            else
                maPage1Rect.Right()     = maThumbRect.Left()-1;
            if ( mnThumbPixPos >= (mnThumbPixRange-mnThumbPixSize) )
                maPage2Rect.Right()     = RECT_EMPTY;
            else
            {
                maPage2Rect.Left()      = maThumbRect.Right()+1;
                maPage2Rect.Right()     = maBtn2Rect.Left()-1;
            }
        }
        else
        {
            maThumbRect.Top()       = maBtn1Rect.Bottom()+1+mnThumbPixPos;
            maThumbRect.Bottom()    = maThumbRect.Top()+mnThumbPixSize-1;
            if ( !mnThumbPixPos )
                maPage1Rect.Bottom()    = RECT_EMPTY;
            else
                maPage1Rect.Bottom()    = maThumbRect.Top()-1;
            if ( mnThumbPixPos >= (mnThumbPixRange-mnThumbPixSize) )
                maPage2Rect.Bottom()    = RECT_EMPTY;
            else
            {
                maPage2Rect.Top()       = maThumbRect.Bottom()+1;
                maPage2Rect.Bottom()    = maBtn2Rect.Top()-1;
            }
        }
    }
    else
    {
        Size aScrBarSize = GetOutputSizePixel();
        if ( GetStyle() & WB_HORZ )
        {
            if ( aScrBarSize.Width() > maBtn1Rect.getWidth() + maBtn2Rect.getWidth() )
            {
                long nSpace = aScrBarSize.Width() - maBtn1Rect.getWidth() - maBtn2Rect.getWidth();
                maPage1Rect.Left()   = maBtn1Rect.Right()+1;
                maPage2Rect.Right()  = maBtn1Rect.Right()+(nSpace/2);
                maPage2Rect.Left()   = maBtn1Rect.Right()+1+(nSpace/2);
                maPage2Rect.Right()  = maBtn2Rect.Left()-1;
            }
        }
        else
        {
            if ( aScrBarSize.Height() > maBtn1Rect.getHeight() + maBtn2Rect.getHeight() )
            {
                long nSpace = aScrBarSize.Height() - maBtn1Rect.getHeight() - maBtn2Rect.getHeight();
                maPage1Rect.Top()    = maBtn1Rect.Bottom()+1;
                maPage1Rect.Bottom() = maBtn1Rect.Bottom()+(nSpace/2);
                maPage2Rect.Top()    = maBtn1Rect.Bottom()+1+(nSpace/2);
                maPage2Rect.Bottom() = maBtn2Rect.Top()-1;
            }
        }
    }

    if( !IsNativeControlSupported(CTRL_SCROLLBAR, PART_ENTIRE_CONTROL) )
    {
        // disable scrollbar buttons only in VCL's own 'theme'
        // as it is uncommon on other platforms
        if ( mnThumbPos == mnMinRange )
            mnStateFlags |= SCRBAR_STATE_BTN1_DISABLE;
        if ( mnThumbPos >= (mnMaxRange-mnVisibleSize) )
            mnStateFlags |= SCRBAR_STATE_BTN2_DISABLE;
    }

    if ( bUpdate )
    {
        USHORT nDraw = 0;
        if ( (nOldStateFlags & SCRBAR_STATE_BTN1_DISABLE) !=
             (mnStateFlags & SCRBAR_STATE_BTN1_DISABLE) )
            nDraw |= SCRBAR_DRAW_BTN1;
        if ( (nOldStateFlags & SCRBAR_STATE_BTN2_DISABLE) !=
             (mnStateFlags & SCRBAR_STATE_BTN2_DISABLE) )
            nDraw |= SCRBAR_DRAW_BTN2;
        if ( aOldPage1Rect != maPage1Rect )
            nDraw |= SCRBAR_DRAW_PAGE1;
        if ( aOldPage2Rect != maPage2Rect )
            nDraw |= SCRBAR_DRAW_PAGE2;
        if ( aOldThumbRect != maThumbRect )
        {
            if ( !ImplUpdateThumbRect( aOldThumbRect ) )
                nDraw |= SCRBAR_DRAW_THUMB;
        }
        ImplDraw( nDraw, this );
    }
}

// -----------------------------------------------------------------------

#ifdef USE_JAVA
void ScrollBar::ImplUpdateRectsNative( BOOL bUpdate )
{
	USHORT      nOldStateFlags  = mnStateFlags;
    Rectangle   aOldPage1Rect = maPage1Rect;
    Rectangle   aOldPage2Rect = maPage2Rect;
    Rectangle   aOldThumbRect = maThumbRect;
	
    mnStateFlags  &= ~SCRBAR_STATE_BTN1_DISABLE;
    mnStateFlags  &= ~SCRBAR_STATE_BTN2_DISABLE;
    
    Size aSize = GetOutputSizePixel();
    Point aPoint( 0, 0 );
    Region aControlRegion( Rectangle( aPoint, aSize ) );
        
    ImplControlValue aControlValue( BUTTONVALUE_DONTKNOW, rtl::OUString(), 0 );

	BOOL bHorz = (GetStyle() & WB_HORZ ? true : false);

	Region  		aCtrlRegion;
	ControlState		nState = ( IsEnabled() ? CTRL_STATE_ENABLED : 0 ) | ( HasFocus() ? CTRL_STATE_FOCUSED : 0 );
	ScrollbarValue	scrValue;

	scrValue.mnMin = mnMinRange;
	scrValue.mnMax = mnMaxRange;
	scrValue.mnCur = mnThumbPos;
	scrValue.mnVisibleSize = mnVisibleSize;
	scrValue.maThumbRect = maThumbRect;
	scrValue.maButton1Rect = maBtn1Rect;
	scrValue.maButton2Rect = maBtn2Rect;
	scrValue.mnButton1State = ((mnStateFlags & SCRBAR_STATE_BTN1_DOWN) ? CTRL_STATE_PRESSED : 0) |
						((!(mnStateFlags & SCRBAR_STATE_BTN1_DISABLE)) ? CTRL_STATE_ENABLED : 0);
	scrValue.mnButton2State = ((mnStateFlags & SCRBAR_STATE_BTN2_DOWN) ? CTRL_STATE_PRESSED : 0) |
						((!(mnStateFlags & SCRBAR_STATE_BTN2_DISABLE)) ? CTRL_STATE_ENABLED : 0);
	scrValue.mnThumbState = nState | ((mnStateFlags & SCRBAR_STATE_THUMB_DOWN) ? CTRL_STATE_PRESSED : 0);
	scrValue.mnPage1State = nState | ((mnStateFlags & SCRBAR_STATE_PAGE1_DOWN) ? CTRL_STATE_PRESSED : 0);
	scrValue.mnPage2State = nState | ((mnStateFlags & SCRBAR_STATE_PAGE2_DOWN) ? CTRL_STATE_PRESSED : 0);

	if( IsMouseOver() )
	{
		Rectangle* pRect = ImplFindPartRect( GetPointerPosPixel() );
		if( pRect )
		{
			if( pRect == &maThumbRect )
				scrValue.mnThumbState |= CTRL_STATE_ROLLOVER;
			else if( pRect == &maBtn1Rect )
				scrValue.mnButton1State |= CTRL_STATE_ROLLOVER;
			else if( pRect == &maBtn2Rect )
				scrValue.mnButton2State |= CTRL_STATE_ROLLOVER;
			else if( pRect == &maPage1Rect )
				scrValue.mnPage1State |= CTRL_STATE_ROLLOVER;
			else if( pRect == &maPage2Rect )
				scrValue.mnPage2State |= CTRL_STATE_ROLLOVER;
		}
	}

	aControlValue.setOptionalVal( (void *)(&scrValue) );
		
	Region aEntireCtrlRegion, aPage1Region, aPage2Region, aThumbRegion, aBoundingRegion;
	
	if( ! mpData )
		ImplNewImplScrollBarData();
	
	if( GetNativeControlRegion( CTRL_SCROLLBAR, PART_ENTIRE_CONTROL, aControlRegion, 0, aControlValue, rtl::OUString(), aBoundingRegion, aEntireCtrlRegion ) )
	{
		mpData->mbHasEntireControlRect = TRUE;
		mpData->maEntireControlRect = aEntireCtrlRegion.GetBoundRect();
	}
	
	GetNativeControlRegion( CTRL_SCROLLBAR, ( ( bHorz ) ? PART_TRACK_HORZ_LEFT : PART_TRACK_VERT_UPPER ), aControlRegion, 0, aControlValue, rtl::OUString(), aBoundingRegion, aPage1Region );
	maPage1Rect = aPage1Region.GetBoundRect();
	
	GetNativeControlRegion( CTRL_SCROLLBAR, ( ( bHorz ) ? PART_TRACK_HORZ_RIGHT : PART_TRACK_VERT_LOWER ), aControlRegion, 0, aControlValue, rtl::OUString(), aBoundingRegion, aPage2Region );
	maPage2Rect = aPage2Region.GetBoundRect();
	
	GetNativeControlRegion( CTRL_SCROLLBAR, ( ( bHorz ) ? PART_THUMB_HORZ : PART_THUMB_VERT ), aControlRegion, 0, aControlValue, rtl::OUString(), aBoundingRegion, aThumbRegion );
	maThumbRect = aThumbRegion.GetBoundRect();
	
	if ( bUpdate )
    {
        USHORT nDraw = 0;
        if ( (nOldStateFlags & SCRBAR_STATE_BTN1_DISABLE) !=
             (mnStateFlags & SCRBAR_STATE_BTN1_DISABLE) )
            nDraw |= SCRBAR_DRAW_BTN1;
        if ( (nOldStateFlags & SCRBAR_STATE_BTN2_DISABLE) !=
             (mnStateFlags & SCRBAR_STATE_BTN2_DISABLE) )
            nDraw |= SCRBAR_DRAW_BTN2;
        if ( aOldPage1Rect != maPage1Rect )
            nDraw |= SCRBAR_DRAW_PAGE1;
        if ( aOldPage2Rect != maPage2Rect )
            nDraw |= SCRBAR_DRAW_PAGE2;
        if ( aOldThumbRect != maThumbRect )
        {
            if ( !ImplUpdateThumbRect( aOldThumbRect ) )
                nDraw |= SCRBAR_DRAW_THUMB;
        }
        ImplDraw( nDraw, this );
    }
}

// -----------------------------------------------------------------------

#endif
long ScrollBar::ImplCalcThumbPos( long nPixPos )
{
    // Position berechnen
    long nCalcThumbPos;
    nCalcThumbPos = ImplMulDiv( nPixPos, mnMaxRange-mnVisibleSize-mnMinRange,
                                mnThumbPixRange-mnThumbPixSize );
    nCalcThumbPos += mnMinRange;
    return nCalcThumbPos;
}

// -----------------------------------------------------------------------

long ScrollBar::ImplCalcThumbPosPix( long nPos )
{
    long nCalcThumbPos;

    // Position berechnen
    nCalcThumbPos = ImplMulDiv( nPos-mnMinRange, mnThumbPixRange-mnThumbPixSize,
                                mnMaxRange-mnVisibleSize-mnMinRange );

    // Am Anfang und Ende des ScrollBars versuchen wir die Anzeige korrekt
    // anzuzeigen
    if ( !nCalcThumbPos && (mnThumbPos > mnMinRange) )
        nCalcThumbPos = 1;
    if ( nCalcThumbPos &&
         ((nCalcThumbPos+mnThumbPixSize) >= mnThumbPixRange) &&
         (mnThumbPos < (mnMaxRange-mnVisibleSize)) )
        nCalcThumbPos--;

    return nCalcThumbPos;
}

// -----------------------------------------------------------------------

void ScrollBar::ImplCalc( BOOL bUpdate )
{
    Size aSize = GetOutputSizePixel();
    long nMinThumbSize = GetSettings().GetStyleSettings().GetMinThumbSize();;

    if ( mbCalcSize )
    {
        Size aBtnSize;
        Point aPoint( 0, 0 );
        Region aControlRegion( Rectangle( aPoint, aSize ) );
        Region aBtn1Region, aBtn2Region, aBoundingRegion;

        if ( GetStyle() & WB_HORZ )
        {
            if ( GetNativeControlRegion( CTRL_SCROLLBAR, PART_BUTTON_LEFT,
                        aControlRegion, 0, ImplControlValue(), rtl::OUString(), aBoundingRegion, aBtn1Region ) &&
                 GetNativeControlRegion( CTRL_SCROLLBAR, PART_BUTTON_RIGHT,
                        aControlRegion, 0, ImplControlValue(), rtl::OUString(), aBoundingRegion, aBtn2Region ) )
            {
                maBtn1Rect = aBtn1Region.GetBoundRect();
                maBtn2Rect = aBtn2Region.GetBoundRect();
            }
            else
            {
                aBtnSize            = Size( aSize.Height(), aSize.Height() );
                maBtn2Rect.Top()    = maBtn1Rect.Top();
                maBtn2Rect.Left()   = aSize.Width()-aSize.Height();
                maBtn1Rect.SetSize( aBtnSize );
                maBtn2Rect.SetSize( aBtnSize );
            }
            // Check if available space is big enough for thumb ( min thumb size = ScrBar width/height )
            if ( aSize.Width() > maBtn1Rect.getWidth() + maBtn2Rect.getWidth() + nMinThumbSize )
                mnThumbPixRange = aSize.Width() - maBtn1Rect.getWidth() - maBtn2Rect.getWidth() - 1;
            else
                mnThumbPixRange = 0;

            if ( aSize.Width() > maBtn1Rect.getWidth() + maBtn2Rect.getWidth() )
            {
                maPage1Rect.Left()      = maBtn1Rect.Right()+1;
                maPage1Rect.Bottom()    = maBtn1Rect.Bottom();
                maPage2Rect.Bottom()    = maBtn1Rect.Bottom();
                maThumbRect.Bottom()    = maBtn1Rect.Bottom();
            }
            else
            {
                maPage1Rect.SetEmpty();
                maPage2Rect.SetEmpty();
            }
        }
        else
        {
            if ( GetNativeControlRegion( CTRL_SCROLLBAR, PART_BUTTON_UP,
                        aControlRegion, 0, ImplControlValue(), rtl::OUString(), aBoundingRegion, aBtn1Region ) &&
                 GetNativeControlRegion( CTRL_SCROLLBAR, PART_BUTTON_DOWN,
                        aControlRegion, 0, ImplControlValue(), rtl::OUString(), aBoundingRegion, aBtn2Region ) )
            {
                maBtn1Rect = aBtn1Region.GetBoundRect();
                maBtn2Rect = aBtn2Region.GetBoundRect();
            }
            else
            {
                aBtnSize            = Size( aSize.Width(), aSize.Width() );
                maBtn2Rect.Left()   = maBtn1Rect.Left();
                maBtn2Rect.Top()    = aSize.Height()-aSize.Width();
                maBtn1Rect.SetSize( aBtnSize );
                maBtn2Rect.SetSize( aBtnSize );
            }

            // Check if available space is big enough for thumb
            if ( aSize.Height() > maBtn1Rect.getHeight() + maBtn2Rect.getHeight() + nMinThumbSize )
                mnThumbPixRange = aSize.Height() - maBtn1Rect.getHeight() - maBtn2Rect.getHeight() - 1;
            else
                mnThumbPixRange = 0;

            if ( aSize.Height() > maBtn1Rect.getHeight() + maBtn2Rect.getHeight() )
            {
                maPage1Rect.Top()       = maBtn1Rect.Bottom()+1;
                maPage1Rect.Right()     = maBtn1Rect.Right();
                maPage2Rect.Right()     = maBtn1Rect.Right();
                maThumbRect.Right()     = maBtn1Rect.Right();
            }
            else
            {
                maPage1Rect.SetEmpty();
                maPage2Rect.SetEmpty();
            }
        }

        if ( !mnThumbPixRange )
            maThumbRect.SetEmpty();

        mbCalcSize = FALSE;
    }

    if ( mnThumbPixRange )
    {
        // Werte berechnen
        if ( (mnVisibleSize >= (mnMaxRange-mnMinRange)) ||
             ((mnMaxRange-mnMinRange) <= 0) )
        {
            mnThumbPos      = mnMinRange;
            mnThumbPixPos   = 0;
            mnThumbPixSize  = mnThumbPixRange;
        }
        else
        {
            if ( mnVisibleSize )
                mnThumbPixSize = ImplMulDiv( mnThumbPixRange, mnVisibleSize, mnMaxRange-mnMinRange );
            else
            {
                if ( GetStyle() & WB_HORZ )
                    mnThumbPixSize = maThumbRect.GetWidth();
                else
                    mnThumbPixSize = maThumbRect.GetHeight();
            }
            if ( mnThumbPixSize < nMinThumbSize )
                mnThumbPixSize = nMinThumbSize;
            if ( mnThumbPixSize > mnThumbPixRange )
                mnThumbPixSize = mnThumbPixRange;
            mnThumbPixPos = ImplCalcThumbPosPix( mnThumbPos );
        }
    }

    // Wenn neu ausgegeben werden soll und wir schon ueber eine
    // Aktion einen Paint-Event ausgeloest bekommen haben, dann
    // geben wir nicht direkt aus, sondern invalidieren nur alles
    if ( bUpdate && HasPaintEvent() )
    {
        Invalidate();
        bUpdate = FALSE;
    }
    ImplUpdateRects( bUpdate );
}

// -----------------------------------------------------------------------

void ScrollBar::Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize, ULONG nFlags )
{
    Point       aPos  = pDev->LogicToPixel( rPos );
    Size        aSize = pDev->LogicToPixel( rSize );
    Rectangle   aRect( aPos, aSize );

    pDev->Push();
    pDev->SetMapMode();
    if ( !(nFlags & WINDOW_DRAW_MONO) )
	{
		// DecoView uses the FaceColor...
		AllSettings aSettings = pDev->GetSettings();
		StyleSettings aStyleSettings = aSettings.GetStyleSettings();
		if ( IsControlBackground() )
			aStyleSettings.SetFaceColor( GetControlBackground() );
		else
			aStyleSettings.SetFaceColor( GetSettings().GetStyleSettings().GetFaceColor() );

		aSettings.SetStyleSettings( aStyleSettings );
		pDev->SetSettings( aSettings );
	}

    // for printing: 
    // -calculate the size of the rects
    // -because this is zero-based add the correct offset
    // -print
    // -force recalculate

    if ( mbCalcSize )
        ImplCalc( FALSE );

    maBtn1Rect+=aPos;
    maBtn2Rect+=aPos;
    maThumbRect+=aPos;
    maPage1Rect+=aPos;
    maPage2Rect+=aPos;

    ImplDraw( SCRBAR_DRAW_ALL, pDev );
    pDev->Pop();

    mbCalcSize = TRUE;
}

// -----------------------------------------------------------------------

BOOL ScrollBar::ImplDrawNative( USHORT nDrawFlags )
{
    BOOL bNativeOK = FALSE;

#ifdef USE_JAVA
    SetLineColor();
    SetFillColor( GetSettings().GetStyleSettings().GetCheckedColor() );
    DrawRect( Rectangle( Point( 0, 0 ), GetSizePixel() ) );
#endif	// USE_JAVA

    ImplControlValue aControlValue( BUTTONVALUE_DONTKNOW, rtl::OUString(), 0 );

    if( bNativeOK = IsNativeControlSupported(CTRL_SCROLLBAR, PART_ENTIRE_CONTROL) )
    {
        BOOL bHorz = (GetStyle() & WB_HORZ ? true : false);

        // Draw the entire background if the control supports it
        if( IsNativeControlSupported(CTRL_SCROLLBAR, bHorz ? PART_DRAW_BACKGROUND_HORZ : PART_DRAW_BACKGROUND_VERT) )
        {
            Region  		aCtrlRegion;
            ControlState		nState = ( IsEnabled() ? CTRL_STATE_ENABLED : 0 ) | ( HasFocus() ? CTRL_STATE_FOCUSED : 0 );
            ScrollbarValue	scrValue;

            scrValue.mnMin = mnMinRange;
            scrValue.mnMax = mnMaxRange;
            scrValue.mnCur = mnThumbPos;
            scrValue.mnVisibleSize = mnVisibleSize;
            scrValue.maThumbRect = maThumbRect;
            scrValue.maButton1Rect = maBtn1Rect;
            scrValue.maButton2Rect = maBtn2Rect;
            scrValue.mnButton1State = ((mnStateFlags & SCRBAR_STATE_BTN1_DOWN) ? CTRL_STATE_PRESSED : 0) |
								((!(mnStateFlags & SCRBAR_STATE_BTN1_DISABLE)) ? CTRL_STATE_ENABLED : 0);
            scrValue.mnButton2State = ((mnStateFlags & SCRBAR_STATE_BTN2_DOWN) ? CTRL_STATE_PRESSED : 0) |
								((!(mnStateFlags & SCRBAR_STATE_BTN2_DISABLE)) ? CTRL_STATE_ENABLED : 0);
            scrValue.mnThumbState = nState | ((mnStateFlags & SCRBAR_STATE_THUMB_DOWN) ? CTRL_STATE_PRESSED : 0);
            scrValue.mnPage1State = nState | ((mnStateFlags & SCRBAR_STATE_PAGE1_DOWN) ? CTRL_STATE_PRESSED : 0);
            scrValue.mnPage2State = nState | ((mnStateFlags & SCRBAR_STATE_PAGE2_DOWN) ? CTRL_STATE_PRESSED : 0);

            if( IsMouseOver() )
            {
                Rectangle* pRect = ImplFindPartRect( GetPointerPosPixel() );
                if( pRect )
                {
                    if( pRect == &maThumbRect )
                        scrValue.mnThumbState |= CTRL_STATE_ROLLOVER;
                    else if( pRect == &maBtn1Rect )
                        scrValue.mnButton1State |= CTRL_STATE_ROLLOVER;
                    else if( pRect == &maBtn2Rect )
                        scrValue.mnButton2State |= CTRL_STATE_ROLLOVER;
                    else if( pRect == &maPage1Rect )
                        scrValue.mnPage1State |= CTRL_STATE_ROLLOVER;
                    else if( pRect == &maPage2Rect )
                        scrValue.mnPage2State |= CTRL_STATE_ROLLOVER;
                }
            }

            aControlValue.setOptionalVal( (void *)(&scrValue) );

#ifdef USE_JAVA			
			if( mpData && mpData->mbHasEntireControlRect )
			{
				// use platform specific preferred boudns
				aCtrlRegion.Union( mpData->maEntireControlRect );
			}
			else
			{
				// approximate valid full control by what we need to cover all
				// hit test areas
#endif
            aCtrlRegion.Union( maBtn1Rect );
            aCtrlRegion.Union( maBtn2Rect );
            aCtrlRegion.Union( maPage1Rect );
            aCtrlRegion.Union( maPage2Rect );
            aCtrlRegion.Union( maThumbRect );
#ifdef USE_JAVA
			}
#endif

            bNativeOK = DrawNativeControl( CTRL_SCROLLBAR, (bHorz ? PART_DRAW_BACKGROUND_HORZ : PART_DRAW_BACKGROUND_VERT),
                            aCtrlRegion, nState, aControlValue, rtl::OUString() );
        }
        else
      {
        if ( (nDrawFlags & SCRBAR_DRAW_PAGE1) || (nDrawFlags & SCRBAR_DRAW_PAGE2) )
        {
            sal_uInt32	part1 = bHorz ? PART_TRACK_HORZ_LEFT : PART_TRACK_VERT_UPPER;
            sal_uInt32	part2 = bHorz ? PART_TRACK_HORZ_RIGHT : PART_TRACK_VERT_LOWER;
            Region  	aCtrlRegion1( maPage1Rect );
            Region  	aCtrlRegion2( maPage2Rect );
            ControlState nState1 = (IsEnabled() ? CTRL_STATE_ENABLED : 0) | (HasFocus() ? CTRL_STATE_FOCUSED : 0);
            ControlState nState2 = nState1;

            nState1 |= ((mnStateFlags & SCRBAR_STATE_PAGE1_DOWN) ? CTRL_STATE_PRESSED : 0);
            nState2 |= ((mnStateFlags & SCRBAR_STATE_PAGE2_DOWN) ? CTRL_STATE_PRESSED : 0);

            if( IsMouseOver() )
            {
                Rectangle* pRect = ImplFindPartRect( GetPointerPosPixel() );
                if( pRect )
                {
                    if( pRect == &maPage1Rect )
                        nState1 |= CTRL_STATE_ROLLOVER;
                    else if( pRect == &maPage2Rect )
                        nState2 |= CTRL_STATE_ROLLOVER;
                }
            }

            if ( nDrawFlags & SCRBAR_DRAW_PAGE1 )
                bNativeOK = DrawNativeControl( CTRL_SCROLLBAR, part1, aCtrlRegion1, nState1, 
                                aControlValue, rtl::OUString() );

            if ( nDrawFlags & SCRBAR_DRAW_PAGE2 )
                bNativeOK = DrawNativeControl( CTRL_SCROLLBAR, part2, aCtrlRegion2, nState2, 
                                aControlValue, rtl::OUString() );
        }
        if ( (nDrawFlags & SCRBAR_DRAW_BTN1) || (nDrawFlags & SCRBAR_DRAW_BTN2) )
        {
            sal_uInt32	part1 = bHorz ? PART_BUTTON_LEFT : PART_BUTTON_UP;
            sal_uInt32	part2 = bHorz ? PART_BUTTON_RIGHT : PART_BUTTON_DOWN;
            Region  	aCtrlRegion1( maBtn1Rect );
            Region  	aCtrlRegion2( maBtn2Rect );
            ControlState nState1 = HasFocus() ? CTRL_STATE_FOCUSED : 0;
            ControlState nState2 = nState1;

            if ( !Window::IsEnabled() || !IsEnabled() )
                nState1 = (nState2 &= ~CTRL_STATE_ENABLED);
            else
                nState1 = (nState2 |= CTRL_STATE_ENABLED);


            nState1 |= ((mnStateFlags & SCRBAR_STATE_BTN1_DOWN) ? CTRL_STATE_PRESSED : 0);
            nState2 |= ((mnStateFlags & SCRBAR_STATE_BTN2_DOWN) ? CTRL_STATE_PRESSED : 0);

            if(mnStateFlags & SCRBAR_STATE_BTN1_DISABLE)
                nState1 &= ~CTRL_STATE_ENABLED;
            if(mnStateFlags & SCRBAR_STATE_BTN2_DISABLE)
                nState2 &= ~CTRL_STATE_ENABLED;

            if( IsMouseOver() )
            {
                Rectangle* pRect = ImplFindPartRect( GetPointerPosPixel() );
                if( pRect )
                {
                    if( pRect == &maBtn1Rect )
                        nState1 |= CTRL_STATE_ROLLOVER;
                    else if( pRect == &maBtn2Rect )
                        nState2 |= CTRL_STATE_ROLLOVER;
                }
            }

            if ( nDrawFlags & SCRBAR_DRAW_BTN1 )
                bNativeOK = DrawNativeControl( CTRL_SCROLLBAR, part1, aCtrlRegion1, nState1, 
                                aControlValue, rtl::OUString() );

            if ( nDrawFlags & SCRBAR_DRAW_BTN2 )
                bNativeOK = DrawNativeControl( CTRL_SCROLLBAR, part2, aCtrlRegion2, nState2, 
                                aControlValue, rtl::OUString() );
        }
        if ( (nDrawFlags & SCRBAR_DRAW_THUMB) && !maThumbRect.IsEmpty() )
        {
            ControlState	nState = IsEnabled() ? CTRL_STATE_ENABLED : 0;
            Region		aCtrlRegion( maThumbRect );

            if ( mnStateFlags & SCRBAR_STATE_THUMB_DOWN )
                nState |= CTRL_STATE_PRESSED;

            if ( HasFocus() )
                nState |= CTRL_STATE_FOCUSED;

            if( IsMouseOver() )
            {
                Rectangle* pRect = ImplFindPartRect( GetPointerPosPixel() );
                if( pRect )
                {
                    if( pRect == &maThumbRect )
                        nState |= CTRL_STATE_ROLLOVER;
                }
            }

            bNativeOK = DrawNativeControl( CTRL_SCROLLBAR, (bHorz ? PART_THUMB_HORZ : PART_THUMB_VERT),
                    aCtrlRegion, nState, aControlValue, rtl::OUString() );
        }
      }
    }
    return bNativeOK;
}

void ScrollBar::ImplDraw( USHORT nDrawFlags, OutputDevice* pOutDev )
{
    DecorationView          aDecoView( pOutDev );
    Rectangle               aTempRect;
    USHORT                  nStyle;
    const StyleSettings&    rStyleSettings = pOutDev->GetSettings().GetStyleSettings();
    SymbolType              eSymbolType;
    BOOL                    bEnabled = IsEnabled();

    // Evt. noch offene Berechnungen nachholen
    if ( mbCalcSize )
        ImplCalc( FALSE );

    Window *pWin = NULL;
    if( pOutDev->GetOutDevType() == OUTDEV_WINDOW )
        pWin = (Window*) pOutDev;
    
    // Draw the entire control if the native theme engine needs it
    if ( nDrawFlags && pWin && pWin->IsNativeControlSupported(CTRL_SCROLLBAR, PART_DRAW_BACKGROUND_HORZ) )
    {
        ImplDrawNative( SCRBAR_DRAW_BACKGROUND );
        return;
    }

    if( (nDrawFlags & SCRBAR_DRAW_BTN1) && (!pWin || !ImplDrawNative( SCRBAR_DRAW_BTN1 ) ) )
    {
        nStyle = BUTTON_DRAW_NOLIGHTBORDER;
        if ( mnStateFlags & SCRBAR_STATE_BTN1_DOWN )
            nStyle |= BUTTON_DRAW_PRESSED;
        aTempRect = aDecoView.DrawButton( maBtn1Rect, nStyle );
        ImplCalcSymbolRect( aTempRect );
        nStyle = 0;
        if ( (mnStateFlags & SCRBAR_STATE_BTN1_DISABLE) || !bEnabled )
            nStyle |= SYMBOL_DRAW_DISABLE;
        if ( rStyleSettings.GetOptions() & STYLE_OPTION_SCROLLARROW )
        {
            if ( GetStyle() & WB_HORZ )
                eSymbolType = SYMBOL_ARROW_LEFT;
            else
                eSymbolType = SYMBOL_ARROW_UP;
        }
        else
        {
            if ( GetStyle() & WB_HORZ )
                eSymbolType = SYMBOL_SPIN_LEFT;
            else
                eSymbolType = SYMBOL_SPIN_UP;
        }
        aDecoView.DrawSymbol( aTempRect, eSymbolType, rStyleSettings.GetButtonTextColor(), nStyle );
    }

    if ( (nDrawFlags & SCRBAR_DRAW_BTN2) && (!pWin || !ImplDrawNative( SCRBAR_DRAW_BTN2 ) ) )
    {
        nStyle = BUTTON_DRAW_NOLIGHTBORDER;
        if ( mnStateFlags & SCRBAR_STATE_BTN2_DOWN )
            nStyle |= BUTTON_DRAW_PRESSED;
        aTempRect = aDecoView.DrawButton(  maBtn2Rect, nStyle );
        ImplCalcSymbolRect( aTempRect );
        nStyle = 0;
        if ( (mnStateFlags & SCRBAR_STATE_BTN2_DISABLE) || !bEnabled )
            nStyle |= SYMBOL_DRAW_DISABLE;
        if ( rStyleSettings.GetOptions() & STYLE_OPTION_SCROLLARROW )
        {
            if ( GetStyle() & WB_HORZ )
                eSymbolType = SYMBOL_ARROW_RIGHT;
            else
                eSymbolType = SYMBOL_ARROW_DOWN;
        }
        else
        {
            if ( GetStyle() & WB_HORZ )
                eSymbolType = SYMBOL_SPIN_RIGHT;
            else
                eSymbolType = SYMBOL_SPIN_DOWN;
        }
        aDecoView.DrawSymbol( aTempRect, eSymbolType, rStyleSettings.GetButtonTextColor(), nStyle );
    }

    pOutDev->SetLineColor();

    if ( (nDrawFlags & SCRBAR_DRAW_THUMB) && (!pWin || !ImplDrawNative( SCRBAR_DRAW_THUMB ) ) )
    {
        if ( !maThumbRect.IsEmpty() )
        {
            if ( bEnabled )
            {
                nStyle = BUTTON_DRAW_NOLIGHTBORDER;
                // pressed thumbs only in OS2 style
                if ( rStyleSettings.GetOptions() & STYLE_OPTION_OS2STYLE )
                    if ( mnStateFlags & SCRBAR_STATE_THUMB_DOWN )
                        nStyle |= BUTTON_DRAW_PRESSED;
                aTempRect = aDecoView.DrawButton( maThumbRect, nStyle );
                // OS2 style requires pattern on the thumb
                if ( rStyleSettings.GetOptions() & STYLE_OPTION_OS2STYLE )
                {
                    if ( GetStyle() & WB_HORZ )
                    {
                        if ( aTempRect.GetWidth() > 6 )
                        {
                            long nX = aTempRect.Center().X();
                            nX -= 6;
                            if ( nX < aTempRect.Left() )
                                nX = aTempRect.Left();
                            for ( int i = 0; i < 6; i++ )
                            {
                                if ( nX > aTempRect.Right()-1 )
                                    break;

                                pOutDev->SetLineColor( rStyleSettings.GetButtonTextColor() );
                                pOutDev->DrawLine( Point( nX, aTempRect.Top()+1 ),
                                          Point( nX, aTempRect.Bottom()-1 ) );
                                nX++;
                                pOutDev->SetLineColor( rStyleSettings.GetLightColor() );
                                pOutDev->DrawLine( Point( nX, aTempRect.Top()+1 ),
                                          Point( nX, aTempRect.Bottom()-1 ) );
                                nX++;
                            }
                        }
                    }
                    else
                    {
                        if ( aTempRect.GetHeight() > 6 )
                        {
                            long nY = aTempRect.Center().Y();
                            nY -= 6;
                            if ( nY < aTempRect.Top() )
                                nY = aTempRect.Top();
                            for ( int i = 0; i < 6; i++ )
                            {
                                if ( nY > aTempRect.Bottom()-1 )
                                    break;

                                pOutDev->SetLineColor( rStyleSettings.GetButtonTextColor() );
                                pOutDev->DrawLine( Point( aTempRect.Left()+1, nY ),
                                          Point( aTempRect.Right()-1, nY ) );
                                nY++;
                                pOutDev->SetLineColor( rStyleSettings.GetLightColor() );
                                pOutDev->DrawLine( Point( aTempRect.Left()+1, nY ),
                                          Point( aTempRect.Right()-1, nY ) );
                                nY++;
                            }
                        }
                    }
                    pOutDev->SetLineColor();
                }
            }
            else
            {
                pOutDev->SetFillColor( rStyleSettings.GetCheckedColor() );
                pOutDev->DrawRect( maThumbRect );
            }
        }
    }

    if ( (nDrawFlags & SCRBAR_DRAW_PAGE1) && (!pWin || !ImplDrawNative( SCRBAR_DRAW_PAGE1 ) ) )
    {
        if ( mnStateFlags & SCRBAR_STATE_PAGE1_DOWN )
            pOutDev->SetFillColor( rStyleSettings.GetShadowColor() );
        else
            pOutDev->SetFillColor( rStyleSettings.GetCheckedColor() );
        pOutDev->DrawRect( maPage1Rect );
    }
    if ( (nDrawFlags & SCRBAR_DRAW_PAGE2) && (!pWin || !ImplDrawNative( SCRBAR_DRAW_PAGE2 ) ) )
    {
        if ( mnStateFlags & SCRBAR_STATE_PAGE2_DOWN )
            pOutDev->SetFillColor( rStyleSettings.GetShadowColor() );
        else
            pOutDev->SetFillColor( rStyleSettings.GetCheckedColor() );
        pOutDev->DrawRect( maPage2Rect );
    }
}

// -----------------------------------------------------------------------

long ScrollBar::ImplScroll( long nNewPos, BOOL bCallEndScroll )
{
    long nOldPos = mnThumbPos;
    SetThumbPos( nNewPos );
    long nDelta = mnThumbPos-nOldPos;
    if ( nDelta )
    {
        mnDelta = nDelta;
        Scroll();
        if ( bCallEndScroll )
            EndScroll();
        mnDelta = 0;
    }
    return nDelta;
}

// -----------------------------------------------------------------------

long ScrollBar::ImplDoAction( BOOL bCallEndScroll )
{
    long nDelta = 0;

    switch ( meScrollType )
    {
        case SCROLL_LINEUP:
            nDelta = ImplScroll( mnThumbPos-mnLineSize, bCallEndScroll );
            break;

        case SCROLL_LINEDOWN:
            nDelta = ImplScroll( mnThumbPos+mnLineSize, bCallEndScroll );
            break;

        case SCROLL_PAGEUP:
            nDelta = ImplScroll( mnThumbPos-mnPageSize, bCallEndScroll );
            break;

        case SCROLL_PAGEDOWN:
            nDelta = ImplScroll( mnThumbPos+mnPageSize, bCallEndScroll );
            break;
        default:
            ;
    }

    return nDelta;
}

// -----------------------------------------------------------------------

void ScrollBar::ImplDoMouseAction( const Point& rMousePos, BOOL bCallAction )
{
    USHORT  nOldStateFlags = mnStateFlags;
    BOOL    bAction = FALSE;
    BOOL    bHorizontal = ( GetStyle() & WB_HORZ )? TRUE: FALSE;
    BOOL    bIsInside = FALSE;

    Point aPoint( 0, 0 );
    Region aControlRegion( Rectangle( aPoint, GetOutputSizePixel() ) );

    switch ( meScrollType )
    {
        case SCROLL_LINEUP:
            if ( HitTestNativeControl( CTRL_SCROLLBAR, bHorizontal? PART_BUTTON_LEFT: PART_BUTTON_UP,
                        aControlRegion, rMousePos, bIsInside )?
                    bIsInside:
                    maBtn1Rect.IsInside( rMousePos ) )
            {
                bAction = bCallAction;
                mnStateFlags |= SCRBAR_STATE_BTN1_DOWN;
            }
            else
                mnStateFlags &= ~SCRBAR_STATE_BTN1_DOWN;
            break;

        case SCROLL_LINEDOWN:
            if ( HitTestNativeControl( CTRL_SCROLLBAR, bHorizontal? PART_BUTTON_RIGHT: PART_BUTTON_DOWN,
                        aControlRegion, rMousePos, bIsInside )?
                    bIsInside:
                    maBtn2Rect.IsInside( rMousePos ) )
            {
                bAction = bCallAction;
                mnStateFlags |= SCRBAR_STATE_BTN2_DOWN;
            }
            else
                mnStateFlags &= ~SCRBAR_STATE_BTN2_DOWN;
            break;

        case SCROLL_PAGEUP:
            if ( maPage1Rect.IsInside( rMousePos ) )
            {
                bAction = bCallAction;
                mnStateFlags |= SCRBAR_STATE_PAGE1_DOWN;
            }
            else
                mnStateFlags &= ~SCRBAR_STATE_PAGE1_DOWN;
            break;

        case SCROLL_PAGEDOWN:
            if ( maPage2Rect.IsInside( rMousePos ) )
            {
                bAction = bCallAction;
                mnStateFlags |= SCRBAR_STATE_PAGE2_DOWN;
            }
            else
                mnStateFlags &= ~SCRBAR_STATE_PAGE2_DOWN;
            break;
        default:
            ;
    }

    if ( nOldStateFlags != mnStateFlags )
        ImplDraw( mnDragDraw, this );
    if ( bAction )
        ImplDoAction( FALSE );
}

// -----------------------------------------------------------------------

void ScrollBar::ImplDragThumb( const Point& rMousePos )
{
    long nMovePix;
    if ( GetStyle() & WB_HORZ )
        nMovePix = rMousePos.X()-(maThumbRect.Left()+mnMouseOff);
    else
        nMovePix = rMousePos.Y()-(maThumbRect.Top()+mnMouseOff);

    // move thumb if necessary
    if ( nMovePix )
    {
        mnThumbPixPos += nMovePix;
        if ( mnThumbPixPos < 0 )
            mnThumbPixPos = 0;
        if ( mnThumbPixPos > (mnThumbPixRange-mnThumbPixSize) )
            mnThumbPixPos = mnThumbPixRange-mnThumbPixSize;
        long nOldPos = mnThumbPos;
        mnThumbPos = ImplCalcThumbPos( mnThumbPixPos );
        ImplUpdateRects();
        if ( mbFullDrag && (nOldPos != mnThumbPos) )
        {
            mnDelta = mnThumbPos-nOldPos;
            Scroll();
            mnDelta = 0;
        }
    }
}

// -----------------------------------------------------------------------

void ScrollBar::MouseButtonDown( const MouseEvent& rMEvt )
{
    if ( rMEvt.IsLeft() || rMEvt.IsMiddle() )
    {
        const Point&    rMousePos = rMEvt.GetPosPixel();
        USHORT          nTrackFlags = 0;
        BOOL            bHorizontal = ( GetStyle() & WB_HORZ )? TRUE: FALSE;
        BOOL            bIsInside = FALSE;
        BOOL            bDragToMouse = FALSE;

        Point aPoint( 0, 0 );
        Region aControlRegion( Rectangle( aPoint, GetOutputSizePixel() ) );

        if ( HitTestNativeControl( CTRL_SCROLLBAR, bHorizontal? PART_BUTTON_LEFT: PART_BUTTON_UP,
                    aControlRegion, rMousePos, bIsInside )?
                bIsInside:
                maBtn1Rect.IsInside( rMousePos ) )
        {
            if ( !(mnStateFlags & SCRBAR_STATE_BTN1_DISABLE) )
            {
                nTrackFlags     = STARTTRACK_BUTTONREPEAT;
                meScrollType    = SCROLL_LINEUP;
                mnDragDraw      = SCRBAR_DRAW_BTN1;
            }
            else
                Sound::Beep( SOUND_DISABLE, this );
        }
        else if ( HitTestNativeControl( CTRL_SCROLLBAR, bHorizontal? PART_BUTTON_RIGHT: PART_BUTTON_DOWN,
                    aControlRegion, rMousePos, bIsInside )?
                bIsInside:
                maBtn2Rect.IsInside( rMousePos ) )
        {
            if ( !(mnStateFlags & SCRBAR_STATE_BTN2_DISABLE) )
            {
                nTrackFlags     = STARTTRACK_BUTTONREPEAT;
                meScrollType    = SCROLL_LINEDOWN;
                mnDragDraw      = SCRBAR_DRAW_BTN2;
            }
            else
                Sound::Beep( SOUND_DISABLE, this );
        }
        else if ( maThumbRect.IsInside( rMousePos ) || rMEvt.IsMiddle() )
        {
            if( mpData )
            {
                mpData->mbHide = TRUE;  // disable focus blinking
                if( HasFocus() )
                    ImplDraw( SCRBAR_DRAW_THUMB, this ); // paint without focus
            }

            if ( mnVisibleSize < mnMaxRange-mnMinRange )
            {
                nTrackFlags     = 0;
                meScrollType    = SCROLL_DRAG;
                mnDragDraw      = SCRBAR_DRAW_THUMB;
                
                // calculate mouse offset
                if( rMEvt.IsMiddle() )
                {
                    bDragToMouse = TRUE;
                    if ( GetStyle() & WB_HORZ )
                        mnMouseOff = maThumbRect.GetWidth()/2;
                    else
                        mnMouseOff = maThumbRect.GetHeight()/2;
                }
                else
                {
                    if ( GetStyle() & WB_HORZ )
                        mnMouseOff = rMousePos.X()-maThumbRect.Left();
                    else
                        mnMouseOff = rMousePos.Y()-maThumbRect.Top();
                }

                mnStateFlags |= SCRBAR_STATE_THUMB_DOWN;
                ImplDraw( mnDragDraw, this );
            }
            else
                Sound::Beep( SOUND_DISABLE, this );
        }
        else
        {
            nTrackFlags = STARTTRACK_BUTTONREPEAT;

            if ( maPage1Rect.IsInside( rMousePos ) )
            {
                meScrollType    = SCROLL_PAGEUP;
                mnDragDraw      = SCRBAR_DRAW_PAGE1;
            }
            else
            {
                meScrollType    = SCROLL_PAGEDOWN;
                mnDragDraw      = SCRBAR_DRAW_PAGE2;
            }
        }

        // Soll Tracking gestartet werden
        if ( meScrollType != SCROLL_DONTKNOW )
        {
            // remember original position in case of abort or EndScroll-Delta
            mnStartPos = mnThumbPos;
            // #92906# Call StartTracking() before ImplDoMouseAction(), otherwise 
            // MouseButtonUp() / EndTracking() may be called if somebody is spending 
            // a lot of time in the scroll handler
            StartTracking( nTrackFlags );
            ImplDoMouseAction( rMousePos );
            
            if( bDragToMouse )
                ImplDragThumb( rMousePos );
        }
    }
}

// -----------------------------------------------------------------------

void ScrollBar::Tracking( const TrackingEvent& rTEvt )
{
    if ( rTEvt.IsTrackingEnded() )
    {
        // Button und PageRect-Status wieder herstellen
        USHORT nOldStateFlags = mnStateFlags;
        mnStateFlags &= ~(SCRBAR_STATE_BTN1_DOWN | SCRBAR_STATE_BTN2_DOWN |
                          SCRBAR_STATE_PAGE1_DOWN | SCRBAR_STATE_PAGE2_DOWN |
                          SCRBAR_STATE_THUMB_DOWN);
        if ( nOldStateFlags != mnStateFlags )
            ImplDraw( mnDragDraw, this );
        mnDragDraw = 0;

        // Bei Abbruch, die alte ThumbPosition wieder herstellen
        if ( rTEvt.IsTrackingCanceled() )
        {
            long nOldPos = mnThumbPos;
            SetThumbPos( mnStartPos );
            mnDelta = mnThumbPos-nOldPos;
            Scroll();
        }

        if ( meScrollType == SCROLL_DRAG )
        {
            // Wenn gedragt wurde, berechnen wir den Thumb neu, damit
            // er wieder auf einer gerundeten ThumbPosition steht
            ImplCalc();

            if ( !mbFullDrag && (mnStartPos != mnThumbPos) )
            {
                mnDelta = mnThumbPos-mnStartPos;
                Scroll();
                mnDelta = 0;
            }
        }

        mnDelta = mnThumbPos-mnStartPos;
        EndScroll();
        mnDelta = 0;
        meScrollType = SCROLL_DONTKNOW;

        if( mpData )
            mpData->mbHide = FALSE; // re-enable focus blinking

    }
    else
    {
        const Point rMousePos = rTEvt.GetMouseEvent().GetPosPixel();

        // Dragging wird speziell behandelt
        if ( meScrollType == SCROLL_DRAG )
            ImplDragThumb( rMousePos );
        else
            ImplDoMouseAction( rMousePos, rTEvt.IsTrackingRepeat() );

        // Wenn ScrollBar-Werte so umgesetzt wurden, das es nichts
        // mehr zum Tracking gibt, dann berechen wir hier ab
        if ( !IsVisible() || (mnVisibleSize >= (mnMaxRange-mnMinRange)) )
            EndTracking();
    }
}

// -----------------------------------------------------------------------

void ScrollBar::KeyInput( const KeyEvent& rKEvt )
{
    if ( !rKEvt.GetKeyCode().GetModifier() )
    {
        switch ( rKEvt.GetKeyCode().GetCode() )
        {
            case KEY_HOME:
                DoScroll( 0 );
                break;

            case KEY_END:
                DoScroll( GetRangeMax() );
                break;

            case KEY_LEFT:
            case KEY_UP:
                DoScrollAction( SCROLL_LINEUP );
                break;

            case KEY_RIGHT:
            case KEY_DOWN:
                DoScrollAction( SCROLL_LINEDOWN );
                break;

            case KEY_PAGEUP:
                DoScrollAction( SCROLL_PAGEUP );
                break;

            case KEY_PAGEDOWN:
                DoScrollAction( SCROLL_PAGEDOWN );
                break;

            default:
                Control::KeyInput( rKEvt );
                break;
        }
    }
    else
        Control::KeyInput( rKEvt );
}

// -----------------------------------------------------------------------

void ScrollBar::Paint( const Rectangle& rRect )
{
    ImplDraw( SCRBAR_DRAW_ALL, this );
}

// -----------------------------------------------------------------------

void ScrollBar::Resize()
{
    Control::Resize();
    mbCalcSize = TRUE;
    if ( IsReallyVisible() )
        ImplCalc( FALSE );
    Invalidate();
}

// -----------------------------------------------------------------------

IMPL_LINK( ScrollBar, ImplAutoTimerHdl, AutoTimer*, EMPTYARG )
{
    if( mpData && mpData->mbHide )
        return 0;
    ImplInvert();
    return 0;
}

void ScrollBar::ImplInvert()
{
    Rectangle aRect( maThumbRect );
    if( aRect.getWidth() > 4 )
    {
        aRect.Left() += 2;
        aRect.Right() -= 2;
    }
    if( aRect.getHeight() > 4 ) 
    {
        aRect.Top() += 2;
        aRect.Bottom() -= 2;
    }

    Invert( aRect, 0 );
}

// -----------------------------------------------------------------------

void ScrollBar::GetFocus()
{
    if( !mpData )
#ifdef USE_JAVA
    	ImplNewImplScrollBarData();
#else
    {
	    mpData = new ImplScrollBarData;
		mpData->maTimer.SetTimeoutHdl( LINK( this, ScrollBar, ImplAutoTimerHdl ) );
        mpData->mbHide = FALSE;
    }
#endif
    ImplInvert();   // react immediately
	mpData->maTimer.SetTimeout( GetSettings().GetStyleSettings().GetCursorBlinkTime() );
    mpData->maTimer.Start();
    Control::GetFocus();
}

// -----------------------------------------------------------------------

#ifdef USE_JAVA
void ScrollBar::ImplNewImplScrollBarData()
{
	mpData = new ImplScrollBarData;
	mpData->maTimer.SetTimeoutHdl( LINK( this, ScrollBar, ImplAutoTimerHdl ) );
	mpData->mbHide = FALSE;
	mpData->mbHasEntireControlRect = FALSE;
}

// -----------------------------------------------------------------------

#endif
void ScrollBar::LoseFocus()
{
    if( mpData )
        mpData->maTimer.Stop();
    ImplDraw( SCRBAR_DRAW_THUMB, this );

    Control::LoseFocus();
}

// -----------------------------------------------------------------------

void ScrollBar::StateChanged( StateChangedType nType )
{
    Control::StateChanged( nType );

    if ( nType == STATE_CHANGE_INITSHOW )
        ImplCalc( FALSE );
    else if ( nType == STATE_CHANGE_DATA )
    {
        if ( IsReallyVisible() && IsUpdateMode() )
            ImplCalc( TRUE );
    }
    else if ( nType == STATE_CHANGE_UPDATEMODE )
    {
        if ( IsReallyVisible() && IsUpdateMode() )
        {
            ImplCalc( FALSE );
            Invalidate();
        }
    }
    else if ( nType == STATE_CHANGE_ENABLE )
    {
        if ( IsReallyVisible() && IsUpdateMode() )
            Invalidate();
    }
    else if ( nType == STATE_CHANGE_STYLE )
    {
        ImplInitStyle( GetStyle() );
        if ( IsReallyVisible() && IsUpdateMode() )
        {
            if ( (GetPrevStyle() & SCRBAR_VIEW_STYLE) !=
                 (GetStyle() & SCRBAR_VIEW_STYLE) )
            {
                mbCalcSize = TRUE;
                ImplCalc( FALSE );
                Invalidate();
            }
        }
    }
}

// -----------------------------------------------------------------------

void ScrollBar::DataChanged( const DataChangedEvent& rDCEvt )
{
    Control::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
         (rDCEvt.GetFlags() & SETTINGS_STYLE) )
    {
        mbCalcSize = TRUE;
        ImplCalc( FALSE );
        Invalidate();
    }
}

// -----------------------------------------------------------------------

Rectangle* ScrollBar::ImplFindPartRect( const Point& rPt )
{
    BOOL    bHorizontal = ( GetStyle() & WB_HORZ )? TRUE: FALSE;
    BOOL    bIsInside = FALSE;

    Point aPoint( 0, 0 );
    Region aControlRegion( Rectangle( aPoint, GetOutputSizePixel() ) );

    if( HitTestNativeControl( CTRL_SCROLLBAR, bHorizontal? PART_BUTTON_LEFT: PART_BUTTON_UP,
                aControlRegion, rPt, bIsInside )?
            bIsInside:
            maBtn1Rect.IsInside( rPt ) )
        return &maBtn1Rect;
    else if( HitTestNativeControl( CTRL_SCROLLBAR, bHorizontal? PART_BUTTON_RIGHT: PART_BUTTON_DOWN,
                aControlRegion, rPt, bIsInside )?
            bIsInside:
            maBtn2Rect.IsInside( rPt ) )
        return &maBtn2Rect;
    else if( maPage1Rect.IsInside( rPt ) )
        return &maPage1Rect;
    else if( maPage2Rect.IsInside( rPt ) )
        return &maPage2Rect;
    else if( maThumbRect.IsInside( rPt ) )
        return &maThumbRect;
    else
        return NULL;
}

long ScrollBar::PreNotify( NotifyEvent& rNEvt )
{
    long nDone = 0;
    const MouseEvent* pMouseEvt = NULL;

    if( (rNEvt.GetType() == EVENT_MOUSEMOVE) && (pMouseEvt = rNEvt.GetMouseEvent()) )
    {
        if( !pMouseEvt->GetButtons() && !pMouseEvt->IsSynthetic() && !pMouseEvt->IsModifierChanged() )
        {
            // trigger redraw if mouse over state has changed
            if( IsNativeControlSupported(CTRL_SCROLLBAR, PART_ENTIRE_CONTROL) )
            {
                Rectangle* pRect = ImplFindPartRect( GetPointerPosPixel() );
                Rectangle* pLastRect = ImplFindPartRect( GetLastPointerPosPixel() );
                if( pRect != pLastRect || pMouseEvt->IsLeaveWindow() || pMouseEvt->IsEnterWindow() )
                {
                    Region aRgn( GetActiveClipRegion() );
                    Region aClipRegion;

                    if ( pRect )
                        aClipRegion.Union( *pRect );
                    if ( pLastRect )
                        aClipRegion.Union( *pLastRect );
                    
                    // Support for 3-button scroll bars
                    BOOL bHas3Buttons = IsNativeControlSupported( CTRL_SCROLLBAR, HAS_THREE_BUTTONS );
                    if ( bHas3Buttons && ( pRect == &maBtn1Rect || pLastRect == &maBtn1Rect ) )
                    {
                        aClipRegion.Union( maBtn2Rect );
                    }

                    SetClipRegion( aClipRegion );
                    Paint( aClipRegion.GetBoundRect() );

                    SetClipRegion( aRgn );
                }
            }
        }
    }

    return nDone ? nDone : Control::PreNotify(rNEvt);
}

// -----------------------------------------------------------------------

void ScrollBar::Scroll()
{
    ImplCallEventListenersAndHandler( VCLEVENT_SCROLLBAR_SCROLL, maScrollHdl, this );
}

// -----------------------------------------------------------------------

void ScrollBar::EndScroll()
{
    ImplCallEventListenersAndHandler( VCLEVENT_SCROLLBAR_ENDSCROLL, maEndScrollHdl, this );
}

// -----------------------------------------------------------------------

long ScrollBar::DoScroll( long nNewPos )
{
    if ( meScrollType != SCROLL_DONTKNOW )
        return 0;

    meScrollType = SCROLL_DRAG;
    long nDelta = ImplScroll( nNewPos, TRUE );
    meScrollType = SCROLL_DONTKNOW;
    return nDelta;
}

// -----------------------------------------------------------------------

long ScrollBar::DoScrollAction( ScrollType eScrollType )
{
    if ( (meScrollType != SCROLL_DONTKNOW) ||
         (eScrollType == SCROLL_DONTKNOW) ||
         (eScrollType == SCROLL_DRAG) )
        return 0;

    meScrollType = eScrollType;
    long nDelta = ImplDoAction( TRUE );
    meScrollType = SCROLL_DONTKNOW;
    return nDelta;
}

// -----------------------------------------------------------------------

void ScrollBar::SetRangeMin( long nNewRange )
{
    SetRange( Range( nNewRange, GetRangeMax() ) );
}

// -----------------------------------------------------------------------

void ScrollBar::SetRangeMax( long nNewRange )
{
    SetRange( Range( GetRangeMin(), nNewRange ) );
}

// -----------------------------------------------------------------------

void ScrollBar::SetRange( const Range& rRange )
{
    // Range einpassen
    Range aRange = rRange;
    aRange.Justify();
    long nNewMinRange = aRange.Min();
    long nNewMaxRange = aRange.Max();

    // Wenn Range sich unterscheidet, dann neuen setzen
    if ( (mnMinRange != nNewMinRange) ||
         (mnMaxRange != nNewMaxRange) )
    {
        mnMinRange = nNewMinRange;
        mnMaxRange = nNewMaxRange;

        // Thumb einpassen
        if ( mnThumbPos > mnMaxRange-mnVisibleSize )
            mnThumbPos = mnMaxRange-mnVisibleSize;
        if ( mnThumbPos < mnMinRange )
            mnThumbPos = mnMinRange;

        StateChanged( STATE_CHANGE_DATA );
    }
}

// -----------------------------------------------------------------------

void ScrollBar::SetThumbPos( long nNewThumbPos )
{
    if ( nNewThumbPos > mnMaxRange-mnVisibleSize )
        nNewThumbPos = mnMaxRange-mnVisibleSize;
    if ( nNewThumbPos < mnMinRange )
        nNewThumbPos = mnMinRange;

    if ( mnThumbPos != nNewThumbPos )
    {
        mnThumbPos = nNewThumbPos;
        StateChanged( STATE_CHANGE_DATA );
    }
}

// -----------------------------------------------------------------------

void ScrollBar::SetVisibleSize( long nNewSize )
{
    if ( mnVisibleSize != nNewSize )
    {
        mnVisibleSize = nNewSize;

        // Thumb einpassen
        if ( mnThumbPos > mnMaxRange-mnVisibleSize )
            mnThumbPos = mnMaxRange-mnVisibleSize;
        if ( mnThumbPos < mnMinRange )
            mnThumbPos = mnMinRange;
        StateChanged( STATE_CHANGE_DATA );
    }
}

// =======================================================================

void ScrollBarBox::ImplInit( Window* pParent, WinBits nStyle )
{
    Window::ImplInit( pParent, nStyle, NULL );

    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
    long nScrollSize = rStyleSettings.GetScrollBarSize();
    SetSizePixel( Size( nScrollSize, nScrollSize ) );
    ImplInitSettings();
}

// -----------------------------------------------------------------------

ScrollBarBox::ScrollBarBox( Window* pParent, WinBits nStyle ) :
    Window( WINDOW_SCROLLBARBOX )
{
    ImplInit( pParent, nStyle );
}

// -----------------------------------------------------------------------

ScrollBarBox::ScrollBarBox( Window* pParent, const ResId& rResId ) :
    Window( WINDOW_SCROLLBARBOX )
{
    rResId.SetRT( RSC_SCROLLBAR );
    ImplInit( pParent, ImplInitRes( rResId ) );
    ImplLoadRes( rResId );
}

// -----------------------------------------------------------------------

void ScrollBarBox::ImplInitSettings()
{
    // Hack, damit man auch DockingWindows ohne Hintergrund bauen kann
    // und noch nicht alles umgestellt ist
    if ( IsBackground() )
    {
        Color aColor;
        if ( IsControlBackground() )
            aColor = GetControlBackground();
        else
            aColor = GetSettings().GetStyleSettings().GetFaceColor();
        SetBackground( aColor );
    }
}

// -----------------------------------------------------------------------

void ScrollBarBox::StateChanged( StateChangedType nType )
{
    Window::StateChanged( nType );

    if ( nType == STATE_CHANGE_CONTROLBACKGROUND )
    {
        ImplInitSettings();
        Invalidate();
    }
}

// -----------------------------------------------------------------------

void ScrollBarBox::DataChanged( const DataChangedEvent& rDCEvt )
{
    Window::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
         (rDCEvt.GetFlags() & SETTINGS_STYLE) )
    {
        ImplInitSettings();
        Invalidate();
    }
}
