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
 *  Patrick Luby, June 2003
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2003 by Patrick Luby (patrick.luby@planamesa.com)
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

#define _SV_SALGDI_CXX

#ifndef _SV_SALGDI_HXX
#include <salgdi.hxx>
#endif
#ifndef _SV_SALFRAME_HXX
#include <salframe.hxx>
#endif
#ifndef _SV_SALINST_HXX
#include <salinst.hxx>
#endif
#ifndef _SV_COM_SUN_STAR_VCL_VCLGRAPHICS_HXX
#include <com/sun/star/vcl/VCLGraphics.hxx>
#endif
#ifndef _SV_COM_SUN_STAR_VCL_VCLFONT_HXX
#include <com/sun/star/vcl/VCLFont.hxx>
#endif
#ifndef _SV_COM_SUN_STAR_VCL_VCLPRINTJOB_HXX
#include <com/sun/star/vcl/VCLPrintJob.hxx>
#endif

#ifndef _SV_SALPRN_HXX
#include <salprn.hxx>
#endif
#ifndef _SV_JAVA_LANG_CLASS_HXX
#include <java/lang/Class.hxx>
#endif
#ifndef _OSL_CONDITN_HXX_
#include <osl/conditn.hxx>
#endif
#ifndef _RTL_STRBUF_HXX_
#include <rtl/strbuf.hxx>
#endif
#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif

#include <premac.h>
#include <Carbon/Carbon.h>
#include <postmac.h>

struct EPSData
{
	PMPrintSession		maSession;
	long				mnX;
	long				mnY;
	long				mnWidth;
	long				mnHeight;
	MacOSPtr			mpPtr;
	ULONG				mnSize;
	::osl::Condition	maCondition;
	BOOL				mbSuccess;
};

static EventLoopTimerUPP pEventLoopTimerUPP = NULL;

using namespace rtl;

static double fExpValues[] = { 1.0, 1e1, 1e2, 1e3, 1e4, 1e5, 1e6, 1e7, 1e8, 1e9, 1e10, 1e11, 1e12, 1e13, 1e14, 1e15, 1e16, 1e17, 1e18, 1e19, 1e20 };

using namespace vcl;

// =======================================================================

inline int isSpace( char cChar )
{
	return cChar == ' ' || cChar == '\t' || cChar == '\r' || cChar == '\n' || cChar == 0x0c || cChar == 0x0b;
}

// -----------------------------------------------------------------------

inline int isProtect( char cChar )
{
	return cChar == '`' || cChar == '\'' || cChar == '"';
}

// -----------------------------------------------------------------------

inline void copyUntil( char*& pTo, const char*& pFrom, char cUntil, int bIncludeUntil = 0 )
{
	do
	{
		if ( *pFrom == '\\' )
		{
			pFrom++;
			if ( *pFrom )
			{
				*pTo = *pFrom;
				pTo++;
			}
		}
		else if ( bIncludeUntil || ! isProtect( *pFrom ) )
		{
			*pTo = *pFrom;
			pTo++;
		}
		pFrom++;
	} while ( *pFrom && *pFrom != cUntil );
	// copy the terminating character unless zero or protector
	if ( ! isProtect( *pFrom ) || bIncludeUntil )
	{
		*pTo = *pFrom;
		if ( *pTo )
			pTo++;
	}
	if ( *pFrom )
		pFrom++;
}

// -----------------------------------------------------------------------

static ByteString GetCommandLineToken( int nToken, const ByteString& rLine )
{
	int nLen = rLine.Len();
	if ( ! nLen )
		return ByteString();

	int nActualToken = 0;
	char* pBuffer = (char*)alloca( nLen + 1 );
	const char* pRun = rLine.GetBuffer();
	char* pLeap;

	while ( *pRun && nActualToken <= nToken )
	{
		while ( *pRun && isSpace( *pRun ) )
			pRun++;
		pLeap = pBuffer;
		while ( *pRun && ! isSpace( *pRun ) )
		{
			if ( *pRun == '\\' )
			{
				// escapement
				pRun++;
				*pLeap = *pRun;
				pLeap++;
				if ( *pRun )
					pRun++;
			}
			else if ( *pRun == '`' )
				copyUntil( pLeap, pRun, '`' );
			else if ( *pRun == '\'' )
				copyUntil( pLeap, pRun, '\'' );
			else if ( *pRun == '"' )
				copyUntil( pLeap, pRun, '"' );
			else
			{
				*pLeap = *pRun;
				pLeap++;
				pRun++;
			}
		}
		if ( nActualToken != nToken )
			pBuffer[0] = 0;
		nActualToken++;
	}

	*pLeap = 0;

	ByteString aRet( pBuffer );
	return aRet;
}

// -----------------------------------------------------------------------

static ByteString WhitespaceToSpace( const ByteString& rLine )
{
	int nLen = rLine.Len();
	if ( ! nLen )
		return ByteString();

	char *pBuffer = (char*)alloca( nLen + 1 );
	const char *pRun = rLine.GetBuffer();
	char *pLeap = pBuffer;

	while ( *pRun )
	{
		if ( *pRun && isSpace( *pRun ) )
		{
			*pLeap = ' ';
			pLeap++;
			pRun++;
		}
		while ( *pRun && isSpace( *pRun ) )
			pRun++;
		while ( *pRun && ! isSpace( *pRun ) )
		{
			if ( *pRun == '\\' )
			{
				// escapement
				pRun++;
				*pLeap = *pRun;
				pLeap++;
				if ( *pRun )
					pRun++;
			}
			else if ( *pRun == '`' )
				copyUntil( pLeap, pRun, '`', TRUE );
			else if ( *pRun == '\'' )
				copyUntil( pLeap, pRun, '\'', TRUE );
			else if ( *pRun == '"' )
				copyUntil( pLeap, pRun, '"', TRUE );
			else
			{
				*pLeap = *pRun;
				*pLeap++;
				*pRun++;
			}
		}
	}

	*pLeap = 0;

	// there might be a space at beginning or end
	pLeap--;
	if ( *pLeap == ' ' )
		*pLeap = 0;

	ByteString aRet( *pBuffer == ' ' ? pBuffer+1 : pBuffer );
	return aRet;
}

// -----------------------------------------------------------------------

static double calcPow10( int nExp )
{
	BOOL bNeg = nExp < 0;
	nExp = bNeg ? -nExp : nExp;
	double fRet = 1.0;
	while ( nExp >= 20 )
	{
		fRet *= fExpValues[20];
		nExp -= 20;
	}
	fRet *= fExpValues[ nExp ];
	return bNeg ? 1.0/fRet : fRet;
}

// -----------------------------------------------------------------------

static double StringToDouble( const ByteString& rStr )
{
	const char* pStr	= rStr.GetBuffer();
	const char* pOrg	= pStr;
	const int nLen	  = rStr.Len();

	BOOL bExp = FALSE, bNeg = FALSE, bNegExp = FALSE, bDecimal=FALSE;
	int nExp = 0;
	double fRet = 0.0, fDiv = 0.1;

	while ( isSpace( *pStr ) )
		pStr++;
	for( ; pStr - pOrg < nLen; pStr++ )
	{
		if ( *pStr >= '0' && *pStr <= '9' )
		{
			if ( bExp )
				nExp = nExp*10 + ( *pStr - '0' );
			else if ( ! bDecimal )
				fRet = 10.0 * fRet + (double)( *pStr - '0' );
			else
			{
				fRet += fDiv * (double)( *pStr - '0' );
				fDiv /= 10.0;
			}
		}
		else if ( *pStr == '.' )
		{
			if ( bExp || bDecimal )
				break;
			bDecimal = TRUE;
		}
		else if ( *pStr == '-' )
		{
			if ( bExp )
			{
				if ( nExp != 0 )
					break;
				bNegExp = ! bNegExp;
			}
			else
			{
				if ( fRet != 0.0 )
					break;
				bNeg = ! bNeg;
			}
		}
		else if ( *pStr == '+' )
		{
			if ( bExp && nExp != 0 )
				break;
			if ( fRet != 0.0 )
				break;
		}
		else if ( *pStr == 'e' || *pStr == 'E' )
		{
			if ( bExp )
				break;
			bExp = TRUE;
		}
		else
			break;
	}
	if ( bExp )
	{
		nExp = bNegExp ? -nExp : nExp;
		fRet *= calcPow10( nExp );
	}
	return bNeg ? -fRet : fRet;
}

// -----------------------------------------------------------------------

static void ImplDrawEPS( EPSData *pData )
{
	CFArrayRef aFormats;
	if ( PMSessionGetDocumentFormatGeneration( pData->maSession, &aFormats ) == kPMNoError && aFormats )
	{
		// Check if the PICT + PS is a supported format
		bool bPS = false;
		CFIndex nCount = CFArrayGetCount( aFormats );
		for ( CFIndex i = 0; i < nCount; i++ )
		{
			CFStringRef aFormat = (CFStringRef)CFArrayGetValueAtIndex( aFormats, i );
			if ( CFStringCompare( aFormat, kPMDocumentFormatPICTPS, 0 ) == kCFCompareEqualTo )
			{
				bPS = true;
				break;
			}
		}

		CFRelease( aFormats );

		if ( bPS )
		{
			CFStringRef aContexts[ 1 ];
			aContexts[ 0 ] = kPMGraphicsContextCoreGraphics;
			CFArrayRef aContextArray = CFArrayCreate( kCFAllocatorDefault, (const void **)aContexts, 1, &kCFTypeArrayCallBacks );
			if ( aContextArray )
			{
				if ( PMSessionSetDocumentFormatGeneration( pData->maSession, kPMDocumentFormatPICTPS, aContextArray, NULL ) == kPMNoError )
				{
					OSStatus nErr = PMSessionPostScriptBegin( pData->maSession );
#ifdef DEBUG
					fprintf( stderr, "Print EPS Start: %i\n", nErr );
#endif	// DEBUG
					if ( nErr == kPMNoError )
					{
						// First find the bounding box in the EPS data
						SvMemoryStream aStream( pData->mpPtr, pData->mnSize, STREAM_READ );
						aStream.Seek( STREAM_SEEK_TO_BEGIN );
						ByteString aLine;
						ByteString aDocTitle;
						double fLeft = 0;
						double fRight = 0;
						double fTop = 0;
						double fBottom = 0;
						bool bEndComments = false;
						while ( ! aStream.IsEof() && ( ( !fLeft && !fRight && !fTop && !fBottom ) || ( !aDocTitle.Len() && !bEndComments ) ) )
						{
							aStream.ReadLine( aLine );
							if ( aLine.Len() > 1 && aLine.GetChar( 0 ) == '%' )
							{
								char cChar = aLine.GetChar( 1 );
								if ( cChar == '%' )
								{
									if ( aLine.CompareIgnoreCaseToAscii( "%%BoundingBox:", 14 ) == COMPARE_EQUAL )
									{
										aLine = WhitespaceToSpace( aLine.GetToken( 1, ':' ) );
										if ( aLine.Len() && aLine.Search( "atend" ) == STRING_NOTFOUND )
										{
											fLeft = StringToDouble( GetCommandLineToken( 0, aLine ) );
											fBottom = StringToDouble( GetCommandLineToken( 1, aLine ) );
											fRight = StringToDouble( GetCommandLineToken( 2, aLine ) );
											fTop = StringToDouble( GetCommandLineToken( 3, aLine ) );
										}
									}
									else if ( aLine.CompareIgnoreCaseToAscii( "%%Title:", 8 ) == COMPARE_EQUAL )
									{
										aDocTitle = WhitespaceToSpace( aLine.Copy( 8 ) );
									}
									else if ( aLine.CompareIgnoreCaseToAscii( "%%EndComments", 13 ) == COMPARE_EQUAL )
									{
										bEndComments = true;
									}
								}
								else if ( cChar == ' ' || cChar == '\t' || cChar == '\r' || cChar == '\n' )
								{
									bEndComments = true;
								}
							}
							else
							{
								bEndComments = true;
							}
						}

						static sal_uInt16 nEps = 0;
						if ( ! aDocTitle.Len() )
							aDocTitle = ByteString::CreateFromInt32( (sal_Int32)nEps++ );

						if ( fLeft != fRight && fTop != fBottom )
						{
							double fScaleX = (double)pData->mnWidth / ( fRight - fLeft );
							double fScaleY = (double)pData->mnHeight / ( fTop - fBottom ) * -1;
							Point aTranslatePoint( pData->mnX - (long)( fLeft * fScaleX ), pData->mnY + pData->mnHeight - (long)( fBottom * fScaleY ) );

							// Write EPS header
							OStringBuffer aBuf( "/b4_Inc_state save def\n"
								"/dict_count countdictstack def\n"
								"/op_count count 1 sub def\n"
								"userdict begin\n"
								"/showpage {} def\n"
								"0 setgray 0 setlinecap 1 setlinewidth 0 setlinejoin\n"
								"10 setmiterlimit [] 0 setdash newpath\n"
								"/languagelevel where\n"
								"{\n"
								"  pop languagelevel\n"
								"  1 ne\n"
								"  {\n"
								"    false setstrokeadjust false setoverprint\n"
								"  } if\n"
								"} if\n" );
							// Set clip to bounding box
							aBuf.append( "grestore\n"
								"gsave\n"
								"readpath\n" );
							aBuf.append( pData->mnX );
							aBuf.append( " " );
							aBuf.append( pData->mnY );
							aBuf.append( " moveto\n" );
							aBuf.append( pData->mnX + pData->mnWidth );
							aBuf.append( " " );
							aBuf.append( pData->mnY );
							aBuf.append( " lineto\n" );
							aBuf.append( pData->mnX + pData->mnWidth );
							aBuf.append( " " );
							aBuf.append( pData->mnY + pData->mnHeight );
							aBuf.append( " lineto\n" );
							aBuf.append( pData->mnX );
							aBuf.append( " " );
							aBuf.append( pData->mnY + pData->mnHeight );
							aBuf.append( " lineto\n" );
							aBuf.append( "closepath clip newpath\n" );
							// Set translation
							aBuf.append( aTranslatePoint.X() );
							aBuf.append( " " );
							aBuf.append( aTranslatePoint.Y() );
							aBuf.append( " translate\n" );
							// Set scale
							aBuf.append( fScaleX );
							aBuf.append( " " );
							aBuf.append( fScaleY );
							aBuf.append( " scale\n" );
							aBuf.append( "%%BeginDocument: " );
							aBuf.append( aDocTitle );
							aBuf.append( "\n" );
							nErr = PMSessionPostScriptData( pData->maSession, (MacOSPtr)aBuf.getStr(), aBuf.getLength() );
#ifdef DEBUG
							fprintf( stderr, "Print EPS Header: %i\n%s\n", nErr, aBuf.getStr() );
#endif	// DEBUG

							// Write EPS data
							nErr = PMSessionPostScriptData( pData->maSession, pData->mpPtr, pData->mnSize );
#ifdef DEBUG
							fprintf( stderr, "Print EPS Data: %i\n", nErr );
#endif	// DEBUG
							if ( ((char*)pData->mpPtr)[ pData->mnSize - 1 ] != '\n' )
							{
								aBuf = OStringBuffer( "\n" );
								nErr = PMSessionPostScriptData( pData->maSession, (MacOSPtr)aBuf.getStr(), aBuf.getLength() );
#ifdef DEBUG
								fprintf( stderr, "Print EPS Data End: %i\n", nErr );
#endif	// DEBUG
							}

							// Write EPS footer
							aBuf = OStringBuffer( "%%EndDocument\n"
								"count op_count sub {pop} repeat\n"
								"countdictstack dict_count sub {end} repeat\n"
								"b4_Inc_state restore\n" );
							nErr = PMSessionPostScriptData( pData->maSession, (MacOSPtr)aBuf.getStr(), aBuf.getLength() );
#ifdef DEBUG
							fprintf( stderr, "Print EPS Footer: %i\n%s\n", nErr, aBuf.getStr() );
#endif	// DEBUG
						}

						nErr = PMSessionPostScriptEnd( pData->maSession );
#ifdef DEBUG
						fprintf( stderr, "Print EPS End: %i\n", nErr );
#endif	// DEBUG

						pData->mbSuccess = TRUE;
					}

					PMSessionSetDocumentFormatGeneration( pData->maSession, kPMDocumentFormatPDF, aContextArray, NULL );
				}

				CFRelease( aContextArray );
			}
		}
	}
}

// -----------------------------------------------------------------------

static void DrawEPSTimerCallback( EventLoopTimerRef aTimer, void *pData )
{
	EPSData *pEPSData = (EPSData *)pData;

	// Block the VCL event loop
	Application::GetSolarMutex().acquire();

	ImplDrawEPS( pEPSData );

	pEPSData->maCondition.set();

	// Unblock the VCL event loop
	Application::GetSolarMutex().release();
}

// =======================================================================

SalGraphics::SalGraphics()
{
}

// -----------------------------------------------------------------------

SalGraphics::~SalGraphics()
{
}

// -----------------------------------------------------------------------

void SalGraphics::GetResolution( long& rDPIX, long& rDPIY )
{
	Size aSize( maGraphicsData.mpVCLGraphics->getResolution() );
	rDPIX = aSize.Width();
	rDPIY = aSize.Height();
}

// -----------------------------------------------------------------------

void SalGraphics::GetScreenFontResolution( long& rDPIX, long& rDPIY )
{
	Size aSize( maGraphicsData.mpVCLGraphics->getScreenFontResolution() );
	rDPIX = aSize.Width();
	rDPIY = aSize.Height();
}

// -----------------------------------------------------------------------

USHORT SalGraphics::GetBitCount()
{
	return maGraphicsData.mpVCLGraphics->getBitCount();
}

// -----------------------------------------------------------------------

void SalGraphics::ResetClipRegion()
{
	maGraphicsData.mpVCLGraphics->resetClipRegion();
}

// -----------------------------------------------------------------------

void SalGraphics::BeginSetClipRegion( ULONG nRectCount )
{
	maGraphicsData.mpVCLGraphics->beginSetClipRegion();
}

// -----------------------------------------------------------------------

BOOL SalGraphics::UnionClipRegion( long nX, long nY, long nWidth, long nHeight,
                                   const OutputDevice *pOutDev )
{
	maGraphicsData.mpVCLGraphics->unionClipRegion( nX, nY, nWidth, nHeight );
	return TRUE;
}

// -----------------------------------------------------------------------

void SalGraphics::EndSetClipRegion()
{
	maGraphicsData.mpVCLGraphics->endSetClipRegion();
}

// -----------------------------------------------------------------------

void SalGraphics::SetLineColor()
{
	maGraphicsData.mnLineColor = 0xffffffff;
}

// -----------------------------------------------------------------------

void SalGraphics::SetLineColor( SalColor nSalColor )
{
	maGraphicsData.mnLineColor = nSalColor;
}

// -----------------------------------------------------------------------

void SalGraphics::SetFillColor()
{
	maGraphicsData.mnFillColor = 0xffffffff;
}

// -----------------------------------------------------------------------

void SalGraphics::SetFillColor( SalColor nSalColor )
{
	maGraphicsData.mnFillColor = nSalColor;
}

// -----------------------------------------------------------------------

void SalGraphics::SetXORMode( BOOL bSet )
{
	maGraphicsData.mpVCLGraphics->setXORMode( bSet );
}

// -----------------------------------------------------------------------

void SalGraphics::SetROPLineColor( SalROPColor nROPColor )
{
	if ( nROPColor == SAL_ROP_0 )
		SetLineColor( MAKE_SALCOLOR( 0, 0, 0 ) );
	else
		SetLineColor( MAKE_SALCOLOR( 0xff, 0xff, 0xff ) );
}

// -----------------------------------------------------------------------

void SalGraphics::SetROPFillColor( SalROPColor nROPColor )
{
	if ( nROPColor == SAL_ROP_0 )
		SetFillColor( MAKE_SALCOLOR( 0, 0, 0 ) );
	else
		SetFillColor( MAKE_SALCOLOR( 0xff, 0xff, 0xff ) );
}

// -----------------------------------------------------------------------

void SalGraphics::DrawPixel( long nX, long nY, const OutputDevice *pOutDev )
{
	if ( maGraphicsData.mnLineColor != 0xffffffff )
		maGraphicsData.mpVCLGraphics->setPixel( nX, nY, maGraphicsData.mnLineColor );
}

// -----------------------------------------------------------------------

void SalGraphics::DrawPixel( long nX, long nY, SalColor nSalColor,
                             const OutputDevice *pOutDev )
{
	maGraphicsData.mpVCLGraphics->setPixel( nX, nY, nSalColor );
}

// -----------------------------------------------------------------------

void SalGraphics::DrawLine( long nX1, long nY1, long nX2, long nY2,
                             const OutputDevice *pOutDev )
{
	if ( maGraphicsData.mnLineColor != 0xffffffff )
		maGraphicsData.mpVCLGraphics->drawLine( nX1, nY1, nX2, nY2, maGraphicsData.mnLineColor );
}

// -----------------------------------------------------------------------

void SalGraphics::DrawRect( long nX, long nY, long nWidth, long nHeight,
                            const OutputDevice *pOutDev )
{
	if ( maGraphicsData.mnFillColor != 0xffffffff )
		maGraphicsData.mpVCLGraphics->drawRect( nX, nY, nWidth, nHeight, maGraphicsData.mnFillColor, TRUE );
	if ( maGraphicsData.mnLineColor != 0xffffffff )
		maGraphicsData.mpVCLGraphics->drawRect( nX, nY, nWidth, nHeight, maGraphicsData.mnLineColor, FALSE );
}

// -----------------------------------------------------------------------

void SalGraphics::DrawPolyLine( ULONG nPoints, const SalPoint* pPtAry,
                                const OutputDevice *pOutDev )
{
	long pXPoints[ nPoints ];
	long pYPoints[ nPoints ];
	for ( ULONG i = 0; i < nPoints; i++ )
	{
		pXPoints[ i ] = pPtAry->mnX;
		pYPoints[ i ] = pPtAry->mnY;
		pPtAry++;
	}

	if ( maGraphicsData.mnLineColor != 0xffffffff )
		maGraphicsData.mpVCLGraphics->drawPolyline( nPoints, pXPoints, pYPoints, maGraphicsData.mnLineColor );
}

// -----------------------------------------------------------------------

void SalGraphics::DrawPolygon( ULONG nPoints, const SalPoint* pPtAry,
                               const OutputDevice *pOutDev )
{
	long pXPoints[ nPoints ];
	long pYPoints[ nPoints ];
	for ( ULONG i = 0; i < nPoints; i++ )
	{
		pXPoints[ i ] = pPtAry->mnX;
		pYPoints[ i ] = pPtAry->mnY;
		pPtAry++;
	}
	
	if ( maGraphicsData.mnFillColor != 0xffffffff )
		maGraphicsData.mpVCLGraphics->drawPolygon( nPoints, pXPoints, pYPoints, maGraphicsData.mnFillColor, TRUE );
	if ( maGraphicsData.mnLineColor != 0xffffffff )
		maGraphicsData.mpVCLGraphics->drawPolygon( nPoints, pXPoints, pYPoints, maGraphicsData.mnLineColor, FALSE );
}

// -----------------------------------------------------------------------

void SalGraphics::DrawPolyPolygon( ULONG nPoly, const ULONG* pPoints,
								   PCONSTSALPOINT* pPtAry,
                                   const OutputDevice *pOutDev )
{
	long *pXPtsAry[ nPoly ];
	long *pYPtsAry[ nPoly ];
	ULONG i;
	for ( i = 0; i < nPoly; i++ )
	{
		long *pXPts = new long[ pPoints[ i ] ];
		long *pYPts = new long[ pPoints[ i ] ];
		const SalPoint *pPts = pPtAry[ i ];
		for ( ULONG j = 0; j < pPoints[ i ]; j++ )
		{
			pXPts[ j ] = pPts->mnX;
			pYPts[ j ] = pPts->mnY;
			pPts++;
		}
		pXPtsAry[ i ] = pXPts;
		pYPtsAry[ i ] = pYPts;
	}
	
	if ( maGraphicsData.mnFillColor != 0xffffffff )
		maGraphicsData.mpVCLGraphics->drawPolyPolygon( nPoly, pPoints, pXPtsAry, pYPtsAry, maGraphicsData.mnFillColor, TRUE );
	if ( maGraphicsData.mnLineColor != 0xffffffff )
		maGraphicsData.mpVCLGraphics->drawPolyPolygon( nPoly, pPoints, pXPtsAry, pYPtsAry, maGraphicsData.mnLineColor, FALSE);

	for ( i = 0; i < nPoly; i++ )
	{
		delete pXPtsAry[ i ];
		delete pYPtsAry[ i ];
	}
}

// -----------------------------------------------------------------------

sal_Bool SalGraphics::DrawPolyLineBezier( ULONG nPoints,
                                          const SalPoint* pPtAry,
                                          const BYTE* pFlgAry,
                                          const OutputDevice* )
{
#ifdef DEBUG
	fprintf( stderr, "SalGraphics::DrawPolyLineBezier not implemented\n" );
#endif
	return sal_False;
}

// -----------------------------------------------------------------------

sal_Bool SalGraphics::DrawPolygonBezier( ULONG nPoints,
                                         const SalPoint* pPtAry,
                                         const BYTE* pFlgAry,
                                         const OutputDevice* )
{
#ifdef DEBUG
	fprintf( stderr, "SalGraphics::DrawPolygonBezier not implemented\n" );
#endif
	return sal_False;
}

// -----------------------------------------------------------------------

sal_Bool SalGraphics::DrawPolyPolygonBezier( ULONG nPoly, const ULONG* nPoints,
                                         const SalPoint* const* pPtAry,
                                         const BYTE* const* pFlgAry,
                                         const OutputDevice* )
{
#ifdef DEBUG
	fprintf( stderr, "SalGraphics::DrawPolyPolygonBezier not implemented\n" );
#endif
	return sal_False;
}

// -----------------------------------------------------------------------

BOOL SalGraphics::DrawEPS( long nX, long nY, long nWidth, long nHeight,
                           void* pPtr, ULONG nSize,
                           const OutputDevice *pOutDev )
{
	BOOL bRet = FALSE;

	if ( !maGraphicsData.mpPrinter )
		return bRet;

	PMPrintSession pSession = (PMPrintSession)maGraphicsData.mpPrinter->maPrinterData.mpVCLPrintJob->getNativePrintJob();
	if ( pSession && pPtr && nSize )
	{
		EPSData aData;
		aData.maSession = pSession;
		aData.mnX = nX;
		aData.mnY = nY;
		aData.mnWidth = nWidth;
		aData.mnHeight = nHeight;
		aData.mpPtr = (MacOSPtr)pPtr;
		aData.mnSize = nSize;
		aData.mbSuccess = FALSE;

		VCLThreadAttach t;
		if ( t.pEnv->GetVersion() < JNI_VERSION_1_4 && !pEventLoopTimerUPP )
			pEventLoopTimerUPP = NewEventLoopTimerUPP( DrawEPSTimerCallback );

		if ( pEventLoopTimerUPP )
		{
			ULONG nCount = Application::ReleaseSolarMutex();

			// For Java 1.3.1, draw EPS in the Carbon event thread
			InstallEventLoopTimer( GetMainEventLoop(), 0, 0, pEventLoopTimerUPP, &aData, NULL );
			aData.maCondition.wait();

			Application::AcquireSolarMutex( nCount );
		}
		else
		{
			ImplDrawEPS( &aData );
		}

		bRet = aData.mbSuccess;
	}

	return bRet;
}

// -----------------------------------------------------------------------

long SalGraphics::GetGraphicsWidth()
{
	if ( maGraphicsData.mpFrame )
		return maGraphicsData.mpFrame->maGeometry.nWidth;
	else
		return 0;
}

// -----------------------------------------------------------------------

void SalGraphics::SetLineAntialiasing( BOOL bAntialias )
{
	maGraphicsData.mpVCLGraphics->setLineAntialiasing( bAntialias );
}

// =======================================================================

SalGraphicsData::SalGraphicsData()
{
	mnFillColor = MAKE_SALCOLOR( 0xff, 0xff, 0xff );
	mnLineColor = MAKE_SALCOLOR( 0, 0, 0 );
	mnTextColor = MAKE_SALCOLOR( 0, 0, 0 );
	mpFrame = NULL;
	mpPrinter = NULL;
	mpVirDev = NULL;
	mpVCLGraphics = NULL;
	mpVCLFont = NULL;
}

// -----------------------------------------------------------------------

SalGraphicsData::~SalGraphicsData()
{
	if ( mpVCLFont )
		delete mpVCLFont;

	for ( ::std::map< int, com_sun_star_vcl_VCLFont* >::const_iterator it = maFallbackFonts.begin(); it != maFallbackFonts.end(); ++it )
		delete it->second;
}
