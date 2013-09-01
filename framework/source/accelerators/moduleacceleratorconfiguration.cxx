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
 * Modified August 2013 by Patrick Luby. NeoOffice is distributed under
 * GPL only under modification term 2 of the LGPL.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_framework.hxx"
#include <accelerators/moduleacceleratorconfiguration.hxx>

//_______________________________________________
// own includes
#include <threadhelp/readguard.hxx>
#include <threadhelp/writeguard.hxx>

#ifndef __FRAMEWORK_ACCELERATORCONST_H_
#include <acceleratorconst.h>
#endif
#include <services.h>

//_______________________________________________
// interface includes
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/embed/ElementModes.hpp>

//_______________________________________________
// other includes

#ifndef _COMPHELPER_SEQUENCEASHASHMAP_HXX
#include <comphelper/sequenceashashmap.hxx>
#endif
#include <vcl/svapp.hxx>

#ifndef _COMPHELPER_CONFIGURATIONHELPER_HXX_
#include <comphelper/configurationhelper.hxx>
#endif

#ifndef _COM_SUN_STAR_UTIL_XCHANGESNOTIFIER_HPP_
#include <com/sun/star/util/XChangesNotifier.hpp>
#endif

#ifndef _RTL_LOGFILE_HXX_
#include <rtl/logfile.hxx>
#endif

#ifndef _RTL_LOGFILE_HXX_
#include <rtl/logfile.h>
#endif

//_______________________________________________
// const

namespace framework
{

//-----------------------------------------------    
// XInterface, XTypeProvider, XServiceInfo
#ifdef USE_JAVA
DEFINE_XINTERFACE_3(ModuleAcceleratorConfiguration              ,
                    XCUBasedAcceleratorConfiguration                    ,
                    DIRECT_INTERFACE(css::lang::XServiceInfo)   ,
                    DIRECT_INTERFACE(css::lang::XInitialization),
                    DIRECT_INTERFACE(css::uno::XReference))

DEFINE_XTYPEPROVIDER_3_WITH_BASECLASS(ModuleAcceleratorConfiguration,
                                      XCUBasedAcceleratorConfiguration      ,
                                      css::lang::XServiceInfo       ,
                                      css::lang::XInitialization,
                                      css::uno::XReference)
#else	// USE_JAVA
DEFINE_XINTERFACE_2(ModuleAcceleratorConfiguration              ,
                    XCUBasedAcceleratorConfiguration                    ,
                    DIRECT_INTERFACE(css::lang::XServiceInfo)   ,
                    DIRECT_INTERFACE(css::lang::XInitialization))

DEFINE_XTYPEPROVIDER_2_WITH_BASECLASS(ModuleAcceleratorConfiguration,
                                      XCUBasedAcceleratorConfiguration      ,
                                      css::lang::XServiceInfo       ,
                                      css::lang::XInitialization    )
#endif	// USE_JAVA
                       
DEFINE_XSERVICEINFO_MULTISERVICE(ModuleAcceleratorConfiguration                   ,
                                 ::cppu::OWeakObject                              ,
                                 SERVICENAME_MODULEACCELERATORCONFIGURATION       ,
                                 IMPLEMENTATIONNAME_MODULEACCELERATORCONFIGURATION)

DEFINE_INIT_SERVICE(ModuleAcceleratorConfiguration,
                    {
                        /*Attention
                        I think we don't need any mutex or lock here ... because we are called by our own static method impl_createInstance()
                        to create a new instance of this class by our own supported service factory.
                        see macro DEFINE_XSERVICEINFO_MULTISERVICE and "impl_initService()" for further informations!
                        */
                    }
                   )
                                    
//-----------------------------------------------    
ModuleAcceleratorConfiguration::ModuleAcceleratorConfiguration(const css::uno::Reference< css::lang::XMultiServiceFactory > xSMGR)
    : XCUBasedAcceleratorConfiguration(xSMGR)
{
}

//-----------------------------------------------    
ModuleAcceleratorConfiguration::~ModuleAcceleratorConfiguration()
{
   // m_aPresetHandler.removeStorageListener(this);
}

//-----------------------------------------------    
void SAL_CALL ModuleAcceleratorConfiguration::initialize(const css::uno::Sequence< css::uno::Any >& lArguments)
    throw(css::uno::Exception       ,
          css::uno::RuntimeException)
{
    // SAFE -> ----------------------------------
    WriteGuard aWriteLock(m_aLock);
    
	::comphelper::SequenceAsHashMap lArgs(lArguments);
    m_sModule = lArgs.getUnpackedValueOrDefault(::rtl::OUString::createFromAscii("ModuleIdentifier"), ::rtl::OUString());
	m_sLocale = lArgs.getUnpackedValueOrDefault(::rtl::OUString::createFromAscii("Locale")          , ::rtl::OUString::createFromAscii("x-default"));
    
    if (!m_sModule.getLength())
        throw css::uno::RuntimeException(
                ::rtl::OUString::createFromAscii("The module dependend accelerator configuration service was initialized with an empty module identifier!"),
                static_cast< ::cppu::OWeakObject* >(this));
    
    aWriteLock.unlock();
    // <- SAFE ----------------------------------
    
    impl_ts_fillCache();
}
          
#ifdef USE_JAVA

//-----------------------------------------------    
void SAL_CALL ModuleAcceleratorConfiguration::dispose()
    throw(css::uno::RuntimeException)
{
    try
    {
        css::uno::Reference< css::util::XChangesNotifier > xBroadcaster(m_xCfg, css::uno::UNO_QUERY);
        if (xBroadcaster.is())
            xBroadcaster->removeChangesListener(static_cast< css::util::XChangesListener* >(this));
    }
    catch ( ... )
    {
    }
}

#endif	// USE_JAVA

//-----------------------------------------------    
void ModuleAcceleratorConfiguration::impl_ts_fillCache()
{
    // SAFE -> ----------------------------------
    ReadGuard aReadLock(m_aLock);
    ::rtl::OUString sModule = m_sModule;
	m_sModuleCFG = m_sModule;
    aReadLock.unlock();
    // <- SAFE ----------------------------------

    // get current office locale ... but dont cache it.
    // Otherwise we must be listener on the configuration layer
    // which seems to superflous for this small implementation .-)
	::comphelper::Locale aLocale = ::comphelper::Locale(m_sLocale);
    
    // May be the current app module does not have any
    // accelerator config? Handle it gracefully :-)
    try
    {
        m_sGlobalOrModules = CFG_ENTRY_MODULES;
		XCUBasedAcceleratorConfiguration::reload();

		css::uno::Reference< css::util::XChangesNotifier > xBroadcaster(m_xCfg, css::uno::UNO_QUERY_THROW);
		xBroadcaster->addChangesListener(static_cast< css::util::XChangesListener* >(this));
	}
    catch(const css::uno::RuntimeException& exRun)
        { throw exRun; }
    catch(const css::uno::Exception&)
        {}
}

} // namespace framework

