/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 * 
 *   Modified January 2018 by Patrick Luby. NeoOffice is only distributed
 *   under the GNU General Public License, Version 3 as allowed by Section 3.3
 *   of the Mozilla Public License, v. 2.0.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef INCLUDED_WRITERPERFECT_INC_WRITERPERFECT_IMPORTFILTER_HXX
#define INCLUDED_WRITERPERFECT_INC_WRITERPERFECT_IMPORTFILTER_HXX

#include <libodfgen/libodfgen.hxx>

#include <librevenge/librevenge.h>

#include <librevenge-stream/librevenge-stream.h>

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/document/XExtendedFilterDetection.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/document/XImporter.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/xml/sax/InputSource.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>

#include <osl/diagnose.h>
#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/implbase4.hxx>

#include <unotools/mediadescriptor.hxx>

#include <writerperfect/DocumentHandler.hxx>
#include <writerperfect/WPXSvInputStream.hxx>

#include <xmloff/attrlist.hxx>

#include "writerperfect/DocumentHandlerFor.hxx"

namespace writerperfect
{

namespace detail
{

template<class Generator>
class ImportFilterImpl : public cppu::WeakImplHelper4
    <
    com::sun::star::document::XFilter,
    com::sun::star::document::XImporter,
    com::sun::star::document::XExtendedFilterDetection,
    com::sun::star::lang::XInitialization
    >
{
public:
    ImportFilterImpl(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > &rxContext)
        : mxContext(rxContext)
    {
    }

    virtual ~ImportFilterImpl()
    {
    }

    // XFilter
    virtual sal_Bool SAL_CALL filter(const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > &rDescriptor)
    throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE
    {
        utl::MediaDescriptor aDescriptor(rDescriptor);
        css::uno::Reference < css::io::XInputStream > xInputStream;
        aDescriptor[utl::MediaDescriptor::PROP_INPUTSTREAM()] >>= xInputStream;
        if (!xInputStream.is())
        {
            OSL_ASSERT(false);
            return sal_False;
        }

        // An XML import service: what we push sax messages to..
        css::uno::Reference < css::xml::sax::XDocumentHandler > xInternalHandler(
            mxContext->getServiceManager()->createInstanceWithContext(
                DocumentHandlerFor<Generator>::name(), mxContext),
            css::uno::UNO_QUERY_THROW);

        // The XImporter sets up an empty target document for XDocumentHandler to write to..
        css::uno::Reference < css::document::XImporter > xImporter(xInternalHandler, css::uno::UNO_QUERY);
        xImporter->setTargetDocument(mxDoc);

        // OO Graphics Handler: abstract class to handle document SAX messages, concrete implementation here
        // writes to in-memory target doc
        DocumentHandler xHandler(xInternalHandler);

        WPXSvInputStream input(xInputStream);

        Generator exporter;
        exporter.addDocumentHandler(&xHandler, ODF_FLAT_XML);

        this->doRegisterHandlers(exporter);

        return this->doImportDocument(input, exporter, aDescriptor);
    }

    virtual void SAL_CALL cancel()
    throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE
    {
    }

    // XImporter
    virtual void SAL_CALL setTargetDocument(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent > &xDoc)
    throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE
    {
        mxDoc = xDoc;
    }

    //XExtendedFilterDetection
    virtual OUString SAL_CALL detect(com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue > &Descriptor)
    throw(com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE
    {
        OUString sTypeName;
#ifdef USE_JAVA
        // Fix Mac App Store crash when loading some .wpg files by catching any
        // unexpected exception types
        try
        {
#endif	// USE_JAVA
        sal_Int32 nLength = Descriptor.getLength();
        sal_Int32 location = nLength;
        const css::beans::PropertyValue *pValue = Descriptor.getConstArray();
        css::uno::Reference < css::io::XInputStream > xInputStream;
        for (sal_Int32 i = 0 ; i < nLength; i++)
        {
            if (pValue[i].Name == "TypeName")
                location=i;
            else if (pValue[i].Name == "InputStream")
                pValue[i].Value >>= xInputStream;
        }

        if (!xInputStream.is())
            return OUString();

        WPXSvInputStream input(xInputStream);

        if (this->doDetectFormat(input, sTypeName))
        {
            assert(!sTypeName.isEmpty());

            if (location == nLength)
            {
                Descriptor.realloc(nLength+1);
                Descriptor[location].Name = "TypeName";
            }

            Descriptor[location].Value <<=sTypeName;
        }
#ifdef USE_JAVA
        }
        catch ( const com::sun::star::uno::RuntimeException &e )
        {
            throw;
        }
        catch ( const std::exception &e )
        {
            throw;
        }
        catch ( ... )
        {
            throw com::sun::star::uno::RuntimeException( "ImportFilterImpl::detect method raised an unknown exception" );
        }
#endif	// USE_JAVA

        return sTypeName;
    }

    // XInitialization
    virtual void SAL_CALL initialize(const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > &aArguments)
    throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE
    {
        css::uno::Sequence < css::beans::PropertyValue > aAnySeq;
        sal_Int32 nLength = aArguments.getLength();
        if (nLength && (aArguments[0] >>= aAnySeq))
        {
            const css::beans::PropertyValue *pValue = aAnySeq.getConstArray();
            nLength = aAnySeq.getLength();
            for (sal_Int32 i = 0 ; i < nLength; i++)
            {
                if (pValue[i].Name == "Type")
                {
                    pValue[i].Value >>= msFilterName;
                    break;
                }
            }
        }
    }

private:
    virtual bool doDetectFormat(librevenge::RVNGInputStream &rInput, OUString &rTypeName) = 0;
    virtual bool doImportDocument(librevenge::RVNGInputStream &rInput, Generator &rGenerator, utl::MediaDescriptor &rDescriptor) = 0;
    virtual void doRegisterHandlers(Generator &)
    {
    }

private:
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > mxContext;
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent > mxDoc;
    OUString msFilterName;
    ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XDocumentHandler > mxHandler;
};

}

/** A base class for import filters.
 */
template<class Generator>
struct ImportFilter : public cppu::ImplInheritanceHelper1<detail::ImportFilterImpl<Generator>, com::sun::star::lang::XServiceInfo>
{
    ImportFilter(const css::uno::Reference<css::uno::XComponentContext> &rxContext)
        : cppu::ImplInheritanceHelper1<detail::ImportFilterImpl<Generator>, com::sun::star::lang::XServiceInfo>(rxContext)
    {
    }
};

}

#endif // INCLUDED_WRITERPERFECT_INC_WRITERPERFECT_IMPORTFILTER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
