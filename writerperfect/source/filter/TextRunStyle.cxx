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
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2002-2003 William Lachance (william.lachance@sympatico.ca)
 *  Copyright 2004 Fridrich Strba (fridrich.strba@bluewin.ch)
 *  http://libwpd.sourceforge.net
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
 *  Modified November 2004 by Patrick Luby. SISSL Removed. NeoOffice is
 *  distributed under GPL only under modification term 3 of the LGPL.
 *
 *  Contributor(s): _______________________________________
 *
 ************************************************************************/


#include "FilterInternal.hxx"
#include "TextRunStyle.hxx"
#include "WriterProperties.hxx"
#include "DocumentElement.hxx"

#ifdef _MSC_VER
#include <minmax.h>
#endif

using namespace ::rtl;
using rtl::OUString;

ParagraphStyle::ParagraphStyle(const uint8_t iParagraphJustification,
			       const float fMarginLeft, const float fMarginRight, const float fTextIndent, const float fLineSpacing,
			       const float fSpacingBeforeParagraph, const float fSpacingAfterParagraph, const vector<WPXTabStop> &tabStops,
			       const bool bColumnBreak, const bool bPageBreak, 
			       const char *psName, const char *psParentName) :
	Style(psName),
	msParentName(psParentName),
	mpsListStyleName(NULL),
	mfMarginLeft(fMarginLeft),
	mfMarginRight(fMarginRight),
	mfTextIndent(fTextIndent),
	mfLineSpacing(fLineSpacing),
	mfSpacingBeforeParagraph(fSpacingBeforeParagraph),
	mfSpacingAfterParagraph(fSpacingAfterParagraph),
	miParagraphJustification(iParagraphJustification),
	miNumTabStops(tabStops.size()),
	mbColumnBreak(bColumnBreak),
	mbPageBreak(bPageBreak)
{
	for (int i=0; i<miNumTabStops; i++)
		mTabStops.push_back(tabStops[i]);
}

ParagraphStyle::~ParagraphStyle()
{
	if (mpsListStyleName)
		delete mpsListStyleName;
}

void ParagraphStyle::write(Reference < XDocumentHandler > &xHandler) const
{
	WRITER_DEBUG_MSG(("Writing a paragraph style..\n"));
	TagOpenElement styleOpen("style:style");
	styleOpen.addAttribute("style:name", getName());
	styleOpen.addAttribute("style:family", "paragraph");
	styleOpen.addAttribute("style:parent-style-name", msParentName);
	if (getMasterPageName())
		styleOpen.addAttribute("style:master-page-name", getMasterPageName()->getUTF8());
	if (mpsListStyleName)
		styleOpen.addAttribute("style:list-style-name", mpsListStyleName->getUTF8());
	styleOpen.write(xHandler);

	TagOpenElement stylePropertiesOpen("style:properties");
	// margin properties
	if (mfMarginLeft != 0.0f || mfMarginRight != 0.0f || mfTextIndent != 0.0f)
	{
		UTF8String sMarginLeft;
		sMarginLeft.sprintf("%finch", mfMarginLeft);
		UTF8String sMarginRight;
		sMarginRight.sprintf("%finch", mfMarginRight);
		UTF8String sTextIndent;
		sTextIndent.sprintf("%finch", mfTextIndent);
		stylePropertiesOpen.addAttribute("fo:margin-left", sMarginLeft.getUTF8());
		stylePropertiesOpen.addAttribute("fo:margin-right", sMarginRight.getUTF8());
		stylePropertiesOpen.addAttribute("fo:text-indent", sTextIndent.getUTF8());
	}
	// line spacing
	if (mfLineSpacing != 1.0f) {
		UTF8String sLineSpacing;
		sLineSpacing.sprintf("%.2f%%", mfLineSpacing*100.0f);
		stylePropertiesOpen.addAttribute("fo:line-height", sLineSpacing.getUTF8());
	}
	if (mfSpacingAfterParagraph != 0.0f || mfSpacingBeforeParagraph != 0.0f) {
		UTF8String sSpacingAfterParagraph;
		sSpacingAfterParagraph.sprintf("%finch", mfSpacingAfterParagraph);
		UTF8String sSpacingBeforeParagraph;
		sSpacingBeforeParagraph.sprintf("%finch", mfSpacingBeforeParagraph);
		stylePropertiesOpen.addAttribute("fo:margin-top", sSpacingBeforeParagraph.getUTF8());
		stylePropertiesOpen.addAttribute("fo:margin-bottom", sSpacingAfterParagraph.getUTF8());
	}

	// column break
	if (mbColumnBreak) {
		stylePropertiesOpen.addAttribute("fo:break-before", "column");
	}

	if (mbPageBreak) {
		stylePropertiesOpen.addAttribute("fo:break-before", "page");
	}

	WRITER_DEBUG_MSG(("WriterWordPerfect: Adding justification style props: %i\n", miParagraphJustification));
	switch (miParagraphJustification)
		{
		case WPX_PARAGRAPH_JUSTIFICATION_LEFT:
			// doesn't require a paragraph prop - it is the default, but, like, whatever
			stylePropertiesOpen.addAttribute("fo:text-align", "left");
			break;
		case WPX_PARAGRAPH_JUSTIFICATION_CENTER:
			stylePropertiesOpen.addAttribute("fo:text-align", "center");
			break;
		case WPX_PARAGRAPH_JUSTIFICATION_RIGHT:
			stylePropertiesOpen.addAttribute("fo:text-align", "end");
			break;
		case WPX_PARAGRAPH_JUSTIFICATION_FULL:
			stylePropertiesOpen.addAttribute("fo:text-align", "justify");
			break;
		case WPX_PARAGRAPH_JUSTIFICATION_FULL_ALL_LINES:
			stylePropertiesOpen.addAttribute("fo:text-align", "justify");
			stylePropertiesOpen.addAttribute("fo:text-align-last", "justify");
			break;
	}
	stylePropertiesOpen.addAttribute("style:justify-single-word", "false");
	stylePropertiesOpen.write(xHandler);
	WRITER_DEBUG_MSG(("Writing %i tab stops\n", miNumTabStops));
	if (miNumTabStops > 0)
	{
		TagOpenElement tabListOpen("style:tab-stops");
		tabListOpen.write(xHandler);
		for (int i=0; i<miNumTabStops; i++)
		{
			TagOpenElement tabStopOpen("style:tab-stop");
			UTF8String sPosition;
			sPosition.sprintf("%.4finch", mTabStops[i].m_position);
			tabStopOpen.addAttribute("style:position", sPosition.getUTF8());
			WRITER_DEBUG_MSG(("Writing tab stops %s\n", sPosition.getUTF8()));
			switch (mTabStops[i].m_alignment)
			{
			case RIGHT:
				tabStopOpen.addAttribute("style:type", "right");
				break;
			case CENTER:
				tabStopOpen.addAttribute("style:type", "center");
				break;
			case DECIMAL:
				tabStopOpen.addAttribute("style:type", "char");
				tabStopOpen.addAttribute("style:char", "."); // Assume a decimal point for the while
				break;
			default:  // Left alignment is the default one and BAR is not handled in OOo
				break;
			}
			UCSString tempLeaderCharacter;
			tempLeaderCharacter.clear();
			if (mTabStops[i].m_leaderCharacter != 0x0000)
			{
				tempLeaderCharacter.append((uint32_t) mTabStops[i].m_leaderCharacter);
				UTF8String leaderCharacter(tempLeaderCharacter);
				tabStopOpen.addAttribute("style:leader-char", leaderCharacter.getUTF8()); 
			}
			tabStopOpen.write(xHandler);
			xHandler->endElement(OUString::createFromAscii("style:tab-stop"));
			
		}
		xHandler->endElement(OUString::createFromAscii("style:tab-stops"));
	}

	xHandler->endElement(OUString::createFromAscii("style:properties"));
	xHandler->endElement(OUString::createFromAscii("style:style"));
}

SpanStyle::SpanStyle(const uint32_t iTextAttributeBits, const char *pFontName, const float fFontSize,
					 const RGBSColor *pFontColor, const RGBSColor *pHighlightColor, const char *psName) :
	Style(psName),
	miTextAttributeBits(iTextAttributeBits),
	msFontName(pFontName),
	mfFontSize(fFontSize),
	m_fontColor(pFontColor->m_r,pFontColor->m_g,pFontColor->m_b,pFontColor->m_s),
	m_highlightColor((pHighlightColor?pHighlightColor->m_r:0xff), (pHighlightColor?pHighlightColor->m_g:0xff),
					 (pHighlightColor?pHighlightColor->m_b:0xff), (pHighlightColor?pHighlightColor->m_s:0xff))
{
}

void SpanStyle::write(Reference < XDocumentHandler > &xHandler) const
{
	WRITER_DEBUG_MSG(("Writing a span style..\n"));
	TagOpenElement styleOpen("style:style");
	styleOpen.addAttribute("style:name", getName());
	styleOpen.addAttribute("style:family", "text");
	styleOpen.write(xHandler);

	TagOpenElement stylePropertiesOpen("style:properties");
 	_addTextProperties(&stylePropertiesOpen);
	stylePropertiesOpen.write(xHandler);

	xHandler->endElement(OUString::createFromAscii("style:properties"));
	xHandler->endElement(OUString::createFromAscii("style:style"));
}

void SpanStyle::_addTextProperties(TagOpenElement *pStylePropertiesOpenElement) const
{
 	if (miTextAttributeBits & WPX_SUPERSCRIPT_BIT) {
		UTF8String sSuperScript;
		sSuperScript.sprintf("super %s", IMP_DEFAULT_SUPER_SUB_SCRIPT);
		pStylePropertiesOpenElement->addAttribute("style:text-position", sSuperScript.getUTF8());
	}
 	if (miTextAttributeBits & WPX_SUBSCRIPT_BIT) {
		UTF8String sSubScript;
		sSubScript.sprintf("sub %s", IMP_DEFAULT_SUPER_SUB_SCRIPT);
		pStylePropertiesOpenElement->addAttribute("style:text-position", sSubScript.getUTF8());
	}
	if (miTextAttributeBits & WPX_ITALICS_BIT) {
		pStylePropertiesOpenElement->addAttribute("fo:font-style", "italic");
	}
	if (miTextAttributeBits & WPX_BOLD_BIT) {
		pStylePropertiesOpenElement->addAttribute("fo:font-weight", "bold");
	}
	if (miTextAttributeBits & WPX_STRIKEOUT_BIT) {
		pStylePropertiesOpenElement->addAttribute("style:text-crossing-out", "single-line");
	}
 	if (miTextAttributeBits & WPX_UNDERLINE_BIT) {
		pStylePropertiesOpenElement->addAttribute("style:text-underline", "single");
	}
	if (miTextAttributeBits & WPX_DOUBLE_UNDERLINE_BIT) {
		pStylePropertiesOpenElement->addAttribute("style:text-underline", "double");
	}
	if (miTextAttributeBits & WPX_OUTLINE_BIT) {
		pStylePropertiesOpenElement->addAttribute("style:text-outline", "true");
	}
	if (miTextAttributeBits & WPX_SMALL_CAPS_BIT) {
		pStylePropertiesOpenElement->addAttribute("fo:font-variant", "small-caps");
	}
	if (miTextAttributeBits & WPX_BLINK_BIT) {
		pStylePropertiesOpenElement->addAttribute("style:text-blinking", "true");
	}
	if (miTextAttributeBits & WPX_SHADOW_BIT) {
		pStylePropertiesOpenElement->addAttribute("fo:text-shadow", "1pt 1pt");
	}

	pStylePropertiesOpenElement->addAttribute("style:font-name", msFontName.getUTF8());
	UTF8String sFontSize;
	sFontSize.sprintf("%ipt", (int)mfFontSize);
	pStylePropertiesOpenElement->addAttribute("fo:font-size", sFontSize.getUTF8());

	if (!(miTextAttributeBits & WPX_REDLINE_BIT))
	// Here we give the priority to the redline bit over the font color. This is how WordPerfect behaves:
	// redline overrides font color even if the color is changed when redline was already defined.
	// When redline finishes, the color is back.
	{
		UTF8String sFontColor;
		float fontShading = (float)((float)m_fontColor.m_s/100.0f); //convert the percents to float between 0 and 1
		// Mix fontShading amount of given color with (1-fontShading) of White (#ffffff)
		int fontRed = (int)0xFF + (int)((float)m_fontColor.m_r*fontShading) - (int)((float)0xFF*fontShading);
		int fontGreen = (int)0xFF + (int)((float)m_fontColor.m_g*fontShading) - (int)((float)0xFF*fontShading);
		int fontBlue = (int)0xFF + (int)((float)m_fontColor.m_b*fontShading) - (int)((float)0xFF*fontShading);
		sFontColor.sprintf("#%.2x%.2x%.2x", fontRed, fontGreen, fontBlue);
		pStylePropertiesOpenElement->addAttribute("fo:color", sFontColor.getUTF8());
	}
	else // redlining applies
	{
		pStylePropertiesOpenElement->addAttribute("fo:color", "#ff3333"); // #ff3333 = a nice bright red
	}

	if (m_highlightColor.m_s != 0xff)
	{
		UTF8String sHighlightColor;
		float highlightShading = (float)((float)m_highlightColor.m_s/100.0f);
		int highlightRed = (int)0xFF + (int)((float)m_highlightColor.m_r*highlightShading) - (int)((float)0xFF*highlightShading);
		int highlightGreen = (int)0xFF + (int)((float)m_highlightColor.m_g*highlightShading) - (int)((float)0xFF*highlightShading);
		int highlightBlue = (int)0xFF + (int)((float)m_highlightColor.m_b*highlightShading) - (int)((float)0xFF*highlightShading);
		sHighlightColor.sprintf("#%.2x%.2x%.2x", highlightRed, highlightGreen, highlightBlue);
		pStylePropertiesOpenElement->addAttribute("style:text-background-color", sHighlightColor.getUTF8());
	}
	else
		pStylePropertiesOpenElement->addAttribute("style:text-background-color", "transparent");

}
