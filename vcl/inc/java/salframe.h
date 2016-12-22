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
 *	 - GNU General Public License Version 2.1
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

#ifndef _SV_SALFRAME_H
#define _SV_SALFRAME_H

#include <list>

#include <tools/link.hxx>
#include <vcl/salgtype.hxx>
#include <vcl/sysdata.hxx>

#include <premac.h>
#include <ApplicationServices/ApplicationServices.h>
#include <postmac.h>
#undef check

#include "salframe.hxx"
#include "salgeom.hxx"

#ifndef __OBJC__
typedef void* id;
#endif	// !__OBJC__

class JavaSalGraphics;
class JavaSalMenu;
class JavaSalObject;
class SalBitmap;

// ----------------
// - JavaSalFrame -
// ----------------

class JavaSalFrame : public SalFrame
{
private:
	CGLayerRef				maFrameLayer;
	CGMutablePathRef		maFrameClipPath;

public:
	id						mpWindow;
	bool					mbAllowKeyBindings;
	JavaSalGraphics*		mpGraphics;
	sal_uLong				mnStyle;
	JavaSalFrame*			mpParent;
	sal_Bool				mbGraphics;
	bool					mbVisible;
	::std::list< JavaSalFrame* >	maChildren;
	SystemEnvData			maSysData;
	sal_Bool				mbCenter;
	SalFrameGeometry		maOriginalGeometry;
	bool					mbFullScreen;
	bool					mbPresentation;
	JavaSalMenu*			mpMenuBar;
	sal_Bool				mbInSetPosSize;
	sal_Bool				mbInShow;
	::std::list< JavaSalObject* >	maObjects;
	::std::list< JavaSalObject* >	maVisibleObjects;
	OUString				maTitle;
	sal_Bool				mbShowOnlyMenus;
	sal_Bool				mbInShowOnlyMenus;
	::std::list< JavaSalMenu* >	maUpdateMenuList;
	sal_Bool				mbInShowFullScreen;
	sal_Bool				mbInWindowDidExitFullScreen;
	sal_Bool				mbInWindowWillEnterFullScreen;
	sal_Bool				mbInSetWindowState;

	static OUString			ConvertVCLKeyCode( sal_uInt16 nKeyCode, bool bIsMenuShortcut );
	static void				FlushAllFrames();
	static unsigned int		GetDefaultScreenNumber();
	static const Rectangle	GetScreenBounds( long nX, long nY, long nWidth, long nHeight, sal_Bool bFullScreenMode );
	static const Rectangle	GetScreenBounds( unsigned int nScreen, sal_Bool bFullScreenMode );
	static unsigned int		GetScreenCount();
	static bool				GetAlternateSelectedControlTextColor( SalColor& rSalColor );
	static bool				GetControlTextColor( SalColor& rSalColor );
	static bool				GetDisabledControlTextColor( SalColor& rSalColor );
	static bool				GetSelectedControlTextColor( SalColor& rSalColor );
	static bool				GetSelectedMenuItemTextColor( SalColor& rSalColor );

							JavaSalFrame( sal_uLong nSalFrameStyle, JavaSalFrame *pParent );
	virtual					~JavaSalFrame();

	void					AddObject( JavaSalObject *pObject, bool bVisible );
	bool					Deminimize();
	bool					IsFloatingFrame();
	bool					IsUtilityWindow();
	void					RemoveObject( JavaSalObject *pObject, bool bDeleted );
	void					FlushAllObjects();
	const Rectangle			GetBounds( sal_Bool *pInLiveResize = NULL, sal_Bool bUseFullScreenOriginalBounds = sal_False );
	const Rectangle			GetInsets();
	id						GetNativeWindow();
	id						GetNativeWindowContentView( sal_Bool bTopLevelWindow );
	sal_uLong				GetState();
	void					MakeModal();
	bool					RequestFocus();
	void					SetState( sal_uLong nFrameState );
	void					SetVisible( bool bVisible, bool bNoActivate );
	bool					ToFront();
	void					UpdateLayer();
	void					AddTrackingRect( vcl::Window *pWindow );
	void					RemoveTrackingRect( vcl::Window *pWindow );

	virtual SalGraphics*	AcquireGraphics() SAL_OVERRIDE;
	virtual void			ReleaseGraphics( SalGraphics* pGraphics ) SAL_OVERRIDE;
	virtual bool			PostEvent( void* pData ) SAL_OVERRIDE;
	virtual void			SetTitle( const OUString& rTitle ) SAL_OVERRIDE;
	virtual void			SetIcon( sal_uInt16 nIcon ) SAL_OVERRIDE;
	virtual void			SetMenu( SalMenu *pSalMenu ) SAL_OVERRIDE;
	virtual void			DrawMenuBar() SAL_OVERRIDE;
	virtual void			SetExtendedFrameStyle( SalExtStyle nExtStyle ) SAL_OVERRIDE;
	virtual void			Show( bool bVisible, bool bNoActivate = false ) SAL_OVERRIDE;
	virtual void			SetMinClientSize( long nWidth, long nHeight ) SAL_OVERRIDE;
	virtual void			SetMaxClientSize( long nWidth, long nHeight ) SAL_OVERRIDE;
	virtual void			SetPosSize( long nX, long nY, long nWidth, long nHeight, sal_uInt16 nFlags ) SAL_OVERRIDE;
	virtual void			GetClientSize( long& rWidth, long& rHeight ) SAL_OVERRIDE;
	virtual void			GetWorkArea( Rectangle& rRect ) SAL_OVERRIDE;
	virtual SalFrame*		GetParent() const SAL_OVERRIDE;
	virtual void			SetWindowState( const SalFrameState* pState ) SAL_OVERRIDE;
	virtual bool			GetWindowState( SalFrameState* pState ) SAL_OVERRIDE;
	virtual void			ShowFullScreen( bool bFullScreen, sal_Int32 nDisplay ) SAL_OVERRIDE;
	virtual void			StartPresentation( bool bStart ) SAL_OVERRIDE;
	virtual void			SetAlwaysOnTop( bool bOnTop ) SAL_OVERRIDE;
	virtual void			ToTop( sal_uInt16 nFlags ) SAL_OVERRIDE;
	virtual void			SetPointer( PointerStyle ePointerStyle ) SAL_OVERRIDE;
	virtual void			CaptureMouse( bool bMouse ) SAL_OVERRIDE;
	virtual void			SetPointerPos( long nX, long nY ) SAL_OVERRIDE;
	virtual void			Flush() SAL_OVERRIDE;
	virtual void			Flush( const Rectangle& rRect ) SAL_OVERRIDE;
	virtual void			Sync() SAL_OVERRIDE;
	virtual void			SetInputContext( SalInputContext* pContext ) SAL_OVERRIDE;
	virtual void			EndExtTextInput( sal_uInt16 nFlags ) SAL_OVERRIDE;
	virtual OUString		GetKeyName( sal_uInt16 nKeyCode ) SAL_OVERRIDE;
	virtual bool			MapUnicodeToKeyCode( sal_Unicode aUnicode, LanguageType aLangType, vcl::KeyCode& rKeyCode ) SAL_OVERRIDE;
	virtual LanguageType	GetInputLanguage() SAL_OVERRIDE;
	virtual void			UpdateSettings( AllSettings& rSettings ) SAL_OVERRIDE;
	virtual void			Beep() SAL_OVERRIDE;
	virtual const SystemEnvData*	GetSystemData() const SAL_OVERRIDE;
	virtual SalPointerState	GetPointerState() SAL_OVERRIDE;
	virtual void			SetParent( SalFrame* pNewParent ) SAL_OVERRIDE;
	virtual bool			SetPluginParent( SystemParentData* pNewParent ) SAL_OVERRIDE;
	virtual void			ResetClipRegion() SAL_OVERRIDE;
	virtual void			BeginSetClipRegion( sal_uLong nRects ) SAL_OVERRIDE;
	virtual void			UnionClipRegion( long nX, long nY, long nWidth, long nHeight ) SAL_OVERRIDE;
	virtual void			EndSetClipRegion() SAL_OVERRIDE;
	virtual void			SetScreenNumber( unsigned int nScreen ) SAL_OVERRIDE;
	virtual SalIndicatorState	GetIndicatorState() SAL_OVERRIDE;
	virtual void			SimulateKeyPress( sal_uInt16 nKeyCode ) SAL_OVERRIDE;
	virtual void			SetApplicationID( const OUString& rApplicationID ) SAL_OVERRIDE;
};

// Note: this must not be static as the symbol will be loaded by the framework
// module
extern "C" SAL_DLLPUBLIC_EXPORT sal_Bool IsShowOnlyMenusWindow( vcl::Window *pWindow );
extern "C" SAL_DLLPUBLIC_EXPORT void ShowOnlyMenusForWindow( vcl::Window *pWindow, sal_Bool bShowOnlyMenus );

#endif // _SV_SALFRAME_H
