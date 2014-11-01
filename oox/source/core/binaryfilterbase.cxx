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



#include "oox/core/binaryfilterbase.hxx"

#include "oox/ole/olestorage.hxx"

namespace oox {
namespace core {

// ============================================================================

using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::uno;

using ::rtl::OUString;

// ============================================================================

#if SUPD == 310
BinaryFilterBase::BinaryFilterBase( const css::uno::Reference< XComponentContext >& rxContext ) throw( RuntimeException ) :
#else	// SUPD == 310
BinaryFilterBase::BinaryFilterBase( const Reference< XComponentContext >& rxContext ) throw( RuntimeException ) :
#endif	// SUPD == 310
    FilterBase( rxContext )
{
}

BinaryFilterBase::~BinaryFilterBase()
{
}

// private --------------------------------------------------------------------

#if SUPD == 310
StorageRef BinaryFilterBase::implCreateStorage( const css::uno::Reference< XInputStream >& rxInStream ) const
#else	// SUPD == 310
StorageRef BinaryFilterBase::implCreateStorage( const Reference< XInputStream >& rxInStream ) const
#endif	// SUPD == 310
{
    return StorageRef( new ::oox::ole::OleStorage( getComponentContext(), rxInStream, true ) );
}

#if SUPD == 310
StorageRef BinaryFilterBase::implCreateStorage( const css::uno::Reference< XStream >& rxOutStream ) const
#else	// SUPD == 310
StorageRef BinaryFilterBase::implCreateStorage( const Reference< XStream >& rxOutStream ) const
#endif	// SUPD == 310
{
    return StorageRef( new ::oox::ole::OleStorage( getComponentContext(), rxOutStream, true ) );
}

// ============================================================================

} // namespace core
} // namespace oox
