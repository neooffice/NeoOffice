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
 *         - GNU General Public License Version 2.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
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
 *  =================================================
 *  Modified September 2003 by Patrick Luby. SISSL Removed. NeoOffice is
 *  distributed under GPL only under modification term 3 of the LGPL.
 *
 *  Contributor(s): _______________________________________
 *
 ************************************************************************/

#ifndef REMOTE_APPSERVER

#define _SV_IMPPRN_CXX
#define _SPOOLPRINTER_EXT
#ifdef USE_JAVA
#define _SV_PRINT_CXX
class SalPrinterQueueInfo;
#endif

#ifndef _QUEUE_HXX
#include <tools/queue.hxx>
#endif

#ifndef _SV_SVAPP_HXX
#include <svapp.hxx>
#endif
#ifndef _SV_METAACT_HXX
#include <metaact.hxx>
#endif
#ifndef _SV_GDIMTF_HXX
#include <gdimtf.hxx>
#endif
#ifndef _SV_TIMER_HXX
#include <timer.hxx>
#endif
#ifndef _SV_IMPPRN_HXX
#include <impprn.hxx>
#endif

#ifdef USE_JAVA
#ifndef _SV_SALPRN_HXX
#include <salprn.hxx>
#endif
#endif	// USE_JAVA

// -----------
// - Defines -
// -----------

#define OPTIMAL_BMP_RESOLUTION  300
#define NORMAL_BMP_RESOLUTION   200

// =======================================================================

struct QueuePage
{
	GDIMetaFile*	mpMtf;
	JobSetup*		mpSetup;
	USHORT			mnPage;
	BOOL			mbEndJob;

					QueuePage() { mpMtf = NULL; mpSetup = NULL; }
					~QueuePage() { delete mpMtf; if ( mpSetup ) delete mpSetup; }
};

// =======================================================================

ImplQPrinter::ImplQPrinter( Printer* pParent ) :
	Printer( pParent->GetName() )
{
	SetSelfAsQueuePrinter( TRUE );
	SetPrinterProps( pParent );
	SetPageQueueSize( 0 );
	mpParent		= pParent;
	mnCopyCount 	= pParent->mnCopyCount;
	mbCollateCopy	= pParent->mbCollateCopy;
	mpQueue 		= new Queue( mpParent->GetPageQueueSize() );
	mbAborted		= FALSE;
	mbUserCopy		= FALSE;
	mbDestroyAllowed= TRUE;
	mbDestroyed		= FALSE;
}

// -----------------------------------------------------------------------

ImplQPrinter::~ImplQPrinter()
{
	QueuePage* pQueuePage = (QueuePage*)mpQueue->Get();
	while ( pQueuePage )
	{
		delete pQueuePage;
		pQueuePage = (QueuePage*)mpQueue->Get();
	}

	delete mpQueue;
}

// -----------------------------------------------------------------------------

void ImplQPrinter::Destroy()
{
	if( mbDestroyAllowed )
		delete this;
	else
		mbDestroyed = TRUE;
}

// -----------------------------------------------------------------------

void ImplQPrinter::ImplPrintMtf( GDIMetaFile& rMtf, long nMaxBmpDPIX, long nMaxBmpDPIY )
{
    const PrinterOptions& rPrinterOptions = GetPrinterOptions();

	for( MetaAction* pAct = rMtf.FirstAction(); pAct && !mbAborted; pAct = rMtf.NextAction() )
	{
		const ULONG		nType = pAct->GetType();
		sal_Bool		bExecuted = sal_False;

		if( nType == META_COMMENT_ACTION )
		{
			// search for special comments ( ..._BEGIN/..._END )
			MetaCommentAction* pComment = (MetaCommentAction*) pAct;

			if( pComment->GetComment().CompareIgnoreCaseToAscii( "XGRAD_SEQ_BEGIN" ) == COMPARE_EQUAL )
			{	
				pAct = rMtf.NextAction();

				// if next action is a GradientEx action, execute this and 
				// skip actions until a XGRAD_SEQ_END comment is found
				if( pAct && ( pAct->GetType() == META_GRADIENTEX_ACTION ) )
				{
					MetaGradientExAction* pGradientExAction = (MetaGradientExAction*) pAct;
                    DrawGradientEx( this, pGradientExAction->GetPolyPolygon(), pGradientExAction->GetGradient() );

					// seek to end of this comment
					do
					{
						pAct = rMtf.NextAction();
					}
					while( pAct && 
						   ( ( pAct->GetType() != META_COMMENT_ACTION ) ||
							 ( ( (MetaCommentAction*) pAct )->GetComment().CompareIgnoreCaseToAscii( "XGRAD_SEQ_END" ) != COMPARE_EQUAL ) ) );

					bExecuted = sal_True;
				}
			}
            else if( pComment->GetComment().CompareIgnoreCaseToAscii( "PRNSPOOL_TRANSPARENTBITMAP_BEGIN" ) == COMPARE_EQUAL )
			{	
				pAct = rMtf.NextAction();

				if( pAct && ( pAct->GetType() == META_BMPSCALE_ACTION ) )
				{
                    MetaBmpScaleAction* pBmpScaleAction = (MetaBmpScaleAction*) pAct;

                    // execute action here to avoid DPI processing of bitmap;
                    pAct->Execute( this );

					// seek to end of this comment
					do
					{
						pAct = rMtf.NextAction();
					}
					while( pAct && 
						   ( ( pAct->GetType() != META_COMMENT_ACTION ) ||
							 ( ( (MetaCommentAction*) pAct )->GetComment().CompareIgnoreCaseToAscii( "PRNSPOOL_TRANSPARENTBITMAP_END" ) != COMPARE_EQUAL ) ) );

					bExecuted = sal_True;
                }
            }
		}
		else if( nType == META_GRADIENT_ACTION )
		{
            MetaGradientAction* pGradientAction = (MetaGradientAction*) pAct;
            DrawGradientEx( this, pGradientAction->GetRect(), pGradientAction->GetGradient() );
            bExecuted = sal_True;
		}
        else if( nType == META_BMPSCALE_ACTION )
		{
            MetaBmpScaleAction* pBmpScaleAction = (MetaBmpScaleAction*) pAct;
            const Bitmap&       rBmp = pBmpScaleAction->GetBitmap();

            DrawBitmap( pBmpScaleAction->GetPoint(), pBmpScaleAction->GetSize(),
                        GetPreparedBitmap( pBmpScaleAction->GetPoint(), pBmpScaleAction->GetSize(), 
                                           Point(), rBmp.GetSizePixel(), 
                                           rBmp, nMaxBmpDPIX, nMaxBmpDPIY ) );

            bExecuted = sal_True;
        }
        else if( nType == META_BMPSCALEPART_ACTION )
		{
            MetaBmpScalePartAction* pBmpScalePartAction = (MetaBmpScalePartAction*) pAct;
            const Bitmap&           rBmp = pBmpScalePartAction->GetBitmap();

            DrawBitmap( pBmpScalePartAction->GetDestPoint(), pBmpScalePartAction->GetDestSize(),
                        GetPreparedBitmap( pBmpScalePartAction->GetDestPoint(), pBmpScalePartAction->GetDestSize(),
                                           pBmpScalePartAction->GetSrcPoint(), pBmpScalePartAction->GetSrcSize(), 
                                           rBmp, nMaxBmpDPIX, nMaxBmpDPIY ) );

            bExecuted = sal_True;
        }
        else if( nType == META_BMPEXSCALE_ACTION )
		{
            MetaBmpExScaleAction*   pBmpExScaleAction = (MetaBmpExScaleAction*) pAct;
            const BitmapEx&         rBmpEx = pBmpExScaleAction->GetBitmapEx();

            DrawBitmapEx( pBmpExScaleAction->GetPoint(), pBmpExScaleAction->GetSize(),
                          GetPreparedBitmapEx( pBmpExScaleAction->GetPoint(), pBmpExScaleAction->GetSize(), 
                                               Point(), rBmpEx.GetSizePixel(), 
                                               rBmpEx, nMaxBmpDPIX, nMaxBmpDPIY ) );

            bExecuted = sal_True;
        }
        else if( nType == META_BMPEXSCALEPART_ACTION )
		{
            MetaBmpExScalePartAction*   pBmpExScalePartAction = (MetaBmpExScalePartAction*) pAct;
            const BitmapEx&             rBmpEx = pBmpExScalePartAction->GetBitmapEx();

            DrawBitmapEx( pBmpExScalePartAction->GetDestPoint(), pBmpExScalePartAction->GetDestSize(),
                          GetPreparedBitmapEx( pBmpExScalePartAction->GetDestPoint(), pBmpExScalePartAction->GetDestSize(),
                                               pBmpExScalePartAction->GetSrcPoint(), pBmpExScalePartAction->GetSrcSize(), 
                                               rBmpEx, nMaxBmpDPIX, nMaxBmpDPIY ) );

            bExecuted = sal_True;
        }
        else if( nType == META_TRANSPARENT_ACTION )
		{
			DrawPolyPolygon( ( (MetaTransparentAction*) pAct )->GetPolyPolygon() );
			bExecuted = sal_True;
		}
		else if( nType == META_FLOATTRANSPARENT_ACTION )
		{
			MetaFloatTransparentAction*	pFloatAction = (MetaFloatTransparentAction*) pAct;
			GDIMetaFile&				rMtf = (GDIMetaFile&) pFloatAction->GetGDIMetaFile();
			MapMode						aDrawMap( rMtf.GetPrefMapMode() );
			Point						aDestPoint( LogicToPixel( pFloatAction->GetPoint() ) ); 
			Size						aDestSize( LogicToPixel( pFloatAction->GetSize() ) );

			if( aDestSize.Width() && aDestSize.Height() )
			{
				Size aTmpPrefSize( LogicToPixel( rMtf.GetPrefSize(), aDrawMap ) );

				if( !aTmpPrefSize.Width() )
					aTmpPrefSize.Width() = aDestSize.Width();

				if( !aTmpPrefSize.Height() )
					aTmpPrefSize.Height() = aDestSize.Height();

				Fraction aScaleX( aDestSize.Width(), aTmpPrefSize.Width() );
				Fraction aScaleY( aDestSize.Height(), aTmpPrefSize.Height() );
				
				aDrawMap.SetScaleX( aScaleX *= aDrawMap.GetScaleX() );
				aDrawMap.SetScaleY( aScaleY *= aDrawMap.GetScaleY() );
				aDrawMap.SetOrigin( PixelToLogic( aDestPoint, aDrawMap ) );

				Push();
				SetMapMode( aDrawMap );
				ImplPrintMtf( rMtf, nMaxBmpDPIX, nMaxBmpDPIY );
				Pop();
			}

			bExecuted = sal_True;
		}

		if( !bExecuted && pAct )
			pAct->Execute( this );

		Application::Reschedule();
	}
}

// -----------------------------------------------------------------------

IMPL_LINK( ImplQPrinter, ImplPrintHdl, Timer*, EMPTYARG )
{
	// Ist Drucken abgebrochen wurden?
	if( !IsPrinting() || ( mpParent->IsJobActive() && ( mpQueue->Count() < (ULONG)mpParent->GetPageQueueSize() ) ) )
		return 0;

	// Druck-Job zuende?
	QueuePage* pActPage = (QueuePage*) mpQueue->Get();
	
	if ( pActPage->mbEndJob )
	{
		maTimer.Stop();
		delete pActPage;
		EndJob();
		mpParent->ImplEndPrint();
	}
	else
	{
		GDIMetaFile		        aMtf;
        const PrinterOptions&   rPrinterOptions = GetPrinterOptions();
        const ULONG             nOldDrawMode = GetDrawMode();
        long                    nMaxBmpDPIX = mnDPIX;
        long                    nMaxBmpDPIY = mnDPIY;
		USHORT			        nCopyCount = 1;

#ifdef USE_JAVA
		// The Java implementation requires that we push the resolution to the
		// printer instead of vice versa so we need to set resolution to the
		// highest resolution bitmap
		long nDPIX = 0;
		long nDPIY = 0;
		MetaAction *pAct;

        for ( pAct = pActPage->mpMtf->FirstAction(); pAct; pAct = pActPage->mpMtf->NextAction() )
		{
			Size aSrcSize;
			Size aDestSize;

			switch ( pAct->GetType() )
			{
				case ( META_BMP_ACTION ):
				case ( META_BMPEX_ACTION ):
				case ( META_MASK_ACTION ):
				{
					// If there is a non-scaling action, don't change the
					// resolution
					nDPIX = 0;
					nDPIY = 0;
					aDestSize = Size( 0, 0 );
					while ( pAct )
						pActPage->mpMtf->NextAction();
					break;
				}
				case ( META_BMPSCALE_ACTION ):
				{
					MetaBmpScaleAction *pA = (MetaBmpScaleAction*)pAct;
					aSrcSize =  pA->GetBitmap().GetSizePixel();
					aDestSize = pA->GetSize();
					break;
				}
				case ( META_BMPSCALEPART_ACTION ):
				{
					MetaBmpScalePartAction* pA = (MetaBmpScalePartAction*)pAct;
					aSrcSize = pA->GetSrcSize();
					aDestSize = pA->GetDestSize();
					break;
				}
				case ( META_BMPEXSCALE_ACTION ):
				{
					MetaBmpExScaleAction *pA = (MetaBmpExScaleAction*)pAct;
					aSrcSize = pA->GetBitmapEx().GetSizePixel();
					aDestSize = pA->GetSize();
					break;
				}
				case ( META_BMPEXSCALEPART_ACTION ):
				{
					MetaBmpExScalePartAction* pA = (MetaBmpExScalePartAction*)pAct;
					aSrcSize = pA->GetSrcSize();
					aDestSize = pA->GetDestSize();
					break;
				}
				case ( META_MASKSCALE_ACTION ):
				{
					MetaMaskScaleAction* pA = (MetaMaskScaleAction*)pAct;
					aSrcSize = pA->GetBitmap().GetSizePixel();
					aDestSize = pA->GetSize();
					break;
				}
				case ( META_MASKSCALEPART_ACTION ):
				{
					MetaMaskScalePartAction* pA = (MetaMaskScalePartAction*)pAct;
					aSrcSize = pA->GetSrcSize();
					aDestSize = pA->GetDestSize();
					break;
				}
				default:
					break;
			}
			if ( aDestSize.Width() > 0 && aDestSize.Height() > 0 )
			{
				long nBmpDPIX = ( aSrcSize.Width() * 2540 ) / aDestSize.Width();
				long nBmpDPIY = ( aSrcSize.Height() * 2540 ) / aDestSize.Height();
				long nBmpDPI = ( nBmpDPIY > nBmpDPIX ? nBmpDPIY : nBmpDPIX );
				if ( nDPIX < nBmpDPI )
					nDPIX = nBmpDPI;
				if ( nDPIY < nBmpDPI )
					nDPIY = nBmpDPI;
			}
		}

		// Update the resolution
		if ( nDPIX && nDPIX < nMaxBmpDPIX )
			nMaxBmpDPIX = nDPIX;
		if ( nDPIY && nDPIY < nMaxBmpDPIY )
			nMaxBmpDPIY = nDPIY;
#endif	// USE_JAVA

        if( rPrinterOptions.IsReduceBitmaps() )
        {
            // calculate maximum resolution for bitmap graphics
            if( PRINTER_BITMAP_OPTIMAL == rPrinterOptions.GetReducedBitmapMode() )
            {
                nMaxBmpDPIX = Min( (long) OPTIMAL_BMP_RESOLUTION, nMaxBmpDPIX );
                nMaxBmpDPIY = Min( (long) OPTIMAL_BMP_RESOLUTION, nMaxBmpDPIY );
            }
            else if( PRINTER_BITMAP_NORMAL == rPrinterOptions.GetReducedBitmapMode() )
            {
                nMaxBmpDPIX = Min( (long) NORMAL_BMP_RESOLUTION, nMaxBmpDPIX );
                nMaxBmpDPIY = Min( (long) NORMAL_BMP_RESOLUTION, nMaxBmpDPIY );
            }
            else
            {
                nMaxBmpDPIX = Min( (long) rPrinterOptions.GetReducedBitmapResolution(), nMaxBmpDPIX );
                nMaxBmpDPIY = Min( (long) rPrinterOptions.GetReducedBitmapResolution(), nMaxBmpDPIY );
            }
        }

        // convert to greysacles
        if( rPrinterOptions.IsConvertToGreyscales() )
        {
            SetDrawMode( GetDrawMode() | ( DRAWMODE_GRAYLINE | DRAWMODE_GRAYFILL | DRAWMODE_GRAYTEXT | 
                                           DRAWMODE_GRAYBITMAP | DRAWMODE_GRAYGRADIENT ) );
        }

        // disable transparency output
		if( rPrinterOptions.IsReduceTransparency() && ( PRINTER_TRANSPARENCY_NONE == rPrinterOptions.GetReducedTransparencyMode() ) )
		{
			SetDrawMode( GetDrawMode() | DRAWMODE_NOTRANSPARENCY );
		}

#ifdef USE_JAVA
		long nOldDPIX = mnDPIX;
		long nOldDPIY = mnDPIY;
		if ( nMaxBmpDPIX && nMaxBmpDPIY && nMaxBmpDPIX < mnDPIX && nMaxBmpDPIY < mnDPIY )
		{
			mpPrinter->SetResolution( nMaxBmpDPIX, nMaxBmpDPIY );
			ImplUpdatePageData();
			ImplUpdateFontList();
		}
#endif	// USE_JAVA

		mbDestroyAllowed = FALSE;
	    GetPreparedMetaFile( *pActPage->mpMtf, aMtf, nMaxBmpDPIX, nMaxBmpDPIY );
		
		if( mbUserCopy && !mbCollateCopy )
			nCopyCount = mnCopyCount;

#if defined USE_JAVA && defined MACOSX
		// Java on Mac OS X expects each page to be printed twice
		nCopyCount *= 2;
#endif	// USE_JAVA && MACOSX

		for ( USHORT i = 0; i < nCopyCount; i++ )
		{
			ULONG nActionPos = 0UL;

			if ( pActPage->mpSetup )
			{
				SetJobSetup( *pActPage->mpSetup );
				if ( mbAborted )
					 break;
			}

			StartPage();
			
			if ( mbAborted )
				break;

			if ( mpJobGraphics )
				ImplPrintMtf( aMtf, nMaxBmpDPIX, nMaxBmpDPIY );

			if( !mbAborted )
				EndPage();
			else
				break;

#ifdef USE_JAVA
			// If the native print job ended or aborted, abort the parent job
			if ( mnError == PRINTER_ABORT )
			{
				mpParent->AbortJob();
				AbortJob();
			}
#endif	// USE_JAVA
		}

#ifdef USE_JAVA
		if ( !mbAborted )
		{
			mpPrinter->SetResolution( nOldDPIX, nOldDPIY );
			ImplUpdatePageData();
			ImplUpdateFontList();
		}
#endif	// USE_JAVA

        SetDrawMode( nOldDrawMode );

		delete pActPage;
		mbDestroyAllowed = TRUE;

		if( mbDestroyed )
			Destroy();
	}

	return 0;
}

// -----------------------------------------------------------------------

void ImplQPrinter::StartQueuePrint()
{
	maTimer.SetTimeout( 50 );
	maTimer.SetTimeoutHdl( LINK( this, ImplQPrinter, ImplPrintHdl ) );
	maTimer.Start();
}

// -----------------------------------------------------------------------

void ImplQPrinter::EndQueuePrint()
{
	QueuePage* pQueuePage	= new QueuePage;
	pQueuePage->mbEndJob	= TRUE;
	mpQueue->Put( pQueuePage );
}

// -----------------------------------------------------------------------

void ImplQPrinter::AbortQueuePrint()
{
	maTimer.Stop();
	mbAborted = TRUE;
	AbortJob();
}

// -----------------------------------------------------------------------

void ImplQPrinter::AddQueuePage( GDIMetaFile* pPage, USHORT nPage, BOOL bNewJobSetup )
{
	QueuePage* pQueuePage	= new QueuePage;
	pQueuePage->mpMtf		= pPage;
	pQueuePage->mnPage		= nPage;
	pQueuePage->mbEndJob	= FALSE;
	if ( bNewJobSetup )
		pQueuePage->mpSetup = new JobSetup( mpParent->GetJobSetup() );
	mpQueue->Put( pQueuePage );
}

#endif
