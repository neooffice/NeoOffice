/*************************************************************************
 *
 *  $RCSfile$
 *
 *  $Revision$
 *
 *  last change: $Author$ $Date$
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU General Public License Version 2.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *  
 *  =================================================
 *  Modified March 2005 by Patrick Luby. SISSL Removed. NeoOffice is
 *  distributed under GPL only under modification term 3 of the LGPL.
 *
 *  Contributor(s): _______________________________________
 *
 ************************************************************************/

//------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#ifndef _CMDMAILMSG_HXX_
#include "cmdmailmsg.hxx"
#endif

//#############################################################
// <HACK> #110368#
// Mozilla and Co. expect file urls but not UTF8 encoded as we
// do but in the current system encoding, so we have to recode 
// our file urls to this encoding
#ifndef _RTL_URI_H_
#include <rtl/uri.hxx>
#endif

#ifndef _OSL_THREAD_H_
#include <osl/thread.h>
#endif

#ifndef _OSL_FILE_H_
#include <osl/file.h>
#endif

/* a slightly modified version of Pchar in rtl/source/uri.c */
const sal_Bool uriCharClass[128] =  
{ 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* Pchar but without encoding slashes */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 1, 0, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /* !"#$%&'()*+,-./  */
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 0, 0, /* 0123456789:;<=>? */
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /* @ABCDEFGHIJKLMNO */
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, /* PQRSTUVWXYZ[\]^_ */
  0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /* `abcdefghijklmno */
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 0  /* pqrstuvwxyz{|}~  */
};

//-------------------------------------
rtl::OUString reencode_file_url(
    const rtl::OUString& file_url, 
    rtl_TextEncoding from_textenc, 
    rtl_TextEncoding to_textenc)
{
    rtl::OUString tmp = rtl::Uri::decode(
        file_url, rtl_UriDecodeWithCharset, from_textenc);
    
    return rtl::Uri::encode(
        tmp, uriCharClass, rtl_UriEncodeIgnoreEscapes, to_textenc);
}

//-------------------------------------
void reencode_file_url_list(/*inout*/ com::sun::star::uno::Sequence<rtl::OUString>& file_url_list)
{
    sal_uInt32 nmax = file_url_list.getLength();
    for (sal_uInt32 i = 0; i < nmax; i++)
        file_url_list[i] = reencode_file_url(
            file_url_list[i], RTL_TEXTENCODING_UTF8, osl_getThreadTextEncoding());   
}
//
// </HACK> #110368#
//#############################################################


//------------------------------------------------------------------------
// namespace directives
//------------------------------------------------------------------------

using com::sun::star::lang::IllegalArgumentException;
using com::sun::star::lang::WrappedTargetException;
using com::sun::star::container::NoSuchElementException;
using com::sun::star::container::XNameAccess;
using rtl::OUString;
using osl::MutexGuard;

using namespace cppu;
using namespace com::sun::star::uno;


//------------------------------------------------
// 
//------------------------------------------------

void SAL_CALL CmdMailMsg::setRecipient( const ::rtl::OUString& aRecipient ) 
    throw (RuntimeException)
{
    MutexGuard aGuard( m_aMutex );
    m_aRecipient = aRecipient;
}

//------------------------------------------------
// 
//------------------------------------------------

::rtl::OUString SAL_CALL CmdMailMsg::getRecipient(  ) 
    throw (RuntimeException)
{
    MutexGuard aGuard( m_aMutex );
    return m_aRecipient;
}

//------------------------------------------------
// 
//------------------------------------------------ 

void SAL_CALL CmdMailMsg::setCcRecipient( const Sequence< OUString >& aCcRecipient ) 
    throw (RuntimeException)
{
    MutexGuard aGuard( m_aMutex );
    m_CcRecipients = aCcRecipient;
}

//------------------------------------------------
// 
//------------------------------------------------

Sequence< OUString > SAL_CALL CmdMailMsg::getCcRecipient(  ) 
    throw (RuntimeException)
{
    MutexGuard aGuard( m_aMutex );
    return m_CcRecipients;
}

//------------------------------------------------
// 
//------------------------------------------------ 

void SAL_CALL CmdMailMsg::setBccRecipient( const Sequence< OUString >& aBccRecipient ) 
    throw (RuntimeException)
{
    MutexGuard aGuard( m_aMutex );
    m_BccRecipients = aBccRecipient;
}

//------------------------------------------------
// 
//------------------------------------------------

Sequence< OUString > SAL_CALL CmdMailMsg::getBccRecipient(  ) 
    throw (RuntimeException)
{
    MutexGuard aGuard( m_aMutex );
    return m_BccRecipients;
}
 
//------------------------------------------------
// 
//------------------------------------------------ 

void SAL_CALL CmdMailMsg::setOriginator( const OUString& aOriginator ) 
    throw (RuntimeException)
{
    MutexGuard aGuard( m_aMutex );
    m_aOriginator = aOriginator;
}

//------------------------------------------------
// 
//------------------------------------------------

OUString SAL_CALL CmdMailMsg::getOriginator(  ) 
    throw (RuntimeException)
{
    MutexGuard aGuard( m_aMutex );
    return m_aOriginator;
}
    
//------------------------------------------------
// 
//------------------------------------------------ 

void SAL_CALL CmdMailMsg::setSubject( const OUString& aSubject ) 
    throw (RuntimeException)
{
    MutexGuard aGuard( m_aMutex );
    m_aSubject = aSubject;
}

//------------------------------------------------
// 
//------------------------------------------------

OUString SAL_CALL CmdMailMsg::getSubject(  ) 
    throw (RuntimeException)
{
    MutexGuard aGuard( m_aMutex );
    return m_aSubject;
}

//------------------------------------------------
// 
//------------------------------------------------ 

void SAL_CALL CmdMailMsg::setAttachement( const Sequence< ::rtl::OUString >& aAttachment ) 
    throw (IllegalArgumentException, RuntimeException)
{
    MutexGuard aGuard( m_aMutex );
    m_Attachments = aAttachment;
#if defined MACOSX && ! defined USE_JAVA
    sal_uInt32 nmax = m_Attachments.getLength();
    for (sal_uInt32 i = 0; i < nmax; i++) {
        OUString tmp;
        osl_getSystemPathFromFileURL(m_Attachments[i].pData, &tmp.pData);
        tmp = rtl::OUString::createFromAscii("\"") + tmp + rtl::OUString::createFromAscii("\"");
        m_Attachments[i] = tmp;
    }
#else
    //#######################################
    //#110368#
    reencode_file_url_list(m_Attachments);
    //#110368#
    //#######################################
#endif
}

//------------------------------------------------
// 
//------------------------------------------------

Sequence< OUString > SAL_CALL CmdMailMsg::getAttachement(  ) 
    throw (RuntimeException)
{
    MutexGuard aGuard( m_aMutex );
    return m_Attachments;
}

//------------------------------------------------
// 
//------------------------------------------------

Any SAL_CALL CmdMailMsg::getByName( const OUString& aName ) 
    throw (NoSuchElementException, WrappedTargetException, RuntimeException)
{
    MutexGuard aGuard( m_aMutex );

    if( 0 == aName.compareToAscii( "from" ) &&  m_aOriginator.getLength() )
        return makeAny( m_aOriginator );

    else if( 0 == aName.compareToAscii( "to" ) &&  m_aRecipient.getLength() )
        return makeAny( m_aRecipient );

    else if( 0 == aName.compareToAscii( "cc" ) &&  m_CcRecipients.getLength() )
        return makeAny( m_CcRecipients );
        
    else if( 0 == aName.compareToAscii( "bcc" ) &&  m_BccRecipients.getLength() )
        return makeAny( m_BccRecipients );
        
    else if( 0 == aName.compareToAscii( "subject" ) &&  m_aSubject.getLength() )
        return makeAny( m_aSubject );
        
    else if( 0 == aName.compareToAscii( "attachment" ) &&  m_Attachments.getLength() )
        return makeAny( m_Attachments );
        
   throw NoSuchElementException( OUString::createFromAscii( "key not found: ") + aName,
        static_cast < XNameAccess * > (this) );
}

//------------------------------------------------
// 
//------------------------------------------------

Sequence< OUString > SAL_CALL CmdMailMsg::getElementNames(  ) 
    throw (::com::sun::star::uno::RuntimeException)
{
    MutexGuard aGuard( m_aMutex );
    
    sal_Int32 nItems = 0;
    Sequence< OUString > aRet( 6 );
    
    if( m_aOriginator.getLength() )
        aRet[nItems++] = OUString::createFromAscii( "from" );

    if( m_aRecipient.getLength() )
        aRet[nItems++] = OUString::createFromAscii( "to" );

    if( m_CcRecipients.getLength() )
        aRet[nItems++] = OUString::createFromAscii( "cc" );

    if( m_BccRecipients.getLength() )
        aRet[nItems++] = OUString::createFromAscii( "bcc" );

    if( m_aSubject.getLength() )
        aRet[nItems++] = OUString::createFromAscii( "subject" );

    if( m_Attachments.getLength() )
        aRet[nItems++] = OUString::createFromAscii( "attachment" );

    aRet.realloc( nItems );
    return aRet;    
}

//------------------------------------------------
// 
//------------------------------------------------

 sal_Bool SAL_CALL CmdMailMsg::hasByName( const ::rtl::OUString& aName ) 
    throw (RuntimeException)
{
    MutexGuard aGuard( m_aMutex );

    if( 0 == aName.compareToAscii( "from" ) &&  m_aOriginator.getLength() )
        return sal_True;

    else if( 0 == aName.compareToAscii( "to" ) &&  m_aRecipient.getLength() )
        return sal_True;

    else if( 0 == aName.compareToAscii( "cc" ) &&  m_CcRecipients.getLength() )
        return sal_True;
        
    else if( 0 == aName.compareToAscii( "bcc" ) &&  m_BccRecipients.getLength() )
        return sal_True;
        
    else if( 0 == aName.compareToAscii( "subject" ) &&  m_aSubject.getLength() )
        return sal_True;
        
    else if( 0 == aName.compareToAscii( "attachment" ) &&  m_Attachments.getLength() )
        return sal_True;
        
    return sal_False;
}

//------------------------------------------------
// 
//------------------------------------------------

Type SAL_CALL CmdMailMsg::getElementType(  ) 
    throw (RuntimeException)
{
    // returning void for multi type container
    return Type();
}

//------------------------------------------------
// 
//------------------------------------------------

sal_Bool SAL_CALL CmdMailMsg::hasElements(  ) 
    throw (RuntimeException)
{
    return 0 != getElementNames().getLength();
}
