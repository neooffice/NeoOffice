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
#include <vcl/sv.h>
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
	sal_Bool				mbVisible;
	::std::list< JavaSalFrame* >	maChildren;
	SystemEnvData			maSysData;
	sal_Bool				mbCenter;
	SalFrameGeometry		maOriginalGeometry;
	sal_Bool				mbFullScreen;
	sal_Bool				mbPresentation;
	JavaSalMenu*			mpMenuBar;
	sal_Bool				mbInSetPosSize;
	sal_Bool				mbInShow;
	::std::list< JavaSalObject* >	maObjects;
	::std::list< JavaSalObject* >	maVisibleObjects;
	::rtl::OUString			maTitle;
	sal_Bool				mbShowOnlyMenus;
	sal_Bool				mbInShowOnlyMenus;
	::std::list< JavaSalMenu* >	maUpdateMenuList;
	sal_Bool				mbInShowFullScreen;
	sal_Bool				mbInWindowDidExitFullScreen;
	sal_Bool				mbInWindowWillEnterFullScreen;
	sal_Bool				mbInSetWindowState;

	static ::rtl::OUString	ConvertVCLKeyCode( sal_uInt16 nKeyCode, bool bIsMenuShortcut );
	static void				FlushAllFrames();
	static unsigned int		GetDefaultScreenNumber();
	static const Rectangle	GetScreenBounds( long nX, long nY, long nWidth, long nHeight, sal_Bool bFullScreenMode );
	static const Rectangle	GetScreenBounds( unsigned int nScreen, sal_Bool bFullScreenMode );
	static unsigned int		GetScreenCount();
	static sal_Bool			GetAlternateSelectedControlTextColor( SalColor& rSalColor );
	static sal_Bool			GetControlTextColor( SalColor& rSalColor );
	static sal_Bool			GetDisabledControlTextColor( SalColor& rSalColor );
	static sal_Bool			GetSelectedControlTextColor( SalColor& rSalColor );
	static sal_Bool			GetSelectedMenuItemTextColor( SalColor& rSalColor );

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
	void					SetVisible( sal_Bool bVisible, sal_Bool bNoActivate );
	bool					ToFront();
	void					UpdateLayer();
	void					AddTrackingRect( Window *pWindow );
	void					RemoveTrackingRect( Window *pWindow );

	virtual SalGraphics*	GetGraphics();
	virtual void			ReleaseGraphics( SalGraphics* pGraphics );
	virtual sal_Bool		PostEvent( void* pData );
	virtual void			SetTitle( const XubString& rTitle );
	virtual void			SetIcon( sal_uInt16 nIcon );
	virtual void			SetMenu( SalMenu *pSalMenu );
	virtual void			DrawMenuBar();
	virtual void			SetExtendedFrameStyle( SalExtStyle nExtStyle );
	virtual void			Show( sal_Bool bVisible, sal_Bool bNoActivate = sal_False );
	virtual void			Enable( sal_Bool bEnable );
	virtual void			SetMinClientSize( long nWidth, long nHeight );
	virtual void			SetMaxClientSize( long nWidth, long nHeight );
	virtual void			SetPosSize( long nX, long nY, long nWidth, long nHeight, sal_uInt16 nFlags );
	virtual void			GetClientSize( long& rWidth, long& rHeight );
	virtual void			GetWorkArea( Rectangle& rRect );
	virtual SalFrame*		GetParent() const;
	virtual void			SetWindowState( const SalFrameState* pState );
	virtual sal_Bool		GetWindowState( SalFrameState* pState );
	virtual void			ShowFullScreen( sal_Bool bFullScreen, sal_Int32 nDisplay );
	virtual void			StartPresentation( sal_Bool bStart );
	virtual void			SetAlwaysOnTop( sal_Bool bOnTop );
	virtual void			ToTop( sal_uInt16 nFlags );
	virtual void			SetPointer( PointerStyle ePointerStyle );
	virtual void			CaptureMouse( sal_Bool bMouse );
	virtual void			SetPointerPos( long nX, long nY );
	virtual void			Flush();
	virtual void			Sync();
	virtual void			SetInputContext( SalInputContext* pContext );
	virtual void			EndExtTextInput( sal_uInt16 nFlags );
	virtual String			GetKeyName( sal_uInt16 nKeyCode );
	virtual String			GetSymbolKeyName( const XubString& rFontName, sal_uInt16 nKeyCode );
	virtual sal_Bool		MapUnicodeToKeyCode( sal_Unicode aUnicode, LanguageType aLangType, KeyCode& rKeyCode );
	virtual LanguageType	GetInputLanguage();
	virtual SalBitmap*		SnapShot();
	virtual void			UpdateSettings( AllSettings& rSettings );
	virtual void			Beep( SoundType eSoundType );
	virtual const SystemEnvData*	GetSystemData() const;
	virtual void			SetBackgroundBitmap( SalBitmap* );
	virtual SalPointerState	GetPointerState();
	virtual void			SetParent( SalFrame* pNewParent );
	virtual bool			SetPluginParent( SystemParentData* pNewParent );
	virtual void			ResetClipRegion();
	virtual void			BeginSetClipRegion( sal_uLong nRects );
	virtual void			UnionClipRegion( long nX, long nY, long nWidth, long nHeight );
	virtual void			EndSetClipRegion();
	virtual void			SetScreenNumber( unsigned int nScreen );
};

// Note: this must not be static as the symbol will be loaded by the framework
// module
extern "C" SAL_DLLPUBLIC_EXPORT sal_Bool IsShowOnlyMenusWindow( Window *pWindow );
extern "C" SAL_DLLPUBLIC_EXPORT void ShowOnlyMenusForWindow( Window *pWindow, sal_Bool bShowOnlyMenus );

#endif // _SV_SALFRAME_H
