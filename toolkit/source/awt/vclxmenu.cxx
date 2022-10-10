/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 * 
 *   Modified August 2022 by Patrick Luby. NeoOffice is only distributed
 *   under the GNU General Public License, Version 3 as allowed by Section 3.3
 *   of the Mozilla Public License, v. 2.0.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <toolkit/awt/vclxmenu.hxx>
#include <toolkit/helper/convert.hxx>
#include <toolkit/helper/macros.hxx>
#include <toolkit/helper/servicenames.hxx>
#include <toolkit/helper/vclunohelper.hxx>

#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <rtl/uuid.h>
#include <osl/mutex.hxx>

#include <vcl/menu.hxx>
#include <vcl/keycod.hxx>
#include <vcl/image.hxx>
#include <vcl/mnemonic.hxx>
#include <vcl/svapp.hxx>

#include <com/sun/star/awt/KeyModifier.hpp>

VCLXMenu::VCLXMenu()
    : maMenuListeners( *this )
{
    mpMenu = NULL;
}

VCLXMenu::VCLXMenu( Menu* pMenu )
    : maMenuListeners( *this )
{
    mpMenu = pMenu;
}

VCLXMenu::~VCLXMenu()
{
    for ( size_t n = maPopupMenuRefs.size(); n; ) {
        delete maPopupMenuRefs[ --n ];
    }
    if ( mpMenu )
    {
        mpMenu->RemoveEventListener( LINK( this, VCLXMenu, MenuEventListener ) );
        delete mpMenu;
    }
}

bool VCLXMenu::IsPopupMenu() const
{
    return (mpMenu && ! mpMenu->IsMenuBar());
}

void VCLXMenu::ImplCreateMenu( bool bPopup )
{
    DBG_ASSERT( !mpMenu, "CreateMenu: Menu exists!" );

    if ( bPopup )
        mpMenu = new PopupMenu;
    else
        mpMenu = new MenuBar;

    mpMenu->AddEventListener( LINK( this, VCLXMenu, MenuEventListener ) );
}

IMPL_LINK( VCLXMenu, MenuEventListener, VclSimpleEvent*, pEvent )
{
    DBG_ASSERT( pEvent && pEvent->ISA( VclMenuEvent ), "Unknown Event!" );
    if ( pEvent && pEvent->ISA( VclMenuEvent ) )
    {
        DBG_ASSERT( static_cast<VclMenuEvent*>(pEvent)->GetMenu() && mpMenu, "Menu???" );

        VclMenuEvent* pMenuEvent = static_cast<VclMenuEvent*>(pEvent);
        if ( pMenuEvent->GetMenu() == mpMenu )  // Also called for the root menu
        {
            switch ( pMenuEvent->GetId() )
            {
                case VCLEVENT_MENU_SELECT:
                {
                    if ( maMenuListeners.getLength() )
                    {
                        css::awt::MenuEvent aEvent;
                        aEvent.Source = (::cppu::OWeakObject*)this;
                        aEvent.MenuId = mpMenu->GetCurItemId();
                        maMenuListeners.itemSelected( aEvent );
                    }
                }
                break;
                case VCLEVENT_OBJECT_DYING:
                {
                    mpMenu = NULL;
                }
                break;
                case VCLEVENT_MENU_HIGHLIGHT:
                {
                    if ( maMenuListeners.getLength() )
                    {
                        css::awt::MenuEvent aEvent;
                        aEvent.Source = (::cppu::OWeakObject*)this;
                        aEvent.MenuId = mpMenu->GetCurItemId();
                        maMenuListeners.itemHighlighted( aEvent );
                    }
                }
                break;
                case VCLEVENT_MENU_ACTIVATE:
                {
                    if ( maMenuListeners.getLength() )
                    {
                        css::awt::MenuEvent aEvent;
                        aEvent.Source = (::cppu::OWeakObject*)this;
                        aEvent.MenuId = mpMenu->GetCurItemId();
                        maMenuListeners.itemActivated( aEvent );
                    }
                }
                break;
                case VCLEVENT_MENU_DEACTIVATE:
                {
                    if ( maMenuListeners.getLength() )
                    {
                        css::awt::MenuEvent aEvent;
                        aEvent.Source = (::cppu::OWeakObject*)this;
                        aEvent.MenuId = mpMenu->GetCurItemId();
                        maMenuListeners.itemDeactivated( aEvent );
                    }
                }
                break;

                // ignore accessibility events
                case VCLEVENT_MENU_ENABLE:
                case VCLEVENT_MENU_INSERTITEM:
                case VCLEVENT_MENU_REMOVEITEM:
                case VCLEVENT_MENU_SUBMENUACTIVATE:
                case VCLEVENT_MENU_SUBMENUDEACTIVATE:
                case VCLEVENT_MENU_SUBMENUCHANGED:
                case VCLEVENT_MENU_DEHIGHLIGHT:
                case VCLEVENT_MENU_DISABLE:
                case VCLEVENT_MENU_ITEMTEXTCHANGED:
                case VCLEVENT_MENU_ITEMCHECKED:
                case VCLEVENT_MENU_ITEMUNCHECKED:
                case VCLEVENT_MENU_SHOW:
                case VCLEVENT_MENU_HIDE:
                break;

                default:    OSL_FAIL( "MenuEventListener - Unknown event!" );
           }
       }
    }
    return 0;
}


OUString SAL_CALL VCLXMenu::getImplementationName(  )
throw (css::uno::RuntimeException, std::exception)
{
    ::osl::ResettableGuard < ::osl::Mutex > aGuard( GetMutex() );
    const bool bIsPopupMenu = IsPopupMenu();
    aGuard.clear();

    OUString implName( "stardiv.Toolkit." );
    if ( bIsPopupMenu )
        implName += "VCLXPopupMenu";
    else
        implName += "VCLXMenuBar";

    return implName;
}

css::uno::Sequence< OUString > SAL_CALL VCLXMenu::getSupportedServiceNames(  )
throw (css::uno::RuntimeException, std::exception)
{
    ::osl::ResettableGuard < ::osl::Mutex > aGuard( GetMutex() );
    const bool bIsPopupMenu = IsPopupMenu();
    aGuard.clear();

    css::uno::Sequence< OUString > aNames( 1 );
    if ( bIsPopupMenu )
        aNames[ 0 ] = OUString::createFromAscii( szServiceName2_PopupMenu );
    else
        aNames[ 0 ] = OUString::createFromAscii( szServiceName2_MenuBar );

    return aNames;
}

sal_Bool SAL_CALL VCLXMenu::supportsService(const OUString& rServiceName )
throw (css::uno::RuntimeException, std::exception)
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Any VCLXMenu::queryInterface(
    const css::uno::Type & rType )
throw(css::uno::RuntimeException, std::exception)
{
    ::osl::ResettableGuard < ::osl::Mutex > aGuard( GetMutex() );
    const bool bIsPopupMenu = IsPopupMenu();
    aGuard.clear();

    css::uno::Any aRet;

    if ( bIsPopupMenu )
        aRet = ::cppu::queryInterface(  rType,
                                        (static_cast< css::awt::XMenu* >((css::awt::XMenuBar*) this) ),
                                        (static_cast< css::awt::XPopupMenu* >(this)),
                                        (static_cast< css::lang::XTypeProvider* >(this)),
                                        (static_cast< css::lang::XServiceInfo* >(this)),
                                        (static_cast< css::lang::XUnoTunnel* >(this)) );
    else
        aRet = ::cppu::queryInterface(  rType,
                                        (static_cast< css::awt::XMenu* >((css::awt::XMenuBar*) this) ),
                                        (static_cast< css::awt::XMenuBar* >(this)),
                                        (static_cast< css::lang::XTypeProvider* >(this)),
                                        (static_cast< css::lang::XServiceInfo* >(this)),
                                        (static_cast< css::lang::XUnoTunnel* >(this)) );

    return (aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType ));
}


IMPL_XUNOTUNNEL( VCLXMenu )

css::uno::Sequence< css::uno::Type > VCLXMenu::getTypes()
throw(css::uno::RuntimeException, std::exception)
{
    ::osl::ResettableGuard < ::osl::Mutex > aGuard( GetMutex() );
    const bool bIsPopupMenu = IsPopupMenu();
    aGuard.clear();

    static ::cppu::OTypeCollection* pCollectionMenuBar = NULL;
    static ::cppu::OTypeCollection* pCollectionPopupMenu = NULL;

    if ( bIsPopupMenu )
    {
        if( !pCollectionPopupMenu )
        {
            ::osl::Guard< ::osl::Mutex > aGlobalGuard( ::osl::Mutex::getGlobalMutex() );
            if( !pCollectionPopupMenu )
            {
                static ::cppu::OTypeCollection collectionPopupMenu(
                cppu::UnoType<css::lang::XTypeProvider>::get(),
                cppu::UnoType<css::awt::XMenu>::get(),
                cppu::UnoType<css::awt::XPopupMenu>::get(),
                cppu::UnoType<css::lang::XServiceInfo>::get());
                pCollectionPopupMenu = &collectionPopupMenu;
            }
        }

        return (*pCollectionPopupMenu).getTypes();
    }
    else
    {
        if( !pCollectionMenuBar )
        {
            ::osl::Guard< ::osl::Mutex > aGlobalGuard( ::osl::Mutex::getGlobalMutex() );
            if( !pCollectionMenuBar )
            {
                static ::cppu::OTypeCollection collectionMenuBar(
                cppu::UnoType<css::lang::XTypeProvider>::get(),
                cppu::UnoType<css::awt::XMenu>::get(),
                cppu::UnoType<css::awt::XMenuBar>::get(),
                cppu::UnoType<css::lang::XServiceInfo>::get());
                pCollectionMenuBar = &collectionMenuBar;
            }
        }
        return (*pCollectionMenuBar).getTypes();
    }
}


css::uno::Sequence< sal_Int8 > VCLXMenu::getImplementationId()
throw(css::uno::RuntimeException, std::exception)
{
    return css::uno::Sequence<sal_Int8>();
}

void VCLXMenu::addMenuListener(
    const css::uno::Reference< css::awt::XMenuListener >& rxListener )
throw(css::uno::RuntimeException, std::exception)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    maMenuListeners.addInterface( rxListener );
}

void VCLXMenu::removeMenuListener(
    const css::uno::Reference< css::awt::XMenuListener >& rxListener )
throw(css::uno::RuntimeException, std::exception)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    maMenuListeners.removeInterface( rxListener );
}

void VCLXMenu::insertItem(
    sal_Int16 nItemId,
    const OUString& aText,
    sal_Int16 nItemStyle,
    sal_Int16 nPos )
throw(css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aSolarGuard;
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    if ( mpMenu )
        mpMenu->InsertItem(nItemId, aText, (MenuItemBits)nItemStyle, OString(), nPos);
}

void VCLXMenu::removeItem(
    sal_Int16 nPos,
    sal_Int16 nCount )
throw(css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aSolarGuard;
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    if (!mpMenu)
        return;

    sal_Int32 nItemCount = (sal_Int32)mpMenu->GetItemCount();
    if ( ( nCount > 0 ) && ( nPos >= 0 ) && ( nPos < nItemCount ) && ( nItemCount > 0 ))
    {
        sal_Int16 nP = sal::static_int_cast< sal_Int16 >(
            std::min( (int)(nPos+nCount), (int)nItemCount ));
        while( nP-nPos > 0 )
            mpMenu->RemoveItem( --nP );
    }
}

sal_Int16 VCLXMenu::getItemCount(  )
throw(css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aSolarGuard;
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    return mpMenu ? mpMenu->GetItemCount() : 0;
}

sal_Int16 VCLXMenu::getItemId(
    sal_Int16 nPos )
throw(css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aSolarGuard;
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    return mpMenu ? mpMenu->GetItemId( nPos ) : 0;
}

sal_Int16 VCLXMenu::getItemPos(
    sal_Int16 nId )
throw(css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aSolarGuard;
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    return mpMenu ? mpMenu->GetItemPos( nId ) : 0;
}

void VCLXMenu::enableItem(
    sal_Int16 nItemId,
    sal_Bool bEnable )
throw(css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aSolarGuard;
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    if ( mpMenu )
        mpMenu->EnableItem( nItemId, bEnable );
}

sal_Bool VCLXMenu::isItemEnabled(
    sal_Int16 nItemId )
throw(css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aSolarGuard;
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    return mpMenu ? mpMenu->IsItemEnabled( nItemId ) : sal_False;
}

void VCLXMenu::setItemText(
    sal_Int16 nItemId,
    const OUString& aText )
throw(css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aSolarGuard;
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    if ( mpMenu )
        mpMenu->SetItemText( nItemId, aText );
}

OUString VCLXMenu::getItemText(
    sal_Int16 nItemId )
throw(css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aSolarGuard;
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    OUString aItemText;
    if ( mpMenu )
        aItemText = mpMenu->GetItemText( nItemId );
    return aItemText;
}

void VCLXMenu::setPopupMenu(
    sal_Int16 nItemId,
    const css::uno::Reference< css::awt::XPopupMenu >& rxPopupMenu )
throw(css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aSolarGuard;
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    VCLXMenu* pVCLMenu = VCLXMenu::GetImplementation( rxPopupMenu );
    DBG_ASSERT( pVCLMenu && pVCLMenu->GetMenu() && pVCLMenu->IsPopupMenu(), "setPopupMenu: Invalid Menu!" );

    if ( mpMenu && pVCLMenu && pVCLMenu->GetMenu() && pVCLMenu->IsPopupMenu() )
    {
        // Selbst eine Ref halten!
        css::uno::Reference< css::awt::XPopupMenu > * pNewRef = new css::uno::Reference< css::awt::XPopupMenu > ;
        *pNewRef = rxPopupMenu;
        maPopupMenuRefs.push_back( pNewRef );

        mpMenu->SetPopupMenu( nItemId, static_cast<PopupMenu*>( pVCLMenu->GetMenu() ) );
    }
}

css::uno::Reference< css::awt::XPopupMenu > VCLXMenu::getPopupMenu(
    sal_Int16 nItemId )
throw(css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aSolarGuard;
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    css::uno::Reference< css::awt::XPopupMenu >  aRef;
    Menu* pMenu = mpMenu ? mpMenu->GetPopupMenu( nItemId ) : NULL;
    if ( pMenu )
    {
        for ( size_t n = maPopupMenuRefs.size(); n; )
        {
            css::uno::Reference< css::awt::XPopupMenu > * pRef = maPopupMenuRefs[ --n ];
            Menu* pM = static_cast<VCLXMenu*>(pRef->get())->GetMenu();
            if ( pM == pMenu )
            {
                aRef = *pRef;
                break;
            }
        }
        // it seems the popup menu is not insert into maPopupMenuRefs
        // if the popup men is not created by stardiv.Toolkit.VCLXPopupMenu
        if( !aRef.is() )
        {
            aRef = new VCLXPopupMenu( static_cast<PopupMenu*>(pMenu) );
        }
    }
    return aRef;
}

// css::awt::XPopupMenu
void VCLXMenu::insertSeparator(
    sal_Int16 nPos )
throw(css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aSolarGuard;
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    if ( mpMenu )
        mpMenu->InsertSeparator(OString(), nPos);
}

void VCLXMenu::setDefaultItem(
    sal_Int16 nItemId )
throw(css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aSolarGuard;
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    if ( mpMenu )
        mpMenu->SetDefaultItem( nItemId );
}

sal_Int16 VCLXMenu::getDefaultItem(  )
throw(css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aSolarGuard;
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    return mpMenu ? mpMenu->GetDefaultItem() : 0;
}

void VCLXMenu::checkItem(
    sal_Int16 nItemId,
    sal_Bool bCheck )
throw(css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aSolarGuard;
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    if ( mpMenu )
        mpMenu->CheckItem( nItemId, bCheck );
}

sal_Bool VCLXMenu::isItemChecked(
    sal_Int16 nItemId )
throw(css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aSolarGuard;
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    return mpMenu ? mpMenu->IsItemChecked( nItemId ) : sal_False;
}

sal_Int16 VCLXMenu::execute(
    const css::uno::Reference< css::awt::XWindowPeer >& rxWindowPeer,
    const css::awt::Rectangle& rPos,
    sal_Int16 nFlags )
throw(css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aSolarGuard;
#ifdef USE_JAVA
    ::osl::ClearableGuard< ::osl::Mutex > aGuard( GetMutex() );
#else	// USE_JAVA
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );
#endif	// USE_JAVA

    sal_Int16 nRet = 0;
    if ( mpMenu && IsPopupMenu() )
    {
#ifdef USE_JAVA
        // Fix deadlock when selecting an item in the New toolbar button's
        // menu by clearing the guard
        aGuard.clear();
#endif	// USE_JAVA
        nRet = static_cast<PopupMenu*>(mpMenu)->Execute( VCLUnoHelper::GetWindow( rxWindowPeer ),
                                              VCLRectangle( rPos ),
                                              nFlags | POPUPMENU_NOMOUSEUPCLOSE );
    }
    return nRet;
}


void SAL_CALL VCLXMenu::setCommand(
    sal_Int16 nItemId,
    const OUString& aCommand )
throw (css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aSolarGuard;
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    if ( mpMenu )
        mpMenu->SetItemCommand( nItemId, aCommand );
}

OUString SAL_CALL VCLXMenu::getCommand(
    sal_Int16 nItemId )
throw (css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aSolarGuard;
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    OUString aItemCommand;
    if ( mpMenu )
        aItemCommand = mpMenu->GetItemCommand( nItemId );
    return aItemCommand;
}

void SAL_CALL VCLXMenu::setHelpCommand(
    sal_Int16 nItemId,
    const OUString& aHelp )
throw (css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aSolarGuard;
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    if ( mpMenu )
        mpMenu->SetHelpCommand( nItemId, aHelp );
}

OUString SAL_CALL VCLXMenu::getHelpCommand(
    sal_Int16 nItemId )
throw (css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aSolarGuard;
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    OUString aHelpCommand;
    if ( mpMenu )
        aHelpCommand = mpMenu->GetHelpCommand( nItemId );
    return aHelpCommand;
}


namespace
{
    static Image lcl_XGraphic2VCLImage(
        const css::uno::Reference< css::graphic::XGraphic >& xGraphic,
        bool bResize )
    {
        Image aImage;
        if ( !xGraphic.is() )
            return aImage;

        aImage = Image( xGraphic );
        const ::Size aCurSize = aImage.GetSizePixel();
        const sal_Int32 nCurWidth = aCurSize.Width();
        const sal_Int32 nCurHeight = aCurSize.Height();
        const sal_Int32 nIdeal( 16 );

        if ( nCurWidth > 0 && nCurHeight > 0 )
        {
            if ( bResize && ( nCurWidth > nIdeal || nCurHeight > nIdeal ) )
            {
                sal_Int32 nIdealWidth  = nCurWidth  > nIdeal ? nIdeal : nCurWidth;
                sal_Int32 nIdealHeight = nCurHeight > nIdeal ? nIdeal : nCurHeight;

                ::Size aNewSize( nIdealWidth, nIdealHeight );

                bool bModified( false );
                BitmapEx aBitmapEx = aImage.GetBitmapEx();
                bModified = aBitmapEx.Scale( aNewSize, BMP_SCALE_BESTQUALITY );

                if ( bModified )
                    aImage = Image( aBitmapEx );
            }
        }
        return aImage;
    }

    /** Copied from svtools/inc/acceleratorexecute.hxx */
    static css::awt::KeyEvent lcl_VCLKey2AWTKey(
        const vcl::KeyCode& aVCLKey)
    {
        css::awt::KeyEvent aAWTKey;
        aAWTKey.Modifiers = 0;
        aAWTKey.KeyCode   = (sal_Int16)aVCLKey.GetCode();

        if (aVCLKey.IsShift())
            aAWTKey.Modifiers |= css::awt::KeyModifier::SHIFT;
        if (aVCLKey.IsMod1())
            aAWTKey.Modifiers |= css::awt::KeyModifier::MOD1;
        if (aVCLKey.IsMod2())
            aAWTKey.Modifiers |= css::awt::KeyModifier::MOD2;
        if (aVCLKey.IsMod3())
            aAWTKey.Modifiers |= css::awt::KeyModifier::MOD3;

        return aAWTKey;
    }

    vcl::KeyCode lcl_AWTKey2VCLKey(const css::awt::KeyEvent& aAWTKey)
    {
        bool bShift = ((aAWTKey.Modifiers & css::awt::KeyModifier::SHIFT) == css::awt::KeyModifier::SHIFT );
        bool bMod1  = ((aAWTKey.Modifiers & css::awt::KeyModifier::MOD1 ) == css::awt::KeyModifier::MOD1  );
        bool bMod2  = ((aAWTKey.Modifiers & css::awt::KeyModifier::MOD2 ) == css::awt::KeyModifier::MOD2  );
        bool bMod3  = ((aAWTKey.Modifiers & css::awt::KeyModifier::MOD3 ) == css::awt::KeyModifier::MOD3  );
        sal_uInt16   nKey   = (sal_uInt16)aAWTKey.KeyCode;

        return vcl::KeyCode(nKey, bShift, bMod1, bMod2, bMod3);
    }

}


sal_Bool SAL_CALL VCLXMenu::isPopupMenu(  )
throw (css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aSolarGuard;
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );
    return IsPopupMenu();
}

void SAL_CALL VCLXMenu::clear(  )
throw (css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aSolarGuard;
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );
    if ( mpMenu )
        mpMenu->Clear();
}


css::awt::MenuItemType SAL_CALL VCLXMenu::getItemType(
    ::sal_Int16 nItemPos )
throw (css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aSolarGuard;
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    css::awt::MenuItemType aMenuItemType =
        css::awt::MenuItemType_DONTKNOW;
    if ( mpMenu )
    {
        aMenuItemType = ( (css::awt::MenuItemType) mpMenu->GetItemType( nItemPos ) );
    }

    return aMenuItemType;
}

void SAL_CALL VCLXMenu::hideDisabledEntries(
    sal_Bool bHide )
throw (css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aSolarGuard;
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );
    if ( mpMenu )
    {
        if ( bHide )
            mpMenu->SetMenuFlags( mpMenu->GetMenuFlags() | MENU_FLAG_HIDEDISABLEDENTRIES );
        else
            mpMenu->SetMenuFlags( mpMenu->GetMenuFlags() & ~MENU_FLAG_HIDEDISABLEDENTRIES );
    }
}


sal_Bool SAL_CALL VCLXMenu::isInExecute(  )
throw (css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aSolarGuard;
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    if ( mpMenu && IsPopupMenu() )
        return PopupMenu::IsInExecute();
    else
        return sal_False;
}


void SAL_CALL VCLXMenu::endExecute()
throw (css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aSolarGuard;
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    if ( mpMenu && IsPopupMenu() )
        static_cast<PopupMenu*>( mpMenu )->EndExecute();
}


void SAL_CALL VCLXMenu::enableAutoMnemonics(
    sal_Bool bEnable )
throw (css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aSolarGuard;
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );
    if ( mpMenu )
    {
        if ( !bEnable )
            mpMenu->SetMenuFlags( mpMenu->GetMenuFlags() | MENU_FLAG_NOAUTOMNEMONICS );
        else
            mpMenu->SetMenuFlags( mpMenu->GetMenuFlags() & ~MENU_FLAG_NOAUTOMNEMONICS );
    }
}


void SAL_CALL VCLXMenu::setAcceleratorKeyEvent(
    ::sal_Int16 nItemId,
    const css::awt::KeyEvent& aKeyEvent )
throw (css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aSolarGuard;
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    if ( mpMenu && IsPopupMenu() && MENU_ITEM_NOTFOUND != mpMenu->GetItemPos( nItemId ) )
    {
        vcl::KeyCode aVCLKeyCode = lcl_AWTKey2VCLKey( aKeyEvent );
        mpMenu->SetAccelKey( nItemId, aVCLKeyCode );
    }
}


css::awt::KeyEvent SAL_CALL VCLXMenu::getAcceleratorKeyEvent(
    ::sal_Int16 nItemId )
throw (css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aSolarGuard;
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    css::awt::KeyEvent aKeyEvent;
    if ( mpMenu && IsPopupMenu() && MENU_ITEM_NOTFOUND != mpMenu->GetItemPos( nItemId ) )
    {
        vcl::KeyCode nKeyCode = mpMenu->GetAccelKey( nItemId );
        aKeyEvent = lcl_VCLKey2AWTKey( nKeyCode );
    }

    return aKeyEvent;
}


void SAL_CALL VCLXMenu::setHelpText(
    ::sal_Int16 nItemId,
    const OUString& sHelpText )
throw (css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aSolarGuard;
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    if ( mpMenu && MENU_ITEM_NOTFOUND != mpMenu->GetItemPos( nItemId ) )
    {
        mpMenu->SetHelpText( nItemId, sHelpText );
    }
}


OUString SAL_CALL VCLXMenu::getHelpText(
    ::sal_Int16 nItemId )
throw (css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aSolarGuard;
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    OUString sHelpText;
    if ( mpMenu && MENU_ITEM_NOTFOUND != mpMenu->GetItemPos( nItemId ) )
    {
        sHelpText = mpMenu->GetHelpText( nItemId );
    }

    return sHelpText;
}


void SAL_CALL VCLXMenu::setTipHelpText(
    ::sal_Int16 nItemId,
    const OUString& sTipHelpText )
throw (css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aSolarGuard;
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    if ( mpMenu && MENU_ITEM_NOTFOUND != mpMenu->GetItemPos( nItemId ) )
    {
        mpMenu->SetTipHelpText( nItemId, sTipHelpText );
    }
}


OUString SAL_CALL VCLXMenu::getTipHelpText(
    ::sal_Int16 nItemId )
throw (css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aSolarGuard;
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    OUString sTipHelpText;
    if ( mpMenu && MENU_ITEM_NOTFOUND != mpMenu->GetItemPos( nItemId ) )
    {
        sTipHelpText = mpMenu->GetTipHelpText( nItemId );
    }
    return sTipHelpText;
}


void SAL_CALL VCLXMenu::setItemImage(
    ::sal_Int16 nItemId,
    const css::uno::Reference< css::graphic::XGraphic >& xGraphic,
    sal_Bool bScale )
throw (css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aSolarGuard;
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    if ( mpMenu && IsPopupMenu() && MENU_ITEM_NOTFOUND != mpMenu->GetItemPos( nItemId ) )
    {
        Image aImage = lcl_XGraphic2VCLImage( xGraphic, bScale );
        mpMenu->SetItemImage( nItemId, aImage );
    }
}


css::uno::Reference< css::graphic::XGraphic > SAL_CALL
VCLXMenu::getItemImage(
    ::sal_Int16 nItemId )
throw (css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aSolarGuard;
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    css::uno::Reference< css::graphic::XGraphic > rxGraphic;

    if ( mpMenu && IsPopupMenu() && MENU_ITEM_NOTFOUND != mpMenu->GetItemPos( nItemId ) )
    {
        Image aImage = mpMenu->GetItemImage( nItemId );
        if ( !!aImage )
            rxGraphic = aImage.GetXGraphic();
    }
    return rxGraphic;
}

VCLXMenuBar::VCLXMenuBar()
{
    ImplCreateMenu( false );
}

VCLXMenuBar::VCLXMenuBar( MenuBar* pMenuBar ) : VCLXMenu( (Menu *)pMenuBar )
{
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
stardiv_Toolkit_VCLXMenuBar_get_implementation(
    css::uno::XComponentContext *,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new VCLXMenuBar());
}

VCLXPopupMenu::VCLXPopupMenu()
{
    ImplCreateMenu( true );
}

VCLXPopupMenu::VCLXPopupMenu( PopupMenu* pPopMenu ) : VCLXMenu( (Menu *)pPopMenu )
{
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
stardiv_Toolkit_VCLXPopupMenu_get_implementation(
    css::uno::XComponentContext *,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new VCLXPopupMenu());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
