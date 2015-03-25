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
 * Modified March 2013 by Patrick Luby. NeoOffice is distributed under
 * GPL only under modification term 2 of the LGPL.
 *
 ************************************************************************/

#ifndef _MANIFEST_IMPORT_HXX
#define _MANIFEST_IMPORT_HXX

#include <cppuhelper/implbase1.hxx> // helper for implementations
#ifndef _COM_SUN_STAR_XML_SAX_XDUCUMENTHANDLER_HPP_
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#endif
#include <vector>
#include <stack>

namespace com { namespace sun { namespace star { 
	namespace xml { namespace sax { class XAttributeList; } }
	namespace beans { struct PropertyValue; }
} } }

enum ElementNames
{
	e_Manifest,
	e_FileEntry,
	e_EncryptionData,
	e_Algorithm,
	e_KeyDerivation
#ifndef NO_OOO_3_4_1_AES_ENCRYPTION
	, e_StartKeyAlgorithm
#endif	// !NO_OOO_3_4_1_AES_ENCRYPTION
};
class ManifestImport : public cppu::WeakImplHelper1 < com::sun::star::xml::sax::XDocumentHandler >
{
protected:
	com::sun::star::uno::Sequence < com::sun::star::beans::PropertyValue > aSequence;
	sal_Int16 		nNumProperty;
	::std::stack < ElementNames > aStack;
#ifndef NO_OOO_3_4_1_AES_ENCRYPTION
	sal_Int32 nDerivedKeySize;
#endif	// !NO_OOO_3_4_1_AES_ENCRYPTION
	sal_Bool bIgnoreEncryptData;
	::std::vector < ::com::sun::star::uno::Sequence < ::com::sun::star::beans::PropertyValue > > & rManVector;

	const ::rtl::OUString sFileEntryElement;
	const ::rtl::OUString sManifestElement;
	const ::rtl::OUString sEncryptionDataElement;
	const ::rtl::OUString sAlgorithmElement;
	const ::rtl::OUString sKeyDerivationElement;
#ifndef NO_OOO_3_4_1_AES_ENCRYPTION
	const ::rtl::OUString sStartKeyAlgorithmElement;
#endif	// !NO_OOO_3_4_1_AES_ENCRYPTION

	const ::rtl::OUString sCdataAttribute;
	const ::rtl::OUString sMediaTypeAttribute;
	const ::rtl::OUString sVersionAttribute;
	const ::rtl::OUString sFullPathAttribute;
	const ::rtl::OUString sSizeAttribute;
	const ::rtl::OUString sSaltAttribute;
	const ::rtl::OUString sInitialisationVectorAttribute;
	const ::rtl::OUString sIterationCountAttribute;
	const ::rtl::OUString sAlgorithmNameAttribute;
	const ::rtl::OUString sKeyDerivationNameAttribute;
	const ::rtl::OUString sChecksumAttribute;
	const ::rtl::OUString sChecksumTypeAttribute;

	const ::rtl::OUString sFullPathProperty;
	const ::rtl::OUString sMediaTypeProperty;
	const ::rtl::OUString sVersionProperty;
	const ::rtl::OUString sIterationCountProperty;
	const ::rtl::OUString sSaltProperty;
	const ::rtl::OUString sInitialisationVectorProperty;
	const ::rtl::OUString sSizeProperty;
	const ::rtl::OUString sDigestProperty;
#ifndef NO_OOO_3_4_1_AES_ENCRYPTION
	const ::rtl::OUString sDigestTypeProperty;
	const ::rtl::OUString sAlgorithmProperty;
	const ::rtl::OUString sDerivedKeySizeProperty;
	const ::rtl::OUString sStartKeyAlgorithmProperty;
#endif	// !NO_OOO_3_4_1_AES_ENCRYPTION

	const ::rtl::OUString sWhiteSpace;
	const ::rtl::OUString sBlowfish;
	const ::rtl::OUString sPBKDF2;
#ifdef NO_OOO_3_4_1_AES_ENCRYPTION
	const ::rtl::OUString sChecksumType;
#endif	// NO_OOO_3_4_1_AES_ENCRYPTION

public:
	ManifestImport( std::vector < ::com::sun::star::uno::Sequence < ::com::sun::star::beans::PropertyValue > > & rNewVector );
	~ManifestImport( void );
    virtual void SAL_CALL startDocument(  ) 	
		throw(::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL endDocument(  ) 	
		throw(::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL startElement( const ::rtl::OUString& aName, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttribs ) 	
		throw(::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL endElement( const ::rtl::OUString& aName ) 	
		throw(::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL characters( const ::rtl::OUString& aChars ) 	
		throw(::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL ignorableWhitespace( const ::rtl::OUString& aWhitespaces ) 	
		throw(::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL processingInstruction( const ::rtl::OUString& aTarget, const ::rtl::OUString& aData ) 	
		throw(::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setDocumentLocator( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XLocator >& xLocator ) 	
		throw(::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
};
#endif
