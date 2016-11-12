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
 * 
 *   Modified November 2016 by Patrick Luby. NeoOffice is only distributed
 *   under the GNU General Public License, Version 3 as allowed by Section 3.3
 *   of the Mozilla Public License, v. 2.0.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <cppuhelper/factory.hxx>
#include <com/sun/star/container/XSet.hpp>

#ifdef USE_JAVA
#include "java_filepicker.hxx"
#include "java_folderpicker.hxx"
#else	// USE_JAVA
#include "SalAquaFilePicker.hxx"
#include "SalAquaFolderPicker.hxx"
#endif	// USE_JAVA

#include "FPServiceInfo.hxx"


using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;
using namespace ::cppu;
using ::com::sun::star::ui::dialogs::XFilePicker;


#ifdef USE_JAVA
static Reference< XInterface > SAL_CALL createFileInstance( const Reference< XMultiServiceFactory >& rServiceManager )
#else	// USE_JAVA
static Reference< XInterface > SAL_CALL createFileInstance( const Reference< XMultiServiceFactory >&  )
#endif	// USE_JAVA
{
#ifdef USE_JAVA
    return Reference< XInterface >( *new ::java::JavaFilePicker( rServiceManager ) );
#else	// USE_JAVA
    return Reference< XInterface >( *new SalAquaFilePicker );
#endif	// USE_JAVA
}

static Reference< XInterface > SAL_CALL createFolderInstance(
    const Reference< XMultiServiceFactory >& rServiceManager )
{
    return Reference< XInterface >(
#ifdef USE_JAVA
        *new ::java::JavaFolderPicker( rServiceManager ) );
#else	// USE_JAVA
        *new SalAquaFolderPicker( rServiceManager ) );
#endif	// USE_JAVA
}

extern "C"
{

SAL_DLLPUBLIC_EXPORT void* SAL_CALL fps_aqua_component_getFactory(
    const sal_Char* pImplName, void* pSrvManager, void* /*pRegistryKey*/ )
{
    void* pRet = 0;

    if( pSrvManager )
    {
            Reference< XSingleServiceFactory > xFactory;

            if (0 == rtl_str_compare(pImplName, FILE_PICKER_IMPL_NAME))
            {
                Sequence< OUString > aSNS( 1 );
                aSNS.getArray( )[0] = FILE_PICKER_SERVICE_NAME;

                xFactory = createSingleFactory(
                    reinterpret_cast< XMultiServiceFactory* > ( pSrvManager ),
                    OUString::createFromAscii( pImplName ),
                    createFileInstance,
                    aSNS );
            }
            else if (0 == rtl_str_compare(pImplName, FOLDER_PICKER_IMPL_NAME))
            {
                Sequence< OUString > aSNS( 1 );
                aSNS.getArray( )[0] = FOLDER_PICKER_SERVICE_NAME;

                xFactory = createSingleFactory(
                    reinterpret_cast< XMultiServiceFactory* > ( pSrvManager ),
                    OUString::createFromAscii( pImplName ),
                    createFolderInstance,
                    aSNS );
            }

            if ( xFactory.is() )
            {
                xFactory->acquire();
                pRet = xFactory.get();
            }
    }

    return pRet;
}

} // extern "C"

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
