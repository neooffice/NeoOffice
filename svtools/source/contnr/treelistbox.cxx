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
 *   Modified November 2016 by Patrick Luby. NeoOffice is only distributed
 *   under the GNU General Public License, Version 3 as allowed by Section 3.3
 *   of the Mozilla Public License, v. 2.0.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
    TODO:
        - delete anchor in SelectionEngine when selecting manually
        - SelectAll( false ) => only repaint the delselected entries
*/

#include <svtools/treelistbox.hxx>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <vcl/svapp.hxx>
#include <vcl/accel.hxx>
#include <vcl/i18nhelp.hxx>
#include <vcl/builder.hxx>
#include <vcl/settings.hxx>
#include <sot/formats.hxx>
#include <unotools/accessiblestatesethelper.hxx>
#include <rtl/instance.hxx>
#include <comphelper/string.hxx>

#include <svtools/svmedit.hxx>
#include <svtools/svlbitm.hxx>
#include <svtools/treelistentry.hxx>
#include <svtools/viewdataentry.hxx>
#include "svimpbox.hxx"

#include <set>
#include <string.h>
#include <vector>

using namespace ::com::sun::star::accessibility;

// Drag&Drop
static SvTreeListBox* pDDSource = NULL;
static SvTreeListBox* pDDTarget = NULL;


#define SVLBOX_ACC_RETURN 1
#define SVLBOX_ACC_ESCAPE 2

// ***************************************************************

class MyEdit_Impl : public Edit
{
    SvInplaceEdit2* pOwner;
public:
                 MyEdit_Impl( vcl::Window* pParent, SvInplaceEdit2* pOwner );
    virtual void KeyInput( const KeyEvent& rKEvt ) SAL_OVERRIDE;
    virtual void LoseFocus() SAL_OVERRIDE;
};

class MyMultiEdit_Impl : public MultiLineEdit
{
    SvInplaceEdit2* pOwner;
public:
                 MyMultiEdit_Impl( vcl::Window* pParent, SvInplaceEdit2* pOwner );
    virtual void KeyInput( const KeyEvent& rKEvt ) SAL_OVERRIDE;
    virtual void LoseFocus() SAL_OVERRIDE;
};

MyEdit_Impl::MyEdit_Impl( vcl::Window* pParent, SvInplaceEdit2* _pOwner ) :

    Edit( pParent, WB_LEFT ),

    pOwner( _pOwner )

{
}

void MyEdit_Impl::KeyInput( const KeyEvent& rKEvt )
{
    if( !pOwner->KeyInput( rKEvt ))
        Edit::KeyInput( rKEvt );
}

void MyEdit_Impl::LoseFocus()
{
    pOwner->LoseFocus();
}

MyMultiEdit_Impl::MyMultiEdit_Impl( vcl::Window* pParent, SvInplaceEdit2* _pOwner )
    : MultiLineEdit( pParent,
    WB_CENTER
    ), pOwner(_pOwner)
{
}

void MyMultiEdit_Impl::KeyInput( const KeyEvent& rKEvt )
{
    if( !pOwner->KeyInput( rKEvt ))
        MultiLineEdit::KeyInput( rKEvt );
}

void MyMultiEdit_Impl::LoseFocus()
{
    pOwner->LoseFocus();
}


SvInplaceEdit2::SvInplaceEdit2
(
    vcl::Window* pParent, const Point& rPos,
    const Size& rSize,
    const OUString& rData,
    const Link& rNotifyEditEnd,
    const Selection& rSelection,
    bool bMulti
) :

     aCallBackHdl       ( rNotifyEditEnd ),
    bCanceled           ( false ),
    bAlreadyInCallBack  ( false )

{

    if( bMulti )
        pEdit = new MyMultiEdit_Impl( pParent, this );
    else
        pEdit = new MyEdit_Impl( pParent, this );

    vcl::Font aFont( pParent->GetFont() );
    aFont.SetTransparent( false );
    Color aColor( pParent->GetBackground().GetColor() );
    aFont.SetFillColor(aColor );
    pEdit->SetFont( aFont );
    pEdit->SetBackground( pParent->GetBackground() );
    pEdit->SetPosPixel( rPos );
    pEdit->SetSizePixel( rSize );
    pEdit->SetText( rData );
    pEdit->SetSelection( rSelection );
    pEdit->SaveValue();

    aAccReturn.InsertItem( SVLBOX_ACC_RETURN, vcl::KeyCode(KEY_RETURN) );
    aAccEscape.InsertItem( SVLBOX_ACC_ESCAPE, vcl::KeyCode(KEY_ESCAPE) );

    aAccReturn.SetActivateHdl( LINK( this, SvInplaceEdit2, ReturnHdl_Impl) );
    aAccEscape.SetActivateHdl( LINK( this, SvInplaceEdit2, EscapeHdl_Impl) );
    Application::InsertAccel( &aAccReturn );
    Application::InsertAccel( &aAccEscape );

    pEdit->Show();
    pEdit->GrabFocus();
}

SvInplaceEdit2::~SvInplaceEdit2()
{
    if( !bAlreadyInCallBack )
    {
        Application::RemoveAccel( &aAccReturn );
        Application::RemoveAccel( &aAccEscape );
    }
    delete pEdit;
}

OUString SvInplaceEdit2::GetSavedValue() const
{
    return pEdit->GetSavedValue();
}

void SvInplaceEdit2::Hide()
{
    pEdit->Hide();
}


IMPL_LINK_NOARG_INLINE_START(SvInplaceEdit2, ReturnHdl_Impl)
{
    bCanceled = false;
    CallCallBackHdl_Impl();
    return 1;
}
IMPL_LINK_NOARG_INLINE_END(SvInplaceEdit2, ReturnHdl_Impl)

IMPL_LINK_NOARG_INLINE_START(SvInplaceEdit2, EscapeHdl_Impl)
{
    bCanceled = true;
    CallCallBackHdl_Impl();
    return 1;
}
IMPL_LINK_NOARG_INLINE_END(SvInplaceEdit2, EscapeHdl_Impl)


bool SvInplaceEdit2::KeyInput( const KeyEvent& rKEvt )
{
    vcl::KeyCode aCode = rKEvt.GetKeyCode();
    sal_uInt16 nCode = aCode.GetCode();

    switch ( nCode )
    {
        case KEY_ESCAPE:
            bCanceled = true;
            CallCallBackHdl_Impl();
            return true;

        case KEY_RETURN:
            bCanceled = false;
            CallCallBackHdl_Impl();
            return true;
    }
    return false;
}

void SvInplaceEdit2::StopEditing( bool bCancel )
{
    if ( !bAlreadyInCallBack )
    {
        bCanceled = bCancel;
        CallCallBackHdl_Impl();
    }
}

void SvInplaceEdit2::LoseFocus()
{
    if ( !bAlreadyInCallBack
    && ((!Application::GetFocusWindow()) || !pEdit->IsChild( Application::GetFocusWindow()) )
    )
    {
        bCanceled = false;
        aTimer.SetTimeout(10);
        aTimer.SetTimeoutHdl(LINK(this,SvInplaceEdit2,Timeout_Impl));
        aTimer.Start();
    }
}

IMPL_LINK_NOARG_INLINE_START(SvInplaceEdit2, Timeout_Impl)
{
    CallCallBackHdl_Impl();
    return 0;
}
IMPL_LINK_NOARG_INLINE_END(SvInplaceEdit2, Timeout_Impl)

void SvInplaceEdit2::CallCallBackHdl_Impl()
{
    aTimer.Stop();
    if ( !bAlreadyInCallBack )
    {
        bAlreadyInCallBack = true;
        Application::RemoveAccel( &aAccReturn );
        Application::RemoveAccel( &aAccEscape );
        pEdit->Hide();
        aCallBackHdl.Call( this );
    }
}

OUString SvInplaceEdit2::GetText() const
{
    return pEdit->GetText();
}

// ***************************************************************
// class SvLBoxTab
// ***************************************************************


SvLBoxTab::SvLBoxTab()
{
    nPos = 0;
    pUserData = 0;
    nFlags = 0;
}

SvLBoxTab::SvLBoxTab( long nPosition, sal_uInt16 nTabFlags )
{
    nPos = nPosition;
    pUserData = 0;
    nFlags = nTabFlags;
}

SvLBoxTab::SvLBoxTab( const SvLBoxTab& rTab )
{
    nPos = rTab.nPos;
    pUserData = rTab.pUserData;
    nFlags = rTab.nFlags;
}

SvLBoxTab::~SvLBoxTab()
{
}


long SvLBoxTab::CalcOffset( long nItemWidth, long nTabWidth )
{
    long nOffset = 0;
    if ( nFlags & SV_LBOXTAB_ADJUST_RIGHT )
    {
        nOffset = nTabWidth - nItemWidth;
        if( nOffset < 0 )
            nOffset = 0;
    }
    else if ( nFlags & SV_LBOXTAB_ADJUST_CENTER )
    {
        if( nFlags & SV_LBOXTAB_FORCE )
        {
            // correct implementation of centering
            nOffset = ( nTabWidth - nItemWidth ) / 2;
            if( nOffset < 0 )
                nOffset = 0;
        }
        else
        {
            // historically grown, wrong calculation of tabs which is needed by
            // Abo-Tabbox, Tools/Options/Customize etc.
            nItemWidth++;
            nOffset = -( nItemWidth / 2 );
        }
    }
    return nOffset;
}

// ***************************************************************
// class SvLBoxItem
// ***************************************************************


SvLBoxItem::SvLBoxItem( SvTreeListEntry*, sal_uInt16 )
{
}

SvLBoxItem::SvLBoxItem()
{
}

SvLBoxItem::~SvLBoxItem()
{
}

const Size& SvLBoxItem::GetSize(const SvTreeListBox* pView, const SvTreeListEntry* pEntry) const
{
    const SvViewDataItem* pViewData = pView->GetViewDataItem( pEntry, this );
    return pViewData->maSize;
}

const Size& SvLBoxItem::GetSize(const SvViewDataEntry* pData, sal_uInt16 nItemPos) const
{
    const SvViewDataItem* pIData = pData->GetItem(nItemPos);
    return pIData->maSize;
}

struct SvTreeListBoxImpl
{
    bool m_bIsEmptyTextAllowed:1;
    bool m_bEntryMnemonicsEnabled:1;
    bool m_bDoingQuickSelection:1;

    Link* m_pLink;

    vcl::MnemonicEngine m_aMnemonicEngine;
    vcl::QuickSelectionEngine m_aQuickSelectionEngine;

    SvTreeListBoxImpl(SvTreeListBox& _rBox) :
        m_bIsEmptyTextAllowed(true),
        m_bEntryMnemonicsEnabled(false),
        m_bDoingQuickSelection(false),
        m_pLink(NULL),
        m_aMnemonicEngine(_rBox),
        m_aQuickSelectionEngine(_rBox) {}
};


SvTreeListBox::SvTreeListBox(vcl::Window* pParent, WinBits nWinStyle) :
    Control(pParent, nWinStyle | WB_CLIPCHILDREN),
    DropTargetHelper(this),
    DragSourceHelper(this),
    mpImpl(new SvTreeListBoxImpl(*this)),
    mbContextBmpExpanded(false),
    mbAlternatingRowColors(false),
    eSelMode(NO_SELECTION),
    nMinWidthInChars(0)
{
    nDragOptions =  DND_ACTION_COPYMOVE | DND_ACTION_LINK;
    nImpFlags = 0;
    pTargetEntry = 0;
    nDragDropMode = 0;
    SvTreeList* pTempModel = new SvTreeList;
    pTempModel->SetRefCount( 0 );
    SetBaseModel(pTempModel);
    pModel->SetCloneLink( LINK(this, SvTreeListBox, CloneHdl_Impl ));
    pModel->InsertView( this );
    pHdlEntry = 0;
    pEdCtrl = 0;
    eSelMode = SINGLE_SELECTION;
    nDragDropMode = SV_DRAGDROP_NONE;
    SetType(WINDOW_TREELISTBOX);

    InitTreeView();

    SetSublistOpenWithLeftRight();
}

SvTreeListBox::SvTreeListBox(vcl::Window* pParent, const ResId& rResId) :
    Control(pParent, rResId),
    DropTargetHelper(this),
    DragSourceHelper(this),
    mpImpl(new SvTreeListBoxImpl(*this)),
    mbContextBmpExpanded(false),
    mbAlternatingRowColors(false),
    eSelMode(NO_SELECTION),
    nMinWidthInChars(0)
{
    pTargetEntry = 0;
    nImpFlags = 0;
    nDragOptions = DND_ACTION_COPYMOVE | DND_ACTION_LINK;
    nDragDropMode = 0;
    SvTreeList* pTempModel = new SvTreeList;
    pTempModel->SetRefCount( 0 );
    SetBaseModel(pTempModel);
    pModel->InsertView( this );
    pHdlEntry = 0;
    pEdCtrl = 0;
    pModel->SetCloneLink( LINK(this, SvTreeListBox, CloneHdl_Impl ));
    SetType(WINDOW_TREELISTBOX);

    InitTreeView();
    Resize();

    SetSublistOpenWithLeftRight();
}

extern "C" SAL_DLLPUBLIC_EXPORT vcl::Window* SAL_CALL makeSvTreeListBox(vcl::Window *pParent, VclBuilder::stringmap &rMap)
{
    WinBits nWinStyle = WB_TABSTOP;
    OString sBorder = VclBuilder::extractCustomProperty(rMap);
    if (!sBorder.isEmpty())
        nWinStyle |= WB_BORDER;
    return new SvTreeListBox(pParent, nWinStyle);
}

void SvTreeListBox::Clear()
{
    pModel->Clear();  // Model calls SvTreeListBox::ModelHasCleared()
}

void SvTreeListBox::EnableEntryMnemonics( bool _bEnable )
{
    if ( _bEnable == IsEntryMnemonicsEnabled() )
        return;

    mpImpl->m_bEntryMnemonicsEnabled = _bEnable;
    Invalidate();
}

bool SvTreeListBox::IsEntryMnemonicsEnabled() const
{
    return mpImpl->m_bEntryMnemonicsEnabled;
}

IMPL_LINK_INLINE_START( SvTreeListBox, CloneHdl_Impl, SvTreeListEntry*, pEntry )
{
    return reinterpret_cast<sal_IntPtr>(CloneEntry((SvTreeListEntry*)pEntry));
}
IMPL_LINK_INLINE_END( SvTreeListBox, CloneHdl_Impl, SvTreeListEntry*, pEntry )

sal_uLong SvTreeListBox::Insert( SvTreeListEntry* pEntry, SvTreeListEntry* pParent, sal_uLong nPos )
{
    sal_uLong nInsPos = pModel->Insert( pEntry, pParent, nPos );
    if(mbAlternatingRowColors)
    {
        if(nPos == TREELIST_APPEND)
            pEntry->SetBackColor( Prev(pEntry) && Prev(pEntry)->GetBackColor() == GetSettings().GetStyleSettings().GetRowColor() ?
                                    GetSettings().GetStyleSettings().GetAlternatingRowColor() :
                                    GetSettings().GetStyleSettings().GetRowColor() );
        else
            SetAlternatingRowColors( true );
    }
    return nInsPos;
}

sal_uLong SvTreeListBox::Insert( SvTreeListEntry* pEntry,sal_uLong nRootPos )
{
    sal_uLong nInsPos = pModel->Insert( pEntry, nRootPos );
    if(mbAlternatingRowColors)
    {
        if(nRootPos == TREELIST_APPEND)
            pEntry->SetBackColor( Prev(pEntry) && Prev(pEntry)->GetBackColor() == GetSettings().GetStyleSettings().GetRowColor() ?
                                    GetSettings().GetStyleSettings().GetAlternatingRowColor() :
                                    GetSettings().GetStyleSettings().GetRowColor() );
        else
            SetAlternatingRowColors( true );
    }
    return nInsPos;
}

bool SvTreeListBox::ExpandingHdl()
{
    return !aExpandingHdl.IsSet() || aExpandingHdl.Call( this );
}

void SvTreeListBox::ExpandedHdl()
{
    aExpandedHdl.Call( this );
}

void SvTreeListBox::SelectHdl()
{
    aSelectHdl.Call( this );
}

void SvTreeListBox::DeselectHdl()
{
    aDeselectHdl.Call( this );
}

bool SvTreeListBox::DoubleClickHdl()
{
    aDoubleClickHdl.Call( this );
    return true;
}


bool SvTreeListBox::CheckDragAndDropMode( SvTreeListBox* pSource, sal_Int8 nAction )
{
    if ( pSource == this )
    {
        if ( !(nDragDropMode & (SV_DRAGDROP_CTRL_MOVE | SV_DRAGDROP_CTRL_COPY) ) )
            return false; // D&D locked within list
        if( DND_ACTION_MOVE == nAction )
        {
            if ( !(nDragDropMode & SV_DRAGDROP_CTRL_MOVE) )
                 return false; // no local move
        }
        else
        {
            if ( !(nDragDropMode & SV_DRAGDROP_CTRL_COPY))
                return false; // no local copy
        }
    }
    else
    {
        if ( !(nDragDropMode & SV_DRAGDROP_APP_DROP ) )
            return false; // no drop
        if ( DND_ACTION_MOVE == nAction )
        {
            if ( !(nDragDropMode & SV_DRAGDROP_APP_MOVE) )
                return false; // no global move
        }
        else
        {
            if ( !(nDragDropMode & SV_DRAGDROP_APP_COPY))
                return false; // no global copy
        }
    }
    return true;
}




void SvTreeListBox::NotifyRemoving( SvTreeListEntry* )
{
}

/*
    NotifyMoving/Copying
    ====================

    default behavior:

    1. target doesn't have children
        - entry becomes sibling of target. entry comes after target
          (->Window: below the target)
    2. target is an expanded parent
        - entry inserted at the beginning of the target childlist
    3. target is a collapsed parent
        - entry is inserted at the end of the target childlist
*/
#ifdef DBG_UTIL
TriState SvTreeListBox::NotifyMoving(
    SvTreeListEntry*  pTarget,       // D&D dropping position in this->GetModel()
    SvTreeListEntry*  pEntry,        // entry that we want to move, from
                                 // GetSourceListBox()->GetModel()
    SvTreeListEntry*& rpNewParent,   // new target parent
    sal_uLong&        rNewChildPos)  // position in childlist of target parent
#else
TriState SvTreeListBox::NotifyMoving(
    SvTreeListEntry*  pTarget,       // D&D dropping position in this->GetModel()
    SvTreeListEntry*,                // entry that we want to move, from
                                 // GetSourceListBox()->GetModel()
    SvTreeListEntry*& rpNewParent,   // new target parent
    sal_uLong&        rNewChildPos)  // position in childlist of target parent
#endif
{
    DBG_ASSERT(pEntry,"NotifyMoving:SoureEntry?");
    if( !pTarget )
    {
        rpNewParent = 0;
        rNewChildPos = 0;
        return TRISTATE_TRUE;
    }
    if ( !pTarget->HasChildren() && !pTarget->HasChildrenOnDemand() )
    {
        // case 1
        rpNewParent = GetParent( pTarget );
        rNewChildPos = pModel->GetRelPos( pTarget ) + 1;
        rNewChildPos += nCurEntrySelPos;
        nCurEntrySelPos++;
    }
    else
    {
        // cases 2 & 3
        rpNewParent = pTarget;
        if( IsExpanded(pTarget))
            rNewChildPos = 0;
        else
            rNewChildPos = TREELIST_APPEND;
    }
    return TRISTATE_TRUE;
}

TriState SvTreeListBox::NotifyCopying(
    SvTreeListEntry*  pTarget,       // D&D dropping position in this->GetModel()
    SvTreeListEntry*  pEntry,        // entry that we want to move, from
                                 // GetSourceListBox()->GetModel()
    SvTreeListEntry*& rpNewParent,   // new target parent
    sal_uLong&        rNewChildPos)  // position in childlist of target parent
{
    return NotifyMoving(pTarget,pEntry,rpNewParent,rNewChildPos);
}

SvTreeListEntry* SvTreeListBox::FirstChild( SvTreeListEntry* pParent ) const
{
    return pModel->FirstChild(pParent);
}

SvTreeListEntry* SvTreeListBox::NextSibling( SvTreeListEntry* pEntry ) const
{
    return pModel->NextSibling(pEntry);
}

SvTreeListEntry* SvTreeListBox::PrevSibling( SvTreeListEntry* pEntry ) const
{
    return pModel->PrevSibling(pEntry);
}

// return: all entries copied
bool SvTreeListBox::CopySelection( SvTreeListBox* pSource, SvTreeListEntry* pTarget )
{
    nCurEntrySelPos = 0; // selection counter for NotifyMoving/Copying
    bool bSuccess = true;
    std::vector<SvTreeListEntry*> aList;
    bool bClone = ( pSource->GetModel() != GetModel() );
    Link aCloneLink( pModel->GetCloneLink() );
    pModel->SetCloneLink( LINK(this, SvTreeListBox, CloneHdl_Impl ));

    // cache selection to simplify iterating over the selection when doing a D&D
    // exchange within the same listbox
    SvTreeListEntry* pSourceEntry = pSource->FirstSelected();
    while ( pSourceEntry )
    {
        // children are copied automatically
        pSource->SelectChildren( pSourceEntry, false );
        aList.push_back( pSourceEntry );
        pSourceEntry = pSource->NextSelected( pSourceEntry );
    }

    std::vector<SvTreeListEntry*>::const_iterator it = aList.begin(), itEnd = aList.end();
    for (; it != itEnd; ++it)
    {
        pSourceEntry = *it;
        SvTreeListEntry* pNewParent = 0;
        sal_uLong nInsertionPos = TREELIST_APPEND;
        TriState nOk = NotifyCopying(pTarget,pSourceEntry,pNewParent,nInsertionPos);
        if ( nOk )
        {
            if ( bClone )
            {
                sal_uLong nCloneCount = 0;
                pSourceEntry = pModel->Clone(pSourceEntry, nCloneCount);
                pModel->InsertTree(pSourceEntry, pNewParent, nInsertionPos);
            }
            else
            {
                sal_uLong nListPos = pModel->Copy(pSourceEntry, pNewParent, nInsertionPos);
                pSourceEntry = GetEntry( pNewParent, nListPos );
            }
        }
        else
            bSuccess = false;

        if (nOk == TRISTATE_INDET)  // HACK: make visible moved entry
            MakeVisible( pSourceEntry );
    }
    pModel->SetCloneLink( aCloneLink );
    return bSuccess;
}

// return: all entries were moved
bool SvTreeListBox::MoveSelection( SvTreeListBox* pSource, SvTreeListEntry* pTarget )
{
    return MoveSelectionCopyFallbackPossible( pSource, pTarget, false );
}

bool SvTreeListBox::MoveSelectionCopyFallbackPossible( SvTreeListBox* pSource, SvTreeListEntry* pTarget, bool bAllowCopyFallback )
{
    nCurEntrySelPos = 0; // selection counter for NotifyMoving/Copying
    bool bSuccess = true;
    std::vector<SvTreeListEntry*> aList;
    bool bClone = ( pSource->GetModel() != GetModel() );
    Link aCloneLink( pModel->GetCloneLink() );
    if ( bClone )
        pModel->SetCloneLink( LINK(this, SvTreeListBox, CloneHdl_Impl ));

    SvTreeListEntry* pSourceEntry = pSource->FirstSelected();
    while ( pSourceEntry )
    {
        // children are automatically moved
        pSource->SelectChildren( pSourceEntry, false );
        aList.push_back( pSourceEntry );
        pSourceEntry = pSource->NextSelected( pSourceEntry );
    }

    std::vector<SvTreeListEntry*>::const_iterator it = aList.begin(), itEnd = aList.end();
    for (; it != itEnd; ++it)
    {
        pSourceEntry = *it;

        SvTreeListEntry* pNewParent = 0;
        sal_uLong nInsertionPos = TREELIST_APPEND;
        TriState nOk = NotifyMoving(pTarget,pSourceEntry,pNewParent,nInsertionPos);
        TriState nCopyOk = nOk;
        if ( !nOk && bAllowCopyFallback )
        {
            nInsertionPos = TREELIST_APPEND;
            nCopyOk = NotifyCopying(pTarget,pSourceEntry,pNewParent,nInsertionPos);
        }

        if ( nOk || nCopyOk )
        {
            if ( bClone )
            {
                sal_uLong nCloneCount = 0;
                pSourceEntry = pModel->Clone(pSourceEntry, nCloneCount);
                pModel->InsertTree(pSourceEntry, pNewParent, nInsertionPos);
            }
            else
            {
                if ( nOk )
                    pModel->Move(pSourceEntry, pNewParent, nInsertionPos);
                else
                    pModel->Copy(pSourceEntry, pNewParent, nInsertionPos);
            }
        }
        else
            bSuccess = false;

        if (nOk == TRISTATE_INDET)  // HACK: make moved entry visible
            MakeVisible( pSourceEntry );
    }
    pModel->SetCloneLink( aCloneLink );
    return bSuccess;
}

void SvTreeListBox::RemoveSelection()
{
    std::vector<const SvTreeListEntry*> aList;
    // cache selection, as the implementation deselects everything on the first
    // remove
    SvTreeListEntry* pEntry = FirstSelected();
    while ( pEntry )
    {
        aList.push_back( pEntry );
        if ( pEntry->HasChildren() )
            // remove deletes all children automatically
            SelectChildren(pEntry, false);
        pEntry = NextSelected( pEntry );
    }

    std::vector<const SvTreeListEntry*>::const_iterator it = aList.begin(), itEnd = aList.end();
    for (; it != itEnd; ++it)
        pModel->Remove(*it);
}

SvTreeListBox* SvTreeListBox::GetSourceView() const
{
    return pDDSource;
}

void SvTreeListBox::RecalcViewData()
{
    SvTreeListEntry* pEntry = First();
    while( pEntry )
    {
        sal_uInt16 nCount = pEntry->ItemCount();
        sal_uInt16 nCurPos = 0;
        while ( nCurPos < nCount )
        {
            SvLBoxItem* pItem = pEntry->GetItem( nCurPos );
            pItem->InitViewData( this, pEntry );
            nCurPos++;
        }
        ViewDataInitialized( pEntry );
        pEntry = Next( pEntry );
    }
}

void SvTreeListBox::ViewDataInitialized( SvTreeListEntry* )
{
}

void SvTreeListBox::ImplShowTargetEmphasis( SvTreeListEntry* pEntry, bool bShow)
{
    if ( bShow && (nImpFlags & SVLBOX_TARGEMPH_VIS) )
        return;
    if ( !bShow && !(nImpFlags & SVLBOX_TARGEMPH_VIS) )
        return;
    ShowTargetEmphasis( pEntry, bShow );
    if( bShow )
        nImpFlags |= SVLBOX_TARGEMPH_VIS;
    else
        nImpFlags &= ~SVLBOX_TARGEMPH_VIS;
}

void SvTreeListBox::OnCurrentEntryChanged()
{
    if ( !mpImpl->m_bDoingQuickSelection )
        mpImpl->m_aQuickSelectionEngine.Reset();
}

SvTreeListEntry* SvTreeListBox::GetEntry( SvTreeListEntry* pParent, sal_uLong nPos ) const
{
    return pModel->GetEntry(pParent, nPos);
}

SvTreeListEntry* SvTreeListBox::GetEntry( sal_uLong nRootPos ) const
{
    return pModel->GetEntry(nRootPos);
}

SvTreeListEntry* SvTreeListBox::GetEntryFromPath( const ::std::deque< sal_Int32 >& _rPath ) const
{

    SvTreeListEntry* pEntry = NULL;
    SvTreeListEntry* pParent = NULL;
    for( ::std::deque< sal_Int32 >::const_iterator pItem = _rPath.begin(); pItem != _rPath.end(); ++pItem )
    {
        pEntry = GetEntry( pParent, *pItem );
        if ( !pEntry )
            break;
        pParent = pEntry;
    }

    return pEntry;
}

void SvTreeListBox::FillEntryPath( SvTreeListEntry* pEntry, ::std::deque< sal_Int32 >& _rPath ) const
{

    if ( pEntry )
    {
        SvTreeListEntry* pParentEntry = GetParent( pEntry );
        while ( true )
        {
            sal_uLong i, nCount = GetLevelChildCount( pParentEntry );
            for ( i = 0; i < nCount; ++i )
            {
                SvTreeListEntry* pTemp = GetEntry( pParentEntry, i );
                DBG_ASSERT( pEntry, "invalid entry" );
                if ( pEntry == pTemp )
                {
                    _rPath.push_front( (sal_Int32)i );
                    break;
                }
            }

            if ( pParentEntry )
            {
                pEntry = pParentEntry;
                pParentEntry = GetParent( pParentEntry );
            }
            else
                break;
        }
    }
}

const SvTreeListEntry* SvTreeListBox::GetParent( const SvTreeListEntry* pEntry ) const
{
    return pModel->GetParent(pEntry);
}

SvTreeListEntry* SvTreeListBox::GetParent( SvTreeListEntry* pEntry ) const
{
    return pModel->GetParent(pEntry);
}

SvTreeListEntry* SvTreeListBox::GetRootLevelParent( SvTreeListEntry* pEntry ) const
{
    return pModel->GetRootLevelParent(pEntry);
}

sal_uLong SvTreeListBox::GetChildCount( SvTreeListEntry* pParent ) const
{
    return pModel->GetChildCount(pParent);
}

sal_uLong SvTreeListBox::GetLevelChildCount( SvTreeListEntry* _pParent ) const
{

    //if _pParent is 0, then pEntry is the first child of the root.
    SvTreeListEntry* pEntry = FirstChild( _pParent );

    if( !pEntry )//there is only root, root don't have children
        return 0;

    if( !_pParent )//root and children of root
        return pEntry->pParent->maChildren.size();

    return _pParent->maChildren.size();
}

SvViewDataEntry* SvTreeListBox::GetViewDataEntry( SvTreeListEntry* pEntry ) const
{
    return (SvViewDataEntry*)SvListView::GetViewData(pEntry);
}

SvViewDataItem* SvTreeListBox::GetViewDataItem(SvTreeListEntry* pEntry, SvLBoxItem* pItem)
{
    return const_cast<SvViewDataItem*>(static_cast<const SvTreeListBox*>(this)->GetViewDataItem(pEntry, pItem));
}

const SvViewDataItem* SvTreeListBox::GetViewDataItem(const SvTreeListEntry* pEntry, const SvLBoxItem* pItem) const
{
    const SvViewDataEntry* pEntryData = (const SvViewDataEntry*)SvListView::GetViewData(pEntry);
    DBG_ASSERT(pEntryData,"Entry not in View");
    sal_uInt16 nItemPos = pEntry->GetPos(pItem);
    return pEntryData->GetItem(nItemPos);
}

SvViewDataEntry* SvTreeListBox::CreateViewData( SvTreeListEntry* )
{
    SvViewDataEntry* pEntryData = new SvViewDataEntry;
    return (SvViewDataEntry*)pEntryData;
}

void SvTreeListBox::InitViewData( SvViewDataEntry* pData, SvTreeListEntry* pEntry )
{
    SvTreeListEntry* pInhEntry = (SvTreeListEntry*)pEntry;
    SvViewDataEntry* pEntryData = (SvViewDataEntry*)pData;

    pEntryData->Init(pInhEntry->ItemCount());
    sal_uInt16 nCount = pInhEntry->ItemCount();
    sal_uInt16 nCurPos = 0;
    while( nCurPos < nCount )
    {
        SvLBoxItem* pItem = pInhEntry->GetItem( nCurPos );
        SvViewDataItem* pItemData = pEntryData->GetItem(nCurPos);
        pItem->InitViewData( this, pInhEntry, pItemData );
        pItemData++;
        nCurPos++;
    }
}



void SvTreeListBox::EnableSelectionAsDropTarget( bool bEnable, bool bWithChildren )
{
    sal_uInt16 nRefDepth;
    SvTreeListEntry* pTemp;

    SvTreeListEntry* pSelEntry = FirstSelected();
    while( pSelEntry )
    {
        if ( !bEnable )
        {
            pSelEntry->nEntryFlags |= SV_ENTRYFLAG_DISABLE_DROP;
            if ( bWithChildren )
            {
                nRefDepth = pModel->GetDepth( pSelEntry );
                pTemp = Next( pSelEntry );
                while( pTemp && pModel->GetDepth( pTemp ) > nRefDepth )
                {
                    pTemp->nEntryFlags |= SV_ENTRYFLAG_DISABLE_DROP;
                    pTemp = Next( pTemp );
                }
            }
        }
        else
        {
            pSelEntry->nEntryFlags &= (~SV_ENTRYFLAG_DISABLE_DROP);
            if ( bWithChildren )
            {
                nRefDepth = pModel->GetDepth( pSelEntry );
                pTemp = Next( pSelEntry );
                while( pTemp && pModel->GetDepth( pTemp ) > nRefDepth )
                {
                    pTemp->nEntryFlags &= (~SV_ENTRYFLAG_DISABLE_DROP);
                    pTemp = Next( pTemp );
                }
            }
        }
        pSelEntry = NextSelected( pSelEntry );
    }
}

// ******************************************************************
// InplaceEditing
// ******************************************************************

void SvTreeListBox::EditText( const OUString& rStr, const Rectangle& rRect,
    const Selection& rSel )
{
    EditText( rStr, rRect, rSel, false );
}

void SvTreeListBox::EditText( const OUString& rStr, const Rectangle& rRect,
    const Selection& rSel, bool bMulti )
{
    if( pEdCtrl )
        delete pEdCtrl;
    nImpFlags |= SVLBOX_IN_EDT;
    nImpFlags &= ~SVLBOX_EDTEND_CALLED;
    HideFocus();
    pEdCtrl = new SvInplaceEdit2(
        this, rRect.TopLeft(), rRect.GetSize(), rStr,
        LINK( this, SvTreeListBox, TextEditEndedHdl_Impl ),
        rSel, bMulti );
}

IMPL_LINK_NOARG(SvTreeListBox, TextEditEndedHdl_Impl)
{
    if ( nImpFlags & SVLBOX_EDTEND_CALLED ) // avoid nesting
        return 0;
    nImpFlags |= SVLBOX_EDTEND_CALLED;
    OUString aStr;
    if ( !pEdCtrl->EditingCanceled() )
        aStr = pEdCtrl->GetText();
    else
        aStr = pEdCtrl->GetSavedValue();
    if ( IsEmptyTextAllowed() || !aStr.isEmpty() )
        EditedText( aStr );
    // Hide may only be called after the new text was put into the entry, so
    // that we don't call the selection handler in the GetFocus of the listbox
    // with the old entry text.
    pEdCtrl->Hide();
    // delete pEdCtrl;
    // pEdCtrl = 0;
    nImpFlags &= (~SVLBOX_IN_EDT);
    GrabFocus();
    return 0;
}

void SvTreeListBox::CancelTextEditing()
{
    if ( pEdCtrl )
        pEdCtrl->StopEditing( true );
    nImpFlags &= (~SVLBOX_IN_EDT);
}

void SvTreeListBox::EndEditing( bool bCancel )
{
    if( pEdCtrl )
        pEdCtrl->StopEditing( bCancel );
    nImpFlags &= (~SVLBOX_IN_EDT);
}


bool SvTreeListBox::IsEmptyTextAllowed() const
{
    return mpImpl->m_bIsEmptyTextAllowed;
}

void SvTreeListBox::ForbidEmptyText()
{
    mpImpl->m_bIsEmptyTextAllowed = false;
}

SvTreeListEntry* SvTreeListBox::CreateEntry() const
{
    return new SvTreeListEntry;
}

const void* SvTreeListBox::FirstSearchEntry( OUString& _rEntryText ) const
{
    SvTreeListEntry* pEntry = GetCurEntry();
    if ( pEntry )
        pEntry = const_cast< SvTreeListEntry* >( static_cast< const SvTreeListEntry* >( NextSearchEntry( pEntry, _rEntryText ) ) );
    else
    {
        pEntry = FirstSelected();
        if ( !pEntry )
            pEntry = First();
    }

    if ( pEntry )
        _rEntryText = GetEntryText( pEntry );

    return pEntry;
}

const void* SvTreeListBox::NextSearchEntry( const void* _pCurrentSearchEntry, OUString& _rEntryText ) const
{
    SvTreeListEntry* pEntry = const_cast< SvTreeListEntry* >( static_cast< const SvTreeListEntry* >( _pCurrentSearchEntry ) );

    if  (   (   ( GetChildCount( pEntry ) > 0 )
            ||  ( pEntry->HasChildrenOnDemand() )
            )
        &&  !IsExpanded( pEntry )
        )
    {
        pEntry = NextSibling( pEntry );
    }
    else
    {
        pEntry = Next( pEntry );
    }

    if ( !pEntry )
        pEntry = First();

    if ( pEntry )
        _rEntryText = GetEntryText( pEntry );

    return pEntry;
}

void SvTreeListBox::SelectSearchEntry( const void* _pEntry )
{
    SvTreeListEntry* pEntry = const_cast< SvTreeListEntry* >( static_cast< const SvTreeListEntry* >( _pEntry ) );
    DBG_ASSERT( pEntry, "SvTreeListBox::SelectSearchEntry: invalid entry!" );
    if ( !pEntry )
        return;

    SelectAll( false );
    SetCurEntry( pEntry );
    Select( pEntry );
}

void SvTreeListBox::ExecuteSearchEntry( const void* /*_pEntry*/ ) const
{
    // nothing to do here, we have no "execution"
}

::vcl::StringEntryIdentifier SvTreeListBox::CurrentEntry( OUString& _out_entryText ) const
{
    // always accept the current entry if there is one
    SvTreeListEntry* pCurrentEntry( GetCurEntry() );
    if ( pCurrentEntry )
    {
        _out_entryText = GetEntryText( pCurrentEntry );
        return pCurrentEntry;
    }
    return FirstSearchEntry( _out_entryText );
}

::vcl::StringEntryIdentifier SvTreeListBox::NextEntry( ::vcl::StringEntryIdentifier _currentEntry, OUString& _out_entryText ) const
{
    return NextSearchEntry( _currentEntry, _out_entryText );
}

void SvTreeListBox::SelectEntry( ::vcl::StringEntryIdentifier _entry )
{
    SelectSearchEntry( _entry );
}

bool SvTreeListBox::HandleKeyInput( const KeyEvent& _rKEvt )
{
    if  (   IsEntryMnemonicsEnabled()
        &&  mpImpl->m_aMnemonicEngine.HandleKeyEvent( _rKEvt )
        )
        return true;

    if ( ( GetStyle() & WB_QUICK_SEARCH ) != 0 )
    {
        mpImpl->m_bDoingQuickSelection = true;
        const bool bHandled = mpImpl->m_aQuickSelectionEngine.HandleKeyEvent( _rKEvt );
        mpImpl->m_bDoingQuickSelection = false;
        if ( bHandled )
            return true;
    }

    return false;
}

void SvTreeListBox::WriteDragServerInfo( const Point&, SvLBoxDDInfo* )
{
}

void SvTreeListBox::ReadDragServerInfo(const Point&, SvLBoxDDInfo* )
{
}

bool SvTreeListBox::EditingCanceled() const
{
    if( pEdCtrl && pEdCtrl->EditingCanceled() )
        return true;
    return false;
}


//JP 28.3.2001: new Drag & Drop API
sal_Int8 SvTreeListBox::AcceptDrop( const AcceptDropEvent& rEvt )
{
    sal_Int8 nRet = DND_ACTION_NONE;

    if( rEvt.mbLeaving || !CheckDragAndDropMode( pDDSource, rEvt.mnAction ) )
    {
        ImplShowTargetEmphasis( pTargetEntry, false );
    }
    else if( !nDragDropMode )
    {
        SAL_WARN( "svtools.contnr", "SvTreeListBox::QueryDrop(): no target" );
    }
    else
    {
        SvTreeListEntry* pEntry = GetDropTarget( rEvt.maPosPixel );
        if( !IsDropFormatSupported( SOT_FORMATSTR_ID_TREELISTBOX ) )
        {
            SAL_WARN( "svtools.contnr", "SvTreeListBox::QueryDrop(): no format" );
        }
        else
        {
            DBG_ASSERT( pDDSource, "SvTreeListBox::QueryDrop(): SourceBox == 0" );
            if( !( pEntry && pDDSource->GetModel() == this->GetModel()
                    && DND_ACTION_MOVE == rEvt.mnAction
                    && ( pEntry->nEntryFlags & SV_ENTRYFLAG_DISABLE_DROP ) ))
            {
                if( NotifyAcceptDrop( pEntry ))
                    nRet = rEvt.mnAction;
            }
        }

        // **** draw emphasis ****
        if( DND_ACTION_NONE == nRet )
               ImplShowTargetEmphasis( pTargetEntry, false );
        else if( pEntry != pTargetEntry || !(nImpFlags & SVLBOX_TARGEMPH_VIS) )
        {
            ImplShowTargetEmphasis( pTargetEntry, false );
            pTargetEntry = pEntry;
            ImplShowTargetEmphasis( pTargetEntry, true );
        }
    }
    return nRet;
}

sal_Int8 SvTreeListBox::ExecuteDrop( const ExecuteDropEvent& rEvt, SvTreeListBox* pSourceView )
{
    sal_Int8 nRet = DND_ACTION_NONE;

    DBG_ASSERT( pSourceView, "SvTreeListBox::ExecuteDrop(): no source view" );
    pSourceView->EnableSelectionAsDropTarget( true, true );

    ImplShowTargetEmphasis( pTargetEntry, false );
    pDDTarget = this;

    SvLBoxDDInfo aDDInfo;

    TransferableDataHelper aData( rEvt.maDropEvent.Transferable );
    if( aData.HasFormat( SOT_FORMATSTR_ID_TREELISTBOX ))
    {
        css::uno::Sequence<sal_Int8> aSeq = aData.GetSequence(SOT_FORMATSTR_ID_TREELISTBOX, OUString());
        if (sizeof(SvLBoxDDInfo) == aSeq.getLength())
        {
            memcpy( &aDDInfo, aSeq.getConstArray(), sizeof(SvLBoxDDInfo) );
            nRet = rEvt.mnAction;
        }
    }

    if( DND_ACTION_NONE != nRet )
    {
        nRet = DND_ACTION_NONE;

        ReadDragServerInfo( rEvt.maPosPixel, &aDDInfo );

        SvTreeListEntry* pTarget = pTargetEntry; // may be 0!

        if( DND_ACTION_COPY == rEvt.mnAction )
        {
            if ( CopySelection( aDDInfo.pSource, pTarget ) )
                nRet = rEvt.mnAction;
        }
        else if( DND_ACTION_MOVE == rEvt.mnAction )
        {
            if ( MoveSelection( aDDInfo.pSource, pTarget ) )
                nRet = rEvt.mnAction;
        }
        else if( DND_ACTION_COPYMOVE == rEvt.mnAction )
        {
            if ( MoveSelectionCopyFallbackPossible( aDDInfo.pSource, pTarget, true ) )
                nRet = rEvt.mnAction;
        }
    }
    return nRet;
}

sal_Int8 SvTreeListBox::ExecuteDrop( const ExecuteDropEvent& rEvt )
{
    return ExecuteDrop( rEvt, GetSourceView() );
}

void SvTreeListBox::StartDrag( sal_Int8, const Point& rPosPixel )
{

    Point aEventPos( rPosPixel );
    MouseEvent aMouseEvt( aEventPos, 1, MouseEventModifiers::SELECT, MOUSE_LEFT );
    MouseButtonUp( aMouseEvt );

    nOldDragMode = GetDragDropMode();
    if ( !nOldDragMode )
        return;

    ReleaseMouse();

    SvTreeListEntry* pEntry = GetEntry( rPosPixel ); // GetDropTarget( rPos );
    if( !pEntry )
    {
        DragFinished( DND_ACTION_NONE );
        return;
    }

    TransferDataContainer* pContainer = new TransferDataContainer;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::datatransfer::XTransferable > xRef( pContainer );

    nDragDropMode = NotifyStartDrag( *pContainer, pEntry );
    if( !nDragDropMode || 0 == GetSelectionCount() )
    {
        nDragDropMode = nOldDragMode;
        DragFinished( DND_ACTION_NONE );
        return;
    }

    SvLBoxDDInfo aDDInfo;
    memset(&aDDInfo,0,sizeof(SvLBoxDDInfo));
    aDDInfo.pApp = GetpApp();
    aDDInfo.pSource = this;
    aDDInfo.pDDStartEntry = pEntry;
    // let derived views do their thing
    WriteDragServerInfo( rPosPixel, &aDDInfo );

    pContainer->CopyAnyData( SOT_FORMATSTR_ID_TREELISTBOX,
                        (sal_Char*)&aDDInfo, sizeof(SvLBoxDDInfo) );
    pDDSource = this;
    pDDTarget = 0;

    bool bOldUpdateMode = Control::IsUpdateMode();
    Control::SetUpdateMode( true );
    Update();
    Control::SetUpdateMode( bOldUpdateMode );

    // Disallow using the selection and its children as drop targets.
    // Important: If the selection of the SourceListBox is changed in the
    // DropHandler, the entries have to be allowed as drop targets again:
    // (GetSourceListBox()->EnableSelectionAsDropTarget( true, true );)
    EnableSelectionAsDropTarget( false, true /* with children */ );

    pContainer->StartDrag( this, nDragOptions, GetDragFinishedHdl() );
}

void SvTreeListBox::DragFinished( sal_Int8
#ifndef UNX
nAction
#endif
)
{
    EnableSelectionAsDropTarget( true, true );

#ifndef UNX
    if( (nAction == DND_ACTION_MOVE) && ( (pDDTarget &&
        ((sal_uLong)(pDDTarget->GetModel())!=(sal_uLong)(this->GetModel()))) ||
        !pDDTarget ))
    {
        RemoveSelection();
    }
#endif

    ImplShowTargetEmphasis( pTargetEntry, false );
    pDDSource = 0;
    pDDTarget = 0;
    pTargetEntry = 0;
    nDragDropMode = nOldDragMode;
}

DragDropMode SvTreeListBox::NotifyStartDrag( TransferDataContainer&, SvTreeListEntry* )
{
    return (DragDropMode)0xffff;
}

bool SvTreeListBox::NotifyAcceptDrop( SvTreeListEntry* )
{
    return true;
}

// Handler and methods for Drag - finished handler.
// The with get GetDragFinishedHdl() get link can set on the
// TransferDataContainer. This link is a callback for the DragFinished
// call. AddBox method is called from the GetDragFinishedHdl() and the
// remove is called in link callback and in the destructor. So it can't
// called to a deleted object.

namespace
{
    struct SortLBoxes : public rtl::Static<std::set<sal_uLong>, SortLBoxes> {};
}

void SvTreeListBox::AddBoxToDDList_Impl( const SvTreeListBox& rB )
{
    sal_uLong nVal = reinterpret_cast<sal_uLong>(&rB);
    SortLBoxes::get().insert( nVal );
}

void SvTreeListBox::RemoveBoxFromDDList_Impl( const SvTreeListBox& rB )
{
    sal_uLong nVal = reinterpret_cast<sal_uLong>(&rB);
    SortLBoxes::get().erase( nVal );
}

IMPL_STATIC_LINK( SvTreeListBox, DragFinishHdl_Impl, sal_Int8*, pAction )
{
    sal_uLong nVal = reinterpret_cast<sal_uLong>(pThis);
    std::set<sal_uLong> &rSortLBoxes = SortLBoxes::get();
    std::set<sal_uLong>::const_iterator it = rSortLBoxes.find(nVal);
    if( it != rSortLBoxes.end() )
    {
        pThis->DragFinished( *pAction );
        rSortLBoxes.erase( it );
    }
    return 0;
}

Link SvTreeListBox::GetDragFinishedHdl() const
{
    AddBoxToDDList_Impl( *this );
    return STATIC_LINK( const_cast<SvTreeListBox*>(this), SvTreeListBox, DragFinishHdl_Impl );
}

/*
    Bugs/TODO

    - calculate rectangle when editing in-place (bug with some fonts)
    - SetSpaceBetweenEntries: offset is not taken into account in SetEntryHeight
*/

#define TREEFLAG_FIXEDHEIGHT        0x0010

#define SV_LBOX_DEFAULT_INDENT_PIXEL 20

void SvTreeListBox::InitTreeView()
{
    pCheckButtonData = NULL;
    pEdEntry = NULL;
    pEdItem = NULL;
    nEntryHeight = 0;
    pEdCtrl = NULL;
    nFirstSelTab = 0;
    nLastSelTab = 0;
    nFocusWidth = -1;
    nAllItemAccRoleType = 0;
    mnCheckboxItemWidth = 0;

    Link* pLink = new Link( LINK(this,SvTreeListBox, DefaultCompare) );
    mpImpl->m_pLink = pLink;

    nTreeFlags = TREEFLAG_RECALCTABS;
    nIndent = SV_LBOX_DEFAULT_INDENT_PIXEL;
    nEntryHeightOffs = SV_ENTRYHEIGHTOFFS_PIXEL;
    pImp = new SvImpLBox( this, GetModel(), GetStyle() );

    mbContextBmpExpanded = true;
    nContextBmpWidthMax = 0;

    SetFont( GetFont() );
    AdjustEntryHeightAndRecalc( GetFont() );

    SetSpaceBetweenEntries( 0 );
    SetLineColor();
    InitSettings( true, true, true );
    ImplInitStyle();
    SetTabs();
}

OUString SvTreeListBox::GetEntryAltText( SvTreeListEntry* ) const
{
    return OUString();
}

OUString SvTreeListBox::GetEntryLongDescription( SvTreeListEntry* ) const
{
    return OUString();
}

OUString SvTreeListBox::SearchEntryTextWithHeadTitle( SvTreeListEntry* pEntry )
{
    DBG_ASSERT( pEntry, "SvTreeListBox::SearchEntryText(): no entry" );
    OUString sRet;

    sal_uInt16 nCount = pEntry->ItemCount();
    sal_uInt16 nCur = 0;
    sal_uInt16 nHeaderCur = 0;
    SvLBoxItem* pItem;
    while( nCur < nCount )
    {
        // MT: SV_ITEM_ID_EXTENDRLBOXSTRING / GetExtendText() was in use in IA2 cws, but only used in sc: ScSolverOptionsString. Needed?
        pItem = pEntry->GetItem( nCur );
        if ( (pItem->GetType() == SV_ITEM_ID_LBOXSTRING ) &&
             !static_cast<SvLBoxString*>( pItem )->GetText().isEmpty() )
        {
            //want the column header
            if (!headString.isEmpty())
            {
                sal_Int32 nEnd = headString.indexOf('\t');
                if( nEnd == -1 )
                {
                    if (!sRet.isEmpty())
                    {
                        sRet += ",";
                    }
                    if (!headString.isEmpty())
                    {
                        sRet += headString ;
                        sRet += ":" ;
                    }
                }
                else
                {
                    OUString  aString=headString.getToken(nHeaderCur, '\t');
                    if (!sRet.isEmpty())
                    {
                        sRet += ",";
                    }
                    if (!aString.isEmpty())
                    {
                        sRet += aString ;
                        sRet += ":" ;
                    }
                    nHeaderCur++;
                }
                sRet += static_cast<SvLBoxString*>( pItem )->GetText();
            }
            else
            {
                sRet += static_cast<SvLBoxString*>( pItem )->GetText();
                sRet += ",";
            }
            //end want to the column header
        }
        nCur++;
    }

    if (!sRet.isEmpty())
        sRet = sRet.copy(0, sRet.getLength() - 1);
    return sRet;
}

SvTreeListBox::~SvTreeListBox()
{

    pImp->CallEventListeners( VCLEVENT_OBJECT_DYING );
    delete pImp;
    delete mpImpl->m_pLink;
    ClearTabList();

    delete pEdCtrl;
    pEdCtrl = 0;
    pModel->RemoveView( this );
    if ( pModel->GetRefCount() == 0 )
    {
        pModel->Clear();
        delete pModel;
        pModel = NULL;
    }

    SvTreeListBox::RemoveBoxFromDDList_Impl( *this );

    if( this == pDDSource )
        pDDSource = 0;
    if( this == pDDTarget )
        pDDTarget = 0;
    delete mpImpl;
}

void SvTreeListBox::SetExtendedWinBits( ExtendedWinBits _nBits )
{
    pImp->SetExtendedWindowBits( _nBits );
}

void SvTreeListBox::SetModel( SvTreeList* pNewModel )
{
    pImp->SetModel( pNewModel );
    SetBaseModel(pNewModel);
}

void SvTreeListBox::SetBaseModel( SvTreeList* pNewModel )
{
    // does the CleanUp
    SvListView::SetModel( pNewModel );
    pModel->SetCloneLink( LINK(this, SvTreeListBox, CloneHdl_Impl ));
    SvTreeListEntry* pEntry = First();
    while( pEntry )
    {
        ModelHasInserted( pEntry );
        pEntry = Next( pEntry );
    }
}

void SvTreeListBox::DisconnectFromModel()
{
    SvTreeList* pNewModel = new SvTreeList;
    pNewModel->SetRefCount( 0 );    // else this will never be deleted
    SvListView::SetModel( pNewModel );

    pImp->SetModel( GetModel() );
}

void SvTreeListBox::SetSublistOpenWithReturn( bool b )
{
    pImp->bSubLstOpRet = b;
}

void SvTreeListBox::SetSublistOpenWithLeftRight( bool b )
{
    pImp->bSubLstOpLR = b;
}

void SvTreeListBox::Resize()
{
    if( IsEditingActive() )
        EndEditing( true );

    Control::Resize();

    pImp->Resize();
    nFocusWidth = -1;
    pImp->ShowCursor( false );
    pImp->ShowCursor( true );
}

/* Cases:

   A) entries have bitmaps
       0. no buttons
       1. node buttons (can optionally also be on root items)
       2. node buttons (can optionally also be on root items) + CheckButton
       3. CheckButton
   B) entries don't have bitmaps  (=>via WindowBits because of D&D!)
       0. no buttons
       1. node buttons (can optionally also be on root items)
       2. node buttons (can optionally also be on root items) + CheckButton
       3. CheckButton
*/

#define NO_BUTTONS              0
#define NODE_BUTTONS            1
#define NODE_AND_CHECK_BUTTONS  2
#define CHECK_BUTTONS           3

#define TABFLAGS_TEXT (SV_LBOXTAB_DYNAMIC |        \
                       SV_LBOXTAB_ADJUST_LEFT |    \
                       SV_LBOXTAB_EDITABLE |       \
                       SV_LBOXTAB_SHOW_SELECTION)

#define TABFLAGS_CONTEXTBMP (SV_LBOXTAB_DYNAMIC | SV_LBOXTAB_ADJUST_CENTER)

#define TABFLAGS_CHECKBTN (SV_LBOXTAB_DYNAMIC |        \
                           SV_LBOXTAB_ADJUST_CENTER |  \
                           SV_LBOXTAB_PUSHABLE)

#define TAB_STARTPOS    2

// take care of GetTextOffset when doing changes
void SvTreeListBox::SetTabs()
{
    if( IsEditingActive() )
        EndEditing( true );
    nTreeFlags &= (~TREEFLAG_RECALCTABS);
    nFocusWidth = -1;
    const WinBits nStyle( GetStyle() );
    bool bHasButtons = (nStyle & WB_HASBUTTONS)!=0;
    bool bHasButtonsAtRoot = (nStyle & (WB_HASLINESATROOT |
                                              WB_HASBUTTONSATROOT))!=0;
    long nStartPos = TAB_STARTPOS;
    long nNodeWidthPixel = GetExpandedNodeBmp().GetSizePixel().Width();

    // pCheckButtonData->Width() knows nothing about the native checkbox width,
    // so we have mnCheckboxItemWidth which becomes valid when something is added.
    long nCheckWidth = 0;
    if( nTreeFlags & TREEFLAG_CHKBTN )
        nCheckWidth = mnCheckboxItemWidth;
    long nCheckWidthDIV2 = nCheckWidth / 2;

    long nContextWidth = nContextBmpWidthMax;
    long nContextWidthDIV2 = nContextWidth / 2;

    ClearTabList();

    int nCase = NO_BUTTONS;
    if( !(nTreeFlags & TREEFLAG_CHKBTN) )
    {
        if( bHasButtons )
            nCase = NODE_BUTTONS;
    }
    else
    {
        if( bHasButtons )
            nCase = NODE_AND_CHECK_BUTTONS;
         else
            nCase = CHECK_BUTTONS;
    }

    switch( nCase )
    {
        case NO_BUTTONS :
            nStartPos += nContextWidthDIV2;  // because of centering
            AddTab( nStartPos, TABFLAGS_CONTEXTBMP );
            nStartPos += nContextWidthDIV2;  // right edge of context bitmap
            // only set a distance if there are bitmaps
            if( nContextBmpWidthMax )
                nStartPos += 5; // distance context bitmap to text
            AddTab( nStartPos, TABFLAGS_TEXT );
            break;

        case NODE_BUTTONS :
            if( bHasButtonsAtRoot )
                nStartPos += ( nIndent + (nNodeWidthPixel/2) );
            else
                nStartPos += nContextWidthDIV2;
            AddTab( nStartPos, TABFLAGS_CONTEXTBMP );
            nStartPos += nContextWidthDIV2;  // right edge of context bitmap
            // only set a distance if there are bitmaps
            if( nContextBmpWidthMax )
                nStartPos += 5; // distance context bitmap to text
            AddTab( nStartPos, TABFLAGS_TEXT );
            break;

        case NODE_AND_CHECK_BUTTONS :
            if( bHasButtonsAtRoot )
                nStartPos += ( nIndent + nNodeWidthPixel );
            else
                nStartPos += nCheckWidthDIV2;
            AddTab( nStartPos, TABFLAGS_CHECKBTN );
            nStartPos += nCheckWidthDIV2;  // right edge of CheckButton
            nStartPos += 3;  // distance CheckButton to context bitmap
            nStartPos += nContextWidthDIV2;  // center of context bitmap
            AddTab( nStartPos, TABFLAGS_CONTEXTBMP );
            nStartPos += nContextWidthDIV2;  // right edge of context bitmap
            // only set a distance if there are bitmaps
            if( nContextBmpWidthMax )
                nStartPos += 5; // distance context bitmap to text
            AddTab( nStartPos, TABFLAGS_TEXT );
            break;

        case CHECK_BUTTONS :
            nStartPos += nCheckWidthDIV2;
            AddTab( nStartPos, TABFLAGS_CHECKBTN );
            nStartPos += nCheckWidthDIV2;  // right edge of CheckButton
            nStartPos += 3;  // distance CheckButton to context bitmap
            nStartPos += nContextWidthDIV2;  // center of context bitmap
            AddTab( nStartPos, TABFLAGS_CONTEXTBMP );
            nStartPos += nContextWidthDIV2;  // right edge of context bitmap
            // only set a distance if there are bitmaps
            if( nContextBmpWidthMax )
                nStartPos += 5; // distance context bitmap to text
            AddTab( nStartPos, TABFLAGS_TEXT );
            break;
    }
    pImp->NotifyTabsChanged();
}

void SvTreeListBox::InitEntry(SvTreeListEntry* pEntry,
    const OUString& aStr, const Image& aCollEntryBmp, const Image& aExpEntryBmp,
    SvLBoxButtonKind eButtonKind)
{
    SvLBoxButton* pButton;
    SvLBoxString* pString;
    SvLBoxContextBmp* pContextBmp;

    if( nTreeFlags & TREEFLAG_CHKBTN )
    {
        pButton= new SvLBoxButton( pEntry,eButtonKind,0,pCheckButtonData );
        pEntry->AddItem( pButton );
    }

    pContextBmp= new SvLBoxContextBmp(
        pEntry,0, aCollEntryBmp,aExpEntryBmp, mbContextBmpExpanded);
    pEntry->AddItem( pContextBmp );

    pString = new SvLBoxString( pEntry, 0, aStr );
    pEntry->AddItem( pString );
}

OUString SvTreeListBox::GetEntryText(SvTreeListEntry* pEntry) const
{
    DBG_ASSERT( pEntry, "SvTreeListBox::GetEntryText(): no entry" );
    SvLBoxString* pItem = static_cast<SvLBoxString*>(pEntry->GetFirstItem(SV_ITEM_ID_LBOXSTRING));
    DBG_ASSERT( pEntry, "SvTreeListBox::GetEntryText(): item not found" );
    return pItem->GetText();
}

const Image& SvTreeListBox::GetExpandedEntryBmp(const SvTreeListEntry* pEntry) const
{
    DBG_ASSERT(pEntry,"Entry?");
    const SvLBoxContextBmp* pItem = static_cast<const SvLBoxContextBmp*>(pEntry->GetFirstItem(SV_ITEM_ID_LBOXCONTEXTBMP));
    DBG_ASSERT(pItem,"GetContextBmp:Item not found");
    return pItem->GetBitmap2( );
}

const Image& SvTreeListBox::GetCollapsedEntryBmp( const SvTreeListEntry* pEntry ) const
{
    DBG_ASSERT(pEntry,"Entry?");
    const SvLBoxContextBmp* pItem = static_cast<const SvLBoxContextBmp*>(pEntry->GetFirstItem(SV_ITEM_ID_LBOXCONTEXTBMP));
    DBG_ASSERT(pItem,"GetContextBmp:Item not found");
    return pItem->GetBitmap1( );
}

IMPL_LINK_INLINE_START( SvTreeListBox, CheckButtonClick, SvLBoxButtonData *, pData )
{
    pHdlEntry = pData->GetActEntry();
    CheckButtonHdl();
    return 0;
}
IMPL_LINK_INLINE_END( SvTreeListBox, CheckButtonClick, SvLBoxButtonData *, pData )

SvTreeListEntry* SvTreeListBox::InsertEntry(
    const OUString& rText,
    SvTreeListEntry* pParent,
    bool bChildrenOnDemand, sal_uLong nPos,
    void* pUser,
    SvLBoxButtonKind eButtonKind
)
{
    nTreeFlags |= TREEFLAG_MANINS;

    const Image& rDefExpBmp = pImp->GetDefaultEntryExpBmp( );
    const Image& rDefColBmp = pImp->GetDefaultEntryColBmp( );

    aCurInsertedExpBmp = rDefExpBmp;
    aCurInsertedColBmp = rDefColBmp;

    SvTreeListEntry* pEntry = CreateEntry();
    pEntry->SetUserData( pUser );
    InitEntry( pEntry, rText, rDefColBmp, rDefExpBmp, eButtonKind );
    pEntry->EnableChildrenOnDemand( bChildrenOnDemand );

    if( !pParent )
        Insert( pEntry, nPos );
    else
        Insert( pEntry, pParent, nPos );

    aPrevInsertedExpBmp = rDefExpBmp;
    aPrevInsertedColBmp = rDefColBmp;

    nTreeFlags &= (~TREEFLAG_MANINS);

    return pEntry;
}

SvTreeListEntry* SvTreeListBox::InsertEntry( const OUString& rText,
    const Image& aExpEntryBmp, const Image& aCollEntryBmp,
    SvTreeListEntry* pParent, bool bChildrenOnDemand, sal_uLong nPos, void* pUser,
    SvLBoxButtonKind eButtonKind )
{
    nTreeFlags |= TREEFLAG_MANINS;

    aCurInsertedExpBmp = aExpEntryBmp;
    aCurInsertedColBmp = aCollEntryBmp;

    SvTreeListEntry* pEntry = CreateEntry();
    pEntry->SetUserData( pUser );
    InitEntry( pEntry, rText, aCollEntryBmp, aExpEntryBmp, eButtonKind );

    pEntry->EnableChildrenOnDemand( bChildrenOnDemand );

    if( !pParent )
        Insert( pEntry, nPos );
    else
        Insert( pEntry, pParent, nPos );

    aPrevInsertedExpBmp = aExpEntryBmp;
    aPrevInsertedColBmp = aCollEntryBmp;

    nTreeFlags &= (~TREEFLAG_MANINS);

    return pEntry;
}

void SvTreeListBox::SetEntryText(SvTreeListEntry* pEntry, const OUString& rStr)
{
    SvLBoxString* pItem = static_cast<SvLBoxString*>(pEntry->GetFirstItem(SV_ITEM_ID_LBOXSTRING));
    DBG_ASSERT(pItem,"SetText:Item not found");
    pItem->SetText(rStr);
    pItem->InitViewData( this, pEntry, 0 );
    GetModel()->InvalidateEntry( pEntry );
}

void SvTreeListBox::SetExpandedEntryBmp( SvTreeListEntry* pEntry, const Image& aBmp )
{
    SvLBoxContextBmp* pItem = static_cast<SvLBoxContextBmp*>(pEntry->GetFirstItem(SV_ITEM_ID_LBOXCONTEXTBMP));

    DBG_ASSERT(pItem,"SetExpBmp:Item not found");
    pItem->SetBitmap2( aBmp );

    GetModel()->InvalidateEntry( pEntry );
    SetEntryHeight( pEntry );
    Size aSize = aBmp.GetSizePixel();
    short nWidth = pImp->UpdateContextBmpWidthVector( pEntry, (short)aSize.Width() );
    if( nWidth > nContextBmpWidthMax )
    {
        nContextBmpWidthMax = nWidth;
        SetTabs();
    }
}

void SvTreeListBox::SetCollapsedEntryBmp(SvTreeListEntry* pEntry,const Image& aBmp )
{
    SvLBoxContextBmp* pItem = static_cast<SvLBoxContextBmp*>(pEntry->GetFirstItem(SV_ITEM_ID_LBOXCONTEXTBMP));

    DBG_ASSERT(pItem,"SetExpBmp:Item not found");
    pItem->SetBitmap1( aBmp );

    GetModel()->InvalidateEntry( pEntry );
    SetEntryHeight( pEntry );
    Size aSize = aBmp.GetSizePixel();
    short nWidth = pImp->UpdateContextBmpWidthVector( pEntry, (short)aSize.Width() );
    if( nWidth > nContextBmpWidthMax )
    {
        nContextBmpWidthMax = nWidth;
        SetTabs();
    }
}

void SvTreeListBox::ImpEntryInserted( SvTreeListEntry* pEntry )
{

    SvTreeListEntry* pParent = (SvTreeListEntry*)pModel->GetParent( pEntry );
    if( pParent )
    {
        sal_uInt16 nFlags = pParent->GetFlags();
        nFlags &= ~SV_ENTRYFLAG_NO_NODEBMP;
        pParent->SetFlags( nFlags );
    }

    if(!((nTreeFlags & TREEFLAG_MANINS) &&
         (aPrevInsertedExpBmp == aCurInsertedExpBmp)  &&
         (aPrevInsertedColBmp == aCurInsertedColBmp) ))
    {
        Size aSize = GetCollapsedEntryBmp( pEntry ).GetSizePixel();
        if( aSize.Width() > nContextBmpWidthMax )
        {
            nContextBmpWidthMax = (short)aSize.Width();
            nTreeFlags |= TREEFLAG_RECALCTABS;
        }
        aSize = GetExpandedEntryBmp( pEntry ).GetSizePixel();
        if( aSize.Width() > nContextBmpWidthMax )
        {
            nContextBmpWidthMax = (short)aSize.Width();
            nTreeFlags |= TREEFLAG_RECALCTABS;
        }
    }
    SetEntryHeight( (SvTreeListEntry*)pEntry );

    if( nTreeFlags & TREEFLAG_CHKBTN )
    {
        SvLBoxButton* pItem = static_cast<SvLBoxButton*>(pEntry->GetFirstItem(SV_ITEM_ID_LBOXBUTTON));
        if( pItem )
        {
            long nWidth = pItem->GetSize(this, pEntry).Width();
            if( mnCheckboxItemWidth < nWidth )
            {
                mnCheckboxItemWidth = nWidth;
                nTreeFlags |= TREEFLAG_RECALCTABS;
            }
        }
    }
}



void SvTreeListBox::SetCheckButtonState( SvTreeListEntry* pEntry, SvButtonState eState)
{
    if( nTreeFlags & TREEFLAG_CHKBTN )
    {
        SvLBoxButton* pItem = static_cast<SvLBoxButton*>(pEntry->GetFirstItem(SV_ITEM_ID_LBOXBUTTON));
        if(!(pItem && pItem->CheckModification()))
            return ;
        switch( eState )
        {
            case SV_BUTTON_CHECKED:
                pItem->SetStateChecked();
                break;

            case SV_BUTTON_UNCHECKED:
                pItem->SetStateUnchecked();
                break;

            case SV_BUTTON_TRISTATE:
                pItem->SetStateTristate();
                break;
        }
        InvalidateEntry( pEntry );
    }
}

void SvTreeListBox::SetCheckButtonInvisible( SvTreeListEntry* pEntry)
{
    if( nTreeFlags & TREEFLAG_CHKBTN )
    {
        SvLBoxButton* pItem = static_cast<SvLBoxButton*>(pEntry->GetFirstItem(SV_ITEM_ID_LBOXBUTTON));
        pItem->SetStateInvisible();
        InvalidateEntry( pEntry );
    }
}

SvButtonState SvTreeListBox::GetCheckButtonState( SvTreeListEntry* pEntry ) const
{
    SvButtonState eState = SV_BUTTON_UNCHECKED;
    if( nTreeFlags & TREEFLAG_CHKBTN )
    {
        SvLBoxButton* pItem = static_cast<SvLBoxButton*>(pEntry->GetFirstItem(SV_ITEM_ID_LBOXBUTTON));
        if(!pItem)
            return SV_BUTTON_TRISTATE;
        sal_uInt16 nButtonFlags = pItem->GetButtonFlags();
        eState = pCheckButtonData->ConvertToButtonState( nButtonFlags );
    }
    return eState;
}

void SvTreeListBox::CheckButtonHdl()
{
    aCheckButtonHdl.Call( this );
    if ( pCheckButtonData )
        pImp->CallEventListeners( VCLEVENT_CHECKBOX_TOGGLE, (void*)pCheckButtonData->GetActEntry() );
}


// TODO: Currently all data is cloned so that they conform to the default tree
// view format. Actually, the model should be used as a reference here. This
// leads to us _not_ calling SvTreeListEntry::Clone, but only its base class
// SvTreeListEntry.


SvTreeListEntry* SvTreeListBox::CloneEntry( SvTreeListEntry* pSource )
{
    OUString aStr;
    Image aCollEntryBmp;
    Image aExpEntryBmp;
    SvLBoxButtonKind eButtonKind = SvLBoxButtonKind_enabledCheckbox;

    SvLBoxString* pStringItem = static_cast<SvLBoxString*>(pSource->GetFirstItem(SV_ITEM_ID_LBOXSTRING));
    if( pStringItem )
        aStr = pStringItem->GetText();
    SvLBoxContextBmp* pBmpItem = static_cast<SvLBoxContextBmp*>(pSource->GetFirstItem(SV_ITEM_ID_LBOXCONTEXTBMP));
    if( pBmpItem )
    {
        aCollEntryBmp = pBmpItem->GetBitmap1( );
        aExpEntryBmp  = pBmpItem->GetBitmap2( );
    }
    SvLBoxButton* pButtonItem = static_cast<SvLBoxButton*>(pSource->GetFirstItem(SV_ITEM_ID_LBOXBUTTON));
    if( pButtonItem )
        eButtonKind = pButtonItem->GetKind();
    SvTreeListEntry* pClone = CreateEntry();
    InitEntry( pClone, aStr, aCollEntryBmp, aExpEntryBmp, eButtonKind );
    pClone->SvTreeListEntry::Clone( pSource );
    pClone->EnableChildrenOnDemand( pSource->HasChildrenOnDemand() );
    pClone->SetUserData( pSource->GetUserData() );

    return pClone;
}

void SvTreeListBox::SetIndent( short nNewIndent )
{
    nIndent = nNewIndent;
    SetTabs();
    if( IsUpdateMode() )
        Invalidate();
}

const Image& SvTreeListBox::GetDefaultExpandedEntryBmp( ) const
{
    return pImp->GetDefaultEntryExpBmp( );
}

const Image& SvTreeListBox::GetDefaultCollapsedEntryBmp( ) const
{
    return pImp->GetDefaultEntryColBmp( );
}

void SvTreeListBox::SetDefaultExpandedEntryBmp( const Image& aBmp )
{
    Size aSize = aBmp.GetSizePixel();
    if( aSize.Width() > nContextBmpWidthMax )
        nContextBmpWidthMax = (short)aSize.Width();
    SetTabs();

    pImp->SetDefaultEntryExpBmp( aBmp );
}

void SvTreeListBox::SetDefaultCollapsedEntryBmp( const Image& aBmp )
{
    Size aSize = aBmp.GetSizePixel();
    if( aSize.Width() > nContextBmpWidthMax )
        nContextBmpWidthMax = (short)aSize.Width();
    SetTabs();

    pImp->SetDefaultEntryColBmp( aBmp );
}

void SvTreeListBox::EnableCheckButton( SvLBoxButtonData* pData )
{
    DBG_ASSERT(!GetEntryCount(),"EnableCheckButton: Entry count != 0");
    if( !pData )
        nTreeFlags &= (~TREEFLAG_CHKBTN);
    else
    {
        SetCheckButtonData( pData );
        nTreeFlags |= TREEFLAG_CHKBTN;
        pData->SetLink( LINK(this, SvTreeListBox, CheckButtonClick));
    }

    SetTabs();
    if( IsUpdateMode() )
        Invalidate();
}

void SvTreeListBox::SetCheckButtonData( SvLBoxButtonData* pData )
{
    if ( pData )
        pCheckButtonData = pData;
}

const Image& SvTreeListBox::GetDefaultExpandedNodeImage( )
{
    return SvImpLBox::GetDefaultExpandedNodeImage( );
}

const Image& SvTreeListBox::GetDefaultCollapsedNodeImage( )
{
    return SvImpLBox::GetDefaultCollapsedNodeImage( );
}

void SvTreeListBox::SetNodeBitmaps( const Image& rCollapsedNodeBmp, const Image& rExpandedNodeBmp )
{
    SetExpandedNodeBmp( rExpandedNodeBmp );
    SetCollapsedNodeBmp( rCollapsedNodeBmp );
    SetTabs();
}

bool SvTreeListBox::EditingEntry( SvTreeListEntry*, Selection& )
{
    return true;
}

bool SvTreeListBox::EditedEntry( SvTreeListEntry* /*pEntry*/,const OUString& /*rNewText*/)
{
    return true;
}

void SvTreeListBox::EnableInplaceEditing( bool bOn )
{
    if (bOn)
        nImpFlags |= SVLBOX_EDT_ENABLED;
    else
        nImpFlags &= ~SVLBOX_EDT_ENABLED;
}

void SvTreeListBox::KeyInput( const KeyEvent& rKEvt )
{
    // under OS/2, we get key up/down even while editing
    if( IsEditingActive() )
        return;

    nImpFlags |= SVLBOX_IS_TRAVELSELECT;

    if( !pImp->KeyInput( rKEvt ) )
    {
        bool bHandled = HandleKeyInput( rKEvt );
        if ( !bHandled )
            Control::KeyInput( rKEvt );
    }

    nImpFlags &= ~SVLBOX_IS_TRAVELSELECT;
}

void SvTreeListBox::RequestingChildren( SvTreeListEntry* pParent )
{
    if( !pParent->HasChildren() )
        InsertEntry( OUString("<dummy>"), pParent, false, TREELIST_APPEND );
}

void SvTreeListBox::GetFocus()
{
    //If there is no item in the tree, draw focus.
    if( !First())
    {
        Invalidate();
    }
    pImp->GetFocus();
    Control::GetFocus();

    SvTreeListEntry* pEntry = FirstSelected();
    if ( !pEntry )
    {
        pEntry = pImp->GetCurrentEntry();
    }
    if (pImp->pCursor)
    {
        if (pEntry != pImp->pCursor)
            pEntry = pImp->pCursor;
    }
    if ( pEntry )
        pImp->CallEventListeners( VCLEVENT_LISTBOX_TREEFOCUS, pEntry );

}

void SvTreeListBox::LoseFocus()
{
    //If there is no item in the tree, delete visual focus.
    if( !First())
    {
        Invalidate();
    }
    pImp->LoseFocus();
    Control::LoseFocus();
}

void SvTreeListBox::ModelHasCleared()
{
    pImp->pCursor = 0; // else we crash in GetFocus when editing in-place
    delete pEdCtrl;
    pEdCtrl = NULL;
    pImp->Clear();
    nFocusWidth = -1;

    nContextBmpWidthMax = 0;
    SetDefaultExpandedEntryBmp( GetDefaultExpandedEntryBmp() );
    SetDefaultCollapsedEntryBmp( GetDefaultCollapsedEntryBmp() );

    if( !(nTreeFlags & TREEFLAG_FIXEDHEIGHT ))
        nEntryHeight = 0;
    AdjustEntryHeight( GetFont() );
    AdjustEntryHeight( GetDefaultExpandedEntryBmp() );
    AdjustEntryHeight( GetDefaultCollapsedEntryBmp() );

    SvListView::ModelHasCleared();
}

void SvTreeListBox::ShowTargetEmphasis( SvTreeListEntry* pEntry, bool /*bShow*/ )
{
    pImp->PaintDDCursor( pEntry );
}

void SvTreeListBox::ScrollOutputArea( short nDeltaEntries )
{
    if( !nDeltaEntries || !pImp->aVerSBar.IsVisible() )
        return;

    long nThumb = pImp->aVerSBar.GetThumbPos();
    long nMax = pImp->aVerSBar.GetRange().Max();

    NotifyBeginScroll();
    if( nDeltaEntries < 0 )
    {
        // move window up
        nDeltaEntries *= -1;
        long nVis = pImp->aVerSBar.GetVisibleSize();
        long nTemp = nThumb + nVis;
        if( nDeltaEntries > (nMax - nTemp) )
            nDeltaEntries = (short)(nMax - nTemp);
        pImp->PageDown( (sal_uInt16)nDeltaEntries );
    }
    else
    {
        if( nDeltaEntries > nThumb )
            nDeltaEntries = (short)nThumb;
        pImp->PageUp( (sal_uInt16)nDeltaEntries );
    }
    pImp->SyncVerThumb();
    NotifyEndScroll();
}

void SvTreeListBox::ScrollToAbsPos( long nPos )
{
    pImp->ScrollToAbsPos( nPos );
}

void SvTreeListBox::SetSelectionMode( SelectionMode eSelectMode )
{
    eSelMode = eSelectMode;
    pImp->SetSelectionMode( eSelectMode );
}

void SvTreeListBox::SetDragDropMode( DragDropMode nDDMode )
{
    nDragDropMode = nDDMode;
    pImp->SetDragDropMode( nDDMode );
}

short SvTreeListBox::GetHeightOffset(const Image& rBmp, Size& aSizeLogic )
{
    short nOffset = 0;
    aSizeLogic = rBmp.GetSizePixel();
    if( GetEntryHeight() > aSizeLogic.Height() )
        nOffset = ( GetEntryHeight() - (short)aSizeLogic.Height()) / 2;
    return nOffset;
}

short SvTreeListBox::GetHeightOffset(const vcl::Font& /* rFont */, Size& aSizeLogic )
{
    short nOffset = 0;
    aSizeLogic = Size(GetTextWidth(OUString('X')), GetTextHeight());
    if( GetEntryHeight() > aSizeLogic.Height() )
        nOffset = ( GetEntryHeight() - (short)aSizeLogic.Height()) / 2;
    return nOffset;
}

void SvTreeListBox::SetEntryHeight( SvTreeListEntry* pEntry )
{
    short nHeight, nHeightMax=0;
    sal_uInt16 nCount = pEntry->ItemCount();
    sal_uInt16 nCur = 0;
    SvViewDataEntry* pViewData = GetViewDataEntry( pEntry );
    while( nCur < nCount )
    {
        SvLBoxItem* pItem = pEntry->GetItem( nCur );
        nHeight = (short)(pItem->GetSize( pViewData, nCur ).Height());
        if( nHeight > nHeightMax )
            nHeightMax = nHeight;
        nCur++;
    }

    if( nHeightMax > nEntryHeight )
    {
        nEntryHeight = nHeightMax;
        Control::SetFont( GetFont() );
        pImp->SetEntryHeight( nHeightMax );
    }
}

void SvTreeListBox::SetEntryHeight( short nHeight, bool bAlways )
{

    if( bAlways || nHeight > nEntryHeight )
    {
        nEntryHeight = nHeight;
        if( nEntryHeight )
            nTreeFlags |= TREEFLAG_FIXEDHEIGHT;
        else
            nTreeFlags &= ~TREEFLAG_FIXEDHEIGHT;
        Control::SetFont( GetFont() );
        pImp->SetEntryHeight( nHeight );
    }
}


void SvTreeListBox::AdjustEntryHeight( const Image& rBmp )
{
    Size aSize;
    GetHeightOffset( rBmp, aSize );
    if( aSize.Height() > nEntryHeight )
    {
        nEntryHeight = (short)aSize.Height() + nEntryHeightOffs;
        pImp->SetEntryHeight( nEntryHeight );
    }
}

void SvTreeListBox::AdjustEntryHeight( const vcl::Font& rFont )
{
    Size aSize;
    GetHeightOffset( rFont, aSize );
    if( aSize.Height()  >  nEntryHeight )
    {
        nEntryHeight = (short)aSize.Height() + nEntryHeightOffs;
        pImp->SetEntryHeight( nEntryHeight );
    }
}

bool SvTreeListBox::Expand( SvTreeListEntry* pParent )
{
    pHdlEntry = pParent;
    bool bExpanded = false;
    sal_uInt16 nFlags;

    if( pParent->HasChildrenOnDemand() )
        RequestingChildren( pParent );
    if( pParent->HasChildren() )
    {
        nImpFlags |= SVLBOX_IS_EXPANDING;
        if( ExpandingHdl() )
        {
            bExpanded = true;
            ExpandListEntry( pParent );
            pImp->EntryExpanded( pParent );
            pHdlEntry = pParent;
            ExpandedHdl();
        }
        nFlags = pParent->GetFlags();
        nFlags &= ~SV_ENTRYFLAG_NO_NODEBMP;
        nFlags |= SV_ENTRYFLAG_HAD_CHILDREN;
        pParent->SetFlags( nFlags );
    }
    else
    {
        nFlags = pParent->GetFlags();
        nFlags |= SV_ENTRYFLAG_NO_NODEBMP;
        pParent->SetFlags( nFlags );
        GetModel()->InvalidateEntry( pParent ); // repaint
    }

    // #i92103#
    if ( bExpanded )
    {
        pImp->CallEventListeners( VCLEVENT_ITEM_EXPANDED, pParent );
    }

    return bExpanded;
}

bool SvTreeListBox::Collapse( SvTreeListEntry* pParent )
{
    nImpFlags &= ~SVLBOX_IS_EXPANDING;
    pHdlEntry = pParent;
    bool bCollapsed = false;

    if( ExpandingHdl() )
    {
        bCollapsed = true;
        pImp->CollapsingEntry( pParent );
        CollapseListEntry( pParent );
        pImp->EntryCollapsed( pParent );
        pHdlEntry = pParent;
        ExpandedHdl();
    }

    // #i92103#
    if ( bCollapsed )
    {
        pImp->CallEventListeners( VCLEVENT_ITEM_COLLAPSED, pParent );
    }

    return bCollapsed;
}

bool SvTreeListBox::Select( SvTreeListEntry* pEntry, bool bSelect )
{
    DBG_ASSERT(pEntry,"Select: Null-Ptr");
    bool bRetVal = SelectListEntry( pEntry, bSelect );
    DBG_ASSERT(IsSelected(pEntry)==bSelect,"Select failed");
    if( bRetVal )
    {
        pImp->EntrySelected( pEntry, bSelect );
        pHdlEntry = pEntry;
        if( bSelect )
        {
            SelectHdl();
            CallEventListeners( VCLEVENT_LISTBOX_TREESELECT, pEntry);
        }
        else
            DeselectHdl();
    }
    return bRetVal;
}

sal_uLong SvTreeListBox::SelectChildren( SvTreeListEntry* pParent, bool bSelect )
{
    pImp->DestroyAnchor();
    sal_uLong nRet = 0;
    if( !pParent->HasChildren() )
        return 0;
    sal_uInt16 nRefDepth = pModel->GetDepth( pParent );
    SvTreeListEntry* pChild = FirstChild( pParent );
    do {
        nRet++;
        Select( pChild, bSelect );
        pChild = Next( pChild );
    } while( pChild && pModel->GetDepth( pChild ) > nRefDepth );
    return nRet;
}

void SvTreeListBox::SelectAll( bool bSelect, bool )
{
    pImp->SelAllDestrAnch(
        bSelect,
        true,       // delete anchor,
        true );     // even when using SINGLE_SELECTION, deselect the cursor
}

void SvTreeListBox::ModelHasInsertedTree( SvTreeListEntry* pEntry )
{
    sal_uInt16 nRefDepth = pModel->GetDepth( (SvTreeListEntry*)pEntry );
    SvTreeListEntry* pTmp = (SvTreeListEntry*)pEntry;
    do
    {
        ImpEntryInserted( pTmp );
        pTmp = Next( pTmp );
    } while( pTmp && nRefDepth < pModel->GetDepth( pTmp ) );
    pImp->TreeInserted( (SvTreeListEntry*)pEntry );
}

void SvTreeListBox::ModelHasInserted( SvTreeListEntry* pEntry )
{
    ImpEntryInserted( (SvTreeListEntry*)pEntry );
    pImp->EntryInserted( (SvTreeListEntry*)pEntry );
}

void SvTreeListBox::ModelIsMoving(SvTreeListEntry* pSource,
                                        SvTreeListEntry* /* pTargetParent */,
                                        sal_uLong /* nChildPos */ )
{
    pImp->MovingEntry( (SvTreeListEntry*)pSource );
}

void SvTreeListBox::ModelHasMoved( SvTreeListEntry* pSource )
{
    pImp->EntryMoved( (SvTreeListEntry*)pSource );
}

void SvTreeListBox::ModelIsRemoving( SvTreeListEntry* pEntry )
{
    if(pEdEntry == pEntry)
        pEdEntry = NULL;

    pImp->RemovingEntry( (SvTreeListEntry*)pEntry );
    NotifyRemoving( (SvTreeListEntry*)pEntry );
}

void SvTreeListBox::ModelHasRemoved( SvTreeListEntry* pEntry  )
{
    if ( pEntry == pHdlEntry)
        pHdlEntry = NULL;
    pImp->EntryRemoved();
}

void SvTreeListBox::SetCollapsedNodeBmp( const Image& rBmp)
{
    AdjustEntryHeight( rBmp );
    pImp->SetCollapsedNodeBmp( rBmp );
}

void SvTreeListBox::SetExpandedNodeBmp( const Image& rBmp )
{
    AdjustEntryHeight( rBmp );
    pImp->SetExpandedNodeBmp( rBmp );
}


void SvTreeListBox::SetFont( const vcl::Font& rFont )
{
    vcl::Font aTempFont( rFont );
    vcl::Font aOrigFont( GetFont() );
    aTempFont.SetTransparent( true );
    if (aTempFont == aOrigFont)
        return;
    Control::SetFont( aTempFont );

    aTempFont.SetColor(aOrigFont.GetColor());
    aTempFont.SetFillColor(aOrigFont.GetFillColor());
    aTempFont.SetTransparent(aOrigFont.IsTransparent());

    if (aTempFont == aOrigFont)
        return;

    AdjustEntryHeightAndRecalc( GetFont() );
}

void SvTreeListBox::AdjustEntryHeightAndRecalc( const vcl::Font& rFont )
{
    AdjustEntryHeight( rFont );
    // always invalidate, else things go wrong in SetEntryHeight
    RecalcViewData();
}

void SvTreeListBox::Paint( const Rectangle& rRect )
{
    Control::Paint( rRect );
    if( nTreeFlags & TREEFLAG_RECALCTABS )
        SetTabs();
    pImp->Paint( rRect );
    //Add visual focus draw
    if( !First() )
    {
        if( HasFocus() )
        {
            long tempHeight = GetTextHeight();
            Rectangle tempRect(
                                Point(0,0),Size(GetSizePixel().Width(),tempHeight)
                               );
            ShowFocus(tempRect);
        }

        else{
            HideFocus();
        }
    }
}

void SvTreeListBox::MouseButtonDown( const MouseEvent& rMEvt )
{
    pImp->MouseButtonDown( rMEvt );
}

void SvTreeListBox::MouseButtonUp( const MouseEvent& rMEvt )
{
    pImp->MouseButtonUp( rMEvt );
}

void SvTreeListBox::MouseMove( const MouseEvent& rMEvt )
{
    pImp->MouseMove( rMEvt );
}


void SvTreeListBox::SetUpdateMode( bool bUpdate )
{
    pImp->SetUpdateMode( bUpdate );
}

void SvTreeListBox::SetSpaceBetweenEntries( short nOffsLogic )
{
    if( nOffsLogic != nEntryHeightOffs )
    {
        nEntryHeight = nEntryHeight - nEntryHeightOffs;
        nEntryHeightOffs = (short)nOffsLogic;
        nEntryHeight = nEntryHeight + nOffsLogic;
        AdjustEntryHeightAndRecalc( GetFont() );
        pImp->SetEntryHeight( nEntryHeight );
    }
}

void SvTreeListBox::SetCursor( SvTreeListEntry* pEntry, bool bForceNoSelect )
{
    pImp->SetCursor(pEntry, bForceNoSelect);
}

void SvTreeListBox::SetCurEntry( SvTreeListEntry* pEntry )
{
    pImp->SetCurEntry( pEntry );
}

Image SvTreeListBox::GetExpandedNodeBmp( ) const
{
    return pImp->GetExpandedNodeBmp( );
}

Point SvTreeListBox::GetEntryPosition( SvTreeListEntry* pEntry ) const
{
    return pImp->GetEntryPosition( pEntry );
}

void SvTreeListBox::ShowEntry( SvTreeListEntry* pEntry )
{
    MakeVisible( pEntry );
}

void SvTreeListBox::MakeVisible( SvTreeListEntry* pEntry )
{
    pImp->MakeVisible(pEntry);
}

void SvTreeListBox::MakeVisible( SvTreeListEntry* pEntry, bool bMoveToTop )
{
    pImp->MakeVisible( pEntry, bMoveToTop );
}

void SvTreeListBox::ModelHasEntryInvalidated( SvTreeListEntry* pEntry )
{

    // reinitialize the separate items of the entries
    sal_uInt16 nCount = ((SvTreeListEntry*)pEntry)->ItemCount();
    for( sal_uInt16 nIdx = 0; nIdx < nCount; nIdx++ )
    {
        SvLBoxItem* pItem = ((SvTreeListEntry*)pEntry)->GetItem( nIdx );
        pItem->InitViewData( this, (SvTreeListEntry*)pEntry, 0 );
    }

    // repaint
    pImp->InvalidateEntry( (SvTreeListEntry*)pEntry );
}

void SvTreeListBox::EditItemText( SvTreeListEntry* pEntry, SvLBoxString* pItem,
    const Selection& rSelection )
{
    DBG_ASSERT(pEntry&&pItem,"EditItemText: Bad params");
    if( IsSelected( pEntry ))
    {
        pImp->ShowCursor( false );
        SelectListEntry( pEntry, false );
        PaintEntry( pEntry );
        SelectListEntry( pEntry, true );
        pImp->ShowCursor( true );
    }
    pEdEntry = pEntry;
    pEdItem = pItem;
    SvLBoxTab* pTab = GetTab( pEntry, pItem );
    DBG_ASSERT(pTab,"EditItemText:Tab not found");

    Size aItemSize( pItem->GetSize(this, pEntry) );
    Point aPos = GetEntryPosition( pEntry );
    aPos.Y() += ( nEntryHeight - aItemSize.Height() ) / 2;
    aPos.X() = GetTabPos( pEntry, pTab );
    long nOutputWidth = pImp->GetOutputSize().Width();
    Size aSize( nOutputWidth - aPos.X(), aItemSize.Height() );
    sal_uInt16 nPos = std::find( aTabs.begin(), aTabs.end(), pTab ) - aTabs.begin();
    if( nPos+1 < (sal_uInt16)aTabs.size() )
    {
        SvLBoxTab* pRightTab = aTabs[ nPos + 1 ];
        long nRight = GetTabPos( pEntry, pRightTab );
        if( nRight <= nOutputWidth )
            aSize.Width() = nRight - aPos.X();
    }
    Point aOrigin( GetMapMode().GetOrigin() );
    aPos += aOrigin; // convert to win coordinates
    aSize.Width() -= aOrigin.X();
    Rectangle aRect( aPos, aSize );
    EditText( pItem->GetText(), aRect, rSelection );
}

void SvTreeListBox::EditEntry( SvTreeListEntry* pEntry )
{
    pImp->aEditClickPos = Point( -1, -1 );
    ImplEditEntry( pEntry );
}

void SvTreeListBox::ImplEditEntry( SvTreeListEntry* pEntry )
{
    if( IsEditingActive() )
        EndEditing();
    if( !pEntry )
        pEntry = GetCurEntry();
    if( pEntry )
    {
        long nClickX = pImp->aEditClickPos.X();
        bool bIsMouseTriggered = nClickX >= 0;

        SvLBoxString* pItem = NULL;
        sal_uInt16 nCount = pEntry->ItemCount();
        long nTabPos, nNextTabPos = 0;
        for( sal_uInt16 i = 0 ; i < nCount ; i++ )
        {
            SvLBoxItem* pTmpItem = pEntry->GetItem( i );
            if (pTmpItem->GetType() != SV_ITEM_ID_LBOXSTRING)
                continue;

            SvLBoxTab* pTab = GetTab( pEntry, pTmpItem );
            nNextTabPos = -1;
            if( i < nCount - 1 )
            {
                SvLBoxItem* pNextItem = pEntry->GetItem( i + 1 );
                SvLBoxTab* pNextTab = GetTab( pEntry, pNextItem );
                nNextTabPos = pNextTab->GetPos();
            }

            if( pTab && pTab->IsEditable() )
            {
                nTabPos = pTab->GetPos();
                if( !bIsMouseTriggered || (nClickX > nTabPos && (nNextTabPos == -1 || nClickX < nNextTabPos ) ) )
                {
                    pItem = static_cast<SvLBoxString*>( pTmpItem );
                    break;
                }
            }
        }

        Selection aSel( SELECTION_MIN, SELECTION_MAX );
        if( pItem && EditingEntry( pEntry, aSel ) )
        {
            SelectAll( false );
            MakeVisible( pEntry );
            EditItemText( pEntry, pItem, aSel );
        }
    }
}

bool SvTreeListBox::AreChildrenTransient() const
{
    return pImp->AreChildrenTransient();
}

void SvTreeListBox::SetChildrenNotTransient()
{
    pImp->SetChildrenNotTransient();
}

void SvTreeListBox::EditedText( const OUString& rStr )

{
    if(pEdEntry) // we have to check if this entry is null that means that it is removed while editing
    {
        if( EditedEntry( pEdEntry, rStr ) )
        {
            static_cast<SvLBoxString*>(pEdItem)->SetText( rStr );
            pModel->InvalidateEntry( pEdEntry );
        }
        if( GetSelectionCount() == 0 )
            Select( pEdEntry );
        if( GetSelectionMode() == MULTIPLE_SELECTION && !GetCurEntry() )
            SetCurEntry( pEdEntry );
    }
}

SvTreeListEntry* SvTreeListBox::GetDropTarget( const Point& rPos )
{
    // scroll
    if( rPos.Y() < 12 )
    {
        ImplShowTargetEmphasis(pTargetEntry, false);
        ScrollOutputArea( +1 );
    }
    else
    {
        Size aSize( pImp->GetOutputSize() );
        if( rPos.Y() > aSize.Height() - 12 )
        {
            ImplShowTargetEmphasis(pTargetEntry, false);
            ScrollOutputArea( -1 );
        }
    }

    SvTreeListEntry* pTarget = pImp->GetEntry( rPos );
    // when dropping in a vacant space, use the last entry
    if( !pTarget )
        return (SvTreeListEntry*)LastVisible();
    else if( (GetDragDropMode() & SV_DRAGDROP_ENABLE_TOP) &&
             pTarget == First() && rPos.Y() < 6 )
        return 0;

    return pTarget;
}


SvTreeListEntry* SvTreeListBox::GetEntry( const Point& rPos, bool bHit ) const
{
    SvTreeListEntry* pEntry = pImp->GetEntry( rPos );
    if( pEntry && bHit )
    {
        long nLine = pImp->GetEntryLine( pEntry );
        if( !(pImp->EntryReallyHit( pEntry, rPos, nLine)) )
            return 0;
    }
    return pEntry;
}

SvTreeListEntry* SvTreeListBox::GetCurEntry() const
{
    return pImp->GetCurEntry();
}

void SvTreeListBox::ImplInitStyle()
{

    const WinBits nWindowStyle = GetStyle();

    nTreeFlags |= TREEFLAG_RECALCTABS;
    if( nWindowStyle & WB_SORT )
    {
        GetModel()->SetSortMode( SortAscending );
        GetModel()->SetCompareHdl( LINK(this,SvTreeListBox,DefaultCompare));
    }
    else
    {
        GetModel()->SetSortMode( SortNone );
        GetModel()->SetCompareHdl( Link() );
    }
    pImp->SetStyle( nWindowStyle );
    pImp->Resize();
    Invalidate();
}

void SvTreeListBox::PaintEntry( SvTreeListEntry* pEntry )
{
    DBG_ASSERT(pEntry,"PaintEntry:No Entry");
    if( pEntry )
        pImp->PaintEntry( pEntry );
}

void SvTreeListBox::InvalidateEntry( SvTreeListEntry* pEntry )
{
    DBG_ASSERT(pEntry,"InvalidateEntry:No Entry");
    if( pEntry )
    {
        GetModel()->InvalidateEntry( pEntry );
    }
}

long SvTreeListBox::PaintEntry(SvTreeListEntry* pEntry,long nLine,sal_uInt16 nTabFlags)
{
    return PaintEntry1(pEntry,nLine,nTabFlags);
}

long SvTreeListBox::PaintEntry1(SvTreeListEntry* pEntry,long nLine,sal_uInt16 nTabFlags,
    bool bHasClipRegion )
{

    Rectangle aRect; // multi purpose

    bool bHorSBar = pImp->HasHorScrollBar();
    PreparePaint( pEntry );

    pImp->UpdateContextBmpWidthMax( pEntry );

    if( nTreeFlags & TREEFLAG_RECALCTABS )
        SetTabs();

    short nTempEntryHeight = GetEntryHeight();
    long nWidth = pImp->GetOutputSize().Width();

    // Did we turn on the scrollbar within PreparePaints? If yes, we have to set
    // the ClipRegion anew.
    if( !bHorSBar && pImp->HasHorScrollBar() )
        SetClipRegion( vcl::Region(pImp->GetClipRegionRect()) );

    Point aEntryPos( GetMapMode().GetOrigin() );
    aEntryPos.X() *= -1; // conversion document coordinates
    long nMaxRight = nWidth + aEntryPos.X() - 1;

    Color aBackupTextColor( GetTextColor() );
    vcl::Font aBackupFont( GetFont() );
    Color aBackupColor = GetFillColor();

    bool bCurFontIsSel = false;
    bool bInUse = pEntry->HasInUseEmphasis();
    // if a ClipRegion was set from outside, we don't have to reset it
    const WinBits nWindowStyle = GetStyle();
    const bool bResetClipRegion = !bHasClipRegion;
    const bool bHideSelection = (nWindowStyle & WB_HIDESELECTION) !=0 && !HasFocus();
    const StyleSettings& rSettings = GetSettings().GetStyleSettings();

    vcl::Font aHighlightFont( GetFont() );
    const Color aHighlightTextColor( rSettings.GetHighlightTextColor() );
    aHighlightFont.SetColor( aHighlightTextColor );

    Size aRectSize( 0, nTempEntryHeight );

    if( !bHasClipRegion && nWindowStyle & WB_HSCROLL )
    {
        SetClipRegion( vcl::Region(pImp->GetClipRegionRect()) );
        bHasClipRegion = true;
    }

    SvViewDataEntry* pViewDataEntry = GetViewDataEntry( pEntry );

    sal_uInt16 nTabCount = aTabs.size();
    sal_uInt16 nItemCount = pEntry->ItemCount();
    sal_uInt16 nCurTab = 0;
    sal_uInt16 nCurItem = 0;

    while( nCurTab < nTabCount && nCurItem < nItemCount )
    {
        SvLBoxTab* pTab = aTabs[ nCurTab ];
        sal_uInt16 nNextTab = nCurTab + 1;
        SvLBoxTab* pNextTab = nNextTab < nTabCount ? aTabs[nNextTab] : 0;
        SvLBoxItem* pItem = nCurItem < nItemCount ? pEntry->GetItem(nCurItem) : 0;

        sal_uInt16 nFlags = pTab->nFlags;
        Size aSize( pItem->GetSize( pViewDataEntry, nCurItem ));
        long nTabPos = GetTabPos( pEntry, pTab );

        long nNextTabPos;
        if( pNextTab )
            nNextTabPos = GetTabPos( pEntry, pNextTab );
        else
        {
            nNextTabPos = nMaxRight;
            if( nTabPos > nMaxRight )
                nNextTabPos += 50;
        }

        long nX;
        if( pTab->nFlags & SV_LBOXTAB_ADJUST_RIGHT )
            // avoid cutting the right edge off the tab separation
            nX = nTabPos + pTab->CalcOffset(aSize.Width(), (nNextTabPos-SV_TAB_BORDER-1) -nTabPos);
        else
            nX = nTabPos + pTab->CalcOffset(aSize.Width(), nNextTabPos-nTabPos);

        if( nFlags & nTabFlags )
        {
            if( !bHasClipRegion && nX + aSize.Width() >= nMaxRight )
            {
                SetClipRegion( vcl::Region(pImp->GetClipRegionRect()) );
                bHasClipRegion = true;
            }
            aEntryPos.X() = nX;
            aEntryPos.Y() = nLine;

            // set background pattern/color

            Wallpaper aWallpaper = GetBackground();

            int bSelTab = nFlags & SV_LBOXTAB_SHOW_SELECTION;
            sal_uInt16 nItemType = pItem->GetType();

            if (pViewDataEntry->IsHighlighted() && bSelTab && !pViewDataEntry->IsCursored())
            {
                Color aNewWallColor = rSettings.GetHighlightColor();
                if ( !bInUse || nItemType != SV_ITEM_ID_LBOXCONTEXTBMP )
                {
                    // if the face color is bright then the deactive color is also bright
                    // -> so you can't see any deactive selection
                    if ( bHideSelection && !rSettings.GetFaceColor().IsBright() &&
                         aWallpaper.GetColor().IsBright() != rSettings.GetDeactiveColor().IsBright() )
                        aNewWallColor = rSettings.GetDeactiveColor();
                    // set font color to highlight
                    if ( !bCurFontIsSel )
                    {
                        SetTextColor( aHighlightTextColor );
                        Control::SetFont( aHighlightFont );
                        bCurFontIsSel = true;
                    }
                }
                aWallpaper.SetColor( aNewWallColor );
            }
            else  // no selection
            {
                if( bInUse && nItemType == SV_ITEM_ID_LBOXCONTEXTBMP )
                    aWallpaper.SetColor( rSettings.GetFieldColor() );
                else if( bCurFontIsSel )
                {
                    bCurFontIsSel = false;
                    SetTextColor( aBackupTextColor );
                    Control::SetFont( aBackupFont );
                }
            }

            // draw background
            if( !(nTreeFlags & TREEFLAG_USESEL))
            {
                // only draw the area that is used by the item
                aRectSize.Width() = aSize.Width();
                aRect.SetPos( aEntryPos );
                aRect.SetSize( aRectSize );
            }
            else
            {
                // draw from the current to the next tab
                if( nCurTab != 0 )
                    aRect.Left() = nTabPos;
                else
                    // if we're in the 0th tab, always draw from column 0 --
                    // else we get problems with centered tabs
                    aRect.Left() = 0;
                aRect.Top() = nLine;
                aRect.Bottom() = nLine + nTempEntryHeight - 1;
                if( pNextTab )
                {
                    long nRight;
                    nRight = GetTabPos(pEntry,pNextTab)-1;
                    if( nRight > nMaxRight )
                        nRight = nMaxRight;
                    aRect.Right() = nRight;
                }
                else
                    aRect.Right() = nMaxRight;
            }
            // A custom selection that starts at a tab position > 0, do not fill
            // the background of the 0th item, else e.g. we might not be able to
            // realize tab listboxes with lines.
            if( !(nCurTab==0 && (nTreeFlags & TREEFLAG_USESEL) && nFirstSelTab) )
            {
                SetFillColor( aWallpaper.GetColor() );
                // this case may occur for smaller horizontal resizes
                if( aRect.Left() < aRect.Right() )
                    DrawRect( aRect );
            }
            // draw item
            // center vertically
            aEntryPos.Y() += ( nTempEntryHeight - aSize.Height() ) / 2;
            pItem->Paint(aEntryPos, *this, pViewDataEntry, pEntry);

            // division line between tabs
            if (pNextTab && pItem->GetType() == SV_ITEM_ID_LBOXSTRING &&
                // not at the right edge of the window!
                aRect.Right() < nMaxRight)
            {
                aRect.Left() = aRect.Right() - SV_TAB_BORDER;
                DrawRect( aRect );
            }

            SetFillColor( aBackupColor );
        }
        nCurItem++;
        nCurTab++;
    }
    if( pViewDataEntry->IsCursored() && !HasFocus() )
    {
        // cursor emphasis
        SetFillColor();
        Color aOldLineColor = GetLineColor();
        SetLineColor( Color( COL_BLACK ) );
        aRect = GetFocusRect( pEntry, nLine );
        aRect.Top()++;
        aRect.Bottom()--;
        DrawRect( aRect );
        SetLineColor( aOldLineColor );
        SetFillColor( aBackupColor );
    }

    if( bCurFontIsSel )
    {
        SetTextColor( aBackupTextColor );
        Control::SetFont( aBackupFont );
    }

    sal_uInt16 nFirstDynTabPos;
    SvLBoxTab* pFirstDynamicTab = GetFirstDynamicTab( nFirstDynTabPos );
    long nDynTabPos = GetTabPos( pEntry, pFirstDynamicTab );
    nDynTabPos += pImp->nNodeBmpTabDistance;
    nDynTabPos += pImp->nNodeBmpWidth / 2;
    nDynTabPos += 4; // 4 pixels of buffer, so the node bitmap is not too close
                     // to the next tab

    if( (!(pEntry->GetFlags() & SV_ENTRYFLAG_NO_NODEBMP)) &&
        (nWindowStyle & WB_HASBUTTONS) && pFirstDynamicTab &&
        ( pEntry->HasChildren() || pEntry->HasChildrenOnDemand() ) )
    {
        // find first tab and check if the node bitmap extends into it
        sal_uInt16 nNextTab = nFirstDynTabPos;
        SvLBoxTab* pNextTab;
        do
        {
            nNextTab++;
            pNextTab = nNextTab < nTabCount ? aTabs[nNextTab] : 0;
        } while( pNextTab && pNextTab->IsDynamic() );

        if( !pNextTab || (GetTabPos( pEntry, pNextTab ) > nDynTabPos) )
        {
            if((nWindowStyle & WB_HASBUTTONSATROOT) || pModel->GetDepth(pEntry) > 0)
            {
                Point aPos( GetTabPos(pEntry,pFirstDynamicTab), nLine );
                aPos.X() += pImp->nNodeBmpTabDistance;

#if defined USE_JAVA && defined MACOSX
                if( IsNativeControlSupported( CTRL_LISTNODE, PART_ENTIRE_CONTROL ) )
                {
                    ControlState nState = CTRL_STATE_ENABLED;
                    if ( !IsEnabled() )
                        nState &= ~CTRL_STATE_ENABLED;
                    if ( HasFocus() )
                        nState |= CTRL_STATE_FOCUSED;

                    ImplControlValue aControlValue;
                    if ( IsExpanded( pEntry ) )
                        aControlValue.setTristateVal( BUTTONVALUE_ON );
                    else
                        aControlValue.setTristateVal( BUTTONVALUE_OFF );

                    Rectangle aCtrlRegion( Point( 0, 0 ), Size( nTempEntryHeight, nTempEntryHeight ) );
                    Rectangle aBoundingRgn, aContentRgn;

                    GetNativeControlRegion( CTRL_LISTNODE, PART_ENTIRE_CONTROL, aCtrlRegion, nState, aControlValue, "", aBoundingRgn, aContentRgn );
                    if( aBoundingRgn.GetHeight() < nTempEntryHeight )
                        aPos.Y() += ( nTempEntryHeight - aBoundingRgn.GetHeight() ) / 2;

                    // Fix bug 1646 by adjusting for the map mode's origin
                    Rectangle aCtrlDrawRegion( Point( aPos.X() + GetMapMode().GetOrigin().X(), aPos.Y() ), aBoundingRgn.GetSize() );
                    DrawNativeControl( CTRL_LISTNODE, PART_ENTIRE_CONTROL, aCtrlDrawRegion, nState, aControlValue, "" );
                }
                else
                {
#endif	// USE_JAVA && MACOSX
                const Image* pImg = 0;

                if( IsExpanded(pEntry) )
                    pImg = &pImp->GetExpandedNodeBmp( );
                else
                {
                    if( (!pEntry->HasChildren()) && pEntry->HasChildrenOnDemand() &&
                        (!(pEntry->GetFlags() & SV_ENTRYFLAG_HAD_CHILDREN)) &&
                        pImp->GetDontKnowNodeBmp().GetSizePixel().Width() )
                        pImg = &pImp->GetDontKnowNodeBmp( );
                    else
                        pImg = &pImp->GetCollapsedNodeBmp( );
                }
                aPos.Y() += (nTempEntryHeight - pImg->GetSizePixel().Height()) / 2;

                sal_uInt16 nStyle = 0;
                if ( !IsEnabled() )
                    nStyle |= IMAGE_DRAW_DISABLE;

                //native
                bool bNativeOK = false;
                if ( IsNativeControlSupported( CTRL_LISTNODE, PART_ENTIRE_CONTROL) )
                {
                    ImplControlValue    aControlValue;
                    Rectangle           aCtrlRegion( aPos,  pImg->GetSizePixel() );
                    ControlState        nState = 0;

                    if ( IsEnabled() )  nState |= CTRL_STATE_ENABLED;

                    if ( IsExpanded(pEntry) )
                        aControlValue.setTristateVal( BUTTONVALUE_ON );//expanded node
                    else
                    {
                        if( (!pEntry->HasChildren() )                              &&
                              pEntry->HasChildrenOnDemand()                        &&
                             (!(pEntry->GetFlags() & SV_ENTRYFLAG_HAD_CHILDREN)) &&
                            pImp->GetDontKnowNodeBmp().GetSizePixel().Width()
                        )
                            aControlValue.setTristateVal( BUTTONVALUE_DONTKNOW ); //dont know
                        else
                            aControlValue.setTristateVal( BUTTONVALUE_OFF ); //collapsed node
                    }

                    bNativeOK = DrawNativeControl( CTRL_LISTNODE, PART_ENTIRE_CONTROL,
                                            aCtrlRegion, nState, aControlValue, OUString() );
                }

                if( !bNativeOK) {
                    DrawImage( aPos, *pImg ,nStyle);
                }
#if defined USE_JAVA && defined MACOSX
                }
#endif	// USE_JAVA && MACOSX
            }
        }
    }


    if( bHasClipRegion && bResetClipRegion )
        SetClipRegion();
    return 0; // nRowLen;
}

void SvTreeListBox::PreparePaint( SvTreeListEntry* )
{
}

Rectangle SvTreeListBox::GetFocusRect( SvTreeListEntry* pEntry, long nLine )
{
    Size aSize;
    Rectangle aRect;
    aRect.Top() = nLine;
    aSize.Height() = GetEntryHeight();

    long nRealWidth = pImp->GetOutputSize().Width();
    nRealWidth -= GetMapMode().GetOrigin().X();

    sal_uInt16 nCurTab;
    SvLBoxTab* pTab = GetFirstTab( SV_LBOXTAB_SHOW_SELECTION, nCurTab );
    long nTabPos = 0;
    if( pTab )
        nTabPos = GetTabPos( pEntry, pTab );
    long nNextTabPos;
    if( pTab && nCurTab < aTabs.size() - 1 )
    {
        SvLBoxTab* pNextTab = aTabs[ nCurTab + 1 ];
        nNextTabPos = GetTabPos( pEntry, pNextTab );
    }
    else
    {
        nNextTabPos = nRealWidth;
        if( nTabPos > nRealWidth )
            nNextTabPos += 50;
    }

    bool bUserSelection = ( nTreeFlags & TREEFLAG_USESEL ) != 0;
    if( !bUserSelection )
    {
        if( pTab && nCurTab < pEntry->ItemCount() )
        {
            SvLBoxItem* pItem = pEntry->GetItem( nCurTab );
            aSize.Width() = pItem->GetSize( this, pEntry ).Width();
            if( !aSize.Width() )
                aSize.Width() = 15;
            long nX = nTabPos; //GetTabPos( pEntry, pTab );
            // alignment
            nX += pTab->CalcOffset( aSize.Width(), nNextTabPos - nTabPos );
            aRect.Left() = nX;
            // make sure that first and last letter aren't cut off slightly
            aRect.SetSize( aSize );
            if( aRect.Left() > 0 )
                aRect.Left()--;
            aRect.Right()++;
        }
    }
    else
    {
        // if SelTab != 0, we have to calculate also
        if( nFocusWidth == -1 || nFirstSelTab )
        {
            SvLBoxTab* pLastTab = NULL; // default to select whole width

            sal_uInt16 nLastTab;
            GetLastTab(SV_LBOXTAB_SHOW_SELECTION,nLastTab);
            nLastTab++;
            if( nLastTab < aTabs.size() ) // is there another one?
                pLastTab = aTabs[ nLastTab ];

            aSize.Width() = pLastTab ? pLastTab->GetPos() : 0x0fffffff;
            nFocusWidth = (short)aSize.Width();
            if( pTab )
                nFocusWidth = nFocusWidth - (short)nTabPos; //pTab->GetPos();
        }
        else
        {
            aSize.Width() = nFocusWidth;
            if( pTab )
            {
                if( nCurTab )
                    aSize.Width() += nTabPos;
                else
                    aSize.Width() += pTab->GetPos(); // Tab0 always from the leftmost position
            }
        }
        // if selection starts with 0th tab, draw from column 0 on
        if( nCurTab != 0 )
        {
            aRect.Left() = nTabPos;
            aSize.Width() -= nTabPos;
        }
        aRect.SetSize( aSize );
    }
    // adjust right edge because of clipping
    if( aRect.Right() >= nRealWidth )
    {
        aRect.Right() = nRealWidth-1;
        nFocusWidth = (short)aRect.GetWidth();
    }
    return aRect;
}


sal_IntPtr SvTreeListBox::GetTabPos( SvTreeListEntry* pEntry, SvLBoxTab* pTab)
{
    DBG_ASSERT(pTab,"No Tab");
    sal_IntPtr nPos = pTab->GetPos();
    if( pTab->IsDynamic() )
    {
        sal_uInt16 nDepth = pModel->GetDepth( pEntry );
        nDepth = nDepth * (sal_uInt16)nIndent;
        nPos += (sal_IntPtr)nDepth;
    }
    return nPos;
}

SvLBoxItem* SvTreeListBox::GetItem_Impl( SvTreeListEntry* pEntry, long nX,
    SvLBoxTab** ppTab, sal_uInt16 nEmptyWidth )
{
    SvLBoxItem* pItemClicked = 0;
    sal_uInt16 nTabCount = aTabs.size();
    sal_uInt16 nItemCount = pEntry->ItemCount();
    SvLBoxTab* pTab = aTabs.front();
    SvLBoxItem* pItem = pEntry->GetItem(0);
    sal_uInt16 nNextItem = 1;
    nX -= GetMapMode().GetOrigin().X();
    long nRealWidth = pImp->GetOutputSize().Width();
    nRealWidth -= GetMapMode().GetOrigin().X();

    while( true )
    {
        SvLBoxTab* pNextTab=nNextItem<nTabCount ? aTabs[nNextItem] : 0;
        long nStart = GetTabPos( pEntry, pTab );

        long nNextTabPos;
        if( pNextTab )
            nNextTabPos = GetTabPos( pEntry, pNextTab );
        else
        {
            nNextTabPos = nRealWidth;
            if( nStart > nRealWidth )
                nNextTabPos += 50;
        }

        Size aItemSize( pItem->GetSize(this, pEntry));
        nStart += pTab->CalcOffset( aItemSize.Width(), nNextTabPos - nStart );
        long nLen = aItemSize.Width();
        if( pNextTab )
        {
            long nTabWidth = GetTabPos( pEntry, pNextTab ) - nStart;
            if( nTabWidth < nLen )
                nLen = nTabWidth;
        }

        if( !nLen )
            nLen = nEmptyWidth;

        if( nX >= nStart && nX < (nStart+nLen ) )
        {
            pItemClicked = pItem;
            if( ppTab )
            {
                *ppTab = pTab;
                break;
            }
        }
        if( nNextItem >= nItemCount || nNextItem >= nTabCount)
            break;
        pTab = aTabs[ nNextItem ];
        pItem = pEntry->GetItem( nNextItem );
        nNextItem++;
    }
    return pItemClicked;
}

long SvTreeListBox::getPreferredDimensions(std::vector<long> &rWidths) const
{
    long nHeight = 0;
    rWidths.clear();
    SvTreeListEntry* pEntry = First();
    while (pEntry)
    {
        sal_uInt16 nCount = pEntry->ItemCount();
        sal_uInt16 nCurPos = 0;
        if (nCount > rWidths.size())
            rWidths.resize(nCount);
        while (nCurPos < nCount)
        {
            SvLBoxItem* pItem = pEntry->GetItem( nCurPos );
            long nWidth = pItem->GetSize(this, pEntry).Width();
            if (nWidth)
            {
                nWidth += SV_TAB_BORDER * 2;
                if (nWidth > rWidths[nCurPos])
                   rWidths[nCurPos] = nWidth;
            }
            ++nCurPos;
        }
        pEntry = Next( pEntry );
        nHeight += GetEntryHeight();
    }
    return nHeight;
}

Size SvTreeListBox::GetOptimalSize() const
{
    std::vector<long> aWidths;
    Size aRet(0, getPreferredDimensions(aWidths));
    for (size_t i = 0; i < aWidths.size(); ++i)
        aRet.Width() += aWidths[i];
    if (GetStyle() & WB_BORDER)
    {
        const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
        aRet.Width() += rStyleSettings.GetBorderSize() * 2;
        aRet.Height() += rStyleSettings.GetBorderSize() * 2;
    }
    long nMinWidth = nMinWidthInChars * approximate_char_width();
    aRet.Width() = std::max(aRet.Width(), nMinWidth);
    return aRet;
}

void SvTreeListBox::SetAlternatingRowColors( bool bEnable )
{
    mbAlternatingRowColors = bEnable;
    if( mbAlternatingRowColors )
    {
        SvTreeListEntry* pEntry = pModel->First();
        for(size_t i = 0; pEntry; ++i)
        {
            pEntry->SetBackColor( i % 2 == 0 ? GetSettings().GetStyleSettings().GetRowColor() :
                                               GetSettings().GetStyleSettings().GetAlternatingRowColor());
            pEntry = pModel->Next(pEntry);
        }
    }
    else
        for(SvTreeListEntry* pEntry = pModel->First(); pEntry; pEntry = pModel->Next(pEntry))
            pEntry->SetBackColor( GetSettings().GetStyleSettings().GetRowColor() );

    pImp->UpdateAll();
}

SvLBoxItem* SvTreeListBox::GetItem(SvTreeListEntry* pEntry,long nX,SvLBoxTab** ppTab)
{
    return GetItem_Impl( pEntry, nX, ppTab, 0 );
}

SvLBoxItem* SvTreeListBox::GetItem(SvTreeListEntry* pEntry,long nX )
{
    SvLBoxTab* pDummyTab;
    return GetItem_Impl( pEntry, nX, &pDummyTab, 0 );
}

void SvTreeListBox::AddTab(long nTabPos,sal_uInt16 nFlags,void* pUserData )
{
    nFocusWidth = -1;
    SvLBoxTab* pTab = new SvLBoxTab( nTabPos, nFlags );
    pTab->SetUserData( pUserData );
    aTabs.push_back( pTab );
    if( nTreeFlags & TREEFLAG_USESEL )
    {
        sal_uInt16 nPos = aTabs.size() - 1;
        if( nPos >= nFirstSelTab && nPos <= nLastSelTab )
            pTab->nFlags |= SV_LBOXTAB_SHOW_SELECTION;
        else
            // string items usually have to be selected -- turn this off
            // explicitly
            pTab->nFlags &= ~SV_LBOXTAB_SHOW_SELECTION;
    }
}



SvLBoxTab* SvTreeListBox::GetFirstDynamicTab( sal_uInt16& rPos ) const
{
    sal_uInt16 nCurTab = 0;
    sal_uInt16 nTabCount = aTabs.size();
    while( nCurTab < nTabCount )
    {
        SvLBoxTab* pTab = aTabs[nCurTab];
        if( pTab->nFlags & SV_LBOXTAB_DYNAMIC )
        {
            rPos = nCurTab;
            return pTab;
        }
        nCurTab++;
    }
    return 0;
}

SvLBoxTab* SvTreeListBox::GetFirstDynamicTab() const
{
    sal_uInt16 nDummy;
    return GetFirstDynamicTab( nDummy );
}

SvLBoxTab* SvTreeListBox::GetTab( SvTreeListEntry* pEntry, SvLBoxItem* pItem) const
{
    sal_uInt16 nPos = pEntry->GetPos( pItem );
    return aTabs[ nPos ];
}

void SvTreeListBox::ClearTabList()
{
    sal_uInt16 nTabCount = aTabs.size();
    while( nTabCount )
    {
        nTabCount--;
        SvLBoxTab* pDelTab = aTabs[ nTabCount ];
        delete pDelTab;
    }
    aTabs.clear();
}


Size SvTreeListBox::GetOutputSizePixel() const
{
    Size aSize = pImp->GetOutputSize();
    return aSize;
}

void SvTreeListBox::NotifyBeginScroll()
{
}

void SvTreeListBox::NotifyEndScroll()
{
}

void SvTreeListBox::NotifyScrolling( long )
{
}

void SvTreeListBox::NotifyScrolled()
{
    aScrolledHdl.Call( this );
}

void SvTreeListBox::NotifyInvalidating()
{
}

void SvTreeListBox::Invalidate( sal_uInt16 nInvalidateFlags )
{
    if( nFocusWidth == -1 )
        // to make sure that the control doesn't show the wrong focus rectangle
        // after painting
        pImp->RecalcFocusRect();
    NotifyInvalidating();
    Control::Invalidate( nInvalidateFlags );
    pImp->Invalidate();
}

void SvTreeListBox::Invalidate( const Rectangle& rRect, sal_uInt16 nInvalidateFlags )
{
    if( nFocusWidth == -1 )
        // to make sure that the control doesn't show the wrong focus rectangle
        // after painting
        pImp->RecalcFocusRect();
    NotifyInvalidating();
    Control::Invalidate( rRect, nInvalidateFlags );
}


void SvTreeListBox::SetHighlightRange( sal_uInt16 nStart, sal_uInt16 nEnd)
{

    sal_uInt16 nTemp;
    nTreeFlags |= TREEFLAG_USESEL;
    if( nStart > nEnd )
    {
        nTemp = nStart;
        nStart = nEnd;
        nEnd = nTemp;
    }
    // select all tabs that lie within the area
    nTreeFlags |= TREEFLAG_RECALCTABS;
    nFirstSelTab = nStart;
    nLastSelTab = nEnd;
    pImp->RecalcFocusRect();
}

void SvTreeListBox::Command( const CommandEvent& rCEvt )
{
    // FIXME gnumake2 resync to DEV300_m84
    pImp->Command( rCEvt );
}


void SvTreeListBox::RemoveParentKeepChildren( SvTreeListEntry* pParent )
{
    DBG_ASSERT(pParent,"RemoveParentKeepChildren:No Parent");
    SvTreeListEntry* pNewParent = GetParent( pParent );
    if( pParent->HasChildren())
    {
        SvTreeListEntry* pChild = FirstChild( pParent );
        while( pChild )
        {
            pModel->Move( pChild, pNewParent, TREELIST_APPEND );
            pChild = FirstChild( pParent );
        }
    }
    pModel->Remove( pParent );
}

SvLBoxTab* SvTreeListBox::GetFirstTab( sal_uInt16 nFlagMask, sal_uInt16& rPos )
{
    sal_uInt16 nTabCount = aTabs.size();
    for( sal_uInt16 nPos = 0; nPos < nTabCount; nPos++ )
    {
        SvLBoxTab* pTab = aTabs[ nPos ];
        if( (pTab->nFlags & nFlagMask) )
        {
            rPos = nPos;
            return pTab;
        }
    }
    rPos = 0xffff;
    return 0;
}

SvLBoxTab* SvTreeListBox::GetLastTab( sal_uInt16 nFlagMask, sal_uInt16& rTabPos )
{
    sal_uInt16 nPos = (sal_uInt16)aTabs.size();
    while( nPos )
    {
        --nPos;
        SvLBoxTab* pTab = aTabs[ nPos ];
        if( (pTab->nFlags & nFlagMask) )
        {
            rTabPos = nPos;
            return pTab;
        }
    }
    rTabPos = 0xffff;
    return 0;
}

void SvTreeListBox::RequestHelp( const HelpEvent& rHEvt )
{
    if( !pImp->RequestHelp( rHEvt ) )
        Control::RequestHelp( rHEvt );
}

void SvTreeListBox::CursorMoved( SvTreeListEntry* )
{
}

IMPL_LINK( SvTreeListBox, DefaultCompare, SvSortData*, pData )
{
    const SvTreeListEntry* pLeft = pData->pLeft;
    const SvTreeListEntry* pRight = pData->pRight;
    OUString aLeft( static_cast<const SvLBoxString*>(pLeft->GetFirstItem(SV_ITEM_ID_LBOXSTRING))->GetText());
    OUString aRight( static_cast<const SvLBoxString*>(pRight->GetFirstItem(SV_ITEM_ID_LBOXSTRING))->GetText());
    pImp->UpdateStringSorter();
    return pImp->m_pStringSorter->compare(aLeft, aRight);
}

void SvTreeListBox::ModelNotification( SvListAction nActionId, SvTreeListEntry* pEntry1,
                        SvTreeListEntry* pEntry2, sal_uLong nPos )
{
    SolarMutexGuard aSolarGuard;

    if( nActionId == SvListAction::CLEARING )
        CancelTextEditing();

    SvListView::ModelNotification( nActionId, pEntry1, pEntry2, nPos );
    switch( nActionId )
    {
        case SvListAction::INSERTED:
        {
            SvTreeListEntry* pEntry( dynamic_cast< SvTreeListEntry* >( pEntry1 ) );
            if ( !pEntry )
            {
                SAL_WARN( "svtools.contnr", "SvTreeListBox::ModelNotification: invalid entry!" );
                break;
            }

            SvLBoxContextBmp* pBmpItem = static_cast< SvLBoxContextBmp* >( pEntry->GetFirstItem( SV_ITEM_ID_LBOXCONTEXTBMP ) );
            if ( !pBmpItem )
                break;
            const Image& rBitmap1( pBmpItem->GetBitmap1() );
            const Image& rBitmap2( pBmpItem->GetBitmap2() );
            short nMaxWidth = short( std::max( rBitmap1.GetSizePixel().Width(), rBitmap2.GetSizePixel().Width() ) );
            nMaxWidth = pImp->UpdateContextBmpWidthVector( pEntry, nMaxWidth );
            if( nMaxWidth > nContextBmpWidthMax )
            {
                nContextBmpWidthMax = nMaxWidth;
                SetTabs();
            }
            if (get_width_request() == -1)
                queue_resize();
        }
        break;

        case SvListAction::RESORTING:
            SetUpdateMode( false );
            break;

        case SvListAction::RESORTED:
            // after a selection: show first entry and also keep the selection
            MakeVisible( (SvTreeListEntry*)pModel->First(), true );
            SetUpdateMode( true );
            break;

        case SvListAction::CLEARED:
            if( IsUpdateMode() )
                Update();
            break;

        default: break;
    }
}

void SvTreeListBox::EndSelection()
{
    pImp->EndSelection();
}

void SvTreeListBox::RepaintScrollBars() const
{
    ((SvTreeListBox*)this)->pImp->RepaintScrollBars();
}

ScrollBar *SvTreeListBox::GetVScroll()
{
    return &((SvTreeListBox*)this)->pImp->aVerSBar;
}

ScrollBar *SvTreeListBox::GetHScroll()
{
    return &((SvTreeListBox*)this)->pImp->aHorSBar;
}

void SvTreeListBox::EnableAsyncDrag( bool b )
{
    pImp->EnableAsyncDrag( b );
}

SvTreeListEntry* SvTreeListBox::GetFirstEntryInView() const
{
    Point aPos;
    return GetEntry( aPos );
}

SvTreeListEntry* SvTreeListBox::GetNextEntryInView(SvTreeListEntry* pEntry ) const
{
    SvTreeListEntry* pNext = (SvTreeListEntry*)NextVisible( pEntry );
    if( pNext )
    {
        Point aPos( GetEntryPosition(pNext) );
        const Size& rSize = pImp->GetOutputSize();
        if( aPos.Y() < 0 || aPos.Y() >= rSize.Height() )
            return 0;
    }
    return pNext;
}

SvTreeListEntry* SvTreeListBox::GetLastEntryInView() const
{
    SvTreeListEntry* pEntry = GetFirstEntryInView();
    SvTreeListEntry* pNext = 0;
    while( pEntry )
    {
        pNext = (SvTreeListEntry*)NextVisible( pEntry );
        if( pNext )
        {
          Point aPos( GetEntryPosition(pNext) );
          const Size& rSize = pImp->GetOutputSize();
          if( aPos.Y() < 0 || aPos.Y() + GetEntryHeight() >= rSize.Height() )
              break;
          else
              pEntry = pNext;
        }
        else
            break;
    }
    return pEntry;
}

void SvTreeListBox::ShowFocusRect( const SvTreeListEntry* pEntry )
{
    pImp->ShowFocusRect( pEntry );
}

void SvTreeListBox::DataChanged( const DataChangedEvent& rDCEvt )
{
    if( (rDCEvt.GetType()==DATACHANGED_SETTINGS) && (rDCEvt.GetFlags() & SETTINGS_STYLE) )
    {
        nEntryHeight = 0;   // _together_ with true of 1. par (bFont) of InitSettings() a zero-height
                            //  forces complete recalc of heights!
        InitSettings( true, true, true );
        Invalidate();
    }
    else
        Control::DataChanged( rDCEvt );
}

void SvTreeListBox::StateChanged( StateChangedType eType )
{
    if( eType == StateChangedType::ENABLE )
        Invalidate( INVALIDATE_CHILDREN );

    Control::StateChanged( eType );

    if ( eType == StateChangedType::STYLE )
        ImplInitStyle();
}

void SvTreeListBox::InitSettings(bool bFont, bool bForeground, bool bBackground)
{
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
    if( bFont )
    {
        vcl::Font aFont;
        aFont = rStyleSettings.GetFieldFont();
        aFont.SetColor( rStyleSettings.GetWindowTextColor() );
        SetPointFont( aFont );
        AdjustEntryHeightAndRecalc( aFont );
    }

    if( bForeground || bFont )
    {
        SetTextColor( rStyleSettings.GetFieldTextColor() );
        SetTextFillColor();
    }

    if( bBackground )
        SetBackground( rStyleSettings.GetFieldColor() );

    // always try to re-create default-SvLBoxButtonData
    if( pCheckButtonData && pCheckButtonData->HasDefaultImages() )
        pCheckButtonData->SetDefaultImages( this );
}

bool SvTreeListBox::IsCellFocusEnabled() const
{
    return pImp->IsCellFocusEnabled();
}

bool SvTreeListBox::SetCurrentTabPos( sal_uInt16 _nNewPos )
{
    return pImp->SetCurrentTabPos( _nNewPos );
}

sal_uInt16 SvTreeListBox::GetCurrentTabPos() const
{
    return pImp->GetCurrentTabPos();
}

void SvTreeListBox::InitStartEntry()
{
    if( !pImp->pStartEntry )
        pImp->pStartEntry = GetModel()->First();
}

PopupMenu* SvTreeListBox::CreateContextMenu( void )
{
    return NULL;
}

void SvTreeListBox::ExcecuteContextMenuAction( sal_uInt16 )
{
    DBG_WARNING( "SvTreeListBox::ExcecuteContextMenuAction(): now there's happening nothing!" );
}

void SvTreeListBox::EnableContextMenuHandling( void )
{
    assert(pImp && "-SvTreeListBox::EnableContextMenuHandling(): No implementation!");
    pImp->bContextMenuHandling = true;
}

void SvTreeListBox::EnableList( bool _bEnable )
{
    // call base class method
    Window::Enable(_bEnable);
    // then paint immediately
    Paint( Rectangle( Point(), GetSizePixel() ) );
}

::com::sun::star::uno::Reference< XAccessible > SvTreeListBox::CreateAccessible()
{
    vcl::Window* pParent = GetAccessibleParentWindow();
    DBG_ASSERT( pParent, "SvTreeListBox::CreateAccessible - accessible parent not found" );

    ::com::sun::star::uno::Reference< XAccessible > xAccessible;
    if ( pParent )
    {
        ::com::sun::star::uno::Reference< XAccessible > xAccParent = pParent->GetAccessible();
        if ( xAccParent.is() )
        {
            // need to be done here to get the vclxwindow later on in the accessbile
            ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer > xTemp(GetComponentInterface());
            xAccessible = pImp->m_aFactoryAccess.getFactory().createAccessibleTreeListBox( *this, xAccParent );
        }
    }
    return xAccessible;
}

void SvTreeListBox::FillAccessibleEntryStateSet( SvTreeListEntry* pEntry, ::utl::AccessibleStateSetHelper& rStateSet ) const
{
    assert(pEntry && "SvTreeListBox::FillAccessibleEntryStateSet: invalid entry");

    if ( pEntry->HasChildrenOnDemand() || pEntry->HasChildren() )
    {
        rStateSet.AddState( AccessibleStateType::EXPANDABLE );
        if ( IsExpanded( pEntry ) )
            rStateSet.AddState( (sal_Int16)AccessibleStateType::EXPANDED );
    }

    if ( GetCheckButtonState( pEntry ) == SV_BUTTON_CHECKED )
        rStateSet.AddState( AccessibleStateType::CHECKED );
    if ( IsEntryVisible( pEntry ) )
        rStateSet.AddState( AccessibleStateType::VISIBLE );
    if ( IsSelected( pEntry ) )
        rStateSet.AddState( AccessibleStateType::SELECTED );
    if ( IsEnabled() )
    {
        rStateSet.AddState( AccessibleStateType::ENABLED );
        rStateSet.AddState( AccessibleStateType::FOCUSABLE );
        rStateSet.AddState( AccessibleStateType::SELECTABLE );
        SvViewDataEntry* pViewDataNewCur = GetViewDataEntry(pEntry);
        if (pViewDataNewCur && pViewDataNewCur->HasFocus())
            rStateSet.AddState( AccessibleStateType::FOCUSED );
    }
}

Rectangle SvTreeListBox::GetBoundingRect( SvTreeListEntry* pEntry )
{
    Point aPos = GetEntryPosition( pEntry );
    Rectangle aRect = GetFocusRect( pEntry, aPos.Y() );
    return aRect;
}

void SvTreeListBox::EnableCellFocus()
{
    pImp->EnableCellFocus();
}

void SvTreeListBox::CallImplEventListeners(sal_uLong nEvent, void* pData)
{
    CallEventListeners(nEvent, pData);
}

void SvTreeListBox::FillAccessibleStateSet( ::utl::AccessibleStateSetHelper& /*rStateSet*/ ) const
{
}

void SvTreeListBox::set_min_width_in_chars(sal_Int32 nChars)
{
    nMinWidthInChars = nChars;
    queue_resize();
}

bool SvTreeListBox::set_property(const OString &rKey, const OString &rValue)
{
    if (rKey == "min-width-chars")
    {
        set_min_width_in_chars(rValue.toInt32());
    }
    else
        return Control::set_property(rKey, rValue);
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
