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
 */
#include <stdlib.h>
#include <string.h>
#include <sal/alloca.h>
#include <cassert>
#include <vector>

#include <osl/diagnose.h>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/xml/sax/XExtendedDocumentHandler.hpp>
#include <com/sun/star/xml/sax/XParser.hpp>
#include <com/sun/star/xml/sax/SAXParseException.hpp>
#include <com/sun/star/io/XSeekable.hpp>

#include <cppuhelper/weak.hxx>
#include <cppuhelper/implbase3.hxx>
#if SUPD == 310
#include <cppuhelper/factory.hxx>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <sal/log.hxx>
#else	// SUPD == 310
#include <cppuhelper/supportsservice.hxx>
#endif	// SUPD == 310
#include <rtl/ref.hxx>

#include <expat.h>

using namespace ::std;
using namespace ::osl;
using namespace ::cppu;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::io;

#if SUPD == 310
#include "factory.hxx"
#endif	// SUPD == 310
#include "attrlistimpl.hxx"
#include "xml2utf.hxx"

namespace com { namespace sun { namespace star { namespace uno {
    class XComponentContext;
} } } }

namespace {

// Useful macros for correct String conversion depending on the chosen expat-mode
#ifdef XML_UNICODE
OUString XmlNChar2OUString( const XML_Char *p , int nLen )
{
    if( p ) {
        if( sizeof( sal_Unicode ) == sizeof( XML_Char ) )
        {
            return OUString( (sal_Unicode*)p,nLen);
        }
        else
        {
            sal_Unicode *pWchar = (sal_Unicode *)alloca( sizeof( sal_Unicode ) * nLen );
            for( int n = 0 ; n < nLen ; n++ ) {
                pWchar[n] = (sal_Unicode) p[n];
            }
            return OUString( pWchar , nLen );
        }
    }
    else {
        return OUString();
    }
}

OUString XmlChar2OUString( const XML_Char *p )
{
    if( p ) {
        int nLen;
        for( nLen = 0 ; p[nLen] ; nLen ++ )
            ;
         return XmlNChar2OUString( p , nLen );
     }
     else return OUString();
}


#define XML_CHAR_TO_OUSTRING(x) XmlChar2OUString(x)
#define XML_CHAR_N_TO_USTRING(x,n) XmlNChar2OUString(x,n)
#else
#define XML_CHAR_TO_OUSTRING(x) OUString(x , strlen( x ), RTL_TEXTENCODING_UTF8)
#define XML_CHAR_N_TO_USTRING(x,n) OUString(x,n, RTL_TEXTENCODING_UTF8 )
#endif


/*
* The following macro encapsulates any call to an event handler.
* It ensures, that exceptions thrown by the event handler are
* treated properly.
*/
#define CALL_ELEMENT_HANDLER_AND_CARE_FOR_EXCEPTIONS(pThis,call) \
    if( ! pThis->bExceptionWasThrown ) { \
        try {\
            pThis->call;\
        }\
        catch( const SAXParseException &e ) {\
            pThis->callErrorHandler( pThis ,  e );\
         }\
        catch( const SAXException &e ) {\
            pThis->callErrorHandler( pThis , SAXParseException(\
                                            e.Message, \
                                            e.Context, \
                                            e.WrappedException,\
                                            pThis->rDocumentLocator->getPublicId(),\
                                            pThis->rDocumentLocator->getSystemId(),\
                                            pThis->rDocumentLocator->getLineNumber(),\
                                            pThis->rDocumentLocator->getColumnNumber()\
                                     ) );\
        }\
        catch( const com::sun::star::uno::RuntimeException &e ) {\
            pThis->bExceptionWasThrown = true; \
            pThis->bRTExceptionWasThrown = true; \
            pImpl->rtexception = e; \
        }\
    }\
    ((void)0)


#if SUPD == 310
#define IMPLEMENTATION_NAME	"com.sun.star.comp.extensions.xml.sax.ParserExpat"
#define SERVICE_NAME		"com.sun.star.xml.sax.Parser"
#endif	// SUPD == 310

class SaxExpatParser_Impl;

// This class implements the external Parser interface
class SaxExpatParser
    : public WeakImplHelper3< XInitialization
                            , XServiceInfo
                            , XParser >
{

public:
    SaxExpatParser();
    virtual ~SaxExpatParser();

#if SUPD == 310
    static css::uno::Sequence< OUString > 	getSupportedServiceNames_Static(void) throw ();
#endif	// SUPD == 310

    // ::com::sun::star::lang::XInitialization:
    virtual void SAL_CALL initialize(css::uno::Sequence<css::uno::Any> const& rArguments)
#if SUPD == 310
        throw (css::uno::RuntimeException, css::uno::Exception) SAL_OVERRIDE;
#else	// SUPD == 310
        throw (css::uno::RuntimeException, css::uno::Exception, std::exception) SAL_OVERRIDE;
#endif	// SUPD == 310

    // The SAX-Parser-Interface
    virtual void SAL_CALL parseStream(  const InputSource& structSource)
        throw ( SAXException,
                IOException,
#if SUPD == 310
                css::uno::RuntimeException) SAL_OVERRIDE;
#else	// SUPD == 310
                css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
#endif	// SUPD == 310
    virtual void SAL_CALL setDocumentHandler(const css::uno::Reference< XDocumentHandler > & xHandler)
#if SUPD == 310
        throw (css::uno::RuntimeException) SAL_OVERRIDE;
#else	// SUPD == 310
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
#endif	// SUPD == 310

    virtual void SAL_CALL setErrorHandler(const css::uno::Reference< XErrorHandler > & xHandler)
#if SUPD == 310
        throw (css::uno::RuntimeException) SAL_OVERRIDE;
#else	// SUPD == 310
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
#endif	// SUPD == 310
    virtual void SAL_CALL setDTDHandler(const css::uno::Reference < XDTDHandler > & xHandler)
#if SUPD == 310
        throw (css::uno::RuntimeException) SAL_OVERRIDE;
#else	// SUPD == 310
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
#endif	// SUPD == 310
    virtual void SAL_CALL setEntityResolver(const css::uno::Reference<  XEntityResolver >& xResolver)
#if SUPD == 310
        throw (css::uno::RuntimeException) SAL_OVERRIDE;
#else	// SUPD == 310
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
#endif	// SUPD == 310

#if SUPD == 310
    virtual void SAL_CALL setLocale( const Locale &locale )                     throw (css::uno::RuntimeException) SAL_OVERRIDE;
#else	// SUPD == 310
    virtual void SAL_CALL setLocale( const Locale &locale )                     throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
#endif	// SUPD == 310

public: // XServiceInfo
#if SUPD == 310
    OUString                     SAL_CALL getImplementationName() throw () SAL_OVERRIDE;
    css::uno::Sequence< OUString >         SAL_CALL getSupportedServiceNames(void) throw () SAL_OVERRIDE;
    sal_Bool                     SAL_CALL supportsService(const OUString& ServiceName) throw () SAL_OVERRIDE;
#else	// SUPD == 310
    OUString                     SAL_CALL getImplementationName() throw (std::exception) SAL_OVERRIDE;
    css::uno::Sequence< OUString >         SAL_CALL getSupportedServiceNames(void) throw (std::exception) SAL_OVERRIDE;
    sal_Bool                     SAL_CALL supportsService(const OUString& ServiceName) throw (std::exception) SAL_OVERRIDE;
#endif	// SUPD == 310

private:

    SaxExpatParser_Impl         *m_pImpl;

};

#if SUPD == 310

css::uno::Reference< css::uno::XInterface > SAL_CALL SaxExpatParser_CreateInstance(
	const css::uno::Reference< XMultiServiceFactory  >  & ) throw(css::uno::Exception)
{	
	SaxExpatParser *p = new SaxExpatParser;

	return css::uno::Reference< css::uno::XInterface > ( (OWeakObject * ) p );
}

css::uno::Sequence< OUString > 	SaxExpatParser::getSupportedServiceNames_Static(void) throw ()
{
	css::uno::Sequence<OUString> aRet(1);
	aRet.getArray()[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(SERVICE_NAME) );
	return aRet;
}

#endif	// SUPD == 310


// Entity binds all information neede for a single file
struct Entity
{
    InputSource         structSource;
    XML_Parser          pParser;
    sax_expatwrap::XMLFile2UTFConverter converter;
};


class SaxExpatParser_Impl
{
public: // module scope
    Mutex               aMutex;
    OUString            sCDATA;
    bool m_bEnableDoS; // fdo#60471 thank you Adobe Illustrator

    css::uno::Reference< XDocumentHandler >   rDocumentHandler;
    css::uno::Reference< XExtendedDocumentHandler > rExtendedDocumentHandler;

    css::uno::Reference< XErrorHandler >  rErrorHandler;
    css::uno::Reference< XDTDHandler >    rDTDHandler;
    css::uno::Reference< XEntityResolver > rEntityResolver;
    css::uno::Reference < XLocator >      rDocumentLocator;


    rtl::Reference < sax_expatwrap::AttributeList > rAttrList;

    // External entity stack
    vector<struct Entity>   vecEntity;
    void pushEntity( const struct Entity &entity )
        { vecEntity.push_back( entity ); }
    void popEntity()
        { vecEntity.pop_back( ); }
    struct Entity &getEntity()
        { return vecEntity.back(); }


    // Exception cannot be thrown through the C-XmlParser (possible resource leaks),
    // therefor the exception must be saved somewhere.
    SAXParseException   exception;
    css::uno::RuntimeException    rtexception;
    bool                bExceptionWasThrown;
    bool                bRTExceptionWasThrown;

    Locale              locale;

public:
    SaxExpatParser_Impl()
        : sCDATA("CDATA")
        , m_bEnableDoS(false)
        , bExceptionWasThrown(false)
        , bRTExceptionWasThrown(false)
    {
    }

    // the C-Callbacks for the expat parser
    void static callbackStartElement(void *userData, const XML_Char *name , const XML_Char **atts);
    void static callbackEndElement(void *userData, const XML_Char *name);
    void static callbackCharacters( void *userData , const XML_Char *s , int nLen );
    void static callbackProcessingInstruction(  void *userData ,
                                                const XML_Char *sTarget ,
                                                const XML_Char *sData );

    void static callbackEntityDecl( void *userData ,
                                    const XML_Char *entityName,
                                    int is_parameter_entity,
                                    const XML_Char *value,
                                    int value_length,
                                    const XML_Char *base,
                                    const XML_Char *systemId,
                                    const XML_Char *publicId,
                                    const XML_Char *notationName);

    void static callbackNotationDecl(   void *userData,
                                        const XML_Char *notationName,
                                        const XML_Char *base,
                                        const XML_Char *systemId,
                                        const XML_Char *publicId);

    bool static callbackExternalEntityRef(   XML_Parser parser,
                                            const XML_Char *openEntityNames,
                                            const XML_Char *base,
                                            const XML_Char *systemId,
                                            const XML_Char *publicId);

    int static callbackUnknownEncoding(void *encodingHandlerData,
                                                  const XML_Char *name,
                                                  XML_Encoding *info);

    void static callbackDefault( void *userData,  const XML_Char *s,  int len);

    void static callbackStartCDATA( void *userData );
    void static callbackEndCDATA( void *userData );
    void static callbackComment( void *userData , const XML_Char *s );
    void static callErrorHandler( SaxExpatParser_Impl *pImpl , const SAXParseException &e );

public:
    void parse();
};

extern "C"
{
    static void call_callbackStartElement(void *userData, const XML_Char *name , const XML_Char **atts)
    {
        SaxExpatParser_Impl::callbackStartElement(userData,name,atts);
    }
    static void call_callbackEndElement(void *userData, const XML_Char *name)
    {
        SaxExpatParser_Impl::callbackEndElement(userData,name);
    }
    static void call_callbackCharacters( void *userData , const XML_Char *s , int nLen )
    {
        SaxExpatParser_Impl::callbackCharacters(userData,s,nLen);
    }
    static void call_callbackProcessingInstruction(void *userData,const XML_Char *sTarget,const XML_Char *sData )
    {
        SaxExpatParser_Impl::callbackProcessingInstruction(userData,sTarget,sData );
    }
    static void call_callbackEntityDecl(void *userData ,
                                        const XML_Char *entityName,
                                        int is_parameter_entity,
                                        const XML_Char *value,
                                        int value_length,
                                        const XML_Char *base,
                                        const XML_Char *systemId,
                                        const XML_Char *publicId,
                                        const XML_Char *notationName)
    {
        SaxExpatParser_Impl::callbackEntityDecl(userData, entityName,
                is_parameter_entity, value, value_length,
                base, systemId, publicId, notationName);
    }
    static void call_callbackNotationDecl(void *userData,
                                          const XML_Char *notationName,
                                          const XML_Char *base,
                                          const XML_Char *systemId,
                                          const XML_Char *publicId)
    {
        SaxExpatParser_Impl::callbackNotationDecl(userData,notationName,base,systemId,publicId);
    }
    static int call_callbackExternalEntityRef(XML_Parser parser,
                                              const XML_Char *openEntityNames,
                                              const XML_Char *base,
                                              const XML_Char *systemId,
                                              const XML_Char *publicId)
    {
        return SaxExpatParser_Impl::callbackExternalEntityRef(parser,openEntityNames,base,systemId,publicId);
    }
    static int call_callbackUnknownEncoding(void *encodingHandlerData,
                                              const XML_Char *name,
                                            XML_Encoding *info)
    {
        return SaxExpatParser_Impl::callbackUnknownEncoding(encodingHandlerData,name,info);
    }
    static void call_callbackDefault( void *userData,  const XML_Char *s,  int len)
    {
        SaxExpatParser_Impl::callbackDefault(userData,s,len);
    }
    static void call_callbackStartCDATA( void *userData )
    {
        SaxExpatParser_Impl::callbackStartCDATA(userData);
    }
    static void call_callbackEndCDATA( void *userData )
    {
        SaxExpatParser_Impl::callbackEndCDATA(userData);
    }
    static void call_callbackComment( void *userData , const XML_Char *s )
    {
        SaxExpatParser_Impl::callbackComment(userData,s);
    }
}



// LocatorImpl

class LocatorImpl :
    public WeakImplHelper2< XLocator, com::sun::star::io::XSeekable >
    // should use a different interface for stream positions!
{
public:
    LocatorImpl( SaxExpatParser_Impl *p )
    {
        m_pParser    = p;
    }

public: //XLocator
#if SUPD == 310
    virtual sal_Int32 SAL_CALL getColumnNumber(void) throw (css::uno::RuntimeException) SAL_OVERRIDE
#else	// SUPD == 310
    virtual sal_Int32 SAL_CALL getColumnNumber(void) throw (std::exception) SAL_OVERRIDE
#endif	// SUPD == 310
    {
        return XML_GetCurrentColumnNumber( m_pParser->getEntity().pParser );
    }
#if SUPD == 310
    virtual sal_Int32 SAL_CALL getLineNumber(void) throw (css::uno::RuntimeException) SAL_OVERRIDE
#else	// SUPD == 310
    virtual sal_Int32 SAL_CALL getLineNumber(void) throw (std::exception) SAL_OVERRIDE
#endif	// SUPD == 310
    {
        return XML_GetCurrentLineNumber( m_pParser->getEntity().pParser );
    }
#if SUPD == 310
    virtual OUString SAL_CALL getPublicId(void) throw (css::uno::RuntimeException) SAL_OVERRIDE
#else	// SUPD == 310
    virtual OUString SAL_CALL getPublicId(void) throw (std::exception) SAL_OVERRIDE
#endif	// SUPD == 310
    {
        return m_pParser->getEntity().structSource.sPublicId;
    }
#if SUPD == 310
    virtual OUString SAL_CALL getSystemId(void) throw (css::uno::RuntimeException) SAL_OVERRIDE
#else	// SUPD == 310
    virtual OUString SAL_CALL getSystemId(void) throw (std::exception) SAL_OVERRIDE
#endif	// SUPD == 310
    {
        return m_pParser->getEntity().structSource.sSystemId;
    }

    // XSeekable (only for getPosition)

#if SUPD == 310
    virtual void SAL_CALL seek( sal_Int64 ) throw(css::uno::RuntimeException) SAL_OVERRIDE
#else	// SUPD == 310
    virtual void SAL_CALL seek( sal_Int64 ) throw(std::exception) SAL_OVERRIDE
#endif	// SUPD == 310
    {
    }
#if SUPD == 310
    virtual sal_Int64 SAL_CALL getPosition() throw(css::uno::RuntimeException) SAL_OVERRIDE
#else	// SUPD == 310
    virtual sal_Int64 SAL_CALL getPosition() throw(std::exception) SAL_OVERRIDE
#endif	// SUPD == 310
    {
        return XML_GetCurrentByteIndex( m_pParser->getEntity().pParser );
    }
#if SUPD == 310
    virtual ::sal_Int64 SAL_CALL getLength() throw(css::uno::RuntimeException) SAL_OVERRIDE
#else	// SUPD == 310
    virtual ::sal_Int64 SAL_CALL getLength() throw(std::exception) SAL_OVERRIDE
#endif	// SUPD == 310
    {
        return 0;
    }

private:

    SaxExpatParser_Impl *m_pParser;
};




SaxExpatParser::SaxExpatParser(  )
{
    m_pImpl = new SaxExpatParser_Impl;

    LocatorImpl *pLoc = new LocatorImpl( m_pImpl );
    m_pImpl->rDocumentLocator = css::uno::Reference< XLocator > ( pLoc );

    // Performance-improvement; handing out the same object with every call of
    // the startElement callback is allowed (see sax-specification):
    m_pImpl->rAttrList = new sax_expatwrap::AttributeList;

    m_pImpl->bExceptionWasThrown = false;
    m_pImpl->bRTExceptionWasThrown = false;
}

SaxExpatParser::~SaxExpatParser()
{
    delete m_pImpl;
}

// ::com::sun::star::lang::XInitialization:
void SAL_CALL
SaxExpatParser::initialize(css::uno::Sequence< css::uno::Any > const& rArguments)
#if SUPD == 310
    throw (css::uno::RuntimeException, css::uno::Exception)
#else	// SUPD == 310
    throw (css::uno::RuntimeException, css::uno::Exception, std::exception)
#endif	// SUPD == 310
{
    // possible arguments: a string "DoSmeplease"
    if (rArguments.getLength())
    {
        OUString str;
        if ((rArguments[0] >>= str) && OUString(RTL_CONSTASCII_USTRINGPARAM("DoSmeplease")) == str)
        {
            MutexGuard guard( m_pImpl->aMutex );
            m_pImpl->m_bEnableDoS = true;
        }
    }
}

/***************
*
* parseStream does Parser-startup initializations. The SaxExpatParser_Impl::parse() method does
* the file-specific initialization work. (During a parser run, external files may be opened)
*
****************/
void SaxExpatParser::parseStream(   const InputSource& structSource)
    throw (SAXException,
           IOException,
#if SUPD == 310
           css::uno::RuntimeException)
#else	// SUPD == 310
           css::uno::RuntimeException, std::exception)
#endif	// SUPD == 310
{
    // Only one text at one time
    MutexGuard guard( m_pImpl->aMutex );


    struct Entity entity;
    entity.structSource = structSource;

    if( ! entity.structSource.aInputStream.is() )
    {
        throw SAXException("No input source",
                            css::uno::Reference< css::uno::XInterface > () , css::uno::Any() );
    }

    entity.converter.setInputStream( entity.structSource.aInputStream );
    if( !entity.structSource.sEncoding.isEmpty() )
    {
        entity.converter.setEncoding(
            OUStringToOString( entity.structSource.sEncoding , RTL_TEXTENCODING_ASCII_US ) );
    }

    // create parser with proper encoding
    entity.pParser = XML_ParserCreate( 0 );
    if( ! entity.pParser )
    {
        throw SAXException("Couldn't create parser",
                            css::uno::Reference< css::uno::XInterface > (), css::uno::Any() );
    }

    // set all necessary C-Callbacks
    XML_SetUserData( entity.pParser , m_pImpl );
    XML_SetElementHandler(  entity.pParser ,
                            call_callbackStartElement ,
                            call_callbackEndElement );
    XML_SetCharacterDataHandler( entity.pParser , call_callbackCharacters );
    XML_SetProcessingInstructionHandler(entity.pParser ,
                                        call_callbackProcessingInstruction );
    if (!m_pImpl->m_bEnableDoS)
    {
        XML_SetEntityDeclHandler(entity.pParser, call_callbackEntityDecl);
    }
    XML_SetNotationDeclHandler( entity.pParser, call_callbackNotationDecl );
    XML_SetExternalEntityRefHandler(    entity.pParser,
                                        call_callbackExternalEntityRef);
    XML_SetUnknownEncodingHandler( entity.pParser,  call_callbackUnknownEncoding ,0);

    if( m_pImpl->rExtendedDocumentHandler.is() ) {

        // These handlers just delegate calls to the ExtendedHandler. If no extended handler is
        // given, these callbacks can be ignored
        XML_SetDefaultHandlerExpand( entity.pParser, call_callbackDefault );
        XML_SetCommentHandler( entity.pParser, call_callbackComment );
        XML_SetCdataSectionHandler(     entity.pParser ,
                                        call_callbackStartCDATA ,
                                         call_callbackEndCDATA );
    }


    m_pImpl->exception = SAXParseException();
    m_pImpl->pushEntity( entity );
    try
    {
        // start the document
        if( m_pImpl->rDocumentHandler.is() ) {
            m_pImpl->rDocumentHandler->setDocumentLocator( m_pImpl->rDocumentLocator );
            m_pImpl->rDocumentHandler->startDocument();
        }

        m_pImpl->parse();

        // finish document
        if( m_pImpl->rDocumentHandler.is() ) {
            m_pImpl->rDocumentHandler->endDocument();
        }
    }
//      catch( SAXParseException &e )
//  {
//      m_pImpl->popEntity();
//          XML_ParserFree( entity.pParser );
//        css::uno::Any aAny;
//        aAny <<= e;
//          throw SAXException( e.Message, e.Context, aAny );
//      }
    catch( SAXException & )
    {
        m_pImpl->popEntity();
        XML_ParserFree( entity.pParser );
          throw;
    }
    catch( IOException & )
    {
        m_pImpl->popEntity();
        XML_ParserFree( entity.pParser );
        throw;
    }
    catch( css::uno::RuntimeException & )
    {
        m_pImpl->popEntity();
        XML_ParserFree( entity.pParser );
        throw;
    }

    m_pImpl->popEntity();
    XML_ParserFree( entity.pParser );
}

void SaxExpatParser::setDocumentHandler(const css::uno::Reference< XDocumentHandler > & xHandler)
#if SUPD == 310
    throw (css::uno::RuntimeException)
#else	// SUPD == 310
    throw (css::uno::RuntimeException, std::exception)
#endif	// SUPD == 310
{
    m_pImpl->rDocumentHandler = xHandler;
    m_pImpl->rExtendedDocumentHandler =
        css::uno::Reference< XExtendedDocumentHandler >( xHandler , css::uno::UNO_QUERY );
}

void SaxExpatParser::setErrorHandler(const css::uno::Reference< XErrorHandler > & xHandler)
#if SUPD == 310
    throw (css::uno::RuntimeException)
#else	// SUPD == 310
    throw (css::uno::RuntimeException, std::exception)
#endif	// SUPD == 310
{
    m_pImpl->rErrorHandler = xHandler;
}

void SaxExpatParser::setDTDHandler(const css::uno::Reference< XDTDHandler > & xHandler)
#if SUPD == 310
    throw (css::uno::RuntimeException)
#else	// SUPD == 310
    throw (css::uno::RuntimeException, std::exception)
#endif	// SUPD == 310
{
    m_pImpl->rDTDHandler = xHandler;
}

void SaxExpatParser::setEntityResolver(const css::uno::Reference < XEntityResolver > & xResolver)
#if SUPD == 310
    throw (css::uno::RuntimeException)
#else	// SUPD == 310
    throw (css::uno::RuntimeException, std::exception)
#endif	// SUPD == 310
{
    m_pImpl->rEntityResolver = xResolver;
}


#if SUPD == 310
void SaxExpatParser::setLocale( const Locale & locale ) throw (css::uno::RuntimeException)
#else	// SUPD == 310
void SaxExpatParser::setLocale( const Locale & locale ) throw (css::uno::RuntimeException, std::exception)
#endif	// SUPD == 310
{
    m_pImpl->locale = locale;
}

// XServiceInfo
#if SUPD == 310
OUString SaxExpatParser::getImplementationName() throw ()
#else	// SUPD == 310
OUString SaxExpatParser::getImplementationName() throw (std::exception)
#endif	// SUPD == 310
{
#if SUPD == 310
    return OUString::createFromAscii( IMPLEMENTATION_NAME );
#else	// SUPD == 310
    return OUString("com.sun.star.comp.extensions.xml.sax.ParserExpat");
#endif	// SUPD == 310
}

// XServiceInfo
#if SUPD == 310
sal_Bool SaxExpatParser::supportsService(const OUString& ServiceName) throw ()
#else	// SUPD == 310
sal_Bool SaxExpatParser::supportsService(const OUString& ServiceName) throw (std::exception)
#endif	// SUPD == 310
{
#if SUPD == 310
    css::uno::Sequence< OUString > aSNL = getSupportedServiceNames();
    const OUString * pArray = aSNL.getConstArray();

    for( sal_Int32 i = 0; i < aSNL.getLength(); i++ )
        if( pArray[i] == ServiceName )
            return sal_True;

    return sal_False;
#else	// SUPD == 310
    return cppu::supportsService(this, ServiceName);
#endif	// SUPD == 310
}

// XServiceInfo
#if SUPD == 310
css::uno::Sequence< OUString > SaxExpatParser::getSupportedServiceNames(void) throw ()
#else	// SUPD == 310
css::uno::Sequence< OUString > SaxExpatParser::getSupportedServiceNames(void) throw (std::exception)
#endif	// SUPD == 310
{
    css::uno::Sequence<OUString> seq(1);
#if SUPD == 310
	seq[0] = OUString( RTL_CONSTASCII_USTRINGPARAM(SERVICE_NAME) );
#else	// SUPD == 310
    seq[0] = "com.sun.star.xml.sax.Parser";
#endif	// SUPD == 310
    return seq;
}


/*---------------------------------------
*
* Helper functions and classes
*
*
*-------------------------------------------*/
OUString getErrorMessage( XML_Error xmlE, const OUString& sSystemId , sal_Int32 nLine )
{
#if SUPD == 310
    const sal_Char *Message = "";
#else	// SUPD == 310
    OUString Message;
#endif	// SUPD == 310
    if( XML_ERROR_NONE == xmlE ) {
        Message = "No";
    }
    else if( XML_ERROR_NO_MEMORY == xmlE ) {
        Message = "no memory";
    }
    else if( XML_ERROR_SYNTAX == xmlE ) {
        Message = "syntax";
    }
    else if( XML_ERROR_NO_ELEMENTS == xmlE ) {
        Message = "no elements";
    }
    else if( XML_ERROR_INVALID_TOKEN == xmlE ) {
        Message = "invalid token";
    }
    else if( XML_ERROR_UNCLOSED_TOKEN == xmlE ) {
        Message = "unclosed token";
    }
    else if( XML_ERROR_PARTIAL_CHAR == xmlE ) {
        Message = "partial char";
    }
    else if( XML_ERROR_TAG_MISMATCH == xmlE ) {
        Message = "tag mismatch";
    }
    else if( XML_ERROR_DUPLICATE_ATTRIBUTE == xmlE ) {
        Message = "duplicate attribute";
    }
    else if( XML_ERROR_JUNK_AFTER_DOC_ELEMENT == xmlE ) {
        Message = "junk after doc element";
    }
    else if( XML_ERROR_PARAM_ENTITY_REF == xmlE ) {
        Message = "parameter entity reference";
    }
    else if( XML_ERROR_UNDEFINED_ENTITY == xmlE ) {
        Message = "undefined entity";
    }
    else if( XML_ERROR_RECURSIVE_ENTITY_REF == xmlE ) {
        Message = "recursive entity reference";
    }
    else if( XML_ERROR_ASYNC_ENTITY == xmlE ) {
        Message = "async entity";
    }
    else if( XML_ERROR_BAD_CHAR_REF == xmlE ) {
        Message = "bad char reference";
    }
    else if( XML_ERROR_BINARY_ENTITY_REF == xmlE ) {
        Message = "binary entity reference";
    }
    else if( XML_ERROR_ATTRIBUTE_EXTERNAL_ENTITY_REF == xmlE ) {
        Message = "attribute external entity reference";
    }
    else if( XML_ERROR_MISPLACED_XML_PI == xmlE ) {
        Message = "misplaced xml processing instruction";
    }
    else if( XML_ERROR_UNKNOWN_ENCODING == xmlE ) {
        Message = "unknown encoding";
    }
    else if( XML_ERROR_INCORRECT_ENCODING == xmlE ) {
        Message = "incorrect encoding";
    }
    else if( XML_ERROR_UNCLOSED_CDATA_SECTION == xmlE ) {
        Message = "unclosed cdata section";
    }
    else if( XML_ERROR_EXTERNAL_ENTITY_HANDLING == xmlE ) {
        Message = "external entity reference";
    }
    else if( XML_ERROR_NOT_STANDALONE == xmlE ) {
        Message = "not standalone";
    }

    OUString str("[");
    str += sSystemId;
    str += " line ";
#if SUPD == 310
    str += OUString::valueOf( nLine );
#else	// SUPD == 310
    str += OUString::number( nLine );
#endif	// SUPD == 310
    str += "]: ";
    str += Message;
    str += "error";

    return str;
}


// starts parsing with actual parser !
void SaxExpatParser_Impl::parse( )
{
    const int nBufSize = 16*1024;

    int nRead   = nBufSize;
    css::uno::Sequence< sal_Int8 > seqOut(nBufSize);

    while( nRead ) {
        nRead = getEntity().converter.readAndConvert( seqOut , nBufSize );

        if( ! nRead ) {
            XML_Parse( getEntity().pParser ,
                                   ( const char * ) seqOut.getArray() ,
                                   0 ,
                                   1 );
            break;
        }

        bool bContinue = ( XML_Parse( getEntity().pParser ,
                                                (const char *) seqOut.getArray(),
                                                nRead,
                                                0 ) != XML_STATUS_ERROR );

        if( ! bContinue || this->bExceptionWasThrown ) {

            if ( this->bRTExceptionWasThrown )
                throw rtexception;

            // Error during parsing !
            XML_Error xmlE = XML_GetErrorCode( getEntity().pParser );
            OUString sSystemId = rDocumentLocator->getSystemId();
            sal_Int32 nLine = rDocumentLocator->getLineNumber();

            SAXParseException aExcept(
                getErrorMessage(xmlE , sSystemId, nLine) ,
                css::uno::Reference< css::uno::XInterface >(),
                css::uno::Any( &exception , getCppuType( &exception) ),
                rDocumentLocator->getPublicId(),
                rDocumentLocator->getSystemId(),
                rDocumentLocator->getLineNumber(),
                rDocumentLocator->getColumnNumber()
                );

            if( rErrorHandler.is() ) {

                // error handler is set, so the handler may throw the exception
                css::uno::Any a;
                a <<= aExcept;
                rErrorHandler->fatalError( a );
            }

            // Error handler has not thrown an exception, but parsing cannot go on,
            // so an exception MUST be thrown.
            throw aExcept;
        } // if( ! bContinue )
    } // while
}



// The C-Callbacks


void SaxExpatParser_Impl::callbackStartElement( void *pvThis ,
                                                const XML_Char *pwName ,
                                                const XML_Char **awAttributes )
{
    SaxExpatParser_Impl *pImpl = ((SaxExpatParser_Impl*)pvThis);

    if( pImpl->rDocumentHandler.is() ) {

        int i = 0;
        pImpl->rAttrList->clear();

        while( awAttributes[i] ) {
            assert(awAttributes[i+1]);
            pImpl->rAttrList->addAttribute(
                XML_CHAR_TO_OUSTRING( awAttributes[i] ) ,
                pImpl->sCDATA,  // expat doesn't know types
                XML_CHAR_TO_OUSTRING( awAttributes[i+1] ) );
            i +=2;
        }

        CALL_ELEMENT_HANDLER_AND_CARE_FOR_EXCEPTIONS(
            pImpl ,
            rDocumentHandler->startElement( XML_CHAR_TO_OUSTRING( pwName ) ,
                                            pImpl->rAttrList.get() ) );
    }
}

void SaxExpatParser_Impl::callbackEndElement( void *pvThis , const XML_Char *pwName  )
{
    SaxExpatParser_Impl  *pImpl = ((SaxExpatParser_Impl*)pvThis);

    if( pImpl->rDocumentHandler.is() ) {
        CALL_ELEMENT_HANDLER_AND_CARE_FOR_EXCEPTIONS( pImpl,
                rDocumentHandler->endElement( XML_CHAR_TO_OUSTRING( pwName ) ) );
    }
}


void SaxExpatParser_Impl::callbackCharacters( void *pvThis , const XML_Char *s , int nLen )
{
    SaxExpatParser_Impl *pImpl = ((SaxExpatParser_Impl*)pvThis);

    if( pImpl->rDocumentHandler.is() ) {
        CALL_ELEMENT_HANDLER_AND_CARE_FOR_EXCEPTIONS( pImpl ,
                rDocumentHandler->characters( XML_CHAR_N_TO_USTRING(s,nLen) ) );
    }
}

void SaxExpatParser_Impl::callbackProcessingInstruction(    void *pvThis,
                                                    const XML_Char *sTarget ,
                                                    const XML_Char *sData )
{
    SaxExpatParser_Impl *pImpl = ((SaxExpatParser_Impl*)pvThis);
    if( pImpl->rDocumentHandler.is() ) {
        CALL_ELEMENT_HANDLER_AND_CARE_FOR_EXCEPTIONS(
                    pImpl ,
                    rDocumentHandler->processingInstruction( XML_CHAR_TO_OUSTRING( sTarget ),
                    XML_CHAR_TO_OUSTRING( sData ) ) );
    }
}


void SaxExpatParser_Impl::callbackEntityDecl(
    void *pvThis, const XML_Char *entityName,
    SAL_UNUSED_PARAMETER int /*is_parameter_entity*/,
    const XML_Char *value, SAL_UNUSED_PARAMETER int /*value_length*/,
    SAL_UNUSED_PARAMETER const XML_Char * /*base*/, const XML_Char *systemId,
    const XML_Char *publicId, const XML_Char *notationName)
{
    SaxExpatParser_Impl *pImpl = ((SaxExpatParser_Impl*)pvThis);
    if (value) { // value != 0 means internal entity
        SAL_INFO("sax","SaxExpatParser: internal entity declaration, stopping");
        XML_StopParser(pImpl->getEntity().pParser, XML_FALSE);
        pImpl->exception = SAXParseException(
            "SaxExpatParser: internal entity declaration, stopping",
            0, css::uno::Any(),
            pImpl->rDocumentLocator->getPublicId(),
            pImpl->rDocumentLocator->getSystemId(),
            pImpl->rDocumentLocator->getLineNumber(),
            pImpl->rDocumentLocator->getColumnNumber() );
        pImpl->bExceptionWasThrown = true;
    } else {
        if( pImpl->rDTDHandler.is() ) {
            CALL_ELEMENT_HANDLER_AND_CARE_FOR_EXCEPTIONS(
                pImpl ,
                rDTDHandler->unparsedEntityDecl(
                    XML_CHAR_TO_OUSTRING( entityName ),
                    XML_CHAR_TO_OUSTRING( publicId ) ,
                    XML_CHAR_TO_OUSTRING( systemId ) ,
                    XML_CHAR_TO_OUSTRING( notationName ) ) );
        }
    }
}

void SaxExpatParser_Impl::callbackNotationDecl(
    void *pvThis, const XML_Char *notationName,
    SAL_UNUSED_PARAMETER const XML_Char * /*base*/, const XML_Char *systemId,
    const XML_Char *publicId)
{
    SaxExpatParser_Impl *pImpl = ((SaxExpatParser_Impl*)pvThis);
    if( pImpl->rDTDHandler.is() ) {
        CALL_ELEMENT_HANDLER_AND_CARE_FOR_EXCEPTIONS( pImpl,
                rDTDHandler->notationDecl(  XML_CHAR_TO_OUSTRING( notationName ) ,
                                            XML_CHAR_TO_OUSTRING( publicId ) ,
                                            XML_CHAR_TO_OUSTRING( systemId ) ) );
    }

}



bool SaxExpatParser_Impl::callbackExternalEntityRef(
    XML_Parser parser, const XML_Char *context,
    SAL_UNUSED_PARAMETER const XML_Char * /*base*/, const XML_Char *systemId,
    const XML_Char *publicId)
{
    bool bOK = true;
    InputSource source;
    SaxExpatParser_Impl *pImpl = ((SaxExpatParser_Impl*)XML_GetUserData( parser ));

    struct Entity entity;

    if( pImpl->rEntityResolver.is() ) {
        try
        {
            entity.structSource = pImpl->rEntityResolver->resolveEntity(
                XML_CHAR_TO_OUSTRING( publicId ) ,
                XML_CHAR_TO_OUSTRING( systemId ) );
        }
        catch( const SAXParseException & e )
        {
            pImpl->exception = e;
            bOK = false;
        }
        catch( const SAXException & e )
        {
            pImpl->exception = SAXParseException(
                e.Message , e.Context , e.WrappedException ,
                pImpl->rDocumentLocator->getPublicId(),
                pImpl->rDocumentLocator->getSystemId(),
                pImpl->rDocumentLocator->getLineNumber(),
                pImpl->rDocumentLocator->getColumnNumber() );
            bOK = false;
        }
    }

    if( entity.structSource.aInputStream.is() ) {
        entity.pParser = XML_ExternalEntityParserCreate( parser , context, 0 );
        if( ! entity.pParser )
        {
            return false;
        }

        entity.converter.setInputStream( entity.structSource.aInputStream );
        pImpl->pushEntity( entity );
        try
        {
            pImpl->parse();
        }
        catch( const SAXParseException & e )
        {
            pImpl->exception = e;
            bOK = false;
        }
        catch( const IOException &e )
        {
            pImpl->exception.WrappedException <<= e;
            bOK = false;
        }
        catch( const css::uno::RuntimeException &e )
        {
            pImpl->exception.WrappedException <<=e;
            bOK = false;
        }

        pImpl->popEntity();

        XML_ParserFree( entity.pParser );
    }

    return bOK;
}

int SaxExpatParser_Impl::callbackUnknownEncoding(
    SAL_UNUSED_PARAMETER void * /*encodingHandlerData*/,
    SAL_UNUSED_PARAMETER const XML_Char * /*name*/,
    SAL_UNUSED_PARAMETER XML_Encoding * /*info*/)
{
    return 0;
}

void SaxExpatParser_Impl::callbackDefault( void *pvThis,  const XML_Char *s,  int len)
{
    SaxExpatParser_Impl *pImpl = ((SaxExpatParser_Impl*)pvThis);

    CALL_ELEMENT_HANDLER_AND_CARE_FOR_EXCEPTIONS(  pImpl,
                rExtendedDocumentHandler->unknown( XML_CHAR_N_TO_USTRING( s ,len) ) );
}

void SaxExpatParser_Impl::callbackComment( void *pvThis , const XML_Char *s )
{
    SaxExpatParser_Impl *pImpl = ((SaxExpatParser_Impl*)pvThis);
    CALL_ELEMENT_HANDLER_AND_CARE_FOR_EXCEPTIONS( pImpl,
                rExtendedDocumentHandler->comment( XML_CHAR_TO_OUSTRING( s ) ) );
}

void SaxExpatParser_Impl::callbackStartCDATA( void *pvThis )
{
    SaxExpatParser_Impl *pImpl = ((SaxExpatParser_Impl*)pvThis);

    CALL_ELEMENT_HANDLER_AND_CARE_FOR_EXCEPTIONS( pImpl, rExtendedDocumentHandler->startCDATA() );
}


void SaxExpatParser_Impl::callErrorHandler( SaxExpatParser_Impl *pImpl ,
                                            const SAXParseException & e )
{
    try
    {
        if( pImpl->rErrorHandler.is() ) {
            css::uno::Any a;
            a <<= e;
            pImpl->rErrorHandler->error( a );
        }
        else {
            pImpl->exception = e;
            pImpl->bExceptionWasThrown = true;
        }
    }
    catch( const SAXParseException & ex ) {
        pImpl->exception = ex;
        pImpl->bExceptionWasThrown = true;
    }
    catch( const SAXException & ex ) {
        pImpl->exception = SAXParseException(
                                    ex.Message,
                                    ex.Context,
                                    ex.WrappedException,
                                    pImpl->rDocumentLocator->getPublicId(),
                                    pImpl->rDocumentLocator->getSystemId(),
                                    pImpl->rDocumentLocator->getLineNumber(),
                                    pImpl->rDocumentLocator->getColumnNumber()
                             );
        pImpl->bExceptionWasThrown = true;
    }
}

void SaxExpatParser_Impl::callbackEndCDATA( void *pvThis )
{
    SaxExpatParser_Impl *pImpl = ((SaxExpatParser_Impl*)pvThis);

    CALL_ELEMENT_HANDLER_AND_CARE_FOR_EXCEPTIONS(pImpl,rExtendedDocumentHandler->endCDATA() );
}

} // namespace

#if SUPD == 310

using namespace sax_expatwrap;

extern "C" 
{

void SAL_CALL component_getImplementationEnvironment(
	const sal_Char ** ppEnvTypeName, uno_Environment ** /*ppEnv*/ )
{
	*ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

sal_Bool SAL_CALL component_writeInfo(
    void * /*pServiceManager*/, void * pRegistryKey )
{
    if (pRegistryKey)
    {
        try
        {
            css::uno::Reference< css::registry::XRegistryKey > xKey(
                reinterpret_cast< css::registry::XRegistryKey * >( pRegistryKey ) );
            
            css::uno::Reference< css::registry::XRegistryKey > xNewKey = xKey->createKey(
                OUString::createFromAscii( "/" IMPLEMENTATION_NAME "/UNO/SERVICES" ) );
            xNewKey->createKey( OUString::createFromAscii( SERVICE_NAME ) );
    
            xNewKey = xKey->createKey( OUString::createFromAscii("/") +
                                       SaxWriter_getImplementationName()+
                                       OUString::createFromAscii( "/UNO/SERVICES" ) );
            xNewKey->createKey( SaxWriter_getServiceName() );
            
            return sal_True;
        }
        catch (css::registry::InvalidRegistryException &)
        {
            OSL_ENSURE( sal_False, "### InvalidRegistryException!" );
        }
    }
    return sal_False;
}

void * SAL_CALL component_getFactory(
	const sal_Char * pImplName, void * pServiceManager, void * /*pRegistryKey*/ )
{
	void * pRet = 0;
	
	if (pServiceManager )
	{
		css::uno::Reference< XSingleServiceFactory > xRet;
		css::uno::Reference< XMultiServiceFactory > xSMgr =
			reinterpret_cast< XMultiServiceFactory * > ( pServiceManager );
		
		OUString aImplementationName = OUString::createFromAscii( pImplName );
		
		if (aImplementationName ==
			OUString( RTL_CONSTASCII_USTRINGPARAM( IMPLEMENTATION_NAME  ) ) )
		{
			xRet = createSingleFactory( xSMgr, aImplementationName,
										SaxExpatParser_CreateInstance,
										SaxExpatParser::getSupportedServiceNames_Static() );
		}
		else if ( aImplementationName == SaxWriter_getImplementationName() )
		{
			xRet = createSingleFactory( xSMgr, aImplementationName,
										SaxWriter_CreateInstance,
										SaxWriter_getSupportedServiceNames() );
		}

		if (xRet.is())
		{
			xRet->acquire();
			pRet = xRet.get();
		}
	}
	
	return pRet;
}
		

}

#else	// SUPD == 310

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_comp_extensions_xml_sax_ParserExpat_get_implementation(
    css::uno::XComponentContext *,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new SaxExpatParser);
}

#endif	// SUPD == 310

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
