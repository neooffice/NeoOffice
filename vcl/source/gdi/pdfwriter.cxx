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
 *   Modified December 2016 by Patrick Luby. NeoOffice is only distributed
 *   under the GNU General Public License, Version 3 as allowed by Section 3.3
 *   of the Mozilla Public License, v. 2.0.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <pdfwriter_impl.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/image.hxx>

using namespace vcl;

#if defined USE_JAVA && defined MACOSX

#include <vcl/gdimtf.hxx>
#include <vcl/metaact.hxx>

class SAL_DLLPRIVATE MetaNewPagePDFAction : public MetaAction
{
private:
    sal_Int32           mnPageWidth;
    sal_Int32           mnPageHeight;
    ::vcl::PDFWriter::Orientation meOrientation;

public:
                        MetaNewPagePDFAction( sal_Int32 nPageWidth, sal_Int32 nPageHeight, ::vcl::PDFWriter::Orientation eOrientation ) : MetaAction( MetaActionType::NEWPAGEPDF ), mnPageWidth( nPageWidth ), mnPageHeight( nPageHeight ), meOrientation( eOrientation ) {}
    virtual             ~MetaNewPagePDFAction() {}

    sal_Int32           GetPageWidth() const { return mnPageWidth; }
    sal_Int32           GetPageHeight() const { return mnPageHeight; }
    ::vcl::PDFWriter::Orientation GetOrientation() const { return meOrientation; }
};

class SAL_DLLPRIVATE MetaJpgPDFAction : public MetaAction
{
private:
    SvMemoryStream      maStream;
    bool                mbTrueColor;
    Size                maSize;
    tools::Rectangle    maRect;
    Bitmap              maMask;
    Graphic             maGraphic;

public:
                        MetaJpgPDFAction( SvStream& rStream, bool bTrueColor, const Size& rSize, const tools::Rectangle& rRect, const Bitmap& rMask, const Graphic& rGraphic ) : MetaAction( MetaActionType::JPGPDF ), mbTrueColor( bTrueColor ), maSize( rSize ), maRect( rRect ), maMask( rMask ), maGraphic( rGraphic ) { rStream.Seek( 0 ); maStream.WriteStream( rStream ); }
    virtual             ~MetaJpgPDFAction() {}

    const SvStream&     GetStream() const { return maStream; }
    bool                IsTrueColor() const { return mbTrueColor; }
    const Size&         GetSize() const { return maSize; }
    const tools::Rectangle& GetRect() const { return maRect; }
    const Bitmap&       GetMask() const { return maMask; }
    const Graphic&      GetGraphic() const { return maGraphic; }
};

class SAL_DLLPRIVATE MetaPolyLinePDFAction : public MetaAction
{
private:
    tools::Polygon      maPoly;
    ::vcl::PDFWriter::ExtLineInfo maInfo;

public:
                        MetaPolyLinePDFAction( const tools::Polygon& rPoly, const ::vcl::PDFWriter::ExtLineInfo& rInfo ) : MetaAction( MetaActionType::POLYLINEPDF ), maPoly( rPoly ), maInfo( rInfo ) {}
    virtual             ~MetaPolyLinePDFAction() {}

    const tools::Polygon& GetPolygon() const { return maPoly; }
    const ::vcl::PDFWriter::ExtLineInfo& GetExtLineInfo() const { return maInfo; }
};

class SAL_DLLPRIVATE MetaCreateLinkPDFAction : public MetaAction
{
private:
    tools::Rectangle    maRect;
    sal_Int32           mnPage;

public:
                        MetaCreateLinkPDFAction( const tools::Rectangle& rRect, sal_Int32 nPage ) : MetaAction( MetaActionType::CREATELINKPDF ), maRect( rRect ), mnPage( nPage ) {}
    virtual             ~MetaCreateLinkPDFAction() {}

    const tools::Rectangle& GetRect() const { return maRect; }
    sal_Int32           GetPage() const { return mnPage; }
};

class SAL_DLLPRIVATE MetaCreateDestPDFAction : public MetaAction
{
private:
    tools::Rectangle    maRect;
    sal_Int32           mnPage;
    ::vcl::PDFWriter::DestAreaType meType;

public:
                        MetaCreateDestPDFAction( const tools::Rectangle& rRect, sal_Int32 nPage, ::vcl::PDFWriter::DestAreaType eType ) : MetaAction( MetaActionType::CREATEDESTPDF ), maRect( rRect ), mnPage( nPage ), meType( eType ) {}
    virtual             ~MetaCreateDestPDFAction() {}

    const tools::Rectangle& GetRect() const { return maRect; }
    sal_Int32           GetPage() const { return mnPage; }
    ::vcl::PDFWriter::DestAreaType GetType() const { return meType; }
};

class SAL_DLLPRIVATE MetaSetLinkDestPDFAction : public MetaAction
{
private:
    sal_Int32           mnLink;
    sal_Int32           mnDest;

public:
                        MetaSetLinkDestPDFAction( sal_Int32 nLink, sal_Int32 nDest ) : MetaAction( MetaActionType::SETLINKDESTPDF ), mnLink( nLink ), mnDest( nDest ) {}
    virtual             ~MetaSetLinkDestPDFAction() {}

    sal_Int32           GetLink() const { return mnLink; }
    sal_Int32           GetDest() const { return mnDest; }
};

class SAL_DLLPRIVATE MetaSetLinkUrlPDFAction : public MetaAction
{
private:
    sal_Int32           mnLink;
    OUString            maURL;

public:
                        MetaSetLinkUrlPDFAction( sal_Int32 nLink, const OUString& rURL ) : MetaAction( MetaActionType::SETLINKURLPDF ), mnLink( nLink ), maURL( rURL ) {}
    virtual             ~MetaSetLinkUrlPDFAction() {}

    sal_Int32           GetLink() const { return mnLink; }
    const OUString&     GetURL() const { return maURL; }
};

class SAL_DLLPRIVATE MetaSetLinkPropertyIdPDFAction : public MetaAction
{
private:
    sal_Int32           mnLink;
    sal_Int32           mnProp;

public:
                        MetaSetLinkPropertyIdPDFAction( sal_Int32 nLink, sal_Int32 nProp ) : MetaAction( MetaActionType::SETLINKPROPERTYIDPDF ), mnLink( nLink ), mnProp( nProp ) {}
    virtual             ~MetaSetLinkPropertyIdPDFAction() {}

    sal_Int32           GetLink() const { return mnLink; }
    sal_Int32           GetProperty() const { return mnProp; }
};

class SAL_DLLPRIVATE MetaCreateOutlineItemPDFAction : public MetaAction
{
private:
    sal_Int32           mnParent;
    OUString            maText;
    sal_Int32           mnDest;

public:
                        MetaCreateOutlineItemPDFAction( sal_Int32 nParent, const OUString& rText, sal_Int32 nDest ) : MetaAction( MetaActionType::CREATEOUTLINEITEMPDF ), mnParent( nParent ), maText( rText ), mnDest( nDest ) {}
    virtual             ~MetaCreateOutlineItemPDFAction() {}

    sal_Int32           GetParent() const { return mnParent; }
    const OUString&     GetText() const { return maText; }
    sal_Int32           GetDest() const { return mnDest; }
};

class SAL_DLLPRIVATE MetaCreateNotePDFAction : public MetaAction
{
private:
    tools::Rectangle    maRect;
    ::vcl::PDFNote      maNote;
    sal_Int32           mnPage;

public:
                        MetaCreateNotePDFAction( const tools::Rectangle& rRect, const ::vcl::PDFNote& rNote, sal_Int32 nPage ) : MetaAction( MetaActionType::CREATENOTEPDF ), maRect( rRect ), maNote( rNote ), mnPage( nPage ) {}
    virtual             ~MetaCreateNotePDFAction() {}

    const tools::Rectangle& GetRect() const { return maRect; }
    sal_Int32           GetPage() const { return mnPage; }
    const ::vcl::PDFNote& GetNote() const { return maNote; }
};

class SAL_DLLPRIVATE MetaCreateControlPDFAction : public MetaAction
{
private:
    ::vcl::PDFWriter::AnyWidget* mpControl;

public:
                        MetaCreateControlPDFAction( const ::vcl::PDFWriter::AnyWidget& rControl ) : MetaAction( MetaActionType::CREATECONTROLPDF ) { mpControl = rControl.Clone(); }
    virtual             ~MetaCreateControlPDFAction() { if ( mpControl ) delete mpControl; }

    const ::vcl::PDFWriter::AnyWidget& GetControl() const { return *mpControl; }
};

class SAL_DLLPRIVATE MetaBeginStructureElementPDFAction : public MetaAction
{
private:
    ::vcl::PDFWriter::StructElement meType;
    OUString            maAlias;

public:
                        MetaBeginStructureElementPDFAction( ::vcl::PDFWriter::StructElement eType, const OUString& rAlias ) : MetaAction( MetaActionType::BEGINSTRUCTUREELEMENTPDF ), meType( eType ), maAlias( rAlias ) {}
    virtual             ~MetaBeginStructureElementPDFAction() {}

    ::vcl::PDFWriter::StructElement GetType() const { return meType; }
    const OUString&     GetAlias() const { return maAlias; }
};

class SAL_DLLPRIVATE MetaEndStructureElementPDFAction : public MetaAction
{
public:
                        MetaEndStructureElementPDFAction() : MetaAction( MetaActionType::ENDSTRUCTUREELEMENTPDF ) {}
    virtual             ~MetaEndStructureElementPDFAction() {}
};

class SAL_DLLPRIVATE MetaSetCurrentStructureElementPDFAction : public MetaAction
{
private:
    sal_Int32           mnElement;

public:
                        MetaSetCurrentStructureElementPDFAction( sal_Int32 nElement ) : MetaAction( MetaActionType::SETCURRENTSTRUCTUREELEMENTPDF ), mnElement( nElement ) {}
    virtual             ~MetaSetCurrentStructureElementPDFAction() {}

    sal_Int32           GetElement() const { return mnElement; }
};

class SAL_DLLPRIVATE MetaSetStructureAttributePDFAction : public MetaAction
{
private:
    enum ::vcl::PDFWriter::StructAttribute meAttr;
    enum ::vcl::PDFWriter::StructAttributeValue meValue;

public:
                        MetaSetStructureAttributePDFAction( enum ::vcl::PDFWriter::StructAttribute eAttr, enum ::vcl::PDFWriter::StructAttributeValue eValue ) : MetaAction( MetaActionType::SETSTRUCTUREATTRIBUTEPDF ), meAttr( eAttr ), meValue( eValue ) {}
    virtual             ~MetaSetStructureAttributePDFAction() {}

    enum ::vcl::PDFWriter::StructAttribute GetAttribute() const { return meAttr; }
    enum ::vcl::PDFWriter::StructAttributeValue GetValue() const { return meValue; }
};

class SAL_DLLPRIVATE MetaSetStructureAttributeNumericalPDFAction : public MetaAction
{
private:
    enum ::vcl::PDFWriter::StructAttribute meAttr;
    sal_Int32           mnValue;

public:
                        MetaSetStructureAttributeNumericalPDFAction( enum ::vcl::PDFWriter::StructAttribute eAttr, sal_Int32 nValue ) : MetaAction( MetaActionType::SETSTRUCTUREATTRIBUTENUMERICALPDF ), meAttr( eAttr ), mnValue( nValue ) {}
    virtual             ~MetaSetStructureAttributeNumericalPDFAction() {}

    enum ::vcl::PDFWriter::StructAttribute GetAttribute() const { return meAttr; }
    sal_Int32           GetValue() const { return mnValue; }
};

class SAL_DLLPRIVATE MetaSetStructureBoundingBoxPDFAction : public MetaAction
{
private:
    tools::Rectangle    maRect;

public:
                        MetaSetStructureBoundingBoxPDFAction( const tools::Rectangle& rRect ) : MetaAction( MetaActionType::SETSTRUCTUREBOUNDINGBOXPDF ), maRect( rRect ) {}
    virtual             ~MetaSetStructureBoundingBoxPDFAction() {}

    const tools::Rectangle& GetRect() const { return maRect; }
};

class SAL_DLLPRIVATE MetaSetActualTextPDFAction : public MetaAction
{
private:
    OUString            maText;

public:
                        MetaSetActualTextPDFAction( const OUString& rText ) : MetaAction( MetaActionType::SETACTUALTEXTPDF ), maText( rText ) {}
    virtual             ~MetaSetActualTextPDFAction() {}

    const OUString&     GetText() const { return maText; }
};

class SAL_DLLPRIVATE MetaSetAlternateTextPDFAction : public MetaAction
{
private:
    OUString            maText;

public:
                        MetaSetAlternateTextPDFAction( const OUString& rText ) : MetaAction( MetaActionType::SETALTERNATETEXTPDF ), maText( rText ) {}
    virtual             ~MetaSetAlternateTextPDFAction() {}

    const OUString&     GetText() const { return maText; }
};

class SAL_DLLPRIVATE MetaSetPageTransitionPDFAction : public MetaAction
{
private:
    ::vcl::PDFWriter::PageTransition meType;
    sal_uInt32          mnMilliSeconds;
    sal_Int32           mnPage;

public:
                        MetaSetPageTransitionPDFAction( ::vcl::PDFWriter::PageTransition eType, sal_uInt32 nMilliSeconds, sal_Int32 nPage ) : MetaAction( MetaActionType::SETPAGETRANSITIONPDF ), meType( eType ), mnMilliSeconds( nMilliSeconds ), mnPage( nPage ) {}
    virtual             ~MetaSetPageTransitionPDFAction() {}

    ::vcl::PDFWriter::PageTransition GetType() const { return meType; }
    sal_uInt32          GetMilliSeconds() const { return mnMilliSeconds; }
    sal_Int32           GetPage() const { return mnPage; }
};

class SAL_DLLPRIVATE MetaDigitLanguagePDFAction : public MetaAction
{
private:
    LanguageType        meLang;

public:
                        MetaDigitLanguagePDFAction( LanguageType eLang ) : MetaAction( MetaActionType::DIGITLANGUAGEPDF ), meLang( eLang ) {}
    virtual             ~MetaDigitLanguagePDFAction() {}

    LanguageType        GetLanguage() const { return meLang; }
};

class SAL_DLLPRIVATE MetaBeginTransparencyGroupPDFAction : public MetaAction
{
public:
                        MetaBeginTransparencyGroupPDFAction() : MetaAction( MetaActionType::BEGINTRANSPARENCYGROUPPDF ) {}
    virtual             ~MetaBeginTransparencyGroupPDFAction() {}
};

class SAL_DLLPRIVATE MetaEndTransparencyGroupPDFAction : public MetaAction
{
private:
    tools::Rectangle    maBoundingRect;
    sal_uInt16          mnTransparentPercent;

public:
                        MetaEndTransparencyGroupPDFAction( const tools::Rectangle& rBoundingRect, sal_uInt16 nTransparentPercent ) : MetaAction( MetaActionType::ENDTRANSPARENCYGROUPPDF ), maBoundingRect( rBoundingRect ), mnTransparentPercent( nTransparentPercent ) {}
    virtual             ~MetaEndTransparencyGroupPDFAction() {}

    const tools::Rectangle& GetBoundingRect() const { return maBoundingRect; }
    sal_uInt16          GetTransparentPercent() const { return mnTransparentPercent; }
};

class SAL_DLLPRIVATE MetaSetLocalePDFAction : public MetaAction
{
private:
    com::sun::star::lang::Locale maLocale;

public:
                        MetaSetLocalePDFAction( const com::sun::star::lang::Locale& rLoc ) : MetaAction( MetaActionType::SETLOCALEPDF ), maLocale( rLoc ) {}
    virtual             ~MetaSetLocalePDFAction() {}

    const com::sun::star::lang::Locale& GetLocale() const { return maLocale; }
};
 
class SAL_DLLPRIVATE MetaCreateNamedDestPDFAction : public MetaAction
{
private:
    OUString            maDestName;
    tools::Rectangle    maRect;
    sal_Int32           mnPage;
    ::vcl::PDFWriter::DestAreaType meType;

public:
                        MetaCreateNamedDestPDFAction( const OUString& rDestName, const tools::Rectangle& rRect, sal_Int32 nPage, ::vcl::PDFWriter::DestAreaType eType ) : MetaAction( MetaActionType::CREATENAMEDDESTPDF ), maDestName( rDestName ), maRect( rRect ), mnPage( nPage ), meType( eType ) {}
    virtual             ~MetaCreateNamedDestPDFAction() {}

    const OUString&     GetDestName() const { return maDestName; }
    const tools::Rectangle& GetRect() const { return maRect; }
    sal_Int32           GetPage() const { return mnPage; }
    ::vcl::PDFWriter::DestAreaType GetType() const { return meType; }
};

class SAL_DLLPRIVATE MetaAddStreamPDFAction : public MetaAction
{
private:
    OUString             maMimeType;
    ::vcl::PDFOutputStream* mpStream;

public:
                        MetaAddStreamPDFAction( const OUString& rMimeType, ::vcl::PDFOutputStream *pStream ) : MetaAction( MetaActionType::ADDSTREAMPDF ), maMimeType( rMimeType ), mpStream( pStream ) {}
    virtual             ~MetaAddStreamPDFAction() {}

    const OUString&     GetMimeType() const { return maMimeType; }
    ::vcl::PDFOutputStream* GetPDFOutputStream() const { return mpStream; }
};

class SAL_DLLPRIVATE MetaRegisterDestReferencePDFAction : public MetaAction
{
private:
    sal_Int32           mnDestId;
    tools::Rectangle    maRect;
    sal_Int32           mnPage;
    ::vcl::PDFWriter::DestAreaType meType;

public:
                        MetaRegisterDestReferencePDFAction( sal_Int32 nDestId, const tools::Rectangle& rRect, sal_Int32 nPage, ::vcl::PDFWriter::DestAreaType eType ) : MetaAction( MetaActionType::REGISTERDESTREFERENCEPDF ), mnDestId( nDestId ), maRect( rRect ), mnPage( nPage ), meType( eType ) {}
    virtual             ~MetaRegisterDestReferencePDFAction() {}

    sal_Int32           GetDestId() const { return mnDestId; }
    const tools::Rectangle& GetRect() const { return maRect; }
    sal_Int32           GetPage() const { return mnPage; }
    ::vcl::PDFWriter::DestAreaType GetType() const { return meType; }
};

class SAL_DLLPRIVATE MetaPlayMetafilePDFAction : public MetaAction
{
private:
    GDIMetaFile         maMtf;
    ::vcl::PDFWriter::PlayMetafileContext maPlayContext;
    ::vcl::PDFExtOutDevData* mpData;

public:
                        MetaPlayMetafilePDFAction( const GDIMetaFile& rMtf, const ::vcl::PDFWriter::PlayMetafileContext& rPlayContext, ::vcl::PDFExtOutDevData* pData ) : MetaAction( MetaActionType::PLAYMETAFILEPDF ), maMtf( rMtf ), maPlayContext( rPlayContext ), mpData( pData ) {}
    virtual             ~MetaPlayMetafilePDFAction() {}

    const GDIMetaFile&  GetMetaFile() const { return maMtf; }
    const ::vcl::PDFWriter::PlayMetafileContext& GetPlayContext() const { return maPlayContext; }
    ::vcl::PDFExtOutDevData* GetData() const { return mpData; }
};

class SAL_DLLPRIVATE MetaBmpScalePDFAction : public MetaAction
{
private:
    Bitmap              maBitmap;
    Point               maPoint;
    Size                maSize;
    Graphic             maGraphic;

public:
                        MetaBmpScalePDFAction( const Point& rPoint, const Size& rSize, const Bitmap& rBitmap, const Graphic& rGraphic ) : MetaAction( MetaActionType::BMPSCALEPDF ), maBitmap( rBitmap ), maPoint( rPoint ), maSize( rSize ), maGraphic( rGraphic ) {}
    virtual             ~MetaBmpScalePDFAction() {}

    const Bitmap&       GetBitmap() const { return maBitmap; }
    const Point&        GetPoint() const { return maPoint; }
    const Size&         GetSize() const { return maSize; }
    const Graphic&      GetGraphic() const { return maGraphic; }
};

class SAL_DLLPRIVATE MetaCreateScreenPDFAction : public MetaAction
{
private:
    tools::Rectangle    maRect;
    sal_Int32           mnPage;

public:
                        MetaCreateScreenPDFAction( const tools::Rectangle& rRect, sal_Int32 nPage ) : MetaAction( MetaActionType::CREATESCREENPDF ), maRect( rRect ), mnPage( nPage ) {}
    virtual             ~MetaCreateScreenPDFAction() {}

    const tools::Rectangle& GetRect() const { return maRect; }
    sal_Int32           GetPage() const { return mnPage; }
};

class SAL_DLLPRIVATE MetaSetScreenStreamPDFAction : public MetaAction
{
private:
    sal_Int32           mnScreen;
    OUString            maURL;

public:
                        MetaSetScreenStreamPDFAction( sal_Int32 nScreen, const OUString& rURL ) : MetaAction( MetaActionType::SETSCREENSTREAMPDF ), mnScreen( nScreen ), maURL( rURL ) {}
    virtual             ~MetaSetScreenStreamPDFAction() {}

    sal_Int32           GetScreen() const { return mnScreen; }
    const OUString&     GetURL() const { return maURL; }
};

#endif	// USE_JAVA && MACOSX

 using namespace vcl;
 
#if defined USE_JAVA && defined MACOSX

static void ReplayMetaFile( PDFWriter &aWriter, GDIMetaFile& rMtf )
{
    for ( size_t i = 0, nCount = rMtf.GetActionSize(); i < nCount; i++ )
    {
        const MetaAction *pAction = rMtf.GetAction( i );
        const MetaActionType nType = pAction->GetType();

        switch( nType )
        {
            case MetaActionType::NEWPAGEPDF:
            {
                const MetaNewPagePDFAction* pA = static_cast< const MetaNewPagePDFAction* >( pAction );
                aWriter.NewPage( pA->GetPageWidth(), pA->GetPageHeight(), pA->GetOrientation() );
            }
            break;

            case MetaActionType::FONT:
            {
                const MetaFontAction* pA = static_cast< const MetaFontAction* >( pAction );
                aWriter.SetFont( pA->GetFont() );
            }
            break;

            case MetaActionType::TEXT:
            {
                const MetaTextAction* pA = static_cast< const MetaTextAction* >( pAction );
                aWriter.DrawText( pA->GetPoint(), pA->GetText() );
            }
            break;

            case MetaActionType::TEXTLINE:
            {
                const MetaTextLineAction* pA = static_cast< const MetaTextLineAction* >( pAction );
                aWriter.DrawTextLine( pA->GetStartPoint(), pA->GetWidth(), pA->GetStrikeout(), pA->GetUnderline(), pA->GetOverline() );
            }
            break;

            case MetaActionType::TEXTARRAY:
            {
                const MetaTextArrayAction* pA = static_cast< const MetaTextArrayAction* >( pAction );
                aWriter.DrawTextArray( pA->GetPoint(), pA->GetText(), pA->GetDXArray(), pA->GetIndex(), pA->GetLen() );
            }
            break;

            case MetaActionType::STRETCHTEXT:
            {
                const MetaStretchTextAction* pA = static_cast< const MetaStretchTextAction* >( pAction );
                aWriter.DrawStretchText( pA->GetPoint(), pA->GetWidth(), pA->GetText(), pA->GetIndex(), pA->GetLen() );
            }
            break;

            case MetaActionType::TEXTRECT:
            {
                const MetaTextRectAction* pA = static_cast< const MetaTextRectAction* >( pAction );
                aWriter.DrawText( pA->GetRect(), pA->GetText(), pA->GetStyle() );
            }
            break;

            case MetaActionType::LINE:
            {
                const MetaLineAction* pA = static_cast< const MetaLineAction* >( pAction );
                aWriter.DrawLine( pA->GetStartPoint(), pA->GetEndPoint(), pA->GetLineInfo() );
            }
            break;

            case MetaActionType::POLYGON:
            {
                const MetaPolygonAction* pA = static_cast< const MetaPolygonAction* >( pAction );
                aWriter.DrawPolygon( pA->GetPolygon() );
            }
            break;

            case MetaActionType::POLYLINE:
            {
                const MetaPolyLineAction* pA = static_cast< const MetaPolyLineAction* >( pAction );
                aWriter.DrawPolyLine( pA->GetPolygon(), pA->GetLineInfo() );
            }
            break;

            case MetaActionType::POLYLINEPDF:
            {
                const MetaPolyLinePDFAction* pA = static_cast< const MetaPolyLinePDFAction* >( pAction );
                aWriter.DrawPolyLine( pA->GetPolygon(), pA->GetExtLineInfo() );
            }
            break;

            case MetaActionType::RECT:
            {
                const MetaRectAction* pA = static_cast< const MetaRectAction* >( pAction );
                aWriter.DrawRect( pA->GetRect() );
            }
            break;

            case MetaActionType::ROUNDRECT:
            {
                const MetaRoundRectAction* pA = static_cast< const MetaRoundRectAction* >( pAction );
                aWriter.DrawRect( pA->GetRect(), pA->GetHorzRound(), pA->GetVertRound() );
            }
            break;

            case MetaActionType::ELLIPSE:
            {
                const MetaEllipseAction* pA = static_cast< const MetaEllipseAction* >( pAction );
                aWriter.DrawEllipse( pA->GetRect() );
            }
            break;

            case MetaActionType::PIE:
            {
                const MetaArcAction* pA = static_cast< const MetaArcAction* >( pAction );
                aWriter.DrawArc( pA->GetRect(), pA->GetStartPoint(), pA->GetEndPoint() );
            }
            break;

            case MetaActionType::CHORD:
            {
                const MetaChordAction* pA = static_cast< const MetaChordAction* >( pAction );
                aWriter.DrawArc( pA->GetRect(), pA->GetStartPoint(), pA->GetEndPoint() );
            }
            break;

            case MetaActionType::ARC:
            {
                const MetaArcAction* pA = static_cast< const MetaArcAction* >( pAction );
                aWriter.DrawArc( pA->GetRect(), pA->GetStartPoint(), pA->GetEndPoint() );
            }
            break;

            case MetaActionType::POLYPOLYGON:
            {
                const MetaPolyPolygonAction* pA = static_cast< const MetaPolyPolygonAction* >( pAction );
                aWriter.DrawPolyPolygon( pA->GetPolyPolygon() );
            }
            break;

            case MetaActionType::PIXEL:
            {
                const MetaPixelAction* pA = static_cast< const MetaPixelAction* >( pAction );
                aWriter.DrawPixel( pA->GetPoint(), pA->GetColor() );
            }
            break;

            case MetaActionType::BMPEXSCALE:
            {
                const MetaBmpExScaleAction* pA = static_cast< const MetaBmpExScaleAction* >( pAction );
                aWriter.DrawBitmapEx( pA->GetPoint(), pA->GetSize(), pA->GetBitmapEx() );
            }
            break;

            case MetaActionType::GRADIENT:
            {
                const MetaGradientAction* pA = static_cast< const MetaGradientAction* >( pAction );
                aWriter.DrawGradient( pA->GetRect(), pA->GetGradient() );
            }
            break;

            case MetaActionType::GRADIENTEX:
            {
                const MetaGradientExAction* pA = static_cast< const MetaGradientExAction* >( pAction );
                aWriter.DrawGradient( pA->GetPolyPolygon(), pA->GetGradient() );
            }
            break;

            case MetaActionType::HATCH:
            {
                const MetaHatchAction* pA = static_cast< const MetaHatchAction* >( pAction );
                aWriter.DrawHatch( pA->GetPolyPolygon(), pA->GetHatch() );
            }
            break;

            case MetaActionType::WALLPAPER:
            {
                const MetaWallpaperAction* pA = static_cast< const MetaWallpaperAction* >( pAction );
                aWriter.DrawWallpaper( pA->GetRect(), pA->GetWallpaper() );
            }
            break;

            case MetaActionType::Transparent:
            {
                const MetaTransparentAction* pA = static_cast< const MetaTransparentAction* >( pAction );
                aWriter.DrawTransparent( pA->GetPolyPolygon(), pA->GetTransparence() );
            }
            break;

            case MetaActionType::PUSH:
            {
                const MetaPushAction* pA = static_cast< const MetaPushAction* >( pAction );

                aWriter.Push( pA->GetFlags() );
            }
            break;

            case MetaActionType::POP:
            {
                aWriter.Pop();
            }
            break;

            case MetaActionType::MAPMODE:
            {
                const MetaMapModeAction* pA = static_cast< const MetaMapModeAction* >( pAction );
                aWriter.SetMapMode( pA->GetMapMode() );
            }
            break;

            case MetaActionType::LINECOLOR:
            {
                const MetaLineColorAction* pA = static_cast< const MetaLineColorAction* >( pAction );
                aWriter.SetLineColor( pA->GetColor() );
            }
            break;

            case MetaActionType::FILLCOLOR:
            {
                const MetaFillColorAction* pA = static_cast< const MetaFillColorAction* >( pAction );
                aWriter.SetFillColor( pA->GetColor() );
            }
            break;

            case MetaActionType::CLIPREGION:
            {
                const MetaClipRegionAction* pA = static_cast< const MetaClipRegionAction* >( pAction );
                if( pA->IsClipping() )
                    aWriter.SetClipRegion( pA->GetRegion().GetAsB2DPolyPolygon() );
                else
                    aWriter.SetClipRegion();
            }
            break;

            case MetaActionType::MOVECLIPREGION:
            {
                const MetaMoveClipRegionAction* pA = static_cast< const MetaMoveClipRegionAction* >( pAction );
                aWriter.MoveClipRegion( pA->GetHorzMove(), pA->GetVertMove() );
            }
            break;

            case MetaActionType::ISECTRECTCLIPREGION:
            {
                const MetaISectRectClipRegionAction* pA = static_cast< const MetaISectRectClipRegionAction* >( pAction );
                aWriter.IntersectClipRegion( pA->GetRect() );
            }
            break;

            case MetaActionType::ISECTREGIONCLIPREGION:
            {
               const MetaISectRegionClipRegionAction* pA = static_cast< const MetaISectRegionClipRegionAction* >( pAction );
               aWriter.IntersectClipRegion( pA->GetRegion().GetAsB2DPolyPolygon() );
            }
            break;

            case MetaActionType::LAYOUTMODE:
            {
                const MetaLayoutModeAction* pA = static_cast< const MetaLayoutModeAction* >( pAction );
                aWriter.SetLayoutMode( pA->GetLayoutMode() );
            }
            break;

            case MetaActionType::TEXTCOLOR:
            {
                const MetaTextColorAction* pA = static_cast< const MetaTextColorAction* >( pAction );
                aWriter.SetTextColor( pA->GetColor() );
            }
            break;

            case MetaActionType::TEXTFILLCOLOR:
            {
                const MetaTextFillColorAction* pA = static_cast< const MetaTextFillColorAction* >( pAction );
                if ( pA->IsSetting() )
                    aWriter.SetTextFillColor( pA->GetColor() );
                else
                    aWriter.SetTextFillColor();
            }
            break;

            case MetaActionType::TEXTLINECOLOR:
            {
                const MetaTextLineColorAction* pA = static_cast< const MetaTextLineColorAction* >( pAction );
                if ( pA->IsSetting() )
                    aWriter.SetTextLineColor( pA->GetColor() );
                else
                    aWriter.SetTextLineColor();
            }
            break;

            case MetaActionType::OVERLINECOLOR:
            {
                const MetaOverlineColorAction* pA = static_cast< const MetaOverlineColorAction* >( pAction );
                if ( pA->IsSetting() )
                    aWriter.SetOverlineColor( pA->GetColor() );
                else
                    aWriter.SetOverlineColor();
            }
            break;

            case MetaActionType::TEXTALIGN:
            {
                const MetaTextAlignAction* pA = static_cast< const MetaTextAlignAction* >( pAction );
                aWriter.SetTextAlign( pA->GetTextAlign() );
            }
            break;

            case MetaActionType::JPGPDF:
            {
                const MetaJpgPDFAction* pA = static_cast< const MetaJpgPDFAction* >( pAction );
                aWriter.DrawJPGBitmap( const_cast< SvStream& >( pA->GetStream() ), pA->IsTrueColor(), pA->GetSize(), pA->GetRect(), pA->GetMask(), pA->GetGraphic() );
            }
            break;

            case MetaActionType::CREATELINKPDF:
            {
                const MetaCreateLinkPDFAction* pA = static_cast< const MetaCreateLinkPDFAction* >( pAction );
                aWriter.CreateLink( pA->GetRect(), pA->GetPage() );
            }
            break;

            case MetaActionType::CREATEDESTPDF:
            {
                const MetaCreateDestPDFAction* pA = static_cast< const MetaCreateDestPDFAction* >( pAction );
                aWriter.CreateDest( pA->GetRect(), pA->GetPage(), pA->GetType() );
            }
            break;

            case MetaActionType::SETLINKDESTPDF:
            {
                const MetaSetLinkDestPDFAction* pA = static_cast< const MetaSetLinkDestPDFAction* >( pAction );
                aWriter.SetLinkDest( pA->GetLink(), pA->GetDest() );
            }
            break;

            case MetaActionType::SETLINKURLPDF:
            {
                const MetaSetLinkUrlPDFAction* pA = static_cast< const MetaSetLinkUrlPDFAction* >( pAction );
                aWriter.SetLinkURL( pA->GetLink(), pA->GetURL() );
            }
            break;

            case MetaActionType::SETLINKPROPERTYIDPDF:
            {
                const MetaSetLinkPropertyIdPDFAction* pA = static_cast< const MetaSetLinkPropertyIdPDFAction* >( pAction );
                aWriter.SetLinkPropertyID( pA->GetLink(), pA->GetProperty() );
            }
            break;

            case MetaActionType::CREATEOUTLINEITEMPDF:
            {
                const MetaCreateOutlineItemPDFAction* pA = static_cast< const MetaCreateOutlineItemPDFAction* >( pAction );
                aWriter.CreateOutlineItem( pA->GetParent(), pA->GetText(), pA->GetDest() );
            }
            break;

            case MetaActionType::CREATENOTEPDF:
            {
                const MetaCreateNotePDFAction* pA = static_cast< const MetaCreateNotePDFAction* >( pAction );
                aWriter.CreateNote( pA->GetRect(), pA->GetNote(), pA->GetPage() );
            }
            break;

            case MetaActionType::BEGINSTRUCTUREELEMENTPDF:
            {
                const MetaBeginStructureElementPDFAction* pA = static_cast< const MetaBeginStructureElementPDFAction* >( pAction );
                aWriter.BeginStructureElement( pA->GetType(), pA->GetAlias() );
            }
            break;

            case MetaActionType::ENDSTRUCTUREELEMENTPDF:
            {
                aWriter.EndStructureElement();
            }
            break;

            case MetaActionType::SETCURRENTSTRUCTUREELEMENTPDF:
            {
                const MetaSetCurrentStructureElementPDFAction* pA = static_cast< const MetaSetCurrentStructureElementPDFAction* >( pAction );
                aWriter.SetCurrentStructureElement( pA->GetElement() );
            }
            break;

            case MetaActionType::SETSTRUCTUREATTRIBUTEPDF:
            {
                const MetaSetStructureAttributePDFAction* pA = static_cast< const MetaSetStructureAttributePDFAction* >( pAction );
                aWriter.SetStructureAttribute( pA->GetAttribute(), pA->GetValue() );
            }
            break;

            case MetaActionType::SETSTRUCTUREATTRIBUTENUMERICALPDF:
            {
                const MetaSetStructureAttributeNumericalPDFAction* pA = static_cast< const MetaSetStructureAttributeNumericalPDFAction* >( pAction );
                aWriter.SetStructureAttributeNumerical( pA->GetAttribute(), pA->GetValue() );
            }
            break;

            case MetaActionType::SETSTRUCTUREBOUNDINGBOXPDF:
            {
                const MetaSetStructureBoundingBoxPDFAction* pA = static_cast< const MetaSetStructureBoundingBoxPDFAction* >( pAction );
                aWriter.SetStructureBoundingBox( pA->GetRect() );
            }
            break;

            case MetaActionType::SETACTUALTEXTPDF:
            {
                const MetaSetActualTextPDFAction* pA = static_cast< const MetaSetActualTextPDFAction* >( pAction );
                aWriter.SetActualText( pA->GetText() );
            }
            break;

            case MetaActionType::SETALTERNATETEXTPDF:
            {
                const MetaSetAlternateTextPDFAction* pA = static_cast< const MetaSetAlternateTextPDFAction* >( pAction );
                aWriter.SetAlternateText( pA->GetText() );
            }
            break;

            case MetaActionType::SETPAGETRANSITIONPDF:
            {
                const MetaSetPageTransitionPDFAction* pA = static_cast< const MetaSetPageTransitionPDFAction* >( pAction );
                aWriter.SetPageTransition( pA->GetType(), pA->GetMilliSeconds(), pA->GetPage() );
            }
            break;

            case MetaActionType::CREATECONTROLPDF:
            {
                const MetaCreateControlPDFAction* pA = static_cast< const MetaCreateControlPDFAction* >( pAction );
                aWriter.CreateControl( pA->GetControl() );
            }
            break;

            case MetaActionType::DIGITLANGUAGEPDF:
            {
                const MetaDigitLanguagePDFAction* pA = static_cast< const MetaDigitLanguagePDFAction* >( pAction );
                aWriter.SetDigitLanguage( pA->GetLanguage() );
            }
            break;

            case MetaActionType::BEGINTRANSPARENCYGROUPPDF:
            {
                aWriter.BeginTransparencyGroup();
            }
            break;

            case MetaActionType::ENDTRANSPARENCYGROUPPDF:
            {
                const MetaEndTransparencyGroupPDFAction* pA = static_cast< const MetaEndTransparencyGroupPDFAction* >( pAction );
                aWriter.EndTransparencyGroup( pA->GetBoundingRect(), pA->GetTransparentPercent() );
            }
            break;

            case MetaActionType::SETLOCALEPDF:
            {
                const MetaSetLocalePDFAction* pA = static_cast< const MetaSetLocalePDFAction* >( pAction );
                aWriter.SetDocumentLocale( pA->GetLocale() );
            }
            break;

            case MetaActionType::CREATENAMEDDESTPDF:
            {
                const MetaCreateNamedDestPDFAction* pA = static_cast< const MetaCreateNamedDestPDFAction* >( pAction );
                aWriter.CreateNamedDest( pA->GetDestName(), pA->GetRect(), pA->GetPage(), pA->GetType() );
            }
            break;

            case MetaActionType::ADDSTREAMPDF:
            {
                const MetaAddStreamPDFAction* pA = static_cast< const MetaAddStreamPDFAction* >( pAction );
                aWriter.AddStream( pA->GetMimeType(), pA->GetPDFOutputStream() );
            }
            break;

            case MetaActionType::REGISTERDESTREFERENCEPDF:
            {
                const MetaRegisterDestReferencePDFAction* pA = static_cast< const MetaRegisterDestReferencePDFAction* >( pAction );
                aWriter.RegisterDestReference( pA->GetDestId(), pA->GetRect(), pA->GetPage(), pA->GetType() );
            }
            break;

            case MetaActionType::PLAYMETAFILEPDF:
            {
                const MetaPlayMetafilePDFAction* pA = static_cast< const MetaPlayMetafilePDFAction* >( pAction );
                aWriter.PlayMetafile( pA->GetMetaFile(), pA->GetPlayContext(), pA->GetData() );
            }
            break;

            case MetaActionType::BMPSCALEPDF:
            {
                const MetaBmpScalePDFAction* pA = static_cast< const MetaBmpScalePDFAction* >( pAction );
                aWriter.DrawBitmap( pA->GetPoint(), pA->GetSize(), pA->GetBitmap(), pA->GetGraphic() );
            }
            break;

            case MetaActionType::CREATESCREENPDF:
            {
                const MetaCreateScreenPDFAction* pA = static_cast< const MetaCreateScreenPDFAction* >( pAction );
                aWriter.CreateScreen( pA->GetRect(), pA->GetPage() );
            }
            break;

            case MetaActionType::SETSCREENSTREAMPDF:
            {
                const MetaSetScreenStreamPDFAction* pA = static_cast< const MetaSetScreenStreamPDFAction* >( pAction );
                aWriter.SetScreenStream( pA->GetScreen(), pA->GetURL() );
            }
            break;

            default:
#ifdef DBG_UTIL
                OSL_FAIL( "PDFWriterImpl::emit: unsupported MetaAction #" );
#endif	// DBG_UTIL
            break;
        }
    }
}

#endif	// USE_JAVA && MACOSX
 
PDFWriter::AnyWidget::~AnyWidget()
{
}

PDFWriter::PDFSignContext::PDFSignContext(OStringBuffer& rCMSHexBuffer)
    : m_pDerEncoded(nullptr),
      m_nDerEncoded(0),
      m_pByteRange1(nullptr),
      m_nByteRange1(0),
      m_pByteRange2(nullptr),
      m_nByteRange2(0),
      m_rCMSHexBuffer(rCMSHexBuffer)
{
}

PDFWriter::PDFWriter( const PDFWriter::PDFWriterContext& rContext, const css::uno::Reference< css::beans::XMaterialHolder >& xEnc )
        :
        xImplementation( new PDFWriterImpl( rContext, xEnc, *this ) )
{
}

PDFWriter::~PDFWriter()
{
}

OutputDevice* PDFWriter::GetReferenceDevice()
{
    return xImplementation->getReferenceDevice();
}

void PDFWriter::NewPage( double nPageWidth, double nPageHeight, Orientation eOrientation )
{
#if defined USE_JAVA && defined MACOSX
    bool bOldInOuterFace = xImplementation->isInOuterFace();
    if ( !bOldInOuterFace && !xImplementation->isReplayWriter() )
        xImplementation->addAction( new MetaNewPagePDFAction( nPageWidth, nPageHeight, eOrientation ) );
    xImplementation->setInOuterFace( true );
#endif	// USE_JAVA && MACOSX
    xImplementation->newPage( nPageWidth, nPageHeight, eOrientation );
#if defined USE_JAVA && defined MACOSX
    xImplementation->setInOuterFace( bOldInOuterFace );
#endif	// USE_JAVA && MACOSX
}

bool PDFWriter::Emit()
{
#if defined USE_JAVA && defined MACOSX
    bool bRet = false;

    // Replay meta actions
    if ( !xImplementation->isReplayWriter() && xImplementation->emit() )
    {
        GDIMetaFile aMtf( xImplementation->getReplayMetaFile() );
        const PDFWriter::PDFWriterContext& rContext = xImplementation->getContext();
        PDFWriter& rPDFWriter = xImplementation->getPDFWriter();

        // Fix bug 3061 by making a substitute writer and copying the actions
        // into that as the current writer seems to get mangled layouts in some
        // cases
        std::unique_ptr<PDFWriterImpl> xSubstituteImplementation( new PDFWriterImpl( rContext, com::sun::star::uno::Reference< com::sun::star::beans::XMaterialHolder >(), rPDFWriter, nullptr, xImplementation.get() ) );
        xSubstituteImplementation.swap( xImplementation );
        ReplayMetaFile( *this, aMtf );
        bRet = xImplementation->emit();
        xImplementation.swap( xSubstituteImplementation );

        // Now replay the same meta file into the final destination
        if ( bRet )
        {
            std::unique_ptr<PDFWriterImpl> xFinalImplementation( new PDFWriterImpl( rContext, com::sun::star::uno::Reference< com::sun::star::beans::XMaterialHolder >(), rPDFWriter, xSubstituteImplementation.get(), xImplementation.get() ) );
            xFinalImplementation.swap( xImplementation );
            ReplayMetaFile( *this, aMtf );
            bRet = xImplementation->emit();
            xImplementation.swap( xFinalImplementation );
        }
    }

    return bRet;
#else	// USE_JAVA && MACOSX
    return xImplementation->emit();
#endif	// USE_JAVA && MACOSX
}

void PDFWriter::SetDocumentLocale( const css::lang::Locale& rLoc )
{
#if defined USE_JAVA && defined MACOSX
    bool bOldInOuterFace = xImplementation->isInOuterFace();
    if ( !bOldInOuterFace && !xImplementation->isReplayWriter() )
        xImplementation->addAction( new MetaSetLocalePDFAction( rLoc ) );
    xImplementation->setInOuterFace( true );
#endif	// USE_JAVA && MACOSX
    xImplementation->setDocumentLocale( rLoc );
#if defined USE_JAVA && defined MACOSX
    xImplementation->setInOuterFace( bOldInOuterFace );
#endif	// USE_JAVA && MACOSX
}

void PDFWriter::SetFont( const vcl::Font& rFont )
{
#if defined USE_JAVA && defined MACOSX
    bool bOldInOuterFace = xImplementation->isInOuterFace();
    if ( !bOldInOuterFace && !xImplementation->isReplayWriter() )
        xImplementation->addAction( new MetaFontAction( rFont ) );
    xImplementation->setInOuterFace( true );
#endif	// USE_JAVA && MACOSX
    xImplementation->setFont( rFont );
#if defined USE_JAVA && defined MACOSX
    xImplementation->setInOuterFace( bOldInOuterFace );
#endif	// USE_JAVA && MACOSX
}

void PDFWriter::DrawText( const Point& rPos, const OUString& rText )
{
#if defined USE_JAVA && defined MACOSX
    bool bOldInOuterFace = xImplementation->isInOuterFace();
    if ( !bOldInOuterFace && !xImplementation->isReplayWriter() )
        xImplementation->addAction( new MetaTextAction( rPos, rText, 0, rText.getLength() ) );
    xImplementation->setInOuterFace( true );
#endif	// USE_JAVA && MACOSX
    xImplementation->drawText( rPos, rText, 0, rText.getLength() );
#if defined USE_JAVA && defined MACOSX
    xImplementation->setInOuterFace( bOldInOuterFace );
#endif	// USE_JAVA && MACOSX
}

void PDFWriter::DrawTextLine(
                             const Point& rPos,
                             long nWidth,
                             FontStrikeout eStrikeout,
                             FontLineStyle eUnderline,
                             FontLineStyle eOverline )
{
#if defined USE_JAVA && defined MACOSX
    bool bOldInOuterFace = xImplementation->isInOuterFace();
    if ( !bOldInOuterFace && !xImplementation->isReplayWriter() )
        xImplementation->addAction( new MetaTextLineAction( rPos, nWidth, eStrikeout, eUnderline, eOverline ) );
    xImplementation->setInOuterFace( true );
#endif	// USE_JAVA && MACOSX
    xImplementation->drawTextLine( rPos, nWidth, eStrikeout, eUnderline, eOverline, false/*bUnderlineAbove*/ );
#if defined USE_JAVA && defined MACOSX
    xImplementation->setInOuterFace( bOldInOuterFace );
#endif	// USE_JAVA && MACOSX
}

void PDFWriter::DrawTextArray(
                              const Point& rStartPt,
                              const OUString& rStr,
                              const long* pDXAry,
                              sal_Int32 nIndex,
                              sal_Int32 nLen )
{
#if defined USE_JAVA && defined MACOSX
    bool bOldInOuterFace = xImplementation->isInOuterFace();
    if ( !bOldInOuterFace && !xImplementation->isReplayWriter() )
        xImplementation->addAction( new MetaTextArrayAction( rStartPt, rStr, pDXAry, nIndex, nLen ) );
    xImplementation->setInOuterFace( true );
#endif	// USE_JAVA && MACOSX
    xImplementation->drawTextArray( rStartPt, rStr, pDXAry, nIndex, nLen );
#if defined USE_JAVA && defined MACOSX
    xImplementation->setInOuterFace( bOldInOuterFace );
#endif	// USE_JAVA && MACOSX
}

void PDFWriter::DrawStretchText(
                                const Point& rStartPt,
                                sal_uLong nWidth,
                                const OUString& rStr,
                                sal_Int32 nIndex,
                                sal_Int32 nLen )
{
#if defined USE_JAVA && defined MACOSX
    bool bOldInOuterFace = xImplementation->isInOuterFace();
    if ( !bOldInOuterFace && !xImplementation->isReplayWriter() )
        xImplementation->addAction( new MetaStretchTextAction( rStartPt, nWidth, rStr, nIndex, nLen ) );
    xImplementation->setInOuterFace( true );
#endif	// USE_JAVA && MACOSX
    xImplementation->drawStretchText( rStartPt, nWidth, rStr, nIndex, nLen );
#if defined USE_JAVA && defined MACOSX
    xImplementation->setInOuterFace( bOldInOuterFace );
#endif	// USE_JAVA && MACOSX
}

void PDFWriter::DrawText(
                         const tools::Rectangle& rRect,
                         const OUString& rStr,
                         DrawTextFlags nStyle )
{
#if defined USE_JAVA && defined MACOSX
    bool bOldInOuterFace = xImplementation->isInOuterFace();
    if ( !bOldInOuterFace && !xImplementation->isReplayWriter() )
        xImplementation->addAction( new MetaTextRectAction( rRect, rStr, nStyle ) );
    xImplementation->setInOuterFace( true );
#endif	// USE_JAVA && MACOSX
    xImplementation->drawText( rRect, rStr, nStyle );
#if defined USE_JAVA && defined MACOSX
    xImplementation->setInOuterFace( bOldInOuterFace );
#endif	// USE_JAVA && MACOSX
}

void PDFWriter::DrawLine( const Point& rStart, const Point& rStop )
{
#if defined USE_JAVA && defined MACOSX
    bool bOldInOuterFace = xImplementation->isInOuterFace();
    if ( !bOldInOuterFace && !xImplementation->isReplayWriter() )
        xImplementation->addAction( new MetaLineAction( rStart, rStop ) );
    xImplementation->setInOuterFace( true );
#endif	// USE_JAVA && MACOSX
    xImplementation->drawLine( rStart, rStop );
#if defined USE_JAVA && defined MACOSX
    xImplementation->setInOuterFace( bOldInOuterFace );
#endif	// USE_JAVA && MACOSX
}

void PDFWriter::DrawLine( const Point& rStart, const Point& rStop, const LineInfo& rInfo )
{
#if defined USE_JAVA && defined MACOSX
    bool bOldInOuterFace = xImplementation->isInOuterFace();
    if ( !bOldInOuterFace && !xImplementation->isReplayWriter() )
        xImplementation->addAction( new MetaLineAction( rStart, rStop, rInfo ) );
    xImplementation->setInOuterFace( true );
#endif	// USE_JAVA && MACOSX
    xImplementation->drawLine( rStart, rStop, rInfo );
#if defined USE_JAVA && defined MACOSX
    xImplementation->setInOuterFace( bOldInOuterFace );
#endif	// USE_JAVA && MACOSX
}

void PDFWriter::DrawPolygon( const tools::Polygon& rPoly )
{
#if defined USE_JAVA && defined MACOSX
    bool bOldInOuterFace = xImplementation->isInOuterFace();
    if ( !bOldInOuterFace && !xImplementation->isReplayWriter() )
        xImplementation->addAction( new MetaPolygonAction( rPoly ) );
    xImplementation->setInOuterFace( true );
#endif	// USE_JAVA && MACOSX
    xImplementation->drawPolygon( rPoly );
#if defined USE_JAVA && defined MACOSX
    xImplementation->setInOuterFace( bOldInOuterFace );
#endif	// USE_JAVA && MACOSX
}

void PDFWriter::DrawPolyLine( const tools::Polygon& rPoly )
{
#if defined USE_JAVA && defined MACOSX
    bool bOldInOuterFace = xImplementation->isInOuterFace();
    if ( !bOldInOuterFace && !xImplementation->isReplayWriter() )
        xImplementation->addAction( new MetaPolyLineAction( rPoly ) );
    xImplementation->setInOuterFace( true );
#endif	// USE_JAVA && MACOSX
    xImplementation->drawPolyLine( rPoly );
#if defined USE_JAVA && defined MACOSX
    xImplementation->setInOuterFace( bOldInOuterFace );
#endif	// USE_JAVA && MACOSX
}

void PDFWriter::DrawRect( const tools::Rectangle& rRect )
{
#if defined USE_JAVA && defined MACOSX
    bool bOldInOuterFace = xImplementation->isInOuterFace();
    if ( !bOldInOuterFace && !xImplementation->isReplayWriter() )
        xImplementation->addAction( new MetaRectAction( rRect ) );
    xImplementation->setInOuterFace( true );
#endif	// USE_JAVA && MACOSX
    xImplementation->drawRectangle( rRect );
#if defined USE_JAVA && defined MACOSX
    xImplementation->setInOuterFace( bOldInOuterFace );
#endif	// USE_JAVA && MACOSX
}

void PDFWriter::DrawRect( const tools::Rectangle& rRect, sal_uLong nHorzRound, sal_uLong nVertRound )
{
#if defined USE_JAVA && defined MACOSX
    bool bOldInOuterFace = xImplementation->isInOuterFace();
    if ( !bOldInOuterFace && !xImplementation->isReplayWriter() )
        xImplementation->addAction( new MetaRoundRectAction( rRect, nHorzRound, nVertRound ) );
    xImplementation->setInOuterFace( true );
#endif	// USE_JAVA && MACOSX
    xImplementation->drawRectangle( rRect, nHorzRound, nVertRound );
#if defined USE_JAVA && defined MACOSX
    xImplementation->setInOuterFace( bOldInOuterFace );
#endif	// USE_JAVA && MACOSX
}

void PDFWriter::DrawEllipse( const tools::Rectangle& rRect )
{
#if defined USE_JAVA && defined MACOSX
    bool bOldInOuterFace = xImplementation->isInOuterFace();
    if ( !bOldInOuterFace && !xImplementation->isReplayWriter() )
        xImplementation->addAction( new MetaEllipseAction( rRect ) );
    xImplementation->setInOuterFace( true );
#endif	// USE_JAVA && MACOSX
    xImplementation->drawEllipse( rRect );
#if defined USE_JAVA && defined MACOSX
    xImplementation->setInOuterFace( bOldInOuterFace );
#endif	// USE_JAVA && MACOSX
}

void PDFWriter::DrawArc( const tools::Rectangle& rRect, const Point& rStart, const Point& rStop )
{
#if defined USE_JAVA && defined MACOSX
    bool bOldInOuterFace = xImplementation->isInOuterFace();
    if ( !bOldInOuterFace && !xImplementation->isReplayWriter() )
        xImplementation->addAction( new MetaArcAction( rRect, rStart, rStop ) );
    xImplementation->setInOuterFace( true );
#endif	// USE_JAVA && MACOSX
    xImplementation->drawArc( rRect, rStart, rStop, false, false );
#if defined USE_JAVA && defined MACOSX
    xImplementation->setInOuterFace( bOldInOuterFace );
#endif	// USE_JAVA && MACOSX
}

void PDFWriter::DrawPie( const tools::Rectangle& rRect, const Point& rStart, const Point& rStop )
{
#if defined USE_JAVA && defined MACOSX
    bool bOldInOuterFace = xImplementation->isInOuterFace();
    if ( !bOldInOuterFace && !xImplementation->isReplayWriter() )
        xImplementation->addAction( new MetaPieAction( rRect, rStart, rStop ) );
    xImplementation->setInOuterFace( true );
#endif	// USE_JAVA && MACOSX
    xImplementation->drawArc( rRect, rStart, rStop, true, false );
#if defined USE_JAVA && defined MACOSX
    xImplementation->setInOuterFace( bOldInOuterFace );
#endif	// USE_JAVA && MACOSX
}

void PDFWriter::DrawChord( const tools::Rectangle& rRect, const Point& rStart, const Point& rStop )
{
#if defined USE_JAVA && defined MACOSX
    bool bOldInOuterFace = xImplementation->isInOuterFace();
    if ( !bOldInOuterFace && !xImplementation->isReplayWriter() )
        xImplementation->addAction( new MetaChordAction( rRect, rStart, rStop ) );
    xImplementation->setInOuterFace( true );
#endif	// USE_JAVA && MACOSX
    xImplementation->drawArc( rRect, rStart, rStop, false, true );
#if defined USE_JAVA && defined MACOSX
    xImplementation->setInOuterFace( bOldInOuterFace );
#endif	// USE_JAVA && MACOSX
}

void PDFWriter::DrawPolyLine( const tools::Polygon& rPoly, const LineInfo& rInfo )
{
#if defined USE_JAVA && defined MACOSX
    bool bOldInOuterFace = xImplementation->isInOuterFace();
    if ( !bOldInOuterFace && !xImplementation->isReplayWriter() )
        xImplementation->addAction( new MetaPolyLineAction( rPoly, rInfo ) );
    xImplementation->setInOuterFace( true );
#endif	// USE_JAVA && MACOSX
    xImplementation->drawPolyLine( rPoly, rInfo );
#if defined USE_JAVA && defined MACOSX
    xImplementation->setInOuterFace( bOldInOuterFace );
#endif	// USE_JAVA && MACOSX
}

void PDFWriter::DrawPolyLine( const tools::Polygon& rPoly, const ExtLineInfo& rInfo )
{
#if defined USE_JAVA && defined MACOSX
    bool bOldInOuterFace = xImplementation->isInOuterFace();
    if ( !bOldInOuterFace && !xImplementation->isReplayWriter() )
        xImplementation->addAction( new MetaPolyLinePDFAction( rPoly, rInfo ) );
    xImplementation->setInOuterFace( true );
#endif	// USE_JAVA && MACOSX
    xImplementation->drawPolyLine( rPoly, rInfo );
#if defined USE_JAVA && defined MACOSX
    xImplementation->setInOuterFace( bOldInOuterFace );
#endif	// USE_JAVA && MACOSX
}

void PDFWriter::DrawPolyPolygon( const tools::PolyPolygon& rPolyPoly )
{
#if defined USE_JAVA && defined MACOSX
    bool bOldInOuterFace = xImplementation->isInOuterFace();
    if ( !bOldInOuterFace && !xImplementation->isReplayWriter() )
        xImplementation->addAction( new MetaPolyPolygonAction( rPolyPoly ) );
    xImplementation->setInOuterFace( true );
#endif	// USE_JAVA && MACOSX
    xImplementation->drawPolyPolygon( rPolyPoly );
#if defined USE_JAVA && defined MACOSX
    xImplementation->setInOuterFace( bOldInOuterFace );
#endif	// USE_JAVA && MACOSX
}

void PDFWriter::DrawPixel( const Point& rPos, const Color& rColor )
{
#if defined USE_JAVA && defined MACOSX
    bool bOldInOuterFace = xImplementation->isInOuterFace();
    if ( !bOldInOuterFace && !xImplementation->isReplayWriter() )
        xImplementation->addAction( new MetaPixelAction( rPos, rColor ) );
    xImplementation->setInOuterFace( true );
#endif	// USE_JAVA && MACOSX
    xImplementation->drawPixel( rPos, rColor );
#if defined USE_JAVA && defined MACOSX
    xImplementation->setInOuterFace( bOldInOuterFace );
#endif	// USE_JAVA && MACOSX
}

void PDFWriter::DrawBitmap( const Point& rDestPt, const Size& rDestSize, const Bitmap& rBitmap, const Graphic& rGraphic )
{
#if defined USE_JAVA && defined MACOSX
    bool bOldInOuterFace = xImplementation->isInOuterFace();
    if ( !bOldInOuterFace && !xImplementation->isReplayWriter() )
        xImplementation->addAction( new MetaBmpScalePDFAction( rDestPt, rDestSize, rBitmap, rGraphic ) );
    xImplementation->setInOuterFace( true );
#endif	// USE_JAVA && MACOSX
    xImplementation->drawBitmap( rDestPt, rDestSize, rBitmap, rGraphic );
#if defined USE_JAVA && defined MACOSX
    xImplementation->setInOuterFace( bOldInOuterFace );
#endif	// USE_JAVA && MACOSX
}

void PDFWriter::DrawBitmapEx( const Point& rDestPt, const Size& rDestSize, const BitmapEx& rBitmap )
{
#if defined USE_JAVA && defined MACOSX
    bool bOldInOuterFace = xImplementation->isInOuterFace();
    if ( !bOldInOuterFace && !xImplementation->isReplayWriter() )
        xImplementation->addAction( new MetaBmpExScaleAction( rDestPt, rDestSize, rBitmap ) );
    xImplementation->setInOuterFace( true );
#endif	// USE_JAVA && MACOSX
    xImplementation->drawBitmap( rDestPt, rDestSize, rBitmap );
#if defined USE_JAVA && defined MACOSX
    xImplementation->setInOuterFace( bOldInOuterFace );
#endif	// USE_JAVA && MACOSX
}

void PDFWriter::DrawHatch( const tools::PolyPolygon& rPolyPoly, const Hatch& rHatch )
{
#if defined USE_JAVA && defined MACOSX
    bool bOldInOuterFace = xImplementation->isInOuterFace();
    if ( !bOldInOuterFace && !xImplementation->isReplayWriter() )
        xImplementation->addAction( new MetaHatchAction( rPolyPoly, rHatch ) );
    xImplementation->setInOuterFace( true );
#endif	// USE_JAVA && MACOSX
    xImplementation->drawHatch( rPolyPoly, rHatch );
#if defined USE_JAVA && defined MACOSX
    xImplementation->setInOuterFace( bOldInOuterFace );
#endif	// USE_JAVA && MACOSX
}

void PDFWriter::DrawGradient( const tools::Rectangle& rRect, const Gradient& rGradient )
{
#if defined USE_JAVA && defined MACOSX
    bool bOldInOuterFace = xImplementation->isInOuterFace();
    if ( !bOldInOuterFace && !xImplementation->isReplayWriter() )
        xImplementation->addAction( new MetaGradientAction( rRect, rGradient ) );
    xImplementation->setInOuterFace( true );
#endif	// USE_JAVA && MACOSX
    xImplementation->drawGradient( rRect, rGradient );
#if defined USE_JAVA && defined MACOSX
    xImplementation->setInOuterFace( bOldInOuterFace );
#endif	// USE_JAVA && MACOSX
}

void PDFWriter::DrawGradient( const tools::PolyPolygon& rPolyPoly, const Gradient& rGradient )
{
#if defined USE_JAVA && defined MACOSX
    bool bOldInOuterFace = xImplementation->isInOuterFace();
    if ( !bOldInOuterFace && !xImplementation->isReplayWriter() )
        xImplementation->addAction( new MetaGradientExAction( rPolyPoly, rGradient ) );
    xImplementation->setInOuterFace( true );
#endif	// USE_JAVA && MACOSX
    xImplementation->push(PushFlags::CLIPREGION);
    xImplementation->setClipRegion( rPolyPoly.getB2DPolyPolygon() );
    xImplementation->drawGradient( rPolyPoly.GetBoundRect(), rGradient );
    xImplementation->pop();
#if defined USE_JAVA && defined MACOSX
    xImplementation->setInOuterFace( bOldInOuterFace );
#endif	// USE_JAVA && MACOSX
}

void PDFWriter::DrawWallpaper( const tools::Rectangle& rRect, const Wallpaper& rWallpaper )
{
#if defined USE_JAVA && defined MACOSX
    bool bOldInOuterFace = xImplementation->isInOuterFace();
    if ( !bOldInOuterFace && !xImplementation->isReplayWriter() )
        xImplementation->addAction( new MetaWallpaperAction( rRect, rWallpaper ) );
    xImplementation->setInOuterFace( true );
#endif	// USE_JAVA && MACOSX
    xImplementation->drawWallpaper( rRect, rWallpaper );
#if defined USE_JAVA && defined MACOSX
    xImplementation->setInOuterFace( bOldInOuterFace );
#endif	// USE_JAVA && MACOSX
}

void PDFWriter::DrawTransparent( const tools::PolyPolygon& rPolyPoly, sal_uInt16 nTransparencePercent )
{
#if defined USE_JAVA && defined MACOSX
    bool bOldInOuterFace = xImplementation->isInOuterFace();
    if ( !bOldInOuterFace && !xImplementation->isReplayWriter() )
        xImplementation->addAction( new MetaTransparentAction( rPolyPoly, nTransparencePercent ) );
    xImplementation->setInOuterFace( true );
#endif	// USE_JAVA && MACOSX
    xImplementation->drawTransparent( rPolyPoly, nTransparencePercent );
#if defined USE_JAVA && defined MACOSX
    xImplementation->setInOuterFace( bOldInOuterFace );
#endif	// USE_JAVA && MACOSX
}

void PDFWriter::BeginTransparencyGroup()
{
#if defined USE_JAVA && defined MACOSX
    bool bOldInOuterFace = xImplementation->isInOuterFace();
    if ( !bOldInOuterFace && !xImplementation->isReplayWriter() )
        xImplementation->addAction( new MetaBeginTransparencyGroupPDFAction() );
    xImplementation->setInOuterFace( true );
#endif	// USE_JAVA && MACOSX
    xImplementation->beginTransparencyGroup();
#if defined USE_JAVA && defined MACOSX
    xImplementation->setInOuterFace( bOldInOuterFace );
#endif	// USE_JAVA && MACOSX
}

void PDFWriter::EndTransparencyGroup( const tools::Rectangle& rRect, sal_uInt16 nTransparentPercent )
{
#if defined USE_JAVA && defined MACOSX
    bool bOldInOuterFace = xImplementation->isInOuterFace();
    if ( !bOldInOuterFace && !xImplementation->isReplayWriter() )
        xImplementation->addAction( new MetaEndTransparencyGroupPDFAction( rRect, nTransparentPercent ) );
    xImplementation->setInOuterFace( true );
#endif	// USE_JAVA && MACOSX
    xImplementation->endTransparencyGroup( rRect, nTransparentPercent );
#if defined USE_JAVA && defined MACOSX
    xImplementation->setInOuterFace( bOldInOuterFace );
#endif	// USE_JAVA && MACOSX
}

void PDFWriter::Push( PushFlags nFlags )
{
#if defined USE_JAVA && defined MACOSX
    bool bOldInOuterFace = xImplementation->isInOuterFace();
    if ( !bOldInOuterFace && !xImplementation->isReplayWriter() )
        xImplementation->addAction( new MetaPushAction( nFlags ) );
    xImplementation->setInOuterFace( true );
#endif	// USE_JAVA && MACOSX
    xImplementation->push( nFlags );
#if defined USE_JAVA && defined MACOSX
    xImplementation->setInOuterFace( bOldInOuterFace );
#endif	// USE_JAVA && MACOSX
}

void PDFWriter::Pop()
{
#if defined USE_JAVA && defined MACOSX
    bool bOldInOuterFace = xImplementation->isInOuterFace();
    if ( !bOldInOuterFace && !xImplementation->isReplayWriter() )
        xImplementation->addAction( new MetaPopAction() ); 
    xImplementation->setInOuterFace( true );
#endif	// USE_JAVA && MACOSX
    xImplementation->pop();
#if defined USE_JAVA && defined MACOSX
    xImplementation->setInOuterFace( bOldInOuterFace );
#endif	// USE_JAVA && MACOSX
}

void PDFWriter::SetMapMode( const MapMode& rMapMode )
{
#if defined USE_JAVA && defined MACOSX
    bool bOldInOuterFace = xImplementation->isInOuterFace();
    if ( !bOldInOuterFace && !xImplementation->isReplayWriter() )
        xImplementation->addAction( new MetaMapModeAction( rMapMode ) );
    xImplementation->setInOuterFace( true );
#endif	// USE_JAVA && MACOSX
    xImplementation->setMapMode( rMapMode );
#if defined USE_JAVA && defined MACOSX
    xImplementation->setInOuterFace( bOldInOuterFace );
#endif	// USE_JAVA && MACOSX
}

void PDFWriter::SetLineColor( const Color& rColor )
{
#if defined USE_JAVA && defined MACOSX
    bool bOldInOuterFace = xImplementation->isInOuterFace();
    if ( !bOldInOuterFace && !xImplementation->isReplayWriter() )
        xImplementation->addAction( new MetaLineColorAction( rColor, true ) );
    xImplementation->setInOuterFace( true );
#endif	// USE_JAVA && MACOSX
    xImplementation->setLineColor( rColor );
#if defined USE_JAVA && defined MACOSX
    xImplementation->setInOuterFace( bOldInOuterFace );
#endif	// USE_JAVA && MACOSX
}

void PDFWriter::SetFillColor( const Color& rColor )
{
#if defined USE_JAVA && defined MACOSX
    bool bOldInOuterFace = xImplementation->isInOuterFace();
    if ( !bOldInOuterFace && !xImplementation->isReplayWriter() )
        xImplementation->addAction( new MetaFillColorAction( rColor, true ) );
    xImplementation->setInOuterFace( true );
#endif	// USE_JAVA && MACOSX
    xImplementation->setFillColor( rColor );
#if defined USE_JAVA && defined MACOSX
    xImplementation->setInOuterFace( bOldInOuterFace );
#endif	// USE_JAVA && MACOSX
}

void PDFWriter::SetClipRegion()
{
#if defined USE_JAVA && defined MACOSX
    bool bOldInOuterFace = xImplementation->isInOuterFace();
    if ( !bOldInOuterFace && !xImplementation->isReplayWriter() )
        xImplementation->addAction( new MetaClipRegionAction( Region(), false ) );
    xImplementation->setInOuterFace( true );
#endif	// USE_JAVA && MACOSX
    xImplementation->clearClipRegion();
#if defined USE_JAVA && defined MACOSX
    xImplementation->setInOuterFace( bOldInOuterFace );
#endif	// USE_JAVA && MACOSX
}

void PDFWriter::SetClipRegion( const basegfx::B2DPolyPolygon& rRegion )
{
#if defined USE_JAVA && defined MACOSX
    bool bOldInOuterFace = xImplementation->isInOuterFace();
    if ( !bOldInOuterFace && !xImplementation->isReplayWriter() )
        xImplementation->addAction( new MetaClipRegionAction( Region( rRegion ), true ) );
    xImplementation->setInOuterFace( true );
#endif	// USE_JAVA && MACOSX
    xImplementation->setClipRegion( rRegion );
#if defined USE_JAVA && defined MACOSX
    xImplementation->setInOuterFace( bOldInOuterFace );
#endif	// USE_JAVA && MACOSX
}

void PDFWriter::MoveClipRegion( long nHorzMove, long nVertMove )
{
#if defined USE_JAVA && defined MACOSX
    bool bOldInOuterFace = xImplementation->isInOuterFace();
    if ( !bOldInOuterFace && !xImplementation->isReplayWriter() )
        xImplementation->addAction( new MetaMoveClipRegionAction( nHorzMove, nVertMove ) );
    xImplementation->setInOuterFace( true );
#endif	// USE_JAVA && MACOSX
    xImplementation->moveClipRegion( nHorzMove, nVertMove );
#if defined USE_JAVA && defined MACOSX
    xImplementation->setInOuterFace( bOldInOuterFace );
#endif	// USE_JAVA && MACOSX
}

void PDFWriter::IntersectClipRegion( const basegfx::B2DPolyPolygon& rRegion )
{
#if defined USE_JAVA && defined MACOSX
    bool bOldInOuterFace = xImplementation->isInOuterFace();
    if ( !bOldInOuterFace && !xImplementation->isReplayWriter() )
        xImplementation->addAction( new MetaISectRegionClipRegionAction( Region( rRegion ) ) );
    xImplementation->setInOuterFace( true );
#endif	// USE_JAVA && MACOSX
    xImplementation->intersectClipRegion( rRegion );
#if defined USE_JAVA && defined MACOSX
    xImplementation->setInOuterFace( bOldInOuterFace );
#endif	// USE_JAVA && MACOSX
}

void PDFWriter::IntersectClipRegion( const tools::Rectangle& rRect )
{
#if defined USE_JAVA && defined MACOSX
    bool bOldInOuterFace = xImplementation->isInOuterFace();
    if ( !bOldInOuterFace && !xImplementation->isReplayWriter() )
        xImplementation->addAction( new MetaISectRectClipRegionAction( rRect ) );
    xImplementation->setInOuterFace( true );
#endif	// USE_JAVA && MACOSX
    xImplementation->intersectClipRegion( rRect );
#if defined USE_JAVA && defined MACOSX
    xImplementation->setInOuterFace( bOldInOuterFace );
#endif	// USE_JAVA && MACOSX
}

void PDFWriter::SetLayoutMode( ComplexTextLayoutFlags nMode )
{
#if defined USE_JAVA && defined MACOSX
    bool bOldInOuterFace = xImplementation->isInOuterFace();
    if ( !bOldInOuterFace && !xImplementation->isReplayWriter() )
        xImplementation->addAction( new MetaLayoutModeAction( nMode ) );
    xImplementation->setInOuterFace( true );
#endif	// USE_JAVA && MACOSX
    xImplementation->setLayoutMode( nMode );
#if defined USE_JAVA && defined MACOSX
    xImplementation->setInOuterFace( bOldInOuterFace );
#endif	// USE_JAVA && MACOSX
}

void PDFWriter::SetDigitLanguage( LanguageType eLang )
{
#if defined USE_JAVA && defined MACOSX
    bool bOldInOuterFace = xImplementation->isInOuterFace();
    if ( !bOldInOuterFace && !xImplementation->isReplayWriter() )
        xImplementation->addAction( new MetaDigitLanguagePDFAction( eLang ) );
    xImplementation->setInOuterFace( true );
#endif	// USE_JAVA && MACOSX
    xImplementation->setDigitLanguage( eLang );
#if defined USE_JAVA && defined MACOSX
    xImplementation->setInOuterFace( bOldInOuterFace );
#endif	// USE_JAVA && MACOSX
}

void PDFWriter::SetTextColor( const Color& rColor )
{
#if defined USE_JAVA && defined MACOSX
    bool bOldInOuterFace = xImplementation->isInOuterFace();
    if ( !bOldInOuterFace && !xImplementation->isReplayWriter() )
        xImplementation->addAction( new MetaTextColorAction( rColor ) );
    xImplementation->setInOuterFace( true );
#endif	// USE_JAVA && MACOSX
    xImplementation->setTextColor( rColor );
#if defined USE_JAVA && defined MACOSX
    xImplementation->setInOuterFace( bOldInOuterFace );
#endif	// USE_JAVA && MACOSX
}

void PDFWriter::SetTextFillColor()
{
#if defined USE_JAVA && defined MACOSX
    bool bOldInOuterFace = xImplementation->isInOuterFace();
    if ( !bOldInOuterFace && !xImplementation->isReplayWriter() )
        xImplementation->addAction( new MetaTextFillColorAction( Color( COL_TRANSPARENT ), false ) );
    xImplementation->setInOuterFace( true );
#endif	// USE_JAVA && MACOSX
    xImplementation->setTextFillColor();
#if defined USE_JAVA && defined MACOSX
    xImplementation->setInOuterFace( bOldInOuterFace );
#endif	// USE_JAVA && MACOSX
}

void PDFWriter::SetTextFillColor( const Color& rColor )
{
#if defined USE_JAVA && defined MACOSX
    bool bOldInOuterFace = xImplementation->isInOuterFace();
    if ( !bOldInOuterFace && !xImplementation->isReplayWriter() )
        xImplementation->addAction( new MetaTextFillColorAction( rColor, true ) );
    xImplementation->setInOuterFace( true );
#endif	// USE_JAVA && MACOSX
    xImplementation->setTextFillColor( rColor );
#if defined USE_JAVA && defined MACOSX
    xImplementation->setInOuterFace( bOldInOuterFace );
#endif	// USE_JAVA && MACOSX
}

void PDFWriter::SetTextLineColor()
{
#if defined USE_JAVA && defined MACOSX
    bool bOldInOuterFace = xImplementation->isInOuterFace();
    if ( !bOldInOuterFace && !xImplementation->isReplayWriter() )
        xImplementation->addAction( new MetaTextLineColorAction( Color( COL_TRANSPARENT ), false ) );
    xImplementation->setInOuterFace( true );
#endif	// USE_JAVA && MACOSX
    xImplementation->setTextLineColor();
#if defined USE_JAVA && defined MACOSX
    xImplementation->setInOuterFace( bOldInOuterFace );
#endif	// USE_JAVA && MACOSX
}

void PDFWriter::SetTextLineColor( const Color& rColor )
{
#if defined USE_JAVA && defined MACOSX
    bool bOldInOuterFace = xImplementation->isInOuterFace();
    if ( !bOldInOuterFace && !xImplementation->isReplayWriter() )
        xImplementation->addAction( new MetaTextLineColorAction( rColor, true ) );
    xImplementation->setInOuterFace( true );
#endif	// USE_JAVA && MACOSX
    xImplementation->setTextLineColor( rColor );
#if defined USE_JAVA && defined MACOSX
    xImplementation->setInOuterFace( bOldInOuterFace );
#endif	// USE_JAVA && MACOSX
}

void PDFWriter::SetOverlineColor()
{
#if defined USE_JAVA && defined MACOSX
    bool bOldInOuterFace = xImplementation->isInOuterFace();
    if ( !bOldInOuterFace && !xImplementation->isReplayWriter() )
        xImplementation->addAction( new MetaOverlineColorAction( Color( COL_TRANSPARENT ), false ) );
    xImplementation->setInOuterFace( true );
#endif	// USE_JAVA && MACOSX
    xImplementation->setOverlineColor();
#if defined USE_JAVA && defined MACOSX
    xImplementation->setInOuterFace( bOldInOuterFace );
#endif	// USE_JAVA && MACOSX
}

void PDFWriter::SetOverlineColor( const Color& rColor )
{
#if defined USE_JAVA && defined MACOSX
    bool bOldInOuterFace = xImplementation->isInOuterFace();
    if ( !bOldInOuterFace && !xImplementation->isReplayWriter() )
        xImplementation->addAction( new MetaOverlineColorAction( rColor, true ) );
    xImplementation->setInOuterFace( true );
#endif	// USE_JAVA && MACOSX
    xImplementation->setOverlineColor( rColor );
#if defined USE_JAVA && defined MACOSX
    xImplementation->setInOuterFace( bOldInOuterFace );
#endif	// USE_JAVA && MACOSX
}

void PDFWriter::SetTextAlign( ::TextAlign eAlign )
{
#if defined USE_JAVA && defined MACOSX
    bool bOldInOuterFace = xImplementation->isInOuterFace();
    if ( !bOldInOuterFace && !xImplementation->isReplayWriter() )
        xImplementation->addAction( new MetaTextAlignAction( eAlign ) );
    xImplementation->setInOuterFace( true );
#endif	// USE_JAVA && MACOSX
    xImplementation->setTextAlign( eAlign );
#if defined USE_JAVA && defined MACOSX
    xImplementation->setInOuterFace( bOldInOuterFace );
#endif	// USE_JAVA && MACOSX
}

void PDFWriter::DrawJPGBitmap( SvStream& rStreamData, bool bIsTrueColor, const Size& rSrcSizePixel, const tools::Rectangle& rTargetArea, const Bitmap& rMask, const Graphic& rGraphic )
{
#if defined USE_JAVA && defined MACOSX
    bool bOldInOuterFace = xImplementation->isInOuterFace();
    if ( !bOldInOuterFace && !xImplementation->isReplayWriter() )
        xImplementation->addAction( new MetaJpgPDFAction( rStreamData, bIsTrueColor, rSrcSizePixel, rTargetArea, rMask, rGraphic ) );
    xImplementation->setInOuterFace( true );
#endif	// USE_JAVA && MACOSX
    xImplementation->drawJPGBitmap( rStreamData, bIsTrueColor, rSrcSizePixel, rTargetArea, rMask, rGraphic );
#if defined USE_JAVA && defined MACOSX
    xImplementation->setInOuterFace( bOldInOuterFace );
#endif	// USE_JAVA && MACOSX
}

sal_Int32 PDFWriter::CreateLink( const tools::Rectangle& rRect, sal_Int32 nPageNr )
{
#if defined USE_JAVA && defined MACOSX
    bool bOldInOuterFace = xImplementation->isInOuterFace();
    if ( !bOldInOuterFace && !xImplementation->isReplayWriter() )
        xImplementation->addAction( new MetaCreateLinkPDFAction( rRect, nPageNr ) );
    xImplementation->setInOuterFace( true );
    sal_Int32 nRet = xImplementation->createLink( rRect, nPageNr );
    xImplementation->setInOuterFace( bOldInOuterFace );
    return nRet;
#else	// USE_JAVA && MACOSX
    return xImplementation->createLink( rRect, nPageNr );
#endif	// USE_JAVA && MACOSX
}

sal_Int32 PDFWriter::CreateScreen(const tools::Rectangle& rRect, sal_Int32 nPageNr)
{
#if defined USE_JAVA && defined MACOSX
    bool bOldInOuterFace = xImplementation->isInOuterFace();
    if ( !bOldInOuterFace && !xImplementation->isReplayWriter() )
        xImplementation->addAction( new MetaCreateScreenPDFAction( rRect, nPageNr ) );
    xImplementation->setInOuterFace( true );
    sal_Int32 nRet = xImplementation->createScreen(rRect, nPageNr);
    xImplementation->setInOuterFace( bOldInOuterFace );
    return nRet;
#else	// USE_JAVA && MACOSX
    return xImplementation->createScreen(rRect, nPageNr);
#endif	// USE_JAVA && MACOSX
}

sal_Int32 PDFWriter::RegisterDestReference( sal_Int32 nDestId, const tools::Rectangle& rRect, sal_Int32 nPageNr, DestAreaType eType )
{
#if defined USE_JAVA && defined MACOSX
    bool bOldInOuterFace = xImplementation->isInOuterFace();
    if ( !bOldInOuterFace && !xImplementation->isReplayWriter() )
        xImplementation->addAction( new MetaRegisterDestReferencePDFAction( nDestId, rRect, nPageNr, eType ) );
    xImplementation->setInOuterFace( true );
    sal_Int32 nRet = xImplementation->registerDestReference( nDestId, rRect, nPageNr, eType );
    xImplementation->setInOuterFace( bOldInOuterFace );
    return nRet;
#else	// USE_JAVA && MACOSX
    return xImplementation->registerDestReference( nDestId, rRect, nPageNr, eType );
#endif	// USE_JAVA && MACOSX
}
//--->i56629
sal_Int32 PDFWriter::CreateNamedDest( const OUString& sDestName, const tools::Rectangle& rRect, sal_Int32 nPageNr, PDFWriter::DestAreaType eType )
{
#if defined USE_JAVA && defined MACOSX
    bool bOldInOuterFace = xImplementation->isInOuterFace();
    if ( !bOldInOuterFace && !xImplementation->isReplayWriter() )
        xImplementation->addAction( new MetaCreateNamedDestPDFAction( sDestName, rRect, nPageNr, eType ) );
    xImplementation->setInOuterFace( true );
    sal_Int32 nRet = xImplementation->createNamedDest( sDestName, rRect, nPageNr, eType );
    xImplementation->setInOuterFace( bOldInOuterFace );
    return nRet;
#else	// USE_JAVA && MACOSX
    return xImplementation->createNamedDest( sDestName, rRect, nPageNr, eType );
#endif	// USE_JAVA && MACOSX
}
sal_Int32 PDFWriter::CreateDest( const tools::Rectangle& rRect, sal_Int32 nPageNr, PDFWriter::DestAreaType eType )
{
#if defined USE_JAVA && defined MACOSX
    bool bOldInOuterFace = xImplementation->isInOuterFace();
    if ( !bOldInOuterFace && !xImplementation->isReplayWriter() )
        xImplementation->addAction( new MetaCreateDestPDFAction( rRect, nPageNr, eType ) );
    xImplementation->setInOuterFace( true );
    sal_Int32 nRet = xImplementation->createDest( rRect, nPageNr, eType );
    xImplementation->setInOuterFace( bOldInOuterFace );
    return nRet;
#else	// USE_JAVA && MACOSX
    return xImplementation->createDest( rRect, nPageNr, eType );
#endif	// USE_JAVA && MACOSX
}

void PDFWriter::SetLinkDest( sal_Int32 nLinkId, sal_Int32 nDestId )
{
#if defined USE_JAVA && defined MACOSX
    bool bOldInOuterFace = xImplementation->isInOuterFace();
    if ( !bOldInOuterFace && !xImplementation->isReplayWriter() )
        xImplementation->addAction( new MetaSetLinkDestPDFAction( nLinkId, nDestId ) );
    xImplementation->setInOuterFace( true );
#endif	// USE_JAVA && MACOSX
    xImplementation->setLinkDest( nLinkId, nDestId );
#if defined USE_JAVA && defined MACOSX
    xImplementation->setInOuterFace( bOldInOuterFace );
#endif	// USE_JAVA && MACOSX
}

void PDFWriter::SetLinkURL( sal_Int32 nLinkId, const OUString& rURL )
{
#if defined USE_JAVA && defined MACOSX
    bool bOldInOuterFace = xImplementation->isInOuterFace();
    if ( !bOldInOuterFace && !xImplementation->isReplayWriter() )
        xImplementation->addAction( new MetaSetLinkUrlPDFAction( nLinkId, rURL ) );
    xImplementation->setInOuterFace( true );
#endif	// USE_JAVA && MACOSX
    xImplementation->setLinkURL( nLinkId, rURL );
#if defined USE_JAVA && defined MACOSX
    xImplementation->setInOuterFace( bOldInOuterFace );
#endif	// USE_JAVA && MACOSX
}

void PDFWriter::SetScreenURL(sal_Int32 nScreenId, const OUString& rURL)
{
#if defined USE_JAVA && defined MACOSX
    bool bOldInOuterFace = xImplementation->isInOuterFace();
    if ( !bOldInOuterFace && !xImplementation->isReplayWriter() )
        xImplementation->addAction( new MetaSetScreenStreamPDFAction( nScreenId, rURL ) );
    xImplementation->setInOuterFace( true );
#endif	// USE_JAVA && MACOSX
    xImplementation->setScreenURL(nScreenId, rURL);
#if defined USE_JAVA && defined MACOSX
    xImplementation->setInOuterFace( bOldInOuterFace );
#endif	// USE_JAVA && MACOSX
}

void PDFWriter::SetScreenStream(sal_Int32 nScreenId, const OUString& rURL)
{
#if defined USE_JAVA && defined MACOSX
    bool bOldInOuterFace = xImplementation->isInOuterFace();
    if ( !bOldInOuterFace && !xImplementation->isReplayWriter() )
        xImplementation->addAction( new MetaSetScreenStreamPDFAction( nScreenId, rURL ) );
    xImplementation->setInOuterFace( true );
#endif	// USE_JAVA && MACOSX
    xImplementation->setScreenStream(nScreenId, rURL);
#if defined USE_JAVA && defined MACOSX
    xImplementation->setInOuterFace( bOldInOuterFace );
#endif	// USE_JAVA && MACOSX
}

void PDFWriter::SetLinkPropertyID( sal_Int32 nLinkId, sal_Int32 nPropertyId )
{
#if defined USE_JAVA && defined MACOSX
    bool bOldInOuterFace = xImplementation->isInOuterFace();
    if ( !bOldInOuterFace && !xImplementation->isReplayWriter() )
        xImplementation->addAction( new MetaSetLinkPropertyIdPDFAction( nLinkId, nPropertyId ) );
    xImplementation->setInOuterFace( true );
#endif	// USE_JAVA && MACOSX
    xImplementation->setLinkPropertyId( nLinkId, nPropertyId );
#if defined USE_JAVA && defined MACOSX
    xImplementation->setInOuterFace( bOldInOuterFace );
#endif	// USE_JAVA && MACOSX
}

sal_Int32 PDFWriter::CreateOutlineItem( sal_Int32 nParent, const OUString& rText, sal_Int32 nDestID )
{
#if defined USE_JAVA && defined MACOSX
    bool bOldInOuterFace = xImplementation->isInOuterFace();
    if ( !bOldInOuterFace && !xImplementation->isReplayWriter() )
        xImplementation->addAction( new MetaCreateOutlineItemPDFAction( nParent, rText, nDestID ) );
    xImplementation->setInOuterFace( true );
    sal_Int32 nRet = xImplementation->createOutlineItem( nParent, rText, nDestID );
    xImplementation->setInOuterFace( bOldInOuterFace );
    return nRet;
#else	// USE_JAVA && MACOSX
    return xImplementation->createOutlineItem( nParent, rText, nDestID );
#endif	// USE_JAVA && MACOSX
}

void PDFWriter::CreateNote( const tools::Rectangle& rRect, const PDFNote& rNote, sal_Int32 nPageNr )
{
#if defined USE_JAVA && defined MACOSX
    bool bOldInOuterFace = xImplementation->isInOuterFace();
    if ( !bOldInOuterFace && !xImplementation->isReplayWriter() )
        xImplementation->addAction( new MetaCreateNotePDFAction( rRect, rNote, nPageNr ) );
    xImplementation->setInOuterFace( true );
#endif	// USE_JAVA && MACOSX
    xImplementation->createNote( rRect, rNote, nPageNr );
#if defined USE_JAVA && defined MACOSX
    xImplementation->setInOuterFace( bOldInOuterFace );
#endif	// USE_JAVA && MACOSX
}

sal_Int32 PDFWriter::BeginStructureElement( PDFWriter::StructElement eType, const OUString& rAlias )
{
#if defined USE_JAVA && defined MACOSX
    bool bOldInOuterFace = xImplementation->isInOuterFace();
    if ( !bOldInOuterFace && !xImplementation->isReplayWriter() )
        xImplementation->addAction( new MetaBeginStructureElementPDFAction( eType, rAlias ) );
    xImplementation->setInOuterFace( true );
    sal_Int32 nRet = xImplementation->beginStructureElement( eType, rAlias );
    xImplementation->setInOuterFace( bOldInOuterFace );
    return nRet;
#else	// USE_JAVA && MACOSX
    return xImplementation->beginStructureElement( eType, rAlias );
#endif	// USE_JAVA && MACOSX
}

void PDFWriter::EndStructureElement()
{
#if defined USE_JAVA && defined MACOSX
    bool bOldInOuterFace = xImplementation->isInOuterFace();
    if ( !bOldInOuterFace && !xImplementation->isReplayWriter() )
        xImplementation->addAction( new MetaEndStructureElementPDFAction() );
    xImplementation->setInOuterFace( true );
#endif	// USE_JAVA && MACOSX
    xImplementation->endStructureElement();
#if defined USE_JAVA && defined MACOSX
    xImplementation->setInOuterFace( bOldInOuterFace );
#endif	// USE_JAVA && MACOSX
}

bool PDFWriter::SetCurrentStructureElement( sal_Int32 nID )
{
#if defined USE_JAVA && defined MACOSX
    bool bOldInOuterFace = xImplementation->isInOuterFace();
    if ( !bOldInOuterFace && !xImplementation->isReplayWriter() )
        xImplementation->addAction( new MetaSetCurrentStructureElementPDFAction( nID ) );
    xImplementation->setInOuterFace( true );
    bool bRet = xImplementation->setCurrentStructureElement( nID );
    xImplementation->setInOuterFace( bOldInOuterFace );
    return bRet;
#else	// USE_JAVA && MACOSX
    return xImplementation->setCurrentStructureElement( nID );
#endif	// USE_JAVA && MACOSX
}

bool PDFWriter::SetStructureAttribute( enum StructAttribute eAttr, enum StructAttributeValue eVal )
{
#if defined USE_JAVA && defined MACOSX
    bool bOldInOuterFace = xImplementation->isInOuterFace();
    if ( !bOldInOuterFace && !xImplementation->isReplayWriter() )
        xImplementation->addAction( new MetaSetStructureAttributePDFAction( eAttr, eVal ) );          
    xImplementation->setInOuterFace( true );
    bool bRet = xImplementation->setStructureAttribute( eAttr, eVal );
    xImplementation->setInOuterFace( bOldInOuterFace );
    return bRet;
#else	// USE_JAVA && MACOSX
    return xImplementation->setStructureAttribute( eAttr, eVal );
#endif	// USE_JAVA && MACOSX
}

bool PDFWriter::SetStructureAttributeNumerical( enum StructAttribute eAttr, sal_Int32 nValue )
{
#if defined USE_JAVA && defined MACOSX
    bool bOldInOuterFace = xImplementation->isInOuterFace();
    if ( !bOldInOuterFace && !xImplementation->isReplayWriter() )
        xImplementation->addAction( new MetaSetStructureAttributeNumericalPDFAction( eAttr, nValue ) );
    xImplementation->setInOuterFace( true );
    bool bRet = xImplementation->setStructureAttributeNumerical( eAttr, nValue );
    xImplementation->setInOuterFace( bOldInOuterFace );
    return bRet;
#else	// USE_JAVA && MACOSX
    return xImplementation->setStructureAttributeNumerical( eAttr, nValue );
#endif	// USE_JAVA && MACOSX
}

void PDFWriter::SetStructureBoundingBox( const tools::Rectangle& rRect )
{
#if defined USE_JAVA && defined MACOSX
    bool bOldInOuterFace = xImplementation->isInOuterFace();
    if ( !bOldInOuterFace && !xImplementation->isReplayWriter() )
        xImplementation->addAction( new MetaSetStructureBoundingBoxPDFAction( rRect ) );
    xImplementation->setInOuterFace( true );
#endif	// USE_JAVA && MACOSX
    xImplementation->setStructureBoundingBox( rRect );
#if defined USE_JAVA && defined MACOSX
    xImplementation->setInOuterFace( bOldInOuterFace );
#endif	// USE_JAVA && MACOSX
}

void PDFWriter::SetActualText( const OUString& rText )
{
#if defined USE_JAVA && defined MACOSX
    bool bOldInOuterFace = xImplementation->isInOuterFace();
    if ( !bOldInOuterFace && !xImplementation->isReplayWriter() )
        xImplementation->addAction( new MetaSetActualTextPDFAction( rText ) );
    xImplementation->setInOuterFace( true );
#endif	// USE_JAVA && MACOSX
    xImplementation->setActualText( rText );
#if defined USE_JAVA && defined MACOSX
    xImplementation->setInOuterFace( bOldInOuterFace );
#endif	// USE_JAVA && MACOSX
}

void PDFWriter::SetAlternateText( const OUString& rText )
{
#if defined USE_JAVA && defined MACOSX
    bool bOldInOuterFace = xImplementation->isInOuterFace();
    if ( !bOldInOuterFace && !xImplementation->isReplayWriter() )
        xImplementation->addAction( new MetaSetAlternateTextPDFAction( rText ) );
    xImplementation->setInOuterFace( true );
#endif	// USE_JAVA && MACOSX
    xImplementation->setAlternateText( rText );
#if defined USE_JAVA && defined MACOSX
    xImplementation->setInOuterFace( bOldInOuterFace );
#endif	// USE_JAVA && MACOSX
}

void PDFWriter::SetPageTransition( PDFWriter::PageTransition eType, sal_uInt32 nMilliSec, sal_Int32 nPageNr )
{
#if defined USE_JAVA && defined MACOSX
    bool bOldInOuterFace = xImplementation->isInOuterFace();
    if ( !bOldInOuterFace && !xImplementation->isReplayWriter() )
        xImplementation->addAction( new MetaSetPageTransitionPDFAction( eType, nMilliSec, nPageNr ) );
    xImplementation->setInOuterFace( true );
#endif	// USE_JAVA && MACOSX
    xImplementation->setPageTransition( eType, nMilliSec, nPageNr );
#if defined USE_JAVA && defined MACOSX
    xImplementation->setInOuterFace( bOldInOuterFace );
#endif	// USE_JAVA && MACOSX
}

sal_Int32 PDFWriter::CreateControl( const PDFWriter::AnyWidget& rControl )
{
#if defined USE_JAVA && defined MACOSX
    bool bOldInOuterFace = xImplementation->isInOuterFace();
    if ( !bOldInOuterFace && !xImplementation->isReplayWriter() )
        xImplementation->addAction( new MetaCreateControlPDFAction( rControl ) );
    xImplementation->setInOuterFace( true );
    sal_Int32 nRet = xImplementation->createControl( rControl );
    xImplementation->setInOuterFace( bOldInOuterFace );
    return nRet;
#else	// USE_JAVA && MACOSX
    return xImplementation->createControl( rControl );
#endif	// USE_JAVA && MACOSX
}

PDFOutputStream::~PDFOutputStream()
{
}

void PDFWriter::AddStream( const OUString& rMimeType, PDFOutputStream* pStream )
{
#if defined USE_JAVA && defined MACOSX
    bool bOldInOuterFace = xImplementation->isInOuterFace();
    if ( !bOldInOuterFace && !xImplementation->isReplayWriter() )
        xImplementation->addAction( new MetaAddStreamPDFAction( rMimeType, pStream ) );
    xImplementation->setInOuterFace( true );
#endif	// USE_JAVA && MACOSX
    xImplementation->addStream( rMimeType, pStream );
#if defined USE_JAVA && defined MACOSX
    xImplementation->setInOuterFace( bOldInOuterFace );
#endif	// USE_JAVA && MACOSX
}

std::set< PDFWriter::ErrorCode > PDFWriter::GetErrors()
{
    return xImplementation->getErrors();
}

css::uno::Reference< css::beans::XMaterialHolder >
PDFWriter::InitEncryption( const OUString& i_rOwnerPassword,
                           const OUString& i_rUserPassword,
                           bool b128Bit
                          )
{
    return PDFWriterImpl::initEncryption( i_rOwnerPassword, i_rUserPassword, b128Bit );
}

void PDFWriter::PlayMetafile( const GDIMetaFile& i_rMTF, const vcl::PDFWriter::PlayMetafileContext& i_rPlayContext, PDFExtOutDevData* i_pData )
{
#if defined USE_JAVA && defined MACOSX
    bool bOldInOuterFace = xImplementation->isInOuterFace();
    if ( !bOldInOuterFace && !xImplementation->isReplayWriter() )
        xImplementation->addAction( new MetaPlayMetafilePDFAction( i_rMTF, i_rPlayContext, i_pData ) );
    xImplementation->setInOuterFace( true );
#endif	// USE_JAVA && MACOSX
    xImplementation->playMetafile( i_rMTF, i_pData, i_rPlayContext );
#if defined USE_JAVA && defined MACOSX
    xImplementation->setInOuterFace( bOldInOuterFace );
#endif	// USE_JAVA && MACOSX
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
