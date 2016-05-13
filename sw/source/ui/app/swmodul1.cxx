/**************************************************************
 * 
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 * 
 * This file incorporates work covered by the following license notice:
 * 
 *   Modified May 2016 by Patrick Luby. NeoOffice is only distributed
 *   under the GNU General Public License, Version 3 as allowed by Section 4
 *   of the Apache License, Version 2.0.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * 
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"


#include <hintids.hxx>
#include <sfx2/request.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/childwin.hxx>
#include <unotools/useroptions.hxx>
#include <cppuhelper/weak.hxx>
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <cppuhelper/implbase1.hxx>	// helper for implementations
#include <svx/dataaccessdescriptor.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/crsditem.hxx>
#include <editeng/cmapitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/brshitem.hxx>
#include <vcl/msgbox.hxx>
#include <swmodule.hxx>
#include <swtypes.hxx>
#include <usrpref.hxx>
#include <modcfg.hxx>
#include <view.hxx>
#include <pview.hxx>
#include <wview.hxx>
#include <wrtsh.hxx>
#include <docsh.hxx>
#include <dbmgr.hxx>
#include <uinums.hxx>
#include <prtopt.hxx>		// fuer PrintOptions
#include <navicfg.hxx>
#include <doc.hxx>
#include <cmdid.h>
#include <app.hrc>
#include "helpid.h"

#include <unomid.h>
#include <tools/color.hxx>
#include "PostItMgr.hxx"

#ifdef USE_JAVA

class SwPrintOptionsReset : public utl::ConfigItem
{
    sal_Bool				mbPrintOptionsReset;
    sal_Bool				mbWebPrintOptionsReset;

	com::sun::star::uno::Sequence< rtl::OUString >	GetPropertyNames();

public:
							SwPrintOptionsReset();
	virtual					~SwPrintOptionsReset() {};

	virtual void			Commit();
	virtual void			Notify( const ::com::sun::star::uno::Sequence< rtl::OUString >& aPropertyNames ) {}
	sal_Bool				IsPrintOptionsReset() const { return mbPrintOptionsReset; }
	sal_Bool				IsWebPrintOptionsReset() const { return mbWebPrintOptionsReset; }
    void					SetPrintOptionsReset(sal_Bool b) { mbPrintOptionsReset = b; }
    void					SetWebPrintOptionsReset(sal_Bool b) { mbWebPrintOptionsReset = b; }
};

#endif	// USE_JAVA

using ::rtl::OUString;
using namespace ::svx;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::view;
using namespace ::com::sun::star::lang;


/*-----------------08/28/97 08:41pm-----------------

--------------------------------------------------*/
void lcl_SetUIPrefs(const SwViewOption* pPref, SwView* pView, ViewShell* pSh )
{
	// in FrameSets kann die tatsaechliche Sichtbarkeit von der Einstellung der ViewOptions abweichen
	sal_Bool bVScrollChanged = pPref->IsViewVScrollBar() != pSh->GetViewOptions()->IsViewVScrollBar();
    sal_Bool bHScrollChanged = pPref->IsViewHScrollBar() != pSh->GetViewOptions()->IsViewHScrollBar();
    sal_Bool bVAlignChanged = pPref->IsVRulerRight() != pSh->GetViewOptions()->IsVRulerRight();

    pSh->SetUIOptions(*pPref);
    const SwViewOption* pNewPref = pSh->GetViewOptions();

	// Scrollbars an / aus
	if(bVScrollChanged)
	{
        pView->ShowVScrollbar(pNewPref->IsViewVScrollBar());
	}
	if(bHScrollChanged)
	{
        pView->ShowHScrollbar( pNewPref->IsViewHScrollBar() || pNewPref->getBrowseMode() );
	}
    //if only the position of the vertical ruler has been changed initiate an update
    if(bVAlignChanged && !bHScrollChanged && !bVScrollChanged)
        pView->InvalidateBorder();

	// Lineale an / aus
    if(pNewPref->IsViewVRuler())
		pView->CreateVLineal();
	else
		pView->KillVLineal();

	// TabWindow an/aus
    if(pNewPref->IsViewHRuler())
		pView->CreateTab();
	else
		pView->KillTab();

	pView->GetPostItMgr()->PrepareView(true);
}

/*--------------------------------------------------------------------
	Beschreibung:	Aktuelle SwWrtShell
 --------------------------------------------------------------------*/


SwWrtShell*	GetActiveWrtShell()
{
	SwView *pActive = ::GetActiveView();
	if( pActive )
		return &pActive->GetWrtShell();
	return 0;
}

/*--------------------------------------------------------------------
	Beschreibung: 	Pointer auf die aktuelle Sicht
 --------------------------------------------------------------------*/


SwView* GetActiveView()
{
	SfxViewShell* pView = SfxViewShell::Current();
	return PTR_CAST( SwView, pView );
}
/*--------------------------------------------------------------------
	Beschreibung:	Ueber Views iterieren - static
 --------------------------------------------------------------------*/

SwView* SwModule::GetFirstView()
{
	// liefert nur sichtbare SwViews
	const TypeId aTypeId = TYPE(SwView);
	SwView* pView = (SwView*)SfxViewShell::GetFirst(&aTypeId);
	return pView;
}


SwView* SwModule::GetNextView(SwView* pView)
{
	DBG_ASSERT(PTR_CAST(SwView, pView),"keine SwView uebergeben");
	const TypeId aTypeId = TYPE(SwView);
    SwView* pNView = (SwView*)SfxViewShell::GetNext(*pView, &aTypeId, sal_True);
	return pNView;
}

/*------------------------------------------------------------------------
 Beschreibung:	Neuer Master fuer die Einstellungen wird gesetzt;
				dieser wirkt sich auf die aktuelle Sicht und alle
				folgenden aus.
------------------------------------------------------------------------*/

void SwModule::ApplyUsrPref(const SwViewOption &rUsrPref, SwView* pActView,
							sal_uInt16 nDest )
{
    SwView* pCurrView = pActView;
    ViewShell* pSh = pCurrView ? &pCurrView->GetWrtShell() : 0;

    SwMasterUsrPref* pPref = (SwMasterUsrPref*)GetUsrPref( static_cast< sal_Bool >(
										 VIEWOPT_DEST_WEB == nDest ? sal_True  :
										 VIEWOPT_DEST_TEXT== nDest ? sal_False :
                                         pCurrView && pCurrView->ISA(SwWebView) ));

	//per Uno soll nur die sdbcx::View, aber nicht das Module veraendert werden
	sal_Bool bViewOnly = VIEWOPT_DEST_VIEW_ONLY == nDest;
	//PreView abfruehstuecken
	SwPagePreView* pPPView;
    if( !pCurrView && 0 != (pPPView = PTR_CAST( SwPagePreView, SfxViewShell::Current())) )
	{
		if(!bViewOnly)
			pPref->SetUIOptions( rUsrPref );
        pPPView->ShowVScrollbar(pPref->IsViewVScrollBar());
        pPPView->ShowHScrollbar(pPref->IsViewHScrollBar());
		if(!bViewOnly)
		{
			pPref->SetPagePrevRow(rUsrPref.GetPagePrevRow());
			pPref->SetPagePrevCol(rUsrPref.GetPagePrevCol());
		}
		return;
	}

	if(!bViewOnly)
	{
		pPref->SetUsrPref( rUsrPref );
		pPref->SetModified();
	}

    if( !pCurrView )
		return;

	// Weitergabe an die CORE
    sal_Bool bReadonly;
    const SwDocShell* pDocSh = pCurrView->GetDocShell();
    if (pDocSh)
        bReadonly = pDocSh->IsReadOnly();
    else //Use existing option if DocShell missing
        bReadonly = pSh->GetViewOptions()->IsReadonly();
	SwViewOption* pViewOpt;
	if(!bViewOnly)
		pViewOpt = new SwViewOption( *pPref );
	else
		pViewOpt = new SwViewOption( rUsrPref );
	pViewOpt->SetReadonly( bReadonly );
	if( !(*pSh->GetViewOptions() == *pViewOpt) )
	{
		//Ist evtl. nur eine ViewShell
		pSh->StartAction();
		pSh->ApplyViewOptions( *pViewOpt );
		((SwWrtShell*)pSh)->SetReadOnlyAvailable(pViewOpt->IsCursorInProtectedArea());
		pSh->EndAction();
	}
	if ( pSh->GetViewOptions()->IsReadonly() != bReadonly )
		pSh->SetReadonlyOption(bReadonly);

    lcl_SetUIPrefs(pViewOpt, pCurrView, pSh);

    // zum Schluss wird das Idle-Flag wieder gesetzt
	// #42510#
	pPref->SetIdle(sal_True);

    delete pViewOpt;
}
/* -----------------------------28.09.00 12:36--------------------------------

 ---------------------------------------------------------------------------*/
void SwModule::ApplyUserMetric( FieldUnit eMetric, sal_Bool bWeb )
{
		SwMasterUsrPref* pPref;
		if(bWeb)
		{
			if(!pWebUsrPref)
				GetUsrPref(sal_True);
			pPref = pWebUsrPref;
		}
		else
		{
			if(!pUsrPref)
				GetUsrPref(sal_False);
			pPref = pUsrPref;
		}
		FieldUnit eOldMetric = pPref->GetMetric();
		if(eOldMetric != eMetric)
			pPref->SetMetric(eMetric);

        FieldUnit eHScrollMetric = pPref->IsHScrollMetric() ? pPref->GetHScrollMetric() : eMetric;
        FieldUnit eVScrollMetric = pPref->IsVScrollMetric() ? pPref->GetVScrollMetric() : eMetric;

		SwView* pTmpView = SwModule::GetFirstView();
		// fuer alle MDI-Fenster das Lineal umschalten
		while(pTmpView)
		{
			if(bWeb == (0 != PTR_CAST(SwWebView, pTmpView)))
			{
                pTmpView->ChangeVLinealMetric(eVScrollMetric);
                pTmpView->ChangeTabMetric(eHScrollMetric);
			}

			pTmpView = SwModule::GetNextView(pTmpView);
		}
}
/*-- 12.11.2008 14:47:58---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwModule::ApplyRulerMetric( FieldUnit eMetric, sal_Bool bHorizontal, sal_Bool bWeb )
{
    SwMasterUsrPref* pPref;
    if(bWeb)
    {
        if(!pWebUsrPref)
            GetUsrPref(sal_True);
        pPref = pWebUsrPref;
    }
    else
    {
        if(!pUsrPref)
            GetUsrPref(sal_False);
        pPref = pUsrPref;
    }
    if( bHorizontal )
        pPref->SetHScrollMetric(eMetric);
    else
        pPref->SetVScrollMetric(eMetric);

    SwView* pTmpView = SwModule::GetFirstView();
    // switch metric at the appropriate rulers
    while(pTmpView)
    {
        if(bWeb == (0 != dynamic_cast<SwWebView *>( pTmpView )))
        {
            if( bHorizontal )
                pTmpView->ChangeTabMetric(eMetric);
            else
                pTmpView->ChangeVLinealMetric(eMetric);
        }
        pTmpView = SwModule::GetNextView(pTmpView);
    }
}
/*-----------------13.11.96 11.57-------------------

--------------------------------------------------*/

SwNavigationConfig*  SwModule::GetNavigationConfig()
{
	if(!pNavigationConfig)
	{
		pNavigationConfig = new SwNavigationConfig;
	}
	return pNavigationConfig;
}

/*-----------------05.02.97 08.03-------------------

--------------------------------------------------*/

SwPrintOptions* 	SwModule::GetPrtOptions(sal_Bool bWeb)
{
#ifdef USE_JAVA
	// Detect if the print options have been corrupted by the bug reported in
	// the following NeoOffice forum topic and, if so, reset to the default
	// values:
	// http://trinity.neooffice.org/modules.php?name=Forums&file=viewtopic&t=8598
	static SwPrintOptionsReset *pReset = NULL;
	if (!pReset )
		pReset = new SwPrintOptionsReset();
#endif	// USE_JAVA

	if(bWeb && !pWebPrtOpt)
	{
		pWebPrtOpt = new SwPrintOptions(sal_True);

#ifdef USE_JAVA
		if (pWebPrtOpt && pReset && !pReset->IsWebPrintOptionsReset())
		{
			pWebPrtOpt->SetPrintGraphic(sal_True);
			pWebPrtOpt->SetPrintTable(sal_True);
			pWebPrtOpt->SetPrintControl(sal_True);
			pWebPrtOpt->SetPrintPageBackground(sal_True);
			pWebPrtOpt->SetPrintEmptyPages(sal_True);
			pWebPrtOpt->Commit();

			pReset->SetWebPrintOptionsReset(sal_True);
			pReset->Commit();
		}
#endif	// USE_JAVA
	}
	else if(!bWeb && !pPrtOpt)
	{
		pPrtOpt = new SwPrintOptions(sal_False);

#ifdef USE_JAVA
		if (pPrtOpt && pReset && !pReset->IsPrintOptionsReset())
		{
			pPrtOpt->SetPrintGraphic(sal_True);
			pPrtOpt->SetPrintTable(sal_True);
			pPrtOpt->SetPrintControl(sal_True);
			pPrtOpt->SetPrintPageBackground(sal_True);
			pPrtOpt->SetPrintDraw(sal_True);
			pPrtOpt->SetPrintLeftPage(sal_True);
			pPrtOpt->SetPrintRightPage(sal_True);
			pPrtOpt->SetPrintEmptyPages(sal_True);
			pPrtOpt->Commit();

			pReset->SetPrintOptionsReset(sal_True);
			pReset->Commit();
		}
#endif	// USE_JAVA
	}

	return bWeb ? pWebPrtOpt : pPrtOpt;
}

/*-----------------26.06.97 07.52-------------------

--------------------------------------------------*/
SwChapterNumRules*	SwModule::GetChapterNumRules()
{
	if(!pChapterNumRules)
		pChapterNumRules = new SwChapterNumRules;
	return pChapterNumRules;
}

/*--------------------------------------------------------------------
	Beschreibung:
 --------------------------------------------------------------------*/

void SwModule::ShowDBObj(SwView& rView, const SwDBData& rData, sal_Bool /*bOnlyIfAvailable*/)
{
    Reference<XFrame> xFrame = rView.GetViewFrame()->GetFrame().GetFrameInterface();
    Reference<XDispatchProvider> xDP(xFrame, uno::UNO_QUERY);

    uno::Reference<frame::XFrame> xBeamerFrame = xFrame->findFrame(
                                        rtl::OUString::createFromAscii("_beamer"),
                                        FrameSearchFlag::CHILDREN);
    if (xBeamerFrame.is())
    {   // the beamer has been opened by the SfxViewFrame
        Reference<XController> xController = xBeamerFrame->getController();
        Reference<XSelectionSupplier> xControllerSelection(xController, UNO_QUERY);
        if (xControllerSelection.is())
        {

            ODataAccessDescriptor aSelection;
            aSelection.setDataSource(rData.sDataSource);
            aSelection[daCommand]       <<= rData.sCommand;
            aSelection[daCommandType]   <<= rData.nCommandType;
            xControllerSelection->select(makeAny(aSelection.createPropertyValueSequence()));
        }
        else {
            DBG_ERROR("no selection supplier in the beamer!");
        }
    }
}
/*--------------------------------------------------------------------
	Beschreibung: Redlining
 --------------------------------------------------------------------*/

sal_uInt16 SwModule::GetRedlineAuthor()
{
	if (!bAuthorInitialised)
	{
        const SvtUserOptions& rOpt = GetUserOptions();
        if( !(sActAuthor = rOpt.GetFullName()).Len() )
			if( !(sActAuthor = rOpt.GetID()).Len() )
				sActAuthor = String( SW_RES( STR_REDLINE_UNKNOWN_AUTHOR ));
		bAuthorInitialised = sal_True;
	}
	return InsertRedlineAuthor( sActAuthor );
}

/*--------------------------------------------------------------------
	Beschreibung:
 --------------------------------------------------------------------*/

const String& SwModule::GetRedlineAuthor(sal_uInt16 nPos)
{
	DBG_ASSERT(nPos<pAuthorNames->Count(), "author not found!"); //#i45342# RTF doc with no author table caused reader to crash
	while (!(nPos<pAuthorNames->Count()))
	{
		InsertRedlineAuthor(String(RTL_CONSTASCII_USTRINGPARAM("nn")));
	};
	return *pAuthorNames->GetObject(nPos);
}

/*--------------------------------------------------------------------
	Beschreibung:
 --------------------------------------------------------------------*/

sal_uInt16 SwModule::InsertRedlineAuthor(const String& rAuthor)
{
	sal_uInt16 nPos = 0;

	while (nPos < pAuthorNames->Count() && *pAuthorNames->GetObject(nPos) != rAuthor)
		nPos++;

	if (nPos == pAuthorNames->Count())
		pAuthorNames->Insert(new String(rAuthor), nPos);

	return nPos;
}

/*--------------------------------------------------------------------
	Beschreibung:
 --------------------------------------------------------------------*/

void lcl_FillAuthorAttr( sal_uInt16 nAuthor, SfxItemSet &rSet,
						const AuthorCharAttr &rAttr )
{
	Color aCol( rAttr.nColor );

	if( COL_TRANSPARENT == rAttr.nColor )
	{
		static const ColorData aColArr[] = {
		 COL_AUTHOR1_DARK,		COL_AUTHOR2_DARK,	COL_AUTHOR3_DARK,
		 COL_AUTHOR4_DARK,		COL_AUTHOR5_DARK,	COL_AUTHOR6_DARK,
		 COL_AUTHOR7_DARK,		COL_AUTHOR8_DARK,	COL_AUTHOR9_DARK };

		aCol.SetColor( aColArr[ nAuthor % (sizeof( aColArr ) /
										   sizeof( aColArr[0] )) ] );
	}

	sal_Bool bBackGr = COL_NONE == rAttr.nColor;

	switch (rAttr.nItemId)
	{
	case SID_ATTR_CHAR_WEIGHT:
		{
			SvxWeightItem aW( (FontWeight)rAttr.nAttr, RES_CHRATR_WEIGHT );
			rSet.Put( aW );
			aW.SetWhich( RES_CHRATR_CJK_WEIGHT );
			rSet.Put( aW );
			aW.SetWhich( RES_CHRATR_CTL_WEIGHT );
			rSet.Put( aW );
		}
		break;

	case SID_ATTR_CHAR_POSTURE:
		{
			SvxPostureItem aP( (FontItalic)rAttr.nAttr, RES_CHRATR_POSTURE );
			rSet.Put( aP );
			aP.SetWhich( RES_CHRATR_CJK_POSTURE );
			rSet.Put( aP );
			aP.SetWhich( RES_CHRATR_CTL_POSTURE );
			rSet.Put( aP );
		}
		break;

	case SID_ATTR_CHAR_UNDERLINE:
		rSet.Put( SvxUnderlineItem( (FontUnderline)rAttr.nAttr,
									RES_CHRATR_UNDERLINE));
		break;

	case SID_ATTR_CHAR_STRIKEOUT:
		rSet.Put(SvxCrossedOutItem( (FontStrikeout)rAttr.nAttr,
									RES_CHRATR_CROSSEDOUT));
		break;

	case SID_ATTR_CHAR_CASEMAP:
		rSet.Put( SvxCaseMapItem( (SvxCaseMap)rAttr.nAttr,
									RES_CHRATR_CASEMAP));
		break;

	case SID_ATTR_BRUSH:
		rSet.Put( SvxBrushItem( aCol, RES_CHRATR_BACKGROUND ));
		bBackGr = sal_True;
		break;
	}

	if( !bBackGr )
        rSet.Put( SvxColorItem( aCol, RES_CHRATR_COLOR ) );
}

/*--------------------------------------------------------------------
	Beschreibung:
 --------------------------------------------------------------------*/

void SwModule::GetInsertAuthorAttr(sal_uInt16 nAuthor, SfxItemSet &rSet)
{
	lcl_FillAuthorAttr(nAuthor, rSet, pModuleConfig->GetInsertAuthorAttr());
}

/*--------------------------------------------------------------------
	Beschreibung:
 --------------------------------------------------------------------*/

void SwModule::GetDeletedAuthorAttr(sal_uInt16 nAuthor, SfxItemSet &rSet)
{
	lcl_FillAuthorAttr(nAuthor, rSet, pModuleConfig->GetDeletedAuthorAttr());
}

/*--------------------------------------------------------------------
	Beschreibung: Fuer zukuenftige Erweiterung:
 --------------------------------------------------------------------*/

void SwModule::GetFormatAuthorAttr( sal_uInt16 nAuthor, SfxItemSet &rSet )
{
	lcl_FillAuthorAttr( nAuthor, rSet, pModuleConfig->GetFormatAuthorAttr() );
}

/*--------------------------------------------------------------------
	Beschreibung:
 --------------------------------------------------------------------*/

sal_uInt16 SwModule::GetRedlineMarkPos()
{
	return pModuleConfig->GetMarkAlignMode();
}

/*--------------------------------------------------------------------
	Beschreibung:
 --------------------------------------------------------------------*/

sal_Bool SwModule::IsInsTblFormatNum(sal_Bool bHTML) const
{
	return pModuleConfig->IsInsTblFormatNum(bHTML);
}

sal_Bool SwModule::IsInsTblChangeNumFormat(sal_Bool bHTML) const
{
	return pModuleConfig->IsInsTblChangeNumFormat(bHTML);
}

/*--------------------------------------------------------------------
	Beschreibung:
 --------------------------------------------------------------------*/

sal_Bool SwModule::IsInsTblAlignNum(sal_Bool bHTML) const
{
	return pModuleConfig->IsInsTblAlignNum(bHTML);
}

/*--------------------------------------------------------------------
	Beschreibung:
 --------------------------------------------------------------------*/

const Color &SwModule::GetRedlineMarkColor()
{
	return pModuleConfig->GetMarkAlignColor();
}

/*-----------------03.03.98 16:47-------------------

--------------------------------------------------*/
const SwViewOption*	SwModule::GetViewOption(sal_Bool bWeb)
{
	return GetUsrPref( bWeb );
}

// returne den definierten DocStat - WordDelimiter
const String& SwModule::GetDocStatWordDelim() const
{
	return pModuleConfig->GetWordDelimiter();
}
/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
// Durchreichen der Metric von der ModuleConfig (fuer HTML-Export)
sal_uInt16 SwModule::GetMetric( sal_Bool bWeb ) const
{
	SwMasterUsrPref* pPref;
	if(bWeb)
	{
		if(!pWebUsrPref)
			GetUsrPref(sal_True);
		pPref = pWebUsrPref;
	}
	else
	{
		if(!pUsrPref)
			GetUsrPref(sal_False);
		pPref = pUsrPref;
	}
    return static_cast< sal_uInt16 >(pPref->GetMetric());
}
/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
// Update-Stati durchreichen
sal_uInt16 SwModule::GetLinkUpdMode( sal_Bool ) const
{
	if(!pUsrPref)
		GetUsrPref(sal_False);
    return (sal_uInt16)pUsrPref->GetUpdateLinkMode();
}
/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
SwFldUpdateFlags SwModule::GetFldUpdateFlags( sal_Bool ) const
{
	if(!pUsrPref)
		GetUsrPref(sal_False);
    return pUsrPref->GetFldUpdateFlags();
}
/* -----------------------------28.09.00 14:18--------------------------------

 ---------------------------------------------------------------------------*/
void SwModule::ApplyFldUpdateFlags(SwFldUpdateFlags eFldFlags)
{
	if(!pUsrPref)
		GetUsrPref(sal_False);
    pUsrPref->SetFldUpdateFlags(eFldFlags);
}
/* -----------------------------28.09.00 14:18--------------------------------

 ---------------------------------------------------------------------------*/
void SwModule::ApplyLinkMode(sal_Int32 nNewLinkMode)
{
	if(!pUsrPref)
		GetUsrPref(sal_False);
	pUsrPref->SetUpdateLinkMode(nNewLinkMode);
}
/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
void SwModule::CheckSpellChanges( sal_Bool bOnlineSpelling,
		sal_Bool bIsSpellWrongAgain, sal_Bool bIsSpellAllAgain, sal_Bool bSmartTags )
{
	sal_Bool bOnlyWrong = bIsSpellWrongAgain && !bIsSpellAllAgain;
	sal_Bool bInvalid = bOnlyWrong || bIsSpellAllAgain;
	if( bOnlineSpelling || bInvalid )
	{
		TypeId aType = TYPE(SwDocShell);
		for( SwDocShell *pDocSh = (SwDocShell*)SfxObjectShell::GetFirst(&aType);
			 pDocSh;
			 pDocSh = (SwDocShell*)SfxObjectShell::GetNext( *pDocSh, &aType ) )
		{
			SwDoc* pTmp = pDocSh->GetDoc();
			if ( pTmp->GetCurrentViewShell() )	//swmod 071108//swmod 071225
            {
				pTmp->SpellItAgainSam( bInvalid, bOnlyWrong, bSmartTags );
                ViewShell* pViewShell = 0;
                pTmp->GetEditShell( &pViewShell );
                if ( bSmartTags && pViewShell && pViewShell->GetWin() )
                    pViewShell->GetWin()->Invalidate();
            }
		}
//		pSpell->SetSpellWrongAgain( sal_False );
//		pSpell->SetSpellAllAgain( sal_False );
	}
}

void SwModule::ApplyDefaultPageMode(sal_Bool bIsSquaredPageMode)
{
    if(!pUsrPref)
        GetUsrPref(sal_False);
    pUsrPref->SetDefaultPageMode(bIsSquaredPageMode);
}

#ifdef USE_JAVA

Sequence< OUString > SwPrintOptionsReset::GetPropertyNames()
{
	static const char* aPropNames[] =
	{
		"PrintOptionsReset",	//  0
		"WebPrintOptionsReset"	//  1
	};

    const int nCount = 2;
	Sequence< OUString > aNames(nCount);
	OUString* pNames = aNames.getArray();
	for(int i = 0; i < nCount; i++)
		pNames[i] = OUString::createFromAscii(aPropNames[i]);

	return aNames;
}

SwPrintOptionsReset::SwPrintOptionsReset() :
	ConfigItem(C2U("Office.WriterPrintOptionsReset/Reset"), CONFIG_MODE_DELAYED_UPDATE|CONFIG_MODE_RELEASE_TREE),
    mbPrintOptionsReset(sal_False),
    mbWebPrintOptionsReset(sal_False)
{
    Sequence< OUString > aNames = GetPropertyNames();
	Sequence< Any > aValues = GetProperties(aNames);
	const Any* pValues = aValues.getConstArray();
	DBG_ASSERT(aValues.getLength() == aNames.getLength(), "GetProperties failed");
	if(aValues.getLength() == aNames.getLength())
	{
		for(int nProp = 0; nProp < aNames.getLength(); nProp++)
		{
			if(pValues[nProp].hasValue())
			{
				switch(nProp)
				{
					case  0: mbPrintOptionsReset = *(sal_Bool*)pValues[nProp].getValue(); break;
					case  1: mbWebPrintOptionsReset = *(sal_Bool*)pValues[nProp].getValue();  break;
                }
			}
		}
	}
}

void SwPrintOptionsReset::Commit()
{
	Sequence< OUString > aNames = GetPropertyNames();
	Sequence<Any> aValues(aNames.getLength());
	Any* pValues = aValues.getArray();

	const Type& rType = ::getBooleanCppuType();
	sal_Bool bVal;
	for(int nProp = 0; nProp < aNames.getLength(); nProp++)
	{
		switch(nProp)
		{
			case  0: bVal = mbPrintOptionsReset; pValues[nProp].setValue(&bVal, rType); break;
			case  1: bVal = mbWebPrintOptionsReset; pValues[nProp].setValue(&bVal, rType); break;
        }
	}
	PutProperties(aNames, aValues);
}

#endif	// USE_JAVA
