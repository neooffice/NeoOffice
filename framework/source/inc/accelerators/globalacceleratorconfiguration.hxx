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

#ifndef __FRAMEWORK_ACCELERATORS_GLOBALACCELERATORCONFIGURATION_HXX_
#define __FRAMEWORK_ACCELERATORS_GLOBALACCELERATORCONFIGURATION_HXX_

//__________________________________________
// own includes

#include <accelerators/acceleratorconfiguration.hxx>
#include <accelerators/presethandler.hxx>

#ifndef __FRAMEWORK_MACROS_XINTERFACE_HXX_
#include <macros/interface.hxx>
#endif
#include <macros/xtypeprovider.hxx>
#include <macros/xserviceinfo.hxx>

//__________________________________________
// interface includes

#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif

#ifdef USE_JAVA
#include <com/sun/star/uno/XReference.hpp>
#endif	// USE_JAVA

//__________________________________________
// other includes

//__________________________________________
// definition

namespace framework
{

//__________________________________________
/**
    implements a read/write access to the global
    accelerator configuration.
 */
class GlobalAcceleratorConfiguration : public XCUBasedAcceleratorConfiguration
                                     , public css::lang::XServiceInfo
									 , public css::lang::XInitialization
#ifdef USE_JAVA
									 , public css::uno::XReference
#endif	// USE_JAVA
{
    //______________________________________
    // interface

    public:
        
        //----------------------------------
        /** initialize this instance and fill the internal cache.
        
            @param  xSMGR
                    reference to an uno service manager, which is used internaly.
         */
        GlobalAcceleratorConfiguration(const css::uno::Reference< css::lang::XMultiServiceFactory > xSMGR);
        
        //----------------------------------
        /** TODO */
        virtual ~GlobalAcceleratorConfiguration();
         
        // XInterface, XTypeProvider, XServiceInfo         
        FWK_DECLARE_XINTERFACE
        FWK_DECLARE_XTYPEPROVIDER
        DECLARE_XSERVICEINFO

		// XInitialization
		virtual void SAL_CALL initialize(const css::uno::Sequence< css::uno::Any >& lArguments)
			throw (css::uno::Exception       ,
			css::uno::RuntimeException);

#ifdef USE_JAVA
		// XReference
		virtual void SAL_CALL dispose() throw (css::uno::RuntimeException);
#endif	// USE_JAVA

    //______________________________________
    // helper
    
    private:

		::rtl::OUString m_sLocale;
         
        //----------------------------------
        /** read all data into the cache. */
        void impl_ts_fillCache();
};

} // namespace framework

#endif // __FRAMEWORK_ACCELERATORS_GLOBALACCELERATORCONFIGURATION_HXX_
