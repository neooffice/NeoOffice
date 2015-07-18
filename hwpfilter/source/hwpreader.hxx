/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of NeoOffice.
 *
 * This file incorporates work covered by the following license notices:
 *
 *   This Source Code Form is subject to the terms of the Mozilla Public
 *   License, v. 2.0. If a copy of the MPL was not distributed with this
 *   file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
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
 * Modified July 2015 by Patrick Luby. NeoOffice is distributed under
 * GPL only under Section 3.3 of the Mozilla Public License v2.0.
 */

#ifndef INCLUDED_HWPFILTER_SOURCE_HWPREADER_HXX
#define INCLUDED_HWPFILTER_SOURCE_HWPREADER_HXX
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sal/alloca.h>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/document/XImporter.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>

#include <com/sun/star/io/XActiveDataSink.hpp>
#include <com/sun/star/io/XActiveDataControl.hpp>
#include <com/sun/star/io/XStreamListener.hpp>
#include <com/sun/star/document/XExtendedFilterDetection.hpp>

#include <cppuhelper/factory.hxx>
#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/implbase2.hxx>
#include <cppuhelper/implbase4.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/weak.hxx>

using namespace ::cppu;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::registry;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::xml::sax;

#include <assert.h>

#if SUPD == 310
#include <comphelper/mediadescriptor.hxx>
#else	// SUPD == 310
#include <unotools/mediadescriptor.hxx>
#endif	// SUPD == 310

#include "hwpfile.h"
#include "hcode.h"
#include "hbox.h"
#include "htags.h"
#include "hstream.h"
#include "drawdef.h"
#include "attributes.hxx"

#define IMPLEMENTATION_NAME "com.sun.comp.hwpimport.HwpImportFilter"
#define SERVICE_NAME1 "com.sun.star.document.ImportFilter"
#define SERVICE_NAME2 "com.sun.star.document.ExtendedTypeDetection"
#define WRITER_IMPORTER_NAME "com.sun.star.comp.Writer.XMLImporter"

struct HwpReaderPrivate;
/**
 * This class implements the external Parser interface
 */
class HwpReader : public WeakImplHelper1<XFilter>
{

public:
    HwpReader();
    virtual ~HwpReader();

public:
    /**
     * parseStream does Parser-startup initializations
     */
#if SUPD == 310
    virtual sal_Bool SAL_CALL filter(const Sequence< PropertyValue >& aDescriptor) throw (RuntimeException) SAL_OVERRIDE;
#else	// SUPD == 310
    virtual sal_Bool SAL_CALL filter(const Sequence< PropertyValue >& aDescriptor) throw (RuntimeException, std::exception) SAL_OVERRIDE;
#endif	// SUPD == 310
#if SUPD == 310
    virtual void SAL_CALL cancel() throw(RuntimeException) SAL_OVERRIDE {}
#else	// SUPD == 310
    virtual void SAL_CALL cancel() throw(RuntimeException, std::exception) SAL_OVERRIDE {}
#endif	// SUPD == 310
    virtual void SAL_CALL setDocumentHandler(Reference< XDocumentHandler > xHandler)
    {
        m_rxDocumentHandler = xHandler;
    }
private:
    Reference< XDocumentHandler > m_rxDocumentHandler;
    Reference< XAttributeList > rList;
    AttributeListImpl *pList;
    HWPFile hwpfile;
    HwpReaderPrivate *d;
private:
    /* -------- Document Parsing --------- */
    void makeMeta();
    void makeStyles();
    void makeDrawMiscStyle(HWPDrawingObject *);
    void makeAutoStyles();
    void makeMasterStyles();
    void makeBody();

    void makeTextDecls();

    /* -------- Paragraph Parsing --------- */
    void parsePara(HWPPara *para, bool bParaStart = false);
    void make_text_p0(HWPPara *para, bool bParaStart = false);
    void make_text_p1(HWPPara *para, bool bParaStart = false);
    void make_text_p3(HWPPara *para, bool bParaStart = false);

    /* -------- rDocument->characters(x) --------- */
    void makeChars(hchar_string & rStr);

    /* -------- Special Char Parsing --------- */
    void makeFieldCode(hchar_string & rStr, FieldCode *hbox); //6
    void makeBookmark(Bookmark *hbox);      //6
    void makeDateFormat(DateCode *hbox);    //7
    void makeDateCode(DateCode *hbox);      //8
    void makeTab(Tab *hbox);            //9
    void makeTable(TxtBox *hbox);
    void makeTextBox(TxtBox *hbox);
    void makeFormula(TxtBox *hbox);
    void makeHyperText(TxtBox *hbox);
    void makePicture(Picture *hbox);
    void makePictureDRAW(HWPDrawingObject *drawobj, Picture *hbox);
    void makeLine(Line *hbox);
    void makeHidden(Hidden *hbox);
    void makeFootnote(Footnote *hbox);
    void makeAutoNum(AutoNum *hbox);
    void makeShowPageNum();
    void makeMailMerge(MailMerge *hbox);
    void makeTocMark(TocMark *hbox);
    void makeIndexMark(IndexMark *hbox);
    void makeOutline(Outline *hbox);

    /* --------- Styles Parsing ------------ */
    void makePageStyle();
    void makeColumns(ColumnDef *);
    void makeTStyle(CharShape *);
    void makePStyle(ParaShape *);
    void makeFStyle(FBoxStyle *);
    void makeCaptionStyle(FBoxStyle *);
    void makeDrawStyle(HWPDrawingObject *,FBoxStyle *);
    void makeTableStyle(Table *);
    void parseCharShape(CharShape *);
    void parseParaShape(ParaShape *);
    char* getTStyleName(int, char *);
    char* getPStyleName(int, char *);
};

class HwpImportFilter : public WeakImplHelper4< XFilter, XImporter, XServiceInfo, XExtendedFilterDetection >
{
public:
    HwpImportFilter( const Reference< XMultiServiceFactory > xFact );
    virtual ~HwpImportFilter();

public:
    static Sequence< OUString > getSupportedServiceNames_Static() throw();
    static OUString getImplementationName_Static() throw();

public:
    // XFilter
    virtual sal_Bool SAL_CALL filter( const Sequence< PropertyValue >& aDescriptor )
#if SUPD == 310
        throw( RuntimeException ) SAL_OVERRIDE;
#else	// SUPD == 310
        throw( RuntimeException, std::exception ) SAL_OVERRIDE;
#endif	// SUPD == 310
#if SUPD == 310
    virtual void SAL_CALL cancel() throw(RuntimeException) SAL_OVERRIDE;
#else	// SUPD == 310
    virtual void SAL_CALL cancel() throw(RuntimeException, std::exception) SAL_OVERRIDE;
#endif	// SUPD == 310

    // XImporter
    virtual void SAL_CALL setTargetDocument( const Reference< XComponent >& xDoc)
#if SUPD == 310
        throw( IllegalArgumentException, RuntimeException ) SAL_OVERRIDE;
#else	// SUPD == 310
        throw( IllegalArgumentException, RuntimeException, std::exception ) SAL_OVERRIDE;
#endif	// SUPD == 310

    // XServiceInfo
#if SUPD == 310
    OUString SAL_CALL getImplementationName() throw (RuntimeException) SAL_OVERRIDE;
#else	// SUPD == 310
    OUString SAL_CALL getImplementationName() throw (RuntimeException, std::exception) SAL_OVERRIDE;
#endif	// SUPD == 310
#if SUPD == 310
    Sequence< OUString > SAL_CALL getSupportedServiceNames(void) throw (::com::sun::star::uno::RuntimeException) SAL_OVERRIDE;
#else	// SUPD == 310
    Sequence< OUString > SAL_CALL getSupportedServiceNames(void) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
#endif	// SUPD == 310
#if SUPD == 310
    sal_Bool SAL_CALL supportsService(const OUString& ServiceName) throw (::com::sun::star::uno::RuntimeException) SAL_OVERRIDE;
#else	// SUPD == 310
    sal_Bool SAL_CALL supportsService(const OUString& ServiceName) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
#endif	// SUPD == 310

    //XExtendedFilterDetection
#if SUPD == 310
    virtual OUString SAL_CALL detect( ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& rDescriptor ) throw (::com::sun::star::uno::RuntimeException) SAL_OVERRIDE;
#else	// SUPD == 310
    virtual OUString SAL_CALL detect( ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& rDescriptor ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
#endif	// SUPD == 310

public:
    Reference< XFilter > rFilter;
    Reference< XImporter > rImporter;
};

Reference< XInterface > HwpImportFilter_CreateInstance(
    const Reference< XMultiServiceFactory >& rSMgr ) throw( Exception )
{
    HwpImportFilter *p = new HwpImportFilter( rSMgr );

    return Reference< XInterface > ( (OWeakObject* )p );
}

Sequence< OUString > HwpImportFilter::getSupportedServiceNames_Static() throw ()
{
    Sequence< OUString > aRet(1);
    aRet.getArray()[0] = HwpImportFilter::getImplementationName_Static();
    return aRet;
}

HwpImportFilter::HwpImportFilter( const Reference< XMultiServiceFactory > xFact )
{
    OUString sService( WRITER_IMPORTER_NAME );
    try {
        Reference< XDocumentHandler >
            xHandler( xFact->createInstance( sService ), UNO_QUERY );

        HwpReader *p = new HwpReader;
        p->setDocumentHandler( xHandler );

        Reference< XImporter > xImporter = Reference< XImporter >( xHandler, UNO_QUERY );
        rImporter = xImporter;
        Reference< XFilter > xFilter = Reference< XFilter >( p );
        rFilter = xFilter;
    }
    catch( Exception & )
    {
        printf(" fail to instanciate %s\n", WRITER_IMPORTER_NAME );
        exit( 1 );
    }
}

HwpImportFilter::~HwpImportFilter()
{
}

sal_Bool HwpImportFilter::filter( const Sequence< PropertyValue >& aDescriptor )
#if SUPD == 310
    throw( RuntimeException )
#else	// SUPD == 310
    throw( RuntimeException, std::exception )
#endif	// SUPD == 310
{
    // delegate to IchitaroImpoter
    return rFilter->filter( aDescriptor );
}

#if SUPD == 310
void HwpImportFilter::cancel() throw(::com::sun::star::uno::RuntimeException)
#else	// SUPD == 310
void HwpImportFilter::cancel() throw(::com::sun::star::uno::RuntimeException, std::exception)
#endif	// SUPD == 310
{
    rFilter->cancel();
}

void HwpImportFilter::setTargetDocument( const Reference< XComponent >& xDoc )
#if SUPD == 310
    throw( IllegalArgumentException, RuntimeException )
#else	// SUPD == 310
    throw( IllegalArgumentException, RuntimeException, std::exception )
#endif	// SUPD == 310
{
        // delegate
    rImporter->setTargetDocument( xDoc );
}

OUString HwpImportFilter::getImplementationName_Static() throw()
{
    return OUString( IMPLEMENTATION_NAME );
}

#if SUPD == 310
OUString HwpImportFilter::getImplementationName() throw(::com::sun::star::uno::RuntimeException)
#else	// SUPD == 310
OUString HwpImportFilter::getImplementationName() throw(::com::sun::star::uno::RuntimeException, std::exception)
#endif	// SUPD == 310
{
    return OUString( IMPLEMENTATION_NAME );
}

#if SUPD == 310
sal_Bool HwpImportFilter::supportsService( const OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException)
#else	// SUPD == 310
sal_Bool HwpImportFilter::supportsService( const OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException, std::exception)
#endif	// SUPD == 310
{
    return cppu::supportsService(this, ServiceName);
}

//XExtendedFilterDetection
#if SUPD == 310
OUString HwpImportFilter::detect( ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& rDescriptor ) throw (::com::sun::star::uno::RuntimeException)
#else	// SUPD == 310
OUString HwpImportFilter::detect( ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& rDescriptor ) throw (::com::sun::star::uno::RuntimeException, std::exception)
#endif	// SUPD == 310
{
    OUString sTypeName;

#if SUPD == 310
    comphelper::MediaDescriptor aDescriptor(rDescriptor);
#else	// SUPD == 310
    utl::MediaDescriptor aDescriptor(rDescriptor);
#endif	// SUPD == 310
    aDescriptor.addInputStream();

    Reference< XInputStream > xInputStream(
#if SUPD == 310
        aDescriptor[comphelper::MediaDescriptor::PROP_INPUTSTREAM()], UNO_QUERY);
#else	// SUPD == 310
        aDescriptor[utl::MediaDescriptor::PROP_INPUTSTREAM()], UNO_QUERY);
#endif	// SUPD == 310

    if (xInputStream.is())
    {
        Sequence< sal_Int8 > aData;
        sal_Int32 nLen = HWPIDLen;
        if (
             nLen == xInputStream->readBytes(aData, nLen) &&
             detect_hwp_version(reinterpret_cast<const char*>(aData.getConstArray()))
           )
        {
            sTypeName = OUString("writer_MIZI_Hwp_97");
        }
    }

    return sTypeName;
}

#if SUPD == 310
Sequence< OUString> HwpImportFilter::getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException)
#else	// SUPD == 310
Sequence< OUString> HwpImportFilter::getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException, std::exception)
#endif	// SUPD == 310
{
    Sequence < OUString > aRet(2);
    OUString* pArray = aRet.getArray();
    pArray[0] = OUString(SERVICE_NAME1);
    pArray[1] = OUString(SERVICE_NAME2);
    return aRet;
}

extern "C"
{
#if SUPD == 310
    void SAL_CALL component_getImplementationEnvironment(
        const sal_Char ** ppEnvTypeName, uno_Environment **  )
    {
        *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
    }

    sal_Bool SAL_CALL component_writeInfo(
        void * , void * pRegistryKey )
    {
        if (pRegistryKey)
        {
            try
            {
                Reference< XRegistryKey > xKey( reinterpret_cast< XRegistryKey * >( pRegistryKey ) );

                Reference< XRegistryKey > xNewKey = xKey->createKey(
                    OUString::createFromAscii( "/" IMPLEMENTATION_NAME "/UNO/SERVICES" ) );
#ifdef USE_JAVA
                xNewKey->createKey( OUString::createFromAscii( SERVICE_NAME1 ) );
                xNewKey->createKey( OUString::createFromAscii( SERVICE_NAME2 ) );
#else	// USE_JAVA
                xNewKey->createKey( OUString::createFromAscii( SERVICE_NAME ) );
#endif	// USE_JAVA

                return sal_True;
            }
            catch (InvalidRegistryException &)
            {
                OSL_ENSURE( sal_False, "### InvalidRegistryException!" );
            }
        }
        return sal_False;
    }

    void * SAL_CALL component_getFactory( const sal_Char * pImplName, void * pServiceManager, void *  )
#else	// SUPD == 310
    SAL_DLLPUBLIC_EXPORT void * SAL_CALL hwp_component_getFactory( const sal_Char * pImplName, void * pServiceManager, void *  )
#endif	// SUPD == 310
    {
        void * pRet = 0;

        if (pServiceManager )
        {
            Reference< XSingleServiceFactory > xRet;
            Reference< XMultiServiceFactory > xSMgr = reinterpret_cast< XMultiServiceFactory * > ( pServiceManager );

            OUString aImplementationName = OUString::createFromAscii( pImplName );

            if (aImplementationName == IMPLEMENTATION_NAME )
            {
                xRet = createSingleFactory( xSMgr, aImplementationName,
                                            HwpImportFilter_CreateInstance,
                                            HwpImportFilter::getSupportedServiceNames_Static() );
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

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
