/**************************************************************
 * 
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 * 
 *************************************************************/



#ifndef OOX_DOCPROP_OOXMLDOCPROPIMPORT_HXX
#define OOX_DOCPROP_OOXMLDOCPROPIMPORT_HXX

#include <com/sun/star/document/XOOXMLDocumentPropertiesImporter.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/xml/sax/InputSource.hpp>
#include <cppuhelper/implbase2.hxx>

namespace oox {
namespace docprop {

// ============================================================================

class DocumentPropertiesImport :
    public ::cppu::WeakImplHelper2<
        ::com::sun::star::lang::XServiceInfo,
        ::com::sun::star::document::XOOXMLDocumentPropertiesImporter >
{
public:
    explicit            DocumentPropertiesImport(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext );

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName() throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& rServiceName ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw (::com::sun::star::uno::RuntimeException);

    // XOOXMLDocumentPropertiesImporter
    virtual void SAL_CALL importProperties(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& rxSource,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::document::XDocumentProperties >& rxDocumentProperties )
                        throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::Exception);

private:
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > mxContext;
};

// ============================================================================

} // namespace docprop
} // namespace oox

#endif
