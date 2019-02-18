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

#include "AppIconControl.hxx"
#include "dbaccess_helpid.hrc"
#include "moduledbu.hxx"
#include "dbu_app.hrc"
#include "bitmaps.hlst"
#include <vcl/image.hxx>
#include "callbacks.hxx"
#include "AppElementType.hxx"
#include <memory>

#if defined USE_JAVA && defined MACOSX

#include <dlfcn.h>

typedef sal_Bool Application_canUseJava_Type();

static Application_canUseJava_Type *pApplication_canUseJava = nullptr;

#endif	// USE_JAVA && MACOSX

using namespace ::dbaui;
// class OApplicationIconControl
OApplicationIconControl::OApplicationIconControl(vcl::Window* _pParent)
    : SvtIconChoiceCtrl(_pParent,WB_ICON | WB_NOCOLUMNHEADER | WB_HIGHLIGHTFRAME | /*!WB_NOSELECTION |*/
                                WB_TABSTOP | WB_CLIPCHILDREN | WB_NOVSCROLL | WB_SMART_ARRANGE | WB_NOHSCROLL | WB_CENTER)
    ,DropTargetHelper(this)
    ,m_pActionListener(nullptr)
{

    const struct CategoryDescriptor
    {
        sal_uInt16 nLabelResId;
        ElementType eType;
        const char* aImageResId;
    }   aCategories[] = {
        { RID_STR_TABLES_CONTAINER,     E_TABLE,    BMP_TABLEFOLDER_TREE_L  },
        { RID_STR_QUERIES_CONTAINER,    E_QUERY,    BMP_QUERYFOLDER_TREE_L  },
        { RID_STR_FORMS_CONTAINER,      E_FORM,     BMP_FORMFOLDER_TREE_L   },
        { RID_STR_REPORTS_CONTAINER,    E_REPORT,   BMP_REPORTFOLDER_TREE_L }
    };
    for (const CategoryDescriptor& aCategorie : aCategories)
    {
#if defined USE_JAVA && defined MACOSX
        if ( !pApplication_canUseJava )
            pApplication_canUseJava = reinterpret_cast< Application_canUseJava_Type* >( dlsym( RTLD_MAIN_ONLY, "Application_canUseJava" ) );
        if ( !pApplication_canUseJava || !pApplication_canUseJava() )
        {
            // All report options require Java
            if ( aCategorie.eType == E_REPORT )
                continue;
        }
#endif	// USE_JAVA && defined MACOSX
        SvxIconChoiceCtrlEntry* pEntry = InsertEntry(
            OUString( ModuleRes( aCategorie.nLabelResId ) ) ,
            Image(BitmapEx(OUString::createFromAscii(aCategorie.aImageResId))));
        if ( pEntry )
            pEntry->SetUserData( new ElementType( aCategorie.eType ) );
    }

    SetChoiceWithCursor();
    SetSelectionMode(SelectionMode::Single);
}

OApplicationIconControl::~OApplicationIconControl()
{
    disposeOnce();
}

void OApplicationIconControl::dispose()
{
    sal_uLong nCount = GetEntryCount();
    for ( sal_uLong i = 0; i < nCount; ++i )
    {
        SvxIconChoiceCtrlEntry* pEntry = GetEntry( i );
        if ( pEntry )
        {
            std::unique_ptr<ElementType> aType(static_cast<ElementType*>(pEntry->GetUserData()));
            pEntry->SetUserData(nullptr);
        }
    }
    DropTargetHelper::dispose();
    SvtIconChoiceCtrl::dispose();
}

sal_Int8 OApplicationIconControl::AcceptDrop( const AcceptDropEvent& _rEvt )
{
    sal_Int8 nDropOption = DND_ACTION_NONE;
    if ( m_pActionListener )
    {

        SvxIconChoiceCtrlEntry* pEntry = GetEntry(_rEvt.maPosPixel);
        if ( pEntry )
        {
            SetCursor(pEntry);
            nDropOption = m_pActionListener->queryDrop( _rEvt, GetDataFlavorExVector() );
            m_aMousePos = _rEvt.maPosPixel;
        }
    }

    return nDropOption;
}

sal_Int8 OApplicationIconControl::ExecuteDrop( const ExecuteDropEvent& _rEvt )
{
    if ( m_pActionListener )
        return m_pActionListener->executeDrop( _rEvt );

    return DND_ACTION_NONE;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
