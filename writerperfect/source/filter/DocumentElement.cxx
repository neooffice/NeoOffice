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


#include "DocumentElement.hxx"
#include "FilterInternal.hxx"
#include <string.h>

#ifndef _COM_SUN_STAR_XML_SAX_XATTRIBUTELIST_HPP_
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#endif

using rtl::OUString;
using namespace ::rtl;

using com::sun::star::xml::sax::XAttributeList;

#define UCS_SPACE 0x0020

void TagElement::print() const
{
	WRITER_DEBUG_MSG(("%s\n", msTagName.getUTF8()));
}

void TagOpenElement::write(Reference < XDocumentHandler > &xHandler) const
{
	WRITER_DEBUG_MSG(("Writing startElement (%s)\n", getTagName().getUTF8()));
	
	SvXMLAttributeList * paAttrList = new SvXMLAttributeList(maAttrList);
	Reference < XAttributeList > xAttrList ( paAttrList );

	xHandler->startElement(OUString::createFromAscii(getTagName().getUTF8()), xAttrList);
	WRITER_DEBUG_MSG(("Done\n"));
}

void TagOpenElement::print() const
{ 
	TagElement::print(); 	
}

void TagOpenElement::addAttribute(const char *szAttributeName, const char *szAttributeValue)
{
	maAttrList.AddAttribute(OUString::createFromAscii(szAttributeName), 
				 OUString::createFromAscii(szAttributeValue));
}

void TagCloseElement::write(Reference < XDocumentHandler > &xHandler) const
{
	WRITER_DEBUG_MSG(("TagCloseElement: write (%s)\n", getTagName().getUTF8()));

	xHandler->endElement(OUString::createFromAscii(getTagName().getUTF8()));
}

void CharDataElement::write(Reference < XDocumentHandler > &xHandler) const
{
	WRITER_DEBUG_MSG(("TextElement: write\n"));
	xHandler->characters(OUString::createFromAscii(msData.getUTF8()) );
}

TextElement::TextElement(const UCSString & sTextBuf) :
	msTextBuf(sTextBuf)
{
}

// write: writes a text run, appropriately converting spaces to <text:s>
// elements
// FIXME: this function is appalling because OUString isn't rich enough.
// probably should allocate some resizable buffer of UCS2 instead
void TextElement::write(Reference < XDocumentHandler > &xHandler) const
{
	WRITER_DEBUG_MSG(("TextElement: write\n"));
	SvXMLAttributeList * pAttrList = new SvXMLAttributeList;
	Reference < XAttributeList > xBlankAttrList ( pAttrList );

	OUString sTempUCS2;
	int iNumConsecutiveSpaces = 0;
	for (int i=0; i<msTextBuf.getLen(); i++) {
		if (msTextBuf.getUCS4()[i] == UCS_SPACE)
			iNumConsecutiveSpaces++;
		else
			iNumConsecutiveSpaces = 0;

		if (iNumConsecutiveSpaces > 1) {
			if (sTempUCS2.getLength() > 0) {
				xHandler->characters(sTempUCS2);
				sTempUCS2 = OUString::createFromAscii("");
			}
			xHandler->startElement(OUString::createFromAscii("text:s"), xBlankAttrList);
			xHandler->endElement(OUString::createFromAscii("text:s"));
		}
		else {
			const uint32_t * ucs4 = msTextBuf.getUCS4();
			sal_Unicode su = static_cast<sal_Unicode>(msTextBuf.getUCS4()[i]); 
			OUString aStringPart(&su,1);
			sTempUCS2 += aStringPart;
		}
	}
	xHandler->characters(sTempUCS2);
}
