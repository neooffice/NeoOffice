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


#ifndef _LISTSTYLE_H
#define _LISTSTYLE_H
#include <libwpd/libwpd.h>

#define WP6_NUM_LIST_LEVELS 8 // see WP6FileStructure.h (we shouldn't need to reference this)

#include "Style.hxx"
#include "WriterProperties.hxx"

#ifndef _COM_SUN_STAR_XML_SAX_XDOCUMENTHANDLER_HPP_
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#endif

using com::sun::star::uno::Reference;
using com::sun::star::xml::sax::XDocumentHandler;

class DocumentElement;

class ListLevelStyle
{
public:
	virtual void write(Reference < XDocumentHandler > &xHandler, int level) const = 0;
};

class OrderedListLevelStyle : public ListLevelStyle
{
public:
	OrderedListLevelStyle(const WPXNumberingType listType, 
			      const UCSString &sTextBeforeNumber, const UCSString &sTextAfterNumber, 
			      const float fSpaceBefore, const int iStartingNumber);
	virtual void write(Reference < XDocumentHandler > &xHandler, int level) const;
private:
	UCSString msTextBeforeNumber;
	UCSString msTextAfterNumber;
	float mfSpaceBefore;
	int miStartingNumber;
	WPXNumberingType mlistType;
};

class UnorderedListLevelStyle : public ListLevelStyle
{
public:
	UnorderedListLevelStyle(const UCSString &sBullet, const float fSpaceBefore);
	virtual void write(Reference < XDocumentHandler > &xHandler, int iLevel) const;
private:
	UCSString msBullet;
	float mfSpaceBefore;
};

class ListStyle : public Style
{
public:
	ListStyle(const char *psName, const int iListID);
	virtual ~ListStyle();
	virtual void write(Reference < XDocumentHandler > &xHandler) const;
	const int getListID() { return miListID; }
	const bool isListLevelDefined(int iLevel) const;

protected:
	void setListLevel(int iLevel, ListLevelStyle *iListLevelStyle);

private:
	ListLevelStyle *mppListLevels[WP6_NUM_LIST_LEVELS];
	int miNumListLevels;
	const int miListID;
};

class OrderedListStyle : public ListStyle
{
public:
	OrderedListStyle(const char *psName, const int iListID) : ListStyle(psName, iListID) {}
	void updateListLevel(const int iLevel, const WPXNumberingType listType, 
			     const UCSString &sTextBeforeNumber, const UCSString &sTextAfterNumber,
			     const int iStartingNumber);
};

class UnorderedListStyle : public ListStyle
{
public:
	UnorderedListStyle(const char *psName, const int iListID) : ListStyle(psName, iListID) {}
	void updateListLevel(const int iLevel, const UCSString &sBullet);
};
#endif
