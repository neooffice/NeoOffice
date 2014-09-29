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

#include <cppuhelper/factory.hxx>
#include <cppuhelper/weak.hxx>
#include <cppuhelper/implbase2.hxx>

#include "../tools/fastserializer.hxx"
#include "fastparser.hxx"

using namespace sax_fastparser;
using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::registry;
using ::rtl::OUString;
using namespace ::com::sun::star::lang;

namespace sax_fastparser
{

//--------------------------------------
// the extern interface 
//---------------------------------------
Reference< XInterface > SAL_CALL FastSaxParser_CreateInstance( const Reference< XMultiServiceFactory  >  & ) throw(Exception)
{	
	FastSaxParser *p = new FastSaxParser;
	return Reference< XInterface > ( (OWeakObject * ) p );
}

Reference< XInterface > SAL_CALL FastSaxSerializer_CreateInstance( const Reference< XMultiServiceFactory  >  & ) throw(Exception)
{	
	FastSaxSerializer *p = new FastSaxSerializer;
	return Reference< XInterface > ( (OWeakObject * ) p );
}
}

extern "C"
{

void SAL_CALL component_getImplementationEnvironment(
	const sal_Char ** ppEnvTypeName, uno_Environment ** /*ppEnv*/ )
{
	*ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

#if SUPD == 310

sal_Bool SAL_CALL component_writeInfo(
    void * /*pServiceManager*/, void * pRegistryKey )
{
    if (pRegistryKey)
    {
        try
        {
            Reference< XRegistryKey > xKey( reinterpret_cast< XRegistryKey * >( pRegistryKey ) );

            Reference< XRegistryKey > xNewKey( xKey->createKey(
                OUString::createFromAscii( "/" PARSER_IMPLEMENTATION_NAME "/UNO/SERVICES" ) ) );
            xNewKey->createKey( OUString::createFromAscii( PARSER_SERVICE_NAME ) );

            Reference< XRegistryKey > xNewKey1( xKey->createKey(
                OUString::createFromAscii( "/" SERIALIZER_IMPLEMENTATION_NAME "/UNO/SERVICES" ) ) );
            xNewKey1->createKey( OUString::createFromAscii( SERIALIZER_SERVICE_NAME ) );
                
            return sal_True;
        }
        catch (InvalidRegistryException &)
        {
            OSL_ENSURE( sal_False, "### InvalidRegistryException!" );
        }
    }
    return sal_False;
}

#endif	// SUPD == 310

void * SAL_CALL component_getFactory( const sal_Char * pImplName, void * pServiceManager, void * /*pRegistryKey*/ )
{
	void * pRet = 0;

	if (pServiceManager )
	{
		Reference< XSingleServiceFactory > xRet;
		Reference< XMultiServiceFactory > xSMgr( reinterpret_cast< XMultiServiceFactory * > ( pServiceManager ) );

		OUString aImplementationName( OUString::createFromAscii( pImplName ) );
		
		if (aImplementationName == OUString( RTL_CONSTASCII_USTRINGPARAM( PARSER_IMPLEMENTATION_NAME  ) ) )
		{
			xRet = createSingleFactory( xSMgr, aImplementationName,
										FastSaxParser_CreateInstance,
										FastSaxParser::getSupportedServiceNames_Static() );
		}
		else if (aImplementationName == OUString( RTL_CONSTASCII_USTRINGPARAM( SERIALIZER_IMPLEMENTATION_NAME  ) ) )
		{
			xRet = createSingleFactory( xSMgr, aImplementationName,
										FastSaxSerializer_CreateInstance,
										FastSaxSerializer::getSupportedServiceNames_Static() );
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
