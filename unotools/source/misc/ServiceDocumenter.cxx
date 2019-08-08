/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <ServiceDocumenter.hxx>
#include <unotoolsservices.hxx>
#include <comphelper/servicedecl.hxx>
#include <com/sun/star/system/XSystemShellExecute.hpp>
#ifndef NO_LIBO_SERVICE_DOCUMENTER_FIX
#include <com/sun/star/system/SystemShellExecuteFlags.hpp>
#endif	// !NO_LIBO_SERVICE_DOCUMENTER_FIX
using namespace com::sun::star;
using uno::Reference;
using lang::XServiceInfo;
using lang::XTypeProvider;

void unotools::misc::ServiceDocumenter::showCoreDocs(const Reference<XServiceInfo>& xService)
{
    if(!xService.is())
        return;
    auto xMSF(m_xContext->getServiceManager());
    Reference<system::XSystemShellExecute> xShell(xMSF->createInstanceWithContext("com.sun.star.system.SystemShellExecute", m_xContext), uno::UNO_QUERY);
#ifdef NO_LIBO_SERVICE_DOCUMENTER_FIX
    xShell->execute(m_sCoreBaseUrl + xService->getImplementationName() + ".html", "", 0);
#else	// NO_LIBO_SERVICE_DOCUMENTER_FIX
    xShell->execute(
        m_sCoreBaseUrl + xService->getImplementationName() + ".html", "",
        css::system::SystemShellExecuteFlags::URIS_ONLY);
#endif	// NO_LIBO_SERVICE_DOCUMENTER_FIX
}

void unotools::misc::ServiceDocumenter::showInterfaceDocs(const Reference<XTypeProvider>& xTypeProvider)
{
    if(!xTypeProvider.is())
        return;
    auto xMSF(m_xContext->getServiceManager());
    Reference<system::XSystemShellExecute> xShell(xMSF->createInstanceWithContext("com.sun.star.system.SystemShellExecute", m_xContext), uno::UNO_QUERY);
    for(const auto& aType : xTypeProvider->getTypes())
    {
        auto sUrl = aType.getTypeName();
        sal_Int32 nIdx = 0;
        while(nIdx != -1)
            sUrl = sUrl.replaceFirst(".", "_1_1", &nIdx);
#ifdef NO_LIBO_SERVICE_DOCUMENTER_FIX
        xShell->execute(m_sServiceBaseUrl + "/interface" + sUrl + ".html", "", 0);
#else	// NO_LIBO_SERVICE_DOCUMENTER_FIX
        xShell->execute(
            m_sServiceBaseUrl + "/interface" + sUrl + ".html", "",
            css::system::SystemShellExecuteFlags::URIS_ONLY);
#endif	// NO_LIBO_SERVICE_DOCUMENTER_FIX
    }
}

void unotools::misc::ServiceDocumenter::showServiceDocs(const Reference<XServiceInfo>& xService)
{
    if(!xService.is())
        return;
    auto xMSF(m_xContext->getServiceManager());
    Reference<system::XSystemShellExecute> xShell(xMSF->createInstanceWithContext("com.sun.star.system.SystemShellExecute", m_xContext), uno::UNO_QUERY);
    for(const auto& sService : xService->getSupportedServiceNames())
    {
        auto sUrl = sService;
        sal_Int32 nIdx = 0;
        while(nIdx != -1)
            sUrl = sUrl.replaceFirst(".", "_1_1", &nIdx);
#ifdef NO_LIBO_SERVICE_DOCUMENTER_FIX
        xShell->execute(m_sServiceBaseUrl + "/service" + sUrl + ".html", "", 0);
#else	// NO_LIBO_SERVICE_DOCUMENTER_FIX
        xShell->execute(
            m_sServiceBaseUrl + "/service" + sUrl + ".html", "",
            css::system::SystemShellExecuteFlags::URIS_ONLY);
#endif	// NO_LIBO_SERVICE_DOCUMENTER_FIX
    }
}

namespace sdecl = ::comphelper::service_decl;
sdecl::class_< unotools::misc::ServiceDocumenter > const ServiceDocumenterImpl;
const sdecl::ServiceDecl ServiceDocumenterDecl(
    ServiceDocumenterImpl,
    "com.sun.star.comp.unotools.misc.ServiceDocumenter",
    "");

