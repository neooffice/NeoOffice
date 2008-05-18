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
 *    Modified May 2008 by Patrick Luby. NeoOffice is distributed under
 *    GPL only under modification term 3 of the LGPL.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#include <pdfwriter_impl.hxx>
#include <bitmapex.hxx>
#include <image.hxx>

using namespace vcl;

PDFWriter::AnyWidget::~AnyWidget()
{
}

PDFWriter::PDFWriter( const PDFWriter::PDFWriterContext& rContext )
        :
#ifdef USE_JAVA
        pImplementation( new PDFWriterImpl( rContext, PDFWriterImpl::FontSubsetData() ) )
#else	// USE_JAVA
        pImplementation( new PDFWriterImpl( rContext ) )
#endif	// USE_JAVA
{
}

PDFWriter::~PDFWriter()
{
    delete (PDFWriterImpl*)pImplementation;
}

OutputDevice* PDFWriter::GetReferenceDevice()
{
    return ((PDFWriterImpl*)pImplementation)->getReferenceDevice();
}

sal_Int32 PDFWriter::NewPage( sal_Int32 nPageWidth, sal_Int32 nPageHeight, Orientation eOrientation )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isUsingMetaFile() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaNewPagePDFAction( nPageWidth, nPageHeight, eOrientation ) );
#endif	// USE_JAVA
    return ((PDFWriterImpl*)pImplementation)->newPage( nPageWidth, nPageHeight, eOrientation );
}

bool PDFWriter::Emit()
{
#ifdef USE_JAVA
    // Replay meta actions
    if ( !((PDFWriterImpl*)pImplementation)->isUsingMetaFile() )
    {
		((PDFWriterImpl*)pImplementation)->emit();

        bool bRet = false;
		void *pOldImplementation = pImplementation;
        pImplementation = new PDFWriterImpl( ((PDFWriterImpl*)pOldImplementation)->getContext(), ((PDFWriterImpl*)pOldImplementation)->getSubsets() );

		const GDIMetaFile &rMtf = ((PDFWriterImpl*)pOldImplementation)->getMetaFile();
        for ( ULONG i = 0, nCount = rMtf.GetActionCount(); i < nCount; i++ )
        {
            const MetaAction *pAction = rMtf.GetAction( i );
            const USHORT nType = pAction->GetType();

            switch( nType )
            {
                case( META_NEW_PAGE_PDF_ACTION ):
                {
                    const MetaNewPagePDFAction* pA = (const MetaNewPagePDFAction*) pAction;
                    NewPage( pA->GetPageWidth(), pA->GetPageHeight(), pA->GetOrientation() );
                }
                break;

                case( META_FONT_ACTION ):
                {
                    const MetaFontAction* pA = (const MetaFontAction*) pAction;
                    SetFont( pA->GetFont() );
                }
                break;

                case( META_TEXT_ACTION ):
                {
                    const MetaTextAction* pA = (const MetaTextAction*) pAction;
                    DrawText( pA->GetPoint(), pA->GetText() );
                }
                break;

                case( META_TEXTLINE_PDF_ACTION ):
                {
                    const MetaTextLinePDFAction* pA = (const MetaTextLinePDFAction*) pAction;
                    DrawTextLine( pA->GetStartPoint(), pA->GetWidth(), pA->GetStrikeout(), pA->GetUnderline(), pA->IsUnderlineAbove() );
                }
                break;

                case( META_TEXTARRAY_ACTION ):
                {
                    const MetaTextArrayAction* pA = (const MetaTextArrayAction*) pAction;
                    DrawTextArray( pA->GetPoint(), pA->GetText(), pA->GetDXArray(), pA->GetIndex(), pA->GetLen() );
                }
                break;

                case( META_STRETCHTEXT_ACTION ):
                {
                    const MetaStretchTextAction* pA = (const MetaStretchTextAction*) pAction;
                    DrawStretchText( pA->GetPoint(), pA->GetWidth(), pA->GetText(), pA->GetIndex(), pA->GetLen() );
                }
                break;

                case( META_TEXTRECT_ACTION ):
                {
                    const MetaTextRectAction* pA = (const MetaTextRectAction*) pAction;
                    DrawText( pA->GetRect(), pA->GetText(), pA->GetStyle() );
                }
                break;

                case( META_LINE_ACTION ):
                {
                    const MetaLineAction* pA = (const MetaLineAction*) pAction;
                    DrawLine( pA->GetStartPoint(), pA->GetEndPoint(), pA->GetLineInfo() );
                }
                break;

                case( META_POLYGON_ACTION ):
                {
                    const MetaPolygonAction* pA = (const MetaPolygonAction*) pAction;
                    DrawPolygon( pA->GetPolygon() );
                }
                break;

                case( META_POLYLINE_ACTION ):
                {
                    const MetaPolyLineAction* pA = (const MetaPolyLineAction*) pAction;
                    DrawPolyLine( pA->GetPolygon(), pA->GetLineInfo() );
                }
                break;

                case( META_POLYLINE_PDF_ACTION ):
                {
                    const MetaPolyLinePDFAction* pA = (const MetaPolyLinePDFAction*) pAction;
                    DrawPolyLine( pA->GetPolygon(), pA->GetExtLineInfo() );
                }
                break;

                case( META_RECT_ACTION ):
                {
                    const MetaRectAction* pA = (const MetaRectAction*) pAction;
                    DrawRect( pA->GetRect() );
                }
                break;

                case( META_ROUNDRECT_ACTION ):
                {
                    const MetaRoundRectAction* pA = (const MetaRoundRectAction*) pAction;
                    DrawRect( pA->GetRect(), pA->GetHorzRound(), pA->GetVertRound() );
                }
                break;

                case( META_ELLIPSE_ACTION ):
                {
                    const MetaEllipseAction* pA = (const MetaEllipseAction*) pAction;
                    DrawEllipse( pA->GetRect() );
                }
                break;

                case( META_PIE_ACTION ):
                {
                    const MetaArcAction* pA = (const MetaArcAction*) pAction;
                    DrawArc( pA->GetRect(), pA->GetStartPoint(), pA->GetEndPoint() );
                }
                break;

                case( META_CHORD_ACTION ):
                {
                    const MetaChordAction* pA = (const MetaChordAction*) pAction;
                    DrawArc( pA->GetRect(), pA->GetStartPoint(), pA->GetEndPoint() );
                }
                break;

                case( META_ARC_ACTION ):
                {
                    const MetaArcAction* pA = (const MetaArcAction*) pAction;
                    DrawArc( pA->GetRect(), pA->GetStartPoint(), pA->GetEndPoint() );
                }
                break;

                case( META_POLYPOLYGON_ACTION ):
                {
                    const MetaPolyPolygonAction* pA = (const MetaPolyPolygonAction*) pAction;
                    DrawPolyPolygon( pA->GetPolyPolygon() );
                }
                break;

                case( META_PIXEL_ACTION ):
                {
                    const MetaPixelAction* pA = (const MetaPixelAction*) pAction;
                    DrawPixel( pA->GetPoint(), pA->GetColor() );
                }
                break;

                case( META_PIXEL_PDF_ACTION ):
                {
                    const MetaPixelPDFAction* pA = (const MetaPixelPDFAction*) pAction;
                    DrawPixel( pA->GetPoints(), pA->GetColors() );
                }
                break;

                case( META_BMP_ACTION ):
                {
                    const MetaBmpAction* pA = (const MetaBmpAction*) pAction;
                    DrawBitmap( pA->GetPoint(), pA->GetBitmap() );
                }
                break;

                case( META_BMPSCALE_ACTION ):
                {
                    const MetaBmpScaleAction* pA = (const MetaBmpScaleAction*) pAction;
                    DrawBitmap( pA->GetPoint(), pA->GetSize(), pA->GetBitmap() );
                }
                break;

                case( META_BMPEX_ACTION ):
                {
                    const MetaBmpExAction* pA = (const MetaBmpExAction*) pAction;
                    DrawBitmapEx( pA->GetPoint(), pA->GetBitmapEx() );
                }
                break;

                case( META_BMPEXSCALE_ACTION ):
                {
                    const MetaBmpExScaleAction* pA = (const MetaBmpExScaleAction*) pAction;
                    DrawBitmapEx( pA->GetPoint(), pA->GetSize(), pA->GetBitmapEx() );
                }
                break;

                case( META_MASK_ACTION ):
                {
                    const MetaMaskScaleAction* pA = (const MetaMaskScaleAction*) pAction;
                    DrawMask( pA->GetPoint(), pA->GetBitmap(), pA->GetColor() );
                }
                break;

                case( META_MASKSCALE_ACTION ):
                {
                    const MetaMaskScaleAction* pA = (const MetaMaskScaleAction*) pAction;
                    DrawMask( pA->GetPoint(), pA->GetSize(), pA->GetBitmap(), pA->GetColor() );
                }
                break;

                case( META_GRADIENT_ACTION ):
                {
                    const MetaGradientAction* pA = (const MetaGradientAction*) pAction;
                    DrawGradient( pA->GetRect(), pA->GetGradient() );
                }
                break;

                case( META_GRADIENTEX_ACTION ):
                {
                    const MetaGradientExAction* pA = (const MetaGradientExAction*) pAction;
                    DrawGradient( pA->GetPolyPolygon(), pA->GetGradient() );
                }
                break;

                case META_HATCH_ACTION:
                {
                    const MetaHatchAction* pA = (const MetaHatchAction*) pAction;
                    DrawHatch( pA->GetPolyPolygon(), pA->GetHatch() );
                }
                break;

                case( META_WALLPAPER_ACTION ):
                {
                    const MetaWallpaperAction* pA = (const MetaWallpaperAction*) pAction;
                    DrawWallpaper( pA->GetRect(), pA->GetWallpaper() );
                }
                break;

                case( META_TRANSPARENT_ACTION ):
                {
                    const MetaTransparentAction* pA = (const MetaTransparentAction*) pAction;
                    DrawTransparent( pA->GetPolyPolygon(), pA->GetTransparence() );
                }
                break;

                case( META_PUSH_ACTION ):
                {
                    const MetaPushAction* pA = (const MetaPushAction*) pAction;

                    Push( pA->GetFlags() );
                }
                break;

                case( META_POP_ACTION ):
                {
                    Pop();
                }
                break;

                case( META_MAPMODE_ACTION ):
                {
                    const MetaMapModeAction* pA = (const MetaMapModeAction*) pAction;
                    SetMapMode( pA->GetMapMode() );
                }
                break;

                case( META_LINECOLOR_ACTION ):
                {
                    const MetaLineColorAction* pA = (const MetaLineColorAction*) pAction;
                    SetLineColor( pA->GetColor() );
                }
                break;

                case( META_FILLCOLOR_ACTION ):
                {
                    const MetaFillColorAction* pA = (const MetaFillColorAction*) pAction;
                    SetFillColor( pA->GetColor() );
                }
                break;

                case( META_CLIPREGION_ACTION ):
                {
                    const MetaClipRegionAction* pA = (const MetaClipRegionAction*) pAction;
                    if( pA->IsClipping() )
                        SetClipRegion( pA->GetRegion() );
                    else
                        SetClipRegion();
                }
                break;

                case( META_MOVECLIPREGION_ACTION ):
                {
                    const MetaMoveClipRegionAction* pA = (const MetaMoveClipRegionAction*) pAction;
                    MoveClipRegion( pA->GetHorzMove(), pA->GetVertMove() );
                }
                break;

                case( META_ISECTRECTCLIPREGION_ACTION ):
                {
                    const MetaISectRectClipRegionAction* pA = (const MetaISectRectClipRegionAction*) pAction;
                    IntersectClipRegion( pA->GetRect() );
                }
                break;

                case( META_ISECTREGIONCLIPREGION_ACTION ):
                {
                   const MetaISectRegionClipRegionAction* pA = (const MetaISectRegionClipRegionAction*) pAction;
                   IntersectClipRegion( pA->GetRegion() );
                }
                break;

                case( META_ANTIALIAS_PDF_ACTION ):
                {
                    const MetaAntiAliasPDFAction* pA = (const MetaAntiAliasPDFAction*) pAction;
                    SetAntialiasing( pA->GetAntiAlias() );
                }
                break;

                case( META_LAYOUTMODE_ACTION ):
                {
                    const MetaLayoutModeAction* pA = (const MetaLayoutModeAction*) pAction;
                    SetLayoutMode( pA->GetLayoutMode() );
                }
                break;

                case( META_TEXTCOLOR_ACTION ):
                {
                    const MetaTextColorAction* pA = (const MetaTextColorAction*) pAction;
                    SetTextColor( pA->GetColor() );
                }
                break;

                case( META_TEXTFILLCOLOR_ACTION ):
                {
                    const MetaTextFillColorAction* pA = (const MetaTextFillColorAction*) pAction;
                    if ( pA->IsSetting() )
                        SetTextFillColor( pA->GetColor() );
                    else
                        SetTextFillColor();
                }
                break;

                case( META_TEXTLINECOLOR_ACTION ):
                {
                    const MetaTextLineColorAction* pA = (const MetaTextLineColorAction*) pAction;
                    if ( pA->IsSetting() )
                        SetTextLineColor( pA->GetColor() );
                    else
                        SetTextLineColor();
                }
                break;

                case( META_TEXTALIGN_ACTION ):
                {
                    const MetaTextAlignAction* pA = (const MetaTextAlignAction*) pAction;
                    SetTextAlign( pA->GetTextAlign() );
                }
                break;

                case( META_JPG_PDF_ACTION ):
                {
                    const MetaJpgPDFAction* pA = (const MetaJpgPDFAction*) pAction;
                    DrawJPGBitmap( (SvStream&)pA->GetStream(), pA->IsTrueColor(), pA->GetSize(), pA->GetRect(), pA->GetMask() );
                }
                break;

	            case( META_CREATELINK_PDF_ACTION ):
                {
                    const MetaCreateLinkPDFAction* pA = (const MetaCreateLinkPDFAction*) pAction;
                    CreateLink( pA->GetRect(), pA->GetPage() );
                }
                break;

	            case( META_CREATEDEST_PDF_ACTION ):
                {
                    const MetaCreateDestPDFAction* pA = (const MetaCreateDestPDFAction*) pAction;
                    CreateDest( pA->GetRect(), pA->GetPage(), pA->GetType() );
                }
                break;

	            case( META_SETLINKDEST_PDF_ACTION ):
                {
                    const MetaSetLinkDestPDFAction* pA = (const MetaSetLinkDestPDFAction*) pAction;
                    SetLinkDest( pA->GetLink(), pA->GetDest() );
                }
                break;

	            case( META_SETLINKURL_PDF_ACTION ):
                {
                    const MetaSetLinkUrlPDFAction* pA = (const MetaSetLinkUrlPDFAction*) pAction;
                    SetLinkURL( pA->GetLink(), pA->GetURL() );
                }
                break;

	            case( META_SETLINKPROPERTYID_PDF_ACTION ):
                {
                    const MetaSetLinkPropertyIdPDFAction* pA = (const MetaSetLinkPropertyIdPDFAction*) pAction;
                    SetLinkPropertyID( pA->GetLink(), pA->GetProperty() );
                }
                break;

	            case( META_CREATEOUTLINEITEM_PDF_ACTION ):
                {
                    const MetaCreateOutlineItemPDFAction* pA = (const MetaCreateOutlineItemPDFAction*) pAction;
                    CreateOutlineItem( pA->GetParent(), pA->GetText(), pA->GetDest() );
                }
                break;

	            case( META_SETOUTLINEITEMPARENT_PDF_ACTION ):
                {
                    const MetaSetOutlineItemParentPDFAction* pA = (const MetaSetOutlineItemParentPDFAction*) pAction;
                    SetOutlineItemParent( pA->GetItem(), pA->GetParent() );
                }
                break;

	            case( META_SETOUTLINEITEMTEXT_PDF_ACTION ):
                {
                    const MetaSetOutlineItemTextPDFAction* pA = (const MetaSetOutlineItemTextPDFAction*) pAction;
                    SetOutlineItemText( pA->GetItem(), pA->GetText() );
                }
                break;

	            case( META_SETOUTLINEITEMDEST_PDF_ACTION ):
                {
                    const MetaSetOutlineItemDestPDFAction* pA = (const MetaSetOutlineItemDestPDFAction*) pAction;
                    SetOutlineItemDest( pA->GetItem(), pA->GetDest() );
                }
                break;

	            case( META_CREATENOTE_PDF_ACTION ):
                {
                    const MetaCreateNotePDFAction* pA = (const MetaCreateNotePDFAction*) pAction;
                    CreateNote( pA->GetRect(), pA->GetNote(), pA->GetPage() );
                }
                break;

	            case( META_BEGINSTRUCTUREELEMENT_PDF_ACTION ):
                {
                    const MetaBeginStructureElementPDFAction* pA = (const MetaBeginStructureElementPDFAction*) pAction;
                    BeginStructureElement( pA->GetType() );
                }
                break;

	            case( META_ENDSTRUCTUREELEMENT_PDF_ACTION ):
                {
                    EndStructureElement();
                }
                break;

	            case( META_SETCURRENTSTRUCTUREELEMENT_PDF_ACTION ):
                {
                    const MetaSetCurrentStructureElementPDFAction* pA = (const MetaSetCurrentStructureElementPDFAction*) pAction;
                    SetCurrentStructureElement( pA->GetElement() );
                }
                break;

	            case( META_SETSTRUCTUREATTRIBUTE_PDF_ACTION ):
                {
                    const MetaSetStructureAttributePDFAction* pA = (const MetaSetStructureAttributePDFAction*) pAction;
                    SetStructureAttribute( pA->GetAttribute(), pA->GetValue() );
                }
                break;

	            case( META_SETSTRUCTUREATTRIBUTENUMERICAL_PDF_ACTION ):
                {
                    const MetaSetStructureAttributeNumericalPDFAction* pA = (const MetaSetStructureAttributeNumericalPDFAction*) pAction;
                    SetStructureAttributeNumerical( pA->GetAttribute(), pA->GetValue() );
                }
                break;

	            case( META_SETSTRUCTUREBOUNDINGBOX_PDF_ACTION ):
                {
                    const MetaSetStructureBoundingBoxPDFAction* pA = (const MetaSetStructureBoundingBoxPDFAction*) pAction;
                    SetStructureBoundingBox( pA->GetRect() );
                }
                break;

	            case( META_SETACTUALTEXT_PDF_ACTION ):
                {
                    const MetaSetActualTextPDFAction* pA = (const MetaSetActualTextPDFAction*) pAction;
                    SetActualText( pA->GetText() );
                }
                break;

	            case( META_SETALTERNATETEXT_PDF_ACTION ):
                {
                    const MetaSetAlternateTextPDFAction* pA = (const MetaSetAlternateTextPDFAction*) pAction;
                    SetAlternateText( pA->GetText() );
                }
                break;

                case( META_SETAUTOADVANCETIME_PDF_ACTION ):
                {
                    const MetaSetAutoAdvanceTimePDFAction* pA = (const MetaSetAutoAdvanceTimePDFAction*) pAction;
                    SetAutoAdvanceTime( pA->GetSeconds(), pA->GetPage() );
                }
                break;

	            case( META_SETPAGETRANSITION_PDF_ACTION ):
                {
                    const MetaSetPageTransitionPDFAction* pA = (const MetaSetPageTransitionPDFAction*) pAction;
                    SetPageTransition( pA->GetType(), pA->GetMilliSeconds(), pA->GetPage() );
                }
                break;

	            case( META_CREATECONTROL_PDF_ACTION ):
                {
                    const MetaCreateControlPDFAction* pA = (const MetaCreateControlPDFAction*) pAction;
                    CreateControl( pA->GetControl(), pA->GetPage() );
                }
                break;

	            case( META_DIGITLANGUAGE_PDF_ACTION ):
                {
                    const MetaDigitLanguagePDFAction* pA = (const MetaDigitLanguagePDFAction*) pAction;
                    SetDigitLanguage( pA->GetLanguage() );
                }
                break;

	            case( META_BEGINPATTERN_PDF_ACTION ):
                {
                    BeginPattern();
                }
                break;

	            case( META_ENDPATTERN_PDF_ACTION ):
                {
                    const MetaEndPatternPDFAction* pA = (const MetaEndPatternPDFAction*) pAction;
                    EndPattern( pA->GetRect(), pA->GetTransform() );
                }
                break;

	            case( META_POLYPOLYGON_PDF_ACTION ):
                {
                    const MetaPolyPolygonPDFAction* pA = (const MetaPolyPolygonPDFAction*) pAction;
                    DrawPolyPolygon( pA->GetPolyPolygon(), pA->GetPattern(), pA->IsEOFill() );
                }
                break;

                case( META_BEGINTRANSPARENCYGROUP_PDF_ACTION ):
                {
                    BeginTransparencyGroup();
                }
                break;

                case( META_ENDTRANSPARENCYGROUP_PDF_ACTION ):
                {
                    const MetaEndTransparencyGroupPDFAction* pA = (const MetaEndTransparencyGroupPDFAction*) pAction;
                    EndTransparencyGroup( pA->GetBoundingRect(), pA->GetTransparentPercent() );
                }
                break;

                case( META_ENDTRANSPARENCYGROUPMASK_PDF_ACTION ):
                {
                    const MetaEndTransparencyGroupMaskPDFAction* pA = (const MetaEndTransparencyGroupMaskPDFAction*) pAction;
                    EndTransparencyGroup( pA->GetBoundingRect(), pA->GetAlphaMask() );
                }
                break;

                case( META_SETDOCINFO_PDF_ACTION ):
                {
                    const MetaSetDocInfoPDFAction* pA = (const MetaSetDocInfoPDFAction*) pAction;
                    SetDocInfo( pA->GetDocInfo() );
                }
                break;

                default:
                    DBG_ERROR( "PDFWriterImpl::emit: unsupported MetaAction #" );
                break;
            }

        }

        bRet = ((PDFWriterImpl*)pImplementation)->emit();

        delete (PDFWriterImpl*)pImplementation;
        pImplementation = pOldImplementation;

		return bRet;
    }
	else
#endif	// USE_JAVA
    return ((PDFWriterImpl*)pImplementation)->emit();
}

PDFWriter::PDFVersion PDFWriter::GetVersion() const
{
    return ((PDFWriterImpl*)pImplementation)->getVersion();
}

void PDFWriter::SetDocInfo( const PDFDocInfo& rInfo )
{
    if ( !((PDFWriterImpl*)pImplementation)->isUsingMetaFile() )
    ((PDFWriterImpl*)pImplementation)->setDocInfo( rInfo );
}

const PDFDocInfo& PDFWriter::GetDocInfo() const
{
    return ((PDFWriterImpl*)pImplementation)->getDocInfo();
}

void PDFWriter::SetFont( const Font& rFont )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isUsingMetaFile() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaFontAction( rFont ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->setFont( rFont );
}

void PDFWriter::DrawText( const Point& rPos, const String& rText )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isUsingMetaFile() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaTextAction( rPos, rText, 0, STRING_LEN ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->drawText( rPos, rText );
}

void PDFWriter::DrawTextLine(
                             const Point& rPos,
                             long nWidth,
                             FontStrikeout eStrikeout,
                             FontUnderline eUnderline,
                             BOOL bUnderlineAbove )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isUsingMetaFile() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaTextLinePDFAction( rPos, nWidth, eStrikeout, eUnderline, bUnderlineAbove ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->drawTextLine( rPos, nWidth, eStrikeout, eUnderline, bUnderlineAbove );
}

void PDFWriter::DrawTextArray(
                              const Point& rStartPt,
                              const XubString& rStr,
                              const sal_Int32* pDXAry,
                              xub_StrLen nIndex,
                              xub_StrLen nLen )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isUsingMetaFile() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaTextArrayAction( rStartPt, rStr, pDXAry, nIndex, nLen ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->drawTextArray( rStartPt, rStr, pDXAry, nIndex, nLen );
}

void PDFWriter::DrawStretchText(
                                const Point& rStartPt,
                                ULONG nWidth,
                                const XubString& rStr,
                                xub_StrLen nIndex,
                                xub_StrLen nLen )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isUsingMetaFile() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaStretchTextAction( rStartPt, nWidth, rStr, nIndex, nLen ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->drawStretchText( rStartPt, nWidth, rStr, nIndex, nLen );
}

void PDFWriter::DrawText(
                         const Rectangle& rRect,
                         const XubString& rStr,
                         USHORT nStyle )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isUsingMetaFile() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaTextRectAction( rRect, rStr, nStyle ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->drawText( rRect, rStr, nStyle );
}

void PDFWriter::DrawLine( const Point& rStart, const Point& rStop )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isUsingMetaFile() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaLineAction( rStart, rStop ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->drawLine( rStart, rStop );
}

void PDFWriter::DrawLine( const Point& rStart, const Point& rStop, const LineInfo& rInfo )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isUsingMetaFile() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaLineAction( rStart, rStop, rInfo ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->drawLine( rStart, rStop, rInfo );
}

void PDFWriter::DrawPolygon( const Polygon& rPoly )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isUsingMetaFile() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaPolygonAction( rPoly ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->drawPolygon( rPoly );
}

void PDFWriter::DrawPolyLine( const Polygon& rPoly )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isUsingMetaFile() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaPolyLineAction( rPoly ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->drawPolyLine( rPoly );
}

void PDFWriter::DrawRect( const Rectangle& rRect )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isUsingMetaFile() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaRectAction( rRect ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->drawRectangle( rRect );
}

void PDFWriter::DrawRect( const Rectangle& rRect, ULONG nHorzRound, ULONG nVertRound )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isUsingMetaFile() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaRoundRectAction( rRect, nHorzRound, nVertRound ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->drawRectangle( rRect, nHorzRound, nVertRound );
}

void PDFWriter::DrawEllipse( const Rectangle& rRect )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isUsingMetaFile() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaEllipseAction( rRect ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->drawEllipse( rRect );
}

void PDFWriter::DrawArc( const Rectangle& rRect, const Point& rStart, const Point& rStop )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isUsingMetaFile() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaArcAction( rRect, rStart, rStop ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->drawArc( rRect, rStart, rStop, false, false );
}

void PDFWriter::DrawPie( const Rectangle& rRect, const Point& rStart, const Point& rStop )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isUsingMetaFile() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaPieAction( rRect, rStart, rStop ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->drawArc( rRect, rStart, rStop, true, false );
}

void PDFWriter::DrawChord( const Rectangle& rRect, const Point& rStart, const Point& rStop )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isUsingMetaFile() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaChordAction( rRect, rStart, rStop ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->drawArc( rRect, rStart, rStop, false, true );
}

void PDFWriter::DrawPolyLine( const Polygon& rPoly, const LineInfo& rInfo )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isUsingMetaFile() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaPolyLineAction( rPoly, rInfo ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->drawPolyLine( rPoly, rInfo );
}

void PDFWriter::DrawPolyLine( const Polygon& rPoly, const ExtLineInfo& rInfo )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isUsingMetaFile() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaPolyLinePDFAction( rPoly, rInfo ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->drawPolyLine( rPoly, rInfo );
}

void PDFWriter::DrawPolyPolygon( const PolyPolygon& rPolyPoly )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isUsingMetaFile() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaPolyPolygonAction( rPolyPoly ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->drawPolyPolygon( rPolyPoly );
}

void PDFWriter::DrawPixel( const Point& rPos, const Color& rColor )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isUsingMetaFile() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaPixelAction( rPos, rColor ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->drawPixel( rPos, rColor );
}

void PDFWriter::DrawPixel( const Polygon& rPts, const Color* pColors )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isUsingMetaFile() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaPixelPDFAction( rPts, pColors ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->drawPixel( rPts, pColors );
}

void PDFWriter::DrawBitmap( const Point& rDestPt, const Bitmap& rBitmap )
{
    Size aSize = OutputDevice::LogicToLogic( rBitmap.GetPrefSize(),
                                             rBitmap.GetPrefMapMode(),
                                             ((PDFWriterImpl*)pImplementation)->getMapMode() );
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isUsingMetaFile() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaBmpAction( rDestPt, rBitmap ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->drawBitmap( rDestPt, aSize, rBitmap );
}

void PDFWriter::DrawBitmap( const Point& rDestPt, const Size& rDestSize, const Bitmap& rBitmap )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isUsingMetaFile() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaBmpScaleAction( rDestPt, rDestSize, rBitmap ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->drawBitmap( rDestPt, rDestSize, rBitmap );
}

void PDFWriter::DrawBitmap( const Point& rDestPt, const Size& rDestSize, const Point& rSrcPtPixel, const Size& rSrcSizePixel, const Bitmap& rBitmap )
{
    Bitmap aBitmap( rBitmap );
    aBitmap.Crop( Rectangle( rSrcPtPixel, rSrcSizePixel ) );
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isUsingMetaFile() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaBmpScaleAction( rDestPt, rDestSize, aBitmap ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->drawBitmap( rDestPt, rDestSize, aBitmap );
}

void PDFWriter::DrawBitmapEx( const Point& rDestPt, const BitmapEx& rBitmap )
{
    Size aSize = OutputDevice::LogicToLogic( rBitmap.GetPrefSize(),
                                             rBitmap.GetPrefMapMode(),
                                             ((PDFWriterImpl*)pImplementation)->getMapMode() );
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isUsingMetaFile() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaBmpExAction( rDestPt, rBitmap ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->drawBitmap( rDestPt, aSize, rBitmap );
}

void PDFWriter::DrawBitmapEx( const Point& rDestPt, const Size& rDestSize, const BitmapEx& rBitmap )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isUsingMetaFile() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaBmpExScaleAction( rDestPt, rDestSize, rBitmap ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->drawBitmap( rDestPt, rDestSize, rBitmap );
}

void PDFWriter::DrawBitmapEx( const Point& rDestPt, const Size& rDestSize, const Point& rSrcPtPixel, const Size& rSrcSizePixel, const BitmapEx& rBitmap )
{
    BitmapEx aBitmap( rBitmap );
    aBitmap.Crop( Rectangle( rSrcPtPixel, rSrcSizePixel ) );
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isUsingMetaFile() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaBmpExScaleAction( rDestPt, rDestSize, aBitmap ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->drawBitmap( rDestPt, rDestSize, aBitmap );
}

void PDFWriter::DrawMask( const Point& rDestPt, const Bitmap& rBitmap, const Color& rMaskColor )
{
    Size aSize = OutputDevice::LogicToLogic( rBitmap.GetPrefSize(),
                                             rBitmap.GetPrefMapMode(),
                                             ((PDFWriterImpl*)pImplementation)->getMapMode() );
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isUsingMetaFile() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaMaskAction( rDestPt, rBitmap, rMaskColor ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->drawMask( rDestPt, aSize, rBitmap, rMaskColor );
}

void PDFWriter::DrawMask( const Point& rDestPt, const Size& rDestSize, const Bitmap& rBitmap, const Color& rMaskColor )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isUsingMetaFile() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaMaskScaleAction( rDestPt, rDestSize, rBitmap, rMaskColor ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->drawMask( rDestPt, rDestSize, rBitmap, rMaskColor );
}

void PDFWriter::DrawMask( const Point& rDestPt, const Size& rDestSize, const Point& rSrcPtPixel, const Size& rSrcSizePixel, const Bitmap& rBitmap, const Color& rMaskColor )
{
    Bitmap aBitmap( rBitmap );
    aBitmap.Crop( Rectangle( rSrcPtPixel, rSrcSizePixel ) );
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isUsingMetaFile() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaMaskScaleAction( rDestPt, rDestSize, aBitmap, rMaskColor ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->drawMask( rDestPt, rDestSize, aBitmap, rMaskColor );
}

void PDFWriter::DrawGradient( const Rectangle& rRect, const Gradient& rGradient )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isUsingMetaFile() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaGradientAction( rRect, rGradient ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->drawGradient( rRect, rGradient );
}

void PDFWriter::DrawGradient( const PolyPolygon& rPolyPoly, const Gradient& rGradient )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isUsingMetaFile() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaGradientExAction( rPolyPoly, rGradient ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->drawGradient( rPolyPoly, rGradient );
}

void PDFWriter::DrawHatch( const PolyPolygon& rPolyPoly, const Hatch& rHatch )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isUsingMetaFile() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaHatchAction( rPolyPoly, rHatch ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->drawHatch( rPolyPoly, rHatch );
}

void PDFWriter::DrawWallpaper( const Rectangle& rRect, const Wallpaper& rWallpaper )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isUsingMetaFile() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaWallpaperAction( rRect, rWallpaper ) );
#endif	// USE_JAVA 
    ((PDFWriterImpl*)pImplementation)->drawWallpaper( rRect, rWallpaper );
}

void PDFWriter::DrawTransparent( const PolyPolygon& rPolyPoly, USHORT nTransparencePercent )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isUsingMetaFile() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaTransparentAction( rPolyPoly, nTransparencePercent ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->drawTransparent( rPolyPoly, nTransparencePercent );
}

void PDFWriter::BeginTransparencyGroup()
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isUsingMetaFile() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaBeginTransparencyGroupPDFAction() );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->beginTransparencyGroup();
}

void PDFWriter::EndTransparencyGroup( const Rectangle& rRect, USHORT nTransparentPercent )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isUsingMetaFile() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaEndTransparencyGroupPDFAction( rRect, nTransparentPercent ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->endTransparencyGroup( rRect, nTransparentPercent );
}

void PDFWriter::EndTransparencyGroup( const Rectangle& rRect, const Bitmap& rAlphaMask )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isUsingMetaFile() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaEndTransparencyGroupMaskPDFAction( rRect, rAlphaMask ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->endTransparencyGroup( rRect, rAlphaMask );
}

void PDFWriter::Push( USHORT nFlags )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isUsingMetaFile() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaPushAction( nFlags ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->push( nFlags );
}

void PDFWriter::Pop()
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isUsingMetaFile() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaPopAction() ); 
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->pop();
}

void PDFWriter::SetMapMode( const MapMode& rMapMode )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isUsingMetaFile() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaMapModeAction( rMapMode ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->setMapMode( rMapMode );
}

void PDFWriter::SetMapMode()
{
    if ( !((PDFWriterImpl*)pImplementation)->isUsingMetaFile() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaMapModeAction( ((PDFWriterImpl*)pImplementation)->getMapMode() ) );
    ((PDFWriterImpl*)pImplementation)->setMapMode();
}

void PDFWriter::SetLineColor( const Color& rColor )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isUsingMetaFile() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaLineColorAction( rColor, TRUE ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->setLineColor( rColor );
}

void PDFWriter::SetFillColor( const Color& rColor )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isUsingMetaFile() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaFillColorAction( rColor, TRUE ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->setFillColor( rColor );
}

void PDFWriter::SetClipRegion()
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isUsingMetaFile() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaClipRegionAction( Region(), FALSE ) );
#endif	// USE_JAVA 
    ((PDFWriterImpl*)pImplementation)->clearClipRegion();
}

void PDFWriter::SetClipRegion( const Region& rRegion )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isUsingMetaFile() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaClipRegionAction( rRegion, TRUE ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->setClipRegion( rRegion );
}

void PDFWriter::MoveClipRegion( long nHorzMove, long nVertMove )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isUsingMetaFile() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaMoveClipRegionAction( nHorzMove, nVertMove ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->moveClipRegion( nHorzMove, nVertMove );
}

void PDFWriter::IntersectClipRegion( const Region& rRegion )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isUsingMetaFile() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaISectRegionClipRegionAction( rRegion ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->intersectClipRegion( rRegion );
}

void PDFWriter::IntersectClipRegion( const Rectangle& rRect )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isUsingMetaFile() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaISectRectClipRegionAction( rRect ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->intersectClipRegion( rRect );
}

void PDFWriter::SetAntialiasing( USHORT nMode )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isUsingMetaFile() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaAntiAliasPDFAction( nMode ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->setAntiAlias( (sal_Int32)nMode );
}

void PDFWriter::SetLayoutMode( ULONG nMode )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isUsingMetaFile() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaLayoutModeAction( nMode ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->setLayoutMode( (sal_Int32)nMode );
}

void PDFWriter::SetDigitLanguage( LanguageType eLang )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isUsingMetaFile() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaDigitLanguagePDFAction( eLang ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->setDigitLanguage( eLang );
}

void PDFWriter::SetTextColor( const Color& rColor )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isUsingMetaFile() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaTextColorAction( rColor ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->setTextColor( rColor );
}

void PDFWriter::SetTextFillColor()
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isUsingMetaFile() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaTextFillColorAction( Color( COL_TRANSPARENT ), FALSE ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->setTextFillColor();
}

void PDFWriter::SetTextFillColor( const Color& rColor )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isUsingMetaFile() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaTextFillColorAction( rColor, TRUE ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->setTextFillColor( rColor );
}

void PDFWriter::SetTextLineColor()
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isUsingMetaFile() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaTextLineColorAction( Color( COL_TRANSPARENT ), FALSE ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->setTextLineColor();
}

void PDFWriter::SetTextLineColor( const Color& rColor )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isUsingMetaFile() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaTextLineColorAction( rColor, TRUE ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->setTextLineColor( rColor );
}

void PDFWriter::SetTextAlign( ::TextAlign eAlign )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isUsingMetaFile() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaTextAlignAction( eAlign ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->setTextAlign( eAlign );
}

void PDFWriter::DrawJPGBitmap( SvStream& rStreamData, bool bIsTrueColor, const Size& rSrcSizePixel, const Rectangle& rTargetArea, const Bitmap& rMask )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isUsingMetaFile() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaJpgPDFAction( rStreamData, bIsTrueColor, rSrcSizePixel, rTargetArea, rMask ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->drawJPGBitmap( rStreamData, bIsTrueColor, rSrcSizePixel, rTargetArea, rMask );
}

sal_Int32 PDFWriter::CreateLink( const Rectangle& rRect, sal_Int32 nPageNr )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isUsingMetaFile() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaCreateLinkPDFAction( rRect, nPageNr ) );
#endif	// USE_JAVA
    return ((PDFWriterImpl*)pImplementation)->createLink( rRect, nPageNr );
}

sal_Int32 PDFWriter::CreateDest( const Rectangle& rRect, sal_Int32 nPageNr, PDFWriter::DestAreaType eType )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isUsingMetaFile() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaCreateDestPDFAction( rRect, nPageNr, eType ) );
#endif	// USE_JAVA
    return ((PDFWriterImpl*)pImplementation)->createDest( rRect, nPageNr, eType );
}

sal_Int32 PDFWriter::SetLinkDest( sal_Int32 nLinkId, sal_Int32 nDestId )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isUsingMetaFile() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaSetLinkDestPDFAction( nLinkId, nDestId ) );
#endif	// USE_JAVA
    return ((PDFWriterImpl*)pImplementation)->setLinkDest( nLinkId, nDestId );
}

sal_Int32 PDFWriter::SetLinkURL( sal_Int32 nLinkId, const rtl::OUString& rURL )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isUsingMetaFile() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaSetLinkUrlPDFAction( nLinkId, rURL ) );
#endif	// USE_JAVA
    return ((PDFWriterImpl*)pImplementation)->setLinkURL( nLinkId, rURL );
}

void PDFWriter::SetLinkPropertyID( sal_Int32 nLinkId, sal_Int32 nPropertyId )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isUsingMetaFile() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaSetLinkPropertyIdPDFAction( nLinkId, nPropertyId ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->setLinkPropertyId( nLinkId, nPropertyId );
}

sal_Int32 PDFWriter::CreateOutlineItem( sal_Int32 nParent, const rtl::OUString& rText, sal_Int32 nDestID )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isUsingMetaFile() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaCreateOutlineItemPDFAction( nParent, rText, nDestID ) );
#endif	// USE_JAVA
    return ((PDFWriterImpl*)pImplementation)->createOutlineItem( nParent, rText, nDestID );
}

sal_Int32 PDFWriter::SetOutlineItemParent( sal_Int32 nItem, sal_Int32 nNewParent )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isUsingMetaFile() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaSetOutlineItemParentPDFAction( nItem, nNewParent ) ); 
#endif	// USE_JAVA
    return ((PDFWriterImpl*)pImplementation)->setOutlineItemParent( nItem, nNewParent );
}

sal_Int32 PDFWriter::SetOutlineItemText( sal_Int32 nItem, const rtl::OUString& rText )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isUsingMetaFile() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaSetOutlineItemTextPDFAction( nItem, rText ) );
#endif	// USE_JAVA
    return  ((PDFWriterImpl*)pImplementation)->setOutlineItemText( nItem, rText );
}

sal_Int32 PDFWriter::SetOutlineItemDest( sal_Int32 nItem, sal_Int32 nDest )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isUsingMetaFile() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaSetOutlineItemDestPDFAction( nItem, nDest ) );
#endif	// USE_JAVA
    return ((PDFWriterImpl*)pImplementation)->setOutlineItemDest( nItem, nDest );
}

void PDFWriter::CreateNote( const Rectangle& rRect, const PDFNote& rNote, sal_Int32 nPageNr )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isUsingMetaFile() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaCreateNotePDFAction( rRect, rNote, nPageNr ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->createNote( rRect, rNote, nPageNr );
}

sal_Int32 PDFWriter::BeginStructureElement( PDFWriter::StructElement eType )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isUsingMetaFile() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaBeginStructureElementPDFAction( eType ) );
#endif	// USE_JAVA
    return ((PDFWriterImpl*)pImplementation)->beginStructureElement( eType );
}

void PDFWriter::EndStructureElement()
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isUsingMetaFile() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaEndStructureElementPDFAction() );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->endStructureElement();
}

bool PDFWriter::SetCurrentStructureElement( sal_Int32 nID )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isUsingMetaFile() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaSetCurrentStructureElementPDFAction( nID ) );
#endif	// USE_JAVA
    return ((PDFWriterImpl*)pImplementation)->setCurrentStructureElement( nID );
}

sal_Int32 PDFWriter::GetCurrentStructureElement()
{
    return ((PDFWriterImpl*)pImplementation)->getCurrentStructureElement();
}

bool PDFWriter::SetStructureAttribute( enum StructAttribute eAttr, enum StructAttributeValue eVal )
{
#ifdef USE_JAVA     
    if ( !((PDFWriterImpl*)pImplementation)->isUsingMetaFile() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaSetStructureAttributePDFAction( eAttr, eVal ) );          
#endif	// USE_JAVA 
    return ((PDFWriterImpl*)pImplementation)->setStructureAttribute( eAttr, eVal );
}

bool PDFWriter::SetStructureAttributeNumerical( enum StructAttribute eAttr, sal_Int32 nValue )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isUsingMetaFile() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaSetStructureAttributeNumericalPDFAction( eAttr, nValue ) );
#endif	// USE_JAVA
    return ((PDFWriterImpl*)pImplementation)->setStructureAttributeNumerical( eAttr, nValue );
}

void PDFWriter::SetStructureBoundingBox( const Rectangle& rRect )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isUsingMetaFile() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaSetStructureBoundingBoxPDFAction( rRect ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->setStructureBoundingBox( rRect );
}

void PDFWriter::SetActualText( const String& rText )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isUsingMetaFile() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaSetActualTextPDFAction( rText ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->setActualText( rText );
}

void PDFWriter::SetAlternateText( const String& rText )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isUsingMetaFile() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaSetAlternateTextPDFAction( rText ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->setAlternateText( rText );
}

void PDFWriter::SetAutoAdvanceTime( sal_uInt32 nSeconds, sal_Int32 nPageNr )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isUsingMetaFile() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaSetAutoAdvanceTimePDFAction( nSeconds, nPageNr ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->setAutoAdvanceTime( nSeconds, nPageNr );
}

void PDFWriter::SetPageTransition( PDFWriter::PageTransition eType, sal_uInt32 nMilliSec, sal_Int32 nPageNr )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isUsingMetaFile() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaSetPageTransitionPDFAction( eType, nMilliSec, nPageNr ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->setPageTransition( eType, nMilliSec, nPageNr );
}

sal_Int32 PDFWriter::CreateControl( const PDFWriter::AnyWidget& rControl, sal_Int32 nPageNr )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isUsingMetaFile() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaCreateControlPDFAction( rControl, nPageNr ) );
#endif	// USE_JAVA
    return ((PDFWriterImpl*)pImplementation)->createControl( rControl, nPageNr );
}

void PDFWriter::BeginPattern()
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isUsingMetaFile() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaBeginPatternPDFAction() );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->beginPattern();
}

sal_Int32 PDFWriter::EndPattern( const Rectangle& rCellBounds, const SvtGraphicFill::Transform& rTransform )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isUsingMetaFile() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaEndPatternPDFAction( rCellBounds, rTransform ) );
#endif	// USE_JAVA
    return ((PDFWriterImpl*)pImplementation)->endPattern( rCellBounds, rTransform );
}

void PDFWriter::DrawPolyPolygon( const PolyPolygon& rPolyPoly, sal_Int32 nPattern, bool bEOFill )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isUsingMetaFile() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaPolyPolygonPDFAction( rPolyPoly, nPattern, bEOFill ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->drawPolyPolygon( rPolyPoly, nPattern, bEOFill );
}
