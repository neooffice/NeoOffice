/*************************************************************************
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * $RCSfile$
 * $Revision$
 *
 * This file is part of NeoOffice.
 *
 * NeoOffice is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * NeoOffice is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU General Public License
 * version 3 along with NeoOffice.  If not, see
 * <http://www.gnu.org/licenses/gpl-3.0.txt>
 * for a copy of the GPLv3 License.
 *
 * Modified February 2006 by Patrick Luby. NeoOffice is distributed under
 * GPL only under modification term 2 of the LGPL.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#define _USE_MATH_DEFINES
#include <math.h>
#include <algorithm>

#include <pdfwriter_impl.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <rtl/ustrbuf.hxx>
#include <tools/debug.hxx>
#include <tools/zcodec.hxx>
#include <tools/stream.hxx>
#include <tools/urlobj.hxx> //for relative url
#include <i18npool/mslangid.hxx>
#include <vcl/virdev.hxx>
#include <vcl/bmpacc.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/image.hxx>
#include <vcl/outdev.h>
#include <vcl/sallayout.hxx>
#include <vcl/metric.hxx>
#include <svsys.h>
#include <vcl/salgdi.hxx>
#include <vcl/svapp.hxx>
#include <osl/thread.h>
#include <osl/file.h>
#include <rtl/crc.h>
#include <rtl/digest.h>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/util/URL.hpp>

#include "implncvt.hxx"

#include "cppuhelper/implbase1.hxx"
#include <icc/sRGB-IEC61966-2.1.hxx>

#if defined USE_JAVA && defined MACOSX

#ifndef _SV_SALATSLAYOUT_HXX
#include <salatslayout.hxx>
#endif
#ifndef _SV_SALDATA_HXX
#include <saldata.hxx>
#endif

#include <unotools/tempfile.hxx>

#endif	// USE_JAVA && MACOSX

using namespace vcl;
using namespace rtl;

#if (OSL_DEBUG_LEVEL < 2)
#define COMPRESS_PAGES
#else
#define DEBUG_DISABLE_PDFCOMPRESSION // also do not compress streams
#endif

#ifdef DO_TEST_PDF
class PDFTestOutputStream : public PDFOutputStream
{
    public:
    virtual ~PDFTestOutputStream();
    virtual void write( const com::sun::star::uno::Reference< com::sun::star::io::XOutputStream >& xStream );
};

PDFTestOutputStream::~PDFTestOutputStream()
{
}

void PDFTestOutputStream::write( const com::sun::star::uno::Reference< com::sun::star::io::XOutputStream >& xStream )
{
    OString aStr( "lalala\ntest\ntest\ntest" );
    com::sun::star::uno::Sequence< sal_Int8 > aData( aStr.getLength() );
    rtl_copyMemory( aData.getArray(), aStr.getStr(), aStr.getLength() );
    xStream->writeBytes( aData );
}

// this test code cannot be used to test PDF/A-1 because it forces
// control item (widgets) to bypass the structure controlling
// the embedding of such elements in actual run
void doTestCode()
{
    static const char* pHome = getenv( "HOME"  );
    rtl::OUString aTestFile( RTL_CONSTASCII_USTRINGPARAM( "file://" ) );
    aTestFile += rtl::OUString( pHome, strlen( pHome ), RTL_TEXTENCODING_MS_1252 );
    aTestFile += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "/pdf_export_test.pdf" ) );

    PDFWriter::PDFWriterContext aContext;
    aContext.URL			= aTestFile;
    aContext.Version		= PDFWriter::PDF_1_4;
    aContext.Tagged			= true;
    aContext.InitialPage    = 2;

    PDFWriter aWriter( aContext );
    PDFDocInfo aDocInfo;
    aDocInfo.Title = OUString( RTL_CONSTASCII_USTRINGPARAM( "PDF export test document" ) );
    aDocInfo.Producer = OUString( RTL_CONSTASCII_USTRINGPARAM( "VCL" ) );
    aWriter.SetDocInfo( aDocInfo );
    aWriter.NewPage();
    aWriter.BeginStructureElement( PDFWriter::Document );
    // set duration of 3 sec for first page
    aWriter.SetAutoAdvanceTime( 3 );
    aWriter.SetMapMode( MapMode( MAP_100TH_MM ) );

    aWriter.SetFillColor( Color( COL_LIGHTRED ) );
    aWriter.SetLineColor( Color( COL_LIGHTGREEN ) );
    aWriter.DrawRect( Rectangle( Point( 2000, 200 ), Size( 8000, 3000 ) ), 5000, 2000 );

    aWriter.SetFont( Font( String( RTL_CONSTASCII_USTRINGPARAM( "Times" ) ), Size( 0, 500 ) ) );
    aWriter.SetTextColor( Color( COL_BLACK ) );
    aWriter.SetLineColor( Color( COL_BLACK ) );
    aWriter.SetFillColor( Color( COL_LIGHTBLUE ) );

    Rectangle aRect( Point( 5000, 5000 ), Size( 6000, 3000 ) );
    aWriter.DrawRect( aRect );
    aWriter.DrawText( aRect, String( RTL_CONSTASCII_USTRINGPARAM( "Link annot 1" ) ) );
    sal_Int32 nFirstLink = aWriter.CreateLink( aRect );
    PDFNote aNote;
    aNote.Title = String( RTL_CONSTASCII_USTRINGPARAM( "A small test note" ) );
    aNote.Contents = String( RTL_CONSTASCII_USTRINGPARAM( "There is no business like show business like no business i know. Everything about it is appealing." ) );
    aWriter.CreateNote( Rectangle( Point( aRect.Right(), aRect.Top() ), Size( 6000, 3000 ) ), aNote );

    Rectangle aTargetRect( Point( 3000, 23000 ), Size( 12000, 6000 ) );
    aWriter.SetFillColor( Color( COL_LIGHTGREEN ) );
    aWriter.DrawRect( aTargetRect );
    aWriter.DrawText( aTargetRect, String( RTL_CONSTASCII_USTRINGPARAM( "Dest second link" ) ) );
    sal_Int32 nSecondDest = aWriter.CreateDest( aTargetRect );

    aWriter.BeginStructureElement( PDFWriter::Section );
    aWriter.BeginStructureElement( PDFWriter::Heading );
    aWriter.DrawText( Point(4500, 9000), String( RTL_CONSTASCII_USTRINGPARAM( "A small structure test" ) ) );
    aWriter.EndStructureElement();
    aWriter.BeginStructureElement( PDFWriter::Paragraph );
    aWriter.SetStructureAttribute( PDFWriter::WritingMode, PDFWriter::LrTb );
    aWriter.SetStructureAttribute( PDFWriter::TextDecorationType, PDFWriter::Underline );
    aWriter.DrawText( Rectangle( Point( 4500, 10000 ), Size( 12000, 6000 ) ),
                      String( RTL_CONSTASCII_USTRINGPARAM( "It was the best of PDF, it was the worst of PDF ... or so. This is a pretty nonsensical text to denote a paragraph. I suggest you stop reading it. Because if you read on you might get bored. So continue on your on risk. Hey, you're still here ? Why do you continue to read this as it is of no use at all ? OK, it's your time, but still... . Woah, i even get bored writing this, so let's end this here and now." ) ),
                      TEXT_DRAW_MULTILINE | TEXT_DRAW_WORDBREAK
                      );
    aWriter.SetActualText( String( RTL_CONSTASCII_USTRINGPARAM( "It was the best of PDF, it was the worst of PDF ... or so. This is a pretty nonsensical text to denote a paragraph. I suggest you stop reading it. Because if you read on you might get bored. So continue on your on risk. Hey, you're still here ? Why do you continue to read this as it is of no use at all ? OK, it's your time, but still... . Woah, i even get bored writing this, so let's end this here and now." ) ) );
    aWriter.SetAlternateText( String( RTL_CONSTASCII_USTRINGPARAM( "This paragraph contains some lengthy nonsense to test structural element emission of PDFWriter." ) ) );
    aWriter.EndStructureElement();
    sal_Int32 nLongPara = aWriter.BeginStructureElement( PDFWriter::Paragraph );
    aWriter.SetStructureAttribute( PDFWriter::WritingMode, PDFWriter::LrTb );
    aWriter.DrawText( Rectangle( Point( 4500, 19000 ), Size( 12000, 1000 ) ),
                      String( RTL_CONSTASCII_USTRINGPARAM( "This paragraph is nothing special either but ends on the next page structurewise" ) ),
                      TEXT_DRAW_MULTILINE | TEXT_DRAW_WORDBREAK
                      );

    aWriter.NewPage();
    // test AddStream interface
    aWriter.AddStream( String( RTL_CONSTASCII_USTRINGPARAM( "text/plain" ) ), new PDFTestOutputStream(), true );
    // set transitional mode
    aWriter.SetPageTransition( PDFWriter::WipeRightToLeft, 1500 );
    aWriter.SetMapMode( MapMode( MAP_100TH_MM ) );
    aWriter.SetTextColor( Color( COL_BLACK ) );
    aWriter.SetFont( Font( String( RTL_CONSTASCII_USTRINGPARAM( "Times" ) ), Size( 0, 500 ) ) );
    aWriter.DrawText( Rectangle( Point( 4500, 1500 ), Size( 12000, 3000 ) ),
                      String( RTL_CONSTASCII_USTRINGPARAM( "Here's where all things come to an end ... well at least the paragaph from the last page." ) ),
                      TEXT_DRAW_MULTILINE | TEXT_DRAW_WORDBREAK
                      );
    aWriter.EndStructureElement();

    aWriter.SetFillColor( Color( COL_LIGHTBLUE ) );
    // disable structure
    aWriter.BeginStructureElement( PDFWriter::NonStructElement );
    aWriter.DrawRect( aRect );
    aWriter.BeginStructureElement( PDFWriter::Paragraph );
    aWriter.DrawText( aRect, String( RTL_CONSTASCII_USTRINGPARAM( "Link annot 2" ) ) );
    sal_Int32 nSecondLink = aWriter.CreateLink( aRect );

    aWriter.SetFillColor( Color( COL_LIGHTGREEN ) );
    aWriter.BeginStructureElement( PDFWriter::ListItem );
    aWriter.DrawRect( aTargetRect );
    aWriter.DrawText( aTargetRect, String( RTL_CONSTASCII_USTRINGPARAM( "Dest first link" ) ) );
    sal_Int32 nFirstDest = aWriter.CreateDest( aTargetRect );
    // enable structure
    aWriter.EndStructureElement();
    // add something to the long paragraph as an afterthought
    sal_Int32 nSaveStruct = aWriter.GetCurrentStructureElement();
    aWriter.SetCurrentStructureElement( nLongPara );
    aWriter.DrawText( Rectangle( Point( 4500,4500 ),  Size( 12000, 1000 ) ),
                      String( RTL_CONSTASCII_USTRINGPARAM( "Add something to the longish paragraph above." ) ),
                      TEXT_DRAW_MULTILINE | TEXT_DRAW_WORDBREAK );
    aWriter.SetCurrentStructureElement( nSaveStruct );
    aWriter.EndStructureElement();
    aWriter.EndStructureElement();
    aWriter.BeginStructureElement( PDFWriter::Figure );
    aWriter.BeginStructureElement( PDFWriter::Caption );
    aWriter.DrawText( Point( 4500, 9000 ), String( RTL_CONSTASCII_USTRINGPARAM( "Some drawing stuff inside the structure" ) ) );
    aWriter.EndStructureElement();
    aWriter.DrawEllipse( Rectangle( Point( 4500, 9600 ), Size( 12000, 3000 ) ) );
    // test transparency
    // draw background
    Rectangle aTranspRect( Point( 7500, 13500 ), Size( 9000, 6000 ) );
    aWriter.SetFillColor( Color( COL_LIGHTRED ) );
    aWriter.DrawRect( aTranspRect );
    aWriter.BeginTransparencyGroup();

    aWriter.SetFillColor( Color( COL_LIGHTGREEN ) );
    aWriter.DrawEllipse( aTranspRect );
    aWriter.SetTextColor( Color( COL_LIGHTBLUE ) );
    aWriter.DrawText( aTranspRect,
                      String( RTL_CONSTASCII_USTRINGPARAM( "Some transparent text" ) ),
                      TEXT_DRAW_CENTER | TEXT_DRAW_VCENTER | TEXT_DRAW_MULTILINE | TEXT_DRAW_WORDBREAK );

    aWriter.EndTransparencyGroup( aTranspRect, 50 );

    // prepare an alpha mask
    Bitmap aTransMask( Size( 256, 256 ), 8, &Bitmap::GetGreyPalette( 256 ) );
    BitmapWriteAccess* pAcc = aTransMask.AcquireWriteAccess();
    for( int nX = 0; nX < 256; nX++ )
        for( int nY = 0; nY < 256; nY++ )
            pAcc->SetPixel( nX, nY, BitmapColor( (BYTE)((nX+nY)/2) ) );
    aTransMask.ReleaseAccess( pAcc );
    aTransMask.SetPrefMapMode( MAP_MM );
    aTransMask.SetPrefSize( Size( 10, 10 ) );

    aWriter.DrawBitmap( Point( 600, 13500 ), Size( 3000, 3000 ), aTransMask );

    aTranspRect = Rectangle( Point( 4200, 13500 ), Size( 3000, 3000 ) );
    aWriter.SetFillColor( Color( COL_LIGHTRED ) );
    aWriter.DrawRect( aTranspRect );
    aWriter.SetFillColor( Color( COL_LIGHTGREEN ) );
    aWriter.DrawEllipse( aTranspRect );
    aWriter.SetTextColor( Color( COL_LIGHTBLUE ) );
    aWriter.DrawText( aTranspRect,
                      String( RTL_CONSTASCII_USTRINGPARAM( "Some transparent text" ) ),
                      TEXT_DRAW_CENTER | TEXT_DRAW_VCENTER | TEXT_DRAW_MULTILINE | TEXT_DRAW_WORDBREAK );
    aTranspRect = Rectangle( Point( 1500, 16500 ), Size( 4800, 3000 ) );
    aWriter.SetFillColor( Color( COL_LIGHTRED ) );
    aWriter.DrawRect( aTranspRect );
    aWriter.BeginTransparencyGroup();
    aWriter.SetFillColor( Color( COL_LIGHTGREEN ) );
    aWriter.DrawEllipse( aTranspRect );
    aWriter.SetTextColor( Color( COL_LIGHTBLUE ) );
    aWriter.DrawText( aTranspRect,
                      String( RTL_CONSTASCII_USTRINGPARAM( "Some transparent text" ) ),
                      TEXT_DRAW_CENTER | TEXT_DRAW_VCENTER | TEXT_DRAW_MULTILINE | TEXT_DRAW_WORDBREAK );
    aWriter.EndTransparencyGroup( aTranspRect, aTransMask );

    Bitmap aImageBmp( Size( 256, 256 ), 24 );
    pAcc = aImageBmp.AcquireWriteAccess();
    pAcc->SetFillColor( Color( 0xff, 0, 0xff ) );
    pAcc->FillRect( Rectangle( Point( 0, 0 ), Size( 256, 256 ) ) );
    aImageBmp.ReleaseAccess( pAcc );
    BitmapEx aBmpEx( aImageBmp, AlphaMask( aTransMask ) );
    aWriter.DrawBitmapEx( Point( 1500, 19500 ), Size( 4800, 3000 ), aBmpEx );


    aWriter.EndStructureElement();
    aWriter.EndStructureElement();

    LineInfo aLI( LINE_DASH, 3 );
    aLI.SetDashCount( 2 );
    aLI.SetDashLen( 50 );
    aLI.SetDotCount( 2 );
    aLI.SetDotLen( 25 );
    aLI.SetDistance( 15 );
    Point aLIPoints[] = { Point( 4000, 10000 ),
                          Point( 8000, 12000 ),
                          Point( 3000, 19000 ) };
    Polygon aLIPoly( 3, aLIPoints );
    aWriter.SetLineColor( Color( COL_BLUE ) );
    aWriter.SetFillColor();
    aWriter.DrawPolyLine( aLIPoly, aLI );

    aLI.SetDashCount( 4 );
    aLIPoly.Move( 1000, 1000 );
    aWriter.DrawPolyLine( aLIPoly, aLI );

    aWriter.NewPage();
    aWriter.SetMapMode( MapMode( MAP_100TH_MM ) );
    Wallpaper aWall( aTransMask );
    aWall.SetStyle( WALLPAPER_TILE );
    aWriter.DrawWallpaper( Rectangle( Point( 4400, 4200 ), Size( 10200, 6300 ) ), aWall );

    aWriter.Push( PUSH_ALL );
    aWriter.BeginPattern(Rectangle(Point(0,0),Size(2000,1000)));
    aWriter.SetFillColor( Color( COL_RED ) );
    aWriter.SetLineColor( Color( COL_LIGHTBLUE ) );
    Point aFillPoints[] = { Point( 1000, 0 ),
                            Point( 0, 1000 ),
                            Point( 2000, 1000 ) };
    aWriter.DrawPolygon( Polygon( 3, aFillPoints ) );
    aWriter.DrawBitmap( Point( 200, 200 ), Size( 1600, 600 ), aTransMask );
    aWriter.DrawText( Rectangle( Point( 200, 200 ), Size( 1600, 600 ) ), String( RTL_CONSTASCII_USTRINGPARAM( "Pattern" ) ) );
    sal_Int32 nPattern = aWriter.EndPattern( SvtGraphicFill::Transform() );
    aWriter.Pop();
    Rectangle aPolyRect( Point( 3800, 11200 ), Size( 10200, 6300 ) );
    aWriter.DrawPolyPolygon( PolyPolygon( Polygon( aPolyRect ) ), nPattern, true );
    aWriter.SetFillColor();
    aWriter.SetLineColor( Color( COL_LIGHTBLUE ) );
    aWriter.DrawRect( aPolyRect );

    aWriter.NewPage();
    aWriter.SetMapMode( MapMode( MAP_100TH_MM ) );
    aWriter.SetFont( Font( String( RTL_CONSTASCII_USTRINGPARAM( "Times" ) ), Size( 0, 500 ) ) );
    aWriter.SetTextColor( Color( COL_BLACK ) );
    aRect = Rectangle( Point( 4500, 6000 ), Size( 6000, 1500 ) );
    aWriter.DrawRect( aRect );
    aWriter.DrawText( aRect, String( RTL_CONSTASCII_USTRINGPARAM( "www.heise.de" ) ) );
    sal_Int32 nURILink = aWriter.CreateLink( aRect );
    aWriter.SetLinkURL( nURILink, OUString( RTL_CONSTASCII_USTRINGPARAM( "http://www.heise.de" ) ) );

    aWriter.SetLinkDest( nFirstLink, nFirstDest );
    aWriter.SetLinkDest( nSecondLink, nSecondDest );

    // include a button
    PDFWriter::PushButtonWidget aBtn;
    aBtn.Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "testButton" ) );
    aBtn.Description = OUString( RTL_CONSTASCII_USTRINGPARAM( "A test button" ) );
    aBtn.Text = OUString( RTL_CONSTASCII_USTRINGPARAM( "hit me" ) );
    aBtn.Location = Rectangle( Point( 4500, 9000 ), Size( 4500, 3000 ) );
    aBtn.Border = aBtn.Background = true;
    aWriter.CreateControl( aBtn );

    // include a uri button
    PDFWriter::PushButtonWidget aUriBtn;
    aUriBtn.Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "wwwButton" ) );
    aUriBtn.Description = OUString( RTL_CONSTASCII_USTRINGPARAM( "A URI button" ) );
    aUriBtn.Text = OUString( RTL_CONSTASCII_USTRINGPARAM( "to www" ) );
    aUriBtn.Location = Rectangle( Point( 9500, 9000 ), Size( 4500, 3000 ) );
    aUriBtn.Border = aUriBtn.Background = true;
    aUriBtn.URL = OUString( RTL_CONSTASCII_USTRINGPARAM( "http://www.heise.de" ) );
    aWriter.CreateControl( aUriBtn );

    // include a dest button
    PDFWriter::PushButtonWidget aDstBtn;
    aDstBtn.Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "destButton" ) );
    aDstBtn.Description = OUString( RTL_CONSTASCII_USTRINGPARAM( "A Dest button" ) );
    aDstBtn.Text = OUString( RTL_CONSTASCII_USTRINGPARAM( "to paragraph" ) );
    aDstBtn.Location = Rectangle( Point( 14500, 9000 ), Size( 4500, 3000 ) );
    aDstBtn.Border = aDstBtn.Background = true;
    aDstBtn.Dest = nFirstDest;
    aWriter.CreateControl( aDstBtn );

    PDFWriter::CheckBoxWidget aCBox;
    aCBox.Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "textCheckBox" ) );
    aCBox.Description = OUString( RTL_CONSTASCII_USTRINGPARAM( "A test check box" ) );
    aCBox.Text = OUString( RTL_CONSTASCII_USTRINGPARAM( "check me" ) );
    aCBox.Location = Rectangle( Point( 4500, 13500 ), Size( 3000, 750 ) );
    aCBox.Checked = true;
    aCBox.Border = aCBox.Background = false;
    aWriter.CreateControl( aCBox );

    PDFWriter::CheckBoxWidget aCBox2;
    aCBox2.Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "textCheckBox2" ) );
    aCBox2.Description = OUString( RTL_CONSTASCII_USTRINGPARAM( "Another test check box" ) );
    aCBox2.Text = OUString( RTL_CONSTASCII_USTRINGPARAM( "check me right" ) );
    aCBox2.Location = Rectangle( Point( 4500, 14250 ), Size( 3000, 750 ) );
    aCBox2.Checked = true;
    aCBox2.Border = aCBox2.Background = false;
    aCBox2.ButtonIsLeft = false;
    aWriter.CreateControl( aCBox2 );

    PDFWriter::RadioButtonWidget aRB1;
    aRB1.Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "rb1_1" ) );
    aRB1.Description = OUString( RTL_CONSTASCII_USTRINGPARAM( "radio 1 button 1" ) );
    aRB1.Text = OUString( RTL_CONSTASCII_USTRINGPARAM( "Despair" ) );
    aRB1.Location = Rectangle( Point( 4500, 15000 ), Size( 6000, 1000 ) );
    aRB1.Selected = true;
    aRB1.RadioGroup = 1;
    aRB1.Border = aRB1.Background = true;
    aRB1.ButtonIsLeft = false;
    aRB1.BorderColor = Color( COL_LIGHTGREEN );
    aRB1.BackgroundColor = Color( COL_LIGHTBLUE );
    aRB1.TextColor = Color( COL_LIGHTRED );
    aRB1.TextFont = Font( String( RTL_CONSTASCII_USTRINGPARAM( "Courier" ) ), Size( 0, 800 ) );
    aWriter.CreateControl( aRB1 );

    PDFWriter::RadioButtonWidget aRB2;
    aRB2.Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "rb2_1" ) );
    aRB2.Description = OUString( RTL_CONSTASCII_USTRINGPARAM( "radio 2 button 1" ) );
    aRB2.Text = OUString( RTL_CONSTASCII_USTRINGPARAM( "Joy" ) );
    aRB2.Location = Rectangle( Point( 10500, 15000 ), Size( 3000, 1000 ) );
    aRB2.Selected = true;
    aRB2.RadioGroup = 2;
    aWriter.CreateControl( aRB2 );

    PDFWriter::RadioButtonWidget aRB3;
    aRB3.Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "rb1_2" ) );
    aRB3.Description = OUString( RTL_CONSTASCII_USTRINGPARAM( "radio 1 button 2" ) );
    aRB3.Text = OUString( RTL_CONSTASCII_USTRINGPARAM( "Desperation" ) );
    aRB3.Location = Rectangle( Point( 4500, 16000 ), Size( 3000, 1000 ) );
    aRB3.Selected = true;
    aRB3.RadioGroup = 1;
    aWriter.CreateControl( aRB3 );

    PDFWriter::EditWidget aEditBox;
    aEditBox.Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "testEdit" ) );
    aEditBox.Description = OUString( RTL_CONSTASCII_USTRINGPARAM( "A test edit field" ) );
    aEditBox.Text = OUString( RTL_CONSTASCII_USTRINGPARAM( "A little test text" ) );
    aEditBox.TextStyle = TEXT_DRAW_LEFT | TEXT_DRAW_VCENTER;
    aEditBox.Location = Rectangle( Point( 10000, 18000 ), Size( 5000, 1500 ) );
    aEditBox.MaxLen = 100;
    aEditBox.Border = aEditBox.Background = true;
    aEditBox.BorderColor = Color( COL_BLACK );
    aWriter.CreateControl( aEditBox );

    // normal list box
    PDFWriter::ListBoxWidget aLstBox;
    aLstBox.Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "testListBox" ) );
    aLstBox.Text = OUString( RTL_CONSTASCII_USTRINGPARAM( "One" ) );
    aLstBox.Description = OUString( RTL_CONSTASCII_USTRINGPARAM( "select me" ) );
    aLstBox.Location = Rectangle( Point( 4500, 18000 ), Size( 3000, 1500 ) );
    aLstBox.Sort = true;
    aLstBox.MultiSelect = true;
    aLstBox.Border = aLstBox.Background = true;
    aLstBox.BorderColor = Color( COL_BLACK );
    aLstBox.Entries.push_back( OUString( RTL_CONSTASCII_USTRINGPARAM( "One" ) ) );
    aLstBox.Entries.push_back( OUString( RTL_CONSTASCII_USTRINGPARAM( "Two" ) ) );
    aLstBox.Entries.push_back( OUString( RTL_CONSTASCII_USTRINGPARAM( "Three" ) ) );
    aLstBox.Entries.push_back( OUString( RTL_CONSTASCII_USTRINGPARAM( "Four" ) ) );
    aLstBox.SelectedEntries.push_back( 1 );
    aLstBox.SelectedEntries.push_back( 2 );
    aWriter.CreateControl( aLstBox );

    // dropdown list box
    aLstBox.Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "testDropDownListBox" ) );
    aLstBox.DropDown = true;
    aLstBox.Location = Rectangle( Point( 4500, 19500 ), Size( 3000, 500 ) );
    aWriter.CreateControl( aLstBox );

    // combo box
    PDFWriter::ComboBoxWidget aComboBox;
    aComboBox.Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "testComboBox" ) );
    aComboBox.Text = OUString( RTL_CONSTASCII_USTRINGPARAM( "test a combobox" ) );
    aComboBox.Entries.push_back( OUString( RTL_CONSTASCII_USTRINGPARAM( "Larry" ) ) );
    aComboBox.Entries.push_back( OUString( RTL_CONSTASCII_USTRINGPARAM( "Curly" ) ) );
    aComboBox.Entries.push_back( OUString( RTL_CONSTASCII_USTRINGPARAM( "Moe" ) ) );
    aComboBox.Location = Rectangle( Point( 4500, 20000 ), Size( 3000, 500 ) );
    aWriter.CreateControl( aComboBox );

    // test outlines
    sal_Int32 nPage1OL = aWriter.CreateOutlineItem();
    aWriter.SetOutlineItemText( nPage1OL, OUString( RTL_CONSTASCII_USTRINGPARAM( "Page 1" ) ) );
    aWriter.SetOutlineItemDest( nPage1OL, nSecondDest );
    aWriter.CreateOutlineItem( nPage1OL, OUString( RTL_CONSTASCII_USTRINGPARAM( "Dest 2" ) ), nSecondDest );
    aWriter.CreateOutlineItem( nPage1OL, OUString( RTL_CONSTASCII_USTRINGPARAM( "Dest 2 revisited" ) ), nSecondDest );
    aWriter.CreateOutlineItem( nPage1OL, OUString( RTL_CONSTASCII_USTRINGPARAM( "Dest 2 again" ) ), nSecondDest );
    sal_Int32 nPage2OL = aWriter.CreateOutlineItem();
    aWriter.SetOutlineItemText( nPage2OL, OUString( RTL_CONSTASCII_USTRINGPARAM( "Page 2" ) ) );
    aWriter.CreateOutlineItem( nPage2OL, OUString( RTL_CONSTASCII_USTRINGPARAM( "Dest 1" ) ), nFirstDest );

    aWriter.EndStructureElement(); // close document
    aWriter.Emit();
}
#endif

static const sal_Int32 nLog10Divisor = 1;
static const double fDivisor = 10.0;

static inline double pixelToPoint( sal_Int32 px ) { return double(px)/fDivisor; }
static inline double pixelToPoint( double px ) { return px/fDivisor; }
static inline sal_Int32 pointToPixel( double pt ) { return sal_Int32(pt*fDivisor); }

static void appendHex( sal_Int8 nInt, OStringBuffer& rBuffer )
{
    static const sal_Char pHexDigits[] = { '0', '1', '2', '3', '4', '5', '6', '7',
                                           '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
    rBuffer.append( pHexDigits[ (nInt >> 4) & 15 ] );
    rBuffer.append( pHexDigits[ nInt & 15 ] );
}

static void appendName( const OUString& rStr, OStringBuffer& rBuffer )
{
// FIXME i59651 add a check for max length of 127 chars? Per PDF spec 1.4, appendix C.1
// I guess than when reading the #xx sequence it will count for a single character.
    OString aStr( OUStringToOString( rStr, RTL_TEXTENCODING_UTF8 ) );
    const sal_Char* pStr = aStr.getStr();
    int nLen = aStr.getLength();
    for( int i = 0; i < nLen; i++ )
    {
        /*  #i16920# PDF recommendation: output UTF8, any byte
         *  outside the interval [33(=ASCII'!');126(=ASCII'~')]
         *  should be escaped hexadecimal
         *  for the sake of ghostscript which also reads PDF
         *  but has a narrower acceptance rate we only pass
         *  alphanumerics and '-' literally.
         */
        if( (pStr[i] >= 'A' && pStr[i] <= 'Z' ) ||
            (pStr[i] >= 'a' && pStr[i] <= 'z' ) ||
            (pStr[i] >= '0' && pStr[i] <= '9' ) ||
            pStr[i] == '-' )
        {
            rBuffer.append( pStr[i] );
        }
        else
        {
            rBuffer.append( '#' );
            appendHex( (sal_Int8)pStr[i], rBuffer );
        }
    }
}

static void appendName( const sal_Char* pStr, OStringBuffer& rBuffer )
{
//FIXME i59651 see above
    while( pStr && *pStr )
    {
        if( (*pStr >= 'A' && *pStr <= 'Z' ) ||
            (*pStr >= 'a' && *pStr <= 'z' ) ||
            (*pStr >= '0' && *pStr <= '9' ) ||
            *pStr == '-' )
        {
            rBuffer.append( *pStr );
        }
        else
        {
            rBuffer.append( '#' );
            appendHex( (sal_Int8)*pStr, rBuffer );
        }
        pStr++;
    }
}

//used only to emit encoded passwords
static void appendLiteralString( const sal_Char* pStr, sal_Int32 nLength, OStringBuffer& rBuffer )
{
	while( nLength )
	{
		switch( *pStr )
		{
		case '\n' :
			rBuffer.append( "\\n" );
			break;
		case '\r' :
			rBuffer.append( "\\r" );
			break;
		case '\t' :
			rBuffer.append( "\\t" );
			break;
		case '\b' :
			rBuffer.append( "\\b" );
			break;
		case '\f' :
			rBuffer.append( "\\f" );
			break;
		case '(' :
		case ')' :
		case '\\' :
			rBuffer.append( "\\" );
			rBuffer.append( (sal_Char) *pStr );
			break;
		default:
			rBuffer.append( (sal_Char) *pStr );
			break;
		}
		pStr++;
		nLength--;
	}
}

/**--->i56629
 * Convert a string before using it.
 *
 * This string conversion function is needed because the destination name
 * in a PDF file seen through an Internet browser should be
 * specially crafted, in order to be used directly by the browser.
 * In this way the fragment part of a hyperlink to a PDF file (e.g. something
 * as 'test1/test2/a-file.pdf#thefragment) will be (hopefully) interpreted by the
 * PDF reader (currently only Adobe Reader plug-in seems to be working that way) called
 * from inside the Internet browser as: 'open the file test1/test2/a-file.pdf
 * and go to named destination thefragment using default zoom'.
 * The conversion is needed because in case of a fragment in the form: Slide%201
 * (meaning Slide 1) as it is converted obeying the Inet rules, it will become Slide25201
 * using this conversion, in both the generated named destinations, fragment and GoToR
 * destination.
 *
 * The names for destinations are name objects and so they don't need to be encrypted
 * even though they expose the content of PDF file (e.g. guessing the PDF content from the
 * destination name).
 *
 * Fhurter limitation: it is advisable to use standard ASCII characters for
 * OOo bookmarks.
*/
static void appendDestinationName( const rtl::OUString& rString, OStringBuffer& rBuffer )
{
    const sal_Unicode* pStr = rString.getStr();
    sal_Int32 nLen = rString.getLength();
    for( int i = 0; i < nLen; i++ )
    {
        sal_Unicode aChar = pStr[i];
        if( (aChar >= '0' && aChar <= '9' ) ||
            (aChar >= 'a' && aChar <= 'z' ) ||
            (aChar >= 'A' && aChar <= 'Z' ) ||
            aChar == '-' )
        {
            rBuffer.append((sal_Char)aChar);
        }
        else
        {
            sal_Int8 aValueHigh = sal_Int8(aChar >> 8);
            if(aValueHigh > 0)
                appendHex( aValueHigh, rBuffer );
            appendHex( (sal_Int8)(aChar & 255 ), rBuffer );
        }
    }
}
//<--- i56629

static void appendUnicodeTextString( const rtl::OUString& rString, OStringBuffer& rBuffer )
{
	rBuffer.append( "FEFF" );
	const sal_Unicode* pStr = rString.getStr();
	sal_Int32 nLen = rString.getLength();
	for( int i = 0; i < nLen; i++ )
	{
		sal_Unicode aChar = pStr[i];
		appendHex( (sal_Int8)(aChar >> 8), rBuffer );
		appendHex( (sal_Int8)(aChar & 255 ), rBuffer );
	}
}

OString PDFWriterImpl::convertWidgetFieldName( const rtl::OUString& rString )
{
    OStringBuffer aBuffer( rString.getLength()+64 );

    /* #i80258# previously we use appendName here
       however we need a slightly different coding scheme than the normal
       name encoding for field names

       also replace all '.' by '_' as '.' indicates a hierarchy level which
       we do not have here
    */

    OString aStr( OUStringToOString( rString, RTL_TEXTENCODING_UTF8 ) );
    const sal_Char* pStr = aStr.getStr();
    int nLen = aStr.getLength();
    for( int i = 0; i < nLen; i++ )
    {
        /*  #i16920# PDF recommendation: output UTF8, any byte
         *  outside the interval [33(=ASCII'!');126(=ASCII'~')]
         *  should be escaped hexadecimal
         */
        if( pStr[i] == '.' )
            aBuffer.append( '_' );
        else if( (pStr[i] >= 33 && pStr[i] <= 126 ) )
            aBuffer.append( pStr[i] );
        else
        {
            aBuffer.append( '#' );
            appendHex( (sal_Int8)pStr[i], aBuffer );
        }
    }

    OString aRet = aBuffer.makeStringAndClear();
    std::hash_map<OString, sal_Int32, OStringHash>::iterator it = m_aFieldNameMap.find( aRet );

    if( it != m_aFieldNameMap.end() ) // not unique
    {
        std::hash_map< OString, sal_Int32, OStringHash >::const_iterator check_it;
        OString aTry;
        do
        {
            OStringBuffer aUnique( aRet.getLength() + 16 );
            aUnique.append( aRet );
            aUnique.append( '_' );
            aUnique.append( it->second );
            it->second++;
            aTry = aUnique.makeStringAndClear();
            check_it = m_aFieldNameMap.find( aTry );
        } while( check_it != m_aFieldNameMap.end() );
        aRet = aTry;
    }
    else
        m_aFieldNameMap[ aRet ] = 2;
    return aRet;
}

static void appendFixedInt( sal_Int32 nValue, OStringBuffer& rBuffer, sal_Int32 nPrecision = nLog10Divisor )
{
    if( nValue < 0 )
    {
        rBuffer.append( '-' );
        nValue = -nValue;
    }
    sal_Int32 nFactor = 1, nDiv = nPrecision;
    while( nDiv-- )
        nFactor *= 10;

    sal_Int32 nInt		= nValue / nFactor;
    rBuffer.append( nInt );
    if( nFactor > 1 )
    {
        sal_Int32 nDecimal	= nValue % nFactor;
        if( nDecimal )
        {
            rBuffer.append( '.' );
            // omit trailing zeros
            while( (nDecimal % 10) == 0 )
                nDecimal /= 10;
            rBuffer.append( nDecimal );
        }
    }
}


// appends a double. PDF does not accept exponential format, only fixed point
static void appendDouble( double fValue, OStringBuffer& rBuffer, int nPrecision = 5 )
{
    bool bNeg = false;
    if( fValue < 0.0 )
    {
        bNeg = true;
        fValue=-fValue;
    }

    sal_Int64 nInt = (sal_Int64)fValue;
    fValue -= (double)nInt;
    // optimizing hardware may lead to a value of 1.0 after the subtraction
    if( fValue == 1.0 || log10( 1.0-fValue ) <= -nPrecision )
    {
        nInt++;
        fValue = 0.0;
    }
    sal_Int64 nFrac = 0;
    if( fValue )
    {
        fValue *= pow( 10.0, (double)nPrecision );
        nFrac = (sal_Int64)fValue;
    }
    if( bNeg && ( nInt || nFrac ) )
        rBuffer.append( '-' );
    rBuffer.append( nInt );
    if( nFrac )
    {
		int i;
        rBuffer.append( '.' );
		sal_Int64 nBound = (sal_Int64)(pow( 10.0, nPrecision - 1.0 )+0.5);
		for ( i = 0; ( i < nPrecision ) && nFrac; i++ )
		{
			sal_Int64 nNumb = nFrac / nBound;
			nFrac -= nNumb * nBound;
			rBuffer.append( nNumb );
			nBound /= 10;
		}
    }
}


static void appendColor( const Color& rColor, OStringBuffer& rBuffer )
{

    if( rColor != Color( COL_TRANSPARENT ) )
    {
        appendDouble( (double)rColor.GetRed() / 255.0, rBuffer );
        rBuffer.append( ' ' );
        appendDouble( (double)rColor.GetGreen() / 255.0, rBuffer );
        rBuffer.append( ' ' );
        appendDouble( (double)rColor.GetBlue() / 255.0, rBuffer );
    }
}

static void appendStrokingColor( const Color& rColor, OStringBuffer& rBuffer )
{
    if( rColor != Color( COL_TRANSPARENT ) )
    {
        appendColor( rColor, rBuffer );
        rBuffer.append( " RG" );
    }
}

static void appendNonStrokingColor( const Color& rColor, OStringBuffer& rBuffer )
{
    if( rColor != Color( COL_TRANSPARENT ) )
    {
        appendColor( rColor, rBuffer );
        rBuffer.append( " rg" );
    }
}

// matrix helper class
// TODO: use basegfx matrix class instead or derive from it
namespace vcl // TODO: use anonymous namespace to keep this class local
{
/*	for sparse matrices of the form (2D linear transformations)
 *  f[0] f[1] 0
 *  f[2] f[3] 0
 *  f[4] f[5] 1
 */
class Matrix3
{
    double f[6];

    void set( double *pn ) { for( int i = 0 ; i < 5; i++ ) f[i] = pn[i]; }
public:
    Matrix3();
    ~Matrix3() {}

    void skew( double alpha, double beta );
    void scale( double sx, double sy );
    void rotate( double angle );
    void translate( double tx, double ty );
    bool invert();

    void append( PDFWriterImpl::PDFPage& rPage, OStringBuffer& rBuffer, Point* pBack = NULL );

    Point transform( const Point& rPoint ) const;
};
}

Matrix3::Matrix3()
{
    // initialize to unity
    f[0] = 1.0;
    f[1] = 0.0;
    f[2] = 0.0;
    f[3] = 1.0;
    f[4] = 0.0;
    f[5] = 0.0;
}

Point Matrix3::transform( const Point& rOrig ) const
{
    double x = (double)rOrig.X(), y = (double)rOrig.Y();
    return Point( (int)(x*f[0] + y*f[2] + f[4]), (int)(x*f[1] + y*f[3] + f[5]) );
}

void Matrix3::skew( double alpha, double beta )
{
    double fn[6];
    double tb = tan( beta );
    fn[0] = f[0] + f[2]*tb;
    fn[1] = f[1];
    fn[2] = f[2] + f[3]*tb;
    fn[3] = f[3];
    fn[4] = f[4] + f[5]*tb;
    fn[5] = f[5];
    if( alpha != 0.0 )
    {
        double ta = tan( alpha );
        fn[1] += f[0]*ta;
        fn[3] += f[2]*ta;
        fn[5] += f[4]*ta;
    }
    set( fn );
}

void Matrix3::scale( double sx, double sy )
{
    double fn[6];
    fn[0] = sx*f[0];
    fn[1] = sy*f[1];
    fn[2] = sx*f[2];
    fn[3] = sy*f[3];
    fn[4] = sx*f[4];
    fn[5] = sy*f[5];
    set( fn );
}

void Matrix3::rotate( double angle )
{
    double fn[6];
    double fSin = sin(angle);
    double fCos = cos(angle);
    fn[0] = f[0]*fCos - f[1]*fSin;
    fn[1] = f[0]*fSin + f[1]*fCos;
    fn[2] = f[2]*fCos - f[3]*fSin;
    fn[3] = f[2]*fSin + f[3]*fCos;
    fn[4] = f[4]*fCos - f[5]*fSin;
    fn[5] = f[4]*fSin + f[5]*fCos;
    set( fn );
}

void Matrix3::translate( double tx, double ty )
{
    f[4] += tx;
    f[5] += ty;
}

bool Matrix3::invert()
{
	// short circuit trivial cases
	if( f[1]==f[2] && f[1]==0.0 && f[0]==f[3] && f[0]==1.0 )
	{
		f[4] = -f[4];
		f[5] = -f[5];
		return true;
	}

	// check determinant
	const double fDet = f[0]*f[3]-f[1]*f[2];
	if( fDet == 0.0 )
		return false;

	// invert the matrix
	double fn[6];
	fn[0] = +f[3] / fDet;
	fn[1] = -f[1] / fDet;
	fn[2] = -f[2] / fDet;
	fn[3] = +f[0] / fDet;

	// apply inversion to translation
	fn[4] = -(f[4]*fn[0] + f[5]*fn[2]);
	fn[5] = -(f[4]*fn[1] + f[5]*fn[3]);

	set( fn );
	return true;
}

void Matrix3::append( PDFWriterImpl::PDFPage& rPage, OStringBuffer& rBuffer, Point* pBack )
{
    appendDouble( f[0], rBuffer );
    rBuffer.append( ' ' );
    appendDouble( f[1], rBuffer );
    rBuffer.append( ' ' );
    appendDouble( f[2], rBuffer );
    rBuffer.append( ' ' );
    appendDouble( f[3], rBuffer );
    rBuffer.append( ' ' );
    rPage.appendPoint( Point( (long)f[4], (long)f[5] ), rBuffer, false, pBack );
}

static void appendResourceMap( OStringBuffer& rBuf, const char* pPrefix, const PDFWriterImpl::ResourceMap& rList )
{
    if( rList.empty() )
        return;
    rBuf.append( '/' );
    rBuf.append( pPrefix );
    rBuf.append( "<<" );
    int ni = 0;
    for( PDFWriterImpl::ResourceMap::const_iterator it = rList.begin(); it != rList.end(); ++it )
    {
        if( it->first.getLength() && it->second > 0 )
        {
            rBuf.append( '/' );
            rBuf.append( it->first );
            rBuf.append( ' ' );
            rBuf.append( it->second );
            rBuf.append( " 0 R" );
            if( ((++ni) & 7) == 0 )
                rBuf.append( '\n' );
        }
    }
    rBuf.append( ">>\n" );
}

void PDFWriterImpl::ResourceDict::append( OStringBuffer& rBuf, sal_Int32 nFontDictObject )
{
    rBuf.append( "<</Font " );
    rBuf.append( nFontDictObject );
    rBuf.append( " 0 R\n" );
    appendResourceMap( rBuf, "XObject", m_aXObjects );
    appendResourceMap( rBuf, "ExtGState", m_aExtGStates );
    appendResourceMap( rBuf, "Shading", m_aShadings );
    appendResourceMap( rBuf, "Pattern", m_aPatterns );
    rBuf.append( "/ProcSet[/PDF/Text" );
    if( !m_aXObjects.empty() )
        rBuf.append( "/ImageC/ImageI/ImageB" );
    rBuf.append( "]\n>>\n" );
};

PDFWriterImpl::PDFPage::PDFPage( PDFWriterImpl* pWriter, sal_Int32 nPageWidth, sal_Int32 nPageHeight, PDFWriter::Orientation eOrientation )
        :
        m_pWriter( pWriter ),
        m_nPageWidth( nPageWidth ),
        m_nPageHeight( nPageHeight ),
        m_eOrientation( eOrientation ),
        m_nPageObject( 0 ),  // invalid object number
        m_nPageIndex( -1 ), // invalid index
        m_nStreamLengthObject( 0 ),
        m_nBeginStreamPos( 0 ),
        m_eTransition( PDFWriter::Regular ),
        m_nTransTime( 0 ),
        m_nDuration( 0 ),
        m_bHasWidgets( false )
{
    // object ref must be only ever updated in emit()
    m_nPageObject = m_pWriter->createObject();
}

PDFWriterImpl::PDFPage::~PDFPage()
{
}

void PDFWriterImpl::PDFPage::beginStream()
{
#if OSL_DEBUG_LEVEL > 1
    {
        OStringBuffer aLine( "PDFWriterImpl::PDFPage::beginStream, +" );
         m_pWriter->emitComment( aLine.getStr() );
    }
#endif
    m_aStreamObjects.push_back(m_pWriter->createObject());
    if( ! m_pWriter->updateObject( m_aStreamObjects.back() ) )
        return;

    m_nStreamLengthObject = m_pWriter->createObject();
    // write content stream header
    OStringBuffer aLine;
    aLine.append( m_aStreamObjects.back() );
    aLine.append( " 0 obj\n<</Length " );
    aLine.append( m_nStreamLengthObject );
    aLine.append( " 0 R" );
#if defined ( COMPRESS_PAGES ) && !defined ( DEBUG_DISABLE_PDFCOMPRESSION )
    aLine.append( "/Filter/FlateDecode" );
#endif
    aLine.append( ">>\nstream\n" );
    if( ! m_pWriter->writeBuffer( aLine.getStr(), aLine.getLength() ) )
        return;
    if( osl_File_E_None != osl_getFilePos( m_pWriter->m_aFile, &m_nBeginStreamPos ) )
    {
        osl_closeFile( m_pWriter->m_aFile );
        m_pWriter->m_bOpen = false;
    }
#if defined ( COMPRESS_PAGES ) && !defined ( DEBUG_DISABLE_PDFCOMPRESSION )
    m_pWriter->beginCompression();
#endif
    m_pWriter->checkAndEnableStreamEncryption( m_aStreamObjects.back() );
}

void PDFWriterImpl::PDFPage::endStream()
{
#if defined ( COMPRESS_PAGES ) && !defined ( DEBUG_DISABLE_PDFCOMPRESSION )
    m_pWriter->endCompression();
#endif
    sal_uInt64 nEndStreamPos;
    if( osl_File_E_None != osl_getFilePos( m_pWriter->m_aFile, &nEndStreamPos ) )
    {
        osl_closeFile( m_pWriter->m_aFile );
        m_pWriter->m_bOpen = false;
        return;
    }
    m_pWriter->disableStreamEncryption();
    if( ! m_pWriter->writeBuffer( "\nendstream\nendobj\n\n", 19 ) )
        return;
    // emit stream length object
    if( ! m_pWriter->updateObject( m_nStreamLengthObject ) )
        return;
    OStringBuffer aLine;
    aLine.append( m_nStreamLengthObject );
    aLine.append( " 0 obj\n" );
    aLine.append( (sal_Int64)(nEndStreamPos-m_nBeginStreamPos) );
    aLine.append( "\nendobj\n\n" );
    m_pWriter->writeBuffer( aLine.getStr(), aLine.getLength() );
}

bool PDFWriterImpl::PDFPage::emit(sal_Int32 nParentObject )
{
    // emit page object
    if( ! m_pWriter->updateObject( m_nPageObject ) )
        return false;
    OStringBuffer aLine;

    aLine.append( m_nPageObject );
    aLine.append( " 0 obj\n"
                  "<</Type/Page/Parent " );
    aLine.append( nParentObject );
    aLine.append( " 0 R" );
    aLine.append( "/Resources " );
    aLine.append( m_pWriter->getResourceDictObj() );
    aLine.append( " 0 R" );
    if( m_nPageWidth && m_nPageHeight )
    {
        aLine.append( "/MediaBox[0 0 " );
        aLine.append( m_nPageWidth );
        aLine.append( ' ' );
        aLine.append( m_nPageHeight );
        aLine.append( "]" );
    }
    switch( m_eOrientation )
    {
        case PDFWriter::Landscape: aLine.append( "/Rotate 90\n" );break;
        case PDFWriter::Seascape:  aLine.append( "/Rotate -90\n" );break;
        case PDFWriter::Portrait:  aLine.append( "/Rotate 0\n" );break;

        case PDFWriter::Inherit:
        default:
            break;
    }
    int nAnnots = m_aAnnotations.size();
    if( nAnnots > 0 )
    {
        aLine.append( "/Annots[\n" );
        for( int i = 0; i < nAnnots; i++ )
        {
            aLine.append( m_aAnnotations[i] );
            aLine.append( " 0 R" );
            aLine.append( ((i+1)%15) ? " " : "\n" );
        }
        aLine.append( "]\n" );
    }
    #if 0
    // FIXME: implement tab order as Structure Tree
    if( m_bHasWidgets && m_pWriter->getVersion() >= PDFWriter::PDF_1_5 )
        aLine.append( "   /Tabs /S\n" );
    #endif
    if( m_aMCIDParents.size() > 0 )
    {
        OStringBuffer aStructParents( 1024 );
        aStructParents.append( "[ " );
        int nParents = m_aMCIDParents.size();
        for( int i = 0; i < nParents; i++ )
        {
            aStructParents.append( m_aMCIDParents[i] );
            aStructParents.append( " 0 R" );
            aStructParents.append( ((i%10) == 9) ? "\n" : " " );
        }
        aStructParents.append( "]" );
        m_pWriter->m_aStructParentTree.push_back( aStructParents.makeStringAndClear() );

        aLine.append( "/StructParents " );
        aLine.append( sal_Int32(m_pWriter->m_aStructParentTree.size()-1) );
        aLine.append( "\n" );
    }
    if( m_nDuration > 0 )
    {
        aLine.append( "/Dur " );
        aLine.append( (sal_Int32)m_nDuration );
        aLine.append( "\n" );
    }
    if( m_eTransition != PDFWriter::Regular && m_nTransTime > 0 )
    {
        // transition duration
        aLine.append( "/Trans<</D " );
        appendDouble( (double)m_nTransTime/1000.0, aLine, 3 );
        aLine.append( "\n" );
        const char *pStyle = NULL, *pDm = NULL, *pM = NULL, *pDi = NULL;
        switch( m_eTransition )
        {
            case PDFWriter::SplitHorizontalInward:
                pStyle = "Split"; pDm = "H"; pM = "I"; break;
            case PDFWriter::SplitHorizontalOutward:
                pStyle = "Split"; pDm = "H"; pM = "O"; break;
            case PDFWriter::SplitVerticalInward:
                pStyle = "Split"; pDm = "V"; pM = "I"; break;
            case PDFWriter::SplitVerticalOutward:
                pStyle = "Split"; pDm = "V"; pM = "O"; break;
            case PDFWriter::BlindsHorizontal:
                pStyle = "Blinds"; pDm = "H"; break;
            case PDFWriter::BlindsVertical:
                pStyle = "Blinds"; pDm = "V"; break;
            case PDFWriter::BoxInward:
                pStyle = "Box"; pM = "I"; break;
            case PDFWriter::BoxOutward:
                pStyle = "Box"; pM = "O"; break;
            case PDFWriter::WipeLeftToRight:
                pStyle = "Wipe"; pDi = "0"; break;
            case PDFWriter::WipeBottomToTop:
                pStyle = "Wipe"; pDi = "90"; break;
            case PDFWriter::WipeRightToLeft:
                pStyle = "Wipe"; pDi = "180"; break;
            case PDFWriter::WipeTopToBottom:
                pStyle = "Wipe"; pDi = "270"; break;
            case PDFWriter::Dissolve:
                pStyle = "Dissolve"; break;
            case PDFWriter::GlitterLeftToRight:
                pStyle = "Glitter"; pDi = "0"; break;
            case PDFWriter::GlitterTopToBottom:
                pStyle = "Glitter"; pDi = "270"; break;
            case PDFWriter::GlitterTopLeftToBottomRight:
                pStyle = "Glitter"; pDi = "315"; break;
            case PDFWriter::Regular:
                break;
        }
        // transition style
        if( pStyle )
        {
            aLine.append( "/S/" );
            aLine.append( pStyle );
            aLine.append( "\n" );
        }
        if( pDm )
        {
            aLine.append( "/Dm/" );
            aLine.append( pDm );
            aLine.append( "\n" );
        }
        if( pM )
        {
            aLine.append( "/M/" );
            aLine.append( pM );
            aLine.append( "\n" );
        }
        if( pDi  )
        {
            aLine.append( "/Di " );
            aLine.append( pDi );
            aLine.append( "\n" );
        }
        aLine.append( ">>\n" );
    }
    if( m_pWriter->getVersion() > PDFWriter::PDF_1_3 && ! m_pWriter->m_bIsPDF_A1 )
    {
        aLine.append( "/Group<</S/Transparency/CS/DeviceRGB/I true>>" );
    }
    aLine.append( "/Contents" );
    unsigned int nStreamObjects = m_aStreamObjects.size();
    if( nStreamObjects > 1 )
        aLine.append( '[' );
    for( unsigned int i = 0; i < m_aStreamObjects.size(); i++ )
    {
        aLine.append( ' ' );
        aLine.append( m_aStreamObjects[i] );
        aLine.append( " 0 R" );
    }
    if( nStreamObjects > 1 )
        aLine.append( ']' );
    aLine.append( ">>\nendobj\n\n" );
    return m_pWriter->writeBuffer( aLine.getStr(), aLine.getLength() );
}

namespace vcl
{
template < class GEOMETRY >
GEOMETRY lcl_convert( const MapMode& _rSource, const MapMode& _rDest, OutputDevice* _pPixelConversion, const GEOMETRY& _rObject )
{
    GEOMETRY aPoint;
    if ( MAP_PIXEL == _rSource.GetMapUnit() )
    {
        aPoint = _pPixelConversion->PixelToLogic( _rObject, _rDest );
    }
    else
    {
        aPoint = OutputDevice::LogicToLogic( _rObject, _rSource, _rDest );
    }
    return aPoint;
}
}

void PDFWriterImpl::PDFPage::appendPoint( const Point& rPoint, OStringBuffer& rBuffer, bool bNeg, Point* pOutPoint ) const
{
    if( pOutPoint )
    {
        Point aPoint( lcl_convert( m_pWriter->m_aGraphicsStack.front().m_aMapMode,
                                   m_pWriter->m_aMapMode,
                                   m_pWriter->getReferenceDevice(),
                                   rPoint ) );
        *pOutPoint = aPoint;
    }

    Point aPoint( lcl_convert( m_pWriter->m_aGraphicsStack.front().m_aMapMode,
                               m_pWriter->m_aMapMode,
                               m_pWriter->getReferenceDevice(),
                               rPoint ) );

    sal_Int32 nValue	= aPoint.X();
    if( bNeg )
        nValue = -nValue;

    appendFixedInt( nValue, rBuffer );

    rBuffer.append( ' ' );

    nValue		= pointToPixel(getHeight()) - aPoint.Y();
    if( bNeg )
        nValue = -nValue;

    appendFixedInt( nValue, rBuffer );
}

void PDFWriterImpl::PDFPage::appendRect( const Rectangle& rRect, OStringBuffer& rBuffer ) const
{
    appendPoint( rRect.BottomLeft() + Point( 0, 1 ), rBuffer );
    rBuffer.append( ' ' );
    appendMappedLength( (sal_Int32)rRect.GetWidth(), rBuffer, false );
    rBuffer.append( ' ' );
    appendMappedLength( (sal_Int32)rRect.GetHeight(), rBuffer, true );
    rBuffer.append( " re" );
}

void PDFWriterImpl::PDFPage::convertRect( Rectangle& rRect ) const
{
    Point aLL = lcl_convert( m_pWriter->m_aGraphicsStack.front().m_aMapMode,
                             m_pWriter->m_aMapMode,
                             m_pWriter->getReferenceDevice(),
                             rRect.BottomLeft() + Point( 0, 1 )
                             );
    Size aSize = lcl_convert( m_pWriter->m_aGraphicsStack.front().m_aMapMode,
                              m_pWriter->m_aMapMode,
                              m_pWriter->getReferenceDevice(),
                              rRect.GetSize() );
    rRect.Left()	= aLL.X();
    rRect.Right()	= aLL.X() + aSize.Width();
    rRect.Top()		= pointToPixel(getHeight()) - aLL.Y();
    rRect.Bottom()	= rRect.Top() + aSize.Height();
}

void PDFWriterImpl::PDFPage::appendPolygon( const Polygon& rPoly, OStringBuffer& rBuffer, bool bClose ) const
{
    USHORT nPoints = rPoly.GetSize();
    /*
     *  #108582# applications do weird things
     */
    sal_uInt32 nBufLen = rBuffer.getLength();
    if( nPoints > 0 )
    {
        const BYTE* pFlagArray = rPoly.GetConstFlagAry();
        appendPoint( rPoly[0], rBuffer );
        rBuffer.append( " m\n" );
        for( USHORT i = 1; i < nPoints; i++ )
        {
            if( pFlagArray && pFlagArray[i] == POLY_CONTROL && nPoints-i > 2 )
            {
                // bezier
                DBG_ASSERT( pFlagArray[i+1] == POLY_CONTROL && pFlagArray[i+2] != POLY_CONTROL, "unexpected sequence of control points" );
                appendPoint( rPoly[i], rBuffer );
                rBuffer.append( " " );
                appendPoint( rPoly[i+1], rBuffer );
                rBuffer.append( " " );
                appendPoint( rPoly[i+2], rBuffer );
                rBuffer.append( " c" );
                i += 2; // add additionally consumed points
            }
            else
            {
                // line
                appendPoint( rPoly[i], rBuffer );
                rBuffer.append( " l" );
            }
            if( (rBuffer.getLength() - nBufLen) > 65 )
            {
                rBuffer.append( "\n" );
                nBufLen = rBuffer.getLength();
            }
            else
                rBuffer.append( " " );
        }
        if( bClose )
            rBuffer.append( "h\n" );
    }
}

void PDFWriterImpl::PDFPage::appendPolyPolygon( const PolyPolygon& rPolyPoly, OStringBuffer& rBuffer, bool bClose ) const
{
    USHORT nPolygons = rPolyPoly.Count();
    for( USHORT n = 0; n < nPolygons; n++ )
        appendPolygon( rPolyPoly[n], rBuffer, bClose );
}

void PDFWriterImpl::PDFPage::appendMappedLength( sal_Int32 nLength, OStringBuffer& rBuffer, bool bVertical, sal_Int32* pOutLength ) const
{
    sal_Int32 nValue = nLength;
    if ( nLength < 0 )
    {
        rBuffer.append( '-' );
        nValue = -nLength;
    }
    Size aSize( lcl_convert( m_pWriter->m_aGraphicsStack.front().m_aMapMode,
                             m_pWriter->m_aMapMode,
                             m_pWriter->getReferenceDevice(),
                             Size( nValue, nValue ) ) );
    nValue = bVertical ? aSize.Height() : aSize.Width();
    if( pOutLength )
        *pOutLength = ((nLength < 0 ) ? -nValue : nValue);

    appendFixedInt( nValue, rBuffer, 1 );
}

void PDFWriterImpl::PDFPage::appendMappedLength( double fLength, OStringBuffer& rBuffer, bool bVertical, sal_Int32* pOutLength ) const
{
    Size aSize( lcl_convert( m_pWriter->m_aGraphicsStack.front().m_aMapMode,
                             m_pWriter->m_aMapMode,
                             m_pWriter->getReferenceDevice(),
                             Size( 1000, 1000 )	) );
    if( pOutLength )
        *pOutLength = (sal_Int32)(fLength*(double)(bVertical ? aSize.Height() : aSize.Width())/1000.0);
    fLength *= pixelToPoint((double)(bVertical ? aSize.Height() : aSize.Width()) / 1000.0);
    appendDouble( fLength, rBuffer );
}

bool PDFWriterImpl::PDFPage::appendLineInfo( const LineInfo& rInfo, OStringBuffer& rBuffer ) const
{
    bool bRet = true;
    if( rInfo.GetStyle() == LINE_DASH )
    {
        rBuffer.append( "[ " );
        if( rInfo.GetDashLen() == rInfo.GetDotLen() ) // degraded case
        {
            appendMappedLength( (sal_Int32)rInfo.GetDashLen(), rBuffer );
            rBuffer.append( ' ' );
            appendMappedLength( (sal_Int32)rInfo.GetDistance(), rBuffer );
            rBuffer.append( ' ' );
        }
        else
        {
            // check for implementation limits of dash array
            // in PDF reader apps (e.g. acroread)
            if( 2*(rInfo.GetDashCount() + rInfo.GetDotCount()) > 10 )
                bRet = false;
            for( int n = 0; n < rInfo.GetDashCount(); n++ )
            {
                appendMappedLength( (sal_Int32)rInfo.GetDashLen(), rBuffer );
                rBuffer.append( ' ' );
                appendMappedLength( (sal_Int32)rInfo.GetDistance(), rBuffer );
                rBuffer.append( ' ' );
            }
            for( int m = 0; m < rInfo.GetDotCount(); m++ )
            {
                appendMappedLength( (sal_Int32)rInfo.GetDotLen(), rBuffer );
                rBuffer.append( ' ' );
                appendMappedLength( (sal_Int32)rInfo.GetDistance(), rBuffer );
                rBuffer.append( ' ' );
            }
        }
        rBuffer.append( "] 0 d\n" );
    }
    if( rInfo.GetWidth() > 1 )
    {
        appendMappedLength( (sal_Int32)rInfo.GetWidth(), rBuffer );
        rBuffer.append( " w\n" );
    }
    else if( rInfo.GetWidth() == 0 )
    {
        // "pixel" line
        appendDouble( 72.0/double(m_pWriter->getReferenceDevice()->ImplGetDPIX()), rBuffer );
        rBuffer.append( " w\n" );
    }
    return bRet;
}

void PDFWriterImpl::PDFPage::appendWaveLine( sal_Int32 nWidth, sal_Int32 nY, sal_Int32 nDelta, OStringBuffer& rBuffer ) const
{
    if( nWidth <= 0 )
        return;
    if( nDelta < 1 )
        nDelta = 1;

    rBuffer.append( "0 " );
    appendMappedLength( nY, rBuffer, true );
    rBuffer.append( " m\n" );
    for( sal_Int32 n = 0; n < nWidth; )
    {
        n += nDelta;
        appendMappedLength( n, rBuffer, false );
        rBuffer.append( ' ' );
        appendMappedLength( nDelta+nY, rBuffer, true );
        rBuffer.append( ' ' );
        n += nDelta;
        appendMappedLength( n, rBuffer, false );
        rBuffer.append( ' ' );
        appendMappedLength( nY, rBuffer, true );
        rBuffer.append( " v " );
        if( n < nWidth )
        {
            n += nDelta;
            appendMappedLength( n, rBuffer, false );
            rBuffer.append( ' ' );
            appendMappedLength( nY-nDelta, rBuffer, true );
            rBuffer.append( ' ' );
            n += nDelta;
            appendMappedLength( n, rBuffer, false );
            rBuffer.append( ' ' );
            appendMappedLength( nY, rBuffer, true );
            rBuffer.append( " v\n" );
        }
    }
    rBuffer.append( "S\n" );
}

/*
 *  class PDFWriterImpl
 */

#if defined USE_JAVA && defined MACOSX
PDFWriterImpl::PDFWriterImpl( const PDFWriter::PDFWriterContext& rContext, PDFWriterImpl *pParentWriter )
#else	// USE_JAVA && MACOSX
PDFWriterImpl::PDFWriterImpl( const PDFWriter::PDFWriterContext& rContext )
#endif	// USE_JAVA && MACOSX
        :
        m_pReferenceDevice( NULL ),
        m_aMapMode( MAP_POINT, Point(), Fraction( 1L, pointToPixel(1) ), Fraction( 1L, pointToPixel(1) ) ),
        m_nCurrentStructElement( 0 ),
        m_bEmitStructure( true ),
        m_bNewMCID( false ),
        m_nCurrentControl( -1 ),
        m_bEmbedStandardFonts( false ),
        m_nNextFID( 1 ),
        m_nInheritedPageWidth( 595 ),  // default A4
        m_nInheritedPageHeight( 842 ), // default A4
        m_eInheritedOrientation( PDFWriter::Portrait ),
        m_nCurrentPage( -1 ),
        m_nResourceDict( -1 ),
        m_nFontDictObject( -1 ),
#if defined USE_JAVA && defined MACOSX
        m_pParentWriter( pParentWriter ),
#endif	// USE_JAVA && MACOSX
        m_pCodec( NULL ),
        m_aDocDigest( rtl_digest_createMD5() ),
		m_aCipher( (rtlCipher)NULL ),
		m_aDigest( NULL ),
		m_bEncryptThisStream( false ),
		m_aDocID( 32 ),
		m_aCreationDateString( 64 ),
		m_aCreationMetaDateString( 64 ),
		m_pEncryptionBuffer( NULL ),
		m_nEncryptionBufferSize( 0 ),
        m_bIsPDF_A1( false )
{
#ifdef DO_TEST_PDF
    static bool bOnce = true;
    if( bOnce )
    {
        bOnce = false;
        doTestCode();
    }
#endif
    m_aContext = rContext;
    m_aStructure.push_back( PDFStructureElement() );
    m_aStructure[0].m_nOwnElement		= 0;
    m_aStructure[0].m_nParentElement	= 0;

    Font aFont;
    aFont.SetName( String( RTL_CONSTASCII_USTRINGPARAM( "Times" ) ) );
    aFont.SetSize( Size( 0, 12 ) );

    GraphicsState aState;
    aState.m_aMapMode		= m_aMapMode;
    aState.m_aFont			= aFont;
    m_aGraphicsStack.push_front( aState );

#if defined USE_JAVA && defined MACOSX
    if ( isReplayWriter() )
    {
        m_aSubsets = m_pParentWriter->m_aSubsets;
        m_nNextFID = m_pParentWriter->m_nNextFID;
    }
#endif	// USE_JAVA && MACOSX

    oslFileError  aError = osl_openFile( m_aContext.URL.pData, &m_aFile, osl_File_OpenFlag_Write | osl_File_OpenFlag_Create );
    if( aError != osl_File_E_None )
    {
        if( aError == osl_File_E_EXIST )
        {
            aError = osl_openFile( m_aContext.URL.pData, &m_aFile, osl_File_OpenFlag_Write );
            if( aError == osl_File_E_None )
                aError = osl_setFileSize( m_aFile, 0 );
        }
    }
    if( aError != osl_File_E_None )
        return;

    m_bOpen = true;

/* prepare the cypher engine, can be done in CTOR, free in DTOR */

	m_aCipher = rtl_cipher_createARCFOUR( rtl_Cipher_ModeStream );
	m_aDigest = rtl_digest_createMD5();

/* the size of the Codec default maximum */
	checkEncryptionBufferSize( 0x4000 );

    // write header
    OStringBuffer aBuffer( 20 );
    aBuffer.append( "%PDF-" );
    switch( m_aContext.Version )
    {
        case PDFWriter::PDF_1_2: aBuffer.append( "1.2" );break;
        case PDFWriter::PDF_1_3: aBuffer.append( "1.3" );break;
        case PDFWriter::PDF_A_1:
        default:
        case PDFWriter::PDF_1_4: aBuffer.append( "1.4" );break;
        case PDFWriter::PDF_1_5: aBuffer.append( "1.5" );break;
    }
    // append something binary as comment (suggested in PDF Reference)
    aBuffer.append( "\n%äüöß\n" );
    if( !writeBuffer( aBuffer.getStr(), aBuffer.getLength() ) )
    {
        osl_closeFile( m_aFile );
        m_bOpen = false;
        return;
    }

    // insert outline root
    m_aOutline.push_back( PDFOutlineEntry() );

    m_bIsPDF_A1 = (m_aContext.Version == PDFWriter::PDF_A_1);
    if( m_bIsPDF_A1 )
        m_aContext.Version = PDFWriter::PDF_1_4; //meaning we need PDF 1.4, PDF/A flavour

    m_bEmbedStandardFonts = m_aContext.EmbedStandardFonts;
}

PDFWriterImpl::~PDFWriterImpl()
{
    if( m_aDocDigest )
        rtl_digest_destroyMD5( m_aDocDigest );
    delete static_cast<VirtualDevice*>(m_pReferenceDevice);

	if( m_aCipher )
		rtl_cipher_destroyARCFOUR( m_aCipher );
	if( m_aDigest )
		rtl_digest_destroyMD5( m_aDigest );

    rtl_freeMemory( m_pEncryptionBuffer );
}

void PDFWriterImpl::setDocInfo( const PDFDocInfo& rInfo )
{
    m_aDocInfo.Title				= rInfo.Title;
    m_aDocInfo.Author				= rInfo.Author;
    m_aDocInfo.Subject				= rInfo.Subject;
    m_aDocInfo.Keywords				= rInfo.Keywords;
    m_aDocInfo.Creator				= rInfo.Creator;
    m_aDocInfo.Producer				= rInfo.Producer;

//build the document id
	rtl::OString aInfoValuesOut;
	OStringBuffer aID( 1024 );
	if( m_aDocInfo.Title.Len() )
		appendUnicodeTextString( m_aDocInfo.Title, aID );
	if( m_aDocInfo.Author.Len() )
		appendUnicodeTextString( m_aDocInfo.Author, aID );
	if( m_aDocInfo.Subject.Len() )
		appendUnicodeTextString( m_aDocInfo.Subject, aID );
	if( m_aDocInfo.Keywords.Len() )
		appendUnicodeTextString( m_aDocInfo.Keywords, aID );
	if( m_aDocInfo.Creator.Len() )
		appendUnicodeTextString( m_aDocInfo.Creator, aID );
	if( m_aDocInfo.Producer.Len() )
		appendUnicodeTextString( m_aDocInfo.Producer, aID );

	TimeValue aTVal, aGMT;
	oslDateTime aDT;
	osl_getSystemTime( &aGMT );
	osl_getLocalTimeFromSystemTime( &aGMT, &aTVal );
	osl_getDateTimeFromTimeValue( &aTVal, &aDT );
	m_aCreationDateString.append( "D:" );
	m_aCreationDateString.append( (sal_Char)('0' + ((aDT.Year/1000)%10)) );
	m_aCreationDateString.append( (sal_Char)('0' + ((aDT.Year/100)%10)) );
	m_aCreationDateString.append( (sal_Char)('0' + ((aDT.Year/10)%10)) );
	m_aCreationDateString.append( (sal_Char)('0' + ((aDT.Year)%10)) );
	m_aCreationDateString.append( (sal_Char)('0' + ((aDT.Month/10)%10)) );
	m_aCreationDateString.append( (sal_Char)('0' + ((aDT.Month)%10)) );
	m_aCreationDateString.append( (sal_Char)('0' + ((aDT.Day/10)%10)) );
	m_aCreationDateString.append( (sal_Char)('0' + ((aDT.Day)%10)) );
	m_aCreationDateString.append( (sal_Char)('0' + ((aDT.Hours/10)%10)) );
	m_aCreationDateString.append( (sal_Char)('0' + ((aDT.Hours)%10)) );
	m_aCreationDateString.append( (sal_Char)('0' + ((aDT.Minutes/10)%10)) );
	m_aCreationDateString.append( (sal_Char)('0' + ((aDT.Minutes)%10)) );
	m_aCreationDateString.append( (sal_Char)('0' + ((aDT.Seconds/10)%10)) );
	m_aCreationDateString.append( (sal_Char)('0' + ((aDT.Seconds)%10)) );
//--> i59651, we fill the Metadata date string as well, if PDF/A is requested
    if( m_bIsPDF_A1 )
    {
// according to ISO 19005-1:2005 6.7.3 the date is corrected for
// local time zone offset UTC only, whereas Acrobat 8 seems
// to use the localtime notation only
// according to a raccomandation in XMP Specification (Jan 2004, page 75)
// the Acrobat way seems the right approach
        m_aCreationMetaDateString.append( (sal_Char)('0' + ((aDT.Year/1000)%10)) );
        m_aCreationMetaDateString.append( (sal_Char)('0' + ((aDT.Year/100)%10)) );
        m_aCreationMetaDateString.append( (sal_Char)('0' + ((aDT.Year/10)%10)) );
        m_aCreationMetaDateString.append( (sal_Char)('0' + ((aDT.Year)%10)) );
        m_aCreationMetaDateString.append( "-" );
        m_aCreationMetaDateString.append( (sal_Char)('0' + ((aDT.Month/10)%10)) );
        m_aCreationMetaDateString.append( (sal_Char)('0' + ((aDT.Month)%10)) );
        m_aCreationMetaDateString.append( "-" );
        m_aCreationMetaDateString.append( (sal_Char)('0' + ((aDT.Day/10)%10)) );
        m_aCreationMetaDateString.append( (sal_Char)('0' + ((aDT.Day)%10)) );
        m_aCreationMetaDateString.append( "T" );
        m_aCreationMetaDateString.append( (sal_Char)('0' + ((aDT.Hours/10)%10)) );
        m_aCreationMetaDateString.append( (sal_Char)('0' + ((aDT.Hours)%10)) );
        m_aCreationMetaDateString.append( ":" );
        m_aCreationMetaDateString.append( (sal_Char)('0' + ((aDT.Minutes/10)%10)) );
        m_aCreationMetaDateString.append( (sal_Char)('0' + ((aDT.Minutes)%10)) );
        m_aCreationMetaDateString.append( ":" );
        m_aCreationMetaDateString.append( (sal_Char)('0' + ((aDT.Seconds/10)%10)) );
        m_aCreationMetaDateString.append( (sal_Char)('0' + ((aDT.Seconds)%10)) );
    }
	sal_uInt32 nDelta = 0;
	if( aGMT.Seconds > aTVal.Seconds )
	{
		m_aCreationDateString.append( "-" );
		nDelta = aGMT.Seconds-aTVal.Seconds;
        if( m_bIsPDF_A1 )
            m_aCreationMetaDateString.append( "-" );
	}
	else if( aGMT.Seconds < aTVal.Seconds )
	{
		m_aCreationDateString.append( "+" );
		nDelta = aTVal.Seconds-aGMT.Seconds;
        if( m_bIsPDF_A1 )
            m_aCreationMetaDateString.append( "+" );
	}
	else
    {
		m_aCreationDateString.append( "Z" );
        if( m_bIsPDF_A1 )
            m_aCreationMetaDateString.append( "Z" );

    }
	if( nDelta )
	{
		m_aCreationDateString.append( (sal_Char)('0' + ((nDelta/36000)%10)) );
		m_aCreationDateString.append( (sal_Char)('0' + ((nDelta/3600)%10)) );
		m_aCreationDateString.append( "'" );
		m_aCreationDateString.append( (sal_Char)('0' + ((nDelta/600)%6)) );
		m_aCreationDateString.append( (sal_Char)('0' + ((nDelta/60)%10)) );
        if( m_bIsPDF_A1 )
        {
            m_aCreationMetaDateString.append( (sal_Char)('0' + ((nDelta/36000)%10)) );
            m_aCreationMetaDateString.append( (sal_Char)('0' + ((nDelta/3600)%10)) );
            m_aCreationMetaDateString.append( ":" );
            m_aCreationMetaDateString.append( (sal_Char)('0' + ((nDelta/600)%6)) );
            m_aCreationMetaDateString.append( (sal_Char)('0' + ((nDelta/60)%10)) );
        }
	}
	m_aCreationDateString.append( "'" );
	aID.append( m_aCreationDateString.getStr(), m_aCreationDateString.getLength() );

	aInfoValuesOut = aID.makeStringAndClear();

	DBG_ASSERT( m_aDigest != NULL, "PDFWrite_Impl::setDocInfo: cannot obtain a digest object !" );

	m_aDocID.setLength( 0 );
	if( m_aDigest )
	{
		osl_getSystemTime( &aGMT );
		rtlDigestError nError = rtl_digest_updateMD5( m_aDigest, &aGMT, sizeof( aGMT ) );
		if( nError == rtl_Digest_E_None )
			nError = rtl_digest_updateMD5( m_aDigest, m_aContext.URL.getStr(), m_aContext.URL.getLength()*sizeof(sal_Unicode) );
		if( nError == rtl_Digest_E_None )
			nError = rtl_digest_updateMD5( m_aDigest, aInfoValuesOut.getStr(), aInfoValuesOut.getLength() );
		if( nError == rtl_Digest_E_None )
		{
//the binary form of the doc id is needed for encryption stuff
			rtl_digest_getMD5( m_aDigest, m_nDocID, 16 );
			for( unsigned int i = 0; i < 16; i++ )
				appendHex( m_nDocID[i], m_aDocID );
		}
	}
}

/* i12626 methods */
/*
check if the Unicode string must be encrypted or not, perform the requested task,
append the string as unicode hex, encrypted if needed
 */
inline void PDFWriterImpl::appendUnicodeTextStringEncrypt( const rtl::OUString& rInString, const sal_Int32 nInObjectNumber, OStringBuffer& rOutBuffer )
{
	rOutBuffer.append( "<" );
	if( m_aContext.Encrypt )
	{
		const sal_Unicode* pStr = rInString.getStr();
		sal_Int32 nLen = rInString.getLength();
//prepare a unicode string, encrypt it
		if( checkEncryptionBufferSize( nLen*2 ) )
		{
			enableStringEncryption( nInObjectNumber );
			register sal_uInt8 *pCopy = m_pEncryptionBuffer;
			sal_Int32 nChars = 2;
			*pCopy++ = 0xFE;
			*pCopy++ = 0xFF;
// we need to prepare a byte stream from the unicode string buffer
			for( register int i = 0; i < nLen; i++ )
			{
				register sal_Unicode aUnChar = pStr[i];
				*pCopy++ = (sal_uInt8)( aUnChar >> 8 );
				*pCopy++ = (sal_uInt8)( aUnChar & 255 );
				nChars += 2;
			}
//encrypt in place
			rtl_cipher_encodeARCFOUR( m_aCipher, m_pEncryptionBuffer, nChars, m_pEncryptionBuffer, nChars );
//now append, hexadecimal (appendHex), the encrypted result
			for(register int i = 0; i < nChars; i++)
				appendHex( m_pEncryptionBuffer[i], rOutBuffer );
		}
	}
	else
		appendUnicodeTextString( rInString, rOutBuffer );
	rOutBuffer.append( ">" );
}

inline void PDFWriterImpl::appendLiteralStringEncrypt( rtl::OStringBuffer& rInString, const sal_Int32 nInObjectNumber, rtl::OStringBuffer& rOutBuffer )
{
	rOutBuffer.append( "(" );
	sal_Int32 nChars = rInString.getLength();
//check for encryption, if ok, encrypt the string, then convert with appndLiteralString
	if( m_aContext.Encrypt && checkEncryptionBufferSize( nChars ) )
	{
//encrypt the string in a buffer, then append it
		enableStringEncryption( nInObjectNumber );
		rtl_cipher_encodeARCFOUR( m_aCipher, rInString.getStr(), nChars, m_pEncryptionBuffer, nChars );
		appendLiteralString( (const sal_Char*)m_pEncryptionBuffer, nChars, rOutBuffer );
	}
	else
        appendLiteralString( rInString.getStr(), nChars , rOutBuffer );
	rOutBuffer.append( ")" );
}

inline void PDFWriterImpl::appendLiteralStringEncrypt( const rtl::OString& rInString, const sal_Int32 nInObjectNumber, rtl::OStringBuffer& rOutBuffer )
{
	rtl::OStringBuffer aBufferString( rInString );
	appendLiteralStringEncrypt( aBufferString, nInObjectNumber, rOutBuffer);
}

inline void PDFWriterImpl::appendLiteralStringEncrypt( const rtl::OUString& rInString, const sal_Int32 nInObjectNumber, rtl::OStringBuffer& rOutBuffer )
{
	rtl::OString aBufferString( rtl::OUStringToOString( rInString, RTL_TEXTENCODING_ASCII_US ) );
	appendLiteralStringEncrypt( aBufferString, nInObjectNumber, rOutBuffer);
}

/* end i12626 methods */

void PDFWriterImpl::emitComment( const char* pComment )
{
    OStringBuffer aLine( 64 );
    aLine.append( "% " );
    aLine.append( (const sal_Char*)pComment );
    aLine.append( "\n" );
    writeBuffer( aLine.getStr(), aLine.getLength() );
}

bool PDFWriterImpl::compressStream( SvMemoryStream* pStream )
{
#ifndef DEBUG_DISABLE_PDFCOMPRESSION
    pStream->Seek( STREAM_SEEK_TO_END );
    ULONG nEndPos = pStream->Tell();
    pStream->Seek( STREAM_SEEK_TO_BEGIN );
    ZCodec* pCodec = new ZCodec( 0x4000, 0x4000 );
    SvMemoryStream aStream;
    pCodec->BeginCompression();
    pCodec->Write( aStream, (const BYTE*)pStream->GetData(), nEndPos );
    pCodec->EndCompression();
    delete pCodec;
    nEndPos = aStream.Tell();
    pStream->Seek( STREAM_SEEK_TO_BEGIN );
    aStream.Seek( STREAM_SEEK_TO_BEGIN );
    pStream->SetStreamSize( nEndPos );
    pStream->Write( aStream.GetData(), nEndPos );
    return true;
#else
    (void)pStream;
    return false;
#endif
}

void PDFWriterImpl::beginCompression()
{
#ifndef DEBUG_DISABLE_PDFCOMPRESSION
    m_pCodec = new ZCodec( 0x4000, 0x4000 );
    m_pMemStream = new SvMemoryStream();
    m_pCodec->BeginCompression();
#endif
}

void PDFWriterImpl::endCompression()
{
#ifndef DEBUG_DISABLE_PDFCOMPRESSION
    if( m_pCodec )
    {
        m_pCodec->EndCompression();
        delete m_pCodec;
        m_pCodec = NULL;
        sal_uInt64 nLen = m_pMemStream->Tell();
        m_pMemStream->Seek( 0 );
        writeBuffer( m_pMemStream->GetData(), nLen );
        delete m_pMemStream;
        m_pMemStream = NULL;
    }
#endif
}

bool PDFWriterImpl::writeBuffer( const void* pBuffer, sal_uInt64 nBytes )
{
    if( ! m_bOpen ) // we are already down the drain
        return false;

#if defined USE_JAVA && defined MACOSX
    if ( !isReplayWriter() )
        return true;
#endif	// USE_JAVA && MACOSX

    if( ! nBytes ) // huh ?
        return true;

    if( m_aOutputStreams.begin() != m_aOutputStreams.end() )
    {
        m_aOutputStreams.front().m_pStream->Seek( STREAM_SEEK_TO_END );
        m_aOutputStreams.front().m_pStream->Write( pBuffer, sal::static_int_cast<sal_Size>(nBytes) );
        return true;
    }

    sal_uInt64 nWritten;
    if( m_pCodec )
    {
        m_pCodec->Write( *m_pMemStream, static_cast<const BYTE*>(pBuffer), (ULONG)nBytes );
        nWritten = nBytes;
    }
    else
    {
		sal_Bool  buffOK = sal_True;
		if( m_bEncryptThisStream )
		{
/* implement the encryption part of the PDF spec encryption algorithm 3.1 */
			if( ( buffOK = checkEncryptionBufferSize( static_cast<sal_Int32>(nBytes) ) ) != sal_False )
				rtl_cipher_encodeARCFOUR( m_aCipher,
                                          (sal_uInt8*)pBuffer, static_cast<sal_Size>(nBytes),
                                          m_pEncryptionBuffer, static_cast<sal_Size>(nBytes) );
		}

        const void* pWriteBuffer = ( m_bEncryptThisStream && buffOK ) ? m_pEncryptionBuffer  : pBuffer;
        if( m_aDocDigest )
            rtl_digest_updateMD5( m_aDocDigest, pWriteBuffer, static_cast<sal_uInt32>(nBytes) );

        if( osl_writeFile( m_aFile,
                           pWriteBuffer,
                           nBytes, &nWritten ) != osl_File_E_None )
            nWritten = 0;

        if( nWritten != nBytes )
        {
            osl_closeFile( m_aFile );
            m_bOpen = false;
        }
    }

    return nWritten == nBytes;
}

OutputDevice* PDFWriterImpl::getReferenceDevice()
{
    if( ! m_pReferenceDevice )
    {
        VirtualDevice*  pVDev = new VirtualDevice( 0 );

        m_pReferenceDevice = pVDev;

        pVDev->SetReferenceDevice( VirtualDevice::REFDEV_MODE_PDF1 );

        pVDev->SetOutputSizePixel( Size( 640, 480 ) );
        pVDev->SetMapMode( MAP_MM );

        m_pReferenceDevice->mpPDFWriter = this;
        m_pReferenceDevice->ImplUpdateFontData( TRUE );
    }
    return m_pReferenceDevice;
}

class ImplPdfBuiltinFontData : public ImplFontData
{
private:
    const PDFWriterImpl::BuiltinFont& mrBuiltin;

public:
    enum {PDF_FONT_MAGIC = 0xBDFF0A1C };
                                        ImplPdfBuiltinFontData( const PDFWriterImpl::BuiltinFont& );
    const PDFWriterImpl::BuiltinFont*   GetBuiltinFont() const  { return &mrBuiltin; }

    virtual ImplFontData*               Clone() const { return new ImplPdfBuiltinFontData(*this); }
    virtual ImplFontEntry*              CreateFontInstance( ImplFontSelectData& ) const;
    virtual sal_IntPtr                  GetFontId() const { return reinterpret_cast<sal_IntPtr>(&mrBuiltin); }
};

inline const ImplPdfBuiltinFontData* GetPdfFontData( const ImplFontData* pFontData )
{
    const ImplPdfBuiltinFontData* pFD = NULL;
    if( pFontData && pFontData->CheckMagic( ImplPdfBuiltinFontData::PDF_FONT_MAGIC ) )
        pFD = static_cast<const ImplPdfBuiltinFontData*>( pFontData );
    return pFD;
}

static ImplDevFontAttributes GetDevFontAttributes( const PDFWriterImpl::BuiltinFont& rBuiltin )
{
    ImplDevFontAttributes aDFA;
    aDFA.maName         = String::CreateFromAscii( rBuiltin.m_pName );
    aDFA.maStyleName    = String::CreateFromAscii( rBuiltin.m_pStyleName );
    aDFA.meFamily       = rBuiltin.m_eFamily;
    aDFA.mbSymbolFlag   = (rBuiltin.m_eCharSet == RTL_TEXTENCODING_SYMBOL);
    aDFA.mePitch        = rBuiltin.m_ePitch;
    aDFA.meWeight       = rBuiltin.m_eWeight;
    aDFA.meItalic       = rBuiltin.m_eItalic;
    aDFA.meWidthType    = rBuiltin.m_eWidthType;

    aDFA.mbOrientation  = true;
    aDFA.mbDevice       = true;
    aDFA.mnQuality      = 50000;
    aDFA.mbSubsettable  = false;
    aDFA.mbEmbeddable   = false;
    return aDFA;
}

ImplPdfBuiltinFontData::ImplPdfBuiltinFontData( const PDFWriterImpl::BuiltinFont& rBuiltin )
:   ImplFontData( GetDevFontAttributes(rBuiltin), PDF_FONT_MAGIC ),
    mrBuiltin( rBuiltin )
{}

ImplFontEntry* ImplPdfBuiltinFontData::CreateFontInstance( ImplFontSelectData& rFSD ) const
{
    ImplFontEntry* pEntry = new ImplFontEntry( rFSD );
    return pEntry;
}

ImplDevFontList* PDFWriterImpl::filterDevFontList( ImplDevFontList* pFontList )
{
#if defined USE_JAVA && defined MACOSX
    // Fix bug 3481 by not using a separate font list
    return pFontList;
#else	// USE_JAVA && MACOSX
    DBG_ASSERT( m_aSubsets.size() == 0, "Fonts changing during PDF generation, document will be invalid" );
    ImplDevFontList* pFiltered = pFontList->Clone( true, true );

    // append the PDF builtin fonts
    if( !m_bIsPDF_A1 && !m_bEmbedStandardFonts)
        for( unsigned int i = 0; i < sizeof(m_aBuiltinFonts)/sizeof(m_aBuiltinFonts[0]); i++ )
        {
            ImplFontData* pNewData = new ImplPdfBuiltinFontData( m_aBuiltinFonts[i] );
            pFiltered->Add( pNewData );
        }
    return pFiltered;
#endif	// USE_JAVA && MACOSX
}

bool PDFWriterImpl::isBuiltinFont( const ImplFontData* pFont ) const
{
    const ImplPdfBuiltinFontData* pFD = GetPdfFontData( pFont );
    return (pFD != NULL);
}

void PDFWriterImpl::getFontMetric( ImplFontSelectData* pSelect, ImplFontMetricData* pMetric ) const
{
    const ImplPdfBuiltinFontData* pFD = GetPdfFontData( pSelect->mpFontData );
    if( !pFD )
        return;
    const BuiltinFont* pBuiltinFont = pFD->GetBuiltinFont();

    pMetric->mnOrientation	= sal::static_int_cast<short>(pSelect->mnOrientation);
    pMetric->meFamily		= pBuiltinFont->m_eFamily;
    pMetric->mePitch		= pBuiltinFont->m_ePitch;
    pMetric->meWeight		= pBuiltinFont->m_eWeight;
    pMetric->meItalic		= pBuiltinFont->m_eItalic;
    pMetric->mbSymbolFlag	= pFD->IsSymbolFont();
    pMetric->mnWidth		= pSelect->mnHeight;
    pMetric->mnAscent		= ( pSelect->mnHeight * +pBuiltinFont->m_nAscent + 500 ) / 1000;
    pMetric->mnDescent		= ( pSelect->mnHeight * -pBuiltinFont->m_nDescent + 500 ) / 1000;
    pMetric->mnIntLeading	= 0;
    pMetric->mnExtLeading	= 0;
    pMetric->mnSlant		= 0;
    pMetric->mbScalableFont	= true;
    pMetric->mbDevice		= true;
}

// -----------------------------------------------------------------------

namespace vcl {

class PDFSalLayout : public GenericSalLayout
{
    PDFWriterImpl&  mrPDFWriterImpl;
    const PDFWriterImpl::BuiltinFont& mrBuiltinFont;
    bool            mbIsSymbolFont;
    long            mnPixelPerEM;
    String          maOrigText;

public:
                    PDFSalLayout( PDFWriterImpl&,
                                  const PDFWriterImpl::BuiltinFont&,
                                  long nPixelPerEM, int nOrientation );

    void            SetText( const String& rText )  { maOrigText = rText; }
    virtual bool    LayoutText( ImplLayoutArgs& );
    virtual void    InitFont() const;
    virtual void    DrawText( SalGraphics& ) const;
};

}

// -----------------------------------------------------------------------

PDFSalLayout::PDFSalLayout( PDFWriterImpl& rPDFWriterImpl,
    const PDFWriterImpl::BuiltinFont& rBuiltinFont,
    long nPixelPerEM, int nOrientation )
:   mrPDFWriterImpl( rPDFWriterImpl ),
    mrBuiltinFont( rBuiltinFont ),
    mnPixelPerEM( nPixelPerEM )
{
    mbIsSymbolFont = (rBuiltinFont.m_eCharSet == RTL_TEXTENCODING_SYMBOL);
    SetOrientation( nOrientation );
}

// -----------------------------------------------------------------------

bool PDFSalLayout::LayoutText( ImplLayoutArgs& rArgs )
{
    const String aText( rArgs.mpStr+rArgs.mnMinCharPos, sal::static_int_cast<xub_StrLen>(rArgs.mnEndCharPos-rArgs.mnMinCharPos) );
    SetText( aText );
    SetUnitsPerPixel( 1000 );

    rtl_UnicodeToTextConverter aConv = rtl_createTextToUnicodeConverter( RTL_TEXTENCODING_MS_1252 );

    Point aNewPos( 0, 0 );
    bool bRightToLeft;
    for( int nCharPos = -1; rArgs.GetNextPos( &nCharPos, &bRightToLeft ); )
    {
        // TODO: handle unicode surrogates
	// on the other hand builtin fonts don't support them anyway
        sal_Unicode cChar = rArgs.mpStr[ nCharPos ];
        if( bRightToLeft )
            cChar = static_cast<sal_Unicode>(GetMirroredChar( cChar ));

        if( cChar & 0xff00 )
        {
            // some characters can be used by conversion
            if( (cChar >= 0xf000) && mbIsSymbolFont )
                cChar -= 0xf000;
            else
            {
                sal_Char aBuf[4];
                sal_uInt32 nInfo;
                sal_Size nSrcCvtChars;

                sal_Size nConv = rtl_convertUnicodeToText( aConv,
                                                           NULL,
                                                           &cChar, 1,
                                                           aBuf, 1,
                                                           RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR,
                                                           &nInfo, &nSrcCvtChars );
                // check whether conversion was possible
                // else fallback font is needed as the standard fonts
                // are handled via WinAnsi encoding
                if( nConv > 0 )
                    cChar = ((sal_Unicode)aBuf[0]) & 0x00ff;
            }
        }
        if( cChar & 0xff00 )
        {
            cChar = 0;   // NotDef glyph
            rArgs.NeedFallback( nCharPos, bRightToLeft );
        }

        long nGlyphWidth = (long)mrBuiltinFont.m_aWidths[cChar] * mnPixelPerEM;
        long nGlyphFlags = (nGlyphWidth > 0) ? 0 : GlyphItem::IS_IN_CLUSTER;
        if( bRightToLeft )
            nGlyphFlags |= GlyphItem::IS_RTL_GLYPH;
        // TODO: get kerning from builtin fonts
        GlyphItem aGI( nCharPos, cChar, aNewPos, nGlyphFlags, nGlyphWidth );
        AppendGlyph( aGI );

        aNewPos.X() += nGlyphWidth;
    }

    rtl_destroyUnicodeToTextConverter( aConv );

    return true;
}

// -----------------------------------------------------------------------

void PDFSalLayout::InitFont() const
{
    // TODO: recreate font with all its attributes
}

// -----------------------------------------------------------------------

void PDFSalLayout::DrawText( SalGraphics& ) const
{
    mrPDFWriterImpl.drawLayout( *const_cast<PDFSalLayout*>(this), maOrigText, true );
}

// -----------------------------------------------------------------------

SalLayout* PDFWriterImpl::GetTextLayout( ImplLayoutArgs& rArgs, ImplFontSelectData* pSelect )
{
    DBG_ASSERT( (pSelect->mpFontData != NULL),
        "PDFWriterImpl::GetTextLayout mpFontData is NULL" );

    const ImplPdfBuiltinFontData* pFD = GetPdfFontData( pSelect->mpFontData );
    if( !pFD )
        return NULL;
    const BuiltinFont* pBuiltinFont = pFD->GetBuiltinFont();

    long nPixelPerEM = pSelect->mnWidth ? pSelect->mnWidth : pSelect->mnHeight;
    int nOrientation = pSelect->mnOrientation;
    PDFSalLayout* pLayout = new PDFSalLayout( *this, *pBuiltinFont, nPixelPerEM, nOrientation );
    pLayout->SetText( rArgs.mpStr );
    return pLayout;
}

sal_Int32 PDFWriterImpl::newPage( sal_Int32 nPageWidth, sal_Int32 nPageHeight, PDFWriter::Orientation eOrientation )
{
    if( m_aContext.Encrypt && m_aPages.empty() )
        initEncryption();

    endPage();
    m_nCurrentPage = m_aPages.size();
    m_aPages.push_back( PDFPage(this, nPageWidth, nPageHeight, eOrientation ) );
    m_aPages.back().m_nPageIndex = m_nCurrentPage;
    m_aPages.back().beginStream();

    // setup global graphics state
    // linewidth is "1 pixel" by default
    OStringBuffer aBuf( 16 );
    appendDouble( 72.0/double(getReferenceDevice()->ImplGetDPIX()), aBuf );
    aBuf.append( " w\n" );
    writeBuffer( aBuf.getStr(), aBuf.getLength() );

    return m_nCurrentPage;
}

void PDFWriterImpl::endPage()
{
    if( m_aPages.begin() != m_aPages.end() )
    {
        // close eventual MC sequence
        endStructureElementMCSeq();

        // sanity check
        if( m_aOutputStreams.begin() != m_aOutputStreams.end() )
        {
            DBG_ERROR( "redirection across pages !!!" );
            m_aOutputStreams.clear(); // leak !
            m_aMapMode.SetOrigin( Point() );
        }

        m_aGraphicsStack.clear();
        m_aGraphicsStack.push_back( GraphicsState() );

        // this should pop the PDF graphics stack if necessary
        updateGraphicsState();

        m_aPages.back().endStream();

        // reset the default font
        Font aFont;
        aFont.SetName( String( RTL_CONSTASCII_USTRINGPARAM( "Times" ) ) );
        aFont.SetSize( Size( 0, 12 ) );

        m_aCurrentPDFState = m_aGraphicsStack.front();
        m_aGraphicsStack.front().m_aFont =  aFont;

        for( std::list<BitmapEmit>::iterator it = m_aBitmaps.begin();
             it != m_aBitmaps.end(); ++it )
        {
            if( ! it->m_aBitmap.IsEmpty() )
            {
                writeBitmapObject( *it );
                it->m_aBitmap = BitmapEx();
            }
        }
        for( std::list<JPGEmit>::iterator jpeg = m_aJPGs.begin(); jpeg != m_aJPGs.end(); ++jpeg )
        {
            if( jpeg->m_pStream )
            {
                writeJPG( *jpeg );
                delete jpeg->m_pStream;
                jpeg->m_pStream = NULL;
                jpeg->m_aMask = Bitmap();
            }
        }
        for( std::list<TransparencyEmit>::iterator t = m_aTransparentObjects.begin();
             t != m_aTransparentObjects.end(); ++t )
        {
            if( t->m_pContentStream )
            {
                writeTransparentObject( *t );
                delete t->m_pContentStream;
                t->m_pContentStream = NULL;
            }
        }
    }
}

sal_Int32 PDFWriterImpl::createObject()
{
    m_aObjects.push_back( ~0U );
    return m_aObjects.size();
}

bool PDFWriterImpl::updateObject( sal_Int32 n )
{
    if( ! m_bOpen )
        return false;

    sal_uInt64 nOffset = ~0U;
    oslFileError aError = osl_getFilePos( m_aFile, &nOffset );
    DBG_ASSERT( aError == osl_File_E_None, "could not register object" );
    if( aError != osl_File_E_None )
    {
        osl_closeFile( m_aFile );
        m_bOpen = false;
    }
    m_aObjects[ n-1 ] = nOffset;
    return aError == osl_File_E_None;
}

#define CHECK_RETURN( x ) if( !(x) ) return 0

sal_Int32 PDFWriterImpl::emitStructParentTree( sal_Int32 nObject )
{
    if( nObject > 0 )
    {
        OStringBuffer aLine( 1024 );

        aLine.append( nObject );
        aLine.append( " 0 obj\n"
                      "<</Nums[\n" );
        sal_Int32 nTreeItems = m_aStructParentTree.size();
        for( sal_Int32 n = 0; n < nTreeItems; n++ )
        {
            aLine.append( n );
            aLine.append( ' ' );
            aLine.append( m_aStructParentTree[n] );
            aLine.append( "\n" );
        }
        aLine.append( "]>>\nendobj\n\n" );
        CHECK_RETURN( updateObject( nObject ) );
        CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );
    }
    return nObject;
}

const sal_Char* PDFWriterImpl::getAttributeTag( PDFWriter::StructAttribute eAttr )
{
    static std::map< PDFWriter::StructAttribute, const char* > aAttributeStrings;
    // fill maps once
    if( aAttributeStrings.empty() )
    {
        aAttributeStrings[ PDFWriter::Placement ]			= "Placement";
        aAttributeStrings[ PDFWriter::WritingMode ]			= "WritingMode";
        aAttributeStrings[ PDFWriter::SpaceBefore ]			= "SpaceBefore";
        aAttributeStrings[ PDFWriter::SpaceAfter ]			= "SpaceAfter";
        aAttributeStrings[ PDFWriter::StartIndent ]			= "StartIndent";
        aAttributeStrings[ PDFWriter::EndIndent ]			= "EndIndent";
        aAttributeStrings[ PDFWriter::TextIndent ]			= "TextIndent";
        aAttributeStrings[ PDFWriter::TextAlign ]			= "TextAlign";
        aAttributeStrings[ PDFWriter::Width ]				= "Width";
        aAttributeStrings[ PDFWriter::Height ]				= "Height";
        aAttributeStrings[ PDFWriter::BlockAlign ]			= "BlockAlign";
        aAttributeStrings[ PDFWriter::InlineAlign ]			= "InlineAlign";
        aAttributeStrings[ PDFWriter::LineHeight ]			= "LineHeight";
        aAttributeStrings[ PDFWriter::BaselineShift ]		= "BaselineShift";
        aAttributeStrings[ PDFWriter::TextDecorationType ]	= "TextDecorationType";
        aAttributeStrings[ PDFWriter::ListNumbering ]		= "ListNumbering";
        aAttributeStrings[ PDFWriter::RowSpan ]				= "RowSpan";
        aAttributeStrings[ PDFWriter::ColSpan ]				= "ColSpan";
        aAttributeStrings[ PDFWriter::LinkAnnotation ]      = "LinkAnnotation";
    }

    std::map< PDFWriter::StructAttribute, const char* >::const_iterator it =
        aAttributeStrings.find( eAttr );

#if OSL_DEBUG_LEVEL > 1
    if( it == aAttributeStrings.end() )
        fprintf( stderr, "invalid PDFWriter::StructAttribute %d\n", eAttr );
#endif

    return it != aAttributeStrings.end() ? it->second : "";
}

const sal_Char* PDFWriterImpl::getAttributeValueTag( PDFWriter::StructAttributeValue eVal )
{
    static std::map< PDFWriter::StructAttributeValue, const char* > aValueStrings;

    if( aValueStrings.empty() )
    {
        aValueStrings[ PDFWriter::NONE ]					= "None";
        aValueStrings[ PDFWriter::Block ]					= "Block";
        aValueStrings[ PDFWriter::Inline ]					= "Inline";
        aValueStrings[ PDFWriter::Before ]					= "Before";
        aValueStrings[ PDFWriter::After ]					= "After";
        aValueStrings[ PDFWriter::Start ]					= "Start";
        aValueStrings[ PDFWriter::End ]						= "End";
        aValueStrings[ PDFWriter::LrTb ]					= "LrTb";
        aValueStrings[ PDFWriter::RlTb ]					= "RlTb";
        aValueStrings[ PDFWriter::TbRl ]					= "TbRl";
        aValueStrings[ PDFWriter::Center ]					= "Center";
        aValueStrings[ PDFWriter::Justify ]					= "Justify";
        aValueStrings[ PDFWriter::Auto ]					= "Auto";
        aValueStrings[ PDFWriter::Middle ]					= "Middle";
        aValueStrings[ PDFWriter::Normal ]					= "Normal";
        aValueStrings[ PDFWriter::Underline ]				= "Underline";
		aValueStrings[ PDFWriter::Overline ]				= "Overline";
        aValueStrings[ PDFWriter::LineThrough ]				= "LineThrough";
        aValueStrings[ PDFWriter::Disc ]					= "Disc";
        aValueStrings[ PDFWriter::Circle ]					= "Circle";
        aValueStrings[ PDFWriter::Square ]					= "Square";
        aValueStrings[ PDFWriter::Decimal ]					= "Decimal";
        aValueStrings[ PDFWriter::UpperRoman ]				= "UpperRoman";
        aValueStrings[ PDFWriter::LowerRoman ]				= "LowerRoman";
        aValueStrings[ PDFWriter::UpperAlpha ]				= "UpperAlpha";
        aValueStrings[ PDFWriter::LowerAlpha ]				= "LowerAlpha";
    }

    std::map< PDFWriter::StructAttributeValue, const char* >::const_iterator it =
        aValueStrings.find( eVal );

#if OSL_DEBUG_LEVEL > 1
    if( it == aValueStrings.end() )
        fprintf( stderr, "invalid PDFWriter::StructAttributeValue %d\n", eVal );
#endif

    return it != aValueStrings.end() ? it->second : "";
}

static void appendStructureAttributeLine( PDFWriter::StructAttribute i_eAttr, const PDFWriterImpl::PDFStructureAttribute& i_rVal, OStringBuffer& o_rLine, bool i_bIsFixedInt )
{
    o_rLine.append( "/" );
    o_rLine.append( PDFWriterImpl::getAttributeTag( i_eAttr ) );

    if( i_rVal.eValue != PDFWriter::Invalid )
    {
        o_rLine.append( "/" );
        o_rLine.append( PDFWriterImpl::getAttributeValueTag( i_rVal.eValue ) );
    }
    else
    {
        // numerical value
        o_rLine.append( " " );
        if( i_bIsFixedInt )
            appendFixedInt( i_rVal.nValue, o_rLine );
        else
            o_rLine.append( i_rVal.nValue );
    }
    o_rLine.append( "\n" );
}

OString PDFWriterImpl::emitStructureAttributes( PDFStructureElement& i_rEle )
{
    // create layout, list and table attribute sets
    OStringBuffer aLayout(256), aList(64), aTable(64);
    for( PDFStructAttributes::const_iterator it = i_rEle.m_aAttributes.begin();
         it != i_rEle.m_aAttributes.end(); ++it )
    {
        if( it->first == PDFWriter::ListNumbering )
            appendStructureAttributeLine( it->first, it->second, aList, true );
        else if( it->first == PDFWriter::RowSpan ||
                 it->first == PDFWriter::ColSpan )
            appendStructureAttributeLine( it->first, it->second, aTable, false );
        else if( it->first == PDFWriter::LinkAnnotation )
        {
            sal_Int32 nLink = it->second.nValue;
            std::map< sal_Int32, sal_Int32 >::const_iterator link_it =
                m_aLinkPropertyMap.find( nLink );
            if( link_it != m_aLinkPropertyMap.end() )
                nLink = link_it->second;
            if( nLink >= 0 && nLink < (sal_Int32)m_aLinks.size() )
            {
                // update struct parent of link
                OStringBuffer aStructParentEntry( 32 );
                aStructParentEntry.append( i_rEle.m_nObject );
                aStructParentEntry.append( " 0 R" );
                m_aStructParentTree.push_back( aStructParentEntry.makeStringAndClear() );
                m_aLinks[ nLink ].m_nStructParent = m_aStructParentTree.size()-1;

                sal_Int32 nRefObject = createObject();
                OStringBuffer aRef( 256 );
                aRef.append( nRefObject );
                aRef.append( " 0 obj\n"
                             "<</Type/OBJR/Obj " );
                aRef.append( m_aLinks[ nLink ].m_nObject );
                aRef.append( " 0 R>>\n"
                             "endobj\n\n"
                             );
                updateObject( nRefObject );
                writeBuffer( aRef.getStr(), aRef.getLength() );

                i_rEle.m_aKids.push_back( PDFStructureElementKid( nRefObject ) );
            }
            else
            {
                DBG_ERROR( "unresolved link id for Link structure" );
#if OSL_DEBUG_LEVEL > 1
                fprintf( stderr, "unresolved link id %" SAL_PRIdINT32 " for Link structure\n", nLink );
                {
                    OStringBuffer aLine( "unresolved link id " );
                    aLine.append( nLink );
                    aLine.append( " for Link structure" );
                    emitComment( aLine.getStr() );
                }
#endif
            }
        }
        else
            appendStructureAttributeLine( it->first, it->second, aLayout, true );
    }
    if( ! i_rEle.m_aBBox.IsEmpty() )
    {
        aLayout.append( "/BBox[" );
        appendFixedInt( i_rEle.m_aBBox.Left(), aLayout );
        aLayout.append( " " );
        appendFixedInt( i_rEle.m_aBBox.Top(), aLayout );
        aLayout.append( " " );
        appendFixedInt( i_rEle.m_aBBox.Right(), aLayout );
        aLayout.append( " " );
        appendFixedInt( i_rEle.m_aBBox.Bottom(), aLayout );
        aLayout.append( "]\n" );
    }

    std::vector< sal_Int32 > aAttribObjects;
    if( aLayout.getLength() )
    {
        aAttribObjects.push_back( createObject() );
        updateObject( aAttribObjects.back() );
        OStringBuffer aObj( 64 );
        aObj.append( aAttribObjects.back() );
        aObj.append( " 0 obj\n"
                     "<</O/Layout\n" );
        aLayout.append( ">>\nendobj\n\n" );
        writeBuffer( aObj.getStr(), aObj.getLength() );
        writeBuffer( aLayout.getStr(), aLayout.getLength() );
    }
    if( aList.getLength() )
    {
        aAttribObjects.push_back( createObject() );
        updateObject( aAttribObjects.back() );
        OStringBuffer aObj( 64 );
        aObj.append( aAttribObjects.back() );
        aObj.append( " 0 obj\n"
                     "<</O/List\n" );
        aList.append( ">>\nendobj\n\n" );
        writeBuffer( aObj.getStr(), aObj.getLength() );
        writeBuffer( aList.getStr(), aList.getLength() );
    }
    if( aTable.getLength() )
    {
        aAttribObjects.push_back( createObject() );
        updateObject( aAttribObjects.back() );
        OStringBuffer aObj( 64 );
        aObj.append( aAttribObjects.back() );
        aObj.append( " 0 obj\n"
                     "<</O/Table\n" );
        aTable.append( ">>\nendobj\n\n" );
        writeBuffer( aObj.getStr(), aObj.getLength() );
        writeBuffer( aTable.getStr(), aTable.getLength() );
    }

    OStringBuffer aRet( 64 );
    if( aAttribObjects.size() > 1 )
        aRet.append( " [" );
    for( std::vector< sal_Int32 >::const_iterator at_it = aAttribObjects.begin();
         at_it != aAttribObjects.end(); ++at_it )
    {
        aRet.append( " " );
        aRet.append( *at_it );
        aRet.append( " 0 R" );
    }
    if( aAttribObjects.size() > 1 )
        aRet.append( " ]" );
    return aRet.makeStringAndClear();
}

sal_Int32 PDFWriterImpl::emitStructure( PDFStructureElement& rEle )
{
    if(
       // do not emit NonStruct and its children
       rEle.m_eType == PDFWriter::NonStructElement &&
       rEle.m_nOwnElement != rEle.m_nParentElement // but of course emit the struct tree root
       )
        return 0;

    for( std::list< sal_Int32 >::const_iterator it = rEle.m_aChildren.begin(); it != rEle.m_aChildren.end(); ++it )
    {
        if( *it > 0 && *it < sal_Int32(m_aStructure.size()) )
        {
            PDFStructureElement& rChild = m_aStructure[ *it ];
            if( rChild.m_eType != PDFWriter::NonStructElement )
            {
                if( rChild.m_nParentElement == rEle.m_nOwnElement )
                    emitStructure( rChild );
                else
                {
                    DBG_ERROR( "PDFWriterImpl::emitStructure: invalid child structure element" );
#if OSL_DEBUG_LEVEL > 1
                    fprintf( stderr, "PDFWriterImpl::emitStructure: invalid child structure elemnt with id %" SAL_PRIdINT32 "\n", *it );
#endif
                }
            }
        }
        else
        {
            DBG_ERROR( "PDFWriterImpl::emitStructure: invalid child structure id" );
#if OSL_DEBUG_LEVEL > 1
            fprintf( stderr, "PDFWriterImpl::emitStructure: invalid child structure id %" SAL_PRIdINT32 "\n", *it );
#endif
        }
    }

    OStringBuffer aLine( 512 );
    aLine.append( rEle.m_nObject );
    aLine.append( " 0 obj\n"
                  "<</Type" );
    sal_Int32 nParentTree = -1;
    if( rEle.m_nOwnElement == rEle.m_nParentElement )
    {
        nParentTree = createObject();
        CHECK_RETURN( nParentTree );
        aLine.append( "/StructTreeRoot\n" );
        aLine.append( "/ParentTree " );
        aLine.append( nParentTree );
        aLine.append( " 0 R\n" );
        if( ! m_aRoleMap.empty() )
        {
            aLine.append( "/RoleMap<<" );
            for( std::hash_map<OString,OString,OStringHash>::const_iterator
                 it = m_aRoleMap.begin(); it != m_aRoleMap.end(); ++it )
            {
                aLine.append( '/' );
                aLine.append(it->first);
                aLine.append( '/' );
                aLine.append( it->second );
                aLine.append( '\n' );
            }
            aLine.append( ">>\n" );
        }
    }
    else
    {
        aLine.append( "/StructElem\n"
                      "/S/" );
        if( rEle.m_aAlias.getLength() > 0 )
            aLine.append( rEle.m_aAlias );
        else
            aLine.append( getStructureTag( rEle.m_eType ) );
        aLine.append( "\n"
                      "/P " );
        aLine.append( m_aStructure[ rEle.m_nParentElement ].m_nObject );
        aLine.append( " 0 R\n"
                      "/Pg " );
        aLine.append( rEle.m_nFirstPageObject );
        aLine.append( " 0 R\n" );
        if( rEle.m_aActualText.getLength() )
        {
            aLine.append( "/ActualText" );
            appendUnicodeTextStringEncrypt( rEle.m_aActualText, rEle.m_nObject, aLine );
            aLine.append( "\n" );
        }
        if( rEle.m_aAltText.getLength() )
        {
            aLine.append( "/Alt" );
            appendUnicodeTextStringEncrypt( rEle.m_aAltText, rEle.m_nObject, aLine );
            aLine.append( "\n" );
        }
    }
    if( ! rEle.m_aBBox.IsEmpty() || rEle.m_aAttributes.size() )
    {
        OString aAttribs =  emitStructureAttributes( rEle );
        if( aAttribs.getLength() )
        {
            aLine.append( "/A" );
            aLine.append( aAttribs );
            aLine.append( "\n" );
        }
    }
    if( rEle.m_aLocale.Language.getLength() > 0 )
    {
        OUStringBuffer aLocBuf( 16 );
        aLocBuf.append( rEle.m_aLocale.Language.toAsciiLowerCase() );
        if( rEle.m_aLocale.Country.getLength() > 0 )
        {
            aLocBuf.append( sal_Unicode('-') );
            aLocBuf.append( rEle.m_aLocale.Country );
        }
        aLine.append( "/Lang" );
        appendLiteralStringEncrypt( aLocBuf.makeStringAndClear(), rEle.m_nObject, aLine );
        aLine.append( "\n" );
    }
    if( ! rEle.m_aKids.empty() )
    {
        unsigned int i = 0;
        aLine.append( "/K[" );
        for( std::list< PDFStructureElementKid >::const_iterator it =
                 rEle.m_aKids.begin(); it != rEle.m_aKids.end(); ++it, i++ )
        {
            if( it->nMCID == -1 )
            {
                aLine.append( it->nObject );
                aLine.append( " 0 R" );
                aLine.append( ( (i & 15) == 15 ) ? "\n" : " " );
            }
            else
            {
                if( it->nObject == rEle.m_nFirstPageObject )
                {
                    aLine.append( it->nMCID );
                    aLine.append( " " );
                }
                else
                {
                    aLine.append( "<</Type/MCR/Pg " );
                    aLine.append( it->nObject );
                    aLine.append( " 0 R /MCID " );
                    aLine.append( it->nMCID );
                    aLine.append( ">>\n" );
                }
            }
        }
        aLine.append( "]\n" );
    }
    aLine.append( ">>\nendobj\n\n" );

    CHECK_RETURN( updateObject( rEle.m_nObject ) );
    CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );

    CHECK_RETURN( emitStructParentTree( nParentTree ) );

    return rEle.m_nObject;
}

bool PDFWriterImpl::emitGradients()
{
    for( std::list<GradientEmit>::iterator it = m_aGradients.begin();
         it != m_aGradients.end(); ++it )
    {
        CHECK_RETURN( writeGradientFunction( *it ) );
    }
    return true;
}

bool PDFWriterImpl::emitTilings()
{
    OStringBuffer aTilingObj( 1024 );

    for( std::vector<TilingEmit>::iterator it = m_aTilings.begin(); it != m_aTilings.end(); ++it )
    {
        DBG_ASSERT( it->m_pTilingStream, "tiling without stream" );
        if( ! it->m_pTilingStream )
            continue;

        aTilingObj.setLength( 0 );

#if OSL_DEBUG_LEVEL > 1
        {
            OStringBuffer aLine( "PDFWriterImpl::emitTilings" );
            emitComment( aLine.getStr() );
        }
#endif

        sal_Int32 nX = (sal_Int32)it->m_aRectangle.Left();
        sal_Int32 nY = (sal_Int32)it->m_aRectangle.Top();
        sal_Int32 nW = (sal_Int32)it->m_aRectangle.GetWidth();
        sal_Int32 nH = (sal_Int32)it->m_aRectangle.GetHeight();
        if( it->m_aCellSize.Width() == 0 )
            it->m_aCellSize.Width() = nW;
        if( it->m_aCellSize.Height() == 0 )
            it->m_aCellSize.Height() = nH;

        bool bDeflate = compressStream( it->m_pTilingStream );
        it->m_pTilingStream->Seek( STREAM_SEEK_TO_END );
        sal_Size nTilingStreamSize = it->m_pTilingStream->Tell();
        it->m_pTilingStream->Seek( STREAM_SEEK_TO_BEGIN );

        // write pattern object
        aTilingObj.append( it->m_nObject );
        aTilingObj.append( " 0 obj\n" );
        aTilingObj.append( "<</Type/Pattern/PatternType 1\n"
                           "/PaintType 1\n"
                           "/TilingType 2\n"
                           "/BBox[" );
        appendFixedInt( nX, aTilingObj );
        aTilingObj.append( ' ' );
        appendFixedInt( nY, aTilingObj );
        aTilingObj.append( ' ' );
        appendFixedInt( nX+nW, aTilingObj );
        aTilingObj.append( ' ' );
        appendFixedInt( nY+nH, aTilingObj );
        aTilingObj.append( "]\n"
                           "/XStep " );
        appendFixedInt( it->m_aCellSize.Width(), aTilingObj );
        aTilingObj.append( "\n"
                           "/YStep " );
        appendFixedInt( it->m_aCellSize.Height(), aTilingObj );
        aTilingObj.append( "\n" );
        if( it->m_aTransform.matrix[0] != 1.0 ||
            it->m_aTransform.matrix[1] != 0.0 ||
            it->m_aTransform.matrix[3] != 0.0 ||
            it->m_aTransform.matrix[4] != 1.0 ||
            it->m_aTransform.matrix[2] != 0.0 ||
            it->m_aTransform.matrix[5] != 0.0 )
        {
            aTilingObj.append( "/Matrix [" );
            // TODO: scaling, mirroring on y, etc
            appendDouble( it->m_aTransform.matrix[0], aTilingObj );
            aTilingObj.append( ' ' );
            appendDouble( it->m_aTransform.matrix[1], aTilingObj );
            aTilingObj.append( ' ' );
            appendDouble( it->m_aTransform.matrix[3], aTilingObj );
            aTilingObj.append( ' ' );
            appendDouble( it->m_aTransform.matrix[4], aTilingObj );
            aTilingObj.append( ' ' );
            appendDouble( it->m_aTransform.matrix[2], aTilingObj );
            aTilingObj.append( ' ' );
            appendDouble( it->m_aTransform.matrix[5], aTilingObj );
            aTilingObj.append( "]\n" );
        }
        aTilingObj.append( "/Resources" );
        it->m_aResources.append( aTilingObj, getFontDictObject() );
        if( bDeflate )
            aTilingObj.append( "/Filter/FlateDecode" );
        aTilingObj.append( "/Length " );
        aTilingObj.append( (sal_Int32)nTilingStreamSize );
        aTilingObj.append( ">>\nstream\n" );
        CHECK_RETURN( updateObject( it->m_nObject ) );
        CHECK_RETURN( writeBuffer( aTilingObj.getStr(), aTilingObj.getLength() ) );
        checkAndEnableStreamEncryption( it->m_nObject );
        nTilingStreamSize = writeBuffer( it->m_pTilingStream->GetData(), nTilingStreamSize );
        delete it->m_pTilingStream;
        it->m_pTilingStream = NULL;
        if( nTilingStreamSize == 0 )
            return false;
        disableStreamEncryption();
        aTilingObj.setLength( 0 );
        aTilingObj.append( "\nendstream\nendobj\n\n" );
        CHECK_RETURN( writeBuffer( aTilingObj.getStr(), aTilingObj.getLength() ) );
    }
    return true;
}

sal_Int32 PDFWriterImpl::emitBuiltinFont( const ImplFontData* pFont, sal_Int32 nFontObject )
{
    const ImplPdfBuiltinFontData* pFD = GetPdfFontData( pFont );
    if( !pFD )
        return 0;
    const BuiltinFont* pBuiltinFont = pFD->GetBuiltinFont();

    OStringBuffer aLine( 1024 );

    if( nFontObject <= 0 )
        nFontObject = createObject();
    CHECK_RETURN( updateObject( nFontObject ) );
    aLine.append( nFontObject );
    aLine.append( " 0 obj\n"
                  "<</Type/Font/Subtype/Type1/BaseFont/" );
    appendName( pBuiltinFont->m_pPSName, aLine );
    aLine.append( "\n" );
    if( pBuiltinFont->m_eCharSet != RTL_TEXTENCODING_SYMBOL )
         aLine.append( "/Encoding/WinAnsiEncoding\n" );
    aLine.append( ">>\nendobj\n\n" );
    CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );
    return nFontObject;
}

std::map< sal_Int32, sal_Int32 > PDFWriterImpl::emitEmbeddedFont( const ImplFontData* pFont, EmbedFont& rEmbed )
{
    std::map< sal_Int32, sal_Int32 > aRet;
    if( isBuiltinFont( pFont ) )
    {
        aRet[ rEmbed.m_nNormalFontID ] = emitBuiltinFont( pFont );
        return aRet;
    }

    sal_Int32 nFontObject = 0;
    sal_Int32 nStreamObject = 0;
    sal_Int32 nFontDescriptor = 0;

    // prepare font encoding
    const Ucs2SIntMap* pEncoding = m_pReferenceDevice->mpGraphics->GetFontEncodingVector( pFont, NULL );
    sal_Int32 nToUnicodeStream = 0;
    sal_uInt8 nEncoding[256];
    sal_Ucs nEncodedCodes[256];
    if( pEncoding )
    {
        memset( nEncodedCodes, 0, sizeof(nEncodedCodes) );
        memset( nEncoding, 0, sizeof(nEncoding) );
        for( Ucs2SIntMap::const_iterator it = pEncoding->begin(); it != pEncoding->end(); ++it )
        {
            if( it->second != -1 )
            {
                sal_Int32 nCode = (sal_Int32)(it->second & 0x000000ff);
                nEncoding[ nCode ] = static_cast<sal_uInt8>( nCode );
                nEncodedCodes[ nCode ] = it->first;
            }
        }
    }

    FontSubsetInfo aInfo;
    sal_Int32 pWidths[256];
    const unsigned char* pFontData = NULL;
    long nFontLen = 0;
    sal_Int32 nLength1, nLength2;
    if( (pFontData = (const unsigned char*)m_pReferenceDevice->mpGraphics->GetEmbedFontData( pFont, nEncodedCodes, pWidths, aInfo, &nFontLen )) != NULL )
    {
        if( aInfo.m_nFontType != SAL_FONTSUBSETINFO_TYPE_TYPE1 )
            goto streamend;
        // see whether it is pfb or pfa; if it is a pfb, fill ranges
        // of 6 bytes that are not part of the font program
        std::list< int > aSections;
        std::list< int >::const_iterator it;
        int nIndex = 0;
        while( pFontData[nIndex] == 0x80 && nIndex < nFontLen-1 )
        {
            aSections.push_back( nIndex );
            if( pFontData[nIndex+1] == 0x03 )
                break;
            sal_Int32 nBytes =
                ((sal_Int32)pFontData[nIndex+2])			|
                ((sal_Int32)pFontData[nIndex+3]) << 8		|
                ((sal_Int32)pFontData[nIndex+4]) << 16		|
                ((sal_Int32)pFontData[nIndex+5]) << 24;
            nIndex += nBytes+6;
        }

        // search for eexec
        nIndex = 0;
        int nEndAsciiIndex;
        int nBeginBinaryIndex;
        int nEndBinaryIndex;
        do
        {
            while( nIndex < nFontLen-4 &&
                   ( pFontData[nIndex] != 'e'	||
                     pFontData[nIndex+1] != 'e' ||
                     pFontData[nIndex+2] != 'x' ||
                     pFontData[nIndex+3] != 'e' ||
                     pFontData[nIndex+4] != 'c'
                     )
                   )
                nIndex++;
            // check whether we are in a excluded section
            for( it = aSections.begin(); it != aSections.end() && (nIndex < *it || nIndex > ((*it) + 5) ); ++it )
                ;
        } while( it != aSections.end() && nIndex < nFontLen-4 );
        // this should end the ascii part
        if( nIndex > nFontLen-5 )
            goto streamend;

        nEndAsciiIndex = nIndex+4;
        // now count backwards until we can account for 512 '0'
        // which is the endmarker of the (hopefully) binary data
        // do not count the pfb header sections
        int nFound = 0;
        nIndex =  nFontLen-1;
        while( nIndex > 0 && nFound < 512 )
        {
            for( it = aSections.begin(); it != aSections.end() && (nIndex < *it || nIndex > ((*it) + 5) ); ++it )
                ;
            if( it == aSections.end() )
            {
                // inside the 512 '0' block there may only be whitespace
                // according to T1 spec; probably it would be to simple
                // if all fonts complied
                if( pFontData[nIndex] == '0' )
                    nFound++;
                else if( nFound > 0					&&
                         pFontData[nIndex] != '\r'		&&
                         pFontData[nIndex] != '\t'		&&
                         pFontData[nIndex] != '\n'		&&
                         pFontData[nIndex] != ' ' )
                    break;
            }
            nIndex--;
        }

        if( nIndex < 1 || nIndex <= nEndAsciiIndex )
            goto streamend;
        // there may be whitespace to ignore before the 512 '0'
        while( pFontData[nIndex] == '\r' || pFontData[nIndex] == '\n' )
        {
            nIndex--;
            for( it = aSections.begin(); it != aSections.end() && (nIndex < *it || nIndex > ((*it) + 5) ); ++it )
                ;
            if( it != aSections.end() )
            {
                nIndex = (*it)-1;
                break; // this is surely a binary boundary, in ascii case it wouldn't matter
            }
        }
        nEndBinaryIndex = nIndex;

        // and count forward again to the point where we have nFound '0'
        // to get the corect value for nLength3
        sal_Int32 nLength3 = 0;
        sal_Int32 nL3Index = nIndex;
        while( nFound && nL3Index < nFontLen )
        {
            for( it = aSections.begin(); it != aSections.end() && (nL3Index < *it || nL3Index > ((*it) + 5) ); ++it )
                ;
            if( it == aSections.end() )
            {
                // inside the 512 '0' block there may only be whitespace
                // according to T1 spec; probably it would be to simple
                // if all fonts complied
                if( pFontData[nL3Index] == '0' )
                    nFound--;
                nLength3++;
            }
            nL3Index++;
        }

        // search for beginning of binary section
        nBeginBinaryIndex = nEndAsciiIndex;
        do
        {
            nBeginBinaryIndex++;
            for( it = aSections.begin(); it != aSections.end() && (nBeginBinaryIndex < *it || nBeginBinaryIndex > ((*it) + 5) ); ++it )
                ;
        } while( nBeginBinaryIndex < nEndBinaryIndex &&
                 ( pFontData[nBeginBinaryIndex] == '\r'	||
                   pFontData[nBeginBinaryIndex] == '\n'	||
                   it != aSections.end() ) );

        // it seems to be vital to copy the exact whitespace between binary data
        // and eexec, else a invalid font results. so make nEndAsciiIndex
        // always immediate in front of nBeginBinaryIndex
        nEndAsciiIndex = nBeginBinaryIndex-1;
        for( it = aSections.begin(); it != aSections.end() && (nEndAsciiIndex < *it || nEndAsciiIndex > ((*it)+5)); ++it )
            ;
        if( it != aSections.end() )
            nEndAsciiIndex = (*it)-1;

        nLength1 = nEndAsciiIndex+1; // including the last character
        for( it = aSections.begin(); it != aSections.end() && *it < nEndAsciiIndex; ++it )
            nLength1 -= 6; // decrease by pfb section size

        // if the first four bytes are all ascii hex characters, then binary data
        // has to be converted to real binary data
        for( nIndex = 0; nIndex < 4 &&
                 ( ( pFontData[ nBeginBinaryIndex+nIndex ] >= '0' && pFontData[ nBeginBinaryIndex+nIndex ] <= '9' ) ||
                   ( pFontData[ nBeginBinaryIndex+nIndex ] >= 'a' && pFontData[ nBeginBinaryIndex+nIndex ] <= 'f' ) ||
                   ( pFontData[ nBeginBinaryIndex+nIndex ] >= 'A' && pFontData[ nBeginBinaryIndex+nIndex ] <= 'F' )
                   ); ++nIndex )
            ;
        bool bConvertHexData = true;
        if( nIndex < 4 )
        {
            bConvertHexData = false;
            nLength2 = nEndBinaryIndex - nBeginBinaryIndex + 1; // include the last byte
            for( it = aSections.begin(); it != aSections.end(); ++it )
                if( *it > nBeginBinaryIndex && *it < nEndBinaryIndex )
                    nLength2 -= 6;
        }
        else
        {
            // count the hex ascii characters to get nLength2
            nLength2 = 0;
            int nNextSectionIndex = 0;
            for( it = aSections.begin(); it != aSections.end() && *it < nBeginBinaryIndex; ++it )
                ;
            if( it != aSections.end() )
                nNextSectionIndex = *it;
            for( nIndex = nBeginBinaryIndex; nIndex <= nEndBinaryIndex; nIndex++ )
            {
                if( nIndex == nNextSectionIndex )
                {
                    nIndex += 6;
                    ++it;
                    nNextSectionIndex = (it == aSections.end() ? 0 : *it );
                }
                if( ( pFontData[ nIndex ] >= '0' && pFontData[ nIndex ] <= '9' ) ||
                    ( pFontData[ nIndex ] >= 'a' && pFontData[ nIndex ] <= 'f' ) ||
                    ( pFontData[ nIndex ] >= 'A' && pFontData[ nIndex ] <= 'F' ) )
                    nLength2++;
            }
            DBG_ASSERT( !(nLength2 & 1), "uneven number of hex chars in binary pfa section" );
            nLength2 /= 2;
        }

        // now we can actually write the font stream !
#if OSL_DEBUG_LEVEL > 1
        {
            OStringBuffer aLine( " PDFWriterImpl::emitEmbeddedFont" );
            emitComment( aLine.getStr() );
        }
#endif
        OStringBuffer aLine( 512 );
        nStreamObject = createObject();
        if( !updateObject(nStreamObject))
            goto streamend;
        sal_Int32 nStreamLengthObject = createObject();
        aLine.append( nStreamObject );
        aLine.append( " 0 obj\n"
                      "<</Length " );
        aLine.append( nStreamLengthObject );
        aLine.append( " 0 R"
#ifndef DEBUG_DISABLE_PDFCOMPRESSION
                      "/Filter/FlateDecode"
#endif
                      "/Length1 " );
        aLine.append( nLength1 );
        aLine.append( " /Length2 " );
        aLine.append( nLength2 );
        aLine.append( " /Length3 ");
        aLine.append( nLength3 );
        aLine.append( ">>\n"
                      "stream\n" );
        if( !writeBuffer( aLine.getStr(), aLine.getLength() ) )
            goto streamend;

        sal_uInt64 nBeginStreamPos = 0;
        osl_getFilePos( m_aFile, &nBeginStreamPos );

        beginCompression();
        checkAndEnableStreamEncryption( nStreamObject );

        // write ascii section
        if( aSections.begin() == aSections.end() )
        {
            if( ! writeBuffer( pFontData, nEndAsciiIndex+1 ) )
			{
                endCompression();
				disableStreamEncryption();
                goto streamend;
			}
        }
        else
        {
            // first section always starts at 0
            it = aSections.begin();
            nIndex = (*it)+6;
            ++it;
            while( *it < nEndAsciiIndex )
            {
                if( ! writeBuffer( pFontData+nIndex, (*it)-nIndex ) )
				{
                endCompression();
					disableStreamEncryption();
                    goto streamend;
				}
                nIndex = (*it)+6;
                ++it;
            }
            // write partial last section
            if( ! writeBuffer( pFontData+nIndex, nEndAsciiIndex-nIndex+1 ) )
			{
                endCompression();
				disableStreamEncryption();
                goto streamend;
			}
        }

        // write binary section
        if( ! bConvertHexData )
        {
            if( aSections.begin() == aSections.end() )
            {
                if( ! writeBuffer( pFontData+nBeginBinaryIndex, nFontLen-nBeginBinaryIndex ) )
				{
                    endCompression();
					disableStreamEncryption();
                    goto streamend;
				}
            }
            else
            {
                for( it = aSections.begin(); *it < nBeginBinaryIndex; ++it )
                    ;
                // write first partial section
                if( ! writeBuffer( pFontData+nBeginBinaryIndex, (*it) - nBeginBinaryIndex ) )
                {
                    endCompression();
                    disableStreamEncryption();
                    goto streamend;
                }
                // write following sections
                while( it != aSections.end() )
                {
                    nIndex = (*it)+6;
                    ++it;
                    if( nIndex < nFontLen ) // last section marker is usually the EOF which has only 2 bytes
                    {
                        sal_Int32 nSectionLen = (it == aSections.end()) ? nFontLen - nIndex : (*it) - nIndex;
                        if( ! writeBuffer( pFontData+nIndex, nSectionLen ) )
                        {
                            endCompression();
                            disableStreamEncryption();
                            goto streamend;
                        }
                    }
                }
            }
        }
        else
        {
            unsigned char* pWriteBuffer = (unsigned char*)rtl_allocateMemory( nLength2 );
            memset( pWriteBuffer, 0, nLength2 );
            int nWriteIndex = 0;

            int nNextSectionIndex = 0;
            for( it = aSections.begin(); it != aSections.end() && *it < nBeginBinaryIndex; ++it )
                ;
            if( it != aSections.end() )
                nNextSectionIndex = *it;
            for( nIndex = nBeginBinaryIndex; nIndex <= nEndBinaryIndex; nIndex++ )
            {
                if( nIndex == nNextSectionIndex )
                {
                    nIndex += 6;
                    ++it;
                    nNextSectionIndex = (it == aSections.end() ? nFontLen : *it );
                }
                unsigned char cNibble = 0x80;
                if( pFontData[ nIndex ] >= '0' && pFontData[ nIndex ] <= '9' )
                    cNibble = pFontData[nIndex] - '0';
                else if( pFontData[ nIndex ] >= 'a' && pFontData[ nIndex ] <= 'f' )
                    cNibble = pFontData[nIndex] - 'a' + 10;
                else if( pFontData[ nIndex ] >= 'A' && pFontData[ nIndex ] <= 'F' )
                    cNibble = pFontData[nIndex] - 'A' + 10;
                if( cNibble != 0x80 )
                {
                    if( !(nWriteIndex & 1 ) )
                        cNibble <<= 4;
                    pWriteBuffer[ nWriteIndex/2 ] |= cNibble;
                    nWriteIndex++;
                }
            }
            if( ! writeBuffer( pWriteBuffer, nLength2 ) )
			{
                endCompression();
				disableStreamEncryption();
                goto streamend;
			}
            rtl_freeMemory( pWriteBuffer );

            if( aSections.empty() )
            {
                if( ! writeBuffer( pFontData+nIndex, nFontLen-nIndex ) )
                {
                    endCompression();
                    disableStreamEncryption();
                    goto streamend;
                }
            }
            else
            {
                // write rest of this section
                if( nIndex < nNextSectionIndex )
                {
                    if( ! writeBuffer( pFontData+nIndex, nNextSectionIndex - nIndex ) )
                    {
                        endCompression();
                        disableStreamEncryption();
                        goto streamend;
                    }
                }
                // write following sections
                while( it != aSections.end() )
                {
                    nIndex = (*it)+6;
                    ++it;
                    if( nIndex < nFontLen ) // last section marker is usually the EOF which has only 2 bytes
                    {
                        sal_Int32 nSectionLen = (it == aSections.end()) ? nFontLen - nIndex : (*it) - nIndex;
                        if( ! writeBuffer( pFontData+nIndex, nSectionLen ) )
                        {
                            endCompression();
                            disableStreamEncryption();
                            goto streamend;
                        }
                    }
                }
            }
        }
        endCompression();
        disableStreamEncryption();


        sal_uInt64 nEndStreamPos = 0;
        osl_getFilePos( m_aFile, &nEndStreamPos );

        // and finally close the stream
        aLine.setLength( 0 );
        aLine.append( "\nendstream\nendobj\n\n" );
        if( ! writeBuffer( aLine.getStr(), aLine.getLength() ) )
            goto streamend;

        // write stream length object
        aLine.setLength( 0 );
        if( ! updateObject( nStreamLengthObject ) )
            goto streamend;
        aLine.append( nStreamLengthObject );
        aLine.append( " 0 obj\n" );
        aLine.append( (sal_Int64)(nEndStreamPos-nBeginStreamPos ) );
        aLine.append( "\nendobj\n\n" );
        if( ! writeBuffer( aLine.getStr(), aLine.getLength() ) )
            goto streamend;
    }
    else
    {
        rtl::OStringBuffer aErrorComment( 256 );
        aErrorComment.append( "GetEmbedFontData failed for font \"" );
        aErrorComment.append( OUStringToOString( pFont->GetFamilyName(), RTL_TEXTENCODING_UTF8 ) );
        aErrorComment.append( '\"' );
        if( pFont->GetSlant() == ITALIC_NORMAL )
            aErrorComment.append( " italic" );
        else if( pFont->GetSlant() == ITALIC_OBLIQUE )
            aErrorComment.append( " oblique" );
        aErrorComment.append( " weight=" );
        aErrorComment.append( sal_Int32(pFont->GetWeight()) );
        emitComment( aErrorComment.getStr() );
    }

    if( nStreamObject )
        // write font descriptor
        nFontDescriptor = emitFontDescriptor( pFont, aInfo, 0, nStreamObject );

    if( nFontDescriptor )
    {
        if( pEncoding )
            nToUnicodeStream = createToUnicodeCMap( nEncoding, nEncodedCodes, sizeof(nEncoding)/sizeof(nEncoding[0]) );

        // write font object
        sal_Int32 nObject = createObject();
        if( ! updateObject( nObject ) )
            goto streamend;

        OStringBuffer aLine( 1024 );
        aLine.append( nObject );
        aLine.append( " 0 obj\n"
                      "<</Type/Font/Subtype/Type1/BaseFont/" );
        appendName( aInfo.m_aPSName, aLine );
        aLine.append( "\n" );
        if( !pFont->mbSymbolFlag &&  pEncoding == 0 )
            aLine.append( "/Encoding/WinAnsiEncoding\n" );
        if( nToUnicodeStream )
        {
            aLine.append( "/ToUnicode " );
            aLine.append( nToUnicodeStream );
            aLine.append( " 0 R\n" );
        }
        aLine.append( "/FirstChar 0 /LastChar 255\n"
                      "/Widths[" );
        for( int i = 0; i < 256; i++ )
        {
            aLine.append( pWidths[i] );
            aLine.append( ((i&15) == 15) ? "\n" : " " );
        }
        aLine.append( "]\n"
                      "/FontDescriptor " );
        aLine.append( nFontDescriptor );
        aLine.append( " 0 R>>\n"
                      "endobj\n\n" );
        if( ! writeBuffer( aLine.getStr(), aLine.getLength() ) )
            goto streamend;

        nFontObject = nObject;

        aRet[ rEmbed.m_nNormalFontID ] = nObject;

        // write additional encodings
        for( std::list< EmbedEncoding >::iterator enc_it = rEmbed.m_aExtendedEncodings.begin(); enc_it != rEmbed.m_aExtendedEncodings.end(); ++enc_it )
        {
            sal_Int32 aEncWidths[ 256 ];
            // emit encoding dict
            sal_Int32 nEncObject = createObject();
            if( ! updateObject( nEncObject ) )
                goto streamend;

            OutputDevice* pRef = getReferenceDevice();
            pRef->Push( PUSH_FONT | PUSH_MAPMODE );
            pRef->SetMapMode( MapMode( MAP_PIXEL ) );
            Font aFont( pFont->GetFamilyName(), pFont->GetStyleName(), Size( 0, 1000 ) );
            aFont.SetWeight( pFont->GetWeight() );
            aFont.SetItalic( pFont->GetSlant() );
            aFont.SetPitch( pFont->GetPitch() );
            pRef->SetFont( aFont );
            pRef->ImplNewFont();

            aLine.setLength( 0 );
            aLine.append( nEncObject );
            aLine.append( " 0 obj\n"
                          "<</Type/Encoding/Differences[ 0\n" );
            int nEncoded = 0;
            for( std::vector< EmbedCode >::iterator str_it = enc_it->m_aEncVector.begin(); str_it != enc_it->m_aEncVector.end(); ++str_it )
            {
                String aStr( str_it->m_aUnicode );
                aEncWidths[nEncoded] = pRef->GetTextWidth( aStr );
                nEncodedCodes[nEncoded] = str_it->m_aUnicode;
                nEncoding[nEncoded] = sal::static_int_cast<sal_uInt8>(nEncoded);

                aLine.append( " /" );
                aLine.append( str_it->m_aName );
                if( !((++nEncoded) & 15) )
                    aLine.append( "\n" );
            }
            aLine.append( "]>>\n"
                          "endobj\n\n" );

            pRef->Pop();

            if( ! writeBuffer( aLine.getStr(), aLine.getLength() ) )
                goto streamend;

            nToUnicodeStream = createToUnicodeCMap( nEncoding, nEncodedCodes, nEncoded );

            nObject = createObject();
            if( ! updateObject( nObject ) )
                goto streamend;

            aLine.setLength( 0 );
            aLine.append( nObject );
            aLine.append( " 0 obj\n"
                          "<</Type/Font/Subtype/Type1/BaseFont/" );
            appendName( aInfo.m_aPSName, aLine );
            aLine.append( "\n" );
            aLine.append( "/Encoding " );
            aLine.append( nEncObject );
            aLine.append( " 0 R\n" );
            if( nToUnicodeStream )
            {
                aLine.append( "/ToUnicode " );
                aLine.append( nToUnicodeStream );
                aLine.append( " 0 R\n" );
            }
            aLine.append( "/FirstChar 0\n"
                          "/LastChar " );
            aLine.append( (sal_Int32)(nEncoded-1) );
            aLine.append( "\n"
                          "/Widths[" );
            for( int i = 0; i < nEncoded; i++ )
            {
                aLine.append( aEncWidths[i] );
                aLine.append( ((i&15) == 15) ? "\n" : " " );
            }
            aLine.append( " ]\n"
                          "/FontDescriptor " );
            aLine.append( nFontDescriptor );
            aLine.append( " 0 R>>\n"
                          "endobj\n\n" );
            if( ! writeBuffer( aLine.getStr(), aLine.getLength() ) )
                goto streamend;

            aRet[ enc_it->m_nFontID ] = nObject;
        }
    }

  streamend:
    if( pFontData )
        m_pReferenceDevice->mpGraphics->FreeEmbedFontData( pFontData, nFontLen );

    return aRet;
}

static void appendSubsetName( int nSubsetID, const OUString& rPSName, OStringBuffer& rBuffer )
{
    if( nSubsetID )
    {
        for( int i = 0; i < 6; i++ )
        {
            int nOffset = (nSubsetID % 26);
            nSubsetID /= 26;
            rBuffer.append( (sal_Char)('A'+nOffset) );
        }
        rBuffer.append( '+' );
    }
    appendName( rPSName, rBuffer );
}

sal_Int32 PDFWriterImpl::createToUnicodeCMap( sal_uInt8* pEncoding, sal_Ucs* pUnicodes, int nGlyphs )
{
    int nMapped = 0, n = 0;
    for( n = 0; n < nGlyphs; n++ )
        if( pUnicodes[n] )
            nMapped++;

    if( nMapped == 0 )
        return 0;

    sal_Int32 nStream = createObject();
    CHECK_RETURN( updateObject( nStream ) );

    OStringBuffer aContents( 1024 );
    aContents.append(
                     "/CIDInit/ProcSet findresource begin\n"
                     "12 dict begin\n"
                     "begincmap\n"
                     "/CIDSystemInfo<<\n"
                     "/Registry (Adobe)\n"
                     "/Ordering (UCS)\n"
                     "/Supplement 0\n"
                     ">> def\n"
                     "/CMapName/Adobe-Identity-UCS def\n"
                     "/CMapType 2 def\n"
                     "1 begincodespacerange\n"
                     "<00> <FF>\n"
                     "endcodespacerange\n"
                     );
    int nCount = 0;
    for( n = 0; n < nGlyphs; n++ )
    {
        if( pUnicodes[n] )
        {
            if( (nCount % 100) == 0 )
            {
                if( nCount )
                    aContents.append( "endbfchar\n" );
                aContents.append( (sal_Int32)((nMapped-nCount > 100) ? 100 : nMapped-nCount ) );
                aContents.append( " beginbfchar\n" );
            }
            aContents.append( '<' );
            appendHex( (sal_Int8)pEncoding[n], aContents );
            aContents.append( "> <" );
	    // TODO: handle unicodes>U+FFFF
            appendHex( (sal_Int8)(pUnicodes[n] / 256), aContents );
            appendHex( (sal_Int8)(pUnicodes[n] & 255), aContents );
            aContents.append( ">\n" );
            nCount++;
        }
    }
    aContents.append( "endbfchar\n"
                      "endcmap\n"
                      "CMapName currentdict /CMap defineresource pop\n"
                      "end\n"
                      "end\n" );
#ifndef DEBUG_DISABLE_PDFCOMPRESSION
    ZCodec* pCodec = new ZCodec( 0x4000, 0x4000 );
    SvMemoryStream aStream;
    pCodec->BeginCompression();
    pCodec->Write( aStream, (const BYTE*)aContents.getStr(), aContents.getLength() );
    pCodec->EndCompression();
    delete pCodec;
#endif

#if OSL_DEBUG_LEVEL > 1
    {
        OStringBuffer aLine( " PDFWriterImpl::createToUnicodeCMap" );
        emitComment( aLine.getStr() );
    }
#endif
    OStringBuffer aLine( 40 );

    aLine.append( nStream );
    aLine.append( " 0 obj\n<</Length " );
#ifndef DEBUG_DISABLE_PDFCOMPRESSION
    sal_Int32 nLen = (sal_Int32)aStream.Tell();
    aStream.Seek( 0 );
    aLine.append( nLen );
    aLine.append( "/Filter/FlateDecode" );
#else
    aLine.append( aContents.getLength() );
#endif
    aLine.append( ">>\nstream\n" );
    CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );
    checkAndEnableStreamEncryption( nStream );
#ifndef DEBUG_DISABLE_PDFCOMPRESSION
    CHECK_RETURN( writeBuffer( aStream.GetData(), nLen ) );
#else
    CHECK_RETURN( writeBuffer( aContents.getStr(), aContents.getLength() ) );
#endif
    disableStreamEncryption();
    aLine.setLength( 0 );
    aLine.append( "\nendstream\n"
                  "endobj\n\n" );
    CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );
    return nStream;
}

sal_Int32 PDFWriterImpl::emitFontDescriptor( const ImplFontData* pFont, FontSubsetInfo& rInfo, sal_Int32 nSubsetID, sal_Int32 nFontStream )
{
    OStringBuffer aLine( 1024 );
    // get font flags, see PDF reference 1.4 p. 358
    // possibly characters outside Adobe standard encoding
    // so set Symbolic flag
    sal_Int32 nFontFlags = (1<<2);
    if( pFont->GetSlant() == ITALIC_NORMAL || pFont->GetSlant() == ITALIC_OBLIQUE )
        nFontFlags |= (1 << 6);
    if( pFont->GetPitch() == PITCH_FIXED )
        nFontFlags |= 1;
    if( pFont->GetFamilyType() == FAMILY_SCRIPT )
        nFontFlags |= (1 << 3);
    else if( pFont->GetFamilyType() == FAMILY_ROMAN )
        nFontFlags |= (1 << 1);

    sal_Int32 nFontDescriptor = createObject();
    CHECK_RETURN( updateObject( nFontDescriptor ) );
    aLine.setLength( 0 );
    aLine.append( nFontDescriptor );
    aLine.append( " 0 obj\n"
                  "<</Type/FontDescriptor/FontName/" );
    appendSubsetName( nSubsetID, rInfo.m_aPSName, aLine );
    aLine.append( "\n"
                  "/Flags " );
    aLine.append( nFontFlags );
    aLine.append( "\n"
                  "/FontBBox[" );
    // note: Top and Bottom are reversed in VCL and PDF rectangles
    aLine.append( (sal_Int32)rInfo.m_aFontBBox.TopLeft().X() );
    aLine.append( ' ' );
    aLine.append( (sal_Int32)rInfo.m_aFontBBox.TopLeft().Y() );
    aLine.append( ' ' );
    aLine.append( (sal_Int32)rInfo.m_aFontBBox.BottomRight().X() );
    aLine.append( ' ' );
    aLine.append( (sal_Int32)(rInfo.m_aFontBBox.BottomRight().Y()+1) );
    aLine.append( "]/ItalicAngle " );
    if( pFont->GetSlant() == ITALIC_OBLIQUE || pFont->GetSlant() == ITALIC_NORMAL )
        aLine.append( "-30" );
    else
        aLine.append( "0" );
    aLine.append( "\n"
                  "/Ascent " );
    aLine.append( (sal_Int32)rInfo.m_nAscent );
    aLine.append( "\n"
                  "/Descent " );
    aLine.append( (sal_Int32)-rInfo.m_nDescent );
    aLine.append( "\n"
                  "/CapHeight " );
    aLine.append( (sal_Int32)rInfo.m_nCapHeight );
    // According to PDF reference 1.4 StemV is required
    // seems a tad strange to me, but well ...
    aLine.append( "\n"
                  "/StemV 80\n"
                  "/FontFile" );
    switch( rInfo.m_nFontType )
    {
        case SAL_FONTSUBSETINFO_TYPE_TRUETYPE:
            aLine.append( '2' );
            break;
        case SAL_FONTSUBSETINFO_TYPE_TYPE1:
            break;
        default:
            DBG_ERROR( "unknown fonttype in PDF font descriptor" );
            return 0;
    }
    aLine.append( ' ' );
    aLine.append( nFontStream );
    aLine.append( " 0 R>>\n"
                  "endobj\n\n" );
    CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );

    return nFontDescriptor;
}

void PDFWriterImpl::appendBuiltinFontsToDict( OStringBuffer& rDict ) const
{
    for( std::map< sal_Int32, sal_Int32 >::const_iterator it =
         m_aBuiltinFontToObjectMap.begin(); it != m_aBuiltinFontToObjectMap.end(); ++it )
    {
        rDict.append( m_aBuiltinFonts[it->first].getNameObject() );
        rDict.append( ' ' );
        rDict.append( it->second );
        rDict.append( " 0 R" );
    }
}

bool PDFWriterImpl::emitFonts()
{
    if( ! m_pReferenceDevice->ImplGetGraphics() )
        return false;

    OStringBuffer aLine( 1024 );
#if !defined USE_JAVA || !defined MACOSX
    char buf[8192];
#endif	// !USE_JAVA || !MACOSX

    std::map< sal_Int32, sal_Int32 > aFontIDToObject;

#if defined USE_JAVA && defined MACOSX
    // Encode the glyphs using the native font encoding before outputting the
    // subsets to the PDF file
    encodeGlyphs();

    std::map< OString, sal_Int32 > aFontStrToObject;
    for ( FontSubsetData::iterator it = m_aSubsets.begin(); it != m_aSubsets.end(); ++it )
    {
        for ( FontEmitList::iterator lit = it->second.m_aSubsets.begin(); lit != it->second.m_aSubsets.end(); ++lit )
        {
            FontEmit& rEmit = *lit;

            int nNumObjs = m_aObjects.size();
            sal_uInt64 nStartPos;
            CHECK_RETURN( ( osl_File_E_None == osl_getFilePos( m_aFile, &nStartPos ) ) );

            oslFileHandle aFontFile;
            CHECK_RETURN( ( osl_File_E_None == osl_openFile( rEmit.m_aFontFileName.pData, &aFontFile, osl_File_OpenFlag_Read ) ) );

            std::map< sal_Int32, sal_Int32 > aIDMapping;
            for ( std::map< OString, sal_Int32 >::iterator pit = rEmit.m_aFontSubIDMapping.begin(); pit != rEmit.m_aFontSubIDMapping.end(); ++pit )
            {
                PDFEmitObject& rObj = rEmit.m_aObjectMapping[ pit->second ];
                sal_Int32 nFontObjID = writePDFObjectTree( rObj, aFontFile, rEmit.m_aObjectMapping, lit->m_nFontID, aIDMapping );

                if ( !nFontObjID )
                {
                    // If writePDFObjectTree() fails, truncate any data and
                    // remove any objects that were added
                    CHECK_RETURN( ( osl_File_E_None == osl_setFilePos( m_aFile, osl_Pos_Absolut, nStartPos ) ) );
                    CHECK_RETURN( ( osl_File_E_None == osl_setFileSize( m_aFile, nStartPos ) ) );

                    std::vector< sal_uInt64 > aObjects;
                    for ( int i = 0; i < nNumObjs; i++ )
                        aObjects.push_back( m_aObjects[i] );
                    m_aObjects = aObjects;

                    aIDMapping.clear();

                    continue;
                }

                OString aFontStr = OStringBuffer().append( rEmit.m_nFontID ).append( '.' ).append( pit->second ).makeStringAndClear();
                aFontStrToObject[ aFontStr ] = nFontObjID;
            }

            osl_closeFile( aFontFile );
        }
    }
#else	// USE_JAVA && MACOSX
    OUString aTmpName;
    osl_createTempFile( NULL, NULL, &aTmpName.pData );
    for( FontSubsetData::iterator it = m_aSubsets.begin(); it != m_aSubsets.end(); ++it )
    {
        for( FontEmitList::iterator lit = it->second.m_aSubsets.begin(); lit != it->second.m_aSubsets.end(); ++lit )
        {
            sal_Int32 pGlyphIDs[ 256 ];
            sal_Int32 pWidths[ 256 ];
            sal_uInt8 pEncoding[ 256 ];
            sal_Ucs   pUnicodes[ 256 ];
            int nGlyphs = 1;
            // fill arrays and prepare encoding index map
            sal_Int32 nToUnicodeStream = 0;

            memset( pGlyphIDs, 0, sizeof( pGlyphIDs ) );
            memset( pEncoding, 0, sizeof( pEncoding ) );
            memset( pUnicodes, 0, sizeof( pUnicodes ) );
            for( FontEmitMapping::iterator fit = lit->m_aMapping.begin(); fit != lit->m_aMapping.end();++fit )
            {
                sal_uInt8 nEnc = fit->second.m_nSubsetGlyphID;

                DBG_ASSERT( pGlyphIDs[nEnc] == 0 && pEncoding[nEnc] == 0, "duplicate glyph" );
                DBG_ASSERT( nEnc <= lit->m_aMapping.size(), "invalid glyph encoding" );

                pGlyphIDs[ nEnc ] = fit->first;
                pEncoding[ nEnc ] = nEnc;
                pUnicodes[ nEnc ] = fit->second.m_aUnicode;
                if( pUnicodes[ nEnc ] )
                    nToUnicodeStream = 1;
                if( nGlyphs < 256 )
                    nGlyphs++;
                else
                {
                    DBG_ERROR( "too many glyphs for subset" );
                }
            }
            FontSubsetInfo aSubsetInfo;
            if( m_pReferenceDevice->mpGraphics->CreateFontSubset( aTmpName, it->first, pGlyphIDs, pEncoding, pWidths, nGlyphs, aSubsetInfo ) )
            {
                DBG_ASSERT( aSubsetInfo.m_nFontType == SAL_FONTSUBSETINFO_TYPE_TRUETYPE, "wrong font type in font subset" );
                // create font stream
                oslFileHandle aFontFile;
                CHECK_RETURN( (osl_File_E_None == osl_openFile( aTmpName.pData, &aFontFile, osl_File_OpenFlag_Read ) ) );
                // get file size
                sal_uInt64 nLength;
                CHECK_RETURN( (osl_File_E_None == osl_setFilePos( aFontFile, osl_Pos_End, 0 ) ) );
                CHECK_RETURN( (osl_File_E_None == osl_getFilePos( aFontFile, &nLength ) ) );
                CHECK_RETURN( (osl_File_E_None == osl_setFilePos( aFontFile, osl_Pos_Absolut, 0 ) ) );

#if OSL_DEBUG_LEVEL > 1
                {
                    OStringBuffer aLine1( " PDFWriterImpl::emitFonts" );
                    emitComment( aLine1.getStr() );
                }
#endif
                sal_Int32 nFontStream = createObject();
                sal_Int32 nStreamLengthObject = createObject();
                CHECK_RETURN( updateObject( nFontStream ) );
                aLine.setLength( 0 );
                aLine.append( nFontStream );
                aLine.append( " 0 obj\n"
                              "<</Length " );
                aLine.append( (sal_Int32)nStreamLengthObject );
                aLine.append( " 0 R"
#ifndef DEBUG_DISABLE_PDFCOMPRESSION
                              "/Filter/FlateDecode"
#endif
                              "/Length1 " );
                aLine.append( (sal_Int32)nLength );
                aLine.append( ">>\n"
                              "stream\n" );
                CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );

                sal_uInt64 nStartPos = 0;
                CHECK_RETURN( (osl_File_E_None == osl_getFilePos( m_aFile, &nStartPos ) ) );

                // copy font file
                beginCompression();
                checkAndEnableStreamEncryption( nFontStream );
                sal_uInt64 nRead;
                sal_Bool bEOF = sal_False;
                do
                {
                    CHECK_RETURN( (osl_File_E_None == osl_readFile( aFontFile, buf, sizeof( buf ), &nRead ) ) );
                    CHECK_RETURN( writeBuffer( buf, nRead ) );
                    CHECK_RETURN( (osl_File_E_None == osl_isEndOfFile( aFontFile, &bEOF ) ) );
                } while( ! bEOF );
                endCompression();
                disableStreamEncryption();
                // close the file
                osl_closeFile( aFontFile );

                sal_uInt64 nEndPos = 0;
                CHECK_RETURN( (osl_File_E_None == osl_getFilePos( m_aFile, &nEndPos ) ) );
                // end the stream
                aLine.setLength( 0 );
                aLine.append( "\nendstream\nendobj\n\n" );
                CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );

                // emit stream length object
                CHECK_RETURN( updateObject( nStreamLengthObject ) );
                aLine.setLength( 0 );
                aLine.append( nStreamLengthObject );
                aLine.append( " 0 obj\n" );
                aLine.append( (sal_Int64)(nEndPos-nStartPos) );
                aLine.append( "\nendobj\n\n" );
                CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );

                // write font descriptor
                sal_Int32 nFontDescriptor = emitFontDescriptor( it->first, aSubsetInfo, lit->m_nFontID, nFontStream );

                if( nToUnicodeStream )
                    nToUnicodeStream = createToUnicodeCMap( pEncoding, pUnicodes, nGlyphs );

                sal_Int32 nFontObject = createObject();
                CHECK_RETURN( updateObject( nFontObject ) );
                aLine.setLength( 0 );
                aLine.append( nFontObject );
                aLine.append( " 0 obj\n"
                              "<</Type/Font/Subtype/TrueType/BaseFont/" );
                appendSubsetName( lit->m_nFontID, aSubsetInfo.m_aPSName, aLine );
                aLine.append( "\n"
                              "/FirstChar 0\n"
                              "/LastChar " );
                aLine.append( (sal_Int32)(nGlyphs-1) );
                aLine.append( "\n"
                              "/Widths[" );
                for( int i = 0; i < nGlyphs; i++ )
                {
                    aLine.append( pWidths[ i ] );
                    aLine.append( ((i & 15) == 15) ? "\n" : " " );
                }
                aLine.append( "]\n"
                              "/FontDescriptor " );
                aLine.append( nFontDescriptor );
                aLine.append( " 0 R\n" );
                if( nToUnicodeStream )
                {
                    aLine.append( "/ToUnicode " );
                    aLine.append( nToUnicodeStream );
                    aLine.append( " 0 R\n" );
                }
                aLine.append( ">>\n"
                              "endobj\n\n" );
                CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );

                aFontIDToObject[ lit->m_nFontID ] = nFontObject;
            }
            else
            {
                const ImplFontData* pFont = it->first;
                rtl::OStringBuffer aErrorComment( 256 );
                aErrorComment.append( "CreateFontSubset failed for font \"" );
                aErrorComment.append( OUStringToOString( pFont->GetFamilyName(), RTL_TEXTENCODING_UTF8 ) );
                aErrorComment.append( '\"' );
                if( pFont->GetSlant() == ITALIC_NORMAL )
                    aErrorComment.append( " italic" );
                else if( pFont->GetSlant() == ITALIC_OBLIQUE )
                    aErrorComment.append( " oblique" );
                aErrorComment.append( " weight=" );
                aErrorComment.append( sal_Int32(pFont->GetWeight()) );
                emitComment( aErrorComment.getStr() );
            }
        }
    }
    osl_removeFile( aTmpName.pData );
#endif	// USE_JAVA && MACOSX

    // emit embedded fonts
    for( FontEmbedData::iterator eit = m_aEmbeddedFonts.begin(); eit != m_aEmbeddedFonts.end(); ++eit )
    {
        std::map< sal_Int32, sal_Int32 > aObjects = emitEmbeddedFont( eit->first, eit->second );
        for( std::map< sal_Int32, sal_Int32 >::iterator fit = aObjects.begin(); fit != aObjects.end(); ++fit )
        {
            CHECK_RETURN( fit->second );
            aFontIDToObject[ fit->first ] = fit->second;
        }
    }

    OStringBuffer aFontDict( 1024 );
    aFontDict.append( getFontDictObject() );
    aFontDict.append( " 0 obj\n"
                      "<<" );
    int ni = 0;
#if defined USE_JAVA && defined MACOSX
    for( std::map< OString, sal_Int32 >::iterator mit = aFontStrToObject.begin(); mit != aFontStrToObject.end(); ++mit )
#else	// USE_JAVA && MACOSX
    for( std::map< sal_Int32, sal_Int32 >::iterator mit = aFontIDToObject.begin(); mit != aFontIDToObject.end(); ++mit )
#endif	// USE_JAVA && MACOSX
    {
        aFontDict.append( "/F" );
        aFontDict.append( mit->first );
        aFontDict.append( ' ' );
        aFontDict.append( mit->second );
        aFontDict.append( " 0 R" );
            if( ((++ni) & 7) == 0 )
                aFontDict.append( '\n' );
    }
    // emit builtin font for widget apperances / variable text
    for( std::map< sal_Int32, sal_Int32 >::iterator it = m_aBuiltinFontToObjectMap.begin();
         it != m_aBuiltinFontToObjectMap.end(); ++it )
    {
        ImplPdfBuiltinFontData aData(m_aBuiltinFonts[it->first]);
        it->second = emitBuiltinFont( &aData, it->second );
    }
    appendBuiltinFontsToDict( aFontDict );
    aFontDict.append( "\n>>\nendobj\n\n" );

    CHECK_RETURN( updateObject( getFontDictObject() ) );
    CHECK_RETURN( writeBuffer( aFontDict.getStr(), aFontDict.getLength() ) );
    return true;
}

sal_Int32 PDFWriterImpl::emitResources()
{
    // emit shadings
    if( ! m_aGradients.empty() )
        CHECK_RETURN( emitGradients() );
    // emit tilings
    if( ! m_aTilings.empty() )
        CHECK_RETURN( emitTilings() );

    // emit font dict
    CHECK_RETURN( emitFonts() );

    // emit Resource dict
    OStringBuffer aLine( 512 );
    sal_Int32 nResourceDict = getResourceDictObj();
    CHECK_RETURN( updateObject( nResourceDict ) );
    aLine.setLength( 0 );
    aLine.append( nResourceDict );
    aLine.append( " 0 obj\n" );
    m_aGlobalResourceDict.append( aLine, getFontDictObject() );
    aLine.append( "endobj\n\n" );
    CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );
    return nResourceDict;
}

sal_Int32 PDFWriterImpl::updateOutlineItemCount( std::vector< sal_Int32 >& rCounts,
                                                 sal_Int32 nItemLevel,
                                                 sal_Int32 nCurrentItemId )
{
    /* The /Count number of an item is
       positive: the number of visible subitems
       negative: the negative number of subitems that will become visible if
                 the item gets opened
       see PDF ref 1.4 p 478
    */

    sal_Int32 nCount = 0;

    if( m_aContext.OpenBookmarkLevels < 0           || // all levels arevisible
        m_aContext.OpenBookmarkLevels >= nItemLevel    // this level is visible
      )
    {
        PDFOutlineEntry& rItem = m_aOutline[ nCurrentItemId ];
        sal_Int32 nChildren = rItem.m_aChildren.size();
        for( sal_Int32 i = 0; i < nChildren; i++ )
            nCount += updateOutlineItemCount( rCounts, nItemLevel+1, rItem.m_aChildren[i] );
        rCounts[nCurrentItemId] = nCount;
        // return 1 (this item) + visible sub items
        if( nCount < 0 )
            nCount = 0;
        nCount++;
    }
    else
    {
        // this bookmark level is invisible
        PDFOutlineEntry& rItem = m_aOutline[ nCurrentItemId ];
        sal_Int32 nChildren = rItem.m_aChildren.size();
        rCounts[ nCurrentItemId ] = -sal_Int32(rItem.m_aChildren.size());
        for( sal_Int32 i = 0; i < nChildren; i++ )
            updateOutlineItemCount( rCounts, nItemLevel+1, rItem.m_aChildren[i] );
        nCount = -1;
    }

    return nCount;
}

sal_Int32 PDFWriterImpl::emitOutline()
{
    int i, nItems = m_aOutline.size();

    // do we have an outline at all ?
    if( nItems < 2 )
        return 0;

    // reserve object numbers for all outline items
    for( i = 0; i < nItems; ++i )
        m_aOutline[i].m_nObject = createObject();

    // update all parent, next and prev object ids
    for( i = 0; i < nItems; ++i )
    {
        PDFOutlineEntry& rItem = m_aOutline[i];
        int nChildren = rItem.m_aChildren.size();

        if( nChildren )
        {
            for( int n = 0; n < nChildren; ++n )
            {
                PDFOutlineEntry& rChild = m_aOutline[ rItem.m_aChildren[n] ];

                rChild.m_nParentObject = rItem.m_nObject;
                rChild.m_nPrevObject = (n > 0) ? m_aOutline[ rItem.m_aChildren[n-1] ].m_nObject : 0;
                rChild.m_nNextObject = (n < nChildren-1) ? m_aOutline[ rItem.m_aChildren[n+1] ].m_nObject : 0;
            }

        }
    }

    // calculate Count entries for all items
    std::vector< sal_Int32 > aCounts( nItems );
    updateOutlineItemCount( aCounts, 0, 0 );

    // emit hierarchy
    for( i = 0; i < nItems; ++i )
    {
        PDFOutlineEntry& rItem = m_aOutline[i];
        OStringBuffer aLine( 1024 );

        CHECK_RETURN( updateObject( rItem.m_nObject ) );
        aLine.append( rItem.m_nObject );
        aLine.append( " 0 obj\n" );
        aLine.append( "<<" );
        // number of visible children (all levels)
        if( i > 0 || aCounts[0] > 0 )
        {
            aLine.append( "/Count " );
            aLine.append( aCounts[i] );
        }
        if( ! rItem.m_aChildren.empty() )
        {
            // children list: First, Last
            aLine.append( "/First " );
            aLine.append( m_aOutline[rItem.m_aChildren.front()].m_nObject );
            aLine.append( " 0 R/Last " );
            aLine.append( m_aOutline[rItem.m_aChildren.back()].m_nObject );
            aLine.append( " 0 R\n" );
        }
        if( i > 0 )
        {
            // Title, Dest, Parent, Prev, Next
            aLine.append( "/Title" );
            appendUnicodeTextStringEncrypt( rItem.m_aTitle, rItem.m_nObject, aLine );
            aLine.append( "\n" );
            // Dest is not required
            if( rItem.m_nDestID >= 0 && rItem.m_nDestID < (sal_Int32)m_aDests.size() )
            {
                aLine.append( "/Dest" );
                appendDest( rItem.m_nDestID, aLine );
            }
            aLine.append( "/Parent " );
            aLine.append( rItem.m_nParentObject );
            aLine.append( " 0 R" );
            if( rItem.m_nPrevObject )
            {
                aLine.append( "/Prev " );
                aLine.append( rItem.m_nPrevObject );
                aLine.append( " 0 R" );
            }
            if( rItem.m_nNextObject )
            {
                aLine.append( "/Next " );
                aLine.append( rItem.m_nNextObject );
                aLine.append( " 0 R" );
            }
        }
        aLine.append( ">>\nendobj\n\n" );
        CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );
    }

    return m_aOutline[0].m_nObject;
}

#undef CHECK_RETURN
#define CHECK_RETURN( x ) if( !x ) return false

bool PDFWriterImpl::appendDest( sal_Int32 nDestID, OStringBuffer& rBuffer )
{
    if( nDestID < 0 || nDestID >= (sal_Int32)m_aDests.size() )
    {
#if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "ERROR: invalid dest %d requested\n", (int)nDestID );
#endif
        return false;
    }


    const PDFDest& rDest		= m_aDests[ nDestID ];
    const PDFPage& rDestPage	= m_aPages[ rDest.m_nPage ];

    rBuffer.append( '[' );
    rBuffer.append( rDestPage.m_nPageObject );
    rBuffer.append( " 0 R" );

    switch( rDest.m_eType )
    {
        case PDFWriter::XYZ:
        default:
            rBuffer.append( "/XYZ " );
            appendFixedInt( rDest.m_aRect.Left(), rBuffer );
            rBuffer.append( ' ' );
            appendFixedInt( rDest.m_aRect.Bottom(), rBuffer );
            rBuffer.append( " 0" );
            break;
        case PDFWriter::Fit:
            rBuffer.append( "/Fit" );
            break;
        case PDFWriter::FitRectangle:
            rBuffer.append( "/FitR " );
            appendFixedInt( rDest.m_aRect.Left(), rBuffer );
            rBuffer.append( ' ' );
            appendFixedInt( rDest.m_aRect.Top(), rBuffer );
            rBuffer.append( ' ' );
            appendFixedInt( rDest.m_aRect.Right(), rBuffer );
            rBuffer.append( ' ' );
            appendFixedInt( rDest.m_aRect.Bottom(), rBuffer );
            break;
        case PDFWriter::FitHorizontal:
            rBuffer.append( "/FitH " );
            appendFixedInt( rDest.m_aRect.Bottom(), rBuffer );
            break;
        case PDFWriter::FitVertical:
            rBuffer.append( "/FitV " );
            appendFixedInt( rDest.m_aRect.Left(), rBuffer );
            break;
        case PDFWriter::FitPageBoundingBox:
            rBuffer.append( "/FitB" );
            break;
        case PDFWriter::FitPageBoundingBoxHorizontal:
            rBuffer.append( "/FitBH " );
            appendFixedInt( rDest.m_aRect.Bottom(), rBuffer );
            break;
        case PDFWriter::FitPageBoundingBoxVertical:
            rBuffer.append( "/FitBV " );
            appendFixedInt( rDest.m_aRect.Left(), rBuffer );
            break;
    }
    rBuffer.append( ']' );

    return true;
}

bool PDFWriterImpl::emitLinkAnnotations()
{
    int nAnnots = m_aLinks.size();
    for( int i = 0; i < nAnnots; i++ )
    {
        const PDFLink& rLink			= m_aLinks[i];
        if( ! updateObject( rLink.m_nObject ) )
            continue;

        OStringBuffer aLine( 1024 );
        aLine.append( rLink.m_nObject );
        aLine.append( " 0 obj\n" );
//i59651  key /F set bits Print to 1 rest to 0. We don't set NoZoom NoRotate to 1, since it's a 'should'
// see PDF 8.4.2 and ISO 19005-1:2005 6.5.3
        aLine.append( "<</Type/Annot" );
        if( m_bIsPDF_A1 )
            aLine.append( "/F 4" );
        aLine.append( "/Subtype/Link/Border[0 0 0]/Rect[" );

        appendFixedInt( rLink.m_aRect.Left()-7, aLine );//the +7 to have a better shape of the border rectangle
        aLine.append( ' ' );
        appendFixedInt( rLink.m_aRect.Top(), aLine );
        aLine.append( ' ' );
        appendFixedInt( rLink.m_aRect.Right()+7, aLine );//the +7 to have a better shape of the border rectangle
        aLine.append( ' ' );
        appendFixedInt( rLink.m_aRect.Bottom(), aLine );
        aLine.append( "]" );
        if( rLink.m_nDest >= 0 )
        {
            aLine.append( "/Dest" );
            appendDest( rLink.m_nDest, aLine );
        }
        else
        {
/*--->i56629
destination is external to the document, so
we check in the following sequence:

 if target type is neither .pdf, nor .od[tpgs], then
          check if relative or absolute and act accordingly (use URI or 'launch application' as requested)
                             end processing
 else if target is .od[tpgs]: then
      if conversion of type from od[tpgs]  to pdf is requested, convert it and this becomes the new target file
      processing continue

 if (new)target is .pdf : then
     if GotToR is requested, then
           convert the target in GoToR where the fragment of the URI is
           considered the named destination in the target file, set relative or absolute as requested
     else strip the fragment from URL and then set URI or 'launch application' as requested
*/
//
// FIXME: check if the decode mechanisms for URL processing throughout this implementation
// are the correct one!!
//
// extract target file type
            INetURLObject aDocumentURL( m_aContext.BaseURL );
            INetURLObject aTargetURL( rLink.m_aURL );
            sal_Int32   nChangeFileExtensionToPDF = 0;
            sal_Int32   nSetGoToRMode = 0;
            sal_Bool    bTargetHasPDFExtension = sal_False;
            INetProtocol eTargetProtocol = aTargetURL.GetProtocol();
			sal_Bool    bIsUNCPath = sal_False;
// check if the protocol is a known one, or if there is no protocol at all (on target only)
// if there is no protocol, make the target relative to the current document directory
// getting the needed URL information from the current document path
            if( eTargetProtocol == INET_PROT_NOT_VALID )
            {
				if( rLink.m_aURL.getLength() > 4 && rLink.m_aURL.compareToAscii( "\\\\\\\\", 4 ) == 0)
				{
					bIsUNCPath = sal_True;
				}
				else
				{
					INetURLObject aNewBase( aDocumentURL );//duplicate document URL
					aNewBase.removeSegment(); //remove last segment from it, obtaining the base URL of the
											  //target document
					aNewBase.insertName( rLink.m_aURL );
					aTargetURL = aNewBase;//reassign the new target URL
//recompute the target protocol, with the new URL
//normal URL processing resumes
					eTargetProtocol = aTargetURL.GetProtocol();
				}
            }

            rtl::OUString aFileExtension = aTargetURL.GetFileExtension();

// Check if the URL ends in '/': if yes it's a directory,
// it will be forced to a URI link.
// possibly a malformed URI, leave it as it is, force as URI
            if( aTargetURL.hasFinalSlash() )
                m_aContext.DefaultLinkAction = PDFWriter::URIAction;

            if( aFileExtension.getLength() > 0 )
            {
                if( m_aContext.ConvertOOoTargetToPDFTarget )
                {
//examine the file type (.odm .odt. .odp, odg, ods)
                    if( aFileExtension.equalsIgnoreAsciiCase(rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "odm" ) ) ) )
                        nChangeFileExtensionToPDF++;
                    if( aFileExtension.equalsIgnoreAsciiCase(rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "odt" ) ) ) )
                        nChangeFileExtensionToPDF++;
                    else if( aFileExtension.equalsIgnoreAsciiCase(rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "odp" ) ) ) )
                        nChangeFileExtensionToPDF++;
                    else if( aFileExtension.equalsIgnoreAsciiCase(rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "odg" ) ) ) )
                        nChangeFileExtensionToPDF++;
                    else if( aFileExtension.equalsIgnoreAsciiCase(rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ods" ) ) ) )
                        nChangeFileExtensionToPDF++;
                    if( nChangeFileExtensionToPDF )
                        aTargetURL.setExtension(rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "pdf" ) ) );
                }
//check if extension is pdf, see if GoToR should be forced
                bTargetHasPDFExtension = aTargetURL.GetFileExtension().equalsIgnoreAsciiCase(rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "pdf" ) ) );
                if( m_aContext.ForcePDFAction && bTargetHasPDFExtension )
                    nSetGoToRMode++;
            }
//prepare the URL, if relative or not
            INetProtocol eBaseProtocol = aDocumentURL.GetProtocol();
//queue the string common to all types of actions
            aLine.append( "/A<</Type/Action/S");
			if( bIsUNCPath ) // handle Win UNC paths
			{
				aLine.append( "/Launch/Win<</F" );
				// INetURLObject is not good with UNC paths, use original path
				appendLiteralStringEncrypt(  rLink.m_aURL, rLink.m_nObject, aLine );
				aLine.append( ">>" );
			}
			else
			{
				sal_Int32 nSetRelative = 0;
//check if relative file link is requested and if the protocol is 'file://'
				if( m_aContext.RelFsys && eBaseProtocol == eTargetProtocol && eTargetProtocol == INET_PROT_FILE )
					nSetRelative++;

				rtl::OUString aFragment = aTargetURL.GetMark( INetURLObject::NO_DECODE /*DECODE_WITH_CHARSET*/ ); //fragment as is,
				if( nSetGoToRMode == 0 )
					switch( m_aContext.DefaultLinkAction )
					{
					default:
					case PDFWriter::URIAction :
					case PDFWriter::URIActionDestination :
						aLine.append( "/URI/URI" );
						break;
					case PDFWriter::LaunchAction:
// now:
// if a launch action is requested and the hyperlink target has a fragment
// and the target file does not have a pdf extension, or it's not a 'file:://' protocol
// then force the uri action on it
// This code will permit the correct opening of application on web pages, the one that
// normally have fragments (but I may be wrong...)
// and will force the use of URI when the protocol is not file://
						if( (aFragment.getLength() > 0 && !bTargetHasPDFExtension) ||
										eTargetProtocol != INET_PROT_FILE )
							aLine.append( "/URI/URI" );
						else
							aLine.append( "/Launch/F" );
						break;
					}
//fragment are encoded in the same way as in the named destination processing
				rtl::OUString aURLNoMark = aTargetURL.GetURLNoMark( INetURLObject::DECODE_WITH_CHARSET );
				if( nSetGoToRMode )
				{//add the fragment
					aLine.append("/GoToR");
					aLine.append("/F");
					appendLiteralStringEncrypt( nSetRelative ? INetURLObject::GetRelURL( m_aContext.BaseURL, aURLNoMark,
																						 INetURLObject::WAS_ENCODED,
																						 INetURLObject::DECODE_WITH_CHARSET ) :
																   aURLNoMark, rLink.m_nObject, aLine );
					if( aFragment.getLength() > 0 )
					{
						aLine.append("/D/");
						appendDestinationName( aFragment , aLine );
					}
				}
				else
				{
// change the fragment to accomodate the bookmark (only if the file extension is PDF and
// the requested action is of the correct type)
					if(m_aContext.DefaultLinkAction == PDFWriter::URIActionDestination &&
							   bTargetHasPDFExtension && aFragment.getLength() > 0 )
					{
						OStringBuffer aLineLoc( 1024 );
						appendDestinationName( aFragment , aLineLoc );
//substitute the fragment
						aTargetURL.SetMark( aLineLoc.getStr() );
					}
					rtl::OUString aURL = aTargetURL.GetMainURL( (nSetRelative || eTargetProtocol == INET_PROT_FILE) ? INetURLObject::DECODE_WITH_CHARSET : INetURLObject::NO_DECODE );
// check if we have a URL available, if the string is empty, set it as the original one
//                 if( aURL.getLength() == 0 )
//                     appendLiteralStringEncrypt( rLink.m_aURL , rLink.m_nObject, aLine );
//                 else
						appendLiteralStringEncrypt( nSetRelative ? INetURLObject::GetRelURL( m_aContext.BaseURL, aURL ) :
																   aURL , rLink.m_nObject, aLine );
				}
//<--- i56629
			}
            aLine.append( ">>\n" );
        }
        if( rLink.m_nStructParent > 0 )
        {
            aLine.append( "/StructParent " );
            aLine.append( rLink.m_nStructParent );
        }
        aLine.append( ">>\nendobj\n\n" );
        CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );
    }

    return true;
}

bool PDFWriterImpl::emitNoteAnnotations()
{
    // emit note annotations
    int nAnnots = m_aNotes.size();
    for( int i = 0; i < nAnnots; i++ )
    {
        const PDFNoteEntry& rNote		= m_aNotes[i];
        if( ! updateObject( rNote.m_nObject ) )
            return false;

        OStringBuffer aLine( 1024 );
        aLine.append( rNote.m_nObject );
        aLine.append( " 0 obj\n" );
//i59651  key /F set bits Print to 1 rest to 0. We don't set NoZoom NoRotate to 1, since it's a 'should'
// see PDF 8.4.2 and ISO 19005-1:2005 6.5.3
        aLine.append( "<</Type/Annot" );
        if( m_bIsPDF_A1 )
            aLine.append( "/F 4" );
        aLine.append( "/Subtype/Text/Rect[" );

        appendFixedInt( rNote.m_aRect.Left(), aLine );
        aLine.append( ' ' );
        appendFixedInt( rNote.m_aRect.Top(), aLine );
        aLine.append( ' ' );
        appendFixedInt( rNote.m_aRect.Right(), aLine );
        aLine.append( ' ' );
        appendFixedInt( rNote.m_aRect.Bottom(), aLine );
        aLine.append( "]" );

        // contents of the note (type text string)
        aLine.append( "/Contents\n" );
        appendUnicodeTextStringEncrypt( rNote.m_aContents.Contents, rNote.m_nObject, aLine );
        aLine.append( "\n" );

        // optional title
        if( rNote.m_aContents.Title.Len() )
        {
            aLine.append( "/T" );
            appendUnicodeTextStringEncrypt( rNote.m_aContents.Title, rNote.m_nObject, aLine );
            aLine.append( "\n" );
        }

        aLine.append( ">>\nendobj\n\n" );
        CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );
    }
    return true;
}

Font PDFWriterImpl::replaceFont( const Font& rControlFont, const Font&  rAppSetFont )
{
    bool bAdjustSize = false;

    Font aFont( rControlFont );
    if( ! aFont.GetName().Len() )
    {
        aFont = rAppSetFont;
        if( rControlFont.GetHeight() )
            aFont.SetSize( Size( 0, rControlFont.GetHeight() ) );
        else
            bAdjustSize = true;
        if( rControlFont.GetItalic() != ITALIC_DONTKNOW )
            aFont.SetItalic( rControlFont.GetItalic() );
        if( rControlFont.GetWeight() != WEIGHT_DONTKNOW )
            aFont.SetWeight( rControlFont.GetWeight() );
    }
    else if( ! aFont.GetHeight() )
    {
        aFont.SetSize( rAppSetFont.GetSize() );
        bAdjustSize = true;
    }
    if( bAdjustSize )
    {
        Size aFontSize = aFont.GetSize();
        OutputDevice* pDefDev = Application::GetDefaultDevice();
        aFontSize = OutputDevice::LogicToLogic( aFontSize, pDefDev->GetMapMode(), getMapMode() );
        aFont.SetSize( aFontSize );
    }
    return aFont;
}

sal_Int32 PDFWriterImpl::getBestBuiltinFont( const Font& rFont )
{
    sal_Int32 nBest = 4; // default to Helvetica
    OUString aFontName( rFont.GetName() );
    aFontName = aFontName.toAsciiLowerCase();

    if( aFontName.indexOf( OUString( RTL_CONSTASCII_USTRINGPARAM( "times" ) ) ) != -1 )
        nBest = 8;
    else if( aFontName.indexOf( OUString( RTL_CONSTASCII_USTRINGPARAM( "courier" ) ) ) != -1 )
        nBest = 0;
    else if( aFontName.indexOf( OUString( RTL_CONSTASCII_USTRINGPARAM( "dingbats" ) ) ) != -1 )
        nBest = 13;
    else if( aFontName.indexOf( OUString( RTL_CONSTASCII_USTRINGPARAM( "symbol" ) ) ) != -1 )
        nBest = 12;
    if( nBest < 12 )
    {
        if( rFont.GetItalic() == ITALIC_OBLIQUE || rFont.GetItalic() == ITALIC_NORMAL )
            nBest += 1;
        if( rFont.GetWeight() > WEIGHT_MEDIUM )
            nBest += 2;
    }

    if( m_aBuiltinFontToObjectMap.find( nBest ) == m_aBuiltinFontToObjectMap.end() )
        m_aBuiltinFontToObjectMap[ nBest ] = createObject();

    return nBest;
}

static inline const Color& replaceColor( const Color& rCol1, const Color& rCol2 )
{
    return (rCol1 == Color( COL_TRANSPARENT )) ? rCol2 : rCol1;
}

void PDFWriterImpl::createDefaultPushButtonAppearance( PDFWidget& rButton, const PDFWriter::PushButtonWidget& rWidget )
{
    const StyleSettings& rSettings = Application::GetSettings().GetStyleSettings();

    // save graphics state
    push( sal::static_int_cast<sal_uInt16>(~0U) );

    // transform relative to control's coordinates since an
    // appearance stream is a form XObject
    // this relies on the m_aRect member of rButton NOT already being transformed
    // to default user space
    if( rWidget.Background || rWidget.Border )
    {
        setLineColor( rWidget.Border ? replaceColor( rWidget.BorderColor, rSettings.GetLightColor() ) : Color( COL_TRANSPARENT ) );
        setFillColor( rWidget.Background ? replaceColor( rWidget.BackgroundColor, rSettings.GetDialogColor() ) : Color( COL_TRANSPARENT ) );
        drawRectangle( rWidget.Location );
    }
    // prepare font to use
    Font aFont = replaceFont( rWidget.TextFont, rSettings.GetPushButtonFont() );
    setFont( aFont );
    setTextColor( replaceColor( rWidget.TextColor, rSettings.GetButtonTextColor() ) );

    drawText( rButton.m_aRect, rButton.m_aText, rButton.m_nTextStyle );

    // create DA string while local mapmode is still in place
    // (that is before endRedirect())
    OStringBuffer aDA( 256 );
    appendNonStrokingColor( replaceColor( rWidget.TextColor, rSettings.GetButtonTextColor() ), aDA );
    Font aDummyFont( String( RTL_CONSTASCII_USTRINGPARAM( "Helvetica" ) ), aFont.GetSize() );
    sal_Int32 nDummyBuiltin = getBestBuiltinFont( aDummyFont );
    aDA.append( ' ' );
    aDA.append( m_aBuiltinFonts[nDummyBuiltin].getNameObject() );
    aDA.append( ' ' );
    m_aPages[m_nCurrentPage].appendMappedLength( sal_Int32( aFont.GetHeight() ), aDA );
    aDA.append( " Tf" );
    rButton.m_aDAString = aDA.makeStringAndClear();

    pop();

    rButton.m_aAppearances[ "N" ][ "Standard" ] = new SvMemoryStream();

    /* seems like a bad hack but at least works in both AR5 and 6:
       we draw the button ourselves and tell AR
       the button would be totally transparent with no text

       One would expect that simply setting a normal appearance
       should suffice, but no, as soon as the user actually presses
       the button and an action is tied to it (gasp! a button that
       does something) the appearance gets replaced by some crap that AR
       creates on the fly even if no DA or MK is given. On AR6 at least
       the DA and MK work as expected, but on AR5 this creates a region
       filled with the background color but nor text. Urgh.
    */
    rButton.m_aMKDict = "/BC [] /BG [] /CA";
    rButton.m_aMKDictCAString = "";
}

Font PDFWriterImpl::drawFieldBorder( PDFWidget& rIntern,
                                     const PDFWriter::AnyWidget& rWidget,
                                     const StyleSettings& rSettings )
{
    Font aFont = replaceFont( rWidget.TextFont, rSettings.GetFieldFont() );

    if( rWidget.Background || rWidget.Border )
    {
        if( rWidget.Border && rWidget.BorderColor == Color( COL_TRANSPARENT ) )
        {
            sal_Int32 nDelta = getReferenceDevice()->ImplGetDPIX() / 500;
            if( nDelta < 1 )
                nDelta = 1;
            setLineColor( Color( COL_TRANSPARENT ) );
            Rectangle aRect = rIntern.m_aRect;
            setFillColor( rSettings.GetLightBorderColor() );
            drawRectangle( aRect );
            aRect.Left()  += nDelta; aRect.Top()     += nDelta;
            aRect.Right() -= nDelta; aRect.Bottom()  -= nDelta;
            setFillColor( rSettings.GetFieldColor() );
            drawRectangle( aRect );
            setFillColor( rSettings.GetLightColor() );
            drawRectangle( Rectangle( Point( aRect.Left(), aRect.Bottom()-nDelta ), aRect.BottomRight() ) );
            drawRectangle( Rectangle( Point( aRect.Right()-nDelta, aRect.Top() ), aRect.BottomRight() ) );
            setFillColor( rSettings.GetDarkShadowColor() );
            drawRectangle( Rectangle( aRect.TopLeft(), Point( aRect.Left()+nDelta, aRect.Bottom() ) ) );
            drawRectangle( Rectangle( aRect.TopLeft(), Point( aRect.Right(), aRect.Top()+nDelta ) ) );
        }
        else
        {
            setLineColor( rWidget.Border ? replaceColor( rWidget.BorderColor, rSettings.GetShadowColor() ) : Color( COL_TRANSPARENT ) );
            setFillColor( rWidget.Background ? replaceColor( rWidget.BackgroundColor, rSettings.GetFieldColor() ) : Color( COL_TRANSPARENT ) );
            drawRectangle( rIntern.m_aRect );
        }

        if( rWidget.Border )
        {
            // adjust edit area accounting for border
            sal_Int32 nDelta = aFont.GetHeight()/4;
            if( nDelta < 1 )
                nDelta = 1;
            rIntern.m_aRect.Left()	+= nDelta;
            rIntern.m_aRect.Top()	+= nDelta;
            rIntern.m_aRect.Right()	-= nDelta;
            rIntern.m_aRect.Bottom()-= nDelta;
        }
    }
    return aFont;
}

void PDFWriterImpl::createDefaultEditAppearance( PDFWidget& rEdit, const PDFWriter::EditWidget& rWidget )
{
    const StyleSettings& rSettings = Application::GetSettings().GetStyleSettings();
    SvMemoryStream* pEditStream = new SvMemoryStream( 1024, 1024 );

    push( sal::static_int_cast<sal_uInt16>(~0U) );

    // prepare font to use, draw field border
    Font aFont = drawFieldBorder( rEdit, rWidget, rSettings );
    sal_Int32 nBest = getBestBuiltinFont( aFont );

    // prepare DA string
    OStringBuffer aDA( 32 );
    appendNonStrokingColor( replaceColor( rWidget.TextColor, rSettings.GetFieldTextColor() ), aDA );
    aDA.append( ' ' );
    aDA.append( m_aBuiltinFonts[nBest].getNameObject() );
    aDA.append( ' ' );
    m_aPages[ m_nCurrentPage ].appendMappedLength( sal_Int32( aFont.GetHeight() ), aDA );
    aDA.append( " Tf" );

    /*  create an empty appearance stream, let the viewer create
        the appearance at runtime. This is because AR5 seems to
        paint the widget appearance always, and a dynamically created
        appearance on top of it. AR6 is well behaved in that regard, so
        that behaviour seems to be a bug. Anyway this empty appearance
        relies on /NeedAppearances in the AcroForm dictionary set to "true"
     */
    beginRedirect( pEditStream, rEdit.m_aRect );
    OStringBuffer aAppearance( 32 );
    aAppearance.append( "/Tx BMC\nEMC\n" );
    writeBuffer( aAppearance.getStr(), aAppearance.getLength() );

    endRedirect();
    pop();

    rEdit.m_aAppearances[ "N" ][ "Standard" ] = pEditStream;

    rEdit.m_aDAString = aDA.makeStringAndClear();
}

void PDFWriterImpl::createDefaultListBoxAppearance( PDFWidget& rBox, const PDFWriter::ListBoxWidget& rWidget )
{
    const StyleSettings& rSettings = Application::GetSettings().GetStyleSettings();
    SvMemoryStream* pListBoxStream = new SvMemoryStream( 1024, 1024 );

    push( sal::static_int_cast<sal_uInt16>(~0U) );

    // prepare font to use, draw field border
    Font aFont = drawFieldBorder( rBox, rWidget, rSettings );
    sal_Int32 nBest = getBestBuiltinFont( aFont );

    beginRedirect( pListBoxStream, rBox.m_aRect );
    OStringBuffer aAppearance( 64 );

#if 0
    if( ! rWidget.DropDown )
    {
        // prepare linewidth for DA string hack, see below
        Size aFontSize = lcl_convert( m_aGraphicsStack.front().m_aMapMode,
                                      m_aMapMode,
                                      getReferenceDevice(),
                                      Size( 0, aFont.GetHeight() ) );
        sal_Int32 nLW = aFontSize.Height() / 40;
        appendFixedInt( nLW > 0 ? nLW : 1, aAppearance );
        aAppearance.append( " w\n" );
        writeBuffer( aAppearance.getStr(), aAppearance.getLength() );
        aAppearance.setLength( 0 );
    }
#endif

    setLineColor( Color( COL_TRANSPARENT ) );
    setFillColor( replaceColor( rWidget.BackgroundColor, rSettings.GetFieldColor() ) );
    drawRectangle( rBox.m_aRect );

    // empty appearance, see createDefaultEditAppearance for reference
    aAppearance.append( "/Tx BMC\nEMC\n" );
    writeBuffer( aAppearance.getStr(), aAppearance.getLength() );

    endRedirect();
    pop();

    rBox.m_aAppearances[ "N" ][ "Standard" ] = pListBoxStream;

    // prepare DA string
    OStringBuffer aDA( 256 );
#if 0
    if( !rWidget.DropDown )
    {
        /* another of AR5's peculiarities: the selected item of a choice
           field is highlighted using the non stroking color - same as the
           text color. so workaround that by using text rendering mode 2
           (fill, then stroke) and set the stroking color
         */
        appendStrokingColor( replaceColor( rWidget.BackgroundColor, rSettings.GetFieldColor() ), aDA );
        aDA.append( " 2 Tr " );
    }
#endif
    appendNonStrokingColor( replaceColor( rWidget.TextColor, rSettings.GetFieldTextColor() ), aDA );
    aDA.append( ' ' );
    aDA.append( m_aBuiltinFonts[nBest].getNameObject() );
    aDA.append( ' ' );
    m_aPages[ m_nCurrentPage ].appendMappedLength( sal_Int32( aFont.GetHeight() ), aDA );
    aDA.append( " Tf" );
    rBox.m_aDAString = aDA.makeStringAndClear();
}

void PDFWriterImpl::createDefaultCheckBoxAppearance( PDFWidget& rBox, const PDFWriter::CheckBoxWidget& rWidget )
{
    const StyleSettings& rSettings = Application::GetSettings().GetStyleSettings();

    // save graphics state
    push( sal::static_int_cast<sal_uInt16>(~0U) );

    if( rWidget.Background || rWidget.Border )
    {
        setLineColor( rWidget.Border ? replaceColor( rWidget.BorderColor, rSettings.GetCheckedColor() ) : Color( COL_TRANSPARENT ) );
        setFillColor( rWidget.Background ? replaceColor( rWidget.BackgroundColor, rSettings.GetFieldColor() ) : Color( COL_TRANSPARENT ) );
        drawRectangle( rBox.m_aRect );
    }

    Font aFont = replaceFont( rWidget.TextFont, rSettings.GetRadioCheckFont() );
    setFont( aFont );
    Size aFontSize = aFont.GetSize();
    if( aFontSize.Height() > rBox.m_aRect.GetHeight() )
        aFontSize.Height() = rBox.m_aRect.GetHeight();
    sal_Int32 nDelta = aFontSize.Height()/10;
    if( nDelta < 1 )
        nDelta = 1;

    Rectangle aCheckRect, aTextRect;
    if( rWidget.ButtonIsLeft )
    {
        aCheckRect.Left()	= rBox.m_aRect.Left() + nDelta;
        aCheckRect.Top()	= rBox.m_aRect.Top() + (rBox.m_aRect.GetHeight()-aFontSize.Height())/2;
        aCheckRect.Right()	= aCheckRect.Left() + aFontSize.Height();
        aCheckRect.Bottom()	= aCheckRect.Top() + aFontSize.Height();

        // #i74206# handle small controls without text area
        while( aCheckRect.GetWidth() > rBox.m_aRect.GetWidth() && aCheckRect.GetWidth() > nDelta )
        {
            aCheckRect.Right()  -= nDelta;
            aCheckRect.Top()    += nDelta/2;
            aCheckRect.Bottom() -= nDelta - (nDelta/2);
        }

        aTextRect.Left()	= rBox.m_aRect.Left() + aCheckRect.GetWidth()+5*nDelta;
        aTextRect.Top()		= rBox.m_aRect.Top();
        aTextRect.Right()	= aTextRect.Left() + rBox.m_aRect.GetWidth() - aCheckRect.GetWidth()-6*nDelta;
        aTextRect.Bottom()	= rBox.m_aRect.Bottom();
    }
    else
    {
        aCheckRect.Left()	= rBox.m_aRect.Right() - nDelta - aFontSize.Height();
        aCheckRect.Top()	= rBox.m_aRect.Top() + (rBox.m_aRect.GetHeight()-aFontSize.Height())/2;
        aCheckRect.Right()	= aCheckRect.Left() + aFontSize.Height();
        aCheckRect.Bottom()	= aCheckRect.Top() + aFontSize.Height();

        // #i74206# handle small controls without text area
        while( aCheckRect.GetWidth() > rBox.m_aRect.GetWidth() && aCheckRect.GetWidth() > nDelta )
        {
            aCheckRect.Left()   += nDelta;
            aCheckRect.Top()    += nDelta/2;
            aCheckRect.Bottom() -= nDelta - (nDelta/2);
        }

        aTextRect.Left()	= rBox.m_aRect.Left();
        aTextRect.Top()		= rBox.m_aRect.Top();
        aTextRect.Right()	= aTextRect.Left() + rBox.m_aRect.GetWidth() - aCheckRect.GetWidth()-6*nDelta;
        aTextRect.Bottom()	= rBox.m_aRect.Bottom();
    }
    setLineColor( Color( COL_BLACK ) );
    setFillColor( Color( COL_TRANSPARENT ) );
    OStringBuffer aLW( 32 );
    aLW.append( "q " );
    m_aPages[m_nCurrentPage].appendMappedLength( nDelta, aLW );
    aLW.append( " w " );
    writeBuffer( aLW.getStr(), aLW.getLength() );
    drawRectangle( aCheckRect );
    writeBuffer( " Q\n", 3 );
    setTextColor( replaceColor( rWidget.TextColor, rSettings.GetRadioCheckTextColor() ) );
    drawText( aTextRect, rBox.m_aText, rBox.m_nTextStyle );

    pop();

    OStringBuffer aDA( 256 );
    appendNonStrokingColor( replaceColor( rWidget.TextColor, rSettings.GetRadioCheckTextColor() ), aDA );
    sal_Int32 nBest = getBestBuiltinFont( Font( String( RTL_CONSTASCII_USTRINGPARAM( "ZapfDingbats" ) ), aFont.GetSize() ) );
    aDA.append( ' ' );
    aDA.append( m_aBuiltinFonts[nBest].getNameObject() );
    aDA.append( " 0 Tf" );
    rBox.m_aDAString = aDA.makeStringAndClear();
    rBox.m_aMKDict = "/CA";
    rBox.m_aMKDictCAString = "8";
    rBox.m_aRect = aCheckRect;

    // create appearance streams
    sal_Char cMark = '8';
    sal_Int32 nCharXOffset = 1000-m_aBuiltinFonts[13].m_aWidths[sal_Int32(cMark)];
    nCharXOffset *= aCheckRect.GetHeight();
    nCharXOffset /= 2000;
    sal_Int32 nCharYOffset = 1000-
        (m_aBuiltinFonts[13].m_nAscent+m_aBuiltinFonts[13].m_nDescent); // descent is negative
    nCharYOffset *= aCheckRect.GetHeight();
    nCharYOffset /= 2000;

    SvMemoryStream* pCheckStream = new SvMemoryStream( 256, 256 );
    beginRedirect( pCheckStream, aCheckRect );
    aDA.append( "/Tx BMC\nq BT\n" );
    appendNonStrokingColor( replaceColor( rWidget.TextColor, rSettings.GetRadioCheckTextColor() ), aDA );
    aDA.append( ' ' );
    aDA.append( m_aBuiltinFonts[nBest].getNameObject() );
    aDA.append( ' ' );
    m_aPages[ m_nCurrentPage ].appendMappedLength( sal_Int32( aCheckRect.GetHeight() ), aDA );
    aDA.append( " Tf\n" );
    m_aPages[ m_nCurrentPage ].appendMappedLength( nCharXOffset, aDA );
	aDA.append( " " );
	m_aPages[ m_nCurrentPage ].appendMappedLength( nCharYOffset, aDA );
	aDA.append( " Td (" );
	aDA.append( cMark );
	aDA.append( ") Tj\nET\nQ\nEMC\n" );
    writeBuffer( aDA.getStr(), aDA.getLength() );
    endRedirect();
    rBox.m_aAppearances[ "N" ][ "Yes" ] = pCheckStream;

    SvMemoryStream* pUncheckStream = new SvMemoryStream( 256, 256 );
    beginRedirect( pUncheckStream, aCheckRect );
    writeBuffer( "/Tx BMC\nEMC\n", 12 );
    endRedirect();
    rBox.m_aAppearances[ "N" ][ "Off" ] = pUncheckStream;
}

void PDFWriterImpl::createDefaultRadioButtonAppearance( PDFWidget& rBox, const PDFWriter::RadioButtonWidget& rWidget )
{
    const StyleSettings& rSettings = Application::GetSettings().GetStyleSettings();

    // save graphics state
    push( sal::static_int_cast<sal_uInt16>(~0U) );

    if( rWidget.Background || rWidget.Border )
    {
        setLineColor( rWidget.Border ? replaceColor( rWidget.BorderColor, rSettings.GetCheckedColor() ) : Color( COL_TRANSPARENT ) );
        setFillColor( rWidget.Background ? replaceColor( rWidget.BackgroundColor, rSettings.GetFieldColor() ) : Color( COL_TRANSPARENT ) );
        drawRectangle( rBox.m_aRect );
    }

    Font aFont = replaceFont( rWidget.TextFont, rSettings.GetRadioCheckFont() );
    setFont( aFont );
    Size aFontSize = aFont.GetSize();
    if( aFontSize.Height() > rBox.m_aRect.GetHeight() )
        aFontSize.Height() = rBox.m_aRect.GetHeight();
    sal_Int32 nDelta = aFontSize.Height()/10;
    if( nDelta < 1 )
        nDelta = 1;

    Rectangle aCheckRect, aTextRect;
    if( rWidget.ButtonIsLeft )
    {
        aCheckRect.Left()	= rBox.m_aRect.Left() + nDelta;
        aCheckRect.Top()	= rBox.m_aRect.Top() + (rBox.m_aRect.GetHeight()-aFontSize.Height())/2;
        aCheckRect.Right()	= aCheckRect.Left() + aFontSize.Height();
        aCheckRect.Bottom()	= aCheckRect.Top() + aFontSize.Height();

        // #i74206# handle small controls without text area
        while( aCheckRect.GetWidth() > rBox.m_aRect.GetWidth() && aCheckRect.GetWidth() > nDelta )
        {
            aCheckRect.Right()  -= nDelta;
            aCheckRect.Top()    += nDelta/2;
            aCheckRect.Bottom() -= nDelta - (nDelta/2);
        }

        aTextRect.Left()	= rBox.m_aRect.Left() + aCheckRect.GetWidth()+5*nDelta;
        aTextRect.Top()		= rBox.m_aRect.Top();
        aTextRect.Right()	= aTextRect.Left() + rBox.m_aRect.GetWidth() - aCheckRect.GetWidth()-6*nDelta;
        aTextRect.Bottom()	= rBox.m_aRect.Bottom();
    }
    else
    {
        aCheckRect.Left()	= rBox.m_aRect.Right() - nDelta - aFontSize.Height();
        aCheckRect.Top()	= rBox.m_aRect.Top() + (rBox.m_aRect.GetHeight()-aFontSize.Height())/2;
        aCheckRect.Right()	= aCheckRect.Left() + aFontSize.Height();
        aCheckRect.Bottom()	= aCheckRect.Top() + aFontSize.Height();

        // #i74206# handle small controls without text area
        while( aCheckRect.GetWidth() > rBox.m_aRect.GetWidth() && aCheckRect.GetWidth() > nDelta )
        {
            aCheckRect.Left()   += nDelta;
            aCheckRect.Top()    += nDelta/2;
            aCheckRect.Bottom() -= nDelta - (nDelta/2);
        }

        aTextRect.Left()	= rBox.m_aRect.Left();
        aTextRect.Top()		= rBox.m_aRect.Top();
        aTextRect.Right()	= aTextRect.Left() + rBox.m_aRect.GetWidth() - aCheckRect.GetWidth()-6*nDelta;
        aTextRect.Bottom()	= rBox.m_aRect.Bottom();
    }
    setLineColor( Color( COL_BLACK ) );
    setFillColor( Color( COL_TRANSPARENT ) );
    OStringBuffer aLW( 32 );
    aLW.append( "q " );
    m_aPages[ m_nCurrentPage ].appendMappedLength( nDelta, aLW );
    aLW.append( " w " );
    writeBuffer( aLW.getStr(), aLW.getLength() );
    drawEllipse( aCheckRect );
    writeBuffer( " Q\n", 3 );
    setTextColor( replaceColor( rWidget.TextColor, rSettings.GetRadioCheckTextColor() ) );
    drawText( aTextRect, rBox.m_aText, rBox.m_nTextStyle );

    pop();

    OStringBuffer aDA( 256 );
    appendNonStrokingColor( replaceColor( rWidget.TextColor, rSettings.GetRadioCheckTextColor() ), aDA );
    sal_Int32 nBest = getBestBuiltinFont( Font( String( RTL_CONSTASCII_USTRINGPARAM( "ZapfDingbats" ) ), aFont.GetSize() ) );
    aDA.append( ' ' );
    aDA.append( m_aBuiltinFonts[nBest].getNameObject() );
    aDA.append( " 0 Tf" );
    rBox.m_aDAString = aDA.makeStringAndClear();
//to encrypt this (el)
    rBox.m_aMKDict = "/CA";
//after this assignement, to m_aMKDic cannot be added anything
    rBox.m_aMKDictCAString = "l";

    rBox.m_aRect = aCheckRect;

    // create appearance streams
    push( sal::static_int_cast<sal_uInt16>(~0U) );
    SvMemoryStream* pCheckStream = new SvMemoryStream( 256, 256 );

    beginRedirect( pCheckStream, aCheckRect );
    aDA.append( "/Tx BMC\nq BT\n" );
    appendNonStrokingColor( replaceColor( rWidget.TextColor, rSettings.GetRadioCheckTextColor() ), aDA );
    aDA.append( ' ' );
    aDA.append( m_aBuiltinFonts[nBest].getNameObject() );
    aDA.append( ' ' );
    m_aPages[m_nCurrentPage].appendMappedLength( sal_Int32( aCheckRect.GetHeight() ), aDA );
    aDA.append( " Tf\n0 0 Td\nET\nQ\n" );
    writeBuffer( aDA.getStr(), aDA.getLength() );
    setFillColor( replaceColor( rWidget.TextColor, rSettings.GetRadioCheckTextColor() ) );
    setLineColor( Color( COL_TRANSPARENT ) );
    aCheckRect.Left()	+= 3*nDelta;
    aCheckRect.Top()	+= 3*nDelta;
    aCheckRect.Bottom()	-= 3*nDelta;
    aCheckRect.Right()	-= 3*nDelta;
    drawEllipse( aCheckRect );
    writeBuffer( "\nEMC\n", 5 );
    endRedirect();

    pop();
    rBox.m_aAppearances[ "N" ][ "Yes" ] = pCheckStream;

    SvMemoryStream* pUncheckStream = new SvMemoryStream( 256, 256 );
    beginRedirect( pUncheckStream, aCheckRect );
    writeBuffer( "/Tx BMC\nEMC\n", 12 );
    endRedirect();
    rBox.m_aAppearances[ "N" ][ "Off" ] = pUncheckStream;
}

bool PDFWriterImpl::emitAppearances( PDFWidget& rWidget, OStringBuffer& rAnnotDict )
{

    // TODO: check and insert default streams
    rtl::OString aStandardAppearance;
    switch( rWidget.m_eType )
    {
        case PDFWriter::CheckBox:
            aStandardAppearance = OUStringToOString( rWidget.m_aValue, RTL_TEXTENCODING_ASCII_US );
            break;
        default:
            break;
    }

    if( rWidget.m_aAppearances.size() )
    {
        rAnnotDict.append( "/AP<<\n" );
        for( PDFAppearanceMap::iterator dict_it = rWidget.m_aAppearances.begin(); dict_it != rWidget.m_aAppearances.end(); ++dict_it )
        {
            rAnnotDict.append( "/" );
            rAnnotDict.append( dict_it->first );
            bool bUseSubDict = (dict_it->second.size() > 1);
            rAnnotDict.append( bUseSubDict ? "<<" : " " );

            for( PDFAppearanceStreams::const_iterator stream_it = dict_it->second.begin();
                 stream_it != dict_it->second.end(); ++stream_it )
            {
                SvMemoryStream* pApppearanceStream = stream_it->second;
                dict_it->second[ stream_it->first ] = NULL;

                bool bDeflate = compressStream( pApppearanceStream );

                pApppearanceStream->Seek( STREAM_SEEK_TO_END );
                sal_Int64 nStreamLen = pApppearanceStream->Tell();
                pApppearanceStream->Seek( STREAM_SEEK_TO_BEGIN );
                sal_Int32 nObject = createObject();
                CHECK_RETURN( updateObject( nObject ) );
#if OSL_DEBUG_LEVEL > 1
                {
                    OStringBuffer aLine( " PDFWriterImpl::emitAppearances" );
                    emitComment( aLine.getStr() );
                }
#endif
                OStringBuffer aLine;
                aLine.append( nObject );

                aLine.append( " 0 obj\n"
                              "<</Type/XObject\n"
                              "/Subtype/Form\n"
                              "/BBox[0 0 " );
                appendFixedInt( rWidget.m_aRect.GetWidth()-1, aLine );
                aLine.append( " " );
                appendFixedInt( rWidget.m_aRect.GetHeight()-1, aLine );
                aLine.append( "]\n"
                              "/Resources " );
                aLine.append( getResourceDictObj() );
                aLine.append( " 0 R\n"
                              "/Length " );
                aLine.append( nStreamLen );
                aLine.append( "\n" );
                if( bDeflate )
                    aLine.append( "/Filter/FlateDecode\n" );
                aLine.append( ">>\nstream\n" );
                CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );
                checkAndEnableStreamEncryption( nObject );
                CHECK_RETURN( writeBuffer( pApppearanceStream->GetData(), nStreamLen ) );
                disableStreamEncryption();
                CHECK_RETURN( writeBuffer( "\nendstream\nendobj\n\n", 19 ) );

                if( bUseSubDict )
                {
                    rAnnotDict.append( " /" );
                    rAnnotDict.append( stream_it->first );
                    rAnnotDict.append( " " );
                }
                rAnnotDict.append( nObject );
                rAnnotDict.append( " 0 R" );

                delete pApppearanceStream;
            }

            rAnnotDict.append( bUseSubDict ? ">>\n" : "\n" );
        }
        rAnnotDict.append( ">>\n" );
        if( aStandardAppearance.getLength() )
        {
            rAnnotDict.append( "/AS /" );
            rAnnotDict.append( aStandardAppearance );
            rAnnotDict.append( "\n" );
        }
    }

    return true;
}

bool PDFWriterImpl::emitWidgetAnnotations()
{
    ensureUniqueRadioOnValues();

    int nAnnots = m_aWidgets.size();
    for( int a = 0; a < nAnnots; a++ )
    {
        PDFWidget& rWidget = m_aWidgets[a];

        OStringBuffer aLine( 1024 );
        OStringBuffer aValue( 256 );
        aLine.append( rWidget.m_nObject );
        aLine.append( " 0 obj\n"
                      "<<" );
        // emit widget annotation only for terminal fields
        if( rWidget.m_aKids.empty() )
        {
            aLine.append( "/Type/Annot/Subtype/Widget/F 4\n"
                          "/Rect[" );
            appendFixedInt( rWidget.m_aRect.Left()-1, aLine );
            aLine.append( ' ' );
            appendFixedInt( rWidget.m_aRect.Top()+1, aLine );
            aLine.append( ' ' );
            appendFixedInt( rWidget.m_aRect.Right()+1, aLine );
            aLine.append( ' ' );
            appendFixedInt( rWidget.m_aRect.Bottom()-1, aLine );
            aLine.append( "]\n" );
        }
        aLine.append( "/FT/" );
        switch( rWidget.m_eType )
        {
            case PDFWriter::RadioButton:
            case PDFWriter::CheckBox:
                // for radio buttons only the RadioButton field, not the
                // CheckBox children should have a value, else acrobat reader
                // does not always check the right button
                // of course real check boxes (not belonging to a readio group)
                // need their values, too
                if( rWidget.m_eType == PDFWriter::RadioButton || rWidget.m_nRadioGroup < 0 )
                {
                    aValue.append( "/" );
                    // check for radio group with all buttons unpressed
                    if( rWidget.m_aValue.getLength() == 0 )
                        aValue.append( "Off" );
                    else
                        appendName( rWidget.m_aValue, aValue );
                }
            case PDFWriter::PushButton:
                aLine.append( "Btn" );
                break;
            case PDFWriter::ListBox:
                if( rWidget.m_nFlags & 0x200000 ) // multiselect
                {
                    aValue.append( "[" );
                    for( unsigned int i = 0; i < rWidget.m_aSelectedEntries.size(); i++ )
                    {
                        sal_Int32 nEntry = rWidget.m_aSelectedEntries[i];
                        if( nEntry >= 0 && nEntry < sal_Int32(rWidget.m_aListEntries.size()) )
                            appendUnicodeTextStringEncrypt( rWidget.m_aListEntries[ nEntry ], rWidget.m_nObject, aValue );
                    }
                    aValue.append( "]" );
                }
                else if( rWidget.m_aSelectedEntries.size() > 0 &&
                         rWidget.m_aSelectedEntries[0] >= 0 &&
                         rWidget.m_aSelectedEntries[0] < sal_Int32(rWidget.m_aListEntries.size()) )
                {
                    appendUnicodeTextStringEncrypt( rWidget.m_aListEntries[ rWidget.m_aSelectedEntries[0] ], rWidget.m_nObject, aValue );
                }
                else
                    appendUnicodeTextStringEncrypt( rtl::OUString(), rWidget.m_nObject, aValue );
                aLine.append( "Ch" );
                break;
            case PDFWriter::ComboBox:
                appendUnicodeTextStringEncrypt( rWidget.m_aValue, rWidget.m_nObject, aValue );
                aLine.append( "Ch" );
                break;
            case PDFWriter::Edit:
                aLine.append( "Tx" );
                appendUnicodeTextStringEncrypt( rWidget.m_aValue, rWidget.m_nObject, aValue );
                break;
        }
        aLine.append( "\n" );
        aLine.append( "/P " );
        aLine.append( m_aPages[ rWidget.m_nPage ].m_nPageObject );
        aLine.append( " 0 R\n" );

        if( rWidget.m_nParent )
        {
            aLine.append( "/Parent " );
            aLine.append( rWidget.m_nParent );
            aLine.append( " 0 R\n" );
        }
        if( rWidget.m_aKids.size() )
        {
            aLine.append( "/Kids[" );
            for( unsigned int i = 0; i < rWidget.m_aKids.size(); i++ )
            {
                aLine.append( rWidget.m_aKids[i] );
                aLine.append( " 0 R" );
                aLine.append( ( (i&15) == 15 ) ? "\n" : " " );
            }
            aLine.append( "]\n" );
        }
        if( rWidget.m_aName.getLength() )
        {
            aLine.append( "/T" );
            appendLiteralStringEncrypt( rWidget.m_aName, rWidget.m_nObject, aLine );
            aLine.append( "\n" );
        }
        if( m_aContext.Version > PDFWriter::PDF_1_2 )
        {
            // the alternate field name should be unicode able since it is
            // supposed to be used in UI
            aLine.append( "/TU" );
            appendUnicodeTextStringEncrypt( rWidget.m_aDescription, rWidget.m_nObject, aLine );
            aLine.append( "\n" );
        }

        if( rWidget.m_nFlags )
        {
            aLine.append( "/Ff " );
            aLine.append( rWidget.m_nFlags );
            aLine.append( "\n" );
        }
        if( aValue.getLength() )
        {
            OString aVal = aValue.makeStringAndClear();
            aLine.append( "/V " );
            aLine.append( aVal );
            aLine.append( "\n"
                          "/DV " );
            aLine.append( aVal );
            aLine.append( "\n" );
        }
        if( rWidget.m_eType == PDFWriter::ListBox || rWidget.m_eType == PDFWriter::ComboBox )
        {
            sal_Int32 nTI = -1;
            aLine.append( "/Opt[\n" );
            sal_Int32 i = 0;
            for( std::vector< OUString >::const_iterator it = rWidget.m_aListEntries.begin(); it != rWidget.m_aListEntries.end(); ++it, ++i )
            {
                appendUnicodeTextStringEncrypt( *it, rWidget.m_nObject, aLine );
                aLine.append( "\n" );
                if( *it == rWidget.m_aValue )
                    nTI = i;
            }
            aLine.append( "]\n" );
            if( nTI > 0 )
            {
                aLine.append( "/TI " );
                aLine.append( nTI );
                aLine.append( "\n" );
                if( rWidget.m_nFlags & 0x200000 ) // Multiselect
                {
                    aLine.append( "/I [" );
                    aLine.append( nTI );
                    aLine.append( "]\n" );
                }
            }
        }
        if( rWidget.m_eType == PDFWriter::Edit && rWidget.m_nMaxLen > 0 )
        {
            aLine.append( "/MaxLen " );
            aLine.append( rWidget.m_nMaxLen );
            aLine.append( "\n" );
        }
        if( rWidget.m_eType == PDFWriter::PushButton )
        {
            if(!m_bIsPDF_A1)
            {
                OStringBuffer aDest;
                if( appendDest( rWidget.m_nDest, aDest ) )
                {
                    aLine.append( "/AA<</D<</Type/Action/S/GoTo/D " );
                    aLine.append( aDest.makeStringAndClear() );
                    aLine.append( ">>>>\n" );
                }
                else if( rWidget.m_aListEntries.empty() )
                {
                    // create a reset form action
                    aLine.append( "/AA<</D<</Type/Action/S/ResetForm>>>>\n" );
                }
                else if( rWidget.m_bSubmit )
                {
                    // create a submit form action
                    aLine.append( "/AA<</D<</Type/Action/S/SubmitForm/F" );
                    appendLiteralStringEncrypt( rWidget.m_aListEntries.front(), rWidget.m_nObject, aLine );
                    aLine.append( "/Flags " );

                    sal_Int32 nFlags = 0;
                    switch( m_aContext.SubmitFormat )
                    {
                    case PDFWriter::HTML:
                        nFlags |= 4;
                        break;
                    case PDFWriter::XML:
                        if( m_aContext.Version > PDFWriter::PDF_1_3 )
                            nFlags |= 32;
                        break;
                    case PDFWriter::PDF:
                        if( m_aContext.Version > PDFWriter::PDF_1_3 )
                            nFlags |= 256;
                        break;
                    case PDFWriter::FDF:
                    default:
                        break;
                    }
                    if( rWidget.m_bSubmitGet )
                        nFlags |= 8;
                    aLine.append( nFlags );
                    aLine.append( ">>>>\n" );
                }
                else
                {
                    // create a URI action
                    aLine.append( "/AA<</D<</Type/Action/S/URI/URI(" );
                    aLine.append( OUStringToOString( rWidget.m_aListEntries.front(), RTL_TEXTENCODING_ASCII_US ) );
                    aLine.append( ")>>>>\n" );
                }
            }
            else
                m_aErrors.insert( PDFWriter::Warning_FormAction_Omitted_PDFA );
        }
        if( rWidget.m_aDAString.getLength() )
        {
            aLine.append( "/DR<</Font<<" );
            appendBuiltinFontsToDict( aLine );
            aLine.append( ">>>>\n" );
            aLine.append( "/DA" );
            appendLiteralStringEncrypt( rWidget.m_aDAString, rWidget.m_nObject, aLine );
            aLine.append( "\n" );
            if( rWidget.m_nTextStyle & TEXT_DRAW_CENTER )
                aLine.append( "/Q 1\n" );
            else if( rWidget.m_nTextStyle & TEXT_DRAW_RIGHT )
                aLine.append( "/Q 2\n" );
        }
        // appearance charactristics for terminal fields
        // which are supposed to have an appearance constructed
        // by the viewer application
        if( rWidget.m_aMKDict.getLength() )
        {
            aLine.append( "/MK<<" );
            aLine.append( rWidget.m_aMKDict );
//add the CA string, encrypting it
            appendLiteralStringEncrypt(rWidget.m_aMKDictCAString, rWidget.m_nObject, aLine);
            aLine.append( ">>\n" );
        }

        CHECK_RETURN( emitAppearances( rWidget, aLine ) );

        aLine.append( ">>\n"
                      "endobj\n\n" );
        CHECK_RETURN( updateObject( rWidget.m_nObject ) );
        CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );
    }
    return true;
}

bool PDFWriterImpl::emitAnnotations()
{
    if( m_aPages.size() < 1 )
        return false;

    CHECK_RETURN( emitLinkAnnotations() );

    CHECK_RETURN( emitNoteAnnotations() );

    CHECK_RETURN( emitWidgetAnnotations() );

    return true;
}

#undef CHECK_RETURN
#define CHECK_RETURN( x ) if( !x ) return false

bool PDFWriterImpl::emitCatalog()
{
    // build page tree
    // currently there is only one node that contains all leaves

    // first create a page tree node id
    sal_Int32 nTreeNode = createObject();

    // emit global resource dictionary (page emit needs it)
    CHECK_RETURN( emitResources() );

    // emit all pages
    for( std::vector<PDFPage>::iterator it = m_aPages.begin(); it != m_aPages.end(); ++it )
        if( ! it->emit( nTreeNode ) )
            return false;

    sal_Int32 nNamedDestinationsDictionary = emitNamedDestinations();

    sal_Int32 nOutlineDict = emitOutline();

    //emit Output intent i59651
    sal_Int32 nOutputIntentObject = emitOutputIntent();

    //emit metadata
    sal_Int32 nMetadataObject = emitDocumentMetadata();

    sal_Int32 nStructureDict = 0;
    if(m_aStructure.size() > 1)
    {
///check if dummy structure containers are needed
        addInternalStructureContainer(m_aStructure[0]);
        nStructureDict = m_aStructure[0].m_nObject = createObject();
        emitStructure( m_aStructure[ 0 ] );
    }

    // adjust tree node file offset
    if( ! updateObject( nTreeNode ) )
        return false;

    // emit tree node
    OStringBuffer aLine( 2048 );
    aLine.append( nTreeNode );
    aLine.append( " 0 obj\n" );
    aLine.append( "<</Type/Pages\n" );
    aLine.append( "/Resources " );
    aLine.append( getResourceDictObj() );
    aLine.append( " 0 R\n" );

    switch( m_eInheritedOrientation )
    {
        case PDFWriter::Landscape: aLine.append( "/Rotate 90\n" );break;
        case PDFWriter::Seascape: aLine.append( "/Rotate -90\n" );break;

        case PDFWriter::Inherit: // actually Inherit would be a bug, but insignificant
        case PDFWriter::Portrait:
        default:
            break;
    }
    sal_Int32 nMediaBoxWidth = 0;
    sal_Int32 nMediaBoxHeight = 0;
    if( m_aPages.empty() ) // sanity check, this should not happen
    {
        nMediaBoxWidth = m_nInheritedPageWidth;
        nMediaBoxHeight = m_nInheritedPageHeight;
    }
    else
    {
        for( std::vector<PDFPage>::const_iterator iter = m_aPages.begin(); iter != m_aPages.end(); ++iter )
        {
            if( iter->m_nPageWidth > nMediaBoxWidth )
                nMediaBoxWidth = iter->m_nPageWidth;
            if( iter->m_nPageHeight > nMediaBoxHeight )
                nMediaBoxHeight = iter->m_nPageHeight;
        }
    }
    aLine.append( "/MediaBox[ 0 0 " );
    aLine.append( nMediaBoxWidth );
    aLine.append( ' ' );
    aLine.append( nMediaBoxHeight );
    aLine.append( " ]\n"
                  "/Kids[ " );
    unsigned int i = 0;
    for( std::vector<PDFPage>::const_iterator iter = m_aPages.begin(); iter != m_aPages.end(); ++iter, i++ )
    {
        aLine.append( iter->m_nPageObject );
        aLine.append( " 0 R" );
        aLine.append( ( (i&15) == 15 ) ? "\n" : " " );
    }
    aLine.append( "]\n"
                  "/Count " );
    aLine.append( (sal_Int32)m_aPages.size() );
    aLine.append( ">>\n"
                  "endobj\n\n" );
    CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );

    // emit annotation objects
    CHECK_RETURN( emitAnnotations() );

    // emit Catalog
    m_nCatalogObject = createObject();
    if( ! updateObject( m_nCatalogObject ) )
        return false;
    aLine.setLength( 0 );
    aLine.append( m_nCatalogObject );
    aLine.append( " 0 obj\n"
                  "<</Type/Catalog/Pages " );
    aLine.append( nTreeNode );
    aLine.append( " 0 R\n" );
//--->i56629
//check if there are named destinations to emit (root must be inside the catalog)
    if( nNamedDestinationsDictionary )
    {
        aLine.append("/Dests ");
        aLine.append( nNamedDestinationsDictionary );
        aLine.append( " 0 R\n" );
    }
//<----
    if( m_aContext.PageLayout != PDFWriter::DefaultLayout )
        switch(  m_aContext.PageLayout )
        {
        default :
        case  PDFWriter::SinglePage :
            aLine.append( "/PageLayout/SinglePage\n" );
            break;
        case  PDFWriter::Continuous :
            aLine.append( "/PageLayout/OneColumn\n" );
            break;
        case  PDFWriter::ContinuousFacing :
//the flag m_aContext.FirstPageLeft below is used to set the page on the left side
            aLine.append( "/PageLayout/TwoColumnRight\n" );//odd page on the right side
            break;
        }
    if( m_aContext.PDFDocumentMode != PDFWriter::ModeDefault && !m_aContext.OpenInFullScreenMode )
        switch(  m_aContext.PDFDocumentMode )
        {
        default :
            aLine.append( "/PageMode/UseNone\n" );
            break;
        case PDFWriter::UseOutlines :
            aLine.append( "/PageMode/UseOutlines\n" ); //document is opened with outline pane open
            break;
        case PDFWriter::UseThumbs :
            aLine.append( "/PageMode/UseThumbs\n" ); //document is opened with thumbnails pane open
            break;
        }
    else if( m_aContext.OpenInFullScreenMode )
        aLine.append( "/PageMode/FullScreen\n" ); //document is opened full screen

    OStringBuffer aInitPageRef;
    if( m_aContext.InitialPage >= 0 && m_aContext.InitialPage < (sal_Int32)m_aPages.size() )
    {
        aInitPageRef.append( m_aPages[m_aContext.InitialPage].m_nPageObject );
        aInitPageRef.append( " 0 R" );
    }
    else
        aInitPageRef.append( "0" );
    switch( m_aContext.PDFDocumentAction )
    {
    case PDFWriter::ActionDefault :     //do nothing, this is the Acrobat default
    default:
        if( aInitPageRef.getLength() > 1 )
        {
            aLine.append( "/OpenAction[" );
            aLine.append( aInitPageRef );
            aLine.append( " /XYZ null null 0]\n" );
        }
        break;
    case PDFWriter::FitInWindow :
        aLine.append( "/OpenAction[" );
        aLine.append( aInitPageRef );
        aLine.append( " /Fit]\n" ); //Open fit page
        break;
    case PDFWriter::FitWidth :
        aLine.append( "/OpenAction[" );
        aLine.append( aInitPageRef );
        aLine.append( " /FitH " );
        aLine.append( m_nInheritedPageHeight );//Open fit width
        aLine.append( "]\n" );
        break;
    case PDFWriter::FitVisible :
        aLine.append( "/OpenAction[" );
        aLine.append( aInitPageRef );
        aLine.append( " /FitBH " );
        aLine.append( m_nInheritedPageHeight );//Open fit visible
        aLine.append( "]\n" );
        break;
    case PDFWriter::ActionZoom :
        aLine.append( "/OpenAction[" );
        aLine.append( aInitPageRef );
        aLine.append( " /XYZ null null " );
        if( m_aContext.Zoom >= 50 && m_aContext.Zoom <= 1600 )
            aLine.append( (double)m_aContext.Zoom/100.0 );
        else
            aLine.append( "0" );
        aLine.append( "]\n" );
        break;
    }
// viewer preferences, if we had some, then emit
    if( m_aContext.HideViewerToolbar ||
        ( m_aContext.Version > PDFWriter::PDF_1_3 && m_aDocInfo.Title.Len() && m_aContext.DisplayPDFDocumentTitle ) ||
        m_aContext.HideViewerMenubar ||
        m_aContext.HideViewerWindowControls || m_aContext.FitWindow ||
        m_aContext.CenterWindow || (m_aContext.FirstPageLeft  &&  m_aContext.PageLayout == PDFWriter::ContinuousFacing ) ||
        m_aContext.OpenInFullScreenMode )
    {
        aLine.append( "/ViewerPreferences<<" );
        if( m_aContext.HideViewerToolbar )
            aLine.append( "/HideToolbar true\n" );
        if( m_aContext.HideViewerMenubar )
            aLine.append( "/HideMenubar true\n" );
        if( m_aContext.HideViewerWindowControls )
            aLine.append( "/HideWindowUI true\n" );
        if( m_aContext.FitWindow )
            aLine.append( "/FitWindow true\n" );
        if( m_aContext.CenterWindow )
            aLine.append( "/CenterWindow true\n" );
        if( m_aContext.Version > PDFWriter::PDF_1_3 && m_aDocInfo.Title.Len() && m_aContext.DisplayPDFDocumentTitle )
            aLine.append( "/DisplayDocTitle true\n" );
        if( m_aContext.FirstPageLeft &&  m_aContext.PageLayout == PDFWriter::ContinuousFacing )
            aLine.append( "/Direction/R2L\n" );
        if( m_aContext.OpenInFullScreenMode )
            switch( m_aContext.PDFDocumentMode )
            {
            default :
            case PDFWriter::ModeDefault :
                aLine.append( "/NonFullScreenPageMode/UseNone\n" );
                break;
            case PDFWriter::UseOutlines :
                aLine.append( "/NonFullScreenPageMode/UseOutlines\n" );
                break;
            case PDFWriter::UseThumbs :
                aLine.append( "/NonFullScreenPageMode/UseThumbs\n" );
                break;
            }
        aLine.append( ">>\n" );
    }

    if( nOutlineDict )
    {
        aLine.append( "/Outlines " );
        aLine.append( nOutlineDict );
        aLine.append( " 0 R\n" );
    }
    if( nStructureDict )
    {
        aLine.append( "/StructTreeRoot " );
        aLine.append( nStructureDict );
        aLine.append( " 0 R\n" );
    }
    if( m_aContext.DocumentLocale.Language.getLength() > 0 )
    {
        OUStringBuffer aLocBuf( 16 );
        aLocBuf.append( m_aContext.DocumentLocale.Language.toAsciiLowerCase() );
        if( m_aContext.DocumentLocale.Country.getLength() > 0 )
        {
            aLocBuf.append( sal_Unicode('-') );
            aLocBuf.append( m_aContext.DocumentLocale.Country );
        }
        aLine.append( "/Lang" );
        appendLiteralStringEncrypt( aLocBuf.makeStringAndClear(), m_nCatalogObject, aLine );
        aLine.append( "\n" );
    }
    if( m_aContext.Tagged && m_aContext.Version > PDFWriter::PDF_1_3 )
    {
        aLine.append( "/MarkInfo<</Marked true>>\n" );
    }
    if( m_aWidgets.size() > 0 )
    {
        aLine.append( "/AcroForm<</Fields[\n" );
        int nWidgets = m_aWidgets.size();
        int nOut = 0;
        for( int j = 0; j < nWidgets; j++ )
        {
            // output only root fields
            if( m_aWidgets[j].m_nParent < 1 )
            {
                aLine.append( m_aWidgets[j].m_nObject );
                aLine.append( (nOut++ % 5)==4 ? " 0 R\n" : " 0 R " );
            }
        }
        aLine.append( "\n]/DR " );
        aLine.append( getResourceDictObj() );
        aLine.append( " 0 R" );
        if( m_bIsPDF_A1 )
            aLine.append( ">>\n" );
        else
            aLine.append( "/NeedAppearances true>>\n" );
    }
//--->i59651
//check if there is a Metadata object
    if( nOutputIntentObject )
    {
        aLine.append("/OutputIntents[");
        aLine.append( nOutputIntentObject );
        aLine.append( " 0 R]" );
    }
    if( nMetadataObject )
    {
        aLine.append("/Metadata ");
        aLine.append( nMetadataObject );
        aLine.append( " 0 R" );
    }
//<----
    aLine.append( ">>\n"
                  "endobj\n\n" );
    CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );

    return true;
}

sal_Int32 PDFWriterImpl::emitInfoDict( )
{
    sal_Int32 nObject = createObject();

    if( updateObject( nObject ) )
    {
        OStringBuffer aLine( 1024 );
        aLine.append( nObject );
        aLine.append( " 0 obj\n"
                      "<<" );
        if( m_aDocInfo.Title.Len() )
        {
            aLine.append( "/Title" );
            appendUnicodeTextStringEncrypt( m_aDocInfo.Title, nObject, aLine );
            aLine.append( "\n" );
        }
        if( m_aDocInfo.Author.Len() )
        {
            aLine.append( "/Author" );
            appendUnicodeTextStringEncrypt( m_aDocInfo.Author, nObject, aLine );
            aLine.append( "\n" );
        }
        if( m_aDocInfo.Subject.Len() )
        {
            aLine.append( "/Subject" );
            appendUnicodeTextStringEncrypt( m_aDocInfo.Subject, nObject, aLine );
            aLine.append( "\n" );
        }
        if( m_aDocInfo.Keywords.Len() )
        {
            aLine.append( "/Keywords" );
            appendUnicodeTextStringEncrypt( m_aDocInfo.Keywords, nObject, aLine );
            aLine.append( "\n" );
        }
        if( m_aDocInfo.Creator.Len() )
        {
            aLine.append( "/Creator" );
            appendUnicodeTextStringEncrypt( m_aDocInfo.Creator, nObject, aLine );
            aLine.append( "\n" );
        }
        if( m_aDocInfo.Producer.Len() )
        {
            aLine.append( "/Producer" );
            appendUnicodeTextStringEncrypt( m_aDocInfo.Producer, nObject, aLine );
            aLine.append( "\n" );
        }

         aLine.append( "/CreationDate" );
         appendLiteralStringEncrypt( m_aCreationDateString, nObject, aLine );
        aLine.append( ">>\nendobj\n\n" );
        if( ! writeBuffer( aLine.getStr(), aLine.getLength() ) )
            nObject = 0;
    }
    else
        nObject = 0;

    return nObject;
}

//--->i56629
// Part of this function may be shared with method appendDest.
//
sal_Int32 PDFWriterImpl::emitNamedDestinations()
{
    sal_Int32  nCount = m_aNamedDests.size();
    if( nCount <= 0 )
        return 0;//define internal error

//get the object number for all the destinations
    sal_Int32 nObject = createObject();

    if( updateObject( nObject ) )
    {
//emit the dictionary
        OStringBuffer aLine( 1024 );
        aLine.append( nObject );
        aLine.append( " 0 obj\n"
                      "<<" );

        sal_Int32  nDestID;
        for( nDestID = 0; nDestID < nCount; nDestID++ )
        {
            const PDFNamedDest& rDest   = m_aNamedDests[ nDestID ];
// In order to correctly function both under an Internet browser and
// directly with a reader (provided the reader has the feature) we
// need to set the name of the destination the same way it will be encoded
// in an Internet link
            INetURLObject aLocalURL(
                OUString( RTL_CONSTASCII_USTRINGPARAM( "http://ahost.ax" ) ) ); //dummy location, won't be used
            aLocalURL.SetMark( rDest.m_aDestName );

            const rtl::OUString aName   = aLocalURL.GetMark( INetURLObject::NO_DECODE ); //same coding as
            // in link creation ( see PDFWriterImpl::emitLinkAnnotations )
            const PDFPage& rDestPage    = m_aPages[ rDest.m_nPage ];

            aLine.append( '/' );
            appendDestinationName( aName, aLine ); // this conversion must be done when forming the link to target ( see in emitCatalog )
            aLine.append( '[' ); // the '[' can be emitted immediately, because the appendDestinationName function
                                 //maps the preceeding character properly
            aLine.append( rDestPage.m_nPageObject );
            aLine.append( " 0 R" );

            switch( rDest.m_eType )
            {
            case PDFWriter::XYZ:
            default:
                aLine.append( "/XYZ " );
                appendFixedInt( rDest.m_aRect.Left(), aLine );
                aLine.append( ' ' );
                appendFixedInt( rDest.m_aRect.Bottom(), aLine );
                aLine.append( " 0" );
                break;
            case PDFWriter::Fit:
                aLine.append( "/Fit" );
                break;
            case PDFWriter::FitRectangle:
                aLine.append( "/FitR " );
                appendFixedInt( rDest.m_aRect.Left(), aLine );
                aLine.append( ' ' );
                appendFixedInt( rDest.m_aRect.Top(), aLine );
                aLine.append( ' ' );
                appendFixedInt( rDest.m_aRect.Right(), aLine );
                aLine.append( ' ' );
                appendFixedInt( rDest.m_aRect.Bottom(), aLine );
                break;
            case PDFWriter::FitHorizontal:
                aLine.append( "/FitH " );
                appendFixedInt( rDest.m_aRect.Bottom(), aLine );
                break;
            case PDFWriter::FitVertical:
                aLine.append( "/FitV " );
                appendFixedInt( rDest.m_aRect.Left(), aLine );
                break;
            case PDFWriter::FitPageBoundingBox:
                aLine.append( "/FitB" );
                break;
            case PDFWriter::FitPageBoundingBoxHorizontal:
                aLine.append( "/FitBH " );
                appendFixedInt( rDest.m_aRect.Bottom(), aLine );
                break;
            case PDFWriter::FitPageBoundingBoxVertical:
                aLine.append( "/FitBV " );
                appendFixedInt( rDest.m_aRect.Left(), aLine );
                break;
            }
            aLine.append( "]\n" );
        }
//close

        aLine.append( ">>\nendobj\n\n" );
        if( ! writeBuffer( aLine.getStr(), aLine.getLength() ) )
            nObject = 0;
    }
    else
        nObject = 0;

    return nObject;
}
//<--- i56629

//--->i59651
// emits the output intent dictionary

sal_Int32 PDFWriterImpl::emitOutputIntent()
{
    if( !m_bIsPDF_A1 )
        return 0;

//emit the sRGB standard profile, in ICC format, in a stream, per IEC61966-2.1

    OStringBuffer aLine( 1024 );
    sal_Int32 nICCObject = createObject();
    sal_Int32 nStreamLengthObject = createObject();

    aLine.append( nICCObject );
// sRGB has 3 colors, hence /N 3 below (PDF 1.4 table 4.16)
    aLine.append( " 0 obj\n<</N 3/Length " );
    aLine.append( nStreamLengthObject );
    aLine.append( " 0 R" );
#ifndef DEBUG_DISABLE_PDFCOMPRESSION
    aLine.append( "/Filter/FlateDecode" );
#endif
    aLine.append( ">>\nstream\n" );
    CHECK_RETURN( updateObject( nICCObject ) );
    CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );
//get file position
    sal_uInt64 nBeginStreamPos = 0;
    osl_getFilePos( m_aFile, &nBeginStreamPos );
    beginCompression();
    checkAndEnableStreamEncryption( nICCObject );
    sal_Int32 nStreamSize = writeBuffer( nsRGB_ICC_profile, (sal_Int32) sizeof( nsRGB_ICC_profile ) );
    disableStreamEncryption();
    endCompression();
    sal_uInt64 nEndStreamPos = 0;
    osl_getFilePos( m_aFile, &nEndStreamPos );

    if( nStreamSize == 0 )
        return 0;
    if( ! writeBuffer( "\nendstream\nendobj\n\n", 19 ) )
        return 0 ;
    aLine.setLength( 0 );

//emit the stream length   object
    CHECK_RETURN( updateObject( nStreamLengthObject ) );
    aLine.setLength( 0 );
    aLine.append( nStreamLengthObject );
    aLine.append( " 0 obj\n" );
    aLine.append( (sal_Int64)(nEndStreamPos-nBeginStreamPos) );
    aLine.append( "\nendobj\n\n" );
    CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );
    aLine.setLength( 0 );

//emit the OutputIntent dictionary
    sal_Int32 nOIObject = createObject();
    CHECK_RETURN( updateObject( nOIObject ) );
    aLine.append( nOIObject );
    aLine.append( " 0 obj\n"
                  "<</Type/OutputIntent/S/GTS_PDFA1/OutputConditionIdentifier");

    rtl::OUString aComment( RTL_CONSTASCII_USTRINGPARAM( "sRGB IEC61966-2.1" ) );
    appendLiteralStringEncrypt( aComment ,nOIObject, aLine );
    aLine.append("/DestOutputProfile ");
    aLine.append( nICCObject );
    aLine.append( " 0 R>>\nendobj\n\n" );;
    CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );

    return nOIObject;
}

// formats the string for the XML stream
static void escapeStringXML( const rtl::OUString& rStr, rtl::OUString &rValue)
{
    const sal_Unicode* pUni = rStr.getStr();
    int nLen = rStr.getLength();
    for( ; nLen; nLen--, pUni++ )
    {
        switch( *pUni )
        {
        case sal_Unicode('&'):
            rValue += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "&amp;" ) );
        break;
        case sal_Unicode('<'):
            rValue += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "&lt;" ) );
        break;
        case sal_Unicode('>'):
            rValue += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "&gt;" ) );
        break;
        case sal_Unicode('\''):
            rValue += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "&apos;" ) );
        break;
        case sal_Unicode('"'):
            rValue += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "&quot;" ) );
        break;
        default:
            rValue += rtl::OUString( *pUni );
            break;
        }
    }
}

// emits the document metadata
//
sal_Int32 PDFWriterImpl::emitDocumentMetadata()
{
    if( !m_bIsPDF_A1 )
        return 0;

    //get the object number for all the destinations
    sal_Int32 nObject = createObject();

    if( updateObject( nObject ) )
    {
// the following string are written in UTF-8 unicode
        OStringBuffer aMetadataStream( 8192 );

        aMetadataStream.append( "<?xpacket begin=\"" );
// this lines writes Unicode “zero width non-breaking space character” (U+FEFF) (aka byte-order mark ) used
// as a byte-order marker.
        aMetadataStream.append( OUStringToOString( OUString( sal_Unicode( 0xFEFF ) ), RTL_TEXTENCODING_UTF8 ) );
        aMetadataStream.append( "\" id=\"W5M0MpCehiHzreSzNTczkc9d\"?>\n" );
        aMetadataStream.append( "<x:xmpmeta xmlns:x=\"adobe:ns:meta/\">\n" );
        aMetadataStream.append( " <rdf:RDF xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\">\n" );
//PDF/A part ( ISO 19005-1:2005 - 6.7.11 )
        aMetadataStream.append( "  <rdf:Description rdf:about=\"\"\n" );
        aMetadataStream.append( "      xmlns:pdfaid=\"http://www.aiim.org/pdfa/ns/id/\">\n" );
        aMetadataStream.append( "   <pdfaid:part>1</pdfaid:part>\n" );
        aMetadataStream.append( "   <pdfaid:conformance>A</pdfaid:conformance>\n" );
        aMetadataStream.append( "  </rdf:Description>\n" );
//... Dublin Core properties go here
        if( m_aDocInfo.Title.Len() ||
            m_aDocInfo.Author.Len() ||
            m_aDocInfo.Subject.Len() )
        {
            aMetadataStream.append( "  <rdf:Description rdf:about=\"\"\n" );
            aMetadataStream.append( "      xmlns:dc=\"http://purl.org/dc/elements/1.1/\">\n" );
            if( m_aDocInfo.Title.Len() )
            {
// this is according to PDF/A-1, technical corrigendum 1 (2007-04-01)
                aMetadataStream.append( "   <dc:title>\n" );
                aMetadataStream.append( "    <rdf:Alt>\n" );
                aMetadataStream.append( "     <rdf:li xml:lang=\"x-default\">" );
                rtl::OUString aTitle;
                escapeStringXML( m_aDocInfo.Title, aTitle );
                aMetadataStream.append( OUStringToOString( aTitle, RTL_TEXTENCODING_UTF8 )  );
                aMetadataStream.append( "</rdf:li>\n" );
                aMetadataStream.append( "    </rdf:Alt>\n" );
                aMetadataStream.append( "   </dc:title>\n" );
            }
            if( m_aDocInfo.Author.Len() )
            {
                aMetadataStream.append( "   <dc:creator>\n" );
                aMetadataStream.append( "    <rdf:Seq>\n" );
                aMetadataStream.append( "     <rdf:li>" );
                rtl::OUString aAuthor;
                escapeStringXML( m_aDocInfo.Author, aAuthor );
                aMetadataStream.append( OUStringToOString( aAuthor , RTL_TEXTENCODING_UTF8 )  );
                aMetadataStream.append( "</rdf:li>\n" );
                aMetadataStream.append( "    </rdf:Seq>\n" );
                aMetadataStream.append( "   </dc:creator>\n" );
            }
            if( m_aDocInfo.Subject.Len() )
            {
// this is according to PDF/A-1, technical corrigendum 1 (2007-04-01)
                aMetadataStream.append( "   <dc:description>\n" );
                aMetadataStream.append( "    <rdf:Alt>\n" );
                aMetadataStream.append( "     <rdf:li xml:lang=\"x-default\">" );
                rtl::OUString aSubject;
                escapeStringXML( m_aDocInfo.Subject, aSubject );
                aMetadataStream.append( OUStringToOString( aSubject , RTL_TEXTENCODING_UTF8 )  );
                aMetadataStream.append( "</rdf:li>\n" );
                aMetadataStream.append( "    </rdf:Alt>\n" );
                aMetadataStream.append( "   </dc:description>\n" );
            }
            aMetadataStream.append( "  </rdf:Description>\n" );
        }

//... PDF properties go here
        if( m_aDocInfo.Producer.Len() ||
            m_aDocInfo.Keywords.Len() )
        {
            aMetadataStream.append( "  <rdf:Description rdf:about=\"\"\n" );
            aMetadataStream.append( "     xmlns:pdf=\"http://ns.adobe.com/pdf/1.3/\">\n" );
            if( m_aDocInfo.Producer.Len() )
            {
                aMetadataStream.append( "   <pdf:Producer>" );
                rtl::OUString aProducer;
                escapeStringXML( m_aDocInfo.Producer, aProducer );
                aMetadataStream.append( OUStringToOString( aProducer , RTL_TEXTENCODING_UTF8 )  );
                aMetadataStream.append( "</pdf:Producer>\n" );
            }
            if( m_aDocInfo.Keywords.Len() )
            {
                aMetadataStream.append( "   <pdf:Keywords>" );
                rtl::OUString aKeywords;
                escapeStringXML( m_aDocInfo.Keywords, aKeywords );
                aMetadataStream.append( OUStringToOString( aKeywords , RTL_TEXTENCODING_UTF8 )  );
                aMetadataStream.append( "</pdf:Keywords>\n" );
            }
            aMetadataStream.append( "  </rdf:Description>\n" );
        }

        aMetadataStream.append( "  <rdf:Description rdf:about=\"\"\n" );
        aMetadataStream.append( "    xmlns:xmp=\"http://ns.adobe.com/xap/1.0/\">\n" );
        if( m_aDocInfo.Creator.Len() )
        {
            aMetadataStream.append( "   <xmp:CreatorTool>" );
            rtl::OUString aCreator;
            escapeStringXML( m_aDocInfo.Creator, aCreator );
            aMetadataStream.append( OUStringToOString( aCreator , RTL_TEXTENCODING_UTF8 )  );
            aMetadataStream.append( "</xmp:CreatorTool>\n" );
        }
//creation date
        aMetadataStream.append( "   <xmp:CreateDate>" );
        aMetadataStream.append( m_aCreationMetaDateString );
        aMetadataStream.append( "</xmp:CreateDate>\n" );

        aMetadataStream.append( "  </rdf:Description>\n" );
        aMetadataStream.append( " </rdf:RDF>\n" );
        aMetadataStream.append( "</x:xmpmeta>\n" );

//add the padding
        for( sal_Int32 nSpaces = 1; nSpaces <= 2100; nSpaces++ )
        {
            aMetadataStream.append( " " );
            if( nSpaces % 100 == 0 )
                aMetadataStream.append( "\n" );
        }

        aMetadataStream.append( "<?xpacket end=\"w\"?>\n" );

        OStringBuffer aMetadataObj( 1024 );

        aMetadataObj.append( nObject );
        aMetadataObj.append( " 0 obj\n" );

        aMetadataObj.append( "<</Type/Metadata/Subtype/XML/Length " );

        aMetadataObj.append( (sal_Int32) aMetadataStream.getLength() );
        aMetadataObj.append( ">>\nstream\n" );
        CHECK_RETURN( writeBuffer( aMetadataObj.getStr(), aMetadataObj.getLength() ) );
//emit the stream
        CHECK_RETURN( writeBuffer( aMetadataStream.getStr(), aMetadataStream.getLength() ) );

        aMetadataObj.setLength( 0 );
        aMetadataObj.append( "\nendstream\nendobj\n\n" );
        if( ! writeBuffer( aMetadataObj.getStr(), aMetadataObj.getLength() ) )
            nObject = 0;
    }
    else
        nObject = 0;

    return nObject;
}
//<---i59651

bool PDFWriterImpl::emitTrailer()
{
    // emit doc info
    OString aInfoValuesOut;
    sal_Int32 nDocInfoObject = emitInfoDict( );

    sal_Int32 nSecObject = 0;

	if( m_aContext.Encrypt == true )
	{
//emit the security information
//must be emitted as indirect dictionary object, since
//Acrobat Reader 5 works only with this kind of implementation
		nSecObject = createObject();

		if( updateObject( nSecObject ) )
		{
			OStringBuffer aLineS( 1024 );
			aLineS.append( nSecObject );
			aLineS.append( " 0 obj\n"
						   "<</Filter/Standard/V " );
			// check the version
			if( m_aContext.Security128bit == true )
				aLineS.append( "2/Length 128/R 3" );
			else
				aLineS.append( "1/R 2" );

			// emit the owner password, must not be encrypted
			aLineS.append( "/O(" );
			appendLiteralString( (const sal_Char*)m_nEncryptedOwnerPassword, 32, aLineS );
			aLineS.append( ")/U(" );
			appendLiteralString( (const sal_Char*)m_nEncryptedUserPassword, 32, aLineS );
			aLineS.append( ")/P " );// the permission set
			aLineS.append( m_nAccessPermissions );
			aLineS.append( ">>\nendobj\n\n" );
			if( !writeBuffer( aLineS.getStr(), aLineS.getLength() ) )
				nSecObject = 0;
		}
		else
			nSecObject = 0;
	}
    // emit xref table
    // remember start
    sal_uInt64 nXRefOffset = 0;
    CHECK_RETURN( (osl_File_E_None == osl_getFilePos( m_aFile, &nXRefOffset )) );
    CHECK_RETURN( writeBuffer( "xref\n", 5 ) );

    sal_Int32 nObjects = m_aObjects.size();
    OStringBuffer aLine;
    aLine.append( "0 " );
    aLine.append( (sal_Int32)(nObjects+1) );
    aLine.append( "\n" );
    aLine.append( "0000000000 65535 f \n" );
    CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );

    for( sal_Int32 i = 0; i < nObjects; i++ )
    {
        aLine.setLength( 0 );
        OString aOffset = OString::valueOf( (sal_Int64)m_aObjects[i] );
        for( sal_Int32 j = 0; j < (10-aOffset.getLength()); j++ )
            aLine.append( '0' );
        aLine.append( aOffset );
        aLine.append( " 00000 n \n" );
        DBG_ASSERT( aLine.getLength() == 20, "invalid xref entry" );
        CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );
    }

    // prepare document checksum
    OStringBuffer aDocChecksum( 2*RTL_DIGEST_LENGTH_MD5+1 );
    if( m_aDocDigest )
    {
        sal_uInt8 nMD5Sum[ RTL_DIGEST_LENGTH_MD5 ];
        rtl_digest_getMD5( m_aDocDigest, nMD5Sum, sizeof(nMD5Sum) );
        for( unsigned int i = 0; i < RTL_DIGEST_LENGTH_MD5; i++ )
            appendHex( nMD5Sum[i], aDocChecksum );
    }
    // document id set in setDocInfo method
    // emit trailer
    aLine.setLength( 0 );
    aLine.append( "trailer\n"
                  "<</Size " );
    aLine.append( (sal_Int32)(nObjects+1) );
    aLine.append( "/Root " );
    aLine.append( m_nCatalogObject );
    aLine.append( " 0 R\n" );
    if( nSecObject |= 0 )
    {
        aLine.append( "/Encrypt ");
        aLine.append( nSecObject );
        aLine.append( " 0 R\n" );
    }
    if( nDocInfoObject )
    {
        aLine.append( "/Info " );
        aLine.append( nDocInfoObject );
        aLine.append( " 0 R\n" );
    }
    if( m_aDocID.getLength() )
    {
        aLine.append( "/ID [ <" );
        aLine.append( m_aDocID.getStr(), m_aDocID.getLength() );
        aLine.append( ">\n"
                      "<" );
        aLine.append( m_aDocID.getStr(), m_aDocID.getLength() );
        aLine.append( "> ]\n" );
    }
    if( aDocChecksum.getLength() )
    {
        aLine.append( "/DocChecksum /" );
        aLine.append( aDocChecksum );
        aLine.append( "\n" );
    }
    if( m_aAdditionalStreams.size() > 0 )
    {
        aLine.append( "/AdditionalStreams [" );
        for( unsigned int i = 0; i < m_aAdditionalStreams.size(); i++ )
        {
            aLine.append( "/" );
            appendName( m_aAdditionalStreams[i].m_aMimeType, aLine );
            aLine.append( " " );
            aLine.append( m_aAdditionalStreams[i].m_nStreamObject );
            aLine.append( " 0 R\n" );
        }
        aLine.append( "]\n" );
    }
    aLine.append( ">>\n"
                  "startxref\n" );
    aLine.append( (sal_Int64)nXRefOffset );
    aLine.append( "\n"
                  "%%EOF\n" );
    CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );

    return true;
}

struct AnnotationSortEntry
{
    sal_Int32 nTabOrder;
    sal_Int32 nObject;
    sal_Int32 nWidgetIndex;

    AnnotationSortEntry( sal_Int32 nTab, sal_Int32 nObj, sal_Int32 nI ) :
        nTabOrder( nTab ),
        nObject( nObj ),
        nWidgetIndex( nI )
    {}
};

struct AnnotSortContainer
{
    std::set< sal_Int32 >               aObjects;
    std::vector< AnnotationSortEntry >    aSortedAnnots;
};

struct AnnotSorterLess
{
    std::vector< PDFWriterImpl::PDFWidget >& m_rWidgets;

    AnnotSorterLess( std::vector< PDFWriterImpl::PDFWidget >& rWidgets ) : m_rWidgets( rWidgets ) {}

    bool operator()( const AnnotationSortEntry& rLeft, const AnnotationSortEntry& rRight )
    {
        if( rLeft.nTabOrder < rRight.nTabOrder )
            return true;
        if( rRight.nTabOrder < rLeft.nTabOrder )
            return false;
        if( rLeft.nWidgetIndex < 0 && rRight.nWidgetIndex < 0 )
            return false;
        if( rRight.nWidgetIndex < 0 )
            return true;
        if( rLeft.nWidgetIndex < 0 )
            return false;
        // remember: widget rects are in PDF coordinates, so they are ordered down up
        if( m_rWidgets[ rLeft.nWidgetIndex ].m_aRect.Top() >
            m_rWidgets[ rRight.nWidgetIndex ].m_aRect.Top() )
            return true;
        if( m_rWidgets[ rRight.nWidgetIndex ].m_aRect.Top() >
            m_rWidgets[ rLeft.nWidgetIndex ].m_aRect.Top() )
            return false;
        if( m_rWidgets[ rLeft.nWidgetIndex ].m_aRect.Left() <
            m_rWidgets[ rRight.nWidgetIndex ].m_aRect.Left() )
            return true;
        return false;
    }
};

void PDFWriterImpl::sortWidgets()
{
    // sort widget annotations on each page as per their
    // TabOrder attribute
    std::hash_map< sal_Int32, AnnotSortContainer > sorted;
    int nWidgets = m_aWidgets.size();
    for( int nW = 0; nW < nWidgets; nW++ )
    {
        const PDFWidget& rWidget = m_aWidgets[nW];
        AnnotSortContainer& rCont = sorted[ rWidget.m_nPage ];
        // optimize vector allocation
        if( rCont.aSortedAnnots.empty() )
            rCont.aSortedAnnots.reserve( m_aPages[ rWidget.m_nPage ].m_aAnnotations.size() );
        // insert widget to tab sorter
        // RadioButtons are not page annotations, only their individual check boxes are
        if( rWidget.m_eType != PDFWriter::RadioButton )
        {
            rCont.aObjects.insert( rWidget.m_nObject );
            rCont.aSortedAnnots.push_back( AnnotationSortEntry( rWidget.m_nTabOrder, rWidget.m_nObject, nW ) );
        }
    }
    for( std::hash_map< sal_Int32, AnnotSortContainer >::iterator it = sorted.begin(); it != sorted.end(); ++it )
    {
        // append entries for non widget annotations
        PDFPage& rPage = m_aPages[ it->first ];
        unsigned int nAnnots = rPage.m_aAnnotations.size();
        for( unsigned int nA = 0; nA < nAnnots; nA++ )
            if( it->second.aObjects.find( rPage.m_aAnnotations[nA] ) == it->second.aObjects.end())
                it->second.aSortedAnnots.push_back( AnnotationSortEntry( 10000, rPage.m_aAnnotations[nA], -1 ) );

        AnnotSorterLess aLess( m_aWidgets );
        std::stable_sort( it->second.aSortedAnnots.begin(), it->second.aSortedAnnots.end(), aLess );
        // sanity check
        if( it->second.aSortedAnnots.size() == nAnnots)
        {
            for( unsigned int nA = 0; nA < nAnnots; nA++ )
                rPage.m_aAnnotations[nA] = it->second.aSortedAnnots[nA].nObject;
        }
        else
        {
            DBG_ASSERT( 0, "wrong number of sorted annotations" );
            #if OSL_DEBUG_LEVEL > 0
            fprintf( stderr, "PDFWriterImpl::sortWidgets(): wrong number of sorted assertions on page nr %ld\n"
					 "    %ld sorted and %ld unsorted\n", (long int)it->first, (long int)it->second.aSortedAnnots.size(), (long int)nAnnots );
            #endif
        }
    }

    // FIXME: implement tab order in structure tree for PDF 1.5
}

namespace vcl {
class PDFStreamIf :
		public cppu::WeakImplHelper1< com::sun::star::io::XOutputStream	>
{
    PDFWriterImpl*  m_pWriter;
    bool            m_bWrite;
    public:
    PDFStreamIf( PDFWriterImpl* pWriter ) : m_pWriter( pWriter ), m_bWrite( true ) {}
    virtual ~PDFStreamIf();

    virtual void SAL_CALL writeBytes( const com::sun::star::uno::Sequence< sal_Int8 >& aData ) throw();
    virtual void SAL_CALL flush() throw();
    virtual void SAL_CALL closeOutput() throw();
};
}

PDFStreamIf::~PDFStreamIf()
{
}

void SAL_CALL  PDFStreamIf::writeBytes( const com::sun::star::uno::Sequence< sal_Int8 >& aData ) throw()
{
    if( m_bWrite )
    {
        sal_Int32 nBytes = aData.getLength();
        if( nBytes > 0 )
            m_pWriter->writeBuffer( aData.getConstArray(), nBytes );
    }
}

void SAL_CALL PDFStreamIf::flush() throw()
{
}

void SAL_CALL PDFStreamIf::closeOutput() throw()
{
    m_bWrite = false;
}

bool PDFWriterImpl::emitAdditionalStreams()
{
    unsigned int nStreams = m_aAdditionalStreams.size();
    for( unsigned int i = 0; i < nStreams; i++ )
    {
        PDFAddStream& rStream = m_aAdditionalStreams[i];
        rStream.m_nStreamObject = createObject();
        sal_Int32 nSizeObject = createObject();

        if( ! updateObject( rStream.m_nStreamObject ) )
            return false;

        OStringBuffer aLine;
        aLine.append( rStream.m_nStreamObject );
        aLine.append( " 0 obj\n<</Length " );
        aLine.append( nSizeObject );
        aLine.append( " 0 R" );
        if( rStream.m_bCompress )
            aLine.append( "/Filter/FlateDecode" );
        aLine.append( ">>\nstream\n" );
        if( ! writeBuffer( aLine.getStr(), aLine.getLength() ) )
            return false;
        sal_uInt64 nBeginStreamPos = 0, nEndStreamPos = 0;
        if( osl_File_E_None != osl_getFilePos( m_aFile, &nBeginStreamPos ) )
        {
            osl_closeFile( m_aFile );
            m_bOpen = false;
        }
        if( rStream.m_bCompress )
            beginCompression();

        checkAndEnableStreamEncryption( rStream.m_nStreamObject );
        com::sun::star::uno::Reference< com::sun::star::io::XOutputStream > xStream( new PDFStreamIf( this ) );
        rStream.m_pStream->write( xStream );
        xStream.clear();
        delete rStream.m_pStream;
        rStream.m_pStream = NULL;
        disableStreamEncryption();

        if( rStream.m_bCompress )
            endCompression();

        if( osl_File_E_None != osl_getFilePos( m_aFile, &nEndStreamPos ) )
        {
            osl_closeFile( m_aFile );
            m_bOpen = false;
            return false;
        }
        if( ! writeBuffer( "\nendstream\nendobj\n\n", 19 ) )
            return false ;
        // emit stream length object
        if( ! updateObject( nSizeObject ) )
            return false;
        aLine.setLength( 0 );
        aLine.append( nSizeObject );
        aLine.append( " 0 obj\n" );
        aLine.append( (sal_Int64)(nEndStreamPos-nBeginStreamPos) );
        aLine.append( "\nendobj\n\n" );
        if( ! writeBuffer( aLine.getStr(), aLine.getLength() ) )
            return false;
    }
    return true;
}

bool PDFWriterImpl::emit()
{
    endPage();

    // resort structure tree and annotations if necessary
    // needed for widget tab order
    sortWidgets();

    // emit additional streams
    CHECK_RETURN( emitAdditionalStreams() );

    // emit catalog
    CHECK_RETURN( emitCatalog() );

    // emit trailer
    CHECK_RETURN( emitTrailer() );

    osl_closeFile( m_aFile );
    m_bOpen = false;

    return true;
}

std::set< PDFWriter::ErrorCode > PDFWriterImpl::getErrors()
{
    return m_aErrors;
}


#if defined USE_JAVA && defined MACOSX
void PDFWriterImpl::registerGlyphs( int nGlyphs,
                                    sal_GlyphId* pGlyphs,
                                    sal_Ucs* pUnicodes,
                                    sal_uInt16* pMappedGlyphs,
                                    bool* pMappedIdentityGlyphs,
                                    sal_Int32* pMappedFontObjects,
                                    sal_Int32* pMappedFontSubObjects,
                                    const ImplFontData* pFallbackFonts[] )
#else	// USE_JAVA && MACOSX
void PDFWriterImpl::registerGlyphs( int nGlyphs,
                                    sal_GlyphId* pGlyphs,
                                    sal_Int32* pGlyphWidths,
                                    sal_Ucs* pUnicodes,
                                    sal_uInt8* pMappedGlyphs,
                                    sal_Int32* pMappedFontObjects,
                                    const ImplFontData* pFallbackFonts[] )
#endif	// USE_JAVA && MACOSX
{
    const ImplFontData* pDevFont = m_pReferenceDevice->mpFontEntry->maFontSelData.mpFontData;
    for( int i = 0; i < nGlyphs; i++ )
    {
#if defined USE_JAVA && defined MACOSX
        if( ! pGlyphs[i] || pGlyphs[i] & ( GF_ISCHAR | GF_GSUB ) )
            continue;
        const int nFontGlyphId = pGlyphs[i] & GF_IDXMASK;
#else	// USE_JAVA && MACOSX
        const int nFontGlyphId = pGlyphs[i] & (GF_IDXMASK | GF_ISCHAR | GF_GSUB);
#endif	// USE_JAVA && MACOSX
        const ImplFontData* pCurrentFont = pFallbackFonts[i] ? pFallbackFonts[i] : pDevFont;

        if( isBuiltinFont( pCurrentFont ) )
        {
            sal_Int32 nFontID = 0;
            FontEmbedData::iterator it = m_aEmbeddedFonts.find( pCurrentFont );
            if( it != m_aEmbeddedFonts.end() )
                nFontID = it->second.m_nNormalFontID;
            else
            {
                nFontID = m_nNextFID++;
                m_aEmbeddedFonts[ pCurrentFont ] = EmbedFont();
                m_aEmbeddedFonts[ pCurrentFont ].m_nNormalFontID = nFontID;
            }

#if defined USE_JAVA && defined MACOSX
            pMappedGlyphs[ i ] = sal::static_int_cast<sal_uInt16>( nFontGlyphId );
#else	// USE_JAVA && MACOSX
            pGlyphWidths[ i ] = 0;
            pMappedGlyphs[ i ] = sal::static_int_cast<sal_Int8>( nFontGlyphId );
#endif	// USE_JAVA && MACOSX
            pMappedFontObjects[ i ] = nFontID;
#if !defined USE_JAVA || !defined MACOSX
            const ImplPdfBuiltinFontData* pFD = GetPdfFontData( pCurrentFont );
            if( pFD )
            {
                const BuiltinFont* pBuiltinFont = pFD->GetBuiltinFont();
                pGlyphWidths[i] = pBuiltinFont->m_aWidths[ nFontGlyphId & 0x00ff ];
            }
#endif	// !USE_JAVA || !MACOSX
        }
        else if( pCurrentFont->mbSubsettable )
        {
#if defined USE_JAVA && defined MACOSX
            FontSubset& rSubset = m_aSubsets[ pCurrentFont->GetFontId() ];
#else	// USE_JAVA && MACOSX
            FontSubset& rSubset = m_aSubsets[ pCurrentFont ];
#endif	// USE_JAVA && MACOSX
            // search for font specific glyphID
            FontMapping::iterator it = rSubset.m_aMapping.find( nFontGlyphId );
            if( it != rSubset.m_aMapping.end() )
            {
                pMappedFontObjects[i] = it->second.m_nFontID;
                pMappedGlyphs[i] = it->second.m_nSubsetGlyphID;
#if defined USE_JAVA && defined MACOSX
                pMappedFontSubObjects[i] = it->second.m_nFontSubID;
                pMappedIdentityGlyphs[i] = it->second.m_bIdentityGlyph;
#endif	// USE_JAVA && MACOSX
            }
            else
            {
                // create new subset if necessary
                if( rSubset.m_aSubsets.empty()
                || (rSubset.m_aSubsets.back().m_aMapping.size() > 254) )
                {
                    rSubset.m_aSubsets.push_back( FontEmit( m_nNextFID++ ) );
                }

                // copy font id
                pMappedFontObjects[i] = rSubset.m_aSubsets.back().m_nFontID;
                // create new glyph in subset
#if defined USE_JAVA && defined MACOSX
                sal_uInt16 nNewId = 0;
#else	// USE_JAVA && MACOSX
                sal_uInt8 nNewId = sal::static_int_cast<sal_uInt8>(rSubset.m_aSubsets.back().m_aMapping.size()+1);
#endif	// USE_JAVA && MACOSX
                pMappedGlyphs[i] = nNewId;

                // add new glyph to emitted font subset
                GlyphEmit& rNewGlyphEmit = rSubset.m_aSubsets.back().m_aMapping[ nFontGlyphId ];
                rNewGlyphEmit.m_nSubsetGlyphID = nNewId;
                rNewGlyphEmit.m_aUnicode = (pUnicodes ? pUnicodes[i] : 0);

                // add new glyph to font mapping
                Glyph& rNewGlyph = rSubset.m_aMapping[ nFontGlyphId ];
                rNewGlyph.m_nFontID = pMappedFontObjects[i];
                rNewGlyph.m_nSubsetGlyphID = nNewId;
#if defined USE_JAVA && defined MACOSX
                rNewGlyph.m_nFontSubID = nNewId;
                rNewGlyph.m_bIdentityGlyph = false;
                pMappedFontSubObjects[i] = rNewGlyph.m_nFontSubID;
                pMappedIdentityGlyphs[i] = rNewGlyph.m_bIdentityGlyph;
#endif	// USE_JAVA && MACOSX
            }
            getReferenceDevice()->ImplGetGraphics();
#if !defined USE_JAVA || !defined MACOSX
            const bool bVertical = ((pGlyphs[i] & GF_ROTMASK) != 0);
            pGlyphWidths[i] = m_aFontCache.getGlyphWidth( pCurrentFont,
                                                          nFontGlyphId,
                                                          bVertical,
                                                          m_pReferenceDevice->mpGraphics );
#endif	// !USE_JAVA || !MACOSX
        }
        else if( pCurrentFont->IsEmbeddable() )
        {
            sal_Int32 nFontID = 0;
            FontEmbedData::iterator it = m_aEmbeddedFonts.find( pCurrentFont );
            if( it != m_aEmbeddedFonts.end() )
                nFontID = it->second.m_nNormalFontID;
            else
            {
                nFontID = m_nNextFID++;
                m_aEmbeddedFonts[ pCurrentFont ] = EmbedFont();
                m_aEmbeddedFonts[ pCurrentFont ].m_nNormalFontID = nFontID;
            }
            EmbedFont& rEmbedFont = m_aEmbeddedFonts[pCurrentFont];

            const Ucs2SIntMap* pEncoding = NULL;
            const Ucs2OStrMap* pNonEncoded = NULL;
            getReferenceDevice()->ImplGetGraphics();
            pEncoding = m_pReferenceDevice->mpGraphics->GetFontEncodingVector( pCurrentFont, &pNonEncoded );

            Ucs2SIntMap::const_iterator enc_it;
            Ucs2OStrMap::const_iterator nonenc_it;

            sal_Int32 nCurFontID = nFontID;
            sal_Ucs cChar = pUnicodes[i];
            if( pEncoding )
            {
                enc_it = pEncoding->find( cChar );
                if( enc_it != pEncoding->end() && enc_it->second > 0 )
                {
                    DBG_ASSERT( (enc_it->second & 0xffffff00) == 0, "Invalid character code" );
                    cChar = (sal_Ucs)enc_it->second;
                }
                else if( (enc_it == pEncoding->end() || enc_it->second == -1) &&
                         pNonEncoded &&
                         (nonenc_it = pNonEncoded->find( cChar )) != pNonEncoded->end() )
                {
                    nCurFontID = 0;
                    // find non encoded glyph
                    for( std::list< EmbedEncoding >::iterator nec_it = rEmbedFont.m_aExtendedEncodings.begin(); nec_it != rEmbedFont.m_aExtendedEncodings.end(); ++nec_it )
                    {
                        if( nec_it->m_aCMap.find( cChar ) != nec_it->m_aCMap.end() )
                        {
                            nCurFontID = nec_it->m_nFontID;
                            cChar = (sal_Ucs)nec_it->m_aCMap[ cChar ];
                            break;
                        }
                    }
                    if( nCurFontID == 0 ) // new nonencoded glyph
                    {
                        if( rEmbedFont.m_aExtendedEncodings.empty() || rEmbedFont.m_aExtendedEncodings.back().m_aEncVector.size() == 255 )
                        {
                            rEmbedFont.m_aExtendedEncodings.push_back( EmbedEncoding() );
                            rEmbedFont.m_aExtendedEncodings.back().m_nFontID = m_nNextFID++;
                        }
                        EmbedEncoding& rEncoding = rEmbedFont.m_aExtendedEncodings.back();
                        rEncoding.m_aEncVector.push_back( EmbedCode() );
                        rEncoding.m_aEncVector.back().m_aUnicode = cChar;
                        rEncoding.m_aEncVector.back().m_aName = nonenc_it->second;
                        rEncoding.m_aCMap[ cChar ] = (sal_Int8)(rEncoding.m_aEncVector.size()-1);
                        nCurFontID = rEncoding.m_nFontID;
                        cChar = (sal_Ucs)rEncoding.m_aCMap[ cChar ];
                    }
                }
                else
                    pEncoding = NULL;
            }
            if( ! pEncoding )
            {
                if( cChar & 0xff00 )
                {
                    // some characters can be used by conversion
                    if( cChar >= 0xf000 && cChar <= 0xf0ff ) // symbol encoding in private use area
                        cChar -= 0xf000;
                    else
                    {
                        String aString(cChar);
                        ByteString aChar( aString, RTL_TEXTENCODING_MS_1252 );
                        cChar = ((sal_Ucs)aChar.GetChar( 0 )) & 0x00ff;
                    }
                }
            }

            pMappedGlyphs[ i ] = (sal_Int8)cChar;
            pMappedFontObjects[ i ] = nCurFontID;
#if !defined USE_JAVA || !defined MACOSX
            pGlyphWidths[ i ] = m_aFontCache.getGlyphWidth( pCurrentFont,
                                                            (pEncoding ? pUnicodes[i] : cChar) | GF_ISCHAR,
                                                            false,
                                                            m_pReferenceDevice->mpGraphics );
#endif	// !USE_JAVA || !MACOSX
        }
    }
}

void PDFWriterImpl::drawRelief( SalLayout& rLayout, const String& rText, bool bTextLines )
{
    push( PUSH_ALL );

    FontRelief eRelief = m_aCurrentPDFState.m_aFont.GetRelief();

    Color aTextColor = m_aCurrentPDFState.m_aFont.GetColor();
    Color aTextLineColor = m_aCurrentPDFState.m_aTextLineColor;
    Color aOverlineColor = m_aCurrentPDFState.m_aOverlineColor;
    Color aReliefColor( COL_LIGHTGRAY );
    if( aTextColor == COL_BLACK )
        aTextColor = Color( COL_WHITE );
    if( aTextLineColor == COL_BLACK )
        aTextLineColor = Color( COL_WHITE );
    if( aOverlineColor == COL_BLACK )
        aOverlineColor = Color( COL_WHITE );
    if( aTextColor == COL_WHITE )
        aReliefColor = Color( COL_BLACK );

    Font aSetFont = m_aCurrentPDFState.m_aFont;
    aSetFont.SetRelief( RELIEF_NONE );
    aSetFont.SetShadow( FALSE );

    aSetFont.SetColor( aReliefColor );
    setTextLineColor( aReliefColor );
    setOverlineColor( aReliefColor );
    setFont( aSetFont );
    long nOff = 1 + getReferenceDevice()->mnDPIX/300;
    if( eRelief == RELIEF_ENGRAVED )
        nOff = -nOff;

    rLayout.DrawOffset() += Point( nOff, nOff );
    updateGraphicsState();
    drawLayout( rLayout, rText, bTextLines );

    rLayout.DrawOffset() -= Point( nOff, nOff );
    setTextLineColor( aTextLineColor );
    setOverlineColor( aOverlineColor );
    aSetFont.SetColor( aTextColor );
    setFont( aSetFont );
    updateGraphicsState();
    drawLayout( rLayout, rText, bTextLines );

    // clean up the mess
    pop();
}

void PDFWriterImpl::drawShadow( SalLayout& rLayout, const String& rText, bool bTextLines )
{
    Font aSaveFont = m_aCurrentPDFState.m_aFont;
    Color aSaveTextLineColor = m_aCurrentPDFState.m_aTextLineColor;
    Color aSaveOverlineColor = m_aCurrentPDFState.m_aOverlineColor;

    Font& rFont = m_aCurrentPDFState.m_aFont;
    if( rFont.GetColor() == Color( COL_BLACK ) || rFont.GetColor().GetLuminance() < 8 )
        rFont.SetColor( Color( COL_LIGHTGRAY ) );
    else
        rFont.SetColor( Color( COL_BLACK ) );
    rFont.SetShadow( FALSE );
    rFont.SetOutline( FALSE );
    setFont( rFont );
    setTextLineColor( rFont.GetColor() );
    setOverlineColor( rFont.GetColor() );
    updateGraphicsState();

    long nOff = 1 + ((m_pReferenceDevice->mpFontEntry->mnLineHeight-24)/24);
    if( rFont.IsOutline() )
        nOff++;
    rLayout.DrawBase() += Point( nOff, nOff );
    drawLayout( rLayout, rText, bTextLines );
    rLayout.DrawBase() -= Point( nOff, nOff );

    setFont( aSaveFont );
    setTextLineColor( aSaveTextLineColor );
    setOverlineColor( aSaveOverlineColor );
    updateGraphicsState();
}

void PDFWriterImpl::drawVerticalGlyphs(
        const std::vector<PDFWriterImpl::PDFGlyph>& rGlyphs,
        OStringBuffer& rLine,
        const Point& rAlignOffset,
        const Matrix3& rRotScale,
        double fAngle,
        double fXScale,
        double fSkew,
        sal_Int32 nFontHeight )
{
    long nXOffset = 0;
    Point aCurPos( rGlyphs[0].m_aPos );
    aCurPos = m_pReferenceDevice->PixelToLogic( aCurPos );
    aCurPos += rAlignOffset;
    for( size_t i = 0; i < rGlyphs.size(); i++ )
    {
        // have to emit each glyph on its own
        double fDeltaAngle = 0.0;
        double fYScale = 1.0;
        double fTempXScale = fXScale;
        double fSkewB = fSkew;
        double fSkewA = 0.0;

        Point aDeltaPos;
        if( ( rGlyphs[i].m_nGlyphId & GF_ROTMASK ) == GF_ROTL )
        {
            fDeltaAngle = M_PI/2.0;
#if defined USE_JAVA && defined MACOSX
            SalATSLayout *pATSLayout = NULL;
            MultiSalLayout *pMultiLayout = dynamic_cast<MultiSalLayout*>( rGlyphs[i].m_pLayout );
            if ( pMultiLayout )
                pATSLayout = dynamic_cast<SalATSLayout*>( pMultiLayout->GetLayout( ( rGlyphs[i].m_nGlyphId & GF_FONTMASK ) >> GF_FONTSHIFT ) );
            if ( !pATSLayout )
                pATSLayout = dynamic_cast<SalATSLayout*>( rGlyphs[i].m_pLayout );

            if ( pATSLayout )
            {
                long nX;
                long nY;
                pATSLayout->GetVerticalGlyphTranslation( rGlyphs[i].m_nGlyphId, rGlyphs[i].m_nCharPos, nX, nY );
                aDeltaPos.X() = m_pReferenceDevice->ImplDevicePixelToLogicHeight( (double)nY / pATSLayout->GetUnitsPerPixel() );
                aDeltaPos.Y() = m_pReferenceDevice->ImplDevicePixelToLogicWidth( (long)( ( fXScale * nX * -1 / pATSLayout->GetUnitsPerPixel() ) + 0.5 ) );
            }
            else
            {
#endif	// USE_JAVA && MACOSX
            aDeltaPos.X() = m_pReferenceDevice->GetFontMetric().GetAscent();
            aDeltaPos.Y() = (int)((double)m_pReferenceDevice->GetFontMetric().GetDescent() * fXScale);
#if defined USE_JAVA && defined MACOSX
            }
#endif	// USE_JAVA && MACOSX
            fYScale = fXScale;
            fTempXScale = 1.0;
            fSkewA = -fSkewB;
            fSkewB = 0.0;
        }
        else if( ( rGlyphs[i].m_nGlyphId & GF_ROTMASK ) == GF_ROTR )
        {
            fDeltaAngle = -M_PI/2.0;
#if defined USE_JAVA && defined MACOSX
            SalATSLayout *pATSLayout = NULL;
            MultiSalLayout *pMultiLayout = dynamic_cast<MultiSalLayout*>( rGlyphs[i].m_pLayout );
            if ( pMultiLayout )
                pATSLayout = dynamic_cast<SalATSLayout*>( pMultiLayout->GetLayout( ( rGlyphs[i].m_nGlyphId & GF_FONTMASK ) >> GF_FONTSHIFT ) );
            if ( !pATSLayout )
                pATSLayout = dynamic_cast<SalATSLayout*>( rGlyphs[i].m_pLayout );

            if ( pATSLayout )
            {
                long nX;
                long nY;
                pATSLayout->GetVerticalGlyphTranslation( rGlyphs[i].m_nGlyphId, rGlyphs[i].m_nCharPos, nX, nY );
                aDeltaPos.X() = m_pReferenceDevice->ImplDevicePixelToLogicHeight( (long)( ( (double)( rGlyphs[i].m_nNativeWidth - nY ) / pATSLayout->GetUnitsPerPixel() ) + 0.5 ) );
                aDeltaPos.Y() = m_pReferenceDevice->ImplDevicePixelToLogicWidth( (long)( ( fXScale * nX * -1 / pATSLayout->GetUnitsPerPixel() ) + 0.5 ) );
            }
            else
            {
#endif	// USE_JAVA && MACOSX
            aDeltaPos.X() = (int)((double)m_pReferenceDevice->GetFontMetric().GetDescent()*fXScale);
            aDeltaPos.Y() = -m_pReferenceDevice->GetFontMetric().GetAscent();
#if defined USE_JAVA && defined MACOSX
            }
#endif	// USE_JAVA && MACOSX
            fYScale = fXScale;
            fTempXScale = 1.0;
            fSkewA = fSkewB;
            fSkewB = 0.0;
        }
        aDeltaPos += (m_pReferenceDevice->PixelToLogic( Point( (int)((double)nXOffset/fXScale), 0 ) ) - m_pReferenceDevice->PixelToLogic( Point() ) );
        if( i < rGlyphs.size()-1 )
            nXOffset += rGlyphs[i+1].m_aPos.Y() - rGlyphs[i].m_aPos.Y();
        if( ! rGlyphs[i].m_nGlyphId )
            continue;

        aDeltaPos = rRotScale.transform( aDeltaPos );

        Matrix3 aMat;
        if( fSkewB != 0.0 || fSkewA != 0.0 )
            aMat.skew( fSkewA, fSkewB );
#ifdef USE_JAVA
        // Glyph scaling is reversed for rotated glyphs
        if( rGlyphs[i].m_nGlyphId & GF_ROTMASK )
            aMat.scale( fYScale, fTempXScale );
        else
#endif	// USE_JAVA
        aMat.scale( fTempXScale, fYScale );
        aMat.rotate( fAngle+fDeltaAngle );
        aMat.translate( aCurPos.X()+aDeltaPos.X(), aCurPos.Y()+aDeltaPos.Y() );
        aMat.append( m_aPages.back(), rLine );
        rLine.append( " Tm" );
#if defined USE_JAVA && defined MACOSX
        if( i == 0 || rGlyphs[i-1].m_nMappedFontId != rGlyphs[i].m_nMappedFontId || rGlyphs[i-1].m_nMappedFontSubId != rGlyphs[i].m_nMappedFontSubId )
        {
            rLine.append( " /F" );
            rLine.append( rGlyphs[i].m_nMappedFontId );
            rLine.append( '.' );
            rLine.append( rGlyphs[i].m_nMappedFontSubId );
            rLine.append( ' ' );
            m_aPages.back().appendMappedLength( nFontHeight, rLine, true );
            rLine.append( " Tf" );
        }
        rLine.append( "<" );
        if ( rGlyphs[i].m_bIdentityGlyph )
            appendHex( (sal_Int8)( ( rGlyphs[i].m_nMappedGlyphId & 0xff00 ) >> 8 ), rLine );
        appendHex( (sal_Int8)( rGlyphs[i].m_nMappedGlyphId & 0x00ff ), rLine );
#else	// USE_JAVA && MACOSX
        if( i == 0 || rGlyphs[i-1].m_nMappedFontId != rGlyphs[i].m_nMappedFontId )
        {
            rLine.append( " /F" );
            rLine.append( rGlyphs[i].m_nMappedFontId );
            rLine.append( ' ' );
            m_aPages.back().appendMappedLength( nFontHeight, rLine, true );
            rLine.append( " Tf" );
        }
        rLine.append( "<" );
        appendHex( rGlyphs[i].m_nMappedGlyphId, rLine );
#endif	// USE_JAVA && MACOSX
        rLine.append( ">Tj\n" );
    }
}

void PDFWriterImpl::drawHorizontalGlyphs(
        const std::vector<PDFWriterImpl::PDFGlyph>& rGlyphs,
        OStringBuffer& rLine,
        const Point& rAlignOffset,
        double fAngle,
        double fXScale,
        double fSkew,
        sal_Int32 nFontHeight,
        sal_Int32 nPixelFontHeight
        )
{
    // horizontal (= normal) case

    // fill in  run end indices
    // end is marked by index of the first glyph of the next run
    // a run is marked by same mapped font id and same Y position
    std::vector< sal_uInt32 > aRunEnds;
    aRunEnds.reserve( rGlyphs.size() );
    for( size_t i = 1; i < rGlyphs.size(); i++ )
    {
        if( rGlyphs[i].m_nMappedFontId != rGlyphs[i-1].m_nMappedFontId ||
#if defined USE_JAVA && defined MACOSX
            rGlyphs[i].m_nMappedFontSubId != rGlyphs[i-1].m_nMappedFontSubId ||
#endif	// USE_JAVA && MACOSX
            rGlyphs[i].m_aPos.Y() != rGlyphs[i-1].m_aPos.Y() )
        {
            aRunEnds.push_back(i);
        }
    }
    // last run ends at last glyph
    aRunEnds.push_back( rGlyphs.size() );

    // loop over runs of the same font
    sal_uInt32 nBeginRun = 0;
    for( size_t nRun = 0; nRun < aRunEnds.size(); nRun++ )
    {
        // setup text matrix
        Point aCurPos = rGlyphs[nBeginRun].m_aPos;
        // back transformation to current coordinate system
        aCurPos = m_pReferenceDevice->PixelToLogic( aCurPos );
        aCurPos += rAlignOffset;
        // the first run can be set with "Td" operator
        // subsequent use of that operator would move
        // the texline matrix relative to what was set before
        // making use of that would drive us into rounding issues
        Matrix3 aMat;
        if( nRun == 0 && fAngle == 0.0 && fXScale == 1.0 && fSkew == 0.0 )
        {
            m_aPages.back().appendPoint( aCurPos, rLine, false );
            rLine.append( " Td " );
        }
        else
        {
            if( fSkew != 0.0 )
                aMat.skew( 0.0, fSkew );
            aMat.scale( fXScale, 1.0 );
            aMat.rotate( fAngle );
            aMat.translate( aCurPos.X(), aCurPos.Y() );
            aMat.append( m_aPages.back(), rLine );
            rLine.append( " Tm\n" );
        }
        // set up correct font
        rLine.append( "/F" );
        rLine.append( rGlyphs[nBeginRun].m_nMappedFontId );
#if defined USE_JAVA && defined MACOSX
        rLine.append( '.' );
        rLine.append( rGlyphs[nBeginRun].m_nMappedFontSubId );
#endif	// USE_JAVA && MACOSX
        rLine.append( ' ' );
        m_aPages.back().appendMappedLength( nFontHeight, rLine, true );
        rLine.append( " Tf" );

        // output glyphs using Tj or TJ
        OStringBuffer aKernedLine( 256 ), aUnkernedLine( 256 );
        aKernedLine.append( "[<" );
        aUnkernedLine.append( '<' );
#if defined USE_JAVA && defined MACOSX
        if ( rGlyphs[nBeginRun].m_bIdentityGlyph )
            appendHex( (sal_Int8)( ( rGlyphs[nBeginRun].m_nMappedGlyphId & 0xff00 ) >> 8 ), aKernedLine );
        appendHex( (sal_Int8)( rGlyphs[nBeginRun].m_nMappedGlyphId & 0x00ff ), aKernedLine );
        if ( rGlyphs[nBeginRun].m_bIdentityGlyph )
            appendHex( (sal_Int8)( ( rGlyphs[nBeginRun].m_nMappedGlyphId & 0xff00 ) >> 8 ), aUnkernedLine );
        appendHex( (sal_Int8)( rGlyphs[nBeginRun].m_nMappedGlyphId & 0x00ff ), aUnkernedLine );
#else	// USE_JAVA && MACOSX
        appendHex( rGlyphs[nBeginRun].m_nMappedGlyphId, aKernedLine );
        appendHex( rGlyphs[nBeginRun].m_nMappedGlyphId, aUnkernedLine );
#endif	// USE_JAVA && MACOSX

        aMat.invert();
        bool bNeedKern = false;
        for( sal_uInt32 nPos = nBeginRun+1; nPos < aRunEnds[nRun]; nPos++ )
        {
#if defined USE_JAVA && defined MACOSX
            if ( rGlyphs[nPos].m_bIdentityGlyph )
                appendHex( (sal_Int8)( ( rGlyphs[nPos].m_nMappedGlyphId & 0xff00 ) >> 8 ), aUnkernedLine );
            appendHex( (sal_Int8)( rGlyphs[nPos].m_nMappedGlyphId & 0x00ff ), aUnkernedLine );
#else	// USE_JAVA && MACOSX
            appendHex( rGlyphs[nPos].m_nMappedGlyphId, aUnkernedLine );
#endif	// USE_JAVA && MACOSX
            // check if glyph advance matches with the width of the previous glyph, else adjust
            const Point aThisPos = aMat.transform( rGlyphs[nPos].m_aPos );
#if defined USE_JAVA && defined MACOSX
            // Fix bug 3659 by subtracting the real native width from the
            // OOo kerning positions
            const Point aPrevPos = aMat.transform( Point( rGlyphs[nPos-1].m_aPos.X() + (long)( ( fXScale * rGlyphs[nPos-1].m_nRealNativeWidth ) + 0.5 ), rGlyphs[nPos-1].m_aPos.Y() ) );
            double fAdvance = aPrevPos.X() - aThisPos.X();
            fAdvance *= 1000.0 / (fXScale * nPixelFontHeight);
            const sal_Int32 nAdjustment = sal_Int32(fAdvance+0.5);
#else	// USE_JAVA && MACOSX
            const Point aPrevPos = aMat.transform( rGlyphs[nPos-1].m_aPos );
            double fAdvance = aThisPos.X() - aPrevPos.X();
            fAdvance *= 1000.0 / (fXScale * nPixelFontHeight);
            const sal_Int32 nAdjustment = rGlyphs[nPos-1].m_nNativeWidth - sal_Int32(fAdvance+0.5);
#endif	// USE_JAVA && MACOSX
            if( nAdjustment != 0 )
            {
                bNeedKern = true;
                aKernedLine.append( ">" );
                aKernedLine.append( nAdjustment );
                aKernedLine.append( "<" );
            }
#if defined USE_JAVA && defined MACOSX
            else if ( rGlyphs[nPos].m_bIdentityGlyph || rGlyphs[nPos-1].m_bIdentityGlyph )
            {
                bNeedKern = true;
                aKernedLine.append( "><" );
            }

            if ( rGlyphs[nPos].m_bIdentityGlyph )
                appendHex( (sal_Int8)( ( rGlyphs[nPos].m_nMappedGlyphId & 0xff00 ) >> 8 ), aKernedLine );
            appendHex( (sal_Int8)( rGlyphs[nPos].m_nMappedGlyphId & 0x00ff ), aKernedLine );
#else	// USE_JAVA && MACOSX
            appendHex( rGlyphs[nPos].m_nMappedGlyphId, aKernedLine );
#endif	// USE_JAVA && MACOSX
        }
        aKernedLine.append( ">]TJ\n" );
        aUnkernedLine.append( ">Tj\n" );
        rLine.append( bNeedKern ? aKernedLine : aUnkernedLine );

        // set beginning of next run
        nBeginRun = aRunEnds[nRun];
    }
}

void PDFWriterImpl::drawLayout( SalLayout& rLayout, const String& rText, bool bTextLines )
{
    // relief takes precedence over shadow (see outdev3.cxx)
    if(  m_aCurrentPDFState.m_aFont.GetRelief() != RELIEF_NONE )
    {
        drawRelief( rLayout, rText, bTextLines );
        return;
    }
    else if( m_aCurrentPDFState.m_aFont.IsShadow() )
        drawShadow( rLayout, rText, bTextLines );

    OStringBuffer aLine( 512 );

    const int nMaxGlyphs = 256;

    sal_GlyphId pGlyphs[nMaxGlyphs];
    sal_Int32 pGlyphWidths[nMaxGlyphs];
#if defined USE_JAVA && defined MACOSX
    sal_uInt16 pMappedGlyphs[nMaxGlyphs];
    bool pMappedIdentityGlyphs[nMaxGlyphs];
#else	// USE_JAVA && MACOSX
    sal_uInt8 pMappedGlyphs[nMaxGlyphs];
#endif	// USE_JAVA && MACOSX
    sal_Int32 pMappedFontObjects[nMaxGlyphs];
#if defined USE_JAVA && defined MACOSX
    sal_Int32 pMappedFontSubObjects[nMaxGlyphs];
#endif	// USE_JAVA && MACOSX
    sal_Ucs   pUnicodes[nMaxGlyphs];
    int pCharPosAry[nMaxGlyphs];
    sal_Int32 nAdvanceWidths[nMaxGlyphs];
    const ImplFontData* pFallbackFonts[nMaxGlyphs];
    bool bVertical = m_aCurrentPDFState.m_aFont.IsVertical();
    int nGlyphs;
    int nIndex = 0;
    int nMinCharPos = 0, nMaxCharPos = rText.Len()-1;
    double fXScale = 1.0;
    double fSkew = 0.0;
    sal_Int32 nPixelFontHeight = m_pReferenceDevice->mpFontEntry->maFontSelData.mnHeight;
    TextAlign eAlign = m_aCurrentPDFState.m_aFont.GetAlign();

    // transform font height back to current units
    // note: the layout calculates in outdevs device pixel !!
    sal_Int32 nFontHeight = m_pReferenceDevice->ImplDevicePixelToLogicHeight( nPixelFontHeight );
    if( m_aCurrentPDFState.m_aFont.GetWidth() )
    {
        Font aFont( m_aCurrentPDFState.m_aFont );
        aFont.SetWidth( 0 );
        FontMetric aMetric = m_pReferenceDevice->GetFontMetric( aFont );
        if( aMetric.GetWidth() != m_aCurrentPDFState.m_aFont.GetWidth() )
        {
            fXScale =
                (double)m_aCurrentPDFState.m_aFont.GetWidth() /
                (double)aMetric.GetWidth();
        }
        // force state before GetFontMetric
        m_pReferenceDevice->ImplNewFont();
    }

#if !defined USE_JAVA || !defined MACOSX
    // perform artificial italics if necessary
    if( ( m_aCurrentPDFState.m_aFont.GetItalic() == ITALIC_NORMAL ||
          m_aCurrentPDFState.m_aFont.GetItalic() == ITALIC_OBLIQUE ) &&
        !( m_pReferenceDevice->mpFontEntry->maFontSelData.mpFontData->GetSlant() == ITALIC_NORMAL ||
           m_pReferenceDevice->mpFontEntry->maFontSelData.mpFontData->GetSlant() == ITALIC_OBLIQUE )
        )
    {
        fSkew = M_PI/12.0;
    }
#endif	// !USE_JAVA || !MACOSX

    // if the mapmode is distorted we need to adjust for that also
    if( m_aCurrentPDFState.m_aMapMode.GetScaleX() != m_aCurrentPDFState.m_aMapMode.GetScaleY() )
    {
        fXScale *= double(m_aCurrentPDFState.m_aMapMode.GetScaleX()) / double(m_aCurrentPDFState.m_aMapMode.GetScaleY());
    }

    int nAngle = m_aCurrentPDFState.m_aFont.GetOrientation();
    // normalize angles
    while( nAngle < 0 )
        nAngle += 3600;
    nAngle = nAngle % 3600;
    double fAngle = (double)nAngle * M_PI / 1800.0;

    Matrix3 aRotScale;
    aRotScale.scale( fXScale, 1.0 );
    if( fAngle != 0.0 )
        aRotScale.rotate( -fAngle );

    bool bPop = false;
    bool bABold = false;
#if !defined USE_JAVA || !defined MACOSX
    // artificial bold necessary ?
    if( m_pReferenceDevice->mpFontEntry->maFontSelData.mpFontData->GetWeight() <= WEIGHT_MEDIUM &&
        m_pReferenceDevice->mpFontEntry->maFontSelData.GetWeight() > WEIGHT_MEDIUM )
    {
        if( ! bPop )
            aLine.append( "q " );
        bPop = true;
        bABold = true;
    }
#endif	 // !USE_JAVA || !MACOSX
    // setup text colors (if necessary)
    Color aStrokeColor( COL_TRANSPARENT );
    Color aNonStrokeColor( COL_TRANSPARENT );

    if( m_aCurrentPDFState.m_aFont.IsOutline() )
    {
        aStrokeColor = m_aCurrentPDFState.m_aFont.GetColor();
        aNonStrokeColor = Color( COL_WHITE );
    }
    else
        aNonStrokeColor = m_aCurrentPDFState.m_aFont.GetColor();
    if( bABold )
        aStrokeColor = m_aCurrentPDFState.m_aFont.GetColor();

    if( aStrokeColor != Color( COL_TRANSPARENT ) && aStrokeColor != m_aCurrentPDFState.m_aLineColor )
    {
        if( ! bPop )
            aLine.append( "q " );
        bPop = true;
        appendStrokingColor( aStrokeColor, aLine );
        aLine.append( "\n" );
    }
    if( aNonStrokeColor != Color( COL_TRANSPARENT ) && aNonStrokeColor != m_aCurrentPDFState.m_aFillColor )
    {
        if( ! bPop )
            aLine.append( "q " );
        bPop = true;
        appendNonStrokingColor( aNonStrokeColor, aLine );
        aLine.append( "\n" );
    }

    // begin text object
    aLine.append( "BT\n" );
    // outline attribute ?
    if( m_aCurrentPDFState.m_aFont.IsOutline() || bABold )
    {
        // set correct text mode, set stroke width
        aLine.append( "2 Tr " ); // fill, then stroke

        if( m_aCurrentPDFState.m_aFont.IsOutline() )
        {
            // unclear what to do in case of outline and artificial bold
            // for the time being outline wins
            aLine.append( "0.25 w \n" );
        }
        else
        {
            double fW = (double)m_aCurrentPDFState.m_aFont.GetHeight() / 30.0;
            m_aPages.back().appendMappedLength( fW, aLine );
            aLine.append ( " w\n" );
        }
    }

    FontMetric aRefDevFontMetric = m_pReferenceDevice->GetFontMetric();

    // collect the glyphs into a single array
    const int nTmpMaxGlyphs = rLayout.GetOrientation() ? 1 : nMaxGlyphs; // #i97991# temporary workaround for #i87686#
    std::vector< PDFGlyph > aGlyphs;
    aGlyphs.reserve( nTmpMaxGlyphs );
    // first get all the glyphs and register them; coordinates still in Pixel
    Point aGNGlyphPos;
    while( (nGlyphs = rLayout.GetNextGlyphs( nTmpMaxGlyphs, pGlyphs, aGNGlyphPos, nIndex, nAdvanceWidths, pCharPosAry )) != 0 )
    {
        for( int i = 0; i < nGlyphs; i++ )
        {
            pFallbackFonts[i] = rLayout.GetFallbackFontData( pGlyphs[i] );

            if( (pGlyphs[i] & GF_ISCHAR) )
                pUnicodes[i] = static_cast<sal_Ucs>(pGlyphs[i] & GF_IDXMASK);
            else if( pCharPosAry[i] >= nMinCharPos && pCharPosAry[i] <= nMaxCharPos )
            {
                pUnicodes[i] = rText.GetChar( sal::static_int_cast<xub_StrLen>(pCharPosAry[i]) );
                // #i36691# hack that is needed because currently the pGlyphs[]
                // argument is ignored for embeddable fonts and so the layout
                // engine's glyph work is ignored (i.e. char mirroring)
                // TODO: a real solution would be to map the layout engine's
                // glyphid (i.e. FreeType's synthetic glyphid for a Type1 font)
                // back to unicode and then to embeddable font's encoding
                if( getReferenceDevice()->GetLayoutMode() & TEXT_LAYOUT_BIDI_RTL )
                    pUnicodes[i] = static_cast<sal_Ucs>(GetMirroredChar(pUnicodes[i]));
            }
            else
                pUnicodes[i] = 0;
            // note: in case of ctl one character may result
            // in multiple glyphs. The current SalLayout
            // implementations set -1 then to indicate that no direct
            // mapping is possible
        }

#if defined USE_JAVA && defined MACOSX
        registerGlyphs( nGlyphs, pGlyphs, pUnicodes, pMappedGlyphs, pMappedIdentityGlyphs, pMappedFontObjects, pMappedFontSubObjects, pFallbackFonts );
        if ( !isReplayWriter() )
            continue;

        sal_Int32 nTotalAdvance = 0;
#else	// USE_JAVA && MACOSX
        registerGlyphs( nGlyphs, pGlyphs, pGlyphWidths, pUnicodes, pMappedGlyphs, pMappedFontObjects, pFallbackFonts );
#endif	// USE_JAVA && MACOSX

        for( int i = 0; i < nGlyphs; i++ )
        {
#if defined USE_JAVA && defined MACOSX
            // Use the same units for glyph width as is used for position
            pGlyphWidths[i] = ( ( nAdvanceWidths[i] + nTotalAdvance ) / rLayout.GetUnitsPerPixel() ) - ( nTotalAdvance / rLayout.GetUnitsPerPixel() );
            nTotalAdvance += nAdvanceWidths[i];

            // Fix bugs 3348 and 3442 by fetching each glyph's actual
            // native unkerned width
            SalATSLayout *pATSLayout = NULL;
            MultiSalLayout *pMultiLayout = dynamic_cast<MultiSalLayout*>( &rLayout );
            if ( pMultiLayout )
                pATSLayout = dynamic_cast<SalATSLayout*>( pMultiLayout->GetLayout( ( pGlyphs[i] & GF_FONTMASK ) >> GF_FONTSHIFT ) );
            if ( !pATSLayout )
                pATSLayout = dynamic_cast<SalATSLayout*>( &rLayout );
            sal_Int32 nNativeGlyphWidth = ( pATSLayout ? sal_Int32( ( (double)pATSLayout->GetNativeGlyphWidth( pGlyphs[i], pCharPosAry[i] ) / pATSLayout->GetUnitsPerPixel() ) + 0.5 ) : 0.0 );

            // Do not allow invalid glyphs to be written to the PDF output
            if( ! ( pGlyphs[i] & ( GF_ISCHAR | GF_GSUB ) ) )
#endif	// USE_JAVA && MACOSX
            aGlyphs.push_back( PDFGlyph( aGNGlyphPos,
                                         pGlyphWidths[i],
                                         pGlyphs[i],
                                         pMappedFontObjects[i],
#if defined USE_JAVA && defined MACOSX
                                         pMappedGlyphs[i],
                                         pMappedFontSubObjects[i],
                                         pMappedIdentityGlyphs[i],
                                         pCharPosAry[i],
                                         nNativeGlyphWidth,
                                         &rLayout ) );
            if( bVertical )
                aGNGlyphPos.Y() += pGlyphWidths[i];
            else
                aGNGlyphPos.X() += pGlyphWidths[i];
#else	// USE_JAVA && MACOSX
                                         pMappedGlyphs[i] ) );
            if( bVertical )
                aGNGlyphPos.Y() += nAdvanceWidths[i]/rLayout.GetUnitsPerPixel();
            else
                aGNGlyphPos.X() += nAdvanceWidths[i]/rLayout.GetUnitsPerPixel();
#endif	// USE_JAVA && MACOSX
        }
    }

    Point aAlignOffset;
    if ( eAlign == ALIGN_BOTTOM )
        aAlignOffset.Y() -= aRefDevFontMetric.GetDescent();
    else if ( eAlign == ALIGN_TOP )
        aAlignOffset.Y() += aRefDevFontMetric.GetAscent();
    if( aAlignOffset.X() || aAlignOffset.Y() )
        aAlignOffset = aRotScale.transform( aAlignOffset );

    if( bVertical )
        drawVerticalGlyphs( aGlyphs, aLine, aAlignOffset, aRotScale, fAngle, fXScale, fSkew, nFontHeight );
    else
        drawHorizontalGlyphs( aGlyphs, aLine, aAlignOffset, fAngle, fXScale, fSkew, nFontHeight, nPixelFontHeight );

    // end textobject
    aLine.append( "ET\n" );
    if( bPop )
        aLine.append( "Q\n" );

    writeBuffer( aLine.getStr(), aLine.getLength() );

    // draw eventual textlines
    FontStrikeout eStrikeout = m_aCurrentPDFState.m_aFont.GetStrikeout();
    FontUnderline eUnderline = m_aCurrentPDFState.m_aFont.GetUnderline();
    FontUnderline eOverline  = m_aCurrentPDFState.m_aFont.GetOverline();
    if( bTextLines &&
        (
         ( eUnderline != UNDERLINE_NONE && eUnderline != UNDERLINE_DONTKNOW ) ||
         ( eOverline  != UNDERLINE_NONE && eOverline  != UNDERLINE_DONTKNOW ) ||
         ( eStrikeout != STRIKEOUT_NONE && eStrikeout != STRIKEOUT_DONTKNOW )
         )
        )
    {
        BOOL bUnderlineAbove = OutputDevice::ImplIsUnderlineAbove( m_aCurrentPDFState.m_aFont );
        if( m_aCurrentPDFState.m_aFont.IsWordLineMode() )
        {
            Point aPos, aStartPt;
            sal_Int32 nWidth = 0, nAdvance=0;
            for( int nStart = 0;;)
            {
                sal_GlyphId nGlyphIndex;
                if( !rLayout.GetNextGlyphs( 1, &nGlyphIndex, aPos, nStart, &nAdvance ) )
                    break;

                if( !rLayout.IsSpacingGlyph( nGlyphIndex ) )
                {
                    if( !nWidth )
                        aStartPt = aPos;

                    nWidth += nAdvance;
                }
                else if( nWidth > 0 )
                {
                    drawTextLine( m_pReferenceDevice->PixelToLogic( aStartPt ),
                                  m_pReferenceDevice->ImplDevicePixelToLogicWidth( nWidth ),
                                  eStrikeout, eUnderline, eOverline, bUnderlineAbove );
                    nWidth = 0;
                }
            }

            if( nWidth > 0 )
            {
                drawTextLine( m_pReferenceDevice->PixelToLogic( aStartPt ),
                              m_pReferenceDevice->ImplDevicePixelToLogicWidth( nWidth ),
                              eStrikeout, eUnderline, eOverline, bUnderlineAbove );
            }
        }
        else
        {
            Point aStartPt = rLayout.GetDrawPosition();
            int nWidth = rLayout.GetTextWidth() / rLayout.GetUnitsPerPixel();
            drawTextLine( m_pReferenceDevice->PixelToLogic( aStartPt ),
                          m_pReferenceDevice->ImplDevicePixelToLogicWidth( nWidth ),
                          eStrikeout, eUnderline, eOverline, bUnderlineAbove );
        }
    }

    // write eventual emphasis marks
    if( m_aCurrentPDFState.m_aFont.GetEmphasisMark() & EMPHASISMARK_STYLE )
    {
        PolyPolygon 			aEmphPoly;
        Rectangle				aEmphRect1;
        Rectangle				aEmphRect2;
        long					nEmphYOff;
        long					nEmphWidth;
        long					nEmphHeight;
        BOOL					bEmphPolyLine;
        FontEmphasisMark		nEmphMark;

        push( PUSH_ALL );

        aLine.setLength( 0 );
        aLine.append( "q\n" );

        nEmphMark = m_pReferenceDevice->ImplGetEmphasisMarkStyle( m_aCurrentPDFState.m_aFont );
        if ( nEmphMark & EMPHASISMARK_POS_BELOW )
            nEmphHeight = m_pReferenceDevice->mnEmphasisDescent;
        else
            nEmphHeight = m_pReferenceDevice->mnEmphasisAscent;
        m_pReferenceDevice->ImplGetEmphasisMark( aEmphPoly,
                                                 bEmphPolyLine,
                                                 aEmphRect1,
                                                 aEmphRect2,
                                                 nEmphYOff,
                                                 nEmphWidth,
                                                 nEmphMark,
                                                 m_pReferenceDevice->ImplDevicePixelToLogicWidth(nEmphHeight),
                                                 m_pReferenceDevice->mpFontEntry->mnOrientation );
        if ( bEmphPolyLine )
        {
            setLineColor( m_aCurrentPDFState.m_aFont.GetColor() );
            setFillColor( Color( COL_TRANSPARENT ) );
        }
        else
        {
            setFillColor( m_aCurrentPDFState.m_aFont.GetColor() );
            setLineColor( Color( COL_TRANSPARENT ) );
        }
        writeBuffer( aLine.getStr(), aLine.getLength() );

        Point aOffset = Point(0,0);

        if ( nEmphMark & EMPHASISMARK_POS_BELOW )
            aOffset.Y() += m_pReferenceDevice->mpFontEntry->maMetric.mnDescent + nEmphYOff;
        else
            aOffset.Y() -= m_pReferenceDevice->mpFontEntry->maMetric.mnAscent + nEmphYOff;

        long nEmphWidth2     = nEmphWidth / 2;
        long nEmphHeight2    = nEmphHeight / 2;
        aOffset += Point( nEmphWidth2, nEmphHeight2 );

        if ( eAlign == ALIGN_BOTTOM )
            aOffset.Y() -= m_pReferenceDevice->mpFontEntry->maMetric.mnDescent;
        else if ( eAlign == ALIGN_TOP )
            aOffset.Y() += m_pReferenceDevice->mpFontEntry->maMetric.mnAscent;

        for( int nStart = 0;;)
        {
            Point aPos;
            sal_GlyphId nGlyphIndex;
            sal_Int32 nAdvance;
            if( !rLayout.GetNextGlyphs( 1, &nGlyphIndex, aPos, nStart, &nAdvance ) )
                break;

            if( !rLayout.IsSpacingGlyph( nGlyphIndex ) )
            {
                Point aAdjOffset = aOffset;
                aAdjOffset.X() += (nAdvance - nEmphWidth) / 2;
                aAdjOffset = aRotScale.transform( aAdjOffset );

                aAdjOffset -= Point( nEmphWidth2, nEmphHeight2 );

                aPos += aAdjOffset;
                aPos = m_pReferenceDevice->PixelToLogic( aPos );
                drawEmphasisMark( aPos.X(), aPos.Y(),
                                  aEmphPoly, bEmphPolyLine,
                                  aEmphRect1, aEmphRect2 );
            }
        }

        writeBuffer( "Q\n", 2 );
        pop();
    }

}

void PDFWriterImpl::drawEmphasisMark( long nX, long nY,
                                      const PolyPolygon& rPolyPoly, BOOL bPolyLine,
                                      const Rectangle& rRect1, const Rectangle& rRect2 )
{
    // TODO: pass nWidth as width of this mark
    // long nWidth = 0;

    if ( rPolyPoly.Count() )
    {
        if ( bPolyLine )
        {
            Polygon aPoly = rPolyPoly.GetObject( 0 );
            aPoly.Move( nX, nY );
            drawPolyLine( aPoly );
        }
        else
        {
            PolyPolygon aPolyPoly = rPolyPoly;
            aPolyPoly.Move( nX, nY );
            drawPolyPolygon( aPolyPoly );
        }
    }

    if ( !rRect1.IsEmpty() )
    {
        Rectangle aRect( Point( nX+rRect1.Left(),
                                nY+rRect1.Top() ), rRect1.GetSize() );
        drawRectangle( aRect );
    }

    if ( !rRect2.IsEmpty() )
    {
        Rectangle aRect( Point( nX+rRect2.Left(),
                                nY+rRect2.Top() ), rRect2.GetSize() );

        drawRectangle( aRect );
    }
}

void PDFWriterImpl::drawText( const Point& rPos, const String& rText, xub_StrLen nIndex, xub_StrLen nLen, bool bTextLines )
{
    MARK( "drawText" );

    updateGraphicsState();

    // get a layout from the OuputDevice's SalGraphics
    // this also enforces font substitution and sets the font on SalGraphics
    SalLayout* pLayout = m_pReferenceDevice->ImplLayout( rText, nIndex, nLen, rPos );
    if( pLayout )
    {
        drawLayout( *pLayout, rText, bTextLines );
        pLayout->Release();
    }
}

void PDFWriterImpl::drawTextArray( const Point& rPos, const String& rText, const sal_Int32* pDXArray, xub_StrLen nIndex, xub_StrLen nLen, bool bTextLines )
{
    MARK( "drawText with array" );

    updateGraphicsState();

    // get a layout from the OuputDevice's SalGraphics
    // this also enforces font substitution and sets the font on SalGraphics
    SalLayout* pLayout = m_pReferenceDevice->ImplLayout( rText, nIndex, nLen, rPos, 0, pDXArray );
    if( pLayout )
    {
        drawLayout( *pLayout, rText, bTextLines );
        pLayout->Release();
    }
}

void PDFWriterImpl::drawStretchText( const Point& rPos, ULONG nWidth, const String& rText, xub_StrLen nIndex, xub_StrLen nLen, bool bTextLines )
{
    MARK( "drawStretchText" );

    updateGraphicsState();

    // get a layout from the OuputDevice's SalGraphics
    // this also enforces font substitution and sets the font on SalGraphics
    SalLayout* pLayout = m_pReferenceDevice->ImplLayout( rText, nIndex, nLen, rPos, nWidth );
    if( pLayout )
    {
        drawLayout( *pLayout, rText, bTextLines );
        pLayout->Release();
    }
}

void PDFWriterImpl::drawText( const Rectangle& rRect, const String& rOrigStr, USHORT nStyle, bool bTextLines )
{
    long        nWidth          = rRect.GetWidth();
    long        nHeight         = rRect.GetHeight();

    if ( nWidth <= 0 || nHeight <= 0 )
        return;

    MARK( "drawText with rectangle" );

    updateGraphicsState();

    // clip with rectangle
    OStringBuffer aLine;
    aLine.append( "q " );
    m_aPages.back().appendRect( rRect, aLine );
    aLine.append( " W* n\n" );
    writeBuffer( aLine.getStr(), aLine.getLength() );

    // if disabled text is needed, put in here

    Point       aPos            = rRect.TopLeft();

    long		nTextHeight		= m_pReferenceDevice->GetTextHeight();
    xub_StrLen  nMnemonicPos    = STRING_NOTFOUND;

    String aStr = rOrigStr;
    if ( nStyle & TEXT_DRAW_MNEMONIC )
        aStr = m_pReferenceDevice->GetNonMnemonicString( aStr, nMnemonicPos );

    // multiline text
    if ( nStyle & TEXT_DRAW_MULTILINE )
    {
        XubString               aLastLine;
        ImplMultiTextLineInfo   aMultiLineInfo;
        ImplTextLineInfo*       pLineInfo;
        long                    nMaxTextWidth;
        xub_StrLen              i;
        xub_StrLen              nLines;
        xub_StrLen              nFormatLines;

        if ( nTextHeight )
        {
            nMaxTextWidth = m_pReferenceDevice->ImplGetTextLines( aMultiLineInfo, nWidth, aStr, nStyle );
            nLines = (xub_StrLen)(nHeight/nTextHeight);
            nFormatLines = aMultiLineInfo.Count();
            if ( !nLines )
                nLines = 1;
            if ( nFormatLines > nLines )
            {
                if ( nStyle & TEXT_DRAW_ENDELLIPSIS )
                {
                    // handle last line
                    nFormatLines = nLines-1;

                    pLineInfo = aMultiLineInfo.GetLine( nFormatLines );
                    aLastLine = aStr.Copy( pLineInfo->GetIndex() );
                    aLastLine.ConvertLineEnd( LINEEND_LF );
                    // replace line feed by space
                    xub_StrLen nLastLineLen = aLastLine.Len();
                    for ( i = 0; i < nLastLineLen; i++ )
                    {
                        if ( aLastLine.GetChar( i ) == _LF )
                            aLastLine.SetChar( i, ' ' );
                    }
                    aLastLine = m_pReferenceDevice->GetEllipsisString( aLastLine, nWidth, nStyle );
                    nStyle &= ~(TEXT_DRAW_VCENTER | TEXT_DRAW_BOTTOM);
                    nStyle |= TEXT_DRAW_TOP;
                }
            }

            // vertical alignment
            if ( nStyle & TEXT_DRAW_BOTTOM )
                aPos.Y() += nHeight-(nFormatLines*nTextHeight);
            else if ( nStyle & TEXT_DRAW_VCENTER )
                aPos.Y() += (nHeight-(nFormatLines*nTextHeight))/2;

            // draw all lines excluding the last
            for ( i = 0; i < nFormatLines; i++ )
            {
                pLineInfo = aMultiLineInfo.GetLine( i );
                if ( nStyle & TEXT_DRAW_RIGHT )
                    aPos.X() += nWidth-pLineInfo->GetWidth();
                else if ( nStyle & TEXT_DRAW_CENTER )
                    aPos.X() += (nWidth-pLineInfo->GetWidth())/2;
                xub_StrLen nIndex   = pLineInfo->GetIndex();
                xub_StrLen nLineLen = pLineInfo->GetLen();
                drawText( aPos, aStr, nIndex, nLineLen, bTextLines );
                // mnemonics should not appear in documents,
                // if the need arises, put them in here
                aPos.Y() += nTextHeight;
                aPos.X() = rRect.Left();
            }


            // output last line left adjusted since it was shortened
            if ( aLastLine.Len() )
                drawText( aPos, aLastLine, 0, STRING_LEN, bTextLines );
        }
    }
    else
    {
        long nTextWidth = m_pReferenceDevice->GetTextWidth( aStr );

        // Evt. Text kuerzen
        if ( nTextWidth > nWidth )
        {
            if ( nStyle & (TEXT_DRAW_ENDELLIPSIS | TEXT_DRAW_PATHELLIPSIS | TEXT_DRAW_NEWSELLIPSIS) )
            {
                aStr = m_pReferenceDevice->GetEllipsisString( aStr, nWidth, nStyle );
                nStyle &= ~(TEXT_DRAW_CENTER | TEXT_DRAW_RIGHT);
                nStyle |= TEXT_DRAW_LEFT;
                nTextWidth = m_pReferenceDevice->GetTextWidth( aStr );
            }
        }

        // vertical alignment
        if ( nStyle & TEXT_DRAW_RIGHT )
            aPos.X() += nWidth-nTextWidth;
        else if ( nStyle & TEXT_DRAW_CENTER )
            aPos.X() += (nWidth-nTextWidth)/2;

        if ( nStyle & TEXT_DRAW_BOTTOM )
            aPos.Y() += nHeight-nTextHeight;
        else if ( nStyle & TEXT_DRAW_VCENTER )
            aPos.Y() += (nHeight-nTextHeight)/2;

        // mnemonics should be inserted here if the need arises

        // draw the actual text
        drawText( aPos, aStr, 0, STRING_LEN, bTextLines );
    }

    // reset clip region to original value
    aLine.setLength( 0 );
    aLine.append( "Q\n" );
    writeBuffer( aLine.getStr(), aLine.getLength() );
}

void PDFWriterImpl::drawLine( const Point& rStart, const Point& rStop )
{
    MARK( "drawLine" );

    updateGraphicsState();

    if( m_aGraphicsStack.front().m_aLineColor == Color( COL_TRANSPARENT ) )
        return;

    OStringBuffer aLine;
    m_aPages.back().appendPoint( rStart, aLine );
    aLine.append( " m " );
    m_aPages.back().appendPoint( rStop, aLine );
    aLine.append( " l S\n" );

    writeBuffer( aLine.getStr(), aLine.getLength() );
}

void PDFWriterImpl::drawLine( const Point& rStart, const Point& rStop, const LineInfo& rInfo )
{
    MARK( "drawLine with LineInfo" );
    updateGraphicsState();

    if( m_aGraphicsStack.front().m_aLineColor == Color( COL_TRANSPARENT ) )
        return;

    if( rInfo.GetStyle() == LINE_SOLID && rInfo.GetWidth() < 2 )
    {
        drawLine( rStart, rStop );
        return;
    }

    OStringBuffer aLine;

    aLine.append( "q " );
    if( m_aPages.back().appendLineInfo( rInfo, aLine ) )
    {
        m_aPages.back().appendPoint( rStart, aLine );
        aLine.append( " m " );
        m_aPages.back().appendPoint( rStop, aLine );
        aLine.append( " l S Q\n" );

        writeBuffer( aLine.getStr(), aLine.getLength() );
    }
    else
    {
        PDFWriter::ExtLineInfo aInfo;
        convertLineInfoToExtLineInfo( rInfo, aInfo );
        Point aPolyPoints[2] = { rStart, rStop };
        Polygon aPoly( 2, aPolyPoints );
        drawPolyLine( aPoly, aInfo );
    }
}

void PDFWriterImpl::drawWaveLine( const Point& rStart, const Point& rStop, sal_Int32 nDelta, sal_Int32 nLineWidth )
{
    Point aDiff( rStop-rStart );
    double fLen = sqrt( (double)(aDiff.X()*aDiff.X() + aDiff.Y()*aDiff.Y()) );
    if( fLen < 1.0 )
        return;

    MARK( "drawWaveLine" );
    updateGraphicsState();

    if( m_aGraphicsStack.front().m_aLineColor == Color( COL_TRANSPARENT ) )
        return;

    OStringBuffer aLine( 512 );
    aLine.append( "q " );
    m_aPages.back().appendMappedLength( nLineWidth, aLine, true );
    aLine.append( " w " );

    appendDouble( (double)aDiff.X()/fLen, aLine );
    aLine.append( ' ' );
    appendDouble( -(double)aDiff.Y()/fLen, aLine );
    aLine.append( ' ' );
    appendDouble( (double)aDiff.Y()/fLen, aLine );
    aLine.append( ' ' );
    appendDouble( (double)aDiff.X()/fLen, aLine );
    aLine.append( ' ' );
    m_aPages.back().appendPoint( rStart, aLine );
    aLine.append( " cm " );
    m_aPages.back().appendWaveLine( (sal_Int32)fLen, 0, nDelta, aLine );
    aLine.append( "Q\n" );
    writeBuffer( aLine.getStr(), aLine.getLength() );
}

#define WCONV( x ) m_pReferenceDevice->ImplDevicePixelToLogicWidth( x )
#define HCONV( x ) m_pReferenceDevice->ImplDevicePixelToLogicHeight( x )

void PDFWriterImpl::drawWaveTextLine( OStringBuffer& aLine, long nWidth, FontUnderline eTextLine, Color aColor, bool bIsAbove )
{
    // note: units in pFontEntry are ref device pixel
    ImplFontEntry*	pFontEntry = m_pReferenceDevice->mpFontEntry;
    long			nLineHeight = 0;
    long			nLinePos = 0;

    appendStrokingColor( aColor, aLine );
    aLine.append( "\n" );

    if ( bIsAbove )
    {
        if ( !pFontEntry->maMetric.mnAboveWUnderlineSize )
            m_pReferenceDevice->ImplInitAboveTextLineSize();
        nLineHeight = HCONV( pFontEntry->maMetric.mnAboveWUnderlineSize );
        nLinePos = HCONV( pFontEntry->maMetric.mnAboveWUnderlineOffset );
    }
    else
    {
        if ( !pFontEntry->maMetric.mnWUnderlineSize )
            m_pReferenceDevice->ImplInitTextLineSize();
        nLineHeight = HCONV( pFontEntry->maMetric.mnWUnderlineSize );
        nLinePos = HCONV( pFontEntry->maMetric.mnWUnderlineOffset );
    }
    if ( (eTextLine == UNDERLINE_SMALLWAVE) && (nLineHeight > 3) )
        nLineHeight = 3;

    long nLineWidth = getReferenceDevice()->mnDPIX/450;
    if ( ! nLineWidth )
        nLineWidth = 1;

    if ( eTextLine == UNDERLINE_BOLDWAVE )
        nLineWidth = 3*nLineWidth;

    m_aPages.back().appendMappedLength( (sal_Int32)nLineWidth, aLine );
    aLine.append( " w " );

    if ( eTextLine == UNDERLINE_DOUBLEWAVE )
    {
        long nOrgLineHeight = nLineHeight;
        nLineHeight /= 3;
        if ( nLineHeight < 2 )
        {
            if ( nOrgLineHeight > 1 )
                nLineHeight = 2;
            else
                nLineHeight = 1;
        }
        long nLineDY = nOrgLineHeight-(nLineHeight*2);
        if ( nLineDY < nLineWidth )
            nLineDY = nLineWidth;
        long nLineDY2 = nLineDY/2;
        if ( !nLineDY2 )
            nLineDY2 = 1;

        nLinePos -= nLineWidth-nLineDY2;

        m_aPages.back().appendWaveLine( nWidth, -nLinePos, 2*nLineHeight, aLine );

        nLinePos += nLineWidth+nLineDY;
        m_aPages.back().appendWaveLine( nWidth, -nLinePos, 2*nLineHeight, aLine );
    }
    else
    {
        if ( eTextLine != UNDERLINE_BOLDWAVE )
            nLinePos -= nLineWidth/2;
        m_aPages.back().appendWaveLine( nWidth, -nLinePos, nLineHeight, aLine );
    }
}

void PDFWriterImpl::drawStraightTextLine( OStringBuffer& aLine, long nWidth, FontUnderline eTextLine, Color aColor, bool bIsAbove )
{
    // note: units in pFontEntry are ref device pixel
    ImplFontEntry*	pFontEntry = m_pReferenceDevice->mpFontEntry;
    long			nLineHeight = 0;
    long			nLinePos  = 0;
    long			nLinePos2 = 0;

    if ( eTextLine > UNDERLINE_BOLDWAVE )
        eTextLine = UNDERLINE_SINGLE;

    switch ( eTextLine )
    {
        case UNDERLINE_SINGLE:
        case UNDERLINE_DOTTED:
        case UNDERLINE_DASH:
        case UNDERLINE_LONGDASH:
        case UNDERLINE_DASHDOT:
        case UNDERLINE_DASHDOTDOT:
            if ( bIsAbove )
            {
                if ( !pFontEntry->maMetric.mnAboveUnderlineSize )
                    m_pReferenceDevice->ImplInitAboveTextLineSize();
                nLineHeight = HCONV( pFontEntry->maMetric.mnAboveUnderlineSize );
                nLinePos    = HCONV( pFontEntry->maMetric.mnAboveUnderlineOffset );
            }
            else
            {
                if ( !pFontEntry->maMetric.mnUnderlineSize )
                    m_pReferenceDevice->ImplInitTextLineSize();
                nLineHeight = HCONV( pFontEntry->maMetric.mnUnderlineSize );
                nLinePos    = HCONV( pFontEntry->maMetric.mnUnderlineOffset );
            }
            break;
        case UNDERLINE_BOLD:
        case UNDERLINE_BOLDDOTTED:
        case UNDERLINE_BOLDDASH:
        case UNDERLINE_BOLDLONGDASH:
        case UNDERLINE_BOLDDASHDOT:
        case UNDERLINE_BOLDDASHDOTDOT:
            if ( bIsAbove )
            {
                if ( !pFontEntry->maMetric.mnAboveBUnderlineSize )
                    m_pReferenceDevice->ImplInitAboveTextLineSize();
                nLineHeight = HCONV( pFontEntry->maMetric.mnAboveBUnderlineSize );
                nLinePos    = HCONV( pFontEntry->maMetric.mnAboveBUnderlineOffset );
            }
            else
            {
                if ( !pFontEntry->maMetric.mnBUnderlineSize )
                    m_pReferenceDevice->ImplInitTextLineSize();
                nLineHeight = HCONV( pFontEntry->maMetric.mnBUnderlineSize );
                nLinePos    = HCONV( pFontEntry->maMetric.mnBUnderlineOffset );
                nLinePos += nLineHeight/2;
            }
            break;
        case UNDERLINE_DOUBLE:
            if ( bIsAbove )
            {
                if ( !pFontEntry->maMetric.mnAboveDUnderlineSize )
                    m_pReferenceDevice->ImplInitAboveTextLineSize();
                nLineHeight = HCONV( pFontEntry->maMetric.mnAboveDUnderlineSize );
                nLinePos    = HCONV( pFontEntry->maMetric.mnAboveDUnderlineOffset1 );
                nLinePos2   = HCONV( pFontEntry->maMetric.mnAboveDUnderlineOffset2 );
            }
            else
            {
                if ( !pFontEntry->maMetric.mnDUnderlineSize )
                    m_pReferenceDevice->ImplInitTextLineSize();
                nLineHeight = HCONV( pFontEntry->maMetric.mnDUnderlineSize );
                nLinePos    = HCONV( pFontEntry->maMetric.mnDUnderlineOffset1 );
                nLinePos2   = HCONV( pFontEntry->maMetric.mnDUnderlineOffset2 );
            }
        default:
            break;
    }

    if ( nLineHeight )
    {
        m_aPages.back().appendMappedLength( (sal_Int32)nLineHeight, aLine, true );
        aLine.append( " w " );
        appendStrokingColor( aColor, aLine );
        aLine.append( "\n" );

        switch ( eTextLine )
        {
            case UNDERLINE_DOTTED:
            case UNDERLINE_BOLDDOTTED:
                aLine.append( "[ " );
                m_aPages.back().appendMappedLength( (sal_Int32)nLineHeight, aLine, false );
                aLine.append( " ] 0 d\n" );
                break;
            case UNDERLINE_DASH:
            case UNDERLINE_LONGDASH:
            case UNDERLINE_BOLDDASH:
            case UNDERLINE_BOLDLONGDASH:
                {
                    sal_Int32 nDashLength = 4*nLineHeight;
                    sal_Int32 nVoidLength = 2*nLineHeight;
                    if ( ( eTextLine == UNDERLINE_LONGDASH ) || ( eTextLine == UNDERLINE_BOLDLONGDASH ) )
                        nDashLength = 8*nLineHeight;

                    aLine.append( "[ " );
                    m_aPages.back().appendMappedLength( nDashLength, aLine, false );
                    aLine.append( ' ' );
                    m_aPages.back().appendMappedLength( nVoidLength, aLine, false );
                    aLine.append( " ] 0 d\n" );
                }
                break;
            case UNDERLINE_DASHDOT:
            case UNDERLINE_BOLDDASHDOT:
                {
                    sal_Int32 nDashLength = 4*nLineHeight;
                    sal_Int32 nVoidLength = 2*nLineHeight;
                    aLine.append( "[ " );
                    m_aPages.back().appendMappedLength( nDashLength, aLine, false );
                    aLine.append( ' ' );
                    m_aPages.back().appendMappedLength( nVoidLength, aLine, false );
                    aLine.append( ' ' );
                    m_aPages.back().appendMappedLength( (sal_Int32)nLineHeight, aLine, false );
                    aLine.append( ' ' );
                    m_aPages.back().appendMappedLength( nVoidLength, aLine, false );
                    aLine.append( " ] 0 d\n" );
                }
                break;
            case UNDERLINE_DASHDOTDOT:
            case UNDERLINE_BOLDDASHDOTDOT:
                {
                    sal_Int32 nDashLength = 4*nLineHeight;
                    sal_Int32 nVoidLength = 2*nLineHeight;
                    aLine.append( "[ " );
                    m_aPages.back().appendMappedLength( nDashLength, aLine, false );
                    aLine.append( ' ' );
                    m_aPages.back().appendMappedLength( nVoidLength, aLine, false );
                    aLine.append( ' ' );
                    m_aPages.back().appendMappedLength( (sal_Int32)nLineHeight, aLine, false );
                    aLine.append( ' ' );
                    m_aPages.back().appendMappedLength( nVoidLength, aLine, false );
                    aLine.append( ' ' );
                    m_aPages.back().appendMappedLength( (sal_Int32)nLineHeight, aLine, false );
                    aLine.append( ' ' );
                    m_aPages.back().appendMappedLength( nVoidLength, aLine, false );
                    aLine.append( " ] 0 d\n" );
                }
                break;
            default:
                break;
        }

        aLine.append( "0 " );
        m_aPages.back().appendMappedLength( (sal_Int32)(-nLinePos), aLine, true );
        aLine.append( " m " );
        m_aPages.back().appendMappedLength( (sal_Int32)nWidth, aLine, false );
        aLine.append( ' ' );
        m_aPages.back().appendMappedLength( (sal_Int32)(-nLinePos), aLine, true );
        aLine.append( " l S\n" );
        if ( eTextLine == UNDERLINE_DOUBLE )
        {
            aLine.append( "0 " );
            m_aPages.back().appendMappedLength( (sal_Int32)(-nLinePos2-nLineHeight), aLine, true );
            aLine.append( " m " );
            m_aPages.back().appendMappedLength( (sal_Int32)nWidth, aLine, false );
            aLine.append( ' ' );
            m_aPages.back().appendMappedLength( (sal_Int32)(-nLinePos2-nLineHeight), aLine, true );
            aLine.append( " l S\n" );
        }
    }
}

void PDFWriterImpl::drawStrikeoutLine( OStringBuffer& aLine, long nWidth, FontStrikeout eStrikeout, Color aColor )
{
    // note: units in pFontEntry are ref device pixel
    ImplFontEntry*	pFontEntry = m_pReferenceDevice->mpFontEntry;
    long			nLineHeight = 0;
    long			nLinePos  = 0;
    long			nLinePos2 = 0;

    if ( eStrikeout > STRIKEOUT_X )
        eStrikeout = STRIKEOUT_SINGLE;

    switch ( eStrikeout )
    {
        case STRIKEOUT_SINGLE:
            if ( !pFontEntry->maMetric.mnStrikeoutSize )
                m_pReferenceDevice->ImplInitTextLineSize();
            nLineHeight = HCONV( pFontEntry->maMetric.mnStrikeoutSize );
            nLinePos    = HCONV( pFontEntry->maMetric.mnStrikeoutOffset );
            break;
        case STRIKEOUT_BOLD:
            if ( !pFontEntry->maMetric.mnBStrikeoutSize )
                m_pReferenceDevice->ImplInitTextLineSize();
            nLineHeight = HCONV( pFontEntry->maMetric.mnBStrikeoutSize );
            nLinePos    = HCONV( pFontEntry->maMetric.mnBStrikeoutOffset );
            break;
        case STRIKEOUT_DOUBLE:
            if ( !pFontEntry->maMetric.mnDStrikeoutSize )
                m_pReferenceDevice->ImplInitTextLineSize();
            nLineHeight = HCONV( pFontEntry->maMetric.mnDStrikeoutSize );
            nLinePos    = HCONV( pFontEntry->maMetric.mnDStrikeoutOffset1 );
            nLinePos2   = HCONV( pFontEntry->maMetric.mnDStrikeoutOffset2 );
            break;
        default:
            break;
    }

    if ( nLineHeight )
    {
        m_aPages.back().appendMappedLength( (sal_Int32)nLineHeight, aLine, true );
        aLine.append( " w " );
        appendStrokingColor( aColor, aLine );
        aLine.append( "\n" );

        aLine.append( "0 " );
        m_aPages.back().appendMappedLength( (sal_Int32)(-nLinePos), aLine, true );
        aLine.append( " m " );
        m_aPages.back().appendMappedLength( (sal_Int32)nWidth, aLine, true );
        aLine.append( ' ' );
        m_aPages.back().appendMappedLength( (sal_Int32)(-nLinePos), aLine, true );
        aLine.append( " l S\n" );

        if ( eStrikeout == STRIKEOUT_DOUBLE )
        {
            aLine.append( "0 " );
            m_aPages.back().appendMappedLength( (sal_Int32)(-nLinePos2-nLineHeight), aLine, true );
            aLine.append( " m " );
            m_aPages.back().appendMappedLength( (sal_Int32)nWidth, aLine, true );
            aLine.append( ' ' );
            m_aPages.back().appendMappedLength( (sal_Int32)(-nLinePos2-nLineHeight), aLine, true );
            aLine.append( " l S\n" );
        }
    }
}

void PDFWriterImpl::drawStrikeoutChar( const Point& rPos, long nWidth, FontStrikeout eStrikeout )
{
    String aStrikeoutChar = String::CreateFromAscii( eStrikeout == STRIKEOUT_SLASH ? "/" : "X" );
    String aStrikeout = aStrikeoutChar;
    while( m_pReferenceDevice->GetTextWidth( aStrikeout ) < nWidth )
        aStrikeout.Append( aStrikeout );

    // do not get broader than nWidth modulo 1 character
    while( m_pReferenceDevice->GetTextWidth( aStrikeout ) >= nWidth )
        aStrikeout.Erase( 0, 1 );
    aStrikeout.Append( aStrikeoutChar );
    BOOL bShadow = m_aCurrentPDFState.m_aFont.IsShadow();
    if ( bShadow )
    {
        Font aFont = m_aCurrentPDFState.m_aFont;
        aFont.SetShadow( FALSE );
        setFont( aFont );
        updateGraphicsState();
    }

    // strikeout string is left aligned non-CTL text
    ULONG nOrigTLM = m_pReferenceDevice->GetLayoutMode();
    m_pReferenceDevice->SetLayoutMode( TEXT_LAYOUT_BIDI_STRONG|TEXT_LAYOUT_COMPLEX_DISABLED );
    drawText( rPos, aStrikeout, 0, aStrikeout.Len(), false );
    m_pReferenceDevice->SetLayoutMode( nOrigTLM );

    if ( bShadow )
    {
        Font aFont = m_aCurrentPDFState.m_aFont;
        aFont.SetShadow( TRUE );
        setFont( aFont );
        updateGraphicsState();
    }
}

void PDFWriterImpl::drawTextLine( const Point& rPos, long nWidth, FontStrikeout eStrikeout, FontUnderline eUnderline, FontUnderline eOverline, bool bUnderlineAbove )
{
    if ( !nWidth ||
         ( ((eStrikeout == STRIKEOUT_NONE)||(eStrikeout == STRIKEOUT_DONTKNOW)) &&
           ((eUnderline == UNDERLINE_NONE)||(eUnderline == UNDERLINE_DONTKNOW)) &&
           ((eOverline  == UNDERLINE_NONE)||(eOverline  == UNDERLINE_DONTKNOW)) ) )
        return;

    MARK( "drawTextLine" );
    updateGraphicsState();

    // note: units in pFontEntry are ref device pixel
    ImplFontEntry*	pFontEntry = m_pReferenceDevice->mpFontEntry;
    Color			aUnderlineColor = m_aCurrentPDFState.m_aTextLineColor;
    Color			aOverlineColor  = m_aCurrentPDFState.m_aOverlineColor;
    Color			aStrikeoutColor = m_aCurrentPDFState.m_aFont.GetColor();
    bool			bStrikeoutDone = false;
    bool			bUnderlineDone = false;
    bool			bOverlineDone  = false;

    if ( (eStrikeout == STRIKEOUT_SLASH) || (eStrikeout == STRIKEOUT_X) )
    {
        drawStrikeoutChar( rPos, nWidth, eStrikeout );
        bStrikeoutDone = true;
    }

    Point aPos( rPos );
    TextAlign eAlign = m_aCurrentPDFState.m_aFont.GetAlign();
    if( eAlign == ALIGN_TOP )
        aPos.Y() += HCONV( pFontEntry->maMetric.mnAscent );
    else if( eAlign == ALIGN_BOTTOM )
        aPos.Y() -= HCONV( pFontEntry->maMetric.mnDescent );

    OStringBuffer aLine( 512 );
    // save GS
    aLine.append( "q " );

    // rotate and translate matrix
    double fAngle = (double)m_aCurrentPDFState.m_aFont.GetOrientation() * M_PI / 1800.0;
    Matrix3 aMat;
    aMat.rotate( fAngle );
    aMat.translate( aPos.X(), aPos.Y() );
    aMat.append( m_aPages.back(), aLine );
    aLine.append( " cm\n" );

    if ( aUnderlineColor.GetTransparency() != 0 )
        aUnderlineColor = aStrikeoutColor;

    if ( (eUnderline == UNDERLINE_SMALLWAVE) ||
         (eUnderline == UNDERLINE_WAVE) ||
         (eUnderline == UNDERLINE_DOUBLEWAVE) ||
         (eUnderline == UNDERLINE_BOLDWAVE) )
    {
        drawWaveTextLine( aLine, nWidth, eUnderline, aUnderlineColor, bUnderlineAbove );
        bUnderlineDone = true;
    }

    if ( (eOverline == UNDERLINE_SMALLWAVE) ||
         (eOverline == UNDERLINE_WAVE) ||
         (eOverline == UNDERLINE_DOUBLEWAVE) ||
         (eOverline == UNDERLINE_BOLDWAVE) )
    {
        drawWaveTextLine( aLine, nWidth, eOverline, aOverlineColor, true );
        bOverlineDone = true;
    }

    if ( !bUnderlineDone )
    {
        drawStraightTextLine( aLine, nWidth, eUnderline, aUnderlineColor, bUnderlineAbove );
    }

    if ( !bOverlineDone )
    {
        drawStraightTextLine( aLine, nWidth, eOverline, aOverlineColor, true );
    }

    if ( !bStrikeoutDone )
    {
        drawStrikeoutLine( aLine, nWidth, eStrikeout, aStrikeoutColor );
    }

    aLine.append( "Q\n" );
    writeBuffer( aLine.getStr(), aLine.getLength() );
}

void PDFWriterImpl::drawPolygon( const Polygon& rPoly )
{
    MARK( "drawPolygon" );

    updateGraphicsState();

    if( m_aGraphicsStack.front().m_aLineColor == Color( COL_TRANSPARENT ) &&
        m_aGraphicsStack.front().m_aFillColor == Color( COL_TRANSPARENT ) )
        return;

    int nPoints = rPoly.GetSize();
    OStringBuffer aLine( 20 * nPoints );
    m_aPages.back().appendPolygon( rPoly, aLine );
    if( m_aGraphicsStack.front().m_aLineColor != Color( COL_TRANSPARENT ) &&
        m_aGraphicsStack.front().m_aFillColor != Color( COL_TRANSPARENT ) )
        aLine.append( "B*\n" );
    else if( m_aGraphicsStack.front().m_aLineColor != Color( COL_TRANSPARENT ) )
        aLine.append( "S\n" );
    else
        aLine.append( "f*\n" );

    writeBuffer( aLine.getStr(), aLine.getLength() );
}

void PDFWriterImpl::drawPolyPolygon( const PolyPolygon& rPolyPoly )
{
    MARK( "drawPolyPolygon" );

    updateGraphicsState();

    if( m_aGraphicsStack.front().m_aLineColor == Color( COL_TRANSPARENT ) &&
        m_aGraphicsStack.front().m_aFillColor == Color( COL_TRANSPARENT ) )
        return;

    int nPolygons = rPolyPoly.Count();

    OStringBuffer aLine( 40 * nPolygons );
    m_aPages.back().appendPolyPolygon( rPolyPoly, aLine );
    if( m_aGraphicsStack.front().m_aLineColor != Color( COL_TRANSPARENT ) &&
        m_aGraphicsStack.front().m_aFillColor != Color( COL_TRANSPARENT ) )
        aLine.append( "B*\n" );
    else if( m_aGraphicsStack.front().m_aLineColor != Color( COL_TRANSPARENT ) )
        aLine.append( "S\n" );
    else
        aLine.append( "f*\n" );

    writeBuffer( aLine.getStr(), aLine.getLength() );
}

void PDFWriterImpl::drawTransparent( const PolyPolygon& rPolyPoly, sal_uInt32 nTransparentPercent )
{
    DBG_ASSERT( nTransparentPercent <= 100, "invalid alpha value" );
    nTransparentPercent = nTransparentPercent % 100;

    MARK( "drawTransparent" );

    updateGraphicsState();

    if( m_aGraphicsStack.front().m_aLineColor == Color( COL_TRANSPARENT ) &&
        m_aGraphicsStack.front().m_aFillColor == Color( COL_TRANSPARENT ) )
        return;

    if( m_bIsPDF_A1 || m_aContext.Version < PDFWriter::PDF_1_4 )
    {
        m_aErrors.insert( m_bIsPDF_A1 ?
                          PDFWriter::Warning_Transparency_Omitted_PDFA :
                          PDFWriter::Warning_Transparency_Omitted_PDF13 );

        drawPolyPolygon( rPolyPoly );
        return;
    }

    // create XObject
    m_aTransparentObjects.push_back( TransparencyEmit() );
    // FIXME: polygons with beziers may yield incorrect bound rect
    m_aTransparentObjects.back().m_aBoundRect	  = rPolyPoly.GetBoundRect();
    // convert rectangle to default user space
    m_aPages.back().convertRect( m_aTransparentObjects.back().m_aBoundRect );
    m_aTransparentObjects.back().m_nObject		    = createObject();
    m_aTransparentObjects.back().m_nExtGStateObject = createObject();
    m_aTransparentObjects.back().m_fAlpha		    = (double)(100-nTransparentPercent) / 100.0;
    m_aTransparentObjects.back().m_pContentStream   = new SvMemoryStream( 256, 256 );
    // create XObject's content stream
    OStringBuffer aContent( 256 );
    m_aPages.back().appendPolyPolygon( rPolyPoly, aContent );
    if( m_aCurrentPDFState.m_aLineColor != Color( COL_TRANSPARENT ) &&
        m_aCurrentPDFState.m_aFillColor != Color( COL_TRANSPARENT ) )
        aContent.append( " B*\n" );
    else if( m_aCurrentPDFState.m_aLineColor != Color( COL_TRANSPARENT ) )
        aContent.append( " S\n" );
    else
        aContent.append( " f*\n" );
    m_aTransparentObjects.back().m_pContentStream->Write( aContent.getStr(), aContent.getLength() );

    OStringBuffer aObjName( 16 );
    aObjName.append( "Tr" );
    aObjName.append( m_aTransparentObjects.back().m_nObject );
    OString aTrName( aObjName.makeStringAndClear() );
    aObjName.append( "EGS" );
    aObjName.append( m_aTransparentObjects.back().m_nExtGStateObject );
    OString aExtName( aObjName.makeStringAndClear() );

    OStringBuffer aLine( 80 );
    // insert XObject
    aLine.append( "q /" );
    aLine.append( aExtName );
    aLine.append( " gs /" );
    aLine.append( aTrName );
    aLine.append( " Do Q\n" );
    writeBuffer( aLine.getStr(), aLine.getLength() );

    pushResource( ResXObject, aTrName, m_aTransparentObjects.back().m_nObject );
    pushResource( ResExtGState, aExtName, m_aTransparentObjects.back().m_nExtGStateObject );
}

void PDFWriterImpl::pushResource( ResourceKind eKind, const OString& rResource, sal_Int32 nObject )
{
    if( nObject >= 0 )
    {
        switch( eKind )
        {
            case ResXObject:
                m_aGlobalResourceDict.m_aXObjects[ rResource ] = nObject;
                if( ! m_aOutputStreams.empty() )
                    m_aOutputStreams.front().m_aResourceDict.m_aXObjects[ rResource ] = nObject;
                break;
            case ResExtGState:
                m_aGlobalResourceDict.m_aExtGStates[ rResource ] = nObject;
                if( ! m_aOutputStreams.empty() )
                    m_aOutputStreams.front().m_aResourceDict.m_aExtGStates[ rResource ] = nObject;
                break;
            case ResShading:
                m_aGlobalResourceDict.m_aShadings[ rResource ] = nObject;
                if( ! m_aOutputStreams.empty() )
                    m_aOutputStreams.front().m_aResourceDict.m_aShadings[ rResource ] = nObject;
                break;
            case ResPattern:
                m_aGlobalResourceDict.m_aPatterns[ rResource ] = nObject;
                if( ! m_aOutputStreams.empty() )
                    m_aOutputStreams.front().m_aResourceDict.m_aPatterns[ rResource ] = nObject;
                break;
        }
    }
}

void PDFWriterImpl::beginRedirect( SvStream* pStream, const Rectangle& rTargetRect )
{
    push( PUSH_ALL );

    setClipRegion( Region() );
    updateGraphicsState();

    m_aOutputStreams.push_front( StreamRedirect() );
    m_aOutputStreams.front().m_pStream = pStream;
    m_aOutputStreams.front().m_aMapMode = m_aMapMode;

    if( !rTargetRect.IsEmpty() )
    {
        m_aOutputStreams.front().m_aTargetRect =
            lcl_convert( m_aGraphicsStack.front().m_aMapMode,
                         m_aMapMode,
                         getReferenceDevice(),
                         rTargetRect );
        Point aDelta = m_aOutputStreams.front().m_aTargetRect.BottomLeft();
        long nPageHeight = pointToPixel(m_aPages[m_nCurrentPage].getHeight());
        aDelta.Y() = -(nPageHeight - m_aOutputStreams.front().m_aTargetRect.Bottom());
        m_aMapMode.SetOrigin( m_aMapMode.GetOrigin() + aDelta );
    }

    // setup graphics state for independent object stream

    // force reemitting colors
    m_aCurrentPDFState.m_aLineColor = Color( COL_TRANSPARENT );
    m_aCurrentPDFState.m_aFillColor = Color( COL_TRANSPARENT );
}

Rectangle PDFWriterImpl::getRedirectTargetRect() const
{
    return m_aOutputStreams.empty() ? Rectangle() : m_aOutputStreams.front().m_aTargetRect;
}

SvStream* PDFWriterImpl::endRedirect()
{
    SvStream* pStream = NULL;
    if( ! m_aOutputStreams.empty() )
    {
        pStream		= m_aOutputStreams.front().m_pStream;
        m_aMapMode	= m_aOutputStreams.front().m_aMapMode;
        m_aOutputStreams.pop_front();
    }

    pop();
    // force reemitting colors
    m_aCurrentPDFState.m_aLineColor = Color( COL_TRANSPARENT );
    m_aCurrentPDFState.m_aFillColor = Color( COL_TRANSPARENT );

    updateGraphicsState();

    return pStream;
}

void PDFWriterImpl::beginTransparencyGroup()
{
    updateGraphicsState();
    if( m_aContext.Version >= PDFWriter::PDF_1_4 )
        beginRedirect( new SvMemoryStream( 1024, 1024 ), Rectangle() );
}

void PDFWriterImpl::endTransparencyGroup( const Rectangle& rBoundingBox, sal_uInt32 nTransparentPercent )
{
    DBG_ASSERT( nTransparentPercent <= 100, "invalid alpha value" );
    nTransparentPercent = nTransparentPercent % 100;

    if( m_aContext.Version >= PDFWriter::PDF_1_4 )
    {
        // create XObject
        m_aTransparentObjects.push_back( TransparencyEmit() );
        m_aTransparentObjects.back().m_aBoundRect	= rBoundingBox;
        // convert rectangle to default user space
        m_aPages.back().convertRect( m_aTransparentObjects.back().m_aBoundRect );
        m_aTransparentObjects.back().m_nObject		= createObject();
        m_aTransparentObjects.back().m_fAlpha		= (double)(100-nTransparentPercent) / 100.0;
        // get XObject's content stream
        m_aTransparentObjects.back().m_pContentStream = static_cast<SvMemoryStream*>(endRedirect());
        m_aTransparentObjects.back().m_nExtGStateObject = createObject();

        OStringBuffer aObjName( 16 );
        aObjName.append( "Tr" );
        aObjName.append( m_aTransparentObjects.back().m_nObject );
        OString aTrName( aObjName.makeStringAndClear() );
        aObjName.append( "EGS" );
        aObjName.append( m_aTransparentObjects.back().m_nExtGStateObject );
        OString aExtName( aObjName.makeStringAndClear() );

        OStringBuffer aLine( 80 );
        // insert XObject
        aLine.append( "q /" );
        aLine.append( aExtName );
        aLine.append( " gs /" );
        aLine.append( aTrName );
        aLine.append( " Do Q\n" );
        writeBuffer( aLine.getStr(), aLine.getLength() );

        pushResource( ResXObject, aTrName, m_aTransparentObjects.back().m_nObject );
        pushResource( ResExtGState, aExtName, m_aTransparentObjects.back().m_nExtGStateObject );
    }
}

void PDFWriterImpl::endTransparencyGroup( const Rectangle& rBoundingBox, const Bitmap& rAlphaMask )
{
    if( m_aContext.Version >= PDFWriter::PDF_1_4 )
    {
        // create XObject
        m_aTransparentObjects.push_back( TransparencyEmit() );
        m_aTransparentObjects.back().m_aBoundRect	= rBoundingBox;
        // convert rectangle to default user space
        m_aPages.back().convertRect( m_aTransparentObjects.back().m_aBoundRect );
        m_aTransparentObjects.back().m_nObject		= createObject();
        m_aTransparentObjects.back().m_fAlpha		= 0.0;
        // get XObject's content stream
        m_aTransparentObjects.back().m_pContentStream = static_cast<SvMemoryStream*>(endRedirect());
        m_aTransparentObjects.back().m_nExtGStateObject = createObject();

        // draw soft mask
        beginRedirect( new SvMemoryStream( 1024, 1024 ), Rectangle() );
        drawBitmap( rBoundingBox.TopLeft(), rBoundingBox.GetSize(), rAlphaMask );
        m_aTransparentObjects.back().m_pSoftMaskStream = static_cast<SvMemoryStream*>(endRedirect());

        OStringBuffer aObjName( 16 );
        aObjName.append( "Tr" );
        aObjName.append( m_aTransparentObjects.back().m_nObject );
        OString aTrName( aObjName.makeStringAndClear() );
        aObjName.append( "EGS" );
        aObjName.append( m_aTransparentObjects.back().m_nExtGStateObject );
        OString aExtName( aObjName.makeStringAndClear() );

        OStringBuffer aLine( 80 );
        // insert XObject
        aLine.append( "q /" );
        aLine.append( aExtName );
        aLine.append( " gs /" );
        aLine.append( aTrName );
        aLine.append( " Do Q\n" );
        writeBuffer( aLine.getStr(), aLine.getLength() );

        pushResource( ResXObject, aTrName, m_aTransparentObjects.back().m_nObject );
        pushResource( ResExtGState, aExtName, m_aTransparentObjects.back().m_nExtGStateObject );
    }
}

void PDFWriterImpl::drawRectangle( const Rectangle& rRect )
{
    MARK( "drawRectangle" );

    updateGraphicsState();

    if( m_aGraphicsStack.front().m_aLineColor == Color( COL_TRANSPARENT ) &&
        m_aGraphicsStack.front().m_aFillColor == Color( COL_TRANSPARENT ) )
        return;

    OStringBuffer aLine( 40 );
    m_aPages.back().appendRect( rRect, aLine );

    if( m_aGraphicsStack.front().m_aLineColor != Color( COL_TRANSPARENT ) &&
        m_aGraphicsStack.front().m_aFillColor != Color( COL_TRANSPARENT ) )
        aLine.append( " B*\n" );
    else if( m_aGraphicsStack.front().m_aLineColor != Color( COL_TRANSPARENT ) )
        aLine.append( " S\n" );
    else
        aLine.append( " f*\n" );

    writeBuffer( aLine.getStr(), aLine.getLength() );
}

void PDFWriterImpl::drawRectangle( const Rectangle& rRect, sal_uInt32 nHorzRound, sal_uInt32 nVertRound )
{
    MARK( "drawRectangle with rounded edges" );

    if( !nHorzRound && !nVertRound )
        drawRectangle( rRect );

    updateGraphicsState();

    if( m_aGraphicsStack.front().m_aLineColor == Color( COL_TRANSPARENT ) &&
        m_aGraphicsStack.front().m_aFillColor == Color( COL_TRANSPARENT ) )
        return;

    if( nHorzRound > (sal_uInt32)rRect.GetWidth()/2 )
        nHorzRound = rRect.GetWidth()/2;
    if( nVertRound > (sal_uInt32)rRect.GetHeight()/2 )
        nVertRound = rRect.GetHeight()/2;

    Point aPoints[16];
    const double kappa = 0.5522847498;
    const sal_uInt32 kx = (sal_uInt32)((kappa*(double)nHorzRound)+0.5);
    const sal_uInt32 ky = (sal_uInt32)((kappa*(double)nVertRound)+0.5);

    aPoints[1]  = Point( rRect.TopLeft().X() + nHorzRound, rRect.TopLeft().Y() );
    aPoints[0]  = Point( aPoints[1].X() - kx, aPoints[1].Y() );
    aPoints[2]  = Point( rRect.TopRight().X()+1 - nHorzRound, aPoints[1].Y() );
    aPoints[3]  = Point( aPoints[2].X()+kx, aPoints[2].Y() );

    aPoints[5]  = Point( rRect.TopRight().X()+1, rRect.TopRight().Y()+nVertRound );
    aPoints[4]  = Point( aPoints[5].X(), aPoints[5].Y()-ky );
    aPoints[6]  = Point( aPoints[5].X(), rRect.BottomRight().Y()+1 - nVertRound );
    aPoints[7]  = Point( aPoints[6].X(), aPoints[6].Y()+ky );

    aPoints[9]  = Point( rRect.BottomRight().X()+1-nHorzRound, rRect.BottomRight().Y()+1 );
    aPoints[8]  = Point( aPoints[9].X()+kx, aPoints[9].Y() );
    aPoints[10] = Point( rRect.BottomLeft().X() + nHorzRound, aPoints[9].Y() );
    aPoints[11] = Point( aPoints[10].X()-kx, aPoints[10].Y() );

    aPoints[13] = Point( rRect.BottomLeft().X(), rRect.BottomLeft().Y()+1-nVertRound );
    aPoints[12] = Point( aPoints[13].X(), aPoints[13].Y()+ky );
    aPoints[14] = Point( rRect.TopLeft().X(), rRect.TopLeft().Y()+nVertRound );
    aPoints[15] = Point( aPoints[14].X(), aPoints[14].Y()-ky );


    OStringBuffer aLine( 80 );
    m_aPages.back().appendPoint( aPoints[1], aLine );
    aLine.append( " m " );
    m_aPages.back().appendPoint( aPoints[2], aLine );
    aLine.append( " l " );
    m_aPages.back().appendPoint( aPoints[3], aLine );
    aLine.append( ' ' );
    m_aPages.back().appendPoint( aPoints[4], aLine );
    aLine.append( ' ' );
    m_aPages.back().appendPoint( aPoints[5], aLine );
    aLine.append( " c\n" );
    m_aPages.back().appendPoint( aPoints[6], aLine );
    aLine.append( " l " );
    m_aPages.back().appendPoint( aPoints[7], aLine );
    aLine.append( ' ' );
    m_aPages.back().appendPoint( aPoints[8], aLine );
    aLine.append( ' ' );
    m_aPages.back().appendPoint( aPoints[9], aLine );
    aLine.append( " c\n" );
    m_aPages.back().appendPoint( aPoints[10], aLine );
    aLine.append( " l " );
    m_aPages.back().appendPoint( aPoints[11], aLine );
    aLine.append( ' ' );
    m_aPages.back().appendPoint( aPoints[12], aLine );
    aLine.append( ' ' );
    m_aPages.back().appendPoint( aPoints[13], aLine );
    aLine.append( " c\n" );
    m_aPages.back().appendPoint( aPoints[14], aLine );
    aLine.append( " l " );
    m_aPages.back().appendPoint( aPoints[15], aLine );
    aLine.append( ' ' );
    m_aPages.back().appendPoint( aPoints[0], aLine );
    aLine.append( ' ' );
    m_aPages.back().appendPoint( aPoints[1], aLine );
    aLine.append( " c " );

    if( m_aGraphicsStack.front().m_aLineColor != Color( COL_TRANSPARENT ) &&
        m_aGraphicsStack.front().m_aFillColor != Color( COL_TRANSPARENT ) )
        aLine.append( "b*\n" );
    else if( m_aGraphicsStack.front().m_aLineColor != Color( COL_TRANSPARENT ) )
        aLine.append( "s\n" );
    else
        aLine.append( "f*\n" );

    writeBuffer( aLine.getStr(), aLine.getLength() );
}

void PDFWriterImpl::drawEllipse( const Rectangle& rRect )
{
    MARK( "drawEllipse" );

    updateGraphicsState();

    if( m_aGraphicsStack.front().m_aLineColor == Color( COL_TRANSPARENT ) &&
        m_aGraphicsStack.front().m_aFillColor == Color( COL_TRANSPARENT ) )
        return;

    Point aPoints[12];
    const double kappa = 0.5522847498;
    const sal_uInt32 kx = (sal_uInt32)((kappa*(double)rRect.GetWidth()/2.0)+0.5);
    const sal_uInt32 ky = (sal_uInt32)((kappa*(double)rRect.GetHeight()/2.0)+0.5);

    aPoints[1]  = Point( rRect.TopLeft().X() + rRect.GetWidth()/2, rRect.TopLeft().Y() );
    aPoints[0]  = Point( aPoints[1].X() - kx, aPoints[1].Y() );
    aPoints[2]  = Point( aPoints[1].X() + kx, aPoints[1].Y() );

    aPoints[4]  = Point( rRect.TopRight().X()+1, rRect.TopRight().Y() + rRect.GetHeight()/2 );
    aPoints[3]  = Point( aPoints[4].X(), aPoints[4].Y() - ky );
    aPoints[5]  = Point( aPoints[4].X(), aPoints[4].Y() + ky );

    aPoints[7]  = Point( rRect.BottomLeft().X() + rRect.GetWidth()/2, rRect.BottomLeft().Y()+1 );
    aPoints[6]  = Point( aPoints[7].X() + kx, aPoints[7].Y() );
    aPoints[8]  = Point( aPoints[7].X() - kx, aPoints[7].Y() );

    aPoints[10] = Point( rRect.TopLeft().X(), rRect.TopLeft().Y() + rRect.GetHeight()/2 );
    aPoints[9]  = Point( aPoints[10].X(), aPoints[10].Y() + ky );
    aPoints[11] = Point( aPoints[10].X(), aPoints[10].Y() - ky );

    OStringBuffer aLine( 80 );
    m_aPages.back().appendPoint( aPoints[1], aLine );
    aLine.append( " m " );
    m_aPages.back().appendPoint( aPoints[2], aLine );
    aLine.append( ' ' );
    m_aPages.back().appendPoint( aPoints[3], aLine );
    aLine.append( ' ' );
    m_aPages.back().appendPoint( aPoints[4], aLine );
    aLine.append( " c\n" );
    m_aPages.back().appendPoint( aPoints[5], aLine );
    aLine.append( ' ' );
    m_aPages.back().appendPoint( aPoints[6], aLine );
    aLine.append( ' ' );
    m_aPages.back().appendPoint( aPoints[7], aLine );
    aLine.append( " c\n" );
    m_aPages.back().appendPoint( aPoints[8], aLine );
    aLine.append( ' ' );
    m_aPages.back().appendPoint( aPoints[9], aLine );
    aLine.append( ' ' );
    m_aPages.back().appendPoint( aPoints[10], aLine );
    aLine.append( " c\n" );
    m_aPages.back().appendPoint( aPoints[11], aLine );
    aLine.append( ' ' );
    m_aPages.back().appendPoint( aPoints[0], aLine );
    aLine.append( ' ' );
    m_aPages.back().appendPoint( aPoints[1], aLine );
    aLine.append( " c " );

    if( m_aGraphicsStack.front().m_aLineColor != Color( COL_TRANSPARENT ) &&
        m_aGraphicsStack.front().m_aFillColor != Color( COL_TRANSPARENT ) )
        aLine.append( "b*\n" );
    else if( m_aGraphicsStack.front().m_aLineColor != Color( COL_TRANSPARENT ) )
        aLine.append( "s\n" );
    else
        aLine.append( "f*\n" );

    writeBuffer( aLine.getStr(), aLine.getLength() );
}

static double calcAngle( const Rectangle& rRect, const Point& rPoint )
{
    Point aOrigin((rRect.Left()+rRect.Right()+1)/2,
                  (rRect.Top()+rRect.Bottom()+1)/2);
    Point aPoint = rPoint - aOrigin;

    double fX = (double)aPoint.X();
    double fY = (double)-aPoint.Y();

    if( rRect.GetWidth() > rRect.GetHeight() )
        fY = fY*((double)rRect.GetWidth()/(double)rRect.GetHeight());
    else if( rRect.GetHeight() > rRect.GetWidth() )
        fX = fX*((double)rRect.GetHeight()/(double)rRect.GetWidth());
    return atan2( fY, fX );
}

void PDFWriterImpl::drawArc( const Rectangle& rRect, const Point& rStart, const Point& rStop, bool bWithPie, bool bWithChord )
{
    MARK( "drawArc" );

    updateGraphicsState();

    if( m_aGraphicsStack.front().m_aLineColor == Color( COL_TRANSPARENT ) &&
        m_aGraphicsStack.front().m_aFillColor == Color( COL_TRANSPARENT ) )
        return;

    // calculate start and stop angles
    const double fStartAngle = calcAngle( rRect, rStart );
    double fStopAngle  = calcAngle( rRect, rStop );
    while( fStopAngle < fStartAngle )
        fStopAngle += 2.0*M_PI;
    const int nFragments = (int)((fStopAngle-fStartAngle)/(M_PI/2.0))+1;
    const double fFragmentDelta = (fStopAngle-fStartAngle)/(double)nFragments;
    const double kappa = fabs( 4.0 * (1.0-cos(fFragmentDelta/2.0))/sin(fFragmentDelta/2.0) / 3.0);
    const double halfWidth = (double)rRect.GetWidth()/2.0;
    const double halfHeight = (double)rRect.GetHeight()/2.0;

    const Point aCenter( (rRect.Left()+rRect.Right()+1)/2,
                         (rRect.Top()+rRect.Bottom()+1)/2 );

    OStringBuffer aLine( 30*nFragments );
    Point aPoint( (int)(halfWidth * cos(fStartAngle) ),
                  -(int)(halfHeight * sin(fStartAngle) ) );
    aPoint += aCenter;
    m_aPages.back().appendPoint( aPoint, aLine );
    aLine.append( " m " );
    if( !basegfx::fTools::equal(fStartAngle, fStopAngle) )
    {
        for( int i = 0; i < nFragments; i++ )
        {
            const double fStartFragment = fStartAngle + (double)i*fFragmentDelta;
            const double fStopFragment = fStartFragment + fFragmentDelta;
            aPoint = Point( (int)(halfWidth * (cos(fStartFragment) - kappa*sin(fStartFragment) ) ),
                            -(int)(halfHeight * (sin(fStartFragment) + kappa*cos(fStartFragment) ) ) );
            aPoint += aCenter;
            m_aPages.back().appendPoint( aPoint, aLine );
            aLine.append( ' ' );

            aPoint = Point( (int)(halfWidth * (cos(fStopFragment) + kappa*sin(fStopFragment) ) ),
                            -(int)(halfHeight * (sin(fStopFragment) - kappa*cos(fStopFragment) ) ) );
            aPoint += aCenter;
            m_aPages.back().appendPoint( aPoint, aLine );
            aLine.append( ' ' );

            aPoint = Point( (int)(halfWidth * cos(fStopFragment) ),
                            -(int)(halfHeight * sin(fStopFragment) ) );
            aPoint += aCenter;
            m_aPages.back().appendPoint( aPoint, aLine );
            aLine.append( " c\n" );
        }
    }
    if( bWithChord || bWithPie )
    {
        if( bWithPie )
        {
            m_aPages.back().appendPoint( aCenter, aLine );
            aLine.append( " l " );
        }
        aLine.append( "h " );
    }
    if( ! bWithChord && ! bWithPie )
        aLine.append( "S\n" );
    else if( m_aGraphicsStack.front().m_aLineColor != Color( COL_TRANSPARENT ) &&
        m_aGraphicsStack.front().m_aFillColor != Color( COL_TRANSPARENT ) )
        aLine.append( "B*\n" );
    else if( m_aGraphicsStack.front().m_aLineColor != Color( COL_TRANSPARENT ) )
        aLine.append( "S\n" );
    else
        aLine.append( "f*\n" );

    writeBuffer( aLine.getStr(), aLine.getLength() );
}

void PDFWriterImpl::drawPolyLine( const Polygon& rPoly )
{
    MARK( "drawPolyLine" );

    USHORT nPoints = rPoly.GetSize();
    if( nPoints < 2 )
        return;

    updateGraphicsState();

    if( m_aGraphicsStack.front().m_aLineColor == Color( COL_TRANSPARENT ) )
        return;

    OStringBuffer aLine( 20 * nPoints );
    m_aPages.back().appendPolygon( rPoly, aLine, rPoly[0] == rPoly[nPoints-1] );
    aLine.append( "S\n" );

    writeBuffer( aLine.getStr(), aLine.getLength() );
}

void PDFWriterImpl::drawPolyLine( const Polygon& rPoly, const LineInfo& rInfo )
{
    MARK( "drawPolyLine with LineInfo" );

    updateGraphicsState();

    if( m_aGraphicsStack.front().m_aLineColor == Color( COL_TRANSPARENT ) )
        return;

    OStringBuffer aLine;
    aLine.append( "q " );
    if( m_aPages.back().appendLineInfo( rInfo, aLine ) )
    {
        writeBuffer( aLine.getStr(), aLine.getLength() );
        drawPolyLine( rPoly );
        writeBuffer( "Q\n", 2 );
    }
    else
    {
        PDFWriter::ExtLineInfo aInfo;
        convertLineInfoToExtLineInfo( rInfo, aInfo );
        drawPolyLine( rPoly, aInfo );
    }
}

void PDFWriterImpl::convertLineInfoToExtLineInfo( const LineInfo& rIn, PDFWriter::ExtLineInfo& rOut )
{
    DBG_ASSERT( rIn.GetStyle() == LINE_DASH, "invalid conversion" );
    rOut.m_fLineWidth           = rIn.GetWidth();
    rOut.m_fTransparency        = 0.0;
    rOut.m_eCap                 = PDFWriter::capButt;
    rOut.m_eJoin                = PDFWriter::joinMiter;
    rOut.m_fMiterLimit          = 10;
    rOut.m_aDashArray.clear();

    int nDashes     = rIn.GetDashCount();
    int nDashLen    = rIn.GetDashLen();
    int nDistance   = rIn.GetDistance();
    for( int n  = 0; n < nDashes; n++ )
    {
        rOut.m_aDashArray.push_back( nDashLen );
        rOut.m_aDashArray.push_back( nDistance );
    }
    int nDots       = rIn.GetDotCount();
    int nDotLen     = rIn.GetDotLen();
    for( int n  = 0; n < nDots; n++ )
    {
        rOut.m_aDashArray.push_back( nDotLen );
        rOut.m_aDashArray.push_back( nDistance );
    }
}

void PDFWriterImpl::drawPolyLine( const Polygon& rPoly, const PDFWriter::ExtLineInfo& rInfo )
{
    MARK( "drawPolyLine with ExtLineInfo" );

    updateGraphicsState();

    if( m_aGraphicsStack.front().m_aLineColor == Color( COL_TRANSPARENT ) )
        return;

    if( rInfo.m_fTransparency >= 1.0 )
        return;

    if( rInfo.m_fTransparency != 0.0 )
        beginTransparencyGroup();

    OStringBuffer aLine;
    aLine.append( "q " );
    m_aPages.back().appendMappedLength( rInfo.m_fLineWidth, aLine );
    aLine.append( " w" );
    if( rInfo.m_aDashArray.size() < 10 ) // implmentation limit of acrobat reader
    {
        switch( rInfo.m_eCap )
        {
            default:
            case PDFWriter::capButt:   aLine.append( " 0 J" );break;
            case PDFWriter::capRound:  aLine.append( " 1 J" );break;
            case PDFWriter::capSquare: aLine.append( " 2 J" );break;
        }
        switch( rInfo.m_eJoin )
        {
            default:
            case PDFWriter::joinMiter:
            {
                double fLimit = rInfo.m_fMiterLimit;
                if( rInfo.m_fLineWidth < rInfo.m_fMiterLimit )
                    fLimit = fLimit / rInfo.m_fLineWidth;
                if( fLimit < 1.0 )
                    fLimit = 1.0;
                aLine.append( " 0 j " );
                appendDouble( fLimit, aLine );
                aLine.append( " M" );
            }
            break;
            case PDFWriter::joinRound:  aLine.append( " 1 j" );break;
            case PDFWriter::joinBevel:  aLine.append( " 2 j" );break;
        }
        if( rInfo.m_aDashArray.size() > 0 )
        {
            aLine.append( " [ " );
            for( std::vector<double>::const_iterator it = rInfo.m_aDashArray.begin();
                 it != rInfo.m_aDashArray.end(); ++it )
            {
                m_aPages.back().appendMappedLength( *it, aLine );
                aLine.append( ' ' );
            }
            aLine.append( "] 0 d" );
        }
        aLine.append( "\n" );
        writeBuffer( aLine.getStr(), aLine.getLength() );
        drawPolyLine( rPoly );
    }
    else
    {
        basegfx::B2DPolygon aPoly(rPoly.getB2DPolygon());
        basegfx::B2DPolyPolygon aPolyPoly;

		basegfx::tools::applyLineDashing(aPoly, rInfo.m_aDashArray, &aPolyPoly);

		// Old applyLineDashing subdivided the polygon. New one will create bezier curve segments.
		// To mimic old behaviour, apply subdivide here. If beziers shall be written (better quality)
		// this line needs to be removed and the loop below adapted accordingly
		aPolyPoly = basegfx::tools::adaptiveSubdivideByAngle(aPolyPoly);

		const sal_uInt32 nPolygonCount(aPolyPoly.count());

		for( sal_uInt32 nPoly = 0; nPoly < nPolygonCount; nPoly++ )
        {
            aLine.append( (nPoly != 0 && (nPoly & 7) == 0) ? "\n" : " " );
            aPoly = aPolyPoly.getB2DPolygon( nPoly );
			const sal_uInt32 nPointCount(aPoly.count());

			if(nPointCount)
			{
				const sal_uInt32 nEdgeCount(aPoly.isClosed() ? nPointCount : nPointCount - 1);
				basegfx::B2DPoint aCurrent(aPoly.getB2DPoint(0));

				for(sal_uInt32 a(0); a < nEdgeCount; a++)
				{
					const sal_uInt32 nNextIndex((a + 1) % nPointCount);
					const basegfx::B2DPoint aNext(aPoly.getB2DPoint(nNextIndex));

					m_aPages.back().appendPoint( Point( FRound(aCurrent.getX()),
														FRound(aCurrent.getY()) ),
												 aLine );
					aLine.append( " m " );
					m_aPages.back().appendPoint( Point( FRound(aNext.getX()),
														FRound(aNext.getY()) ),
												 aLine );
					aLine.append( " l" );

					// prepare next edge
					aCurrent = aNext;
				}
			}
        }
        aLine.append( " S " );
        writeBuffer( aLine.getStr(), aLine.getLength() );
    }
    writeBuffer( "Q\n", 2 );

    if( rInfo.m_fTransparency != 0.0 )
    {
        // FIXME: actually this may be incorrect with bezier polygons
        Rectangle aBoundRect( rPoly.GetBoundRect() );
        // avoid clipping with thick lines
        if( rInfo.m_fLineWidth > 0.0 )
        {
            sal_Int32 nLW = sal_Int32(rInfo.m_fLineWidth);
            aBoundRect.Top()    -= nLW;
            aBoundRect.Left()   -= nLW;
            aBoundRect.Right()  += nLW;
            aBoundRect.Bottom() += nLW;
        }
        endTransparencyGroup( aBoundRect, (USHORT)(100.0*rInfo.m_fTransparency) );
    }
}

void PDFWriterImpl::drawPixel( const Point& rPoint, const Color& rColor )
{
    MARK( "drawPixel" );

    Color aColor = ( rColor == Color( COL_TRANSPARENT ) ? m_aGraphicsStack.front().m_aLineColor : rColor );

    if( aColor == Color( COL_TRANSPARENT ) )
        return;

    // pixels are drawn in line color, so have to set
    // the nonstroking color to line color
    Color aOldFillColor = m_aGraphicsStack.front().m_aFillColor;
    setFillColor( aColor );

    updateGraphicsState();

    OStringBuffer aLine( 20 );
    m_aPages.back().appendPoint( rPoint, aLine );
    aLine.append( ' ' );
    appendDouble( 1.0/double(getReferenceDevice()->ImplGetDPIX()), aLine );
    aLine.append( ' ' );
    appendDouble( 1.0/double(getReferenceDevice()->ImplGetDPIY()), aLine );
    aLine.append( " re f\n" );
    writeBuffer( aLine.getStr(), aLine.getLength() );

    setFillColor( aOldFillColor );
}

void PDFWriterImpl::drawPixel( const Polygon& rPoints, const Color* pColors )
{
    MARK( "drawPixel with Polygon" );

    updateGraphicsState();

    if( m_aGraphicsStack.front().m_aLineColor == Color( COL_TRANSPARENT ) && ! pColors )
        return;

    USHORT nPoints = rPoints.GetSize();
    OStringBuffer aLine( nPoints*40 );
    aLine.append( "q " );
    if( ! pColors )
    {
        appendNonStrokingColor( m_aGraphicsStack.front().m_aLineColor, aLine );
        aLine.append( ' ' );
    }

    OStringBuffer aPixel(32);
    aPixel.append( ' ' );
    appendDouble( 1.0/double(getReferenceDevice()->ImplGetDPIX()), aPixel );
    aPixel.append( ' ' );
    appendDouble( 1.0/double(getReferenceDevice()->ImplGetDPIY()), aPixel );
    OString aPixelStr = aPixel.makeStringAndClear();
    for( USHORT i = 0; i < nPoints; i++ )
    {
        if( pColors )
        {
            if( pColors[i] == Color( COL_TRANSPARENT ) )
                continue;

            appendNonStrokingColor( pColors[i], aLine );
            aLine.append( ' ' );
        }
        m_aPages.back().appendPoint( rPoints[i], aLine );
        aLine.append( aPixelStr );
        aLine.append( " re f\n" );
    }
    aLine.append( "Q\n" );
    writeBuffer( aLine.getStr(), aLine.getLength() );
}

class AccessReleaser
{
    BitmapReadAccess* m_pAccess;
public:
    AccessReleaser( BitmapReadAccess* pAccess ) : m_pAccess( pAccess ){}
    ~AccessReleaser() { delete m_pAccess; }
};

bool PDFWriterImpl::writeTransparentObject( TransparencyEmit& rObject )
{
    CHECK_RETURN( updateObject( rObject.m_nObject ) );

    bool bFlateFilter = compressStream( rObject.m_pContentStream );
    rObject.m_pContentStream->Seek( STREAM_SEEK_TO_END );
    ULONG nSize = rObject.m_pContentStream->Tell();
    rObject.m_pContentStream->Seek( STREAM_SEEK_TO_BEGIN );
#if OSL_DEBUG_LEVEL > 1
    {
        OStringBuffer aLine( " PDFWriterImpl::writeTransparentObject" );
        emitComment( aLine.getStr() );
    }
#endif
    OStringBuffer aLine( 512 );
    CHECK_RETURN( updateObject( rObject.m_nObject ) );
    aLine.append( rObject.m_nObject );
    aLine.append( " 0 obj\n"
                  "<</Type/XObject\n"
                  "/Subtype/Form\n"
                  "/BBox[ " );
    appendFixedInt( rObject.m_aBoundRect.Left(), aLine );
    aLine.append( ' ' );
    appendFixedInt( rObject.m_aBoundRect.Top(), aLine );
    aLine.append( ' ' );
    appendFixedInt( rObject.m_aBoundRect.Right(), aLine );
    aLine.append( ' ' );
    appendFixedInt( rObject.m_aBoundRect.Bottom()+1, aLine );
    aLine.append( " ]\n" );
    /* #i42884# the PDF reference recommends that each Form XObject
    *  should have a resource dict; alas if that is the same object
    *  as the one of the page it triggers an endless recursion in
    *  acroread 5 (6 and up have that fixed). Since we have only one
    *  resource dict anyway, let's use the one from the page by NOT
    *  emitting a Resources entry.
    */
    #if 0
    aLine.append( "   /Resources " );
    aLine.append( getResourceDictObj() );
    aLine.append( " 0 R\n" );
    #endif

    aLine.append( "/Length " );
    aLine.append( (sal_Int32)(nSize) );
    aLine.append( "\n" );
    if( bFlateFilter )
        aLine.append( "/Filter/FlateDecode\n" );
    aLine.append( ">>\n"
                  "stream\n" );
    CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );
    checkAndEnableStreamEncryption( rObject.m_nObject );
    CHECK_RETURN( writeBuffer( rObject.m_pContentStream->GetData(), nSize ) );
    disableStreamEncryption();
    aLine.setLength( 0 );
    aLine.append( "\n"
                  "endstream\n"
                  "endobj\n\n" );
    CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );

    // write ExtGState dict for this XObject
    aLine.setLength( 0 );
    aLine.append( rObject.m_nExtGStateObject );
    aLine.append( " 0 obj\n"
                  "<<" );
    if( ! rObject.m_pSoftMaskStream )
    {
//i59651
        if( m_bIsPDF_A1 )
        {
            aLine.append( "/CA 1.0/ca 1.0" );
            m_aErrors.insert( PDFWriter::Warning_Transparency_Omitted_PDFA );
        }
        else
        {
            aLine.append(  "/CA " );
            appendDouble( rObject.m_fAlpha, aLine );
            aLine.append( "\n"
                          "   /ca " );
            appendDouble( rObject.m_fAlpha, aLine );
        }
        aLine.append( "\n" );
    }
    else
    {
        if( m_bIsPDF_A1 )
        {
            aLine.append( "/SMask/None" );
            m_aErrors.insert( PDFWriter::Warning_Transparency_Omitted_PDFA );
        }
        else
        {
            rObject.m_pSoftMaskStream->Seek( STREAM_SEEK_TO_END );
            sal_Int32 nMaskSize = (sal_Int32)rObject.m_pSoftMaskStream->Tell();
            rObject.m_pSoftMaskStream->Seek( STREAM_SEEK_TO_BEGIN );
            sal_Int32 nMaskObject = createObject();
            aLine.append( "/SMask<</Type/Mask/S/Luminosity/G " );
            aLine.append( nMaskObject );
            aLine.append( " 0 R>>\n" );

            OStringBuffer aMask;
            aMask.append( nMaskObject );
            aMask.append( " 0 obj\n"
                          "<</Type/XObject\n"
                          "/Subtype/Form\n"
                          "/BBox[" );
            appendFixedInt( rObject.m_aBoundRect.Left(), aMask );
            aMask.append( ' ' );
            appendFixedInt( rObject.m_aBoundRect.Top(), aMask );
            aMask.append( ' ' );
            appendFixedInt( rObject.m_aBoundRect.Right(), aMask );
            aMask.append( ' ' );
            appendFixedInt( rObject.m_aBoundRect.Bottom()+1, aMask );
            aMask.append( "]\n" );

            /* #i42884# see above */
#if 0
            aLine.append( "/Resources " );
            aMask.append( getResourceDictObj() );
            aMask.append( " 0 R\n" );
#endif

            aMask.append( "/Group<</S/Transparency/CS/DeviceRGB>>\n" );
            aMask.append( "/Length " );
            aMask.append( nMaskSize );
            aMask.append( ">>\n"
                          "stream\n" );
            CHECK_RETURN( updateObject( nMaskObject ) );
            checkAndEnableStreamEncryption(  nMaskObject );
            CHECK_RETURN( writeBuffer( aMask.getStr(), aMask.getLength() ) );
            CHECK_RETURN( writeBuffer( rObject.m_pSoftMaskStream->GetData(), nMaskSize ) );
            disableStreamEncryption();
            aMask.setLength( 0 );
            aMask.append( "\nendstream\n"
                          "endobj\n\n" );
            CHECK_RETURN( writeBuffer( aMask.getStr(), aMask.getLength() ) );
        }
    }
    aLine.append( ">>\n"
                  "endobj\n\n" );
    CHECK_RETURN( updateObject( rObject.m_nExtGStateObject ) );
    CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );

    return true;
}

bool PDFWriterImpl::writeGradientFunction( GradientEmit& rObject )
{
    sal_Int32 nFunctionObject = createObject();
    CHECK_RETURN( updateObject( nFunctionObject ) );

    OutputDevice* pRefDevice = getReferenceDevice();
    pRefDevice->Push( PUSH_ALL );
    if( rObject.m_aSize.Width() > pRefDevice->GetOutputSizePixel().Width() )
        rObject.m_aSize.Width() = pRefDevice->GetOutputSizePixel().Width();
    if( rObject.m_aSize.Height() > pRefDevice->GetOutputSizePixel().Height() )
        rObject.m_aSize.Height() = pRefDevice->GetOutputSizePixel().Height();
    pRefDevice->SetMapMode( MapMode( MAP_PIXEL ) );
    pRefDevice->DrawGradient( Rectangle( Point( 0, 0 ), rObject.m_aSize ), rObject.m_aGradient );

    Bitmap aSample = pRefDevice->GetBitmap( Point( 0, 0 ), rObject.m_aSize );
    BitmapReadAccess* pAccess = aSample.AcquireReadAccess();
    AccessReleaser aReleaser( pAccess );

    Size aSize = aSample.GetSizePixel();

    sal_Int32 nStreamLengthObject = createObject();
#if OSL_DEBUG_LEVEL > 1
    {
        OStringBuffer aLine( " PDFWriterImpl::writeGradientFunction" );
        emitComment( aLine.getStr() );
    }
#endif
    OStringBuffer aLine( 120 );
    aLine.append( nFunctionObject );
    aLine.append( " 0 obj\n"
                  "<</FunctionType 0\n"
                  "/Domain[ 0 1 0 1 ]\n"
                  "/Size[ " );
    aLine.append( (sal_Int32)aSize.Width() );
    aLine.append( ' ' );
    aLine.append( (sal_Int32)aSize.Height() );
    aLine.append( " ]\n"
                  "/BitsPerSample 8\n"
                  "/Range[ 0 1 0 1 0 1 ]\n"
                  "/Length " );
    aLine.append( nStreamLengthObject );
    aLine.append( " 0 R\n"
#ifndef DEBUG_DISABLE_PDFCOMPRESSION
                  "/Filter/FlateDecode"
#endif
                  ">>\n"
                  "stream\n" );
    CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );

    sal_uInt64 nStartStreamPos = 0;
    CHECK_RETURN( (osl_File_E_None == osl_getFilePos( m_aFile, &nStartStreamPos )) );

    checkAndEnableStreamEncryption( nFunctionObject );
    beginCompression();
    for( int y = 0; y < aSize.Height(); y++ )
    {
        for( int x = 0; x < aSize.Width(); x++ )
        {
            sal_uInt8 aCol[3];
            BitmapColor aColor = pAccess->GetColor( y, x );
            aCol[0] = aColor.GetRed();
            aCol[1] = aColor.GetGreen();
            aCol[2] = aColor.GetBlue();
            CHECK_RETURN( writeBuffer( aCol, 3 ) );
        }
    }
    endCompression();
    disableStreamEncryption();

    sal_uInt64 nEndStreamPos = 0;
    CHECK_RETURN( (osl_File_E_None == osl_getFilePos( m_aFile, &nEndStreamPos )) );

    aLine.setLength( 0 );
    aLine.append( "\nendstream\nendobj\n\n" );
    CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );

    // write stream length
    CHECK_RETURN( updateObject( nStreamLengthObject ) );
    aLine.setLength( 0 );
    aLine.append( nStreamLengthObject );
    aLine.append( " 0 obj\n" );
    aLine.append( (sal_Int64)(nEndStreamPos-nStartStreamPos) );
    aLine.append( "\nendobj\n\n" );
    CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );

    CHECK_RETURN( updateObject( rObject.m_nObject ) );
    aLine.setLength( 0 );
    aLine.append( rObject.m_nObject );
    aLine.append( " 0 obj\n"
                  "<</ShadingType 1\n"
                  "/ColorSpace/DeviceRGB\n"
                  "/AntiAlias true\n"
                  "/Domain[ 0 1 0 1 ]\n"
                  "/Matrix[ " );
    aLine.append( (sal_Int32)aSize.Width() );
    aLine.append( " 0 0 " );
    aLine.append( (sal_Int32)aSize.Height() );
    aLine.append( " 0 0 ]\n"
                  "/Function " );
    aLine.append( nFunctionObject );
    aLine.append( " 0 R\n"
                  ">>\n"
                  "endobj\n\n" );
    CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );

    pRefDevice->Pop();

    return true;
}

bool PDFWriterImpl::writeJPG( JPGEmit& rObject )
{
    CHECK_RETURN( rObject.m_pStream );
    CHECK_RETURN( updateObject( rObject.m_nObject ) );

    sal_Int32 nLength = 0;
    rObject.m_pStream->Seek( STREAM_SEEK_TO_END );
    nLength = rObject.m_pStream->Tell();
    rObject.m_pStream->Seek( STREAM_SEEK_TO_BEGIN );

    sal_Int32 nMaskObject = 0;
    if( !!rObject.m_aMask )
    {
        if( rObject.m_aMask.GetBitCount() == 1 ||
            ( rObject.m_aMask.GetBitCount() == 8 && m_aContext.Version >= PDFWriter::PDF_1_4 && !m_bIsPDF_A1 )//i59651
            )
        {
            nMaskObject = createObject();
        }
        else if( m_bIsPDF_A1 )
            m_aErrors.insert( PDFWriter::Warning_Transparency_Omitted_PDFA );
        else if( m_aContext.Version < PDFWriter::PDF_1_4 )
            m_aErrors.insert( PDFWriter::Warning_Transparency_Omitted_PDF13 );

    }
#if OSL_DEBUG_LEVEL > 1
    {
        OStringBuffer aLine( " PDFWriterImpl::writeJPG" );
        emitComment( aLine.getStr() );
    }
#endif

    OStringBuffer aLine(200);
    aLine.append( rObject.m_nObject );
    aLine.append( " 0 obj\n"
                  "<</Type/XObject/Subtype/Image/Width " );
    aLine.append( (sal_Int32)rObject.m_aID.m_aPixelSize.Width() );
    aLine.append( " /Height " );
    aLine.append( (sal_Int32)rObject.m_aID.m_aPixelSize.Height() );
    aLine.append( " /BitsPerComponent 8 " );
    if( rObject.m_bTrueColor )
        aLine.append( "/ColorSpace/DeviceRGB" );
    else
        aLine.append( "/ColorSpace/DeviceGray" );
    aLine.append( "/Filter/DCTDecode/Length " );
    aLine.append( nLength );
    if( nMaskObject )
    {
        aLine.append( rObject.m_aMask.GetBitCount() == 1 ? " /Mask " : " /SMask " );
        aLine.append( nMaskObject );
        aLine.append( " 0 R " );
    }
    aLine.append( ">>\nstream\n" );
    CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );

    checkAndEnableStreamEncryption( rObject.m_nObject );
    CHECK_RETURN( writeBuffer( rObject.m_pStream->GetData(), nLength ) );
    disableStreamEncryption();

    aLine.setLength( 0 );
    aLine.append( "\nendstream\nendobj\n\n" );
    CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );

    if( nMaskObject )
    {
        BitmapEmit aEmit;
        aEmit.m_nObject = nMaskObject;
        if( rObject.m_aMask.GetBitCount() == 1 )
            aEmit.m_aBitmap = BitmapEx( rObject.m_aMask, rObject.m_aMask );
        else if( rObject.m_aMask.GetBitCount() == 8 )
            aEmit.m_aBitmap = BitmapEx( rObject.m_aMask, AlphaMask( rObject.m_aMask ) );
        writeBitmapObject( aEmit, true );
    }

    return true;
}

bool PDFWriterImpl::writeBitmapObject( BitmapEmit& rObject, bool bMask )
{
    CHECK_RETURN( updateObject( rObject.m_nObject ) );

    Bitmap	aBitmap;
    Color	aTransparentColor( COL_TRANSPARENT );
    bool	bWriteMask = false;
    if( ! bMask )
    {
        aBitmap = rObject.m_aBitmap.GetBitmap();
        if( rObject.m_aBitmap.IsAlpha() )
        {
            if( m_aContext.Version >= PDFWriter::PDF_1_4 )
                bWriteMask = true;
            // else draw without alpha channel
        }
        else
        {
            switch( rObject.m_aBitmap.GetTransparentType() )
            {
                case TRANSPARENT_NONE:
                    // comes from drawMask function
                    if( aBitmap.GetBitCount() == 1 && rObject.m_bDrawMask )
                        bMask = true;
                    break;
                case TRANSPARENT_COLOR:
                    aTransparentColor = rObject.m_aBitmap.GetTransparentColor();
                    break;
                case TRANSPARENT_BITMAP:
                    bWriteMask = true;
                    break;
            }
        }
    }
    else
    {
        if( m_aContext.Version < PDFWriter::PDF_1_4 || ! rObject.m_aBitmap.IsAlpha() )
        {
            aBitmap = rObject.m_aBitmap.GetMask();
            aBitmap.Convert( BMP_CONVERSION_1BIT_THRESHOLD );
            DBG_ASSERT( aBitmap.GetBitCount() == 1, "mask conversion failed" );
        }
        else if( aBitmap.GetBitCount() != 8 )
        {
            aBitmap = rObject.m_aBitmap.GetAlpha().GetBitmap();
            aBitmap.Convert( BMP_CONVERSION_8BIT_GREYS );
            DBG_ASSERT( aBitmap.GetBitCount() == 8, "alpha mask conversion failed" );
        }
    }

    BitmapReadAccess* pAccess = aBitmap.AcquireReadAccess();
    AccessReleaser aReleaser( pAccess );

    bool bTrueColor;
    sal_Int32 nBitsPerComponent;
    switch( aBitmap.GetBitCount() )
    {
        case 1:
        case 2:
        case 4:
        case 8:
            bTrueColor = false;
            nBitsPerComponent = aBitmap.GetBitCount();
            break;
        default:
            bTrueColor = true;
            nBitsPerComponent = 8;
            break;
    }

    sal_Int32 nStreamLengthObject	= createObject();
    sal_Int32 nMaskObject			= 0;

#if OSL_DEBUG_LEVEL > 1
    {
        OStringBuffer aLine( " PDFWriterImpl::writeBitmapObject" );
        emitComment( aLine.getStr() );
    }
#endif
    OStringBuffer aLine(1024);
    aLine.append( rObject.m_nObject );
    aLine.append( " 0 obj\n"
                  "<</Type/XObject/Subtype/Image/Width " );
    aLine.append( (sal_Int32)aBitmap.GetSizePixel().Width() );
    aLine.append( " /Height " );
    aLine.append( (sal_Int32)aBitmap.GetSizePixel().Height() );
    aLine.append( " /BitsPerComponent " );
    aLine.append( nBitsPerComponent );
    aLine.append( " /Length " );
    aLine.append( nStreamLengthObject );
    aLine.append( " 0 R\n" );
#ifndef DEBUG_DISABLE_PDFCOMPRESSION
    aLine.append( "/Filter/FlateDecode" );
#endif
    if( ! bMask )
    {
        aLine.append( "/ColorSpace" );
        if( bTrueColor )
            aLine.append( "/DeviceRGB\n" );
        else if( aBitmap.HasGreyPalette() )
        {
            aLine.append( "/DeviceGray\n" );
            if( aBitmap.GetBitCount() == 1 )
            {
                // #i47395# 1 bit bitmaps occasionally have an inverted grey palette
                sal_Int32 nBlackIndex = pAccess->GetBestPaletteIndex( BitmapColor( Color( COL_BLACK ) ) );
                DBG_ASSERT( nBlackIndex == 0 || nBlackIndex == 1, "wrong black index" );
                if( nBlackIndex == 1 )
                    aLine.append( "/Decode[1 0]\n" );
            }
        }
        else
        {
            aLine.append( "[ /Indexed/DeviceRGB " );
            aLine.append( (sal_Int32)(pAccess->GetPaletteEntryCount()-1) );
            aLine.append( "\n<" );
			if( m_aContext.Encrypt )
			{
				enableStringEncryption( rObject.m_nObject );
				//check encryption buffer size
				if( checkEncryptionBufferSize( pAccess->GetPaletteEntryCount()*3 ) )
				{
					int	nChar = 0;
					//fill the encryption buffer
					for( USHORT i = 0; i < pAccess->GetPaletteEntryCount(); i++ )
					{
						const BitmapColor& rColor = pAccess->GetPaletteColor( i );
						m_pEncryptionBuffer[nChar++] = rColor.GetRed();
						m_pEncryptionBuffer[nChar++] = rColor.GetGreen();
						m_pEncryptionBuffer[nChar++] = rColor.GetBlue();
					}
					//encrypt the colorspace lookup table
					rtl_cipher_encodeARCFOUR( m_aCipher, m_pEncryptionBuffer, nChar, m_pEncryptionBuffer, nChar );
					//now queue the data for output
                    nChar = 0;
					for( USHORT i = 0; i < pAccess->GetPaletteEntryCount(); i++ )
					{
						appendHex(m_pEncryptionBuffer[nChar++], aLine );
						appendHex(m_pEncryptionBuffer[nChar++], aLine );
						appendHex(m_pEncryptionBuffer[nChar++], aLine );
					}
				}
			}
			else //no encryption requested (PDF/A-1a program flow drops here)
			{
				for( USHORT i = 0; i < pAccess->GetPaletteEntryCount(); i++ )
				{
					const BitmapColor& rColor = pAccess->GetPaletteColor( i );
					appendHex( rColor.GetRed(), aLine );
					appendHex( rColor.GetGreen(), aLine );
					appendHex( rColor.GetBlue(), aLine );
				}
			}
            aLine.append( ">\n]\n" );
        }
    }
    else
    {
        if( aBitmap.GetBitCount() == 1 )
        {
            aLine.append( " /ImageMask true\n" );
            sal_Int32 nBlackIndex = pAccess->GetBestPaletteIndex( BitmapColor( Color( COL_BLACK ) ) );
            DBG_ASSERT( nBlackIndex == 0 || nBlackIndex == 1, "wrong black index" );
            if( nBlackIndex )
                aLine.append( "/Decode[ 1 0 ]\n" );
            else
                aLine.append( "/Decode[ 0 1 ]\n" );
        }
        else if( aBitmap.GetBitCount() == 8 )
        {
            aLine.append( "/ColorSpace/DeviceGray\n"
                          "/Decode [ 1 0 ]\n" );
        }
    }

    if( ! bMask && m_aContext.Version > PDFWriter::PDF_1_2 && !m_bIsPDF_A1 )//i59651
    {
        if( bWriteMask )
        {
            nMaskObject = createObject();
            if( rObject.m_aBitmap.IsAlpha() && m_aContext.Version > PDFWriter::PDF_1_3 )
                aLine.append( "/SMask " );
            else
                aLine.append( "/Mask " );
            aLine.append( nMaskObject );
            aLine.append( " 0 R\n" );
        }
        else if( aTransparentColor != Color( COL_TRANSPARENT ) )
        {
            aLine.append( "/Mask[ " );
            if( bTrueColor )
            {
                aLine.append( (sal_Int32)aTransparentColor.GetRed() );
                aLine.append( ' ' );
                aLine.append( (sal_Int32)aTransparentColor.GetRed() );
                aLine.append( ' ' );
                aLine.append( (sal_Int32)aTransparentColor.GetGreen() );
                aLine.append( ' ' );
                aLine.append( (sal_Int32)aTransparentColor.GetGreen() );
                aLine.append( ' ' );
                aLine.append( (sal_Int32)aTransparentColor.GetBlue() );
                aLine.append( ' ' );
                aLine.append( (sal_Int32)aTransparentColor.GetBlue() );
            }
            else
            {
                sal_Int32 nIndex = pAccess->GetBestPaletteIndex( BitmapColor( aTransparentColor ) );
                aLine.append( nIndex );
            }
            aLine.append( " ]\n" );
        }
    }
    else if( m_bIsPDF_A1 )
        m_aErrors.insert( PDFWriter::Warning_Transparency_Omitted_PDFA );

    aLine.append( ">>\n"
                  "stream\n" );
    CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );
    sal_uInt64 nStartPos = 0;
    CHECK_RETURN( (osl_File_E_None == osl_getFilePos( m_aFile, &nStartPos )) );

    checkAndEnableStreamEncryption(  rObject.m_nObject );
    beginCompression();
#if defined USE_JAVA && defined MACOSX
    if( ! bTrueColor )
#else	// USE_JAVA && MACOSX
    if( ! bTrueColor || pAccess->GetScanlineFormat() == BMP_FORMAT_24BIT_TC_RGB )
#endif	// USE_JAVA && MACOSX
    {
        const int nScanLineBytes = 1 + ( pAccess->GetBitCount() * ( pAccess->Width() - 1 ) / 8U );

        for( int i = 0; i < pAccess->Height(); i++ )
        {
            CHECK_RETURN( writeBuffer( pAccess->GetScanline( i ), nScanLineBytes ) );
        }
    }
    else
    {
        const int nScanLineBytes = pAccess->Width()*3;
        sal_uInt8 *pCol = (sal_uInt8*)rtl_allocateMemory( nScanLineBytes );
        for( int y = 0; y < pAccess->Height(); y++ )
        {
            for( int x = 0; x < pAccess->Width(); x++ )
            {
                BitmapColor aColor = pAccess->GetColor( y, x );
                pCol[3*x+0] = aColor.GetRed();
                pCol[3*x+1] = aColor.GetGreen();
                pCol[3*x+2] = aColor.GetBlue();
            }
            CHECK_RETURN( writeBuffer( pCol, nScanLineBytes ) );
        }
        rtl_freeMemory( pCol );
    }
    endCompression();
    disableStreamEncryption();

    sal_uInt64 nEndPos = 0;
    CHECK_RETURN( (osl_File_E_None == osl_getFilePos( m_aFile, &nEndPos )) );
    aLine.setLength( 0 );
    aLine.append( "\nendstream\nendobj\n\n" );
    CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );
    CHECK_RETURN( updateObject( nStreamLengthObject ) );
    aLine.setLength( 0 );
    aLine.append( nStreamLengthObject );
    aLine.append( " 0 obj\n" );
    aLine.append( (sal_Int64)(nEndPos-nStartPos) );
    aLine.append( "\nendobj\n\n" );
    CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );

    if( nMaskObject )
    {
        BitmapEmit aEmit;
        aEmit.m_nObject				= nMaskObject;
        aEmit.m_aBitmap				= rObject.m_aBitmap;
        return writeBitmapObject( aEmit, true );
    }

    return true;
}

void PDFWriterImpl::drawJPGBitmap( SvStream& rDCTData, bool bIsTrueColor, const Size& rSizePixel, const Rectangle& rTargetArea, const Bitmap& rMask )
{
    MARK( "drawJPGBitmap" );

    OStringBuffer aLine( 80 );
    updateGraphicsState();

    // #i40055# sanity check
    if( ! (rTargetArea.GetWidth() && rTargetArea.GetHeight() ) )
        return;
    if( ! (rSizePixel.Width() && rSizePixel.Height()) )
        return;

#if defined USE_JAVA && defined MACOSX
    if ( !isReplayWriter() )
        return;
#endif	// USE_JAVA && MACOSX

    SvMemoryStream* pStream = new SvMemoryStream;
    rDCTData.Seek( 0 );
    *pStream << rDCTData;
    pStream->Seek( STREAM_SEEK_TO_END );

    BitmapID aID;
    aID.m_aPixelSize	= rSizePixel;
    aID.m_nSize			= pStream->Tell();
    pStream->Seek( STREAM_SEEK_TO_BEGIN );
    aID.m_nChecksum		= rtl_crc32( 0, pStream->GetData(), aID.m_nSize );
    if( ! rMask.IsEmpty() )
        aID.m_nMaskChecksum	= rMask.GetChecksum();

    std::list< JPGEmit >::const_iterator it;
    for( it = m_aJPGs.begin(); it != m_aJPGs.end() && ! (aID == it->m_aID); ++it )
        ;
    if( it == m_aJPGs.end() )
    {
        m_aJPGs.push_front( JPGEmit() );
        JPGEmit& rEmit = m_aJPGs.front();
        rEmit.m_nObject		= createObject();
        rEmit.m_aID			= aID;
        rEmit.m_pStream		= pStream;
        rEmit.m_bTrueColor  = bIsTrueColor;
        if( !! rMask && rMask.GetSizePixel() == rSizePixel )
            rEmit.m_aMask	= rMask;

        it = m_aJPGs.begin();
    }
    else
        delete pStream;

    aLine.append( "q " );
    m_aPages.back().appendMappedLength( (sal_Int32)rTargetArea.GetWidth(), aLine, false );
    aLine.append( " 0 0 " );
    m_aPages.back().appendMappedLength( (sal_Int32)rTargetArea.GetHeight(), aLine, true );
    aLine.append( ' ' );
    m_aPages.back().appendPoint( rTargetArea.BottomLeft(), aLine );
    aLine.append( " cm\n/Im" );
    aLine.append( it->m_nObject );
    aLine.append( " Do Q\n" );
    writeBuffer( aLine.getStr(), aLine.getLength() );

    OStringBuffer aObjName( 16 );
    aObjName.append( "Im" );
    aObjName.append( it->m_nObject );
    pushResource( ResXObject, aObjName.makeStringAndClear(), it->m_nObject );

}

void PDFWriterImpl::drawBitmap( const Point& rDestPoint, const Size& rDestSize, const BitmapEmit& rBitmap, const Color& rFillColor )
{
    OStringBuffer aLine( 80 );
    updateGraphicsState();

    aLine.append( "q " );
    if( rFillColor != Color( COL_TRANSPARENT ) )
    {
        appendNonStrokingColor( rFillColor, aLine );
        aLine.append( ' ' );
    }
    m_aPages.back().appendMappedLength( (sal_Int32)rDestSize.Width(), aLine, false );
    aLine.append( " 0 0 " );
    m_aPages.back().appendMappedLength( (sal_Int32)rDestSize.Height(), aLine, true );
    aLine.append( ' ' );
    m_aPages.back().appendPoint( rDestPoint + Point( 0, rDestSize.Height()-1 ), aLine );
    aLine.append( " cm\n/Im" );
    aLine.append( rBitmap.m_nObject );
    aLine.append( " Do Q\n" );
    writeBuffer( aLine.getStr(), aLine.getLength() );
}

const PDFWriterImpl::BitmapEmit& PDFWriterImpl::createBitmapEmit( const BitmapEx& rBitmap, bool bDrawMask )
{
    BitmapID aID;
    aID.m_aPixelSize		= rBitmap.GetSizePixel();
    aID.m_nSize				= rBitmap.GetBitCount();
    aID.m_nChecksum			= rBitmap.GetBitmap().GetChecksum();
    aID.m_nMaskChecksum		= 0;
    if( rBitmap.IsAlpha() )
        aID.m_nMaskChecksum = rBitmap.GetAlpha().GetChecksum();
    else
    {
        Bitmap aMask = rBitmap.GetMask();
        if( ! aMask.IsEmpty() )
            aID.m_nMaskChecksum = aMask.GetChecksum();
    }
    std::list< BitmapEmit >::const_iterator it;
    for( it = m_aBitmaps.begin(); it != m_aBitmaps.end(); ++it )
    {
        if( aID == it->m_aID )
            break;
    }
    if( it == m_aBitmaps.end() )
    {
        m_aBitmaps.push_front( BitmapEmit() );
        m_aBitmaps.front().m_aID		= aID;
        m_aBitmaps.front().m_aBitmap	= rBitmap;
        m_aBitmaps.front().m_nObject	= createObject();
        m_aBitmaps.front().m_bDrawMask	= bDrawMask;
        it = m_aBitmaps.begin();
    }

    OStringBuffer aObjName( 16 );
    aObjName.append( "Im" );
    aObjName.append( it->m_nObject );
    pushResource( ResXObject, aObjName.makeStringAndClear(), it->m_nObject );

    return *it;
}

void PDFWriterImpl::drawBitmap( const Point& rDestPoint, const Size& rDestSize, const Bitmap& rBitmap )
{
    MARK( "drawBitmap (Bitmap)" );

    // #i40055# sanity check
    if( ! (rDestSize.Width() && rDestSize.Height()) )
        return;

    const BitmapEmit& rEmit = createBitmapEmit( BitmapEx( rBitmap ) );
    drawBitmap( rDestPoint, rDestSize, rEmit, Color( COL_TRANSPARENT ) );
}

void PDFWriterImpl::drawBitmap( const Point& rDestPoint, const Size& rDestSize, const BitmapEx& rBitmap )
{
    MARK( "drawBitmap (BitmapEx)" );

    // #i40055# sanity check
    if( ! (rDestSize.Width() && rDestSize.Height()) )
        return;

    const BitmapEmit& rEmit = createBitmapEmit( rBitmap );
    drawBitmap( rDestPoint, rDestSize, rEmit, Color( COL_TRANSPARENT ) );
}

void PDFWriterImpl::drawMask( const Point& rDestPoint, const Size& rDestSize, const Bitmap& rBitmap, const Color& rFillColor )
{
    MARK( "drawMask" );

    // #i40055# sanity check
    if( ! (rDestSize.Width() && rDestSize.Height()) )
        return;

    Bitmap aBitmap( rBitmap );
    if( aBitmap.GetBitCount() > 1 )
        aBitmap.Convert( BMP_CONVERSION_1BIT_THRESHOLD );
    DBG_ASSERT( aBitmap.GetBitCount() == 1, "mask conversion failed" );

    const BitmapEmit& rEmit = createBitmapEmit( BitmapEx( aBitmap ), true );
    drawBitmap( rDestPoint, rDestSize, rEmit, rFillColor );
}

sal_Int32 PDFWriterImpl::createGradient( const Gradient& rGradient, const Size& rSize )
{
    Size aPtSize( lcl_convert( m_aGraphicsStack.front().m_aMapMode,
                               MapMode( MAP_POINT ),
                               getReferenceDevice(),
                               rSize ) );
    // check if we already have this gradient
    std::list<GradientEmit>::iterator it;
    for( it = m_aGradients.begin(); it != m_aGradients.end(); ++it )
    {
        if( it->m_aGradient == rGradient )
        {
            if( it->m_aSize.Width() < aPtSize.Width() )
                it->m_aSize.Width() = aPtSize.Width();
            if( it->m_aSize.Height() <= aPtSize.Height() )
                it->m_aSize.Height() = aPtSize.Height();
           break;
        }
    }
    if( it == m_aGradients.end() )
    {
        m_aGradients.push_front( GradientEmit() );
        m_aGradients.front().m_aGradient	= rGradient;
        m_aGradients.front().m_nObject	    = createObject();
        m_aGradients.front().m_aSize		= aPtSize;
        it = m_aGradients.begin();
    }

    OStringBuffer aObjName( 16 );
    aObjName.append( 'P' );
    aObjName.append( it->m_nObject );
    pushResource( ResShading, aObjName.makeStringAndClear(), it->m_nObject );

    return it->m_nObject;
}

void PDFWriterImpl::drawGradient( const Rectangle& rRect, const Gradient& rGradient )
{
    MARK( "drawGradient (Rectangle)" );

    if( m_aContext.Version == PDFWriter::PDF_1_2 )
    {
        drawRectangle( rRect );
        return;
    }

    sal_Int32 nGradient = createGradient( rGradient, rRect.GetSize() );

    Point aTranslate( rRect.BottomLeft() );
    aTranslate += Point( 0, 1 );

    updateGraphicsState();

    OStringBuffer aLine( 80 );
    aLine.append( "q 1 0 0 1 " );
    m_aPages.back().appendPoint( aTranslate, aLine );
    aLine.append( " cm " );
    // if a stroke is appended reset the clip region before stroke
    if( m_aGraphicsStack.front().m_aLineColor != Color( COL_TRANSPARENT ) )
        aLine.append( "q " );
    aLine.append( "0 0 " );
    m_aPages.back().appendMappedLength( (sal_Int32)rRect.GetWidth(), aLine, false );
    aLine.append( ' ' );
    m_aPages.back().appendMappedLength( (sal_Int32)rRect.GetHeight(), aLine, true );
    aLine.append( " re W n\n" );

    aLine.append( "/P" );
    aLine.append( nGradient );
    aLine.append( " sh " );
    if( m_aGraphicsStack.front().m_aLineColor != Color( COL_TRANSPARENT ) )
    {
        aLine.append( "Q 0 0 " );
        m_aPages.back().appendMappedLength( (sal_Int32)rRect.GetWidth(), aLine, false );
        aLine.append( ' ' );
        m_aPages.back().appendMappedLength( (sal_Int32)rRect.GetHeight(), aLine, true );
        aLine.append( " re S " );
    }
    aLine.append( "Q\n" );
    writeBuffer( aLine.getStr(), aLine.getLength() );
}

void PDFWriterImpl::drawGradient( const PolyPolygon& rPolyPoly, const Gradient& rGradient )
{
    MARK( "drawGradient (PolyPolygon)" );

    if( m_aContext.Version == PDFWriter::PDF_1_2 )
    {
        drawPolyPolygon( rPolyPoly );
        return;
    }

    sal_Int32 nGradient = createGradient( rGradient, rPolyPoly.GetBoundRect().GetSize() );

    updateGraphicsState();

    Rectangle aBoundRect = rPolyPoly.GetBoundRect();
    Point aTranslate = aBoundRect.BottomLeft() + Point( 0, 1 );
    int nPolygons = rPolyPoly.Count();

    OStringBuffer aLine( 80*nPolygons );
    aLine.append( "q " );
    // set PolyPolygon as clip path
    m_aPages.back().appendPolyPolygon( rPolyPoly, aLine );
    aLine.append( "W* n\n" );
    aLine.append( "1 0 0 1 " );
    m_aPages.back().appendPoint( aTranslate, aLine );
    aLine.append( " cm\n" );
    aLine.append( "/P" );
    aLine.append( nGradient );
    aLine.append( " sh Q\n" );
    if( m_aGraphicsStack.front().m_aLineColor != Color( COL_TRANSPARENT ) )
    {
        // and draw the surrounding path
        m_aPages.back().appendPolyPolygon( rPolyPoly, aLine );
        aLine.append( "S\n" );
    }
    writeBuffer( aLine.getStr(), aLine.getLength() );
}

void PDFWriterImpl::drawHatch( const PolyPolygon& rPolyPoly, const Hatch& rHatch )
{
    MARK( "drawHatch" );

    updateGraphicsState();

    if( m_aGraphicsStack.front().m_aLineColor == Color( COL_TRANSPARENT ) &&
        m_aGraphicsStack.front().m_aFillColor == Color( COL_TRANSPARENT ) )
        return;
	if( rPolyPoly.Count() )
	{
		PolyPolygon		aPolyPoly( rPolyPoly );

		aPolyPoly.Optimize( POLY_OPTIMIZE_NO_SAME );
		push( PUSH_LINECOLOR );
		setLineColor( rHatch.GetColor() );
		getReferenceDevice()->ImplDrawHatch( aPolyPoly, rHatch, FALSE );
		pop();
	}
}

void PDFWriterImpl::drawWallpaper( const Rectangle& rRect, const Wallpaper& rWall )
{
    MARK( "drawWallpaper" );

    bool bDrawColor			= false;
    bool bDrawGradient		= false;
    bool bDrawBitmap		= false;

    BitmapEx aBitmap;
    Point aBmpPos = rRect.TopLeft();
    Size aBmpSize;
    if( rWall.IsBitmap() )
    {
        aBitmap = rWall.GetBitmap();
		aBmpSize = lcl_convert( aBitmap.GetPrefMapMode(),
                                getMapMode(),
                                getReferenceDevice(),
                                aBitmap.GetPrefSize() );
        Rectangle aRect( rRect );
        if( rWall.IsRect() )
        {
            aRect = rWall.GetRect();
            aBmpPos = aRect.TopLeft();
            aBmpSize = aRect.GetSize();
        }
        if( rWall.GetStyle() != WALLPAPER_SCALE )
        {
            if( rWall.GetStyle() != WALLPAPER_TILE )
            {
                bDrawBitmap		= true;
                if( rWall.IsGradient() )
                    bDrawGradient = true;
                else
                    bDrawColor = true;
                switch( rWall.GetStyle() )
                {
                    case WALLPAPER_TOPLEFT:
                        break;
                    case WALLPAPER_TOP:
                        aBmpPos.X() += (aRect.GetWidth()-aBmpSize.Width())/2;
                        break;
                    case WALLPAPER_LEFT:
                        aBmpPos.Y() += (aRect.GetHeight()-aBmpSize.Height())/2;
                        break;
                    case WALLPAPER_TOPRIGHT:
                        aBmpPos.X() += aRect.GetWidth()-aBmpSize.Width();
                        break;
                    case WALLPAPER_CENTER:
                        aBmpPos.X() += (aRect.GetWidth()-aBmpSize.Width())/2;
                        aBmpPos.Y() += (aRect.GetHeight()-aBmpSize.Height())/2;
                        break;
                    case WALLPAPER_RIGHT:
                        aBmpPos.X() += aRect.GetWidth()-aBmpSize.Width();
                        aBmpPos.Y() += (aRect.GetHeight()-aBmpSize.Height())/2;
                        break;
                    case WALLPAPER_BOTTOMLEFT:
                        aBmpPos.Y() += aRect.GetHeight()-aBmpSize.Height();
                        break;
                    case WALLPAPER_BOTTOM:
                        aBmpPos.X() += (aRect.GetWidth()-aBmpSize.Width())/2;
                        aBmpPos.Y() += aRect.GetHeight()-aBmpSize.Height();
                        break;
                    case WALLPAPER_BOTTOMRIGHT:
                        aBmpPos.X() += aRect.GetWidth()-aBmpSize.Width();
                        aBmpPos.Y() += aRect.GetHeight()-aBmpSize.Height();
                        break;
                    default: ;
                }
            }
            else
            {
                // push the bitmap
                const BitmapEmit& rEmit = createBitmapEmit( BitmapEx( aBitmap ) );

                // convert to page coordinates; this needs to be done here
                // since the emit does not know the page anymore
                Rectangle aConvertRect( aBmpPos, aBmpSize );
                m_aPages.back().convertRect( aConvertRect );

                OStringBuffer aNameBuf(16);
                aNameBuf.append( "Im" );
                aNameBuf.append( rEmit.m_nObject );
                OString aImageName( aNameBuf.makeStringAndClear() );

                // push the pattern
                OStringBuffer aTilingStream( 32 );
                appendFixedInt( aConvertRect.GetWidth(), aTilingStream );
                aTilingStream.append( " 0 0 " );
                appendFixedInt( aConvertRect.GetHeight(), aTilingStream );
                aTilingStream.append( " 0 0 cm\n/" );
                aTilingStream.append( aImageName );
                aTilingStream.append( " Do\n" );

                m_aTilings.push_back( TilingEmit() );
                m_aTilings.back().m_nObject			= createObject();
                m_aTilings.back().m_aRectangle		= Rectangle( Point( 0, 0 ), aConvertRect.GetSize() );
                m_aTilings.back().m_pTilingStream   = new SvMemoryStream();
                m_aTilings.back().m_pTilingStream->Write( aTilingStream.getStr(), aTilingStream.getLength() );
                // phase the tiling so wallpaper begins on upper left
                m_aTilings.back().m_aTransform.matrix[2] = double(aConvertRect.Left() % aConvertRect.GetWidth()) / fDivisor;
                m_aTilings.back().m_aTransform.matrix[5] = double(aConvertRect.Top() % aConvertRect.GetHeight()) / fDivisor;
                m_aTilings.back().m_aResources.m_aXObjects[aImageName] = rEmit.m_nObject;

                updateGraphicsState();

                OStringBuffer aObjName( 16 );
                aObjName.append( 'P' );
                aObjName.append( m_aTilings.back().m_nObject );
                OString aPatternName( aObjName.makeStringAndClear() );
                pushResource( ResPattern, aPatternName, m_aTilings.back().m_nObject );

                // fill a rRect with the pattern
                OStringBuffer aLine( 100 );
                aLine.append( "q /Pattern cs /" );
                aLine.append( aPatternName );
                aLine.append( " scn " );
                m_aPages.back().appendRect( rRect, aLine );
                aLine.append( " f Q\n" );
                writeBuffer( aLine.getStr(), aLine.getLength() );
            }
        }
        else
        {
            aBmpPos		= aRect.TopLeft();
            aBmpSize	= aRect.GetSize();
            bDrawBitmap	= true;
        }

        if( aBitmap.IsTransparent() )
        {
            if( rWall.IsGradient() )
                bDrawGradient = true;
            else
                bDrawColor = true;
        }
    }
    else if( rWall.IsGradient() )
        bDrawGradient = true;
    else
        bDrawColor = true;

    if( bDrawGradient )
    {
        drawGradient( rRect, rWall.GetGradient() );
    }
    if( bDrawColor )
    {
        Color aOldLineColor = m_aGraphicsStack.front().m_aLineColor;
        Color aOldFillColor = m_aGraphicsStack.front().m_aFillColor;
        setLineColor( Color( COL_TRANSPARENT ) );
        setFillColor( rWall.GetColor() );
        drawRectangle( rRect );
        setLineColor( aOldLineColor );
        setFillColor( aOldFillColor );
    }
    if( bDrawBitmap )
    {
        // set temporary clip region since aBmpPos and aBmpSize
        // may be outside rRect
        OStringBuffer aLine( 20 );
        aLine.append( "q " );
        m_aPages.back().appendRect( rRect, aLine );
        aLine.append( " W n\n" );
        writeBuffer( aLine.getStr(), aLine.getLength() );
        drawBitmap( aBmpPos, aBmpSize, aBitmap );
        writeBuffer( "Q\n", 2 );
    }
}

void PDFWriterImpl::beginPattern( const Rectangle& rCellRect )
{
    beginRedirect( new SvMemoryStream(), rCellRect );
}

sal_Int32 PDFWriterImpl::endPattern( const SvtGraphicFill::Transform& rTransform )
{
    Rectangle aConvertRect( getRedirectTargetRect() );
    DBG_ASSERT( aConvertRect.GetWidth() != 0 && aConvertRect.GetHeight() != 0, "empty cell rectangle in pattern" );

    // get scaling between current mapmode and PDF output
    Size aScaling( lcl_convert( m_aGraphicsStack.front().m_aMapMode, m_aMapMode, getReferenceDevice(), Size( 10000, 10000 ) ) );
    double fSX = (double(aScaling.Width()) / 10000.0);
    double fSY = (double(aScaling.Height()) / 10000.0);

    // transform translation part of matrix
    Size aTranslation( (long)rTransform.matrix[2], (long)rTransform.matrix[5] );
    aTranslation = lcl_convert( m_aGraphicsStack.front().m_aMapMode, m_aMapMode, getReferenceDevice(), aTranslation );

    sal_Int32 nTilingId = m_aTilings.size();
    m_aTilings.push_back( TilingEmit() );
    TilingEmit& rTile = m_aTilings.back();
    rTile.m_nObject         = createObject();
    rTile.m_aResources      = m_aOutputStreams.front().m_aResourceDict;
    rTile.m_aTransform.matrix[0] = rTransform.matrix[0] * fSX;
    rTile.m_aTransform.matrix[1] = rTransform.matrix[1] * fSY;
    rTile.m_aTransform.matrix[2] = aTranslation.Width();
    rTile.m_aTransform.matrix[3] = rTransform.matrix[3] * fSX;
    rTile.m_aTransform.matrix[4] = rTransform.matrix[4] * fSY;
    rTile.m_aTransform.matrix[5] = -aTranslation.Height();
    // caution: endRedirect pops the stream, so do this last
    rTile.m_pTilingStream   = dynamic_cast<SvMemoryStream*>(endRedirect());
    // FIXME: bound rect will not work with rotated matrix
    rTile.m_aRectangle      = Rectangle( Point(0,0), aConvertRect.GetSize() );
    rTile.m_aCellSize       = aConvertRect.GetSize();

    OStringBuffer aObjName( 16 );
    aObjName.append( 'P' );
    aObjName.append( rTile.m_nObject );
    pushResource( ResPattern, aObjName.makeStringAndClear(), rTile.m_nObject );
    return nTilingId;
}

void PDFWriterImpl::drawPolyPolygon( const PolyPolygon& rPolyPoly, sal_Int32 nPattern, bool bEOFill )
{
    if( nPattern < 0 || nPattern >= (sal_Int32)m_aTilings.size() )
        return;

    m_aPages.back().endStream();
    sal_Int32 nXObject = createObject();
    OStringBuffer aNameBuf( 16 );
    aNameBuf.append( "Pol" );
    aNameBuf.append( nXObject );
    OString aObjName( aNameBuf.makeStringAndClear() );
    Rectangle aObjRect;
    if( updateObject( nXObject ) )
    {
        // get bounding rect of object
        PolyPolygon aSubDiv;
        rPolyPoly.AdaptiveSubdivide( aSubDiv );
        aObjRect = aSubDiv.GetBoundRect();
        Rectangle aConvObjRect( aObjRect );
        m_aPages.back().convertRect( aConvObjRect );

        // move polypolygon to bottom left of page
        PolyPolygon aLocalPath( rPolyPoly );
        sal_Int32 nPgWd = getReferenceDevice()->ImplGetDPIX() * m_aPages.back().getWidth() / 72;
        sal_Int32 nPgHt = getReferenceDevice()->ImplGetDPIY() * m_aPages.back().getHeight() / 72;
        Size aLogicPgSz = getReferenceDevice()->PixelToLogic( Size( nPgWd, nPgHt ), m_aGraphicsStack.front().m_aMapMode );
        sal_Int32 nXOff = aObjRect.Left();
        sal_Int32 nYOff = aLogicPgSz.Height() - aObjRect.Bottom();
        aLocalPath.Move( -nXOff, nYOff );

        // prepare XObject's content stream
        OStringBuffer aStream( 512 );
        aStream.append( "/Pattern cs /P" );
        aStream.append( m_aTilings[ nPattern ].m_nObject );
        aStream.append( " scn\n" );
        m_aPages.back().appendPolyPolygon( aLocalPath, aStream );
        aStream.append( bEOFill ? "f*" : "f" );
        SvMemoryStream aMemStream( aStream.getLength() );
        aMemStream.Write( aStream.getStr(), aStream.getLength() );
        bool bDeflate = compressStream( &aMemStream );
        aMemStream.Seek( STREAM_SEEK_TO_END );
        sal_Int32 nStreamLen = (sal_Int32)aMemStream.Tell();
        aMemStream.Seek( STREAM_SEEK_TO_BEGIN );

        // add new XObject to global resource dict
        m_aGlobalResourceDict.m_aXObjects[ aObjName ] = nXObject;

        // write XObject
        OStringBuffer aLine( 512 );
        aLine.append( nXObject );
        aLine.append( " 0 obj\n"
                      "<</Type/XObject/Subtype/Form/BBox[0 0 " );
        appendFixedInt( aConvObjRect.GetWidth(), aLine );
        aLine.append( ' ' );
        appendFixedInt( aConvObjRect.GetHeight(), aLine );
        aLine.append( "]/Length " );
        aLine.append( nStreamLen );
        if( bDeflate )
            aLine.append( "/Filter/FlateDecode" );
        aLine.append( ">>\n"
                      "stream\n" );
        writeBuffer( aLine.getStr(), aLine.getLength() );
        checkAndEnableStreamEncryption( nXObject );
        writeBuffer( aMemStream.GetData(), nStreamLen );
        disableStreamEncryption();
        writeBuffer( "\nendstream\nendobj\n\n", 19 );
    }
    m_aPages.back().beginStream();
    OStringBuffer aLine( 80 );
    aLine.append( "q 1 0 0 1 " );
    m_aPages.back().appendPoint( aObjRect.BottomLeft(), aLine );
    aLine.append( " cm/" );
    aLine.append( aObjName );
    aLine.append( " Do Q\n" );
    writeBuffer( aLine.getStr(), aLine.getLength() );
}

void PDFWriterImpl::updateGraphicsState()
{
    OStringBuffer aLine( 256 );
    GraphicsState& rNewState = m_aGraphicsStack.front();
    // first set clip region since it might invalidate everything else

    if( (rNewState.m_nUpdateFlags & GraphicsState::updateClipRegion) )
    {
        rNewState.m_nUpdateFlags &= ~GraphicsState::updateClipRegion;

        Region& rNewClip = rNewState.m_aClipRegion;

        /*  #103137# equality operator is not implemented
        *  const as API promises but may change Region
        *  from Polygon to rectangles. Arrrgghh !!!!
        */
        Region aLeft = m_aCurrentPDFState.m_aClipRegion;
        Region aRight = rNewClip;
        if( aLeft != aRight )
        {
            if( ! m_aCurrentPDFState.m_aClipRegion.IsEmpty() &&
                ! m_aCurrentPDFState.m_aClipRegion.IsNull() )
            {
                aLine.append( "Q " );
                // invalidate everything but the clip region
                m_aCurrentPDFState = GraphicsState();
                rNewState.m_nUpdateFlags = sal::static_int_cast<sal_uInt16>(~GraphicsState::updateClipRegion);
            }
            if( ! rNewClip.IsEmpty() && ! rNewClip.IsNull() )
            {
                // clip region is always stored in private PDF mapmode
                MapMode aNewMapMode = rNewState.m_aMapMode;
                rNewState.m_aMapMode = m_aMapMode;
                getReferenceDevice()->SetMapMode( rNewState.m_aMapMode );
                m_aCurrentPDFState.m_aMapMode = rNewState.m_aMapMode;

                aLine.append( "q " );
                if( rNewClip.HasPolyPolygon() )
                {
                    m_aPages.back().appendPolyPolygon( rNewClip.GetPolyPolygon(), aLine );
                    aLine.append( "W* n\n" );
                }
                else
                {
                    // need to clip all rectangles
                    RegionHandle aHandle = rNewClip.BeginEnumRects();
                    Rectangle aRect;
                    while( rNewClip.GetNextEnumRect( aHandle, aRect ) )
                    {
                        m_aPages.back().appendRect( aRect, aLine );
                        if( aLine.getLength() > 80 )
                        {
                            aLine.append( "\n" );
                            writeBuffer( aLine.getStr(), aLine.getLength() );
                            aLine.setLength( 0 );
                        }
                        else
                            aLine.append( ' ' );
                    }
                    rNewClip.EndEnumRects( aHandle );
                    aLine.append( "W* n\n" );
                }

                rNewState.m_aMapMode = aNewMapMode;
                getReferenceDevice()->SetMapMode( rNewState.m_aMapMode );
                m_aCurrentPDFState.m_aMapMode = rNewState.m_aMapMode;
            }
        }
    }

    if( (rNewState.m_nUpdateFlags & GraphicsState::updateMapMode) )
    {
        rNewState.m_nUpdateFlags &= ~GraphicsState::updateMapMode;
        getReferenceDevice()->SetMapMode( rNewState.m_aMapMode );
    }

    if( (rNewState.m_nUpdateFlags & GraphicsState::updateFont) )
    {
        rNewState.m_nUpdateFlags &= ~GraphicsState::updateFont;
        getReferenceDevice()->SetFont( rNewState.m_aFont );
        getReferenceDevice()->ImplNewFont();
    }

    if( (rNewState.m_nUpdateFlags & GraphicsState::updateLayoutMode) )
    {
        rNewState.m_nUpdateFlags &= ~GraphicsState::updateLayoutMode;
        getReferenceDevice()->SetLayoutMode( rNewState.m_nLayoutMode );
    }

    if( (rNewState.m_nUpdateFlags & GraphicsState::updateDigitLanguage) )
    {
        rNewState.m_nUpdateFlags &= ~GraphicsState::updateDigitLanguage;
        getReferenceDevice()->SetDigitLanguage( rNewState.m_aDigitLanguage );
    }

    if( (rNewState.m_nUpdateFlags & GraphicsState::updateLineColor) )
    {
        rNewState.m_nUpdateFlags &= ~GraphicsState::updateLineColor;
        if( m_aCurrentPDFState.m_aLineColor != rNewState.m_aLineColor &&
            rNewState.m_aLineColor != Color( COL_TRANSPARENT ) )
        {
            appendStrokingColor( rNewState.m_aLineColor, aLine );
            aLine.append( "\n" );
        }
    }

    if( (rNewState.m_nUpdateFlags & GraphicsState::updateFillColor) )
    {
        rNewState.m_nUpdateFlags &= ~GraphicsState::updateFillColor;
        if( m_aCurrentPDFState.m_aFillColor != rNewState.m_aFillColor &&
            rNewState.m_aFillColor != Color( COL_TRANSPARENT ) )
        {
            appendNonStrokingColor( rNewState.m_aFillColor, aLine );
            aLine.append( "\n" );
        }
    }

    if( (rNewState.m_nUpdateFlags & GraphicsState::updateTransparentPercent) )
    {
        rNewState.m_nUpdateFlags &= ~GraphicsState::updateTransparentPercent;
        if( m_aContext.Version >= PDFWriter::PDF_1_4 && m_aCurrentPDFState.m_nTransparentPercent != rNewState.m_nTransparentPercent )
        {
            // TODO: switch extended graphicsstate
        }
    }

    // everything is up to date now
    m_aCurrentPDFState = m_aGraphicsStack.front();
    if( aLine.getLength() )
        writeBuffer( aLine.getStr(), aLine.getLength() );
}

/* #i47544# imitate OutputDevice behaviour:
*  if a font with a nontransparent color is set, it overwrites the current
*  text color. OTOH setting the text color will overwrite the color of the font.
*/
void PDFWriterImpl::setFont( const Font& rFont )
{
    Color aColor = rFont.GetColor();
    if( aColor == Color( COL_TRANSPARENT ) )
        aColor = m_aGraphicsStack.front().m_aFont.GetColor();
    m_aGraphicsStack.front().m_aFont = rFont;
    m_aGraphicsStack.front().m_aFont.SetColor( aColor );
    m_aGraphicsStack.front().m_nUpdateFlags |= GraphicsState::updateFont;
}

void PDFWriterImpl::push( sal_uInt16 nFlags )
{
    m_aGraphicsStack.push_front( m_aGraphicsStack.front() );
    m_aGraphicsStack.front().m_nFlags = nFlags;
}

void PDFWriterImpl::pop()
{
    GraphicsState aState = m_aGraphicsStack.front();
    m_aGraphicsStack.pop_front();
    GraphicsState& rOld = m_aGraphicsStack.front();

    // move those parameters back that were not pushed
    // in the first place
    if( ! (aState.m_nFlags & PUSH_LINECOLOR) )
        setLineColor( aState.m_aLineColor );
    if( ! (aState.m_nFlags & PUSH_FILLCOLOR) )
        setFillColor( aState.m_aFillColor );
    if( ! (aState.m_nFlags & PUSH_FONT) )
        setFont( aState.m_aFont );
    if( ! (aState.m_nFlags & PUSH_TEXTCOLOR) )
        setTextColor( aState.m_aFont.GetColor() );
    if( ! (aState.m_nFlags & PUSH_MAPMODE) )
        setMapMode( aState.m_aMapMode );
    if( ! (aState.m_nFlags & PUSH_CLIPREGION) )
        // do not use setClipRegion here
        // it would convert again assuming the current mapmode
        rOld.m_aClipRegion = aState.m_aClipRegion;
    if( ! (aState.m_nFlags & PUSH_TEXTLINECOLOR ) )
        setTextLineColor( aState.m_aTextLineColor );
    if( ! (aState.m_nFlags & PUSH_OVERLINECOLOR ) )
        setOverlineColor( aState.m_aOverlineColor );
    if( ! (aState.m_nFlags & PUSH_TEXTALIGN ) )
        setTextAlign( aState.m_aFont.GetAlign() );
    if( ! (aState.m_nFlags & PUSH_TEXTFILLCOLOR) )
        setTextFillColor( aState.m_aFont.GetFillColor() );
    if( ! (aState.m_nFlags & PUSH_REFPOINT) )
    {
        // what ?
    }
    // invalidate graphics state
    m_aGraphicsStack.front().m_nUpdateFlags = sal::static_int_cast<sal_uInt16>(~0U);
}

void PDFWriterImpl::setMapMode( const MapMode& rMapMode )
{
    m_aGraphicsStack.front().m_aMapMode = rMapMode;
    getReferenceDevice()->SetMapMode( rMapMode );
    m_aCurrentPDFState.m_aMapMode = rMapMode;
}

void PDFWriterImpl::setClipRegion( const Region& rRegion )
{
    Region aRegion = getReferenceDevice()->LogicToPixel( rRegion, m_aGraphicsStack.front().m_aMapMode );
    aRegion = getReferenceDevice()->PixelToLogic( aRegion, m_aMapMode );
    m_aGraphicsStack.front().m_aClipRegion = aRegion;
    m_aGraphicsStack.front().m_nUpdateFlags |= GraphicsState::updateClipRegion;
}

void PDFWriterImpl::moveClipRegion( sal_Int32 nX, sal_Int32 nY )
{
    Point aPoint( lcl_convert( m_aGraphicsStack.front().m_aMapMode,
                               m_aMapMode,
                               getReferenceDevice(),
                               Point( nX, nY ) ) );
    aPoint -= lcl_convert( m_aGraphicsStack.front().m_aMapMode,
                           m_aMapMode,
                           getReferenceDevice(),
                           Point() );
    m_aGraphicsStack.front().m_aClipRegion.Move( aPoint.X(), aPoint.Y() );
    m_aGraphicsStack.front().m_nUpdateFlags |= GraphicsState::updateClipRegion;
}

bool PDFWriterImpl::intersectClipRegion( const Rectangle& rRect )
{
    Rectangle aRect( lcl_convert( m_aGraphicsStack.front().m_aMapMode,
                                  m_aMapMode,
                                  getReferenceDevice(),
                                  rRect ) );
    m_aGraphicsStack.front().m_nUpdateFlags |= GraphicsState::updateClipRegion;
    return m_aGraphicsStack.front().m_aClipRegion.Intersect( aRect );
}


bool PDFWriterImpl::intersectClipRegion( const Region& rRegion )
{
    Region aRegion = getReferenceDevice()->LogicToPixel( rRegion, m_aGraphicsStack.front().m_aMapMode );
    aRegion = getReferenceDevice()->PixelToLogic( aRegion, m_aMapMode );
    m_aGraphicsStack.front().m_nUpdateFlags |= GraphicsState::updateClipRegion;
    return m_aGraphicsStack.front().m_aClipRegion.Intersect( aRegion );
}

void PDFWriterImpl::createNote( const Rectangle& rRect, const PDFNote& rNote, sal_Int32 nPageNr )
{
    if( nPageNr < 0 )
        nPageNr = m_nCurrentPage;

    if( nPageNr < 0 || nPageNr >= (sal_Int32)m_aPages.size() )
        return;

    m_aNotes.push_back( PDFNoteEntry() );
    m_aNotes.back().m_nObject		= createObject();
    m_aNotes.back().m_aContents		= rNote;
    m_aNotes.back().m_aRect			= rRect;
    // convert to default user space now, since the mapmode may change
    m_aPages[nPageNr].convertRect( m_aNotes.back().m_aRect );

    // insert note to page's annotation list
    m_aPages[ nPageNr ].m_aAnnotations.push_back( m_aNotes.back().m_nObject );
}

sal_Int32 PDFWriterImpl::createLink( const Rectangle& rRect, sal_Int32 nPageNr )
{
    if( nPageNr < 0 )
        nPageNr = m_nCurrentPage;

    if( nPageNr < 0 || nPageNr >= (sal_Int32)m_aPages.size() )
        return -1;

    sal_Int32 nRet = m_aLinks.size();

    m_aLinks.push_back( PDFLink() );
    m_aLinks.back().m_nObject	= createObject();
    m_aLinks.back().m_nPage		= nPageNr;
    m_aLinks.back().m_aRect		= rRect;
    // convert to default user space now, since the mapmode may change
    m_aPages[nPageNr].convertRect( m_aLinks.back().m_aRect );

    // insert link to page's annotation list
    m_aPages[ nPageNr ].m_aAnnotations.push_back( m_aLinks.back().m_nObject );

    return nRet;
}

//--->i56629
sal_Int32 PDFWriterImpl::createNamedDest( const rtl::OUString& sDestName, const Rectangle& rRect, sal_Int32 nPageNr, PDFWriter::DestAreaType eType )
{
    if( nPageNr < 0 )
        nPageNr = m_nCurrentPage;

    if( nPageNr < 0 || nPageNr >= (sal_Int32)m_aPages.size() )
        return -1;

    sal_Int32 nRet = m_aNamedDests.size();

    m_aNamedDests.push_back( PDFNamedDest() );
    m_aNamedDests.back().m_aDestName = sDestName;
    m_aNamedDests.back().m_nPage = nPageNr;
    m_aNamedDests.back().m_eType = eType;
    m_aNamedDests.back().m_aRect = rRect;
    // convert to default user space now, since the mapmode may change
    m_aPages[nPageNr].convertRect( m_aNamedDests.back().m_aRect );

    return nRet;
}
//<---i56629

sal_Int32 PDFWriterImpl::createDest( const Rectangle& rRect, sal_Int32 nPageNr, PDFWriter::DestAreaType eType )
{
    if( nPageNr < 0 )
        nPageNr = m_nCurrentPage;

    if( nPageNr < 0 || nPageNr >= (sal_Int32)m_aPages.size() )
        return -1;

    sal_Int32 nRet = m_aDests.size();

    m_aDests.push_back( PDFDest() );
    m_aDests.back().m_nPage = nPageNr;
    m_aDests.back().m_eType	= eType;
    m_aDests.back().m_aRect = rRect;
    // convert to default user space now, since the mapmode may change
    m_aPages[nPageNr].convertRect( m_aDests.back().m_aRect );

    return nRet;
}

sal_Int32 PDFWriterImpl::setLinkDest( sal_Int32 nLinkId, sal_Int32 nDestId )
{
    if( nLinkId < 0 || nLinkId >= (sal_Int32)m_aLinks.size() )
        return -1;
    if( nDestId < 0 || nDestId >= (sal_Int32)m_aDests.size() )
        return -2;

    m_aLinks[ nLinkId ].m_nDest = nDestId;

    return 0;
}

sal_Int32 PDFWriterImpl::setLinkURL( sal_Int32 nLinkId, const OUString& rURL )
{
    if( nLinkId < 0 || nLinkId >= (sal_Int32)m_aLinks.size() )
        return -1;

    m_aLinks[ nLinkId ].m_nDest	= -1;

    using namespace ::com::sun::star;

    if (!m_xTrans.is())
    {
        uno::Reference< lang::XMultiServiceFactory > xFact( comphelper::getProcessServiceFactory() );
        if( xFact.is() )
        {
            m_xTrans = uno::Reference < util::XURLTransformer >(
                xFact->createInstance( OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.util.URLTransformer" ) ) ), uno::UNO_QUERY );
        }
    }

    util::URL aURL;
    aURL.Complete = rURL;

    if (m_xTrans.is())
        m_xTrans->parseStrict( aURL );

    m_aLinks[ nLinkId ].m_aURL	= aURL.Complete;

    return 0;
}

void PDFWriterImpl::setLinkPropertyId( sal_Int32 nLinkId, sal_Int32 nPropertyId )
{
    m_aLinkPropertyMap[ nPropertyId ] = nLinkId;
}

sal_Int32 PDFWriterImpl::createOutlineItem( sal_Int32 nParent, const OUString& rText, sal_Int32 nDestID )
{
    // create new item
    sal_Int32 nNewItem = m_aOutline.size();
    m_aOutline.push_back( PDFOutlineEntry() );

    // set item attributes
    setOutlineItemParent( nNewItem, nParent );
    setOutlineItemText( nNewItem, rText );
    setOutlineItemDest( nNewItem, nDestID );

    return nNewItem;
}

sal_Int32 PDFWriterImpl::setOutlineItemParent( sal_Int32 nItem, sal_Int32 nNewParent )
{
    if( nItem < 1 || nItem >= (sal_Int32)m_aOutline.size() )
        return -1;

    int nRet = 0;

    if( nNewParent < 0 || nNewParent >= (sal_Int32)m_aOutline.size() || nNewParent == nItem )
    {
        nNewParent = 0;
        nRet = -2;
    }
    // remove item from previous parent
    sal_Int32 nParentID = m_aOutline[ nItem ].m_nParentID;
    if( nParentID >= 0 && nParentID < (sal_Int32)m_aOutline.size() )
    {
        PDFOutlineEntry& rParent = m_aOutline[ nParentID ];

        for( std::vector<sal_Int32>::iterator it = rParent.m_aChildren.begin();
             it != rParent.m_aChildren.end(); ++it )
        {
            if( *it == nItem )
            {
                rParent.m_aChildren.erase( it );
                break;
            }
        }
    }

    // insert item to new parent's list of children
    m_aOutline[ nNewParent ].m_aChildren.push_back( nItem );

    return nRet;
}

sal_Int32 PDFWriterImpl::setOutlineItemText( sal_Int32 nItem, const OUString& rText )
{
    if( nItem < 1 || nItem >= (sal_Int32)m_aOutline.size() )
        return -1;

    m_aOutline[ nItem ].m_aTitle = rText;
    return 0;
}

sal_Int32 PDFWriterImpl::setOutlineItemDest( sal_Int32 nItem, sal_Int32 nDestID )
{
    if( nItem < 1 || nItem >= (sal_Int32)m_aOutline.size() ) // item does not exist
        return -1;
    if( nDestID < 0 || nDestID >= (sal_Int32)m_aDests.size() ) // dest does not exist
        return -2;
    m_aOutline[nItem].m_nDestID = nDestID;
    return 0;
}

const sal_Char* PDFWriterImpl::getStructureTag( PDFWriter::StructElement eType )
{
    static std::map< PDFWriter::StructElement, const char* > aTagStrings;
    if( aTagStrings.empty() )
    {
        aTagStrings[ PDFWriter::NonStructElement] = "NonStruct";
        aTagStrings[ PDFWriter::Document ]		= "Document";
        aTagStrings[ PDFWriter::Part ]			= "Part";
        aTagStrings[ PDFWriter::Article ]		= "Art";
        aTagStrings[ PDFWriter::Section ]		= "Sect";
        aTagStrings[ PDFWriter::Division ]		= "Div";
        aTagStrings[ PDFWriter::BlockQuote ]	= "BlockQuote";
        aTagStrings[ PDFWriter::Caption ]		= "Caption";
        aTagStrings[ PDFWriter::TOC ]			= "TOC";
        aTagStrings[ PDFWriter::TOCI ]			= "TOCI";
        aTagStrings[ PDFWriter::Index ]			= "Index";
        aTagStrings[ PDFWriter::Paragraph ]		= "P";
        aTagStrings[ PDFWriter::Heading ]		= "H";
        aTagStrings[ PDFWriter::H1 ]			= "H1";
        aTagStrings[ PDFWriter::H2 ]			= "H2";
        aTagStrings[ PDFWriter::H3 ]			= "H3";
        aTagStrings[ PDFWriter::H4 ]			= "H4";
        aTagStrings[ PDFWriter::H5 ]			= "H5";
        aTagStrings[ PDFWriter::H6 ]			= "H6";
        aTagStrings[ PDFWriter::List ]			= "L";
        aTagStrings[ PDFWriter::ListItem ]		= "LI";
        aTagStrings[ PDFWriter::LILabel ]		= "Lbl";
        aTagStrings[ PDFWriter::LIBody ]		= "LBody";
        aTagStrings[ PDFWriter::Table ]			= "Table";
        aTagStrings[ PDFWriter::TableRow ]		= "TR";
        aTagStrings[ PDFWriter::TableHeader ]	= "TH";
        aTagStrings[ PDFWriter::TableData ]		= "TD";
        aTagStrings[ PDFWriter::Span ]			= "Span";
        aTagStrings[ PDFWriter::Quote ]			= "Quote";
        aTagStrings[ PDFWriter::Note ]			= "Note";
        aTagStrings[ PDFWriter::Reference ]		= "Reference";
        aTagStrings[ PDFWriter::BibEntry ]		= "BibEntry";
        aTagStrings[ PDFWriter::Code ]			= "Code";
        aTagStrings[ PDFWriter::Link ]			= "Link";
        aTagStrings[ PDFWriter::Figure ]		= "Figure";
        aTagStrings[ PDFWriter::Formula ]		= "Formula";
        aTagStrings[ PDFWriter::Form ]			= "Form";
    }

    std::map< PDFWriter::StructElement, const char* >::const_iterator it = aTagStrings.find( eType );

    return it != aTagStrings.end() ? it->second : "Div";
}

void PDFWriterImpl::beginStructureElementMCSeq()
{
    if(	m_bEmitStructure &&
        m_nCurrentStructElement > 0 && // StructTreeRoot
        ! m_aStructure[ m_nCurrentStructElement ].m_bOpenMCSeq // already opened sequence
        )
    {
        PDFStructureElement& rEle = m_aStructure[ m_nCurrentStructElement ];
        OStringBuffer aLine( 128 );
        sal_Int32 nMCID = m_aPages[ m_nCurrentPage ].m_aMCIDParents.size();
        aLine.append( "/" );
        if( rEle.m_aAlias.getLength() > 0 )
            aLine.append( rEle.m_aAlias );
        else
            aLine.append( getStructureTag( rEle.m_eType ) );
        aLine.append( "<</MCID " );
        aLine.append( nMCID );
        aLine.append( ">>BDC\n" );
        writeBuffer( aLine.getStr(), aLine.getLength() );

        // update the element's content list
#if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "beginning marked content id %" SAL_PRIdINT32 " on page object %" SAL_PRIdINT32 ", structure first page = %" SAL_PRIdINT32 "\n",
                 nMCID,
                 m_aPages[ m_nCurrentPage ].m_nPageObject,
                 rEle.m_nFirstPageObject );
#endif
        rEle.m_aKids.push_back( PDFStructureElementKid( nMCID, m_aPages[m_nCurrentPage].m_nPageObject ) );
        // update the page's mcid parent list
        m_aPages[ m_nCurrentPage ].m_aMCIDParents.push_back( rEle.m_nObject );
        // mark element MC sequence as open
        rEle.m_bOpenMCSeq = true;
    }
    // handle artifacts
    else if( ! m_bEmitStructure && m_aContext.Tagged &&
               m_nCurrentStructElement > 0 &&
               m_aStructure[ m_nCurrentStructElement ].m_eType == PDFWriter::NonStructElement &&
             ! m_aStructure[ m_nCurrentStructElement ].m_bOpenMCSeq // already opened sequence
             )
    {
        OStringBuffer aLine( 128 );
        aLine.append( "/Artifact BMC\n" );
        writeBuffer( aLine.getStr(), aLine.getLength() );
        // mark element MC sequence as open
        m_aStructure[ m_nCurrentStructElement ].m_bOpenMCSeq = true;
    }
}

void PDFWriterImpl::endStructureElementMCSeq()
{
    if( m_nCurrentStructElement > 0 && // StructTreeRoot
        ( m_bEmitStructure || m_aStructure[ m_nCurrentStructElement ].m_eType == PDFWriter::NonStructElement ) &&
        m_aStructure[ m_nCurrentStructElement ].m_bOpenMCSeq // must have an opened MC sequence
        )
    {
        writeBuffer( "EMC\n", 4 );
        m_aStructure[ m_nCurrentStructElement ].m_bOpenMCSeq = false;
    }
}

bool PDFWriterImpl::checkEmitStructure()
{
    bool bEmit = false;
    if( m_aContext.Tagged )
    {
        bEmit = true;
        sal_Int32 nEle = m_nCurrentStructElement;
        while( nEle > 0 && nEle < sal_Int32(m_aStructure.size()) )
        {
            if( m_aStructure[ nEle ].m_eType == PDFWriter::NonStructElement )
            {
                bEmit = false;
                break;
            }
            nEle = m_aStructure[ nEle ].m_nParentElement;
        }
    }
    return bEmit;
}

sal_Int32 PDFWriterImpl::beginStructureElement( PDFWriter::StructElement eType, const rtl::OUString& rAlias )
{
    if( m_nCurrentPage < 0 )
        return -1;

    if( ! m_aContext.Tagged )
        return -1;

    // close eventual current MC sequence
    endStructureElementMCSeq();

    if( m_nCurrentStructElement == 0 &&
        eType != PDFWriter::Document && eType != PDFWriter::NonStructElement )
    {
        // struct tree root hit, but not beginning document
        // this might happen with setCurrentStructureElement
        // silently insert structure into document again if one properly exists
        if( ! m_aStructure[ 0 ].m_aChildren.empty() )
        {
            PDFWriter::StructElement childType = PDFWriter::NonStructElement;
            sal_Int32 nNewCurElement = 0;
            const std::list< sal_Int32 >& rRootChildren = m_aStructure[0].m_aChildren;
            for( std::list< sal_Int32 >::const_iterator it = rRootChildren.begin();
                 childType != PDFWriter::Document && it != rRootChildren.end(); ++it )
            {
                nNewCurElement = *it;
                childType = m_aStructure[ nNewCurElement ].m_eType;
            }
            if( childType == PDFWriter::Document )
            {
                m_nCurrentStructElement = nNewCurElement;
                DBG_ASSERT( 0, "Structure element inserted to StructTreeRoot that is not a document" );
            }
            else {
                DBG_ERROR( "document structure in disorder !" );
            }
        }
        else {
            DBG_ERROR( "PDF document structure MUST be contained in a Document element" );
        }
    }

    sal_Int32 nNewId = sal_Int32(m_aStructure.size());
    m_aStructure.push_back( PDFStructureElement() );
    PDFStructureElement& rEle = m_aStructure.back();
    rEle.m_eType			= eType;
    rEle.m_nOwnElement		= nNewId;
    rEle.m_nParentElement	= m_nCurrentStructElement;
    rEle.m_nFirstPageObject	= m_aPages[ m_nCurrentPage ].m_nPageObject;
    m_aStructure[ m_nCurrentStructElement ].m_aChildren.push_back( nNewId );
    m_nCurrentStructElement = nNewId;

    // handle alias names
    if( rAlias.getLength() && eType != PDFWriter::NonStructElement )
    {
        OStringBuffer aNameBuf( rAlias.getLength() );
        appendName( rAlias, aNameBuf );
        OString aAliasName( aNameBuf.makeStringAndClear() );
        rEle.m_aAlias = aAliasName;
        m_aRoleMap[ aAliasName ] = getStructureTag( eType );
    }

#if OSL_DEBUG_LEVEL > 1
    OStringBuffer aLine( "beginStructureElement " );
    aLine.append( m_nCurrentStructElement );
    aLine.append( ": " );
    aLine.append( getStructureTag( eType ) );
    if( rEle.m_aAlias.getLength() )
    {
        aLine.append( " aliased as \"" );
        aLine.append( rEle.m_aAlias );
        aLine.append( '\"' );
    }
    emitComment( aLine.getStr() );
#endif

    // check whether to emit structure henceforth
    m_bEmitStructure = checkEmitStructure();

    if( m_bEmitStructure ) // don't create nonexistant objects
    {
        rEle.m_nObject		= createObject();
        // update parent's kids list
        m_aStructure[ rEle.m_nParentElement ].m_aKids.push_back( rEle.m_nObject );
    }
    return nNewId;
}

void PDFWriterImpl::endStructureElement()
{
    if( m_nCurrentPage < 0 )
        return;

    if( ! m_aContext.Tagged )
        return;

    if( m_nCurrentStructElement == 0 )
    {
        // hit the struct tree root, that means there is an endStructureElement
        // without corresponding beginStructureElement
        return;
    }

    // end the marked content sequence
    endStructureElementMCSeq();

#if OSL_DEBUG_LEVEL > 1
    OStringBuffer aLine( "endStructureElement " );
    aLine.append( m_nCurrentStructElement );
    aLine.append( ": " );
    aLine.append( getStructureTag( m_aStructure[ m_nCurrentStructElement ].m_eType ) );
    if( m_aStructure[ m_nCurrentStructElement ].m_aAlias.getLength() )
    {
        aLine.append( " aliased as \"" );
        aLine.append( m_aStructure[ m_nCurrentStructElement ].m_aAlias );
        aLine.append( '\"' );
    }
#endif

    // "end" the structure element, the parent becomes current element
    m_nCurrentStructElement = m_aStructure[ m_nCurrentStructElement ].m_nParentElement;

    // check whether to emit structure henceforth
    m_bEmitStructure = checkEmitStructure();

#if OSL_DEBUG_LEVEL > 1
    if( m_bEmitStructure )
        emitComment( aLine.getStr() );
#endif
}

//---> i94258
/*
 * This function adds an internal structure list container to overcome the 8191 elements array limitation
 * in kids element emission.
 * Recursive function
 * 
 */
void PDFWriterImpl::addInternalStructureContainer( PDFStructureElement& rEle )
{
    if( rEle.m_eType == PDFWriter::NonStructElement &&
        rEle.m_nOwnElement != rEle.m_nParentElement )
        return;

    for( std::list< sal_Int32 >::const_iterator it = rEle.m_aChildren.begin(); it != rEle.m_aChildren.end(); ++it )
    {
        if( *it > 0 && *it < sal_Int32(m_aStructure.size()) )
        {
            PDFStructureElement& rChild = m_aStructure[ *it ];
            if( rChild.m_eType != PDFWriter::NonStructElement )
            {
                //triggered when a child of the rEle element is found
                if( rChild.m_nParentElement == rEle.m_nOwnElement )
                    addInternalStructureContainer( rChild );//examine the child
                else
                {
                    DBG_ERROR( "PDFWriterImpl::addInternalStructureContainer: invalid child structure element" );
#if OSL_DEBUG_LEVEL > 1
                    fprintf( stderr, "PDFWriterImpl::addInternalStructureContainer: invalid child structure elemnt with id %" SAL_PRIdINT32 "\n", *it );
#endif
                }
            }
        }
        else
        {
            DBG_ERROR( "PDFWriterImpl::emitStructure: invalid child structure id" );
#if OSL_DEBUG_LEVEL > 1
            fprintf( stderr, "PDFWriterImpl::addInternalStructureContainer: invalid child structure id %" SAL_PRIdINT32 "\n", *it );
#endif
        }
    }

    if( rEle.m_nOwnElement != rEle.m_nParentElement )
    {
        if( !rEle.m_aKids.empty() )
        {
            if( rEle.m_aKids.size() > ncMaxPDFArraySize ) {
                //then we need to add the containers for the kids elements
                // a list to be used for the new kid element
                std::list< PDFStructureElementKid > aNewKids;
                std::list< sal_Int32 > aNewChildren;

                // add Div in RoleMap, in case no one else did (TODO: is it needed? Is it dangerous?)
                OStringBuffer aNameBuf( "Div" );
                OString aAliasName( aNameBuf.makeStringAndClear() );
                m_aRoleMap[ aAliasName ] = getStructureTag( PDFWriter::Division );

                while( rEle.m_aKids.size() > ncMaxPDFArraySize )
                {
                    sal_Int32 nCurrentStructElement = rEle.m_nOwnElement;
                    sal_Int32 nNewId = sal_Int32(m_aStructure.size());
                    m_aStructure.push_back( PDFStructureElement() );
                    PDFStructureElement& rEleNew = m_aStructure.back();
                    rEleNew.m_aAlias            = aAliasName;
                    rEleNew.m_eType			    = PDFWriter::Division; // a new Div type container
                    rEleNew.m_nOwnElement		= nNewId;
                    rEleNew.m_nParentElement	= nCurrentStructElement;
                    //inherit the same page as the first child to be reparented
                    rEleNew.m_nFirstPageObject	= m_aStructure[ rEle.m_aChildren.front() ].m_nFirstPageObject;
                    rEleNew.m_nObject           = createObject();//assign a PDF object number
                    //add the object to the kid list of the parent
                    aNewKids.push_back( PDFStructureElementKid( rEleNew.m_nObject ) );
                    aNewChildren.push_back( nNewId );
                    
                    std::list< sal_Int32 >::iterator aChildEndIt( rEle.m_aChildren.begin() );
                    std::list< PDFStructureElementKid >::iterator aKidEndIt( rEle.m_aKids.begin() );
                    advance( aChildEndIt, ncMaxPDFArraySize );
                    advance( aKidEndIt, ncMaxPDFArraySize );
                    
                    rEleNew.m_aKids.splice( rEleNew.m_aKids.begin(),
                                            rEle.m_aKids,
                                            rEle.m_aKids.begin(),
                                            aKidEndIt );
                    rEleNew.m_aChildren.splice( rEleNew.m_aChildren.begin(),
                                                rEle.m_aChildren,
                                                rEle.m_aChildren.begin(),
                                                aChildEndIt );
                    // set the kid's new parent
                    for( std::list< sal_Int32 >::const_iterator it = rEleNew.m_aChildren.begin();
                         it != rEleNew.m_aChildren.end(); ++it )
                    {
                        m_aStructure[ *it ].m_nParentElement = nNewId;
                    }
                }
                //finally add the new kids resulting from the container added
                rEle.m_aKids.insert( rEle.m_aKids.begin(), aNewKids.begin(), aNewKids.end() );
                rEle.m_aChildren.insert( rEle.m_aChildren.begin(), aNewChildren.begin(), aNewChildren.end() );
            }        
        }
    }
}
//<--- i94258

bool PDFWriterImpl::setCurrentStructureElement( sal_Int32 nEle )
{
    bool bSuccess = false;

    if( m_aContext.Tagged && nEle >= 0 && nEle < sal_Int32(m_aStructure.size()) )
    {
        // end eventual previous marked content sequence
        endStructureElementMCSeq();

        m_nCurrentStructElement = nEle;
        m_bEmitStructure = checkEmitStructure();
#if OSL_DEBUG_LEVEL > 1
        OStringBuffer aLine( "setCurrentStructureElement " );
        aLine.append( m_nCurrentStructElement );
        aLine.append( ": " );
        aLine.append( getStructureTag( m_aStructure[ m_nCurrentStructElement ].m_eType ) );
        if( m_aStructure[ m_nCurrentStructElement ].m_aAlias.getLength() )
        {
            aLine.append( " aliased as \"" );
            aLine.append( m_aStructure[ m_nCurrentStructElement ].m_aAlias );
            aLine.append( '\"' );
        }
        if( ! m_bEmitStructure )
            aLine.append( " (inside NonStruct)" );
        emitComment( aLine.getStr() );
#endif
        bSuccess = true;
    }

    return bSuccess;
}

sal_Int32 PDFWriterImpl::getCurrentStructureElement()
{
    return m_nCurrentStructElement;
}

bool PDFWriterImpl::setStructureAttribute( enum PDFWriter::StructAttribute eAttr, enum PDFWriter::StructAttributeValue eVal )
{
    if( !m_aContext.Tagged )
        return false;

    bool bInsert = false;
    if( m_nCurrentStructElement > 0 && m_bEmitStructure )
    {
        PDFWriter::StructElement eType = m_aStructure[ m_nCurrentStructElement ].m_eType;
        switch( eAttr )
        {
            case PDFWriter::Placement:
                if( eVal == PDFWriter::Block		||
                    eVal == PDFWriter::Inline		||
                    eVal == PDFWriter::Before		||
                    eVal == PDFWriter::Start		||
                    eVal == PDFWriter::End )
                    bInsert = true;
                break;
            case PDFWriter::WritingMode:
                if( eVal == PDFWriter::LrTb			||
                    eVal == PDFWriter::RlTb			||
                    eVal == PDFWriter::TbRl )
                {
                    bInsert = true;
                }
                break;
            case PDFWriter::TextAlign:
                if( eVal == PDFWriter::Start		||
                    eVal == PDFWriter::Center		||
                    eVal == PDFWriter::End			||
                    eVal == PDFWriter::Justify )
                {
                    if( eType == PDFWriter::Paragraph	||
                        eType == PDFWriter::Heading		||
                        eType == PDFWriter::H1			||
                        eType == PDFWriter::H2			||
                        eType == PDFWriter::H3			||
                        eType == PDFWriter::H4			||
                        eType == PDFWriter::H5			||
                        eType == PDFWriter::H6			||
                        eType == PDFWriter::List		||
                        eType == PDFWriter::ListItem	||
                        eType == PDFWriter::LILabel		||
                        eType == PDFWriter::LIBody		||
                        eType == PDFWriter::Table		||
                        eType == PDFWriter::TableRow	||
                        eType == PDFWriter::TableHeader	||
                        eType == PDFWriter::TableData )
                    {
                        bInsert = true;
                    }
                }
                break;
            case PDFWriter::Width:
            case PDFWriter::Height:
                if( eVal == PDFWriter::Auto )
                {
                    if( eType == PDFWriter::Figure		||
                        eType == PDFWriter::Formula		||
                        eType == PDFWriter::Form		||
                        eType == PDFWriter::Table		||
                        eType == PDFWriter::TableHeader	||
                        eType == PDFWriter::TableData )
                    {
                        bInsert = true;
                    }
                }
                break;
            case PDFWriter::BlockAlign:
                if( eVal == PDFWriter::Before		||
                    eVal == PDFWriter::Middle		||
                    eVal == PDFWriter::After		||
                    eVal == PDFWriter::Justify )
                {
                    if( eType == PDFWriter::TableHeader	||
                        eType == PDFWriter::TableData )
                    {
                        bInsert = true;
                    }
                }
                break;
            case PDFWriter::InlineAlign:
                if( eVal == PDFWriter::Start		||
                    eVal == PDFWriter::Center		||
                    eVal == PDFWriter::End )
                {
                    if( eType == PDFWriter::TableHeader	||
                        eType == PDFWriter::TableData )
                    {
                        bInsert = true;
                    }
                }
                break;
            case PDFWriter::LineHeight:
                if( eVal == PDFWriter::Normal		||
                    eVal == PDFWriter::Auto )
                {
                    // only for ILSE and BLSE
                    if( eType == PDFWriter::Paragraph	||
                        eType == PDFWriter::Heading		||
                        eType == PDFWriter::H1			||
                        eType == PDFWriter::H2			||
                        eType == PDFWriter::H3			||
                        eType == PDFWriter::H4			||
                        eType == PDFWriter::H5			||
                        eType == PDFWriter::H6			||
                        eType == PDFWriter::List		||
                        eType == PDFWriter::ListItem	||
                        eType == PDFWriter::LILabel		||
                        eType == PDFWriter::LIBody		||
                        eType == PDFWriter::Table		||
                        eType == PDFWriter::TableRow	||
                        eType == PDFWriter::TableHeader	||
                        eType == PDFWriter::TableData	||
                        eType == PDFWriter::Span		||
                        eType == PDFWriter::Quote		||
                        eType == PDFWriter::Note		||
                        eType == PDFWriter::Reference	||
                        eType == PDFWriter::BibEntry	||
                        eType == PDFWriter::Code		||
                        eType == PDFWriter::Link )
                    {
                        bInsert = true;
                    }
                }
                break;
            case PDFWriter::TextDecorationType:
                if( eVal == PDFWriter::NONE			||
                    eVal == PDFWriter::Underline	||
                    eVal == PDFWriter::Overline		||
                    eVal == PDFWriter::LineThrough )
                {
                    // only for ILSE and BLSE
                    if( eType == PDFWriter::Paragraph	||
                        eType == PDFWriter::Heading		||
                        eType == PDFWriter::H1			||
                        eType == PDFWriter::H2			||
                        eType == PDFWriter::H3			||
                        eType == PDFWriter::H4			||
                        eType == PDFWriter::H5			||
                        eType == PDFWriter::H6			||
                        eType == PDFWriter::List		||
                        eType == PDFWriter::ListItem	||
                        eType == PDFWriter::LILabel		||
                        eType == PDFWriter::LIBody		||
                        eType == PDFWriter::Table		||
                        eType == PDFWriter::TableRow	||
                        eType == PDFWriter::TableHeader	||
                        eType == PDFWriter::TableData	||
                        eType == PDFWriter::Span		||
                        eType == PDFWriter::Quote		||
                        eType == PDFWriter::Note		||
                        eType == PDFWriter::Reference	||
                        eType == PDFWriter::BibEntry	||
                        eType == PDFWriter::Code		||
                        eType == PDFWriter::Link )
                    {
                        bInsert = true;
                    }
                }
                break;
            case PDFWriter::ListNumbering:
                if( eVal == PDFWriter::NONE			||
                    eVal == PDFWriter::Disc			||
                    eVal == PDFWriter::Circle		||
                    eVal == PDFWriter::Square		||
                    eVal == PDFWriter::Decimal		||
                    eVal == PDFWriter::UpperRoman	||
                    eVal == PDFWriter::LowerRoman	||
                    eVal == PDFWriter::UpperAlpha	||
                    eVal == PDFWriter::LowerAlpha )
                {
                    if( eType == PDFWriter::List )
                        bInsert = true;
                }
                break;
            default: break;
        }
    }

    if( bInsert )
        m_aStructure[ m_nCurrentStructElement ].m_aAttributes[ eAttr ] = PDFStructureAttribute( eVal );
#if OSL_DEBUG_LEVEL > 1
    else if( m_nCurrentStructElement > 0 && m_bEmitStructure )
        fprintf( stderr, "rejecting setStructureAttribute( %s, %s ) on %s (%s) element\n",
                 getAttributeTag( eAttr ),
                 getAttributeValueTag( eVal ),
                 getStructureTag( m_aStructure[ m_nCurrentStructElement ].m_eType ),
                 m_aStructure[ m_nCurrentStructElement ].m_aAlias.getStr()
                 );
#endif

    return bInsert;
}

bool PDFWriterImpl::setStructureAttributeNumerical( enum PDFWriter::StructAttribute eAttr, sal_Int32 nValue )
{
    if( ! m_aContext.Tagged )
        return false;

    bool bInsert = false;
    if( m_nCurrentStructElement > 0 && m_bEmitStructure )
    {
        if( eAttr == PDFWriter::Language )
        {
            m_aStructure[ m_nCurrentStructElement ].m_aLocale = MsLangId::convertLanguageToLocale( (LanguageType)nValue );
            return true;
        }

        PDFWriter::StructElement eType = m_aStructure[ m_nCurrentStructElement ].m_eType;
        switch( eAttr )
        {
            case PDFWriter::SpaceBefore:
            case PDFWriter::SpaceAfter:
            case PDFWriter::StartIndent:
            case PDFWriter::EndIndent:
                // just for BLSE
                if( eType == PDFWriter::Paragraph	||
                    eType == PDFWriter::Heading		||
                    eType == PDFWriter::H1			||
                    eType == PDFWriter::H2			||
                    eType == PDFWriter::H3			||
                    eType == PDFWriter::H4			||
                    eType == PDFWriter::H5			||
                    eType == PDFWriter::H6			||
                    eType == PDFWriter::List		||
                    eType == PDFWriter::ListItem	||
                    eType == PDFWriter::LILabel		||
                    eType == PDFWriter::LIBody		||
                    eType == PDFWriter::Table		||
                    eType == PDFWriter::TableRow	||
                    eType == PDFWriter::TableHeader	||
                    eType == PDFWriter::TableData )
                {
                    bInsert = true;
                }
                break;
            case PDFWriter::TextIndent:
                // paragraph like BLSE and additional elements
                if( eType == PDFWriter::Paragraph	||
                    eType == PDFWriter::Heading		||
                    eType == PDFWriter::H1			||
                    eType == PDFWriter::H2			||
                    eType == PDFWriter::H3			||
                    eType == PDFWriter::H4			||
                    eType == PDFWriter::H5			||
                    eType == PDFWriter::H6			||
                    eType == PDFWriter::LILabel		||
                    eType == PDFWriter::LIBody		||
                    eType == PDFWriter::TableHeader	||
                    eType == PDFWriter::TableData )
                {
                    bInsert = true;
                }
                break;
            case PDFWriter::Width:
            case PDFWriter::Height:
                if( eType == PDFWriter::Figure		||
                    eType == PDFWriter::Formula		||
                    eType == PDFWriter::Form		||
                    eType == PDFWriter::Table		||
                    eType == PDFWriter::TableHeader	||
                    eType == PDFWriter::TableData )
                {
                    bInsert = true;
                }
                break;
            case PDFWriter::LineHeight:
            case PDFWriter::BaselineShift:
                // only for ILSE and BLSE
                if( eType == PDFWriter::Paragraph	||
                    eType == PDFWriter::Heading		||
                    eType == PDFWriter::H1			||
                    eType == PDFWriter::H2			||
                    eType == PDFWriter::H3			||
                    eType == PDFWriter::H4			||
                    eType == PDFWriter::H5			||
                    eType == PDFWriter::H6			||
                    eType == PDFWriter::List		||
                    eType == PDFWriter::ListItem	||
                    eType == PDFWriter::LILabel		||
                    eType == PDFWriter::LIBody		||
                    eType == PDFWriter::Table		||
                    eType == PDFWriter::TableRow	||
                    eType == PDFWriter::TableHeader	||
                    eType == PDFWriter::TableData	||
                    eType == PDFWriter::Span		||
                    eType == PDFWriter::Quote		||
                    eType == PDFWriter::Note		||
                    eType == PDFWriter::Reference	||
                    eType == PDFWriter::BibEntry	||
                    eType == PDFWriter::Code		||
                    eType == PDFWriter::Link )
                {
                        bInsert = true;
                }
                break;
            case PDFWriter::RowSpan:
            case PDFWriter::ColSpan:
                // only for table cells
                if( eType == PDFWriter::TableHeader	||
                    eType == PDFWriter::TableData )
                {
                    bInsert = true;
                }
                break;
            case PDFWriter::LinkAnnotation:
                if( eType == PDFWriter::Link )
                    bInsert = true;
                break;
            default: break;
        }
    }

    if( bInsert )
        m_aStructure[ m_nCurrentStructElement ].m_aAttributes[ eAttr ] = PDFStructureAttribute( nValue );
#if OSL_DEBUG_LEVEL > 1
    else if( m_nCurrentStructElement > 0 && m_bEmitStructure )
        fprintf( stderr, "rejecting setStructureAttributeNumerical( %s, %d ) on %s (%s) element\n",
                 getAttributeTag( eAttr ),
                 (int)nValue,
                 getStructureTag( m_aStructure[ m_nCurrentStructElement ].m_eType ),
                 m_aStructure[ m_nCurrentStructElement ].m_aAlias.getStr() );
#endif

    return bInsert;
}

void PDFWriterImpl::setStructureBoundingBox( const Rectangle& rRect )
{
    sal_Int32 nPageNr = m_nCurrentPage;
    if( nPageNr < 0 || nPageNr >= (sal_Int32)m_aPages.size() || !m_aContext.Tagged )
        return;


    if( m_nCurrentStructElement > 0 && m_bEmitStructure )
    {
        PDFWriter::StructElement eType = m_aStructure[ m_nCurrentStructElement ].m_eType;
        if( eType == PDFWriter::Figure		||
            eType == PDFWriter::Formula		||
            eType == PDFWriter::Form		||
            eType == PDFWriter::Table )
        {
            m_aStructure[ m_nCurrentStructElement ].m_aBBox = rRect;
            // convert to default user space now, since the mapmode may change
            m_aPages[nPageNr].convertRect( m_aStructure[ m_nCurrentStructElement ].m_aBBox );
        }
    }
}

void PDFWriterImpl::setActualText( const String& rText )
{
    if( m_aContext.Tagged && m_nCurrentStructElement > 0 && m_bEmitStructure )
    {
        m_aStructure[ m_nCurrentStructElement ].m_aActualText = rText;
    }
}

void PDFWriterImpl::setAlternateText( const String& rText )
{
    if( m_aContext.Tagged && m_nCurrentStructElement > 0 && m_bEmitStructure )
    {
        m_aStructure[ m_nCurrentStructElement ].m_aAltText = rText;
    }
}

void PDFWriterImpl::setAutoAdvanceTime( sal_uInt32 nSeconds, sal_Int32 nPageNr )
{
    if( nPageNr < 0 )
        nPageNr = m_nCurrentPage;

    if( nPageNr < 0 || nPageNr >= (sal_Int32)m_aPages.size() )
        return;

    m_aPages[ nPageNr ].m_nDuration = nSeconds;
}

void PDFWriterImpl::setPageTransition( PDFWriter::PageTransition eType, sal_uInt32 nMilliSec, sal_Int32 nPageNr )
{
    if( nPageNr < 0 )
        nPageNr = m_nCurrentPage;

    if( nPageNr < 0 || nPageNr >= (sal_Int32)m_aPages.size() )
        return;

    m_aPages[ nPageNr ].m_eTransition	= eType;
    m_aPages[ nPageNr ].m_nTransTime	= nMilliSec;
}

void PDFWriterImpl::ensureUniqueRadioOnValues()
{
    // loop over radio groups
    for( std::map<sal_Int32,sal_Int32>::const_iterator group = m_aRadioGroupWidgets.begin();
         group != m_aRadioGroupWidgets.end(); ++group )
    {
        PDFWidget& rGroupWidget = m_aWidgets[ group->second ];
        // check whether all kids have a unique OnValue
        std::hash_map< OUString, sal_Int32, OUStringHash > aOnValues;
        int nChildren = rGroupWidget.m_aKidsIndex.size();
        bool bIsUnique = true;
        for( int nKid = 0; nKid < nChildren && bIsUnique; nKid++ )
        {
            int nKidIndex = rGroupWidget.m_aKidsIndex[nKid];
            const OUString& rVal = m_aWidgets[nKidIndex].m_aOnValue;
            #if OSL_DEBUG_LEVEL > 1
            fprintf( stderr, "OnValue: %s\n", OUStringToOString( rVal, RTL_TEXTENCODING_UTF8 ).getStr() );
            #endif
            if( aOnValues.find( rVal ) == aOnValues.end() )
            {
                aOnValues[ rVal ] = 1;
            }
            else
            {
                bIsUnique = false;
            }
        }
        if( ! bIsUnique )
        {
            #if OSL_DEBUG_LEVEL > 1
            fprintf( stderr, "enforcing unique OnValues\n" );
            #endif
            // make unique by using ascending OnValues
            for( int nKid = 0; nKid < nChildren; nKid++ )
            {
                int nKidIndex = rGroupWidget.m_aKidsIndex[nKid];
                PDFWidget& rKid = m_aWidgets[nKidIndex];
                rKid.m_aOnValue = OUString::valueOf( sal_Int32(nKid+1) );
                if( ! rKid.m_aValue.equalsAscii( "Off" ) )
                    rKid.m_aValue = rKid.m_aOnValue;
            }
        }
        // finally move the "Yes" appearance to the OnValue appearance
        for( int nKid = 0; nKid < nChildren; nKid++ )
        {
            int nKidIndex = rGroupWidget.m_aKidsIndex[nKid];
            PDFWidget& rKid = m_aWidgets[nKidIndex];
            PDFAppearanceMap::iterator app_it = rKid.m_aAppearances.find( "N" );
            if( app_it != rKid.m_aAppearances.end() )
            {
                PDFAppearanceStreams::iterator stream_it = app_it->second.find( "Yes" );
                if( stream_it != app_it->second.end() )
                {
                    SvMemoryStream* pStream = stream_it->second;
                    app_it->second.erase( stream_it );
                    OStringBuffer aBuf( rKid.m_aOnValue.getLength()*2 );
                    appendName( rKid.m_aOnValue, aBuf );
                    (app_it->second)[ aBuf.makeStringAndClear() ] = pStream;
                }
                #if OSL_DEBUG_LEVEL > 1
                else
                    fprintf( stderr, "error: RadioButton without \"Yes\" stream\n" );
                #endif
            }
            // update selected radio button
            if( ! rKid.m_aValue.equalsAscii( "Off" ) )
            {
                rGroupWidget.m_aValue = rKid.m_aValue;
            }
        }
    }
}

sal_Int32 PDFWriterImpl::findRadioGroupWidget( const PDFWriter::RadioButtonWidget& rBtn )
{
    sal_Int32 nRadioGroupWidget = -1;

    std::map< sal_Int32, sal_Int32 >::const_iterator it = m_aRadioGroupWidgets.find( rBtn.RadioGroup );

    if( it == m_aRadioGroupWidgets.end() )
    {
        m_aRadioGroupWidgets[ rBtn.RadioGroup ] = nRadioGroupWidget =
            sal_Int32(m_aWidgets.size());

        // new group, insert the radiobutton
        m_aWidgets.push_back( PDFWidget() );
        m_aWidgets.back().m_nObject		= createObject();
        m_aWidgets.back().m_nPage		= m_nCurrentPage;
        m_aWidgets.back().m_eType		= PDFWriter::RadioButton;
        m_aWidgets.back().m_nRadioGroup = rBtn.RadioGroup;
        m_aWidgets.back().m_nFlags |= 0x00008000;

        // create radio button field name
        const rtl::OUString& rName = (m_aContext.Version > PDFWriter::PDF_1_2) ?
                                     rBtn.Name : rBtn.Text;
        if( rName.getLength() )
        {
            m_aWidgets.back().m_aName   = convertWidgetFieldName( rName );
        }
        else
        {
            m_aWidgets.back().m_aName	= "RadioGroup";
            m_aWidgets.back().m_aName  += OString::valueOf( rBtn.RadioGroup );
        }
    }
    else
        nRadioGroupWidget = it->second;

    return nRadioGroupWidget;
}

sal_Int32 PDFWriterImpl::createControl( const PDFWriter::AnyWidget& rControl, sal_Int32 nPageNr )
{
    if( nPageNr < 0 )
        nPageNr = m_nCurrentPage;

    if( nPageNr < 0 || nPageNr >= (sal_Int32)m_aPages.size() )
        return -1;

    m_aWidgets.push_back( PDFWidget() );
    sal_Int32 nNewWidget = m_aWidgets.size()-1;

    // create eventual radio button before getting any references
    // from m_aWidgets as the push_back operation potentially assigns new
    // memory to the vector and thereby invalidates the reference
    int nRadioGroupWidget = -1;
    if( rControl.getType() == PDFWriter::RadioButton )
        nRadioGroupWidget = findRadioGroupWidget( static_cast<const PDFWriter::RadioButtonWidget&>(rControl) );

    PDFWidget& rNewWidget			= m_aWidgets[nNewWidget];
    rNewWidget.m_nObject			= createObject();
    rNewWidget.m_aRect				= rControl.Location;
    rNewWidget.m_nPage				= nPageNr;
    rNewWidget.m_eType				= rControl.getType();

    // for unknown reasons the radio buttons of a radio group must not have a
    // field name, else the buttons are in fact check boxes -
    // that is multiple buttons of the radio group can be selected
    if( rControl.getType() != PDFWriter::RadioButton )
    {
        // acrobat reader since 3.0 does not support unicode text
        // strings for the field name; so we need to encode unicodes
        // larger than 255

        rNewWidget.m_aName			=
            convertWidgetFieldName( (m_aContext.Version > PDFWriter::PDF_1_2) ?
                                    rControl.Name : rControl.Text );
        // #i88040# acrobat reader crashes on empty field names,
        // so always create one
        if( rNewWidget.m_aName.getLength() == 0 )
        {
            OUStringBuffer aBuf( 32 );
            aBuf.appendAscii( "Widget" );
            aBuf.append( nNewWidget );
            rNewWidget.m_aName = convertWidgetFieldName( aBuf.makeStringAndClear() );
        }
    }
    rNewWidget.m_aDescription		= rControl.Description;
    rNewWidget.m_aText				= rControl.Text;
    rNewWidget.m_nTextStyle			= rControl.TextStyle &
        (  TEXT_DRAW_LEFT | TEXT_DRAW_CENTER | TEXT_DRAW_RIGHT | TEXT_DRAW_TOP |
           TEXT_DRAW_VCENTER | TEXT_DRAW_BOTTOM |
           TEXT_DRAW_MULTILINE | TEXT_DRAW_WORDBREAK  );
    rNewWidget.m_nTabOrder          = rControl.TabOrder;

    // various properties are set via the flags (/Ff) property of the field dict
    if( rControl.ReadOnly )
        rNewWidget.m_nFlags |= 1;
    if( rControl.getType() == PDFWriter::PushButton )
    {
        const PDFWriter::PushButtonWidget& rBtn = static_cast<const PDFWriter::PushButtonWidget&>(rControl);
        if( rNewWidget.m_nTextStyle == 0 )
            rNewWidget.m_nTextStyle =
                TEXT_DRAW_CENTER | TEXT_DRAW_VCENTER |
                TEXT_DRAW_MULTILINE | TEXT_DRAW_WORDBREAK;

        rNewWidget.m_nFlags |= 0x00010000;
        if( rBtn.URL.getLength() )
            rNewWidget.m_aListEntries.push_back( rBtn.URL );
        rNewWidget.m_bSubmit    = rBtn.Submit;
        rNewWidget.m_bSubmitGet = rBtn.SubmitGet;
        rNewWidget.m_nDest      = rBtn.Dest;
        createDefaultPushButtonAppearance( rNewWidget, rBtn );
    }
    else if( rControl.getType() == PDFWriter::RadioButton )
    {
        const PDFWriter::RadioButtonWidget& rBtn = static_cast<const PDFWriter::RadioButtonWidget&>(rControl);
        if( rNewWidget.m_nTextStyle == 0 )
            rNewWidget.m_nTextStyle =
                TEXT_DRAW_VCENTER | TEXT_DRAW_MULTILINE | TEXT_DRAW_WORDBREAK;
        /*  PDF sees a RadioButton group as one radio button with
         *  children which are in turn check boxes
         *
         *  so we need to create a radio button on demand for a new group
         *  and insert a checkbox for each RadioButtonWidget as its child
         */
        rNewWidget.m_eType			= PDFWriter::CheckBox;
        rNewWidget.m_nRadioGroup	= rBtn.RadioGroup;

        DBG_ASSERT( nRadioGroupWidget >= 0 && nRadioGroupWidget < (sal_Int32)m_aWidgets.size(), "no radio group parent" );

        PDFWidget& rRadioButton = m_aWidgets[nRadioGroupWidget];
        rRadioButton.m_aKids.push_back( rNewWidget.m_nObject );
        rRadioButton.m_aKidsIndex.push_back( nNewWidget );
        rNewWidget.m_nParent = rRadioButton.m_nObject;

        rNewWidget.m_aValue     = OUString( RTL_CONSTASCII_USTRINGPARAM( "Off" ) );
        rNewWidget.m_aOnValue   = rBtn.OnValue;
        if( ! rRadioButton.m_aValue.getLength() && rBtn.Selected )
        {
            rNewWidget.m_aValue		= rNewWidget.m_aOnValue;
            rRadioButton.m_aValue	= rNewWidget.m_aOnValue;
        }
        createDefaultRadioButtonAppearance( rNewWidget, rBtn );

        // union rect of radio group
        Rectangle aRect = rNewWidget.m_aRect;
        m_aPages[ nPageNr ].convertRect( aRect );
        rRadioButton.m_aRect.Union( aRect );
    }
    else if( rControl.getType() == PDFWriter::CheckBox )
    {
        const PDFWriter::CheckBoxWidget& rBox = static_cast<const PDFWriter::CheckBoxWidget&>(rControl);
        if( rNewWidget.m_nTextStyle == 0 )
            rNewWidget.m_nTextStyle =
                TEXT_DRAW_VCENTER | TEXT_DRAW_MULTILINE | TEXT_DRAW_WORDBREAK;

        rNewWidget.m_aValue = OUString::createFromAscii( rBox.Checked ? "Yes" : "Off" );
        // create default appearance before m_aRect gets transformed
        createDefaultCheckBoxAppearance( rNewWidget, rBox );
    }
    else if( rControl.getType() == PDFWriter::ListBox )
    {
        if( rNewWidget.m_nTextStyle == 0 )
            rNewWidget.m_nTextStyle = TEXT_DRAW_VCENTER;

        const PDFWriter::ListBoxWidget& rLstBox = static_cast<const PDFWriter::ListBoxWidget&>(rControl);
        rNewWidget.m_aListEntries	  = rLstBox.Entries;
        rNewWidget.m_aSelectedEntries = rLstBox.SelectedEntries;
        rNewWidget.m_aValue			  = rLstBox.Text;
        if( rLstBox.DropDown )
            rNewWidget.m_nFlags |= 0x00020000;
        if( rLstBox.Sort )
            rNewWidget.m_nFlags |= 0x00080000;
        if( rLstBox.MultiSelect && !rLstBox.DropDown && (int)m_aContext.Version > (int)PDFWriter::PDF_1_3 )
            rNewWidget.m_nFlags |= 0x00200000;

        createDefaultListBoxAppearance( rNewWidget, rLstBox );
    }
    else if( rControl.getType() == PDFWriter::ComboBox )
    {
        if( rNewWidget.m_nTextStyle == 0 )
            rNewWidget.m_nTextStyle = TEXT_DRAW_VCENTER;

        const PDFWriter::ComboBoxWidget& rBox = static_cast<const PDFWriter::ComboBoxWidget&>(rControl);
        rNewWidget.m_aValue			= rBox.Text;
        rNewWidget.m_aListEntries	= rBox.Entries;
        rNewWidget.m_nFlags |= 0x00060000; // combo and edit flag
        if( rBox.Sort )
            rNewWidget.m_nFlags |= 0x00080000;

        PDFWriter::ListBoxWidget aLBox;
        aLBox.Name				= rBox.Name;
        aLBox.Description		= rBox.Description;
        aLBox.Text				= rBox.Text;
        aLBox.TextStyle			= rBox.TextStyle;
        aLBox.ReadOnly			= rBox.ReadOnly;
        aLBox.Border			= rBox.Border;
        aLBox.BorderColor		= rBox.BorderColor;
        aLBox.Background		= rBox.Background;
        aLBox.BackgroundColor	= rBox.BackgroundColor;
        aLBox.TextFont			= rBox.TextFont;
        aLBox.TextColor			= rBox.TextColor;
        aLBox.DropDown			= true;
        aLBox.Sort				= rBox.Sort;
        aLBox.MultiSelect		= false;
        aLBox.Entries			= rBox.Entries;

        createDefaultListBoxAppearance( rNewWidget, aLBox );
    }
    else if( rControl.getType() == PDFWriter::Edit )
    {
        if( rNewWidget.m_nTextStyle == 0 )
            rNewWidget.m_nTextStyle = TEXT_DRAW_LEFT | TEXT_DRAW_VCENTER;

        const PDFWriter::EditWidget& rEdit = static_cast<const  PDFWriter::EditWidget&>(rControl);
        if( rEdit.MultiLine )
        {
            rNewWidget.m_nFlags |= 0x00001000;
            rNewWidget.m_nTextStyle |= TEXT_DRAW_MULTILINE | TEXT_DRAW_WORDBREAK;
        }
        if( rEdit.Password )
            rNewWidget.m_nFlags |= 0x00002000;
        if( rEdit.FileSelect && m_aContext.Version > PDFWriter::PDF_1_3 )
            rNewWidget.m_nFlags |= 0x00100000;
        rNewWidget.m_nMaxLen = rEdit.MaxLen;
        rNewWidget.m_aValue = rEdit.Text;

        createDefaultEditAppearance( rNewWidget, rEdit );
    }

    // convert to default user space now, since the mapmode may change
    // note: create default appearances before m_aRect gets transformed
    m_aPages[ nPageNr ].convertRect( rNewWidget.m_aRect );

    // insert widget to page's annotation list
    m_aPages[ nPageNr ].m_aAnnotations.push_back( rNewWidget.m_nObject );

    // mark page as having widgets
    m_aPages[ nPageNr ].m_bHasWidgets = true;

    return nNewWidget;
}

void PDFWriterImpl::beginControlAppearance( sal_Int32 nControl )
{
    if( nControl < 0 || nControl >= (sal_Int32)m_aWidgets.size() )
        return;

    PDFWidget& rWidget = m_aWidgets[ nControl ];
    m_nCurrentControl = nControl;

    SvMemoryStream* pControlStream = new SvMemoryStream( 1024, 1024 );
    // back conversion of control rect to current MapMode; necessary because
    // MapMode between createControl and beginControlAppearance
    // could have changed; therefore the widget rectangle is
    // already converted
    Rectangle aBack( Point( rWidget.m_aRect.Left(), pointToPixel(m_aPages[m_nCurrentPage].getHeight()) - rWidget.m_aRect.Top() - rWidget.m_aRect.GetHeight() ),
                     rWidget.m_aRect.GetSize() );
    aBack = lcl_convert( m_aMapMode,
                         m_aGraphicsStack.front().m_aMapMode,
                         getReferenceDevice(),
                         aBack );
    beginRedirect( pControlStream, aBack );
    writeBuffer( "/Tx BMC\n", 8 );
}

bool PDFWriterImpl::endControlAppearance( PDFWriter::WidgetState eState )
{
    bool bRet = false;
    if( ! m_aOutputStreams.empty() )
        writeBuffer( "\nEMC\n", 5 );
    SvMemoryStream* pAppearance = static_cast<SvMemoryStream*>(endRedirect());
    if( pAppearance && m_nCurrentControl >= 0 && m_nCurrentControl < (sal_Int32)m_aWidgets.size() )
    {
        PDFWidget& rWidget = m_aWidgets[ m_nCurrentControl ];
        OString aState, aStyle;
        switch( rWidget.m_eType )
        {
            case PDFWriter::PushButton:
                if( eState == PDFWriter::Up || eState == PDFWriter::Down )
                {
                    aState = (eState == PDFWriter::Up) ? "N" : "D";
                    aStyle = "Standard";
                }
                break;
            case PDFWriter::CheckBox:
                if( eState == PDFWriter::Up || eState == PDFWriter::Down )
                {
                    aState = "N";
                    aStyle = (eState == PDFWriter::Up) ? "Off" : "Yes";
                    /* cf PDFReference 3rd ed. V1.4 p539:
                       recommended name for on state is "Yes",
                       recommended name for off state is "Off"
                     */
                }
                break;
            case PDFWriter::RadioButton:
                if( eState == PDFWriter::Up || eState == PDFWriter::Down )
                {
                    aState = "N";
                    if( eState == PDFWriter::Up )
                        aStyle = "Off";
                    else
                    {
                        OStringBuffer aBuf( rWidget.m_aOnValue.getLength()*2 );
                        appendName( rWidget.m_aOnValue, aBuf );
                        aStyle = aBuf.makeStringAndClear();
                    }
                }
                break;
            case PDFWriter::Edit:
                aState = "N";
                aStyle = "Standard";
                break;
            case PDFWriter::ListBox:
            case PDFWriter::ComboBox:
                break;
        }
        if( aState.getLength() && aStyle.getLength() )
        {
            // delete eventual existing stream
            PDFAppearanceStreams::iterator it =
                rWidget.m_aAppearances[ aState ].find( aStyle );
            if( it != rWidget.m_aAppearances[ aState ].end() )
                delete it->second;
            rWidget.m_aAppearances[ aState ][ aStyle ] = pAppearance;
            bRet = true;
        }
    }

    if( ! bRet )
        delete pAppearance;

    m_nCurrentControl = -1;

    return bRet;
}

void PDFWriterImpl::addStream( const String& rMimeType, PDFOutputStream* pStream, bool bCompress )
{
#if defined USE_JAVA && defined MACOSX
    // Avoid deletion of streams in first pass
    if ( !isReplayWriter() )
        return;
#endif	// USE_JAVA && MACOSX

    if( pStream )
    {
        m_aAdditionalStreams.push_back( PDFAddStream() );
        PDFAddStream& rStream = m_aAdditionalStreams.back();
        rStream.m_aMimeType = rMimeType.Len()
                              ? OUString( rMimeType )
                              : OUString( RTL_CONSTASCII_USTRINGPARAM( "application/octet-stream" ) );
        rStream.m_pStream = pStream;
        rStream.m_bCompress = bCompress;
    }
}

/*************************************************************
begin i12626 methods

Implements Algorithm 3.2, step 1 only
*/
void PDFWriterImpl::padPassword( rtl::OUString aPassword, sal_uInt8 *paPasswordTarget )
{
// get ansi-1252 version of the password string CHECKIT ! i12626
    rtl::OString aString = rtl::OUStringToOString( aPassword, RTL_TEXTENCODING_MS_1252 );

//copy the string to the target
    sal_Int32 nToCopy = ( aString.getLength() < 32 ) ? aString.getLength() : 32;
    sal_Int32 nCurrentChar;

    for( nCurrentChar = 0; nCurrentChar < nToCopy; nCurrentChar++ )
        paPasswordTarget[nCurrentChar] = (sal_uInt8)( aString.getStr()[nCurrentChar] );

//pad it
    if( nCurrentChar < 32 )
    {//fill with standard byte string
        sal_Int32 i,y;
        for( i = nCurrentChar, y = 0 ; i < 32; i++, y++ )
            paPasswordTarget[i] = m_nPadString[y];
    }
}

/**********************************
Algorithm 3.2  Compute the encryption key used

step 1 should already be done before calling, the paThePaddedPassword parameter should contain
the padded password and must be 32 byte long, the encryption key is returned into the paEncryptionKey parameter,
it will be 16 byte long for 128 bit security; for 40 bit security only the first 5 bytes are used

TODO: in pdf ver 1.5 and 1.6 the step 6 is different, should be implemented. See spec.

*/
void PDFWriterImpl::computeEncryptionKey(sal_uInt8 *paThePaddedPassword, sal_uInt8 *paEncryptionKey )
{
//step 2
    if( m_aDigest )
    {
        rtlDigestError nError = rtl_digest_updateMD5( m_aDigest, paThePaddedPassword, ENCRYPTED_PWD_SIZE );
//step 3
        if( nError == rtl_Digest_E_None )
            nError = rtl_digest_updateMD5( m_aDigest, m_nEncryptedOwnerPassword , sizeof( m_nEncryptedOwnerPassword ) );
//Step 4
        sal_uInt8 nPerm[4];

        nPerm[0] = (sal_uInt8)m_nAccessPermissions;
        nPerm[1] = (sal_uInt8)( m_nAccessPermissions >> 8 );
        nPerm[2] = (sal_uInt8)( m_nAccessPermissions >> 16 );
        nPerm[3] = (sal_uInt8)( m_nAccessPermissions >> 24 );

        if( nError == rtl_Digest_E_None )
            nError = rtl_digest_updateMD5( m_aDigest, nPerm , sizeof( nPerm ) );

//step 5, get the document ID, binary form
        if( nError == rtl_Digest_E_None )
            nError = rtl_digest_updateMD5( m_aDigest, m_nDocID , sizeof( m_nDocID ) );
//get the digest
        sal_uInt8 nMD5Sum[ RTL_DIGEST_LENGTH_MD5 ];
        if( nError == rtl_Digest_E_None )
        {
            rtl_digest_getMD5( m_aDigest, nMD5Sum, sizeof( nMD5Sum ) );

//step 6, only if 128 bit
            if( m_aContext.Security128bit )
            {
                for( sal_Int32 i = 0; i < 50; i++ )
                {
                    nError = rtl_digest_updateMD5( m_aDigest, &nMD5Sum, sizeof( nMD5Sum ) );
                    if( nError != rtl_Digest_E_None )
                        break;
                    rtl_digest_getMD5( m_aDigest, nMD5Sum, sizeof( nMD5Sum ) );
                }
            }
        }
//Step 7
        for( sal_Int32 i = 0; i < MD5_DIGEST_SIZE; i++ )
            paEncryptionKey[i] = nMD5Sum[i];
    }
}

/**********************************
Algorithm 3.3  Compute the encryption dictionary /O value, save into the class data member
the step numbers down here correspond to the ones in PDF v.1.4 specfication
*/
void PDFWriterImpl::computeODictionaryValue()
{
//step 1 already done, data is in m_nPaddedOwnerPassword
//step 2
    if( m_aDigest )
    {
        rtlDigestError nError = rtl_digest_updateMD5( m_aDigest, &m_nPaddedOwnerPassword, sizeof( m_nPaddedOwnerPassword ) );
        if( nError == rtl_Digest_E_None )
        {
            sal_uInt8 nMD5Sum[ RTL_DIGEST_LENGTH_MD5 ];

            rtl_digest_getMD5( m_aDigest, nMD5Sum, sizeof(nMD5Sum) );
//step 3, only if 128 bit
            if( m_aContext.Security128bit )
            {
                sal_Int32 i;
                for( i = 0; i < 50; i++ )
                {
                    nError = rtl_digest_updateMD5( m_aDigest, nMD5Sum, sizeof( nMD5Sum ) );
                    if( nError != rtl_Digest_E_None )
                        break;
                    rtl_digest_getMD5( m_aDigest, nMD5Sum, sizeof( nMD5Sum ) );
                }
            }
//Step 4, the key is in nMD5Sum
//step 5 already done, data is in m_nPaddedUserPassword
//step 6
            rtl_cipher_initARCFOUR( m_aCipher, rtl_Cipher_DirectionEncode,
                                    nMD5Sum, m_nKeyLength , NULL, 0 );
// encrypt the user password using the key set above
            rtl_cipher_encodeARCFOUR( m_aCipher, m_nPaddedUserPassword, sizeof( m_nPaddedUserPassword ), // the data to be encrypted
                                      m_nEncryptedOwnerPassword, sizeof( m_nEncryptedOwnerPassword ) ); //encrypted data, stored in class data member
//Step 7, only if 128 bit
            if( m_aContext.Security128bit )
            {
                sal_uInt32 i, y;
                sal_uInt8 nLocalKey[ SECUR_128BIT_KEY ]; // 16 = 128 bit key

                for( i = 1; i <= 19; i++ ) // do it 19 times, start with 1
                {
                    for( y = 0; y < sizeof( nLocalKey ); y++ )
                        nLocalKey[y] = (sal_uInt8)( nMD5Sum[y] ^ i );

                    rtl_cipher_initARCFOUR( m_aCipher, rtl_Cipher_DirectionEncode,
                                            nLocalKey, SECUR_128BIT_KEY, NULL, 0 ); //destination data area, on init can be NULL
                    rtl_cipher_encodeARCFOUR( m_aCipher, m_nEncryptedOwnerPassword, sizeof( m_nEncryptedOwnerPassword ), // the data to be encrypted
                                              m_nEncryptedOwnerPassword, sizeof( m_nEncryptedOwnerPassword ) ); // encrypted data, can be the same as the input, encrypt "in place"
//step 8, store in class data member
                }
            }
        }
    }
}

/**********************************
Algorithms 3.4 and 3.5  Compute the encryption dictionary /U value, save into the class data member, revision 2 (40 bit) or 3 (128 bit)
*/
void PDFWriterImpl::computeUDictionaryValue()
{
//step 1, common to both 3.4 and 3.5
    computeEncryptionKey( m_nPaddedUserPassword , m_nEncryptionKey );

    if( m_aContext.Security128bit == false )
    {
//3.4
//step 2 and 3
        rtl_cipher_initARCFOUR( m_aCipher, rtl_Cipher_DirectionEncode,
                                    m_nEncryptionKey, 5 , // key and key length
                                    NULL, 0 ); //destination data area
// encrypt the user password using the key set above, save for later use
        rtl_cipher_encodeARCFOUR( m_aCipher, m_nPadString, sizeof( m_nPadString ), // the data to be encrypted
                                  m_nEncryptedUserPassword, sizeof( m_nEncryptedUserPassword ) ); //encrypted data, stored in class data member
    }
    else
    {
//or 3.5, for 128 bit security
//step6, initilize the last 16 bytes of the encrypted user password to 0
        for(sal_uInt32 i = MD5_DIGEST_SIZE; i < sizeof( m_nEncryptedUserPassword ); i++)
            m_nEncryptedUserPassword[i] = 0;
//step 2
        if( m_aDigest )
        {
            rtlDigestError nError = rtl_digest_updateMD5( m_aDigest, m_nPadString, sizeof( m_nPadString ) );
//step 3
            if( nError == rtl_Digest_E_None )
                nError = rtl_digest_updateMD5( m_aDigest, m_nDocID , sizeof(m_nDocID) );

            sal_uInt8 nMD5Sum[ RTL_DIGEST_LENGTH_MD5 ];
            rtl_digest_getMD5( m_aDigest, nMD5Sum, sizeof(nMD5Sum) );
//Step 4
            rtl_cipher_initARCFOUR( m_aCipher, rtl_Cipher_DirectionEncode,
                                    m_nEncryptionKey, SECUR_128BIT_KEY, NULL, 0 ); //destination data area
            rtl_cipher_encodeARCFOUR( m_aCipher, nMD5Sum, sizeof( nMD5Sum ), // the data to be encrypted
                                      m_nEncryptedUserPassword, sizeof( nMD5Sum ) ); //encrypted data, stored in class data member
//step 5
            sal_uInt32 i, y;
            sal_uInt8 nLocalKey[SECUR_128BIT_KEY];

            for( i = 1; i <= 19; i++ ) // do it 19 times, start with 1
            {
                for( y = 0; y < sizeof( nLocalKey ) ; y++ )
                    nLocalKey[y] = (sal_uInt8)( m_nEncryptionKey[y] ^ i );

                rtl_cipher_initARCFOUR( m_aCipher, rtl_Cipher_DirectionEncode,
                                        nLocalKey, SECUR_128BIT_KEY, // key and key length
                                        NULL, 0 ); //destination data area, on init can be NULL
                rtl_cipher_encodeARCFOUR( m_aCipher, m_nEncryptedUserPassword, SECUR_128BIT_KEY, // the data to be encrypted
                                          m_nEncryptedUserPassword, SECUR_128BIT_KEY ); // encrypted data, can be the same as the input, encrypt "in place"
            }
        }
    }
}

/* init the encryption engine
1. init the document id, used both for building the document id and for building the encryption key(s)
2. build the encryption key following algorithms described in the PDF specification
 */
void PDFWriterImpl::initEncryption()
{
    m_aOwnerPassword = m_aContext.OwnerPassword;
    m_aUserPassword = m_aContext.UserPassword;
/* password stuff computing, before sending out anything */
    DBG_ASSERT( m_aCipher != NULL, "PDFWriterImpl::initEncryption: a cipher (ARCFOUR) object is not available !" );
    DBG_ASSERT( m_aDigest != NULL, "PDFWriterImpl::initEncryption: a digest (MD5) object is not available !" );

    if( m_aCipher && m_aDigest )
    {
//if there is no owner password, force it to the user password
        if( m_aOwnerPassword.getLength() == 0 )
            m_aOwnerPassword = m_aUserPassword;

        initPadString();
/*
1) pad passwords
*/
        padPassword( m_aOwnerPassword, m_nPaddedOwnerPassword );
        padPassword( m_aUserPassword, m_nPaddedUserPassword );
/*
2) compute the access permissions, in numerical form

the default value depends on the revision 2 (40 bit) or 3 (128 bit security):
- for 40 bit security the unused bit must be set to 1, since they are not used
- for 128 bit security the same bit must be preset to 0 and set later if needed
according to the table 3.15, pdf v 1.4 */
        m_nAccessPermissions = ( m_aContext.Security128bit ) ? 0xfffff0c0 : 0xffffffc0 ;

/* check permissions for 40 bit security case */
        m_nAccessPermissions |= ( m_aContext.AccessPermissions.CanPrintTheDocument ) ?  1 << 2 : 0;
        m_nAccessPermissions |= ( m_aContext.AccessPermissions.CanModifyTheContent ) ? 1 << 3 : 0;
        m_nAccessPermissions |= ( m_aContext.AccessPermissions.CanCopyOrExtract ) ?   1 << 4 : 0;
        m_nAccessPermissions |= ( m_aContext.AccessPermissions.CanAddOrModify ) ? 1 << 5 : 0;
        m_nKeyLength = SECUR_40BIT_KEY;
        m_nRC4KeyLength = SECUR_40BIT_KEY+5; // for this value see PDF spec v 1.4, algorithm 3.1 step 4, where n is 5

        if( m_aContext.Security128bit )
        {
            m_nKeyLength = SECUR_128BIT_KEY;
            m_nRC4KeyLength = 16; // for this value see PDF spec v 1.4, algorithm 3.1 step 4, where n is 16, thus maximum
                                  // permitted value is 16
            m_nAccessPermissions |= ( m_aContext.AccessPermissions.CanFillInteractive ) ?         1 << 8 : 0;
            m_nAccessPermissions |= ( m_aContext.AccessPermissions.CanExtractForAccessibility ) ? 1 << 9 : 0;
            m_nAccessPermissions |= ( m_aContext.AccessPermissions.CanAssemble ) ?                1 << 10 : 0;
            m_nAccessPermissions |= ( m_aContext.AccessPermissions.CanPrintFull ) ?               1 << 11 : 0;
        }
        computeODictionaryValue();
        computeUDictionaryValue();

//clear out exceding key values, prepares for generation number default to 0 as well
// see checkAndEnableStreamEncryption in pdfwriter_impl.hxx
        sal_Int32 i, y;
        for( i = m_nKeyLength, y = 0; y < 5 ; y++ )
            m_nEncryptionKey[i++] = 0;
    }
    else //either no cipher or no digest or both, something is wrong with memory or something else
        m_aContext.Encrypt = false; //then turn the encryption off
}
/* end i12626 methods */

#if defined USE_JAVA && defined MACOSX

sal_Int32 PDFWriterImpl::getNextPDFObject( oslFileHandle aFile, PDFObjectMapping& rObjectMapping )
{
    sal_uInt64 nLastNewlinePos;
    CHECK_RETURN( osl_File_E_None == osl_getFilePos( aFile, &nLastNewlinePos ) );

    PDFEmitObject aObj;
    const sal_Char *pStart = " 0 obj\n";
    sal_uInt64 nStartLen = strlen( pStart );
    bool bStartFound = false;   
    sal_uInt64 nBufSize = 1024;
    sal_Char aBuf[ nBufSize ];
    sal_uInt64 nBytesRead;

    // Find start of next object
    while ( !bStartFound && osl_readFile( aFile, aBuf, nBufSize, &nBytesRead ) == osl_File_E_None )
    {
        sal_Bool bEOF = sal_False;
        CHECK_RETURN( osl_File_E_None == osl_isEndOfFile( aFile, &bEOF ) );

        if ( !nBytesRead )
        {
            if ( bEOF )
                return 0;
            else
                continue;
        }

        // Look for beginning token
        if ( nBytesRead < nStartLen )
        {
            if ( bEOF )
                return 0;

            sal_uInt64 nOffset = nBytesRead;
            sal_uInt64 nTmpBytesRead;
            while ( osl_readFile( aFile, aBuf + nOffset, nBufSize - nOffset, &nTmpBytesRead ) == osl_File_E_None )
            {
                nOffset += nTmpBytesRead;
                if ( nOffset >= nStartLen )
                    break;

                CHECK_RETURN( osl_File_E_None == osl_isEndOfFile( aFile, &bEOF ) );

                if ( bEOF )
                    break;
            }

            nBytesRead = nOffset;
        }

        // Skip bytes at the end so that split tokens are handled in the
        // next pass
        sal_uInt64 nSkippedBytes = nStartLen - 1;
        nBytesRead -= nSkippedBytes;
        sal_Char *pBuf = aBuf;
        sal_uInt64 i;
        for ( i = 0; i < nBytesRead; i++, pBuf++ )
        {
            if ( *pBuf == *pStart && !strncmp( pBuf, pStart, nStartLen ) )
            {
                nSkippedBytes += nBytesRead - i - nStartLen;
                nBytesRead = i + nStartLen;
                bStartFound = true;
                break;
            }
            else if ( *pBuf == '\n' )
            {
                CHECK_RETURN( osl_File_E_None == osl_getFilePos( aFile, &nLastNewlinePos ) );
                nLastNewlinePos -= nBytesRead + nSkippedBytes - i;
            }
        }

        CHECK_RETURN( osl_File_E_None == osl_setFilePos( aFile, osl_Pos_Current, nSkippedBytes * -1 ) );
    }

    if ( !bStartFound )
        return 0;

    // Parse object ID
    sal_uInt64 nEndOfStartPos;
    CHECK_RETURN( osl_File_E_None == osl_getFilePos( aFile, &nEndOfStartPos ) );
    CHECK_RETURN( osl_File_E_None == osl_setFilePos( aFile, osl_Pos_Absolut, nLastNewlinePos ) );

    OStringBuffer aIDBuf;
    bool bSpaceFound = false;
    sal_uInt64 nMaxBytes = nEndOfStartPos - nLastNewlinePos;
    while ( !bSpaceFound && nMaxBytes && osl_readFile( aFile, aBuf, nBufSize, &nBytesRead ) == osl_File_E_None )
    {
        sal_Bool bEOF = sal_False;
        CHECK_RETURN( osl_File_E_None == osl_isEndOfFile( aFile, &bEOF ) );

        if ( !nBytesRead )
        {
            if ( bEOF )
                return 0;
            else
                continue;
        }

        if ( nMaxBytes < nBytesRead )
            nBytesRead = nMaxBytes;
        sal_Char *pBuf = aBuf;
        sal_uInt64 i;
        for ( i = 0; i < nBytesRead; i++, pBuf++ )
        {
            if ( *pBuf == ' ' )
            {
                bSpaceFound = true;
                break;
            }
        }

        aIDBuf.append( aBuf, i );
        nMaxBytes -= i;
    }

    aObj.m_nID = aIDBuf.makeStringAndClear().toInt32();
    if ( !aObj.m_nID )
        return 0;
    CHECK_RETURN( osl_File_E_None == osl_setFilePos( aFile, osl_Pos_Absolut, nEndOfStartPos ) );

    const sal_Char *pEnd = "\nendobj\n";
    sal_uInt64 nEndLen = strlen( pEnd );
    bool bEndFound = false;    

    // Find end of object
    while ( !bEndFound && osl_readFile( aFile, aBuf, nBufSize, &nBytesRead ) == osl_File_E_None )
    {
        sal_Bool bEOF = sal_False;
        CHECK_RETURN( osl_File_E_None == osl_isEndOfFile( aFile, &bEOF ) );

        if ( !nBytesRead )
        {
            if ( bEOF )
                break;
            else
                continue;
        }

        // Look for ending token
        if ( nBytesRead < nEndLen )
        {
            if ( bEOF )
                return 0;

            sal_uInt64 nOffset = nBytesRead;
            sal_uInt64 nTmpBytesRead;
            while ( osl_readFile( aFile, aBuf + nOffset, nBufSize - nOffset, &nTmpBytesRead ) == osl_File_E_None )
            {
                nOffset += nTmpBytesRead;
                if ( nOffset >= nEndLen )
                    break;

                CHECK_RETURN( osl_File_E_None == osl_isEndOfFile( aFile, &bEOF ) );

                if ( bEOF )
                    break;
            }

            nBytesRead = nOffset;
        }

        // Skip bytes at the end so that split tokens are handled in the
        // next pass
        sal_uInt64 nSkippedBytes = nEndLen - 1;
        nBytesRead -= nSkippedBytes;
        sal_Char *pBuf = aBuf;
        for ( sal_uInt64 i = 0; i < nBytesRead; i++, pBuf++ )
        {
            if ( *pBuf == *pEnd && !strncmp( pBuf, pEnd, nEndLen ) )
            {
                nSkippedBytes += nBytesRead - i;
                nBytesRead = i;
                bEndFound = true;
                break;
            }
        }

        CHECK_RETURN( osl_File_E_None == osl_setFilePos( aFile, osl_Pos_Current, nSkippedBytes * -1 ) );
    }

    if ( !bEndFound )
        return 0;

    sal_uInt64 nStartOfEndPos;
    CHECK_RETURN( osl_File_E_None == osl_getFilePos( aFile, &nStartOfEndPos ) );
    CHECK_RETURN( osl_File_E_None == osl_setFilePos( aFile, osl_Pos_Absolut, nEndOfStartPos ) );

    // Cache content and find the stream if one exists
    OStringBuffer aContentBuf;
    const sal_Char *pStreamStart = "\nstream\n";
    sal_uInt64 nStreamStartLen = strlen( pStreamStart );
    nMaxBytes = nStartOfEndPos - nEndOfStartPos;
    while ( !aObj.m_bStream && nMaxBytes && osl_readFile( aFile, aBuf, nBufSize, &nBytesRead ) == osl_File_E_None )
    {
        sal_Bool bEOF = sal_False;
        CHECK_RETURN( osl_File_E_None == osl_isEndOfFile( aFile, &bEOF ) );

        if ( !nBytesRead )
        {
            if ( bEOF )
                break;
            else
                continue;
        }

        // Look for beginning token
        if ( nBytesRead < nStreamStartLen )
        {
            if ( bEOF )
            {
                aContentBuf.append( aBuf, nMaxBytes < nBytesRead ? nMaxBytes : nBytesRead );
                break;
            }

            sal_uInt64 nOffset = nBytesRead;
            sal_uInt64 nTmpBytesRead;
            while ( osl_readFile( aFile, aBuf + nOffset, nBufSize - nOffset, &nTmpBytesRead ) == osl_File_E_None )
            {
                if ( nOffset >= nStreamStartLen )
                    break;

                CHECK_RETURN( osl_File_E_None == osl_isEndOfFile( aFile, &bEOF ) );

                if ( bEOF )
                    break;
            }

            nBytesRead = nOffset;
        }

        // Skip bytes at the end so that split tokens are handled in the
        // next pass
        sal_uInt64 nSkippedBytes = nStreamStartLen - 1;
        nBytesRead -= nSkippedBytes;
        sal_Char *pBuf = aBuf;
        sal_uInt64 i = 0;
        for ( i = 0; i < nMaxBytes && i < nBytesRead; i++, pBuf++ )
        {
            if ( nStreamStartLen <= nMaxBytes - i && *pBuf == *pStreamStart && !strncmp( pBuf, pStreamStart, nStreamStartLen ) )
            {
                nSkippedBytes += nBytesRead - i - nStreamStartLen;
                nBytesRead = i + nStreamStartLen;
                CHECK_RETURN( osl_File_E_None == osl_getFilePos( aFile, &aObj.m_nStreamPos ) );
                aObj.m_nStreamPos -= nSkippedBytes;
                aObj.m_bStream = true;
                break;
            }
        }

        aContentBuf.append( aBuf, i );
        nMaxBytes -= i;

        CHECK_RETURN( osl_File_E_None == osl_setFilePos( aFile, osl_Pos_Current, nSkippedBytes * -1 ) );
    }

    sal_uInt64 nEndPos = nStartOfEndPos + nEndLen;
    CHECK_RETURN( osl_File_E_None == osl_setFilePos( aFile, osl_Pos_Absolut, nEndPos ) );

    // Replace all whitespace with spaces for ease of parsing
    for ( sal_Char *pBuf = (sal_Char *)aContentBuf.getStr(); *pBuf; pBuf++ )
    {
        switch ( *pBuf )
        {
            case 0x09:
            case 0x0A:
            case 0x0C:
            case 0x0D:
                *pBuf = 0x20;
        }
    }

    aObj.m_aContent = aContentBuf.makeStringAndClear();

    rObjectMapping[ aObj.m_nID ] = aObj;

    return aObj.m_nID;
}

sal_Int32 PDFWriterImpl::writePDFObjectTree( PDFEmitObject& rObj, oslFileHandle aFile, PDFObjectMapping& rObjMapping, sal_Int32 nFontID, std::map< sal_Int32, sal_Int32 >& rIDMapping )
{
    // Check if we have already handled this object
    std::map< sal_Int32, sal_Int32 >::iterator it = rIDMapping.find( rObj.m_nID );
    if ( it != rIDMapping.end() )
        return it->second;

    sal_Int32 nNewID = createObject();
    rIDMapping[ rObj.m_nID ] = nNewID;
    OStringBuffer aLine;
    aLine.append( nNewID );
    aLine.append( " 0 obj\n" );

    // Copy content and perform any required substitutions
    OString aRefTag( "0 R " );
    OString aDescendantFontsTag( "/DescendantFonts" );
    OString aDescendantFontsRefTag( " 0 R]" );
    OString aFontNameTag( "/FontName " );
    OString aBaseFontTag( "/BaseFont " );
    const sal_Char *pBuf = rObj.m_aContent.getStr();
    sal_Int32 nContentLen = rObj.m_aContent.getLength();
    sal_Int32 nCurrentPos = 0;
    while ( nCurrentPos < nContentLen )
    {
        // Find next space
        sal_Int32 nNextPos = rObj.m_aContent.indexOf( ' ', nCurrentPos );
        if ( nNextPos < 0 )
            nNextPos = nContentLen;
        else
            nNextPos++;

        sal_Int32 nCurrentLen = nNextPos - nCurrentPos;
        OString aStr( pBuf, nCurrentLen );
        sal_Int32 nNum = aStr.toInt32();
        sal_Int32 nNextNextPos;
        if ( nNum )
        {
            if ( nNextPos < nContentLen && rObj.m_aContent.match( aRefTag, nNextPos ) )
            {
                PDFEmitObject& rRefObj = rObjMapping[ nNum ];
                sal_Int32 nRefID = writePDFObjectTree( rRefObj, aFile, rObjMapping, nFontID, rIDMapping );
                if ( !nRefID )
                    return 0;
                aLine.append( nRefID );
                aLine.append( ' ' );
                aLine.append( aRefTag );
                nNextPos += aRefTag.getLength();
                nCurrentLen = nNextPos - nCurrentPos;
            }
            else
            {
                aLine.append( aStr );
            }
        }
        else if ( aStr.match( aDescendantFontsTag ) )
        {
            aLine.append( aStr );
            if ( nNextPos < nContentLen && ( nNextNextPos = rObj.m_aContent.indexOf( ']', nNextPos ) ) >= 0 )
            {
                nNextNextPos++;

                sal_Int32 nNextContentLen = nNextNextPos - nNextPos;
                const sal_Char *pNextBuf = rObj.m_aContent.getStr() + nNextPos;
                if ( nNextContentLen > aDescendantFontsRefTag.getLength() + 1 && pNextBuf[0] == '[' )
                {
                    OString aSubStr( pNextBuf + 1, nNextContentLen - 2 );
                    sal_Int32 nSubNum = aSubStr.toInt32();
                    if ( !nSubNum )
                        return 0;
                    PDFEmitObject& rRefObj = rObjMapping[ nSubNum ];
                    sal_Int32 nRefID = writePDFObjectTree( rRefObj, aFile, rObjMapping, nFontID, rIDMapping );
                    if ( !nRefID )
                        return 0;
                    aLine.append( '[' );
                    aLine.append( nRefID );
                    aLine.append( aDescendantFontsRefTag );
                    aLine.append( ' ' );
                }

                nNextPos = nNextNextPos + 1;
                nCurrentLen = nNextPos - nCurrentPos;
            }
        }
        else if ( aStr.match( aFontNameTag ) || aStr.match( aBaseFontTag ) )
        {
            aLine.append( aStr );
            if ( nNextPos < nContentLen && ( nNextNextPos = rObj.m_aContent.indexOf( ' ', nNextPos ) ) >= 0 )
            {
                nNextNextPos++;

                bool bAppendSubset;
                sal_Int32 nNextContentLen = nNextNextPos - nNextPos;
                const sal_Char *pNextBuf = rObj.m_aContent.getStr() + nNextPos;
                if ( nNextContentLen > 8 && pNextBuf[0] == '/' && pNextBuf[7] == '+' && pNextBuf[8] != ' ' )
                {
                    nNextContentLen -= 8;
                    pNextBuf += 8;
                    bAppendSubset = true;
                }
                else
                {
                    nNextContentLen -= 1;
                    pNextBuf += 1;
                    bAppendSubset = false;
                }
                OUString aFontName( pNextBuf, nNextContentLen - 1, RTL_TEXTENCODING_UTF8 );
                aLine.append( '/' );
                if ( bAppendSubset )
                    appendSubsetName( nFontID, aFontName, aLine );
                aLine.append( ' ' );

                nNextPos = nNextNextPos;
                nCurrentLen = nNextPos - nCurrentPos;
            }
        }
        else
        {
            aLine.append( aStr );
        }

        pBuf += nCurrentLen;
        nCurrentPos += nCurrentLen;
    }
    aLine.append( "\n" );

    // Write the object
    CHECK_RETURN( updateObject( nNewID ) );
    CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );
    if ( rObj.m_bStream )
    {
        CHECK_RETURN( ( osl_File_E_None == osl_setFilePos( aFile, osl_Pos_Absolut, rObj.m_nStreamPos ) ) );

        aLine.setLength( 0 );
        aLine.append( "stream\n" );
        CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );

        checkAndEnableStreamEncryption( nNewID );

        sal_uInt64 nBufSize = 8192;
        sal_Char aBuf[ nBufSize ];
        sal_uInt64 nBytesRead;
        sal_uInt64 nBytesLeft = rObj.m_nStreamLen;
        sal_Bool bEOF = sal_False;
        do
        {
            CHECK_RETURN( ( osl_File_E_None == osl_readFile( aFile, aBuf, nBufSize, &nBytesRead ) ) );
            if ( nBytesLeft < nBytesRead )
                nBytesRead = nBytesLeft;
            nBytesLeft -= nBytesRead;
            CHECK_RETURN( writeBuffer( aBuf, nBytesRead ) );
            CHECK_RETURN( ( osl_File_E_None == osl_isEndOfFile( aFile, &bEOF ) ) );
        } while ( nBytesLeft && !bEOF );

        disableStreamEncryption();

        aLine.setLength( 0 );
        aLine.append( "\nendstream\n" );
        CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );
    }

    aLine.setLength( 0 );
    aLine.append( "endobj\n" );
    CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );

    return nNewID;
}

void PDFWriterImpl::encodeGlyphs()
{
    // This cannot be called within the replay writer
    if ( isReplayWriter() )
        return;

    // Create font objects using Mac OS X's PDF rendering APIs
    for ( FontSubsetData::iterator it = m_aSubsets.begin(); it != m_aSubsets.end(); ++it )
    {
        CTFontRef aFont = CTFontCreateCopyWithAttributes( (CTFontRef)it->first, 12.0f, NULL, NULL );
        if ( !aFont )
            continue;

        FontSubset& rSubset = it->second;
        for ( FontEmitList::iterator lit = rSubset.m_aSubsets.begin(); lit != rSubset.m_aSubsets.end(); ++lit )
        {
            FontEmit& rEmit = *lit;

            // Fix bug 3600 by including zero glyphs in CGPDFContext
            CGGlyph aGlyphIDs[ 256 ];
            int nGlyphIDs = 0;
            for ( FontEmitMapping::iterator fit = rEmit.m_aMapping.begin(); fit != rEmit.m_aMapping.end(); ++fit )
                aGlyphIDs[ nGlyphIDs++ ] = (CGGlyph)fit->first;

            if ( !nGlyphIDs )
                continue;

            OUString aTmpName( utl::TempFile::CreateTempName() );

            CFStringRef aPath = CFStringCreateWithCharactersNoCopy( NULL, aTmpName.getStr(), aTmpName.getLength(), kCFAllocatorNull );
            if ( aPath )
            {
                CFURLRef aURL = CFURLCreateWithFileSystemPath( NULL, aPath, kCFURLPOSIXPathStyle, false );
                if ( aURL )
                {
                    CGContextRef aContext = CGPDFContextCreateWithURL( aURL, NULL, NULL );
                    if ( aContext )
                    {
                        CGPoint aGlyphPositions[ 256 ];
                        CGSize aGlyphSizes[ 256 ];
                        memset( aGlyphSizes, 0, sizeof( aGlyphSizes ) );
                        CTFontGetAdvancesForGlyphs( aFont, kCTFontDefaultOrientation, aGlyphIDs, aGlyphSizes, nGlyphIDs );
                        aGlyphPositions[ 0 ].x = 0.0f;
                        aGlyphPositions[ 0 ].y = 0.0f;
                        for ( int i = 1; i < nGlyphIDs; i++ )
                        {
                            aGlyphPositions[ i ].x = aGlyphPositions[ i - 1 ].x + aGlyphSizes[ i - 1 ].width;
                            aGlyphPositions[ i ].y = aGlyphPositions[ i - 1 ].y + aGlyphSizes[ i - 1 ].height;
                        }

                        CGContextBeginPage( aContext, NULL );
                        CTFontDrawGlyphs( aFont, aGlyphIDs, aGlyphPositions, nGlyphIDs, aContext );
                        CGContextEndPage( aContext );
                        CGContextRelease( aContext );
                    }

                    CFRelease( aURL );
                }

                CFRelease( aPath );
            }

            osl_getFileURLFromSystemPath( aTmpName.pData, &rEmit.m_aFontFileName.pData );

            oslFileHandle aFile;
            if ( osl_openFile( rEmit.m_aFontFileName.pData, &aFile, osl_File_OpenFlag_Read ) == osl_File_E_None )
            {
                // Get the PDF objects from the file and parse the page content
                // and font objects
                OString aPageObjTag( "<< /Type /Page " );
                OString aPageContentTag( " /Contents " );
                OString aProcSetObjTag( "<< /ProcSet " );
                OString aFontDescriptorObjTag( "<< /Type /FontDescriptor " );
                OString aFontFileTag( " /FontFile " );
                OString aFontFile2Tag( " /FontFile2 " );
    			OString aRefTag( " 0 R" );
                sal_Int32 nPageContentObjID = 0;
                sal_Int32 nProcSetObjID = 0;
                sal_Int32 nFontDescriptorObjID = 0;
                sal_Int32 nFontFile2ObjStart = 0;
                sal_Int32 nFontFile2ObjLen = 0;
                sal_Int32 nFontFile2ObjID = 0;
                sal_Int32 nObjID = 0;
                while ( ( nObjID = getNextPDFObject( aFile, rEmit.m_aObjectMapping ) ) > 0 )
                {
                    PDFEmitObject& rObj = rEmit.m_aObjectMapping[ nObjID ];
                    if ( !nPageContentObjID && rObj.m_aContent.match( aPageObjTag ) )
                    {
                        if ( !rObj.m_bStream )
                        {
                            sal_Int32 nContentPos = rObj.m_aContent.indexOf( aPageContentTag );
                            if ( nContentPos >= 0 )
                            {
                                // Find object reference
                                OStringBuffer aIDBuf;
                                nContentPos += aPageContentTag.getLength();
                                const sal_Char *pBuf = rObj.m_aContent.getStr();
                                for ( pBuf += nContentPos; *pBuf && *pBuf != ' '; pBuf++ )
                                    aIDBuf.append( *pBuf );
                                nPageContentObjID = aIDBuf.makeStringAndClear().toInt32();
                            }
                        }
                        else
                        {
                            nPageContentObjID = nObjID;
                        }
                    }
                    else if ( !nProcSetObjID && rObj.m_aContent.match( aProcSetObjTag ) )
                    {
                        nProcSetObjID = nObjID;
                    }
                    else if ( !nFontDescriptorObjID && rObj.m_aContent.match( aFontDescriptorObjTag ) )
                    {
                        if ( !rObj.m_bStream )
                        {
                            sal_Int32 nContentPos = 0;
                            sal_Int32 nFontFile2TagLength = 0;
                            if ( ( nContentPos = rObj.m_aContent.indexOf( aFontFileTag ) ) >= 0 )
                                nFontFile2TagLength = aFontFileTag.getLength();
                            else if ( ( nContentPos = rObj.m_aContent.indexOf( aFontFile2Tag ) ) >= 0 )
                                nFontFile2TagLength = aFontFile2Tag.getLength();

                            if ( nContentPos >= 0 )
                            {
                                nFontDescriptorObjID = nObjID;
                                nFontFile2ObjStart = nContentPos;

                                // Find object reference
                                OStringBuffer aIDBuf;
                                nContentPos += nFontFile2TagLength;
                                const sal_Char *pBuf = rObj.m_aContent.getStr();
                                for ( pBuf += nContentPos; *pBuf && *pBuf != ' '; pBuf++, nContentPos++ )
                                    aIDBuf.append( *pBuf );
                                nFontFile2ObjID = aIDBuf.makeStringAndClear().toInt32();

                                nFontFile2ObjLen = nContentPos - nFontFile2ObjStart;
                                if ( rObj.m_aContent.indexOf( aRefTag, nContentPos ) == nContentPos )
                                    nFontFile2ObjLen += aRefTag.getLength();
                            }
                        }
                    }
                }

                // Fix bug reported in the following NeoOffice forum topic by
                // removing the FontFile2 attribute if it references an object
                // that does not exist:
                // http://trinity.neooffice.org/modules.php?name=Forums&file=viewtopic&t=8572
                if ( nFontDescriptorObjID && nFontFile2ObjLen && ( !nFontFile2ObjID || rEmit.m_aObjectMapping.find( nFontFile2ObjID ) == rEmit.m_aObjectMapping.end() ) )
                {
                    PDFEmitObject& rObj = rEmit.m_aObjectMapping[ nFontDescriptorObjID ];
                    rObj.m_aContent = rObj.m_aContent.replaceAt( nFontFile2ObjStart, nFontFile2ObjLen, OString() );
                }

                // Update stream lengths
                OString aStreamLengthTag( "<< /Length " );
                OString aRef( " 0 R " );
                for ( PDFObjectMapping::iterator pit = rEmit.m_aObjectMapping.begin(); pit != rEmit.m_aObjectMapping.end(); ++pit )
                {
                    PDFEmitObject& rObj = pit->second;
                    if ( rObj.m_bStream )
                    {
                        // Find stream length
                        sal_Int32 nLenPos = rObj.m_aContent.indexOf( aStreamLengthTag );
                        if ( nLenPos >= 0 )
                        {
                            OStringBuffer aLenBuf;
                            nLenPos += aStreamLengthTag.getLength();
                            const sal_Char *pBuf = rObj.m_aContent.getStr();
                            for ( pBuf += nLenPos; *pBuf && *pBuf != ' '; pBuf++ )
                                aLenBuf.append( *pBuf );
                            sal_Int32 nStreamLen = aLenBuf.makeStringAndClear().toInt32();
                            if ( nStreamLen && !OString( pBuf ).compareTo( aRef, aRef.getLength() ) )
                                rObj.m_nStreamLen = rEmit.m_aObjectMapping[ nStreamLen ].m_aContent.toInt32();
                            else
                                rObj.m_nStreamLen = nStreamLen;
                        }
                    }
                }

                // Map font IDs to objects
                OString aFontIDTagF = OString( "/F" );
                OString aFontIDTagG = OString( "/G" );
                OString aFontIDTagTc = OString( "/Tc" );
                OString aFontIDTagTT = OString( "/TT" );
                OString aFontIDTagTy = OString( "/Ty" );
                OString aFontIDTagC = OString( "/C" );
                OString aFontIDTagCs = OString( "/Cs" );
                sal_Int32 nFontIDTagLenF = aFontIDTagF.getLength();
                sal_Int32 nFontIDTagLenG = aFontIDTagG.getLength();
                sal_Int32 nFontIDTagLenTc = aFontIDTagTc.getLength();
                sal_Int32 nFontIDTagLenTT = aFontIDTagTT.getLength();
                sal_Int32 nFontIDTagLenTy = aFontIDTagTy.getLength();
                sal_Int32 nFontIDTagLenC = aFontIDTagC.getLength();
                sal_Int32 nFontIDTagLenCs = aFontIDTagCs.getLength();
                if ( nProcSetObjID )
                {
                    PDFEmitObject& rObj = rEmit.m_aObjectMapping[ nProcSetObjID ];
                    OString aFontStartTag( " /Font << " );
                    sal_Int32 nFontStartTagLen = aFontStartTag.getLength();
                    OString aFontEndTag( " >> " );
                    sal_Int32 nCurrentPos = 0;
                    sal_Int32 nCurrentPosC = 0;
                    sal_Int32 nCurrentPosF = 0;
                    sal_Int32 nCurrentPosG = 0;
                    sal_Int32 nCurrentPosTc = 0;
                    sal_Int32 nCurrentPosTT = 0;
                    sal_Int32 nCurrentPosTy = 0;
                    sal_Int32 nCurrentFontIDTagLen;
                    sal_Int32 nFontStartPos;
                    if ( ( nFontStartPos = rObj.m_aContent.indexOf( aFontStartTag, nCurrentPos ) ) >= 0 )
                    {
                        nCurrentPos = nFontStartPos + nFontStartTagLen;

                        sal_Int32 nFontEndPos;
                        if ( ( nFontEndPos = rObj.m_aContent.indexOf( aFontEndTag, nCurrentPos ) ) >= 0 )
                        {
                            nCurrentPos = nFontStartPos + nFontStartTagLen;

                            while ( ( nCurrentPosF = rObj.m_aContent.indexOf( aFontIDTagF, nCurrentPos ) ) >= 0 || ( nCurrentPosG = rObj.m_aContent.indexOf( aFontIDTagG, nCurrentPos ) ) >= 0 || ( nCurrentPosTc = rObj.m_aContent.indexOf( aFontIDTagTc, nCurrentPos ) ) >= 0 || ( nCurrentPosTT = rObj.m_aContent.indexOf( aFontIDTagTT, nCurrentPos ) ) >= 0 || ( nCurrentPosTy = rObj.m_aContent.indexOf( aFontIDTagTy, nCurrentPos ) ) >= 0 || ( nCurrentPosC = rObj.m_aContent.indexOf( aFontIDTagC, nCurrentPos ) ) >= 0 )
                            {
                                if ( nCurrentPosF >= 0 )
                                {
                                    nCurrentPos = nCurrentPosF;
                                    nCurrentFontIDTagLen = nFontIDTagLenF;
                                }
                                else if ( nCurrentPosG >= 0 )
                                {
                                    nCurrentPos = nCurrentPosG;
                                    nCurrentFontIDTagLen = nFontIDTagLenG;
                                }
                                else if ( nCurrentPosTc >= 0 )
                                {
                                    nCurrentPos = nCurrentPosTc;
                                    nCurrentFontIDTagLen = nFontIDTagLenTc;
                                }
                                else if ( nCurrentPosTT >= 0 )
                                {
                                    nCurrentPos = nCurrentPosTT;
                                    nCurrentFontIDTagLen = nFontIDTagLenTT;
                                }
                                else if ( nCurrentPosTy >= 0 )
                                {
                                    nCurrentPos = nCurrentPosTy;
                                    nCurrentFontIDTagLen = nFontIDTagLenTy;
                                }
                                else if ( nCurrentPosC >= 0 )
                                {
									while ( rObj.m_aContent.indexOf( aFontIDTagCs, nCurrentPosC ) == nCurrentPosC )
									{
										nCurrentPosC += nFontIDTagLenCs;
                                		if ( ( nCurrentPosC = rObj.m_aContent.indexOf( aFontIDTagC, nCurrentPosC ) ) < 0 )
										{
                                    		nCurrentPos = nFontEndPos;
                                    		nCurrentFontIDTagLen = 0;
											break;
										}
									}

                                    nCurrentPos = nCurrentPosC;
                                    nCurrentFontIDTagLen = nFontIDTagLenC;
                                }
                                else
                                {
                                    nCurrentPos = nFontEndPos;
                                    nCurrentFontIDTagLen = 0;
                                }

                                if ( nCurrentPos >= nFontEndPos )
                                    break;

                                nCurrentPos += nCurrentFontIDTagLen;

                                sal_Int32 nNextPos = rObj.m_aContent.indexOf( ' ', nCurrentPos );
                                if ( nNextPos < 0 )
                                    continue;

                                sal_Int32 nNextNextPos = rObj.m_aContent.indexOf( aRef, nCurrentPos );
                                if ( nNextNextPos < 0 )
                                    continue;

                                const sal_Char *pBuf = rObj.m_aContent.getStr();
                                OString aFontSubID( pBuf + nCurrentPos, nNextPos - nCurrentPos );
                                sal_Int32 nFontSubID = OString( pBuf + nNextPos, nNextNextPos - nNextPos ).toInt32();
                                if ( nFontSubID && aFontSubID.getLength() )
                                    rEmit.m_aFontSubIDMapping[ aFontSubID ] = nFontSubID;
                            }
                        }
                    }
                }

                // Inflate page content stream and get encoding
                if ( nPageContentObjID )
                {
                    PDFEmitObject& rObj = rEmit.m_aObjectMapping[ nPageContentObjID ];
                    if ( rObj.m_bStream && rObj.m_nStreamLen && osl_setFilePos( aFile, osl_Pos_Absolut, rObj.m_nStreamPos ) == osl_File_E_None )
                    {
                        ZCodec aInflater( 0x4000, 0x4000 );
                        SvMemoryStream aDeflatedStream;
                        SvMemoryStream aInflatedStream;

                        // Read object into memory stream
                        sal_uInt64 nBufSize = 4096;
                        sal_Char aBuf[ nBufSize ];
                        sal_uInt64 nBytesRead;
                        sal_uInt64 nBytesLeft = rObj.m_nStreamLen;
                        while ( nBytesLeft && osl_readFile( aFile, aBuf, nBufSize, &nBytesRead ) == osl_File_E_None )
                        {
                            if ( nBytesLeft < nBytesRead )
                                nBytesRead = nBytesLeft;

                            if ( aDeflatedStream.Write( aBuf, nBytesRead ) != nBytesRead )
                                break;

                            nBytesLeft -= nBytesRead;

                            sal_Bool bEOF = sal_False;
                            if ( osl_isEndOfFile( aFile, &bEOF ) != osl_File_E_None || bEOF )
                                break;
                        }

                        sal_uInt64 nContentLen = aDeflatedStream.Tell();
                        if ( !nBytesLeft && nContentLen )
                        {
                            aDeflatedStream.Seek( 0 );
                            aInflater.BeginCompression();
                            while ( aDeflatedStream.Tell() < nContentLen && aInflater.Decompress( aDeflatedStream, aInflatedStream ) >= 0 )
                                ;
                            aInflater.EndCompression();
                        }

                        nContentLen = aInflatedStream.Tell();
                        if ( nContentLen )
                        {
                            aInflatedStream.Seek( 0 );
                            OString aPageContent( (sal_Char *)aInflatedStream.GetData(), nContentLen );

                            // Replace all whitespace with spaces for ease of
                            // parsing
                            for ( sal_Char *pPageContentBuf = (sal_Char *)aPageContent.getStr(); *pPageContentBuf; pPageContentBuf++ )
                            {
                                switch ( *pPageContentBuf )
                                {
                                    case 0x09:
                                    case 0x0A:
                                    case 0x0C:
                                    case 0x0D:
                                        *pPageContentBuf = 0x20;
                                }
                            }

                            OString aFontRunTag( " Tf [ " );
                            OString aFontTag( " Tf " );
                            OString aAltFontTag( " Tm " );
                            OString aTextRunTag( " ] TJ " );
                            OString aTextTag( " Tj " );
                            int nFontRunTagLen = aFontRunTag.getLength();
                            int nFontTagLen = aFontTag.getLength();
                            int nAltFontTagLen = aAltFontTag.getLength();
                            int nTextRunTagLen = aTextRunTag.getLength();
                            int nTextTagLen = aTextTag.getLength();
                            int nCurrentGlyph = 0;
                            sal_Int32 nCurrentPos = 0;
                            OString aCurrentFontID;
                            sal_Int32 nCurrentFontSubID = 0;
                            sal_Int32 nFontIDPos;
                            sal_Int32 nNextFontIDPos;
                            sal_Int32 nNextFontPos;
                            sal_Int32 nNextAltFontPos;
                            sal_Int32 nTextPos;
                            sal_Int32 nCurrentFontIDTagLen;
                            while ( nCurrentGlyph < nGlyphIDs )
                            {
                                if ( ( nNextFontIDPos = aPageContent.indexOf( aFontIDTagF, nCurrentPos ) ) >= 0 )
								{
                                    nCurrentFontIDTagLen = nFontIDTagLenF;
								}
                                else if ( ( nNextFontIDPos = aPageContent.indexOf( aFontIDTagG, nCurrentPos ) ) >= 0 )
								{
                                    nCurrentFontIDTagLen = nFontIDTagLenG;
								}
                                else if ( ( nNextFontIDPos = aPageContent.indexOf( aFontIDTagTc, nCurrentPos ) ) >= 0 )
								{
                                    nCurrentFontIDTagLen = nFontIDTagLenTc;
								}
                                else if ( ( nNextFontIDPos = aPageContent.indexOf( aFontIDTagTT, nCurrentPos ) ) >= 0 )
								{
                                    nCurrentFontIDTagLen = nFontIDTagLenTT;
								}
                                else if ( ( nNextFontIDPos = aPageContent.indexOf( aFontIDTagTy, nCurrentPos ) ) >= 0 )
								{
                                    nCurrentFontIDTagLen = nFontIDTagLenTy;
								}
                                else if ( ( nNextFontIDPos = aPageContent.indexOf( aFontIDTagC, nCurrentPos ) ) >= 0 )
								{
									while ( aPageContent.indexOf( aFontIDTagCs, nNextFontIDPos ) == nNextFontIDPos )
									{
										nNextFontIDPos += nFontIDTagLenCs;
                                		if ( ( nNextFontIDPos = aPageContent.indexOf( aFontIDTagC, nNextFontIDPos ) ) < 0 )
										{
                                    		nCurrentFontIDTagLen = 0;
											break;
										}
									}

                                   	nCurrentFontIDTagLen = nFontIDTagLenC;
								}
                                else
								{
                                    nCurrentFontIDTagLen = 0;
								}

                                bool bIsTextRun = false;
                                if ( aCurrentFontID.getLength() )
                                {
                                    // Fix bug 3570 by detecting when both the
                                    // font tag and the font run tag are used
                                    // in the same page content
                                    nNextFontPos = aPageContent.indexOf( aFontTag, nCurrentPos );
                                    sal_Int32 nNextFontRunPos = aPageContent.indexOf( aFontRunTag, nCurrentPos );
                                    if ( nNextFontRunPos >= 0 && nNextFontRunPos <= nNextFontPos )
                                    {
                                         nNextFontPos = nNextFontRunPos;
                                         bIsTextRun = true;
                                    }
                                    nNextAltFontPos = aPageContent.indexOf( aAltFontTag, nCurrentPos );
                                }
                                else
                                {
                                    nNextFontPos = -1;
                                    nNextAltFontPos = -1;
                                }

                                const sal_Char *pBuf = aPageContent.getStr();

                                if ( nNextFontIDPos >= 0 && ( nNextFontPos < 0 || nNextFontIDPos < nNextFontPos ) && ( nNextAltFontPos < 0 || nNextFontIDPos < nNextAltFontPos ) )
                                {
                                    nCurrentPos = nNextFontIDPos + nCurrentFontIDTagLen;
                                    if ( ( nFontIDPos = aPageContent.indexOf( ' ', nCurrentPos ) ) < 0 )
                                        continue;

                                    nCurrentFontSubID = 0;
                                    aCurrentFontID = OString( pBuf + nCurrentPos, nFontIDPos - nCurrentPos );
                                    std::map< OString, sal_Int32 >::iterator sit = rEmit.m_aFontSubIDMapping.find( aCurrentFontID );
                                    if ( sit != rEmit.m_aFontSubIDMapping.end() )
                                        nCurrentFontSubID = sit->second;

                                    continue;
                                }
                                else if ( nNextFontPos >= 0 && ( nNextAltFontPos < 0 || nNextFontPos < nNextAltFontPos ) )
                                {
                                    nCurrentPos = nNextFontPos + ( bIsTextRun ? nFontRunTagLen : nFontTagLen );
                                }
                                else if ( nNextAltFontPos >= 0 && ( nNextFontPos < 0 || nNextAltFontPos < nNextFontPos ) )
                                {
                                    nCurrentPos = nNextAltFontPos + nAltFontTagLen;
                                }
                                else
                                {
                                    break;
                                }

                                sal_Int32 nTextStart = nCurrentPos;

                                if ( bIsTextRun && ( nTextPos = aPageContent.indexOf( aTextRunTag, nCurrentPos ) ) >= 0 )
                                    nCurrentPos = nTextPos + nTextRunTagLen;
                                else if ( !bIsTextRun && ( nTextPos = aPageContent.indexOf( aTextTag, nCurrentPos ) ) >= 0 )
                                    nCurrentPos = nTextPos + nTextTagLen;
                                else
                                    break;

                                sal_Int32 nTextEnd = nTextPos - 1;

                                bool bTextIsHex;
                                if ( pBuf[ nTextStart ] == '<' )
                                    bTextIsHex = true;
                                else if ( pBuf[ nTextStart ] == '(' )
                                    bTextIsHex = false;
                                else
                                    continue;
                                nTextStart++;

                                sal_Int32 nTextLen = nTextEnd - nTextStart;
                                if ( nTextLen <= 0 )
                                    continue;

                                // Note: we assume that Mac OS X only generates
                                // hexadecimal strings when the font encoding
                                // is Identity-H or Identity-V
                                OStringBuffer aGlyphBuf;
                                pBuf = aPageContent.getStr() + nTextStart;
                                if ( bTextIsHex )
                                {
                                    for ( sal_Int32 j = 0; j < nTextLen; j += 2, pBuf += 2 )
                                        if ( bIsTextRun && *pBuf == '>' )
                                        {
                                            // Fix bug 3502 by skipping
                                            // characters not in a text run
                                            j++;
                                            pBuf++;
                                            for ( ; j < nTextLen && *pBuf != '<'; j++, pBuf++ )
                                                ;
                                            if ( j < nTextLen )
                                            {
                                                j--;
                                                pBuf--;
                                            }
                                        }
                                        else
                                        {
                                            aGlyphBuf.append( (sal_Char)OString( pBuf, nTextLen - j == 1 ? 1 : 2 ).toInt32( 16 ) );
                                        }
                                }
                                else
                                {
                                    OStringBuffer aOctalBuf;
                                    bool bLastSlash = false;
                                    for ( sal_Int32 j = 0; j < nTextLen; j++, pBuf++ )
                                    {
                                        sal_Int32 nOctalBufLen = aOctalBuf.getLength();
                                        // Fix bug 3481 by limiting octal
                                        // strings to only 3 digits
                                        if ( nOctalBufLen && ( nOctalBufLen > 2 || *pBuf < '0' || *pBuf > '9' ) )
                                        {
                                            aGlyphBuf.append( (sal_Char)aOctalBuf.makeStringAndClear().toInt32( 8 ) );
                                            nOctalBufLen = 0;
                                        }

                                        if ( !bLastSlash && *pBuf == '\\' )
                                        {
                                            bLastSlash = true;
                                        }
                                        else if ( *pBuf >= '0' && *pBuf <= '9' && ( bLastSlash || nOctalBufLen ) )
                                        {
                                            bLastSlash = false;
                                            aOctalBuf.append( *pBuf );
                                        }
                                        else if ( bLastSlash )
                                        {
                                            bLastSlash = false;
                                            switch ( *pBuf )
                                            {
                                                case 'n':
                                                    aGlyphBuf.append( (sal_Char)0x0a );
                                                    break;
                                                case 'r':
                                                    aGlyphBuf.append( (sal_Char)0x0d );
                                                    break;
                                                case 't':
                                                    aGlyphBuf.append( (sal_Char)0x09 );
                                                    break;
                                                case 'b':
                                                    aGlyphBuf.append( (sal_Char)0x09 );
                                                    break;
                                                case 'f':
                                                    aGlyphBuf.append( (sal_Char)0x0c );
                                                    break;
                                                default:
                                                    aGlyphBuf.append( *pBuf );
                                                    break;
                                            }
                                        }
                                        else if ( bIsTextRun && *pBuf == ')' )
                                        {
                                            // Fix bug 3502 by skipping
                                            // characters not in a text run
                                            j++;
                                            pBuf++;
                                            for ( ; j < nTextLen && *pBuf != '('; j++, pBuf++ )
                                                ;
                                        }
                                        else
                                        {
                                            aGlyphBuf.append( *pBuf );
                                        }
                                    }

                                    if ( aOctalBuf.getLength() )
                                        aGlyphBuf.append( (sal_Char)aOctalBuf.makeStringAndClear().toInt32( 8 ) );
                                }

                                pBuf = aGlyphBuf.getStr();
                                nTextLen = aGlyphBuf.getLength();
                                for ( sal_Int32 j = 0; j < nTextLen && nCurrentGlyph < nGlyphIDs; j++, nCurrentGlyph++, pBuf++ )
                                {
                                    long nGlyph = (long)aGlyphIDs[ nCurrentGlyph ];

                                    sal_uInt16 nEncodedGlyph;
                                    if ( bTextIsHex )
                                    {
                                        nEncodedGlyph = (sal_uInt8)( *pBuf ) << 8;
                                        if ( ++pBuf && ++j < nTextLen )
                                            nEncodedGlyph |= (sal_uInt8)( *pBuf );
                                    }
                                    else
                                    {
                                        nEncodedGlyph = (sal_uInt16)( *pBuf & 0x00ff );
                                    }

                                    // Cache encoding
                                    rEmit.m_aGlyphEncoding[ nGlyph ] = nEncodedGlyph;

                                    // Update glyph mappings
                                    rEmit.m_aMapping[ nGlyph ].m_nSubsetGlyphID = nEncodedGlyph;
                                    rSubset.m_aMapping[ nGlyph ].m_nFontSubID = nCurrentFontSubID;
                                    rSubset.m_aMapping[ nGlyph ].m_bIdentityGlyph = bTextIsHex;
                                    rSubset.m_aMapping[ nGlyph ].m_nSubsetGlyphID = nEncodedGlyph;
                                }
                            }
                        }
                    }
                }

                osl_closeFile( aFile );
            }
        }

        CFRelease( aFont );
    }
}

void PDFWriterImpl::addAction( MetaAction *pAction )
{
    if ( pAction && !isReplayWriter() )
        m_aReplayMtf.AddAction( pAction );
}

#endif	// USE_JAVA && MACOSX
