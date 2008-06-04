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

#ifdef USE_JAVA

#ifndef _SV_GDIMTF_HXX
#include <gdimtf.hxx>
#endif
#ifndef _SV_METAACT_HXX
#include <metaact.hxx>
#endif

#define META_TEXTLINE_PDF_ACTION				META_TEXTLINE_ACTION
#define META_NEW_PAGE_PDF_ACTION				(10000)
#define META_PIXEL_PDF_ACTION					(10001)
#define META_JPG_PDF_ACTION						(10002)
#define META_ANTIALIAS_PDF_ACTION				(10003)
#define META_POLYLINE_PDF_ACTION				(10004)
#define META_CREATELINK_PDF_ACTION				(10005)
#define META_CREATEDEST_PDF_ACTION				(10006)
#define META_SETLINKDEST_PDF_ACTION				(10007)
#define META_SETLINKURL_PDF_ACTION				(10008)
#define META_SETLINKPROPERTYID_PDF_ACTION		(10009)
#define META_CREATEOUTLINEITEM_PDF_ACTION		(10010)
#define META_SETOUTLINEITEMPARENT_PDF_ACTION	(10011)
#define META_SETOUTLINEITEMTEXT_PDF_ACTION		(10012)
#define META_SETOUTLINEITEMDEST_PDF_ACTION		(10013)
#define META_CREATENOTE_PDF_ACTION				(10014)
#define META_BEGINSTRUCTUREELEMENT_PDF_ACTION	(10015)
#define META_ENDSTRUCTUREELEMENT_PDF_ACTION		(10016)
#define META_SETCURRENTSTRUCTUREELEMENT_PDF_ACTION	(10017)
#define META_SETSTRUCTUREATTRIBUTE_PDF_ACTION	(10018)
#define META_SETSTRUCTUREATTRIBUTENUMERICAL_PDF_ACTION	(10019)
#define META_SETSTRUCTUREBOUNDINGBOX_PDF_ACTION	(10020)
#define META_SETACTUALTEXT_PDF_ACTION			(10021)
#define META_SETALTERNATETEXT_PDF_ACTION		(10022)
#define META_SETAUTOADVANCETIME_PDF_ACTION		(10023)
#define META_SETPAGETRANSITION_PDF_ACTION		(10024)
#define META_CREATECONTROL_PDF_ACTION			(10025)
#define META_DIGITLANGUAGE_PDF_ACTION			(10026)
#define META_BEGINPATTERN_PDF_ACTION			(10027)
#define META_ENDPATTERN_PDF_ACTION				(10028)
#define META_POLYPOLYGON_PDF_ACTION				(10029)
#define META_BEGINTRANSPARENCYGROUP_PDF_ACTION	(10030)
#define META_ENDTRANSPARENCYGROUP_PDF_ACTION	(10031)
#define META_ENDTRANSPARENCYGROUPMASK_PDF_ACTION	(10032)
#define META_SETDOCINFO_PDF_ACTION				(10033)

class SAL_DLLPRIVATE MetaTextLinePDFAction : public MetaTextLineAction
{
private:
    bool                mbUnderlineAbove;

public:
    					MetaTextLinePDFAction( const Point& rPos, long nWidth, FontStrikeout eStrikeout, FontUnderline eUnderline, bool bUnderlineAbove ) : MetaTextLineAction( rPos, nWidth, eStrikeout, eUnderline ), mbUnderlineAbove( bUnderlineAbove ) {}
    virtual				~MetaTextLinePDFAction() {}

    bool				IsUnderlineAbove() const { return mbUnderlineAbove; }
};

class SAL_DLLPRIVATE MetaNewPagePDFAction : public MetaAction
{
private:
    sal_Int32			mnPageWidth;
    sal_Int32			mnPageHeight;
    ::vcl::PDFWriter::Orientation	meOrientation;

public:
    					MetaNewPagePDFAction( sal_Int32 nPageWidth, sal_Int32 nPageHeight, ::vcl::PDFWriter::Orientation eOrientation ) : MetaAction( META_NEW_PAGE_PDF_ACTION ), mnPageWidth( nPageWidth ), mnPageHeight( nPageHeight ), meOrientation( eOrientation ) {}
    virtual				~MetaNewPagePDFAction() {}

    sal_Int32			GetPageWidth() const { return mnPageWidth; }
    sal_Int32			GetPageHeight() const { return mnPageHeight; }
    ::vcl::PDFWriter::Orientation	GetOrientation() const { return meOrientation; }
};

class SAL_DLLPRIVATE MetaPixelPDFAction : public MetaAction
{
private:
    Polygon				maPoints;
    Color*				mpColors;

public:
    					MetaPixelPDFAction( const Polygon& rPoints, const Color* pColors ) : MetaAction( META_PIXEL_PDF_ACTION ), maPoints( rPoints ), mpColors( NULL ) { int nPoints = rPoints.GetSize(); if ( nPoints ) { mpColors = new Color[ nPoints ]; for ( int i = 0; i < nPoints; i++ ) mpColors[ i ] = pColors[ i ]; } }
    virtual				~MetaPixelPDFAction() { if ( mpColors ) delete mpColors; }

    const Polygon&		GetPoints() const { return maPoints; }
    const Color*		GetColors() const { return mpColors; }
};

class SAL_DLLPRIVATE MetaJpgPDFAction : public MetaAction
{
private:
    SvMemoryStream		maStream;
    bool				mbTrueColor;
    Size				maSize;
    Rectangle			maRect;
    Bitmap				maMask;

public:
    					MetaJpgPDFAction( SvStream& rStream, bool bTrueColor, const Size& rSize, const Rectangle& rRect, const Bitmap& rMask ) : MetaAction( META_JPG_PDF_ACTION ), mbTrueColor( bTrueColor ), maSize( rSize ), maRect( rRect ), maMask( rMask ) { rStream.Seek( 0 ); maStream << rStream; }
    virtual				~MetaJpgPDFAction() {}

    const SvStream&		GetStream() const { return maStream; }
    bool				IsTrueColor() const { return mbTrueColor; }
    const Size&			GetSize() const { return maSize; }
    const Rectangle&	GetRect() const { return maRect; }
    const Bitmap&		GetMask() const { return maMask; }
};

class SAL_DLLPRIVATE MetaAntiAliasPDFAction : public MetaAction
{
private:
    sal_Int32			mnAntiAlias;

public:
    					MetaAntiAliasPDFAction( sal_Int32 nAntiAlias ) : MetaAction( META_ANTIALIAS_PDF_ACTION ), mnAntiAlias( nAntiAlias ) {}
    virtual				~MetaAntiAliasPDFAction() {}

    sal_Int32			GetAntiAlias() const { return mnAntiAlias; }
};

class SAL_DLLPRIVATE MetaPolyLinePDFAction : public MetaAction
{
private:
    Polygon				maPoly;
    ::vcl::PDFWriter::ExtLineInfo	maInfo;

public:
    					MetaPolyLinePDFAction( const Polygon& rPoly, const ::vcl::PDFWriter::ExtLineInfo& rInfo ) : MetaAction( META_POLYLINE_PDF_ACTION ), maPoly( rPoly ), maInfo( rInfo ) {}
    virtual				~MetaPolyLinePDFAction() {}

    const Polygon&		GetPolygon() const { return maPoly; }
    const ::vcl::PDFWriter::ExtLineInfo&	GetExtLineInfo() const { return maInfo; }
};

class SAL_DLLPRIVATE MetaCreateLinkPDFAction : public MetaAction
{
private:
    Rectangle			maRect;
    sal_Int32			mnPage;

public:
    					MetaCreateLinkPDFAction( const Rectangle& rRect, sal_Int32 nPage ) : MetaAction( META_CREATELINK_PDF_ACTION ), maRect( rRect ), mnPage( nPage ) {}
    virtual				~MetaCreateLinkPDFAction() {}

    const Rectangle&	GetRect() const { return maRect; }
    sal_Int32			GetPage() const { return mnPage; }
};

class SAL_DLLPRIVATE MetaCreateDestPDFAction : public MetaAction
{
private:
    Rectangle			maRect;
    sal_Int32			mnPage;
    ::vcl::PDFWriter::DestAreaType	meType;

public:
    					MetaCreateDestPDFAction( const Rectangle& rRect, sal_Int32 nPage, ::vcl::PDFWriter::DestAreaType eType ) : MetaAction( META_CREATEDEST_PDF_ACTION ), maRect( rRect ), mnPage( nPage ), meType( eType ) {}
    virtual				~MetaCreateDestPDFAction() {}

    const Rectangle&	GetRect() const { return maRect; }
    sal_Int32			GetPage() const { return mnPage; }
    ::vcl::PDFWriter::DestAreaType	GetType() const { return meType; }
};

class SAL_DLLPRIVATE MetaSetLinkDestPDFAction : public MetaAction
{
private:
    sal_Int32			mnLink;
    sal_Int32			mnDest;

public:
    					MetaSetLinkDestPDFAction( sal_Int32 nLink, sal_Int32 nDest ) : MetaAction( META_SETLINKDEST_PDF_ACTION ), mnLink( nLink ), mnDest( nDest ) {}
    virtual				~MetaSetLinkDestPDFAction() {}

    sal_Int32			GetLink() const { return mnLink; }
    sal_Int32			GetDest() const { return mnDest; }
};

class SAL_DLLPRIVATE MetaSetLinkUrlPDFAction : public MetaAction
{
private:
    sal_Int32			mnLink;
    rtl::OUString		maURL;

public:
    					MetaSetLinkUrlPDFAction( sal_Int32 nLink, const rtl::OUString& rURL ) : MetaAction( META_SETLINKURL_PDF_ACTION ), mnLink( nLink ), maURL( rURL ) {}
    virtual				~MetaSetLinkUrlPDFAction() {}

    sal_Int32			GetLink() const { return mnLink; }
    const rtl::OUString&	GetURL() const { return maURL; }
};

class SAL_DLLPRIVATE MetaSetLinkPropertyIdPDFAction : public MetaAction
{
private:
    sal_Int32			mnLink;
    sal_Int32			mnProp;

public:
    					MetaSetLinkPropertyIdPDFAction( sal_Int32 nLink, sal_Int32 nProp ) : MetaAction( META_SETLINKPROPERTYID_PDF_ACTION ), mnLink( nLink ), mnProp( nProp ) {}
    virtual				~MetaSetLinkPropertyIdPDFAction() {}

    sal_Int32			GetLink() const { return mnLink; }
    sal_Int32			GetProperty() const { return mnProp; }
};

class SAL_DLLPRIVATE MetaCreateOutlineItemPDFAction : public MetaAction
{
private:
    sal_Int32			mnParent;
    rtl::OUString		maText;
    sal_Int32			mnDest;

public:
    					MetaCreateOutlineItemPDFAction( sal_Int32 nParent, const rtl::OUString& rText, sal_Int32 nDest ) : MetaAction( META_CREATEOUTLINEITEM_PDF_ACTION ), mnParent( nParent ), maText( rText ), mnDest( nDest ) {}
    virtual				~MetaCreateOutlineItemPDFAction() {}

    sal_Int32			GetParent() const { return mnParent; }
    const rtl::OUString&	GetText() const { return maText; }
    sal_Int32			GetDest() const { return mnDest; }
};

class SAL_DLLPRIVATE MetaSetOutlineItemParentPDFAction : public MetaAction
{
private:
    sal_Int32			mnItem;
    sal_Int32			mnParent;

public:
    					MetaSetOutlineItemParentPDFAction( sal_Int32 nItem, sal_Int32 nParent ) : MetaAction( META_SETOUTLINEITEMPARENT_PDF_ACTION ), mnItem( nItem ), mnParent( nParent ) {}
    virtual				~MetaSetOutlineItemParentPDFAction() {}

    sal_Int32			GetItem() const { return mnItem; }
    sal_Int32			GetParent() const { return mnParent; }
};

class SAL_DLLPRIVATE MetaSetOutlineItemTextPDFAction : public MetaAction
{
private:
    sal_Int32			mnItem;
    rtl::OUString		maText;

public:
    					MetaSetOutlineItemTextPDFAction( sal_Int32 nItem, const rtl::OUString& rText ) : MetaAction( META_SETOUTLINEITEMTEXT_PDF_ACTION ), mnItem( nItem ), maText( rText ) {}
    virtual				~MetaSetOutlineItemTextPDFAction() {}

    sal_Int32			GetItem() const { return mnItem; }
    const rtl::OUString&	GetText() const { return maText; }
};

class SAL_DLLPRIVATE MetaSetOutlineItemDestPDFAction : public MetaAction
{
private:
    sal_Int32			mnItem;
    sal_Int32			mnDest;

public:
    					MetaSetOutlineItemDestPDFAction( sal_Int32 nItem, sal_Int32 nDest ) : MetaAction( META_SETOUTLINEITEMDEST_PDF_ACTION ), mnItem( nItem ), mnDest( nDest ) {}
    virtual				~MetaSetOutlineItemDestPDFAction() {}

    sal_Int32			GetItem() const { return mnItem; }
    sal_Int32			GetDest() const { return mnDest; }
};

class SAL_DLLPRIVATE MetaCreateNotePDFAction : public MetaAction
{
private:
    Rectangle			maRect;
    ::vcl::PDFNote		maNote;
    sal_Int32			mnPage;

public:
    					MetaCreateNotePDFAction( const Rectangle& rRect, const ::vcl::PDFNote& rNote, sal_Int32 nPage ) : MetaAction( META_CREATENOTE_PDF_ACTION ), maRect( rRect ), maNote( rNote ), mnPage( nPage ) {}
    virtual				~MetaCreateNotePDFAction() {}

    const Rectangle&	GetRect() const { return maRect; }
    sal_Int32			GetPage() const { return mnPage; }
    const ::vcl::PDFNote&	GetNote() const { return maNote; }
};

class SAL_DLLPRIVATE MetaCreateControlPDFAction : public MetaAction
{
private:
    ::vcl::PDFWriter::AnyWidget*	mpControl;
    sal_Int32			mnPage;

public:
						MetaCreateControlPDFAction( const ::vcl::PDFWriter::AnyWidget& rControl, sal_Int32 nPage ) : MetaAction( META_CREATECONTROL_PDF_ACTION ), mnPage( nPage ) { mpControl = rControl.Clone(); }
    virtual				~MetaCreateControlPDFAction() { if ( mpControl ) delete mpControl; }

    const ::vcl::PDFWriter::AnyWidget&	GetControl() const { return *mpControl; }
    sal_Int32			GetPage() const { return mnPage; }
};

class SAL_DLLPRIVATE MetaBeginStructureElementPDFAction : public MetaAction
{
private:
    ::vcl::PDFWriter::StructElement meType;

public:
    					MetaBeginStructureElementPDFAction( ::vcl::PDFWriter::StructElement eType ) : MetaAction( META_BEGINSTRUCTUREELEMENT_PDF_ACTION ), meType( eType ) {}
    virtual				~MetaBeginStructureElementPDFAction() {}

    ::vcl::PDFWriter::StructElement	GetType() const { return meType; }
};

class SAL_DLLPRIVATE MetaEndStructureElementPDFAction : public MetaAction
{
public:
    					MetaEndStructureElementPDFAction() : MetaAction( META_ENDSTRUCTUREELEMENT_PDF_ACTION ) {}
    virtual				~MetaEndStructureElementPDFAction() {}
};

class SAL_DLLPRIVATE MetaSetCurrentStructureElementPDFAction : public MetaAction
{
private:
    sal_Int32			mnElement;

public:
    					MetaSetCurrentStructureElementPDFAction( sal_Int32 nElement ) : MetaAction( META_SETCURRENTSTRUCTUREELEMENT_PDF_ACTION ), mnElement( nElement ) {}
    virtual				~MetaSetCurrentStructureElementPDFAction() {}

    sal_Int32			GetElement() const { return mnElement; }
};

class SAL_DLLPRIVATE MetaSetStructureAttributePDFAction : public MetaAction
{
private:
    enum ::vcl::PDFWriter::StructAttribute	meAttr;
    enum ::vcl::PDFWriter::StructAttributeValue	meValue;

public:
    					MetaSetStructureAttributePDFAction( enum ::vcl::PDFWriter::StructAttribute eAttr, enum ::vcl::PDFWriter::StructAttributeValue eValue ) : MetaAction( META_SETSTRUCTUREATTRIBUTE_PDF_ACTION ), meAttr( eAttr ), meValue( eValue ) {}
    virtual				~MetaSetStructureAttributePDFAction() {}

    enum ::vcl::PDFWriter::StructAttribute	GetAttribute() const { return meAttr; }
    enum ::vcl::PDFWriter::StructAttributeValue	GetValue() const { return meValue; }
};

class SAL_DLLPRIVATE MetaSetStructureAttributeNumericalPDFAction : public MetaAction
{
private:
    enum ::vcl::PDFWriter::StructAttribute	meAttr;
    sal_Int32			mnValue;

public:
    					MetaSetStructureAttributeNumericalPDFAction( enum ::vcl::PDFWriter::StructAttribute eAttr, sal_Int32 nValue ) : MetaAction( META_SETSTRUCTUREATTRIBUTENUMERICAL_PDF_ACTION ), meAttr( eAttr ), mnValue( nValue ) {}
    virtual				~MetaSetStructureAttributeNumericalPDFAction() {}

    enum ::vcl::PDFWriter::StructAttribute	GetAttribute() const { return meAttr; }
    sal_Int32			GetValue() const { return mnValue; }
};

class SAL_DLLPRIVATE MetaSetStructureBoundingBoxPDFAction : public MetaAction
{
private:
    Rectangle			maRect;

public:
    					MetaSetStructureBoundingBoxPDFAction( const Rectangle& rRect ) : MetaAction( META_SETSTRUCTUREBOUNDINGBOX_PDF_ACTION ), maRect( rRect ) {}
    virtual				~MetaSetStructureBoundingBoxPDFAction() {}

    const Rectangle&	GetRect() const { return maRect; }
};

class SAL_DLLPRIVATE MetaSetActualTextPDFAction : public MetaAction
{
private:
    String				maText;

public:
    					MetaSetActualTextPDFAction( const String& rText ) : MetaAction( META_SETACTUALTEXT_PDF_ACTION ), maText( rText ) {}
    virtual				~MetaSetActualTextPDFAction() {}

    const String&		GetText() const { return maText; }
};

class SAL_DLLPRIVATE MetaSetAlternateTextPDFAction : public MetaAction
{
private:
    String				maText;

public:
    					MetaSetAlternateTextPDFAction( const String& rText ) : MetaAction( META_SETALTERNATETEXT_PDF_ACTION ), maText( rText ) {}
    virtual				~MetaSetAlternateTextPDFAction() {}

    const String&		GetText() const { return maText; }
};

class SAL_DLLPRIVATE MetaSetAutoAdvanceTimePDFAction : public MetaAction
{
private:
    sal_uInt32			mnSeconds;
    sal_Int32			mnPage;

public:
    					MetaSetAutoAdvanceTimePDFAction( sal_uInt32 nSeconds, sal_Int32 nPage ) : MetaAction( META_SETAUTOADVANCETIME_PDF_ACTION ), mnSeconds( nSeconds ), mnPage( nPage ) {}
    virtual				~MetaSetAutoAdvanceTimePDFAction() {}

    sal_uInt32			GetSeconds() const { return mnSeconds; }
    sal_Int32			GetPage() const { return mnPage; }
};

class SAL_DLLPRIVATE MetaSetPageTransitionPDFAction : public MetaAction
{
private:
	::vcl::PDFWriter::PageTransition	meType;
    sal_uInt32			mnMilliSeconds;
    sal_Int32			mnPage;

public:
    					MetaSetPageTransitionPDFAction( ::vcl::PDFWriter::PageTransition eType, sal_uInt32 nMilliSeconds, sal_Int32 nPage ) : MetaAction( META_SETPAGETRANSITION_PDF_ACTION ), meType( eType ), mnMilliSeconds( nMilliSeconds ), mnPage( nPage ) {}
    virtual				~MetaSetPageTransitionPDFAction() {}

    ::vcl::PDFWriter::PageTransition	GetType() const { return meType; }
    sal_uInt32			GetMilliSeconds() const { return mnMilliSeconds; }
    sal_Int32			GetPage() const { return mnPage; }
};

class SAL_DLLPRIVATE MetaDigitLanguagePDFAction : public MetaAction
{
private:
    LanguageType		meLang;

public:
    					MetaDigitLanguagePDFAction( LanguageType eLang ) : MetaAction( META_DIGITLANGUAGE_PDF_ACTION ), meLang( eLang ) {}
    virtual				~MetaDigitLanguagePDFAction() {}

    LanguageType		GetLanguage() const { return meLang; }
};

class SAL_DLLPRIVATE MetaBeginPatternPDFAction : public MetaAction
{
public:
    					MetaBeginPatternPDFAction() : MetaAction( META_BEGINPATTERN_PDF_ACTION ) {}
    virtual				~MetaBeginPatternPDFAction() {}
};

class SAL_DLLPRIVATE MetaEndPatternPDFAction : public MetaAction
{
private:
    Rectangle			maRect;
    SvtGraphicFill::Transform	maTransform;

public:
    					MetaEndPatternPDFAction( const Rectangle& rRect, const SvtGraphicFill::Transform& rTransform ) : MetaAction( META_ENDPATTERN_PDF_ACTION ), maRect( rRect ), maTransform( rTransform ) {}
    virtual				~MetaEndPatternPDFAction() {}

    const Rectangle&	GetRect() const { return maRect; }
    const SvtGraphicFill::Transform&	GetTransform() const { return maTransform; }
};

class SAL_DLLPRIVATE MetaPolyPolygonPDFAction : public MetaAction
{
private:
    PolyPolygon			maPolyPoly;
    sal_Int32			mnPattern;
    bool				mbEOFill;

public:
						MetaPolyPolygonPDFAction( const PolyPolygon& rPolyPoly, sal_Int32 nPattern, bool bEOFill ) : MetaAction( META_POLYPOLYGON_PDF_ACTION ), maPolyPoly( rPolyPoly ), mnPattern( nPattern ), mbEOFill( bEOFill ) {}
    virtual				~MetaPolyPolygonPDFAction() {}

    const PolyPolygon&	GetPolyPolygon() const { return maPolyPoly; }
    sal_Int32			GetPattern() const { return mnPattern; }
    bool				IsEOFill() const { return mbEOFill; }
};

class SAL_DLLPRIVATE MetaBeginTransparencyGroupPDFAction : public MetaAction
{
public:
    					MetaBeginTransparencyGroupPDFAction() : MetaAction( META_BEGINTRANSPARENCYGROUP_PDF_ACTION ) {}
    virtual				~MetaBeginTransparencyGroupPDFAction() {}
};

class SAL_DLLPRIVATE MetaEndTransparencyGroupPDFAction : public MetaAction
{
private:
    Rectangle			maBoundingRect;
    sal_uInt32			mnTransparentPercent;

public:
    					MetaEndTransparencyGroupPDFAction( const Rectangle& rBoundingRect, sal_uInt32 nTransparentPercent ) : MetaAction( META_ENDTRANSPARENCYGROUP_PDF_ACTION ), maBoundingRect( rBoundingRect ), mnTransparentPercent( nTransparentPercent ) {}
    virtual				~MetaEndTransparencyGroupPDFAction() {}

    const Rectangle&	GetBoundingRect() const { return maBoundingRect; }
    sal_uInt32			GetTransparentPercent() const { return mnTransparentPercent; }
};

class SAL_DLLPRIVATE MetaEndTransparencyGroupMaskPDFAction : public MetaAction
{
private:
    Rectangle			maBoundingRect;
    Bitmap				maAlphaMask;

public:
    					MetaEndTransparencyGroupMaskPDFAction( const Rectangle& rBoundingRect, const Bitmap& rAlphaMask ) : MetaAction( META_ENDTRANSPARENCYGROUPMASK_PDF_ACTION ), maBoundingRect( rBoundingRect ), maAlphaMask( rAlphaMask ) {}
    virtual				~MetaEndTransparencyGroupMaskPDFAction() {}

    const Rectangle&	GetBoundingRect() const { return maBoundingRect; }
    const Bitmap&		GetAlphaMask() const { return maAlphaMask; }
};

class SAL_DLLPRIVATE MetaSetDocInfoPDFAction : public MetaAction
{
private:
    ::vcl::PDFDocInfo	maDocInfo;

public:
    					MetaSetDocInfoPDFAction( const ::vcl::PDFDocInfo& rDocInfo ) : MetaAction( META_SETDOCINFO_PDF_ACTION ), maDocInfo( rDocInfo ) {}
    virtual				~MetaSetDocInfoPDFAction() {}

    const ::vcl::PDFDocInfo&	GetDocInfo() const { return maDocInfo; }
};

#endif	// USE_JAVA

using namespace vcl;

#ifdef USE_JAVA

static void ReplayMetaFile( PDFWriter &aWriter, GDIMetaFile& rMtf )
{
    for ( ULONG i = 0, nCount = rMtf.GetActionCount(); i < nCount; i++ )
    {
        const MetaAction *pAction = rMtf.GetAction( i );
        const USHORT nType = pAction->GetType();

        switch( nType )
        {
            case( META_NEW_PAGE_PDF_ACTION ):
            {
                const MetaNewPagePDFAction* pA = (const MetaNewPagePDFAction*) pAction;
                aWriter.NewPage( pA->GetPageWidth(), pA->GetPageHeight(), pA->GetOrientation() );
            }
            break;

            case( META_FONT_ACTION ):
            {
                const MetaFontAction* pA = (const MetaFontAction*) pAction;
                aWriter.SetFont( pA->GetFont() );
            }
            break;

            case( META_TEXT_ACTION ):
            {
                const MetaTextAction* pA = (const MetaTextAction*) pAction;
                aWriter.DrawText( pA->GetPoint(), pA->GetText() );
            }
            break;

            case( META_TEXTLINE_PDF_ACTION ):
            {
                const MetaTextLinePDFAction* pA = (const MetaTextLinePDFAction*) pAction;
                aWriter.DrawTextLine( pA->GetStartPoint(), pA->GetWidth(), pA->GetStrikeout(), pA->GetUnderline(), pA->IsUnderlineAbove() );
            }
            break;

            case( META_TEXTARRAY_ACTION ):
            {
                const MetaTextArrayAction* pA = (const MetaTextArrayAction*) pAction;
                aWriter.DrawTextArray( pA->GetPoint(), pA->GetText(), pA->GetDXArray(), pA->GetIndex(), pA->GetLen() );
            }
            break;

            case( META_STRETCHTEXT_ACTION ):
            {
                const MetaStretchTextAction* pA = (const MetaStretchTextAction*) pAction;
                aWriter.DrawStretchText( pA->GetPoint(), pA->GetWidth(), pA->GetText(), pA->GetIndex(), pA->GetLen() );
            }
            break;

            case( META_TEXTRECT_ACTION ):
            {
                const MetaTextRectAction* pA = (const MetaTextRectAction*) pAction;
                aWriter.DrawText( pA->GetRect(), pA->GetText(), pA->GetStyle() );
            }
            break;

            case( META_LINE_ACTION ):
            {
                const MetaLineAction* pA = (const MetaLineAction*) pAction;
                aWriter.DrawLine( pA->GetStartPoint(), pA->GetEndPoint(), pA->GetLineInfo() );
            }
            break;

            case( META_POLYGON_ACTION ):
            {
                const MetaPolygonAction* pA = (const MetaPolygonAction*) pAction;
                aWriter.DrawPolygon( pA->GetPolygon() );
            }
            break;

            case( META_POLYLINE_ACTION ):
            {
                const MetaPolyLineAction* pA = (const MetaPolyLineAction*) pAction;
                aWriter.DrawPolyLine( pA->GetPolygon(), pA->GetLineInfo() );
            }
            break;

            case( META_POLYLINE_PDF_ACTION ):
            {
                const MetaPolyLinePDFAction* pA = (const MetaPolyLinePDFAction*) pAction;
                aWriter.DrawPolyLine( pA->GetPolygon(), pA->GetExtLineInfo() );
            }
            break;

            case( META_RECT_ACTION ):
            {
                const MetaRectAction* pA = (const MetaRectAction*) pAction;
                aWriter.DrawRect( pA->GetRect() );
            }
            break;

            case( META_ROUNDRECT_ACTION ):
            {
                const MetaRoundRectAction* pA = (const MetaRoundRectAction*) pAction;
                aWriter.DrawRect( pA->GetRect(), pA->GetHorzRound(), pA->GetVertRound() );
            }
            break;

            case( META_ELLIPSE_ACTION ):
            {
                const MetaEllipseAction* pA = (const MetaEllipseAction*) pAction;
                aWriter.DrawEllipse( pA->GetRect() );
            }
            break;

            case( META_PIE_ACTION ):
            {
                const MetaArcAction* pA = (const MetaArcAction*) pAction;
                aWriter.DrawArc( pA->GetRect(), pA->GetStartPoint(), pA->GetEndPoint() );
            }
            break;

            case( META_CHORD_ACTION ):
            {
                const MetaChordAction* pA = (const MetaChordAction*) pAction;
                aWriter.DrawArc( pA->GetRect(), pA->GetStartPoint(), pA->GetEndPoint() );
            }
            break;

            case( META_ARC_ACTION ):
            {
                const MetaArcAction* pA = (const MetaArcAction*) pAction;
                aWriter.DrawArc( pA->GetRect(), pA->GetStartPoint(), pA->GetEndPoint() );
            }
            break;

            case( META_POLYPOLYGON_ACTION ):
            {
                const MetaPolyPolygonAction* pA = (const MetaPolyPolygonAction*) pAction;
                aWriter.DrawPolyPolygon( pA->GetPolyPolygon() );
            }
            break;

            case( META_PIXEL_ACTION ):
            {
                const MetaPixelAction* pA = (const MetaPixelAction*) pAction;
                aWriter.DrawPixel( pA->GetPoint(), pA->GetColor() );
            }
            break;

            case( META_PIXEL_PDF_ACTION ):
            {
                const MetaPixelPDFAction* pA = (const MetaPixelPDFAction*) pAction;
                aWriter.DrawPixel( pA->GetPoints(), pA->GetColors() );
            }
            break;

            case( META_BMP_ACTION ):
            {
                const MetaBmpAction* pA = (const MetaBmpAction*) pAction;
                aWriter.DrawBitmap( pA->GetPoint(), pA->GetBitmap() );
            }
            break;

            case( META_BMPSCALE_ACTION ):
            {
                const MetaBmpScaleAction* pA = (const MetaBmpScaleAction*) pAction;
                aWriter.DrawBitmap( pA->GetPoint(), pA->GetSize(), pA->GetBitmap() );
            }
            break;

            case( META_BMPEX_ACTION ):
            {
                const MetaBmpExAction* pA = (const MetaBmpExAction*) pAction;
                aWriter.DrawBitmapEx( pA->GetPoint(), pA->GetBitmapEx() );
            }
            break;

            case( META_BMPEXSCALE_ACTION ):
            {
                const MetaBmpExScaleAction* pA = (const MetaBmpExScaleAction*) pAction;
                aWriter.DrawBitmapEx( pA->GetPoint(), pA->GetSize(), pA->GetBitmapEx() );
            }
            break;

            case( META_MASK_ACTION ):
            {
                const MetaMaskScaleAction* pA = (const MetaMaskScaleAction*) pAction;
                aWriter.DrawMask( pA->GetPoint(), pA->GetBitmap(), pA->GetColor() );
            }
            break;

            case( META_MASKSCALE_ACTION ):
            {
                const MetaMaskScaleAction* pA = (const MetaMaskScaleAction*) pAction;
                aWriter.DrawMask( pA->GetPoint(), pA->GetSize(), pA->GetBitmap(), pA->GetColor() );
            }
            break;

            case( META_GRADIENT_ACTION ):
            {
                const MetaGradientAction* pA = (const MetaGradientAction*) pAction;
                aWriter.DrawGradient( pA->GetRect(), pA->GetGradient() );
            }
            break;

            case( META_GRADIENTEX_ACTION ):
            {
                const MetaGradientExAction* pA = (const MetaGradientExAction*) pAction;
                aWriter.DrawGradient( pA->GetPolyPolygon(), pA->GetGradient() );
            }
            break;

            case META_HATCH_ACTION:
            {
                const MetaHatchAction* pA = (const MetaHatchAction*) pAction;
                aWriter.DrawHatch( pA->GetPolyPolygon(), pA->GetHatch() );
            }
            break;

            case( META_WALLPAPER_ACTION ):
            {
                const MetaWallpaperAction* pA = (const MetaWallpaperAction*) pAction;
                aWriter.DrawWallpaper( pA->GetRect(), pA->GetWallpaper() );
            }
            break;

            case( META_TRANSPARENT_ACTION ):
            {
                const MetaTransparentAction* pA = (const MetaTransparentAction*) pAction;
                aWriter.DrawTransparent( pA->GetPolyPolygon(), pA->GetTransparence() );
            }
            break;

            case( META_PUSH_ACTION ):
            {
                const MetaPushAction* pA = (const MetaPushAction*) pAction;

                aWriter.Push( pA->GetFlags() );
            }
            break;

            case( META_POP_ACTION ):
            {
                aWriter.Pop();
            }
            break;

            case( META_MAPMODE_ACTION ):
            {
                const MetaMapModeAction* pA = (const MetaMapModeAction*) pAction;
                aWriter.SetMapMode( pA->GetMapMode() );
            }
            break;

            case( META_LINECOLOR_ACTION ):
            {
                const MetaLineColorAction* pA = (const MetaLineColorAction*) pAction;
                aWriter.SetLineColor( pA->GetColor() );
            }
            break;

            case( META_FILLCOLOR_ACTION ):
            {
                const MetaFillColorAction* pA = (const MetaFillColorAction*) pAction;
                aWriter.SetFillColor( pA->GetColor() );
            }
            break;

            case( META_CLIPREGION_ACTION ):
            {
                const MetaClipRegionAction* pA = (const MetaClipRegionAction*) pAction;
                if( pA->IsClipping() )
                    aWriter.SetClipRegion( pA->GetRegion() );
                else
                    aWriter.SetClipRegion();
            }
            break;

            case( META_MOVECLIPREGION_ACTION ):
            {
                const MetaMoveClipRegionAction* pA = (const MetaMoveClipRegionAction*) pAction;
                aWriter.MoveClipRegion( pA->GetHorzMove(), pA->GetVertMove() );
            }
            break;

            case( META_ISECTRECTCLIPREGION_ACTION ):
            {
                const MetaISectRectClipRegionAction* pA = (const MetaISectRectClipRegionAction*) pAction;
                aWriter.IntersectClipRegion( pA->GetRect() );
            }
            break;

            case( META_ISECTREGIONCLIPREGION_ACTION ):
            {
               const MetaISectRegionClipRegionAction* pA = (const MetaISectRegionClipRegionAction*) pAction;
               aWriter.IntersectClipRegion( pA->GetRegion() );
            }
            break;

            case( META_ANTIALIAS_PDF_ACTION ):
            {
                const MetaAntiAliasPDFAction* pA = (const MetaAntiAliasPDFAction*) pAction;
                aWriter.SetAntialiasing( pA->GetAntiAlias() );
            }
            break;

            case( META_LAYOUTMODE_ACTION ):
            {
                const MetaLayoutModeAction* pA = (const MetaLayoutModeAction*) pAction;
                aWriter.SetLayoutMode( pA->GetLayoutMode() );
            }
            break;

            case( META_TEXTCOLOR_ACTION ):
            {
                const MetaTextColorAction* pA = (const MetaTextColorAction*) pAction;
                aWriter.SetTextColor( pA->GetColor() );
            }
            break;

            case( META_TEXTFILLCOLOR_ACTION ):
            {
                const MetaTextFillColorAction* pA = (const MetaTextFillColorAction*) pAction;
                if ( pA->IsSetting() )
                    aWriter.SetTextFillColor( pA->GetColor() );
                else
                    aWriter.SetTextFillColor();
            }
            break;

            case( META_TEXTLINECOLOR_ACTION ):
            {
                const MetaTextLineColorAction* pA = (const MetaTextLineColorAction*) pAction;
                if ( pA->IsSetting() )
                    aWriter.SetTextLineColor( pA->GetColor() );
                else
                    aWriter.SetTextLineColor();
            }
            break;

            case( META_TEXTALIGN_ACTION ):
            {
                const MetaTextAlignAction* pA = (const MetaTextAlignAction*) pAction;
                aWriter.SetTextAlign( pA->GetTextAlign() );
            }
            break;

            case( META_JPG_PDF_ACTION ):
            {
                const MetaJpgPDFAction* pA = (const MetaJpgPDFAction*) pAction;
                aWriter.DrawJPGBitmap( (SvStream&)pA->GetStream(), pA->IsTrueColor(), pA->GetSize(), pA->GetRect(), pA->GetMask() );
            }
            break;

            case( META_CREATELINK_PDF_ACTION ):
            {
                const MetaCreateLinkPDFAction* pA = (const MetaCreateLinkPDFAction*) pAction;
                aWriter.CreateLink( pA->GetRect(), pA->GetPage() );
            }
            break;

            case( META_CREATEDEST_PDF_ACTION ):
            {
                const MetaCreateDestPDFAction* pA = (const MetaCreateDestPDFAction*) pAction;
                aWriter.CreateDest( pA->GetRect(), pA->GetPage(), pA->GetType() );
            }
            break;

            case( META_SETLINKDEST_PDF_ACTION ):
            {
                const MetaSetLinkDestPDFAction* pA = (const MetaSetLinkDestPDFAction*) pAction;
                aWriter.SetLinkDest( pA->GetLink(), pA->GetDest() );
            }
            break;

            case( META_SETLINKURL_PDF_ACTION ):
            {
                const MetaSetLinkUrlPDFAction* pA = (const MetaSetLinkUrlPDFAction*) pAction;
                aWriter.SetLinkURL( pA->GetLink(), pA->GetURL() );
            }
            break;

            case( META_SETLINKPROPERTYID_PDF_ACTION ):
            {
                const MetaSetLinkPropertyIdPDFAction* pA = (const MetaSetLinkPropertyIdPDFAction*) pAction;
                aWriter.SetLinkPropertyID( pA->GetLink(), pA->GetProperty() );
            }
            break;

            case( META_CREATEOUTLINEITEM_PDF_ACTION ):
            {
                const MetaCreateOutlineItemPDFAction* pA = (const MetaCreateOutlineItemPDFAction*) pAction;
                aWriter.CreateOutlineItem( pA->GetParent(), pA->GetText(), pA->GetDest() );
            }
            break;

            case( META_SETOUTLINEITEMPARENT_PDF_ACTION ):
            {
                const MetaSetOutlineItemParentPDFAction* pA = (const MetaSetOutlineItemParentPDFAction*) pAction;
                aWriter.SetOutlineItemParent( pA->GetItem(), pA->GetParent() );
            }
            break;

            case( META_SETOUTLINEITEMTEXT_PDF_ACTION ):
            {
                const MetaSetOutlineItemTextPDFAction* pA = (const MetaSetOutlineItemTextPDFAction*) pAction;
                aWriter.SetOutlineItemText( pA->GetItem(), pA->GetText() );
            }
            break;

            case( META_SETOUTLINEITEMDEST_PDF_ACTION ):
            {
                const MetaSetOutlineItemDestPDFAction* pA = (const MetaSetOutlineItemDestPDFAction*) pAction;
                aWriter.SetOutlineItemDest( pA->GetItem(), pA->GetDest() );
            }
            break;

            case( META_CREATENOTE_PDF_ACTION ):
            {
                const MetaCreateNotePDFAction* pA = (const MetaCreateNotePDFAction*) pAction;
                aWriter.CreateNote( pA->GetRect(), pA->GetNote(), pA->GetPage() );
            }
            break;

            case( META_BEGINSTRUCTUREELEMENT_PDF_ACTION ):
            {
                const MetaBeginStructureElementPDFAction* pA = (const MetaBeginStructureElementPDFAction*) pAction;
                aWriter.BeginStructureElement( pA->GetType() );
            }
            break;

            case( META_ENDSTRUCTUREELEMENT_PDF_ACTION ):
            {
                aWriter.EndStructureElement();
            }
            break;

            case( META_SETCURRENTSTRUCTUREELEMENT_PDF_ACTION ):
            {
                const MetaSetCurrentStructureElementPDFAction* pA = (const MetaSetCurrentStructureElementPDFAction*) pAction;
                aWriter.SetCurrentStructureElement( pA->GetElement() );
            }
            break;

            case( META_SETSTRUCTUREATTRIBUTE_PDF_ACTION ):
            {
                const MetaSetStructureAttributePDFAction* pA = (const MetaSetStructureAttributePDFAction*) pAction;
                aWriter.SetStructureAttribute( pA->GetAttribute(), pA->GetValue() );
            }
            break;

            case( META_SETSTRUCTUREATTRIBUTENUMERICAL_PDF_ACTION ):
            {
                const MetaSetStructureAttributeNumericalPDFAction* pA = (const MetaSetStructureAttributeNumericalPDFAction*) pAction;
                aWriter.SetStructureAttributeNumerical( pA->GetAttribute(), pA->GetValue() );
            }
            break;

            case( META_SETSTRUCTUREBOUNDINGBOX_PDF_ACTION ):
            {
                const MetaSetStructureBoundingBoxPDFAction* pA = (const MetaSetStructureBoundingBoxPDFAction*) pAction;
                aWriter.SetStructureBoundingBox( pA->GetRect() );
            }
            break;

            case( META_SETACTUALTEXT_PDF_ACTION ):
            {
                const MetaSetActualTextPDFAction* pA = (const MetaSetActualTextPDFAction*) pAction;
                aWriter.SetActualText( pA->GetText() );
            }
            break;

            case( META_SETALTERNATETEXT_PDF_ACTION ):
            {
                const MetaSetAlternateTextPDFAction* pA = (const MetaSetAlternateTextPDFAction*) pAction;
                aWriter.SetAlternateText( pA->GetText() );
            }
            break;

            case( META_SETAUTOADVANCETIME_PDF_ACTION ):
            {
                const MetaSetAutoAdvanceTimePDFAction* pA = (const MetaSetAutoAdvanceTimePDFAction*) pAction;
                aWriter.SetAutoAdvanceTime( pA->GetSeconds(), pA->GetPage() );
            }
            break;

            case( META_SETPAGETRANSITION_PDF_ACTION ):
            {
                const MetaSetPageTransitionPDFAction* pA = (const MetaSetPageTransitionPDFAction*) pAction;
                aWriter.SetPageTransition( pA->GetType(), pA->GetMilliSeconds(), pA->GetPage() );
            }
            break;

            case( META_CREATECONTROL_PDF_ACTION ):
            {
                const MetaCreateControlPDFAction* pA = (const MetaCreateControlPDFAction*) pAction;
                aWriter.CreateControl( pA->GetControl(), pA->GetPage() );
            }
            break;

            case( META_DIGITLANGUAGE_PDF_ACTION ):
            {
                const MetaDigitLanguagePDFAction* pA = (const MetaDigitLanguagePDFAction*) pAction;
                aWriter.SetDigitLanguage( pA->GetLanguage() );
            }
            break;

            case( META_BEGINPATTERN_PDF_ACTION ):
            {
                aWriter.BeginPattern();
            }
            break;

            case( META_ENDPATTERN_PDF_ACTION ):
            {
                const MetaEndPatternPDFAction* pA = (const MetaEndPatternPDFAction*) pAction;
                aWriter.EndPattern( pA->GetRect(), pA->GetTransform() );
            }
            break;

            case( META_POLYPOLYGON_PDF_ACTION ):
            {
                const MetaPolyPolygonPDFAction* pA = (const MetaPolyPolygonPDFAction*) pAction;
                aWriter.DrawPolyPolygon( pA->GetPolyPolygon(), pA->GetPattern(), pA->IsEOFill() );
            }
            break;

            case( META_BEGINTRANSPARENCYGROUP_PDF_ACTION ):
            {
                aWriter.BeginTransparencyGroup();
            }
            break;

            case( META_ENDTRANSPARENCYGROUP_PDF_ACTION ):
            {
                const MetaEndTransparencyGroupPDFAction* pA = (const MetaEndTransparencyGroupPDFAction*) pAction;
                aWriter.EndTransparencyGroup( pA->GetBoundingRect(), pA->GetTransparentPercent() );
            }
            break;

            case( META_ENDTRANSPARENCYGROUPMASK_PDF_ACTION ):
            {
                const MetaEndTransparencyGroupMaskPDFAction* pA = (const MetaEndTransparencyGroupMaskPDFAction*) pAction;
                aWriter.EndTransparencyGroup( pA->GetBoundingRect(), pA->GetAlphaMask() );
            }
            break;

            case( META_SETDOCINFO_PDF_ACTION ):
            {
                const MetaSetDocInfoPDFAction* pA = (const MetaSetDocInfoPDFAction*) pAction;
                aWriter.SetDocInfo( pA->GetDocInfo() );
            }
            break;

            default:
                DBG_ERROR( "PDFWriterImpl::emit: unsupported MetaAction #" );
            break;
        }
    }
}

#endif	// USE_JAVA

PDFWriter::AnyWidget::~AnyWidget()
{
}

PDFWriter::PDFWriter( const PDFWriter::PDFWriterContext& rContext )
        :
        pImplementation( new PDFWriterImpl( rContext ) )
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
    if ( !((PDFWriterImpl*)pImplementation)->isReplayWriter() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaNewPagePDFAction( nPageWidth, nPageHeight, eOrientation ) );
#endif	// USE_JAVA
    return ((PDFWriterImpl*)pImplementation)->newPage( nPageWidth, nPageHeight, eOrientation );
}

bool PDFWriter::Emit()
{
#ifdef USE_JAVA
    bool bRet = false;

    // Replay meta actions
    if ( !((PDFWriterImpl*)pImplementation)->isReplayWriter() && ((PDFWriterImpl*)pImplementation)->emit() )
    {
        GDIMetaFile aMtf( ((PDFWriterImpl*)pImplementation)->getReplayMetaFile() );
        PDFWriterContext aContext( ((PDFWriterImpl*)pImplementation)->getContext() );
        void *pOldImplementation = pImplementation;

        // Fix bug 3061 by making a substitute writer and copying the actions
        // into that as the current writer seems to get mangled layouts in some
        // cases
        PDFWriterImpl aSubstituteWriter( aContext );
        pImplementation = &aSubstituteWriter;
        ReplayMetaFile( *this, aMtf );
        bRet = aSubstituteWriter.emit();
        pImplementation = pOldImplementation;

        // Now replay the same meta file into the final destination
        if ( bRet )
        {
            PDFWriterImpl aFinalWriter( aContext, &aSubstituteWriter );
            pImplementation = &aFinalWriter;
            ReplayMetaFile( *this, aMtf );
            bRet = aFinalWriter.emit();
            pImplementation = pOldImplementation;
        }
    }

    return bRet;
#else	// USE_JAVA
    return ((PDFWriterImpl*)pImplementation)->emit();
#endif	// USE_JAVA
}

PDFWriter::PDFVersion PDFWriter::GetVersion() const
{
    return ((PDFWriterImpl*)pImplementation)->getVersion();
}

void PDFWriter::SetDocInfo( const PDFDocInfo& rInfo )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isReplayWriter() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaSetDocInfoPDFAction( rInfo ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->setDocInfo( rInfo );
}

const PDFDocInfo& PDFWriter::GetDocInfo() const
{
    return ((PDFWriterImpl*)pImplementation)->getDocInfo();
}

void PDFWriter::SetFont( const Font& rFont )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isReplayWriter() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaFontAction( rFont ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->setFont( rFont );
}

void PDFWriter::DrawText( const Point& rPos, const String& rText )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isReplayWriter() )
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
    if ( !((PDFWriterImpl*)pImplementation)->isReplayWriter() )
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
    if ( !((PDFWriterImpl*)pImplementation)->isReplayWriter() )
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
    if ( !((PDFWriterImpl*)pImplementation)->isReplayWriter() )
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
    if ( !((PDFWriterImpl*)pImplementation)->isReplayWriter() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaTextRectAction( rRect, rStr, nStyle ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->drawText( rRect, rStr, nStyle );
}

void PDFWriter::DrawLine( const Point& rStart, const Point& rStop )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isReplayWriter() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaLineAction( rStart, rStop ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->drawLine( rStart, rStop );
}

void PDFWriter::DrawLine( const Point& rStart, const Point& rStop, const LineInfo& rInfo )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isReplayWriter() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaLineAction( rStart, rStop, rInfo ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->drawLine( rStart, rStop, rInfo );
}

void PDFWriter::DrawPolygon( const Polygon& rPoly )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isReplayWriter() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaPolygonAction( rPoly ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->drawPolygon( rPoly );
}

void PDFWriter::DrawPolyLine( const Polygon& rPoly )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isReplayWriter() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaPolyLineAction( rPoly ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->drawPolyLine( rPoly );
}

void PDFWriter::DrawRect( const Rectangle& rRect )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isReplayWriter() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaRectAction( rRect ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->drawRectangle( rRect );
}

void PDFWriter::DrawRect( const Rectangle& rRect, ULONG nHorzRound, ULONG nVertRound )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isReplayWriter() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaRoundRectAction( rRect, nHorzRound, nVertRound ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->drawRectangle( rRect, nHorzRound, nVertRound );
}

void PDFWriter::DrawEllipse( const Rectangle& rRect )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isReplayWriter() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaEllipseAction( rRect ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->drawEllipse( rRect );
}

void PDFWriter::DrawArc( const Rectangle& rRect, const Point& rStart, const Point& rStop )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isReplayWriter() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaArcAction( rRect, rStart, rStop ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->drawArc( rRect, rStart, rStop, false, false );
}

void PDFWriter::DrawPie( const Rectangle& rRect, const Point& rStart, const Point& rStop )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isReplayWriter() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaPieAction( rRect, rStart, rStop ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->drawArc( rRect, rStart, rStop, true, false );
}

void PDFWriter::DrawChord( const Rectangle& rRect, const Point& rStart, const Point& rStop )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isReplayWriter() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaChordAction( rRect, rStart, rStop ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->drawArc( rRect, rStart, rStop, false, true );
}

void PDFWriter::DrawPolyLine( const Polygon& rPoly, const LineInfo& rInfo )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isReplayWriter() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaPolyLineAction( rPoly, rInfo ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->drawPolyLine( rPoly, rInfo );
}

void PDFWriter::DrawPolyLine( const Polygon& rPoly, const ExtLineInfo& rInfo )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isReplayWriter() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaPolyLinePDFAction( rPoly, rInfo ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->drawPolyLine( rPoly, rInfo );
}

void PDFWriter::DrawPolyPolygon( const PolyPolygon& rPolyPoly )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isReplayWriter() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaPolyPolygonAction( rPolyPoly ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->drawPolyPolygon( rPolyPoly );
}

void PDFWriter::DrawPixel( const Point& rPos, const Color& rColor )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isReplayWriter() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaPixelAction( rPos, rColor ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->drawPixel( rPos, rColor );
}

void PDFWriter::DrawPixel( const Polygon& rPts, const Color* pColors )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isReplayWriter() )
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
    if ( !((PDFWriterImpl*)pImplementation)->isReplayWriter() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaBmpAction( rDestPt, rBitmap ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->drawBitmap( rDestPt, aSize, rBitmap );
}

void PDFWriter::DrawBitmap( const Point& rDestPt, const Size& rDestSize, const Bitmap& rBitmap )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isReplayWriter() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaBmpScaleAction( rDestPt, rDestSize, rBitmap ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->drawBitmap( rDestPt, rDestSize, rBitmap );
}

void PDFWriter::DrawBitmap( const Point& rDestPt, const Size& rDestSize, const Point& rSrcPtPixel, const Size& rSrcSizePixel, const Bitmap& rBitmap )
{
    Bitmap aBitmap( rBitmap );
    aBitmap.Crop( Rectangle( rSrcPtPixel, rSrcSizePixel ) );
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isReplayWriter() )
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
    if ( !((PDFWriterImpl*)pImplementation)->isReplayWriter() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaBmpExAction( rDestPt, rBitmap ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->drawBitmap( rDestPt, aSize, rBitmap );
}

void PDFWriter::DrawBitmapEx( const Point& rDestPt, const Size& rDestSize, const BitmapEx& rBitmap )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isReplayWriter() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaBmpExScaleAction( rDestPt, rDestSize, rBitmap ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->drawBitmap( rDestPt, rDestSize, rBitmap );
}

void PDFWriter::DrawBitmapEx( const Point& rDestPt, const Size& rDestSize, const Point& rSrcPtPixel, const Size& rSrcSizePixel, const BitmapEx& rBitmap )
{
    BitmapEx aBitmap( rBitmap );
    aBitmap.Crop( Rectangle( rSrcPtPixel, rSrcSizePixel ) );
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isReplayWriter() )
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
    if ( !((PDFWriterImpl*)pImplementation)->isReplayWriter() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaMaskAction( rDestPt, rBitmap, rMaskColor ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->drawMask( rDestPt, aSize, rBitmap, rMaskColor );
}

void PDFWriter::DrawMask( const Point& rDestPt, const Size& rDestSize, const Bitmap& rBitmap, const Color& rMaskColor )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isReplayWriter() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaMaskScaleAction( rDestPt, rDestSize, rBitmap, rMaskColor ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->drawMask( rDestPt, rDestSize, rBitmap, rMaskColor );
}

void PDFWriter::DrawMask( const Point& rDestPt, const Size& rDestSize, const Point& rSrcPtPixel, const Size& rSrcSizePixel, const Bitmap& rBitmap, const Color& rMaskColor )
{
    Bitmap aBitmap( rBitmap );
    aBitmap.Crop( Rectangle( rSrcPtPixel, rSrcSizePixel ) );
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isReplayWriter() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaMaskScaleAction( rDestPt, rDestSize, aBitmap, rMaskColor ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->drawMask( rDestPt, rDestSize, aBitmap, rMaskColor );
}

void PDFWriter::DrawGradient( const Rectangle& rRect, const Gradient& rGradient )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isReplayWriter() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaGradientAction( rRect, rGradient ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->drawGradient( rRect, rGradient );
}

void PDFWriter::DrawGradient( const PolyPolygon& rPolyPoly, const Gradient& rGradient )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isReplayWriter() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaGradientExAction( rPolyPoly, rGradient ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->drawGradient( rPolyPoly, rGradient );
}

void PDFWriter::DrawHatch( const PolyPolygon& rPolyPoly, const Hatch& rHatch )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isReplayWriter() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaHatchAction( rPolyPoly, rHatch ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->drawHatch( rPolyPoly, rHatch );
}

void PDFWriter::DrawWallpaper( const Rectangle& rRect, const Wallpaper& rWallpaper )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isReplayWriter() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaWallpaperAction( rRect, rWallpaper ) );
#endif	// USE_JAVA 
    ((PDFWriterImpl*)pImplementation)->drawWallpaper( rRect, rWallpaper );
}

void PDFWriter::DrawTransparent( const PolyPolygon& rPolyPoly, USHORT nTransparencePercent )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isReplayWriter() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaTransparentAction( rPolyPoly, nTransparencePercent ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->drawTransparent( rPolyPoly, nTransparencePercent );
}

void PDFWriter::BeginTransparencyGroup()
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isReplayWriter() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaBeginTransparencyGroupPDFAction() );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->beginTransparencyGroup();
}

void PDFWriter::EndTransparencyGroup( const Rectangle& rRect, USHORT nTransparentPercent )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isReplayWriter() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaEndTransparencyGroupPDFAction( rRect, nTransparentPercent ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->endTransparencyGroup( rRect, nTransparentPercent );
}

void PDFWriter::EndTransparencyGroup( const Rectangle& rRect, const Bitmap& rAlphaMask )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isReplayWriter() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaEndTransparencyGroupMaskPDFAction( rRect, rAlphaMask ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->endTransparencyGroup( rRect, rAlphaMask );
}

void PDFWriter::Push( USHORT nFlags )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isReplayWriter() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaPushAction( nFlags ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->push( nFlags );
}

void PDFWriter::Pop()
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isReplayWriter() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaPopAction() ); 
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->pop();
}

void PDFWriter::SetMapMode( const MapMode& rMapMode )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isReplayWriter() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaMapModeAction( rMapMode ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->setMapMode( rMapMode );
}

void PDFWriter::SetMapMode()
{
    if ( !((PDFWriterImpl*)pImplementation)->isReplayWriter() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaMapModeAction( ((PDFWriterImpl*)pImplementation)->getMapMode() ) );
    ((PDFWriterImpl*)pImplementation)->setMapMode();
}

void PDFWriter::SetLineColor( const Color& rColor )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isReplayWriter() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaLineColorAction( rColor, TRUE ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->setLineColor( rColor );
}

void PDFWriter::SetFillColor( const Color& rColor )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isReplayWriter() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaFillColorAction( rColor, TRUE ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->setFillColor( rColor );
}

void PDFWriter::SetClipRegion()
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isReplayWriter() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaClipRegionAction( Region(), FALSE ) );
#endif	// USE_JAVA 
    ((PDFWriterImpl*)pImplementation)->clearClipRegion();
}

void PDFWriter::SetClipRegion( const Region& rRegion )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isReplayWriter() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaClipRegionAction( rRegion, TRUE ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->setClipRegion( rRegion );
}

void PDFWriter::MoveClipRegion( long nHorzMove, long nVertMove )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isReplayWriter() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaMoveClipRegionAction( nHorzMove, nVertMove ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->moveClipRegion( nHorzMove, nVertMove );
}

void PDFWriter::IntersectClipRegion( const Region& rRegion )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isReplayWriter() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaISectRegionClipRegionAction( rRegion ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->intersectClipRegion( rRegion );
}

void PDFWriter::IntersectClipRegion( const Rectangle& rRect )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isReplayWriter() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaISectRectClipRegionAction( rRect ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->intersectClipRegion( rRect );
}

void PDFWriter::SetAntialiasing( USHORT nMode )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isReplayWriter() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaAntiAliasPDFAction( nMode ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->setAntiAlias( (sal_Int32)nMode );
}

void PDFWriter::SetLayoutMode( ULONG nMode )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isReplayWriter() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaLayoutModeAction( nMode ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->setLayoutMode( (sal_Int32)nMode );
}

void PDFWriter::SetDigitLanguage( LanguageType eLang )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isReplayWriter() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaDigitLanguagePDFAction( eLang ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->setDigitLanguage( eLang );
}

void PDFWriter::SetTextColor( const Color& rColor )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isReplayWriter() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaTextColorAction( rColor ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->setTextColor( rColor );
}

void PDFWriter::SetTextFillColor()
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isReplayWriter() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaTextFillColorAction( Color( COL_TRANSPARENT ), FALSE ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->setTextFillColor();
}

void PDFWriter::SetTextFillColor( const Color& rColor )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isReplayWriter() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaTextFillColorAction( rColor, TRUE ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->setTextFillColor( rColor );
}

void PDFWriter::SetTextLineColor()
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isReplayWriter() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaTextLineColorAction( Color( COL_TRANSPARENT ), FALSE ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->setTextLineColor();
}

void PDFWriter::SetTextLineColor( const Color& rColor )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isReplayWriter() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaTextLineColorAction( rColor, TRUE ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->setTextLineColor( rColor );
}

void PDFWriter::SetTextAlign( ::TextAlign eAlign )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isReplayWriter() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaTextAlignAction( eAlign ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->setTextAlign( eAlign );
}

void PDFWriter::DrawJPGBitmap( SvStream& rStreamData, bool bIsTrueColor, const Size& rSrcSizePixel, const Rectangle& rTargetArea, const Bitmap& rMask )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isReplayWriter() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaJpgPDFAction( rStreamData, bIsTrueColor, rSrcSizePixel, rTargetArea, rMask ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->drawJPGBitmap( rStreamData, bIsTrueColor, rSrcSizePixel, rTargetArea, rMask );
}

sal_Int32 PDFWriter::CreateLink( const Rectangle& rRect, sal_Int32 nPageNr )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isReplayWriter() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaCreateLinkPDFAction( rRect, nPageNr ) );
#endif	// USE_JAVA
    return ((PDFWriterImpl*)pImplementation)->createLink( rRect, nPageNr );
}

sal_Int32 PDFWriter::CreateDest( const Rectangle& rRect, sal_Int32 nPageNr, PDFWriter::DestAreaType eType )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isReplayWriter() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaCreateDestPDFAction( rRect, nPageNr, eType ) );
#endif	// USE_JAVA
    return ((PDFWriterImpl*)pImplementation)->createDest( rRect, nPageNr, eType );
}

sal_Int32 PDFWriter::SetLinkDest( sal_Int32 nLinkId, sal_Int32 nDestId )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isReplayWriter() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaSetLinkDestPDFAction( nLinkId, nDestId ) );
#endif	// USE_JAVA
    return ((PDFWriterImpl*)pImplementation)->setLinkDest( nLinkId, nDestId );
}

sal_Int32 PDFWriter::SetLinkURL( sal_Int32 nLinkId, const rtl::OUString& rURL )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isReplayWriter() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaSetLinkUrlPDFAction( nLinkId, rURL ) );
#endif	// USE_JAVA
    return ((PDFWriterImpl*)pImplementation)->setLinkURL( nLinkId, rURL );
}

void PDFWriter::SetLinkPropertyID( sal_Int32 nLinkId, sal_Int32 nPropertyId )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isReplayWriter() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaSetLinkPropertyIdPDFAction( nLinkId, nPropertyId ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->setLinkPropertyId( nLinkId, nPropertyId );
}

sal_Int32 PDFWriter::CreateOutlineItem( sal_Int32 nParent, const rtl::OUString& rText, sal_Int32 nDestID )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isReplayWriter() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaCreateOutlineItemPDFAction( nParent, rText, nDestID ) );
#endif	// USE_JAVA
    return ((PDFWriterImpl*)pImplementation)->createOutlineItem( nParent, rText, nDestID );
}

sal_Int32 PDFWriter::SetOutlineItemParent( sal_Int32 nItem, sal_Int32 nNewParent )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isReplayWriter() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaSetOutlineItemParentPDFAction( nItem, nNewParent ) ); 
#endif	// USE_JAVA
    return ((PDFWriterImpl*)pImplementation)->setOutlineItemParent( nItem, nNewParent );
}

sal_Int32 PDFWriter::SetOutlineItemText( sal_Int32 nItem, const rtl::OUString& rText )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isReplayWriter() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaSetOutlineItemTextPDFAction( nItem, rText ) );
#endif	// USE_JAVA
    return  ((PDFWriterImpl*)pImplementation)->setOutlineItemText( nItem, rText );
}

sal_Int32 PDFWriter::SetOutlineItemDest( sal_Int32 nItem, sal_Int32 nDest )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isReplayWriter() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaSetOutlineItemDestPDFAction( nItem, nDest ) );
#endif	// USE_JAVA
    return ((PDFWriterImpl*)pImplementation)->setOutlineItemDest( nItem, nDest );
}

void PDFWriter::CreateNote( const Rectangle& rRect, const PDFNote& rNote, sal_Int32 nPageNr )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isReplayWriter() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaCreateNotePDFAction( rRect, rNote, nPageNr ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->createNote( rRect, rNote, nPageNr );
}

sal_Int32 PDFWriter::BeginStructureElement( PDFWriter::StructElement eType )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isReplayWriter() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaBeginStructureElementPDFAction( eType ) );
#endif	// USE_JAVA
    return ((PDFWriterImpl*)pImplementation)->beginStructureElement( eType );
}

void PDFWriter::EndStructureElement()
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isReplayWriter() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaEndStructureElementPDFAction() );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->endStructureElement();
}

bool PDFWriter::SetCurrentStructureElement( sal_Int32 nID )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isReplayWriter() )
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
    if ( !((PDFWriterImpl*)pImplementation)->isReplayWriter() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaSetStructureAttributePDFAction( eAttr, eVal ) );          
#endif	// USE_JAVA 
    return ((PDFWriterImpl*)pImplementation)->setStructureAttribute( eAttr, eVal );
}

bool PDFWriter::SetStructureAttributeNumerical( enum StructAttribute eAttr, sal_Int32 nValue )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isReplayWriter() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaSetStructureAttributeNumericalPDFAction( eAttr, nValue ) );
#endif	// USE_JAVA
    return ((PDFWriterImpl*)pImplementation)->setStructureAttributeNumerical( eAttr, nValue );
}

void PDFWriter::SetStructureBoundingBox( const Rectangle& rRect )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isReplayWriter() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaSetStructureBoundingBoxPDFAction( rRect ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->setStructureBoundingBox( rRect );
}

void PDFWriter::SetActualText( const String& rText )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isReplayWriter() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaSetActualTextPDFAction( rText ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->setActualText( rText );
}

void PDFWriter::SetAlternateText( const String& rText )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isReplayWriter() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaSetAlternateTextPDFAction( rText ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->setAlternateText( rText );
}

void PDFWriter::SetAutoAdvanceTime( sal_uInt32 nSeconds, sal_Int32 nPageNr )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isReplayWriter() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaSetAutoAdvanceTimePDFAction( nSeconds, nPageNr ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->setAutoAdvanceTime( nSeconds, nPageNr );
}

void PDFWriter::SetPageTransition( PDFWriter::PageTransition eType, sal_uInt32 nMilliSec, sal_Int32 nPageNr )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isReplayWriter() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaSetPageTransitionPDFAction( eType, nMilliSec, nPageNr ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->setPageTransition( eType, nMilliSec, nPageNr );
}

sal_Int32 PDFWriter::CreateControl( const PDFWriter::AnyWidget& rControl, sal_Int32 nPageNr )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isReplayWriter() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaCreateControlPDFAction( rControl, nPageNr ) );
#endif	// USE_JAVA
    return ((PDFWriterImpl*)pImplementation)->createControl( rControl, nPageNr );
}

void PDFWriter::BeginPattern()
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isReplayWriter() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaBeginPatternPDFAction() );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->beginPattern();
}

sal_Int32 PDFWriter::EndPattern( const Rectangle& rCellBounds, const SvtGraphicFill::Transform& rTransform )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isReplayWriter() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaEndPatternPDFAction( rCellBounds, rTransform ) );
#endif	// USE_JAVA
    return ((PDFWriterImpl*)pImplementation)->endPattern( rCellBounds, rTransform );
}

void PDFWriter::DrawPolyPolygon( const PolyPolygon& rPolyPoly, sal_Int32 nPattern, bool bEOFill )
{
#ifdef USE_JAVA
    if ( !((PDFWriterImpl*)pImplementation)->isReplayWriter() )
        ((PDFWriterImpl*)pImplementation)->addAction( new MetaPolyPolygonPDFAction( rPolyPoly, nPattern, bEOFill ) );
#endif	// USE_JAVA
    ((PDFWriterImpl*)pImplementation)->drawPolyPolygon( rPolyPoly, nPattern, bEOFill );
}
