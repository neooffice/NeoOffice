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
 *  Sun Microsystems Inc., December, 2003
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2003 by Sun Microsystems, Inc.
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
 *  Modified July 2004 by Patrick Luby. SISSL Removed. NeoOffice is
 *  distributed under GPL only under modification term 3 of the LGPL.
 *
 *  Original source obtained from patch submitted to OpenOffice.org in issue
 *  23283 (see http://qa.openoffice.org/issues/show_bug.cgi?id=23283).
 *
 *  Contributor(s): _______________________________________
 *
 ************************************************************************/

#ifndef _SV_SALGDI_HXX
#include <salgdi.hxx>
#endif
#ifndef _SV_OUTFONT_HXX
#include <outfont.hxx>
#endif
#ifndef _SV_SALLAYOUT_HXX
#include <sallayout.hxx>
#endif
#ifndef _SV_COM_SUN_STAR_VCL_VCLFONT_HXX
#include <com/sun/star/vcl/VCLFont.hxx>
#endif
#ifndef _SV_COM_SUN_STAR_VCL_VCLGRAPHICS_HXX
#include <com/sun/star/vcl/VCLGraphics.hxx>
#endif
#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif
#ifndef _RTL_STRBUF_HXX_
#include <rtl/strbuf.hxx>
#endif

#include <premac.h>
#include <Carbon/Carbon.h>
#include <postmac.h>

inline int Float32ToLong( Float32 f ) { return (long)( f + 0.5 ); }

class ATSLayout : public GenericSalLayout
{
	::vcl::com_sun_star_vcl_VCLFont*	mpVCLFont;
	bool				mbUseScreenMetrics;
	ATSUStyle			maFontStyle;
	int					mnGlyphCount;
	ATSUGlyphInfoArray*	mpGlyphInfoArray;
	long*				mpGlyphTranslations;
	int*				mpCharsToGlyphs;
	int*				mpVerticalFlags;

	void				Destroy();

public:
						ATSLayout( ::vcl::com_sun_star_vcl_VCLFont *pVCLFont, bool bUseScreenMetrics );
	virtual				~ATSLayout();

	virtual bool		LayoutText( ImplLayoutArgs& rArgs );
	virtual void		AdjustLayout( ImplLayoutArgs& rArgs );
	virtual void		DrawText( SalGraphics& rGraphics ) const;
};

using namespace osl;
using namespace rtl;
using namespace vcl;

// ============================================================================

static size_t DataConsumerPutBytes( void *info, const void *buffer, size_t count )
{
	sal_uInt64 nBytesWritten = 0;

	if ( info )
		((File *)info)->write( buffer, count, nBytesWritten );

	return nBytesWritten;
}

// ----------------------------------------------------------------------------

SalLayout *SalGraphics::GetTextLayout( ImplLayoutArgs& rArgs, int nFallbackLevel )
{
	return new ATSLayout( maGraphicsData.mpVCLFont, !maGraphicsData.mpPrinter );
}

// ----------------------------------------------------------------------------

BOOL SalGraphics::CreateFontSubset( const OUString& rToFile,
                                    ImplFontData* pFont, long* pGlyphIDs,
                                    sal_uInt8* pEncoding, sal_Int32* pWidths,
                                    int nGlyphs, FontSubsetInfo& rInfo )
{
	BOOL bRet = FALSE;

	com_sun_star_vcl_VCLFont *pVCLFont = (com_sun_star_vcl_VCLFont *)pFont->mpSysData;
	ATSFontRef aATSFont = FMGetATSFontRefFromFont( (FMFont)pVCLFont->getNativeFont() );
	CGFontRef aFont = CGFontCreateWithPlatformFont( (void *)&aATSFont );
	if ( !aFont )
		return bRet;

	OUString aTmpName;
	osl_createTempFile( NULL, NULL, &aTmpName.pData );
	File aTmpFile( aTmpName );

	if ( aTmpFile.open( OpenFlag_Write ) == FileBase::E_None )
	{
		CGDataConsumerCallbacks aCallbacks;
		memset( &aCallbacks, 0, sizeof( CGDataConsumerCallbacks ) );
		aCallbacks.putBytes = DataConsumerPutBytes;

		CGDataConsumerRef aDataConsumer = CGDataConsumerCreate( &aTmpFile, &aCallbacks );
		if ( aDataConsumer )
		{
			CGContextRef aContext = CGPDFContextCreate( aDataConsumer, NULL, NULL );
			if ( aContext )
			{
				CGContextBeginPage( aContext, NULL );
				CGContextSetFont( aContext, aFont );
				CGContextSetFontSize( aContext, pVCLFont->getSize() );
				CGContextShowGlyphs( aContext, (CGGlyph *)pGlyphIDs, nGlyphs );
				CGContextEndPage( aContext );

				CGContextRelease( aContext );
			}

			CGDataConsumerRelease( aDataConsumer );
		}

		aTmpFile.close();
	}

	OString aFontDescriptor;

	if ( aTmpFile.open( OpenFlag_Read ) == FileBase::E_None )
	{
		static const char *pFontDescStart = "<< /Type /FontDescriptor";
		static const char *pFontDescEnd = ">>";

		OStringBuffer aBuffer;
		bool bFontDescStartFound = false;
		bool bFontDescEndFound = false;
		sal_uInt64 nBufSize = 1024;
		sal_Char aBuf[ 1024 ];
		sal_uInt64 nBytesRead;
		sal_uInt64 nOffset = 0;
		while ( !bFontDescEndFound && aTmpFile.read( aBuf + nOffset, nBufSize - nOffset, nBytesRead ) == FileBase::E_None  )
		{
			sal_Bool bEOF = false;
			if ( aTmpFile.isEndOfFile( &bEOF ) != FileBase::E_None )
				bEOF = true;

			if ( !nBytesRead )
			{
				if ( bEOF )
					break;
				else
					continue;
			}

			if ( !bFontDescStartFound )
			{
				// Look for beginning token
				sal_uInt32 nLen = strlen( pFontDescStart );
				if ( nBytesRead < nLen )
				{
					if ( bEOF )
					{
						break;
					}
					else
					{
						nOffset += nBytesRead;
						continue;
					}
				}

				// Skip bytes at the end so that split tokens are handled in
				// the next pass
				sal_uInt64 nSkippedBytes = nLen;
				nBytesRead += nOffset - nSkippedBytes;

				sal_Char *pBuf = aBuf;
				for ( sal_uInt64 i = 0; i < nBytesRead; i++, pBuf++ )
				{
					if ( *pBuf == *pFontDescStart && !strncmp( pBuf, pFontDescStart, nLen ) )
					{
						// Adjust skipped bytes to skip over matched token
						nSkippedBytes += nBytesRead - i - nLen;
						nBytesRead = i + nLen;
						bFontDescStartFound = true;
						break;
					}
				}

				// Move skipped bytes to beginning of buffer
				memmove( aBuf, aBuf + nBytesRead, nSkippedBytes );
				nOffset = nSkippedBytes;
			}
			else
			{
				// Look for the ending token
				sal_uInt32 nLen = strlen( pFontDescEnd );
				if ( nBytesRead < nLen )
				{
					if ( bEOF )
					{
						break;
					}
					else
					{
						nOffset += nBytesRead;
						continue;
					}
				}

				// Skip bytes at the end so that split tokens are handled in
				// the next pass
				sal_uInt64 nSkippedBytes = nLen;
				nBytesRead += nOffset - nSkippedBytes;

				sal_Char *pBuf = aBuf;
				for ( sal_uInt64 i = 0; i < nBytesRead; i++, pBuf++ )
				{
					if ( *pBuf == *pFontDescEnd && !strncmp( pBuf, pFontDescEnd, nLen ) )
					{
						// Adjust skipped bytes to start of ending token
						nSkippedBytes += nBytesRead - i;
						nBytesRead = i;
						bFontDescEndFound = true;
						break;
					}
				}

				// Save all bytes up to the ending token
				aBuffer.append( aBuf, nBytesRead );

				// Move skipped bytes to beginning of buffer
				memmove( aBuf, aBuf + nBytesRead, nSkippedBytes );
				nOffset = nSkippedBytes;
			}
		}

		aTmpFile.close();

		if ( aBuffer.getLength() )
		{
			// Replace all whitespace with spaces for ease of parsing
			for ( sal_Char *pBuf = (sal_Char *)aBuffer.getStr(); *pBuf; pBuf++ )
			{
				switch ( *pBuf )
				{
					case 0x09:
					case 0x0A:
					case 0x0C:
					case 0x0D:
						*pBuf = 0x20;
						break;
					default:
						break;
				}
			}

			aFontDescriptor = aBuffer.makeStringAndClear();
		}
	}

	if ( aFontDescriptor.getLength() )
	{
		static OString aFontAscentTag( "/Ascent " );
		static OString aFontBBoxTag( "/FontBBox [ " );
		static OString aFontCapHeightTag( "/CapHeight " );
		static OString aFontDescentTag( "/Descent " );
		static OString aFontFileTag( "/FontFile " );
		static OString aFontFile2Tag( "/FontFile2 " );
		static OString aFontNameTag( "/FontName /" );

		bool bContinue = true;

		// Get font type
		sal_Int32 nValuePos;
		if ( ( nValuePos = aFontDescriptor.indexOf( aFontFile2Tag ) ) >= 0 )
		{
			rInfo.m_nFontType = SAL_FONTSUBSETINFO_TYPE_TRUETYPE;
			nValuePos += aFontFile2Tag.getLength();
		}
		else if ( ( nValuePos = aFontDescriptor.indexOf( aFontFileTag ) ) >= 0 )
		{
			rInfo.m_nFontType = SAL_FONTSUBSETINFO_TYPE_TYPE1;
			nValuePos += aFontFileTag.getLength();
		}
		else
		{
			bContinue = false;
		}

		// Get font file object reference
		sal_Int32 nFontFileObj = 0;
		if ( bContinue )
		{
			sal_Int32 nLen = 0;
			const char *pBuf = aFontDescriptor.getStr() + nValuePos;
			for ( ; *pBuf; pBuf++, nLen++ )
			{
				if ( *pBuf == 0x20 )
					break;
			}

			if ( nLen )
				nFontFileObj = OString( pBuf - nLen, nLen ).toInt32();
		}

		if ( nFontFileObj <= 0 )
			bContinue = false;

		// Get PostScript name
		if ( bContinue && ( nValuePos = aFontDescriptor.indexOf( aFontNameTag ) ) >= 0 )
		{
			nValuePos += aFontNameTag.getLength();

			sal_Int32 nLen = 0;
			const sal_Char *pBuf = aFontDescriptor.getStr() + nValuePos;
			for ( ; *pBuf; pBuf++, nLen++ )
			{
				if ( *pBuf == 0x20 )
					break;
				else if ( nLen == 6 && *pBuf == '+' )
					nLen = -1;
			}

			if ( nLen )
				rInfo.m_aPSName = OUString( pBuf - nLen, nLen, RTL_TEXTENCODING_UTF8 );
		}

		if ( !rInfo.m_aPSName.Len() )
			bContinue = false;

		// Get ascent
		rInfo.m_nAscent = 0;
		if ( bContinue && ( nValuePos = aFontDescriptor.indexOf( aFontAscentTag ) ) >= 0 )
		{
			nValuePos += aFontAscentTag.getLength();

			sal_Int32 nLen = 0;
			const sal_Char *pBuf = aFontDescriptor.getStr() + nValuePos;
			for ( ; *pBuf; pBuf++, nLen++ )
			{
				if ( *pBuf == 0x20 )
					break;
			}

			if ( nLen )
				rInfo.m_nAscent = OString( pBuf - nLen, nLen ).toInt32();
		}

		if ( !rInfo.m_nAscent )
			bContinue = false;

		// Get descent
		rInfo.m_nDescent = 0;
		if ( bContinue && ( nValuePos = aFontDescriptor.indexOf( aFontDescentTag ) ) >= 0 )
		{
			nValuePos += aFontDescentTag.getLength();

			sal_Int32 nLen = 0;
			const sal_Char *pBuf = aFontDescriptor.getStr() + nValuePos;
			for ( ; *pBuf; pBuf++, nLen++ )
			{
				if ( *pBuf == 0x20 )
					break;
			}

			if ( nLen )
				rInfo.m_nDescent = OString( pBuf - nLen, nLen ).toInt32();
		}

		if ( !rInfo.m_nDescent )
			bContinue = false;

		// Get cap height. Note that cap height can be zero.
		rInfo.m_nCapHeight = 0;
		if ( bContinue && ( nValuePos = aFontDescriptor.indexOf( aFontCapHeightTag ) ) >= 0 )
		{
			nValuePos += aFontCapHeightTag.getLength();

			sal_Int32 nLen = 0;
			const sal_Char *pBuf = aFontDescriptor.getStr() + nValuePos;
			for ( ; *pBuf; pBuf++, nLen++ )
			{
				if ( *pBuf == 0x20 )
					break;
			}

			if ( nLen )
				rInfo.m_nCapHeight = OString( pBuf - nLen, nLen ).toInt32();
		}

		// Get bounding box
		rInfo.m_aFontBBox.SetEmpty();
		if ( bContinue && ( nValuePos = aFontDescriptor.indexOf( aFontBBoxTag ) ) >= 0 )
		{
			nValuePos += aFontBBoxTag.getLength();

			sal_Int32 nLen = 0;
			const sal_Char *pBuf = aFontDescriptor.getStr() + nValuePos;

			// Get X coordinate
			for ( ; *pBuf; pBuf++, nLen++ )
			{
				if ( *pBuf == 0x20 )
					break;
			}

			if ( nLen )
				rInfo.m_aFontBBox.setX( OString( pBuf - nLen, nLen ).toInt32() );

			// Get Y coordinate
			nLen = 0;
			for ( pBuf++; *pBuf; pBuf++, nLen++ )
			{
				if ( *pBuf == 0x20 )
					break;
			}

			if ( nLen )
				rInfo.m_aFontBBox.setY( OString( pBuf - nLen, nLen ).toInt32() );

			// Get width
			nLen = 0;
			for ( pBuf++; *pBuf; pBuf++, nLen++ )
			{
				if ( *pBuf == 0x20 )
					break;
			}

			if ( nLen )
				rInfo.m_aFontBBox.setWidth( OString( pBuf - nLen, nLen ).toInt32() );

			// Get height
			nLen = 0;
			for ( pBuf++; *pBuf; pBuf++, nLen++ )
			{
				if ( *pBuf == 0x20 )
					break;
			}

			if ( nLen )
				rInfo.m_aFontBBox.setHeight( OString( pBuf - nLen, nLen ).toInt32() );
		}

		if ( rInfo.m_aFontBBox.IsEmpty() )
			bContinue = false;

		bRet = bContinue;
	}

	osl_removeFile( aTmpName.pData );

	return bRet;
}

// ============================================================================

ATSLayout::ATSLayout( com_sun_star_vcl_VCLFont *pVCLFont, bool bUseScreenMetrics ) :
	maFontStyle( NULL ),
	mbUseScreenMetrics( bUseScreenMetrics ),
	mnGlyphCount( 0 ),
	mpGlyphInfoArray( NULL ),
	mpGlyphTranslations( NULL ),
	mpCharsToGlyphs( NULL ),
	mpVerticalFlags( NULL )
{
	SetUnitsPerPixel( 1000 );

	mpVCLFont = new com_sun_star_vcl_VCLFont( pVCLFont->getJavaObject() );

	// Create font style
	if ( ATSUCreateStyle( &maFontStyle ) == noErr )
	{
		ATSUAttributeTag nTags[3];
		ByteCount nBytes[3];
		ATSUAttributeValuePtr nVals[3];

		// Set font
		ATSUFontID nFontID = (ATSUFontID)mpVCLFont->getNativeFont();
		if ( !nFontID )
		{
			// Fall back to Geneva as a last resort
			if ( ATSUFindFontFromName( "Geneva", 6, kFontFullName, kFontNoPlatformCode, kFontNoScriptCode, kFontNoLanguageCode, &nFontID ) != noErr )
				nFontID = kATSUInvalidFontID;
		}
		nTags[0] = kATSUFontTag;
		nBytes[0] = sizeof( ATSUFontID );
		nVals[0] = &nFontID;

		// Set font size
		Fixed nSize = Long2Fix( mpVCLFont->getSize() );
		nTags[1] = kATSUSizeTag;
		nBytes[1] = sizeof( Fixed );
		nVals[1] = &nSize;

		// Set antialiasing
		ATSStyleRenderingOptions nOptions;
		if ( mpVCLFont->isAntialiased() )
			nOptions = kATSStyleApplyAntiAliasing;
		else
			nOptions = kATSStyleNoAntiAliasing;
		nTags[2] = kATSUStyleRenderingOptionsTag;
		nBytes[2] = sizeof( ATSStyleRenderingOptions );
		nVals[2] = &nOptions;

		if ( ATSUSetAttributes( maFontStyle, 3, nTags, nBytes, nVals ) != noErr )
		{
			ATSUDisposeStyle( maFontStyle );
			maFontStyle = NULL;
		}
	}
}

// ----------------------------------------------------------------------------

ATSLayout::~ATSLayout()
{
	Destroy();

	if ( mpVCLFont )
		delete mpVCLFont;

	if ( maFontStyle )
		ATSUDisposeStyle( maFontStyle );
}

// ----------------------------------------------------------------------------

void ATSLayout::Destroy()
{
	mnGlyphCount = 0;

	if ( mpGlyphInfoArray )
		rtl_freeMemory( mpGlyphInfoArray );
	mpGlyphInfoArray = NULL;

	if ( mpGlyphTranslations )
		rtl_freeMemory( mpGlyphTranslations );
	mpGlyphTranslations = NULL;

	if ( mpCharsToGlyphs )
		rtl_freeMemory( mpCharsToGlyphs );
	mpCharsToGlyphs = NULL;

	if ( mpVerticalFlags )
		rtl_freeMemory( mpVerticalFlags );
	mpVerticalFlags = NULL;
}

// ----------------------------------------------------------------------------

bool ATSLayout::LayoutText( ImplLayoutArgs& rArgs )
{
	if ( !maFontStyle )
		return false;

	int nLen = rArgs.mnEndCharPos - rArgs.mnMinCharPos;
	if ( !nLen )
		return false;

	bool bRTL = ( rArgs.mnFlags & SAL_LAYOUT_BIDI_STRONG && rArgs.mnFlags & SAL_LAYOUT_BIDI_RTL );
	bool bVertical = ( rArgs.mnFlags & SAL_LAYOUT_VERTICAL );
	long nAscent = bVertical ? mpVCLFont->getAscent() : 0;
	long nDescent = bVertical ? mpVCLFont->getDescent() : 0;
	long nAdjust = bVertical ? Float32ToLong( ( nAscent + nDescent ) * 0.05 ) : 0;
	long nDoubleAdjust = bVertical ? nAdjust * 2 : 0;

	if ( ! ( rArgs.mnFlags & SAL_LAYOUT_DISABLE_GLYPH_PROCESSING ) )
	{
		// Create a copy of the string so that we can perform mirroring. Note
		// that we add the leading and/or trailing characters if this is a
		// substring to ensure that we get the correct layout of glyphs.
		nLen += 2;
		sal_Unicode aStr[ nLen ];

		if ( rArgs.mnMinCharPos )
			aStr[ 0 ] = rArgs.mpStr[ rArgs.mnMinCharPos - 1 ];
		else
			aStr[ 0 ] = 0x0020;

		if ( rArgs.mnEndCharPos < rArgs.mnLength )
			aStr[ nLen - 1 ] = rArgs.mpStr[ rArgs.mnEndCharPos ];
		else
			aStr[ nLen - 1 ] = 0x0020;

		// Copy characters
		int nRunStart;
		int nRunEnd;
		bool bRunRTL;
		while ( rArgs.GetNextRun( &nRunStart, &nRunEnd, &bRunRTL ) )
		{
			for ( int i = nRunStart; i < nRunEnd; i++ )
			{
				int j = i - rArgs.mnMinCharPos + 1;
				aStr[ j ] = rArgs.mpStr[ i ];

				// Mirror RTL characters
				if ( bRTL && bRunRTL )
				{
					sal_Unicode nChar = GetMirroredChar( aStr[ j ] );
					if ( nChar )
						aStr[ j ] = nChar;
				}
			}
		}

		ATSUTextLayout aLayout;
		if ( ATSUCreateTextLayoutWithTextPtr( aStr, kATSUFromTextBeginning, kATSUToTextEnd, nLen, 1, (const UniCharCount *)&nLen, &maFontStyle, &aLayout ) != noErr )
		{
			Destroy();
			return false;
		}

		ATSUAttributeTag nTags[2];
		ByteCount nBytes[2];
		ATSUAttributeValuePtr nVals[2];
		MacOSBoolean nDirection;
		if ( bRTL )
			nDirection = kATSURightToLeftBaseDirection;
		else
			nDirection = kATSULeftToRightBaseDirection;
		nTags[0] = kATSULineDirectionTag;
		nBytes[0] = sizeof( MacOSBoolean );
		nVals[0] = &nDirection;
		ATSLineLayoutOptions nOptions = kATSLineKeepSpacesOutOfMargin;
		nTags[1] = kATSULineLayoutOptionsTag;
		nBytes[1] = sizeof( ATSLineLayoutOptions );
		nVals[1] = &nOptions;

		if ( ATSUSetLayoutControls( aLayout, 2, nTags, nBytes, nVals ) != noErr )
		{
			ATSUDisposeTextLayout( aLayout );
			Destroy();
			return false;
		}

		ByteCount nBufSize;
		if ( ATSUGetGlyphInfo( aLayout, kATSUFromTextBeginning, kATSUToTextEnd, &nBufSize, NULL ) != noErr )
		{
			ATSUDisposeTextLayout( aLayout );
			Destroy();
			return false;
		}

		if ( mpGlyphInfoArray )
			rtl_freeMemory( mpGlyphInfoArray );
		mpGlyphInfoArray = (ATSUGlyphInfoArray *)rtl_allocateMemory( nBufSize );

		ByteCount nRetSize = nBufSize;
		if ( ATSUGetGlyphInfo( aLayout, kATSUFromTextBeginning, kATSUToTextEnd, &nRetSize, mpGlyphInfoArray ) != noErr || nRetSize != nBufSize )
		{
			ATSUDisposeTextLayout( aLayout );
			Destroy();
			return false;
		}

		mnGlyphCount = mpGlyphInfoArray->numGlyphs;

		ATSUDisposeTextLayout( aLayout );

		if ( bVertical )
		{
			// Cache vertical flags and glyph translations
			nBufSize = nLen * sizeof( int );
			if ( mpVerticalFlags )
				rtl_freeMemory( mpVerticalFlags );
			mpVerticalFlags = (int *)rtl_allocateMemory( nBufSize );

			nBufSize = nLen * 2 * sizeof( long );
			if ( mpGlyphTranslations )
				rtl_freeMemory( mpGlyphTranslations );
			mpGlyphTranslations = (long *)rtl_allocateMemory( nBufSize );
			memset( mpGlyphTranslations, 0, nBufSize );

			int i;
			int j;
			for ( i = 0, j = 0; i < nLen; i++, j += 2 )
			{
				mpVerticalFlags[ i ] = GetVerticalFlags( aStr[ i ] );

				if ( mpVerticalFlags[ i ] & GF_ROTL )
				{
					ATSGlyphScreenMetrics aScreenMetrics;
					if ( ATSUGlyphGetScreenMetrics( mpGlyphInfoArray->glyphs[ i ].style, 1, &mpGlyphInfoArray->glyphs[ i ].glyphID, sizeof( GlyphID ), true, true, &aScreenMetrics ) == noErr )
					{
						mpGlyphTranslations[ j ] = Float32ToLong( ( nAscent + nDescent - aScreenMetrics.deviceAdvance.x + aScreenMetrics.sideBearing.x + aScreenMetrics.otherSideBearing.x ) / 2 ) - nDescent;
						mpGlyphTranslations[ j + 1 ] = Float32ToLong( aScreenMetrics.topLeft.y ) + nAdjust;
					}
				}
				else if ( mpVerticalFlags[ i ] & GF_ROTR )
				{
					ATSGlyphScreenMetrics aScreenMetrics;
					if ( ATSUGlyphGetScreenMetrics( mpGlyphInfoArray->glyphs[ i ].style, 1, &mpGlyphInfoArray->glyphs[ i ].glyphID, sizeof( GlyphID ), true, true, &aScreenMetrics ) == noErr )
					{
						mpGlyphTranslations[ j ] = nAscent - Float32ToLong( ( nAscent + nDescent - aScreenMetrics.deviceAdvance.x + aScreenMetrics.sideBearing.x + aScreenMetrics.otherSideBearing.x ) / 2 );
						mpGlyphTranslations[ j + 1 ] = Float32ToLong( aScreenMetrics.height - aScreenMetrics.topLeft.y ) + nAdjust;
					}
				}
			}
		}

		// Cache mapping of characters to glyphs
		nBufSize = nLen * sizeof( int );
		if ( mpCharsToGlyphs )
			rtl_freeMemory( mpCharsToGlyphs );
		mpCharsToGlyphs = (int *)rtl_allocateMemory( nBufSize );

		int i;
		for ( i = 0; i < nLen; i++ )
			mpCharsToGlyphs[ i ] = -1;
		for ( i = 0; i < mnGlyphCount; i++ )
		{
			int nIndex = mpGlyphInfoArray->glyphs[ i ].charIndex;
			if ( mpCharsToGlyphs[ nIndex ] < 0 || i < mpCharsToGlyphs[ nIndex ] )
				mpCharsToGlyphs[ nIndex ] = i;
		}

		rArgs.mnFlags |= SAL_LAYOUT_DISABLE_GLYPH_PROCESSING;
	}

	if ( !mnGlyphCount || !mpGlyphInfoArray || !mpCharsToGlyphs )
	{
		Destroy();
		return false;
	}

	// Calculate and cache glyph advances
	bool bPosRTL;
	Point aPos( 0, 0 );
	int nCharPos = -1;
	while ( rArgs.GetNextPos( &nCharPos, &bPosRTL ) )
	{
		int nIndex = nCharPos - rArgs.mnMinCharPos + 1;
		for ( int i = mpCharsToGlyphs[ nIndex ]; i < mnGlyphCount && mpGlyphInfoArray->glyphs[ i ].charIndex == nIndex; i++ )
		{
			int nGlyph = mpGlyphInfoArray->glyphs[ i ].glyphID;
			long nCharWidth = 0;
			if ( mpGlyphTranslations && mpVerticalFlags && mpVerticalFlags[ mpGlyphInfoArray->glyphs[ i ].charIndex ] & ( GF_ROTL | GF_ROTR ) )
			{
				ATSGlyphScreenMetrics aScreenMetrics;
				if ( ATSUGlyphGetScreenMetrics( mpGlyphInfoArray->glyphs[ i ].style, 1, &mpGlyphInfoArray->glyphs[ i ].glyphID, sizeof( GlyphID ), true, true, &aScreenMetrics ) == noErr )
					nCharWidth = Float32ToLong( ( aScreenMetrics.height + nDoubleAdjust ) * mnUnitsPerPixel );
			}
			else if ( mbUseScreenMetrics )
			{
				nCharWidth = Float32ToLong( ( mpGlyphInfoArray->glyphs[ i + 1 ].screenX - mpGlyphInfoArray->glyphs[ i ].screenX ) * mnUnitsPerPixel );
			}
			else
			{
				nCharWidth = Float32ToLong( ( mpGlyphInfoArray->glyphs[ i + 1 ].idealX - mpGlyphInfoArray->glyphs[ i ].idealX ) * mnUnitsPerPixel );
			}

			int nGlyphFlags = nCharWidth ? 0 : GlyphItem::IS_IN_CLUSTER;

			if ( bPosRTL )
				nGlyphFlags |= GlyphItem::IS_RTL_GLYPH;

			GlyphItem aGI( nCharPos, nGlyph, aPos, nGlyphFlags, nCharWidth );
			aGI.mnNewWidth = nCharWidth;
			AppendGlyph( aGI );

			aPos.X() += nCharWidth;
		}
	}

	return ( nCharPos >= 0 );
}

// ----------------------------------------------------------------------------

void ATSLayout::AdjustLayout( ImplLayoutArgs& rArgs )
{
	GenericSalLayout::AdjustLayout( rArgs );

	// Asian kerning
	if ( rArgs.mnFlags & SAL_LAYOUT_KERNING_ASIAN && ! ( rArgs.mnFlags & SAL_LAYOUT_VERTICAL ) )
		ApplyAsianKerning( rArgs.mpStr, rArgs.mnLength );
}

// ----------------------------------------------------------------------------

void ATSLayout::DrawText( SalGraphics& rGraphics ) const
{
	if ( !mnGlyphCount )
		return;

	int nMaxGlyphs( mnGlyphCount );
	long aGlyphArray[ nMaxGlyphs ];
	long aDXArray[ nMaxGlyphs ];
	int aCharPosArray[ nMaxGlyphs ];

	Point aPos;
	for ( int nStart = 0; ; )
	{
		int nGlyphCount = GetNextGlyphs( nMaxGlyphs, aGlyphArray, aPos, nStart, aDXArray, aCharPosArray );

		if ( !nGlyphCount )
			break;

		int nOrientation = GetOrientation();

		int j = aCharPosArray[ 0 ] - mnMinCharPos + 1;
		if ( mpGlyphTranslations && mpVerticalFlags && mpVerticalFlags[ j ] & ( GF_ROTL | GF_ROTR ) )
		{
			// Draw rotated glyphs one at a time. Note that we rotated the
			// coordinates when the glyphs were added.
			int nGlyphRotation;
			if ( mpVerticalFlags[ j ] & GF_ROTL )
				nGlyphRotation = 900;
			else
				nGlyphRotation = -900;

			aGlyphArray[ 0 ] &= GF_IDXMASK;

			int k = j * 2;
			rGraphics.maGraphicsData.mpVCLGraphics->drawGlyphs( aPos.X(), aPos.Y(), 1, aGlyphArray, aDXArray, mpVCLFont, rGraphics.maGraphicsData.mnTextColor, nOrientation + nGlyphRotation, mpGlyphTranslations[ k ], mpGlyphTranslations[ k + 1 ], mnUnitsPerPixel );

			nStart -= nGlyphCount - 1;
			continue;
		}
		else
		{
			for ( int i = 0; i < nGlyphCount; i++ )
				aGlyphArray[ i ] &= GF_IDXMASK;

			rGraphics.maGraphicsData.mpVCLGraphics->drawGlyphs( aPos.X(), aPos.Y(), nGlyphCount, aGlyphArray, aDXArray, mpVCLFont, rGraphics.maGraphicsData.mnTextColor, nOrientation, 0, 0, mnUnitsPerPixel );
		}
	}
}
