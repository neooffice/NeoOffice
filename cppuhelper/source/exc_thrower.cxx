/*************************************************************************
 *
 *  $RCSfile$
 *
 *  $Revision$
 *
 *  last change: $Author$ $Date$
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU General Public License Version 2.1.
 *
 *
 *    GNU General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 *    Modified May 2006 by Patrick Luby. NeoOffice is distributed under
 *    GPL only under modification term 3 of the LGPL.
 *
 ************************************************************************/

#include "osl/diagnose.h"
#include "osl/doublecheckedlocking.h"
#include "osl/mutex.hxx"
#include "uno/dispatcher.hxx"
#include "uno/mapping.hxx"
#include "cppuhelper/detail/XExceptionThrower.hpp"
#include "com/sun/star/uno/RuntimeException.hpp"

#define OUSTR(x) ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(x) )


using namespace ::rtl;
using namespace ::osl;
using namespace ::cppu;
using namespace ::com::sun::star::uno;

namespace
{

using cppuhelper::detail::XExceptionThrower;

//==============================================================================
struct ExceptionThrower : public uno_Interface, XExceptionThrower
{
    inline ExceptionThrower();
    
public:
    static ExceptionThrower * get();
    static inline Type const & getCppuType()
    {
        return ::getCppuType(
            reinterpret_cast< Reference< XExceptionThrower > const * >(0) );
    }
    
    // XInterface
    virtual Any SAL_CALL queryInterface( Type const & type )
        throw (RuntimeException);
    virtual void SAL_CALL acquire() throw ();
    virtual void SAL_CALL release() throw ();
    
    // XExceptionThrower
    virtual void SAL_CALL throwException( Any const & exc ) throw (Exception);
    virtual void SAL_CALL rethrowException() throw (Exception);
};

extern "C"
{

//------------------------------------------------------------------------------
static void SAL_CALL ExceptionThrower_acquire_release_nop( uno_Interface * )
{
}

//------------------------------------------------------------------------------
static void SAL_CALL ExceptionThrower_dispatch(
    uno_Interface * pUnoI, typelib_TypeDescription const * pMemberType,
    void * pReturn, void * pArgs [], uno_Any ** ppException )
{
    OSL_ASSERT( pMemberType->eTypeClass == typelib_TypeClass_INTERFACE_METHOD );
    
    switch (reinterpret_cast< typelib_InterfaceMemberTypeDescription * >(
                const_cast< typelib_TypeDescription * >( pMemberType ) )->
            nPosition)
    {
    case 0: // queryInterace()
    {
        Type const & rType_demanded =
            *reinterpret_cast< Type const * >( pArgs[ 0 ] );
        if (rType_demanded.equals(
                ::getCppuType( reinterpret_cast<
                               Reference< XInterface > const * >(0) ) ) ||
            rType_demanded.equals( ExceptionThrower::getCppuType() ))
        {
            typelib_TypeDescription * pTD = 0;
            TYPELIB_DANGER_GET( &pTD, rType_demanded.getTypeLibType() );
            uno_any_construct(
                reinterpret_cast< uno_Any * >( pReturn ), &pUnoI, pTD, 0 );
            TYPELIB_DANGER_RELEASE( pTD );
        }
        else
        {
            uno_any_construct(
                reinterpret_cast< uno_Any * >( pReturn ), 0, 0, 0 );
        }
        *ppException = 0;
        break;
    }
    case 1: // acquire()
    case 2: // release()
        *ppException = 0;
        break;
    case 3: // throwException()
    {
        uno_Any * pAny = reinterpret_cast< uno_Any * >( pArgs[ 0 ] );
        OSL_ASSERT( pAny->pType->eTypeClass == typelib_TypeClass_EXCEPTION );
        uno_type_any_construct( *ppException, pAny->pData, pAny->pType, 0 );
        break;
    }
    default:
    {
        OSL_ASSERT( 0 );
        RuntimeException exc(
            OUSTR("not implemented!"), Reference< XInterface >() );
        uno_type_any_construct(
            *ppException, &exc, ::getCppuType( &exc ).getTypeLibType(), 0 );
        break;
    }
    }
}

} // extern "C"

//______________________________________________________________________________
Any ExceptionThrower::queryInterface( Type const & type )
    throw (RuntimeException)
{
    if (type.equals( ::getCppuType( reinterpret_cast<
                                    Reference< XInterface > const * >(0) ) ) ||
        type.equals( ExceptionThrower::getCppuType() ))
    {
        XExceptionThrower * that = static_cast< XExceptionThrower * >( this );
        return Any( &that, type );
    }
    return Any();
}

//______________________________________________________________________________
void ExceptionThrower::acquire() throw ()
{
}
//______________________________________________________________________________
void ExceptionThrower::release() throw ()
{
}

//______________________________________________________________________________
void ExceptionThrower::throwException( Any const & exc ) throw (Exception)
{
    OSL_ENSURE( 0, "unexpected!" );
    throwException( exc );
}

#include <stdio.h>
//______________________________________________________________________________
void ExceptionThrower::rethrowException() throw (Exception)
{
#if defined MACOSX && __GNUC__ < 4
    try
    {
        throw;
    }
    catch ( Exception& exc )
    {
        throw;
    }
    catch ( ... )
    {
        throw RuntimeException( OUSTR( "unknown exception" ), Reference< XInterface >() );
    }
#else	// MACOSX && __GNUC__ < 4
    throw;
#endif	// MACOSX && __GNUC__ < 4
}

//______________________________________________________________________________
inline ExceptionThrower::ExceptionThrower()
{
    uno_Interface::acquire = ExceptionThrower_acquire_release_nop;
    uno_Interface::release = ExceptionThrower_acquire_release_nop;
    uno_Interface::pDispatcher = ExceptionThrower_dispatch;
}

//______________________________________________________________________________
ExceptionThrower * ExceptionThrower::get()
{
    ExceptionThrower * s_pThrower = 0;
    if (s_pThrower == 0)
    {
        MutexGuard guard( Mutex::getGlobalMutex() );
        static ExceptionThrower s_thrower;
        OSL_DOUBLE_CHECKED_LOCKING_MEMORY_BARRIER();
        s_pThrower = &s_thrower;
    }
    else
    {
        OSL_DOUBLE_CHECKED_LOCKING_MEMORY_BARRIER();
    }
    return s_pThrower;
}

} // anonymous namespace


namespace cppu
{

//==============================================================================
void SAL_CALL throwException( Any const & exc ) SAL_THROW( (Exception) )
{
    if (exc.getValueTypeClass() != TypeClass_EXCEPTION)
    {
        throw RuntimeException(
            OUSTR("no UNO exception given "
                  "(must be derived from com::sun::star::uno::Exception)!"),
            Reference< XInterface >() );
    }
    
    Mapping uno2cpp(
        OUSTR(UNO_LB_UNO), OUSTR(CPPU_CURRENT_LANGUAGE_BINDING_NAME) );
    if (! uno2cpp.is())
    {
        throw RuntimeException(
            OUSTR("cannot get binary UNO to C++ mapping!"),
            Reference< XInterface >() );
    }
    
    Reference< XExceptionThrower > xThrower;
    uno2cpp.mapInterface(
        reinterpret_cast< void ** >( &xThrower ),
        static_cast< uno_Interface * >( ExceptionThrower::get() ),
        ExceptionThrower::getCppuType() );
    OSL_ASSERT( xThrower.is() );
    xThrower->throwException( exc );
}

//==============================================================================
Any SAL_CALL getCaughtException()
{
    Mapping cpp2uno(
        OUSTR(CPPU_CURRENT_LANGUAGE_BINDING_NAME), OUSTR(UNO_LB_UNO) );
    if (! cpp2uno.is())
    {
        throw RuntimeException(
            OUSTR("cannot get C++ to binary UNO mapping!"),
            Reference< XInterface >() );
    }
    Mapping uno2cpp(
        OUSTR(UNO_LB_UNO), OUSTR(CPPU_CURRENT_LANGUAGE_BINDING_NAME) );
    if (! uno2cpp.is())
    {
        throw RuntimeException(
            OUSTR("cannot get binary UNO to C++ mapping!"),
            Reference< XInterface >() );
    }
    
    typelib_TypeDescription * pTD = 0;
    TYPELIB_DANGER_GET(
        &pTD, ExceptionThrower::getCppuType().getTypeLibType() );
    
    UnoInterfaceReference unoI;
    cpp2uno.mapInterface(
        reinterpret_cast< void ** >( &unoI.m_pUnoI ),
        static_cast< XExceptionThrower * >( ExceptionThrower::get() ), pTD );
    OSL_ASSERT( unoI.is() );

    typelib_TypeDescription * pMemberTD = 0;
    TYPELIB_DANGER_GET(
        &pMemberTD,
        reinterpret_cast< typelib_InterfaceTypeDescription * >( pTD )->
        ppMembers[ 1 ] /* rethrowException() */ );
    
    uno_Any exc_mem;
    uno_Any * exc = &exc_mem;
    unoI.dispatch( pMemberTD, 0, 0, &exc );
    
    TYPELIB_DANGER_RELEASE( pMemberTD );
    TYPELIB_DANGER_RELEASE( pTD );
    
    if (exc == 0)
    {
        throw RuntimeException(
            OUSTR("rethrowing C++ exception failed!"),
            Reference< XInterface >() );
    }
    
    Any ret;
    uno_any_destruct( &ret, cpp_release );
    uno_type_any_constructAndConvert(
        &ret, exc->pData, exc->pType, uno2cpp.get() );
    uno_any_destruct( exc, 0 );
    return ret;
}

}
