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

/* "This product is not manufactured, approved, or supported by 
 * Corel Corporation or Corel Corporation Limited."
 */

#include "DocumentElement.hxx"
#include "DocumentHandler.hxx"
#include "FilterInternal.hxx"
#include <string.h>

#define ASCII_SPACE 0x0020

void TagElement::print() const
{
	WRITER_DEBUG_MSG(("%s\n", msTagName.cstr()));
}

void TagOpenElement::write(DocumentHandler &xHandler) const
{
	xHandler.startElement(getTagName().cstr(), maAttrList);
}

void TagOpenElement::print() const
{ 
	TagElement::print(); 	
}

void TagOpenElement::addAttribute(const char *szAttributeName, const WPXString &sAttributeValue)
{
        maAttrList.insert(szAttributeName, sAttributeValue);
}

void TagCloseElement::write(DocumentHandler &xHandler) const
{
	WRITER_DEBUG_MSG(("TagCloseElement: write (%s)\n", getTagName().cstr()));

	xHandler.endElement(getTagName().cstr());
}

void CharDataElement::write(DocumentHandler &xHandler) const
{
	WRITER_DEBUG_MSG(("TextElement: write\n"));
	xHandler.characters(msData);
}

TextElement::TextElement(const WPXString & sTextBuf) :
	msTextBuf(sTextBuf, false)
{
}

// write: writes a text run, appropriately converting spaces to <text:s>
// elements
void TextElement::write(DocumentHandler &xHandler) const
{
	WPXPropertyList xBlankAttrList;
        
	WPXString sTemp;

	int iNumConsecutiveSpaces = 0;
        WPXString::Iter i(msTextBuf);
	for (i.rewind(); i.next();) 
        {
		if (*(i()) == ASCII_SPACE)
			iNumConsecutiveSpaces++;
		else
			iNumConsecutiveSpaces = 0;

		if (iNumConsecutiveSpaces > 1) {
			if (sTemp.len() > 0) {
				xHandler.characters(sTemp);
				sTemp.clear();
			}
			xHandler.startElement("text:s", xBlankAttrList);
			xHandler.endElement("text:s");
		}
		else {
                        sTemp.append(i());
		}
	}
	xHandler.characters(sTemp);
}
