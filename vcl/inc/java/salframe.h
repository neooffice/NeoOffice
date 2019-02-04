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
	SalFrameStyleFlags		mnStyle;
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
	static const tools::Rectangle	GetScreenBounds( long nX, long nY, long nWidth, long nHeight, sal_Bool bFullScreenMode );
	static const tools::Rectangle	GetScreenBounds( unsigned int nScreen, sal_Bool bFullScreenMode );
	static unsigned int		GetScreenCount();
	static bool				UseDarkModeColors();
	static bool				GetAlternateSelectedControlTextColor( SalColor& rSalColor );
	static bool				GetControlTextColor( SalColor& rSalColor );
	static bool				GetDisabledControlTextColor( SalColor& rSalColor );
	static bool				GetSelectedControlTextColor( SalColor& rSalColor );
	static bool				GetSelectedMenuItemTextColor( SalColor& rSalColor );
	static bool				GetSelectedTabTextColor( SalColor& rSalColor );

							JavaSalFrame( SalFrameStyleFlags nSalFrameStyle, JavaSalFrame *pParent );
	virtual					~JavaSalFrame();

	void					AddObject( JavaSalObject *pObject, bool bVisible );
	bool					Deminimize();
	bool					IsFloatingFrame();
	bool					IsUtilityWindow();
	void					RemoveObject( JavaSalObject *pObject, bool bDeleted );
	void					FlushAllObjects();
	const tools::Rectangle	GetBounds( sal_Bool *pInLiveResize = NULL, sal_Bool *pInFullScreenMode = NULL, sal_Bool bUseFullScreenOriginalBounds = sal_False );
	const tools::Rectangle	GetInsets();
	id						GetNativeWindow();
	id						GetNativeWindowContentView( sal_Bool bTopLevelWindow );
	WindowStateState		GetState();
	void					MakeModal();
	bool					RequestFocus();
	void					SetState( WindowStateState nFrameState );
	void					SetVisible( bool bVisible, bool bNoActivate );
	bool					ToFront();
	void					UpdateLayer();
	void					AddTrackingRect( vcl::Window *pWindow );
	void					RemoveTrackingRect( vcl::Window *pWindow );
	void					SetMovable( bool bMoveable );

	virtual SalGraphics*	AcquireGraphics() override;
	virtual void			ReleaseGraphics( SalGraphics* pGraphics ) override;
	virtual bool			PostEvent( ImplSVEvent* pData ) override;
	virtual void			SetTitle( const OUString& rTitle ) override;
	virtual void			SetIcon( sal_uInt16 nIcon ) override;
	virtual void			SetMenu( SalMenu *pSalMenu ) override;
	virtual void			DrawMenuBar() override;
	virtual void			SetExtendedFrameStyle( SalExtStyle nExtStyle ) override;
	virtual void			Show( bool bVisible, bool bNoActivate = false ) override;
	virtual void			SetMinClientSize( long nWidth, long nHeight ) override;
	virtual void			SetMaxClientSize( long nWidth, long nHeight ) override;
	virtual void			SetPosSize( long nX, long nY, long nWidth, long nHeight, sal_uInt16 nFlags ) override;
	virtual void			GetClientSize( long& rWidth, long& rHeight ) override;
	virtual void			GetWorkArea( tools::Rectangle& rRect ) override;
	virtual SalFrame*		GetParent() const override;
	virtual void			SetWindowState( const SalFrameState* pState ) override;
	virtual bool			GetWindowState( SalFrameState* pState ) override;
	virtual void			ShowFullScreen( bool bFullScreen, sal_Int32 nDisplay ) override;
	virtual void			StartPresentation( bool bStart ) override;
	virtual void			SetAlwaysOnTop( bool bOnTop ) override;
	virtual void			ToTop( SalFrameToTop nFlags ) override;
	virtual void			SetPointer( PointerStyle ePointerStyle ) override;
	virtual void			CaptureMouse( bool bMouse ) override;
	virtual void			SetPointerPos( long nX, long nY ) override;
	virtual void			Flush() override;
	virtual void			SetInputContext( SalInputContext* pContext ) override;
	virtual void			EndExtTextInput( EndExtTextInputFlags nFlags ) override;
	virtual OUString		GetKeyName( sal_uInt16 nKeyCode ) override;
	virtual bool			MapUnicodeToKeyCode( sal_Unicode aUnicode, LanguageType aLangType, vcl::KeyCode& rKeyCode ) override;
	virtual LanguageType	GetInputLanguage() override;
	virtual void			UpdateSettings( AllSettings& rSettings ) override;
	virtual void			Beep() override;
	virtual const SystemEnvData*	GetSystemData() const override;
	virtual SalPointerState	GetPointerState() override;
	virtual void			SetParent( SalFrame* pNewParent ) override;
	virtual bool			SetPluginParent( SystemParentData* pNewParent ) override;
	virtual void			ResetClipRegion() override;
	virtual void			BeginSetClipRegion( sal_uLong nRects ) override;
	virtual void			UnionClipRegion( long nX, long nY, long nWidth, long nHeight ) override;
	virtual void			EndSetClipRegion() override;
	virtual void			SetScreenNumber( unsigned int nScreen ) override;
	virtual KeyIndicatorState	GetIndicatorState() override;
	virtual void			SimulateKeyPress( sal_uInt16 nKeyCode ) override;
	virtual void			SetApplicationID( const OUString& rApplicationID ) override;
};

// Note: this must not be static as the symbol will be loaded by the framework
// module
extern "C" SAL_DLLPUBLIC_EXPORT sal_Bool IsShowOnlyMenusWindow( vcl::Window *pWindow );
extern "C" SAL_DLLPUBLIC_EXPORT void ShowOnlyMenusForWindow( vcl::Window *pWindow, sal_Bool bShowOnlyMenus );
// Note: this must not be static as the symbol will be loaded by the svtools
// module
extern "C" SAL_DLLPUBLIC_EXPORT sal_Bool UseDarkModeColors();

#endif // _SV_SALFRAME_H
