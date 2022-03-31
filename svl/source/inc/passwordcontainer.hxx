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
 */
#ifndef INCLUDED_SVL_SOURCE_INC_PASSWORDCONTAINER_HXX
#define INCLUDED_SVL_SOURCE_INC_PASSWORDCONTAINER_HXX

#include <list>
#include <vector>
#include <map>
#include <com/sun/star/task/XPasswordContainer2.hpp>
#include <com/sun/star/task/PasswordRequestMode.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <cppuhelper/implbase3.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/factory.hxx>

#include <tools/stream.hxx>
#include <unotools/configitem.hxx>
#include <ucbhelper/interactionrequest.hxx>

#ifndef NO_LIBO_MASTER_PASSWORD_FIX
#include <rtl/random.h>
#endif	// !NO_LIBO_MASTER_PASSWORD_FIX
#include <rtl/ref.hxx>
#include <osl/mutex.hxx>

#include "syscreds.hxx"

#define MEMORY_RECORD         0
#define PERSISTENT_RECORD     1


class NamePassRecord
{
    OUString                                       m_aName;

    // there are two lists of passwords, memory passwords and persistent passwords
    bool                                                  m_bHasMemPass;
    ::std::vector< OUString >                      m_aMemPass;

    // persistent passwords are encrypted in one string
    bool                                                  m_bHasPersPass;
    OUString                                       m_aPersPass;
#ifndef NO_LIBO_MASTER_PASSWORD_FIX
    OUString                    m_aPersistentIV;
#endif	// !NO_LIBO_MASTER_PASSWORD_FIX

    void InitArrays( bool bHasMemoryList, const ::std::vector< OUString >& aMemoryList,
#ifdef NO_LIBO_MASTER_PASSWORD_FIX
                     bool bHasPersistentList, const OUString& aPersistentList )
#else	// NO_LIBO_MASTER_PASSWORD_FIX
                     bool bHasPersistentList, const OUString& aPersistentList, const OUString& aPersistentIV )
#endif	// NO_LIBO_MASTER_PASSWORD_FIX
    {
        m_bHasMemPass = bHasMemoryList;
        if ( bHasMemoryList )
            m_aMemPass = aMemoryList;

        m_bHasPersPass = bHasPersistentList;
        if ( bHasPersistentList )
#ifndef NO_LIBO_MASTER_PASSWORD_FIX
        {
#endif	// !NO_LIBO_MASTER_PASSWORD_FIX
            m_aPersPass = aPersistentList;
#ifndef NO_LIBO_MASTER_PASSWORD_FIX
            m_aPersistentIV = aPersistentIV;
        }
#endif	// !NO_LIBO_MASTER_PASSWORD_FIX
    }

public:

    NamePassRecord( const OUString& aName )
        : m_aName( aName )
        , m_bHasMemPass( false )
        , m_bHasPersPass( false )
    {
    }

    NamePassRecord( const OUString& aName, const ::std::vector< OUString >& aMemoryList )
        : m_aName( aName )
        , m_bHasMemPass( true )
        , m_aMemPass( aMemoryList )
        , m_bHasPersPass( false )
    {
    }

#ifdef NO_LIBO_MASTER_PASSWORD_FIX
    NamePassRecord( const OUString& aName, const OUString& aPersistentList )
#else	// NO_LIBO_MASTER_PASSWORD_FIX
    NamePassRecord( const OUString& aName, const OUString& aPersistentList, const OUString& aPersistentIV )
#endif	// NO_LIBO_MASTER_PASSWORD_FIX
        : m_aName( aName )
        , m_bHasMemPass( false )
        , m_bHasPersPass( true )
        , m_aPersPass( aPersistentList )
#ifndef NO_LIBO_MASTER_PASSWORD_FIX
        , m_aPersistentIV( aPersistentIV )
#endif	// !NO_LIBO_MASTER_PASSWORD_FIX
    {
    }

    NamePassRecord( const OUString& aName,
                    bool bHasMemoryList, const ::std::vector< OUString >& aMemoryList,
#ifdef NO_LIBO_MASTER_PASSWORD_FIX
                    bool bHasPersistentList, const OUString & aPersistentList )
#else	// NO_LIBO_MASTER_PASSWORD_FIX
                    bool bHasPersistentList, const OUString & aPersistentList, const OUString & aPersistentIV )
#endif	// NO_LIBO_MASTER_PASSWORD_FIX
        : m_aName( aName )
        , m_bHasMemPass( bHasMemoryList )
        , m_bHasPersPass( bHasPersistentList )
    {
#ifdef NO_LIBO_MASTER_PASSWORD_FIX
        InitArrays( bHasMemoryList, aMemoryList, bHasPersistentList, aPersistentList );
#else	// NO_LIBO_MASTER_PASSWORD_FIX
        InitArrays( bHasMemoryList, aMemoryList, bHasPersistentList, aPersistentList, aPersistentIV );
#endif	// NO_LIBO_MASTER_PASSWORD_FIX
    }

    NamePassRecord( const NamePassRecord& aRecord )
        : m_aName( aRecord.m_aName )
        , m_bHasMemPass( false )
        , m_bHasPersPass( false )
    {
#ifdef NO_LIBO_MASTER_PASSWORD_FIX
        InitArrays( aRecord.m_bHasMemPass, aRecord.m_aMemPass, aRecord.m_bHasPersPass, aRecord.m_aPersPass );
#else	// NO_LIBO_MASTER_PASSWORD_FIX
        InitArrays( aRecord.m_bHasMemPass, aRecord.m_aMemPass, aRecord.m_bHasPersPass, aRecord.m_aPersPass, aRecord.m_aPersistentIV );
#endif	// NO_LIBO_MASTER_PASSWORD_FIX
    }

    NamePassRecord& operator=( const NamePassRecord& aRecord )
    {
        m_aName = aRecord.m_aName;

        m_aMemPass.clear();
        m_aPersPass.clear();
#ifdef NO_LIBO_MASTER_PASSWORD_FIX
        InitArrays( aRecord.m_bHasMemPass, aRecord.m_aMemPass, aRecord.m_bHasPersPass, aRecord.m_aPersPass );
#else	// NO_LIBO_MASTER_PASSWORD_FIX
        m_aPersistentIV.clear();
        InitArrays( aRecord.m_bHasMemPass, aRecord.m_aMemPass, aRecord.m_bHasPersPass, aRecord.m_aPersPass, aRecord.m_aPersistentIV );
#endif	// NO_LIBO_MASTER_PASSWORD_FIX

        return *this;
    }

    OUString GetUserName() const
    {
        return m_aName;
    }

    bool HasPasswords( sal_Int8 nStatus ) const
    {
        if ( nStatus == MEMORY_RECORD )
            return m_bHasMemPass;
        if ( nStatus == PERSISTENT_RECORD )
            return m_bHasPersPass;

        return false;
    }

    ::std::vector< OUString > GetMemPasswords() const
    {
        if ( m_bHasMemPass )
            return m_aMemPass;

        return ::std::vector< OUString >();
    }

    OUString GetPersPasswords() const
    {
        if ( m_bHasPersPass )
            return m_aPersPass;

        return OUString();
    }

#ifndef NO_LIBO_MASTER_PASSWORD_FIX
    OUString GetPersistentIV() const
    {
        if ( m_bHasPersPass )
            return m_aPersistentIV;

        return OUString();
    }
#endif	// !NO_LIBO_MASTER_PASSWORD_FIX

    void SetMemPasswords( const ::std::vector< OUString >& aMemList )
    {
        m_aMemPass = aMemList;
        m_bHasMemPass = true;
    }

#ifdef NO_LIBO_MASTER_PASSWORD_FIX
    void SetPersPasswords( const OUString& aPersList )
#else	// NO_LIBO_MASTER_PASSWORD_FIX
    void SetPersistentPasswords( const OUString& aPersList, const OUString& aPersIV )
#endif	// NO_LIBO_MASTER_PASSWORD_FIX
    {
        m_aPersPass = aPersList;
        m_bHasPersPass = true;
#ifndef NO_LIBO_MASTER_PASSWORD_FIX
        m_aPersistentIV = aPersIV;
#endif	// !NO_LIBO_MASTER_PASSWORD_FIX
    }

    void RemovePasswords( sal_Int8 nStatus )
    {
        if ( nStatus == MEMORY_RECORD )
        {
            m_bHasMemPass = false;
            m_aMemPass.clear();
        }
        else if ( nStatus == PERSISTENT_RECORD )
        {
            m_bHasPersPass = false;
            m_aPersPass.clear();
#ifndef NO_LIBO_MASTER_PASSWORD_FIX
            m_aPersistentIV.clear();
#endif	// !NO_LIBO_MASTER_PASSWORD_FIX
        }
    }

};


typedef ::std::pair< const OUString, ::std::list< NamePassRecord > > PairUrlRecord;
typedef ::std::map< OUString, ::std::list< NamePassRecord > > PassMap;

#ifndef NO_LIBO_MASTER_PASSWORD_FIX
// org.openoffice.Office.Common/Passwords/StorageVersion bump if details of
// how password details are saved changes. Enables migration from previous
// schemes.
constexpr sal_Int32 nCurrentStorageVersion = 1;
#endif	// !NO_LIBO_MASTER_PASSWORD_FIX

class PasswordContainer;

class StorageItem : public ::utl::ConfigItem {
    PasswordContainer*     mainCont;
    bool                   hasEncoded;
    OUString        mEncoded;
#ifndef NO_LIBO_MASTER_PASSWORD_FIX
    OUString               mEncodedIV;
#endif	// !NO_LIBO_MASTER_PASSWORD_FIX
public:
    StorageItem( PasswordContainer* point, const OUString& path ) :
        ConfigItem( path, CONFIG_MODE_IMMEDIATE_UPDATE ),
        mainCont( point ),
        hasEncoded( false )
    {
        ::com::sun::star::uno::Sequence< OUString > aNode( 1 );
        *aNode.getArray()  = path;
        *aNode.getArray() += "/Store";
        EnableNotification( aNode );
    }

    PassMap getInfo();
    void update( const OUString& url, const NamePassRecord& rec );
    void remove( const OUString& url, const OUString& rec );
    void clear();

#ifdef NO_LIBO_MASTER_PASSWORD_FIX
    bool getEncodedMP( OUString& aResult );
    void setEncodedMP( const OUString& aResult, bool bAcceptEnmpty = false );
#else	// NO_LIBO_MASTER_PASSWORD_FIX
    sal_Int32 getStorageVersion();

    bool getEncodedMasterPassword( OUString& aResult, OUString& aResultIV );
    void setEncodedMasterPassword( const OUString& aResult, const OUString& aResultIV, bool bAcceptEmpty = false );
#endif	// NO_LIBO_MASTER_PASSWORD_FIX
    void setUseStorage( bool bUse );
    bool useStorage();

    virtual void            Notify( const ::com::sun::star::uno::Sequence< OUString >& aPropertyNames ) SAL_OVERRIDE;
    virtual void            Commit() SAL_OVERRIDE;
};


enum PasswordState {
    no_password,
    entered,
    cancelled
};

class PasswordContainer : public ::cppu::WeakImplHelper3<
        ::com::sun::star::task::XPasswordContainer2,
        ::com::sun::star::lang::XServiceInfo,
        ::com::sun::star::lang::XEventListener >
{
private:
    PassMap      m_aContainer;
    StorageItem* m_pStorageFile;
    ::osl::Mutex mMutex;
    OUString m_aMasterPasswd; // master password is set when the string is not empty
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent > mComponent;
    SysCredentialsConfig mUrlContainer;

#ifndef NO_LIBO_MASTER_PASSWORD_FIX
    class RandomPool
    {
    private:
        rtlRandomPool m_aRandomPool;
    public:
        RandomPool() : m_aRandomPool(rtl_random_createPool())
        {
        }
        rtlRandomPool get()
        {
            return m_aRandomPool;
        }
        ~RandomPool()
        {
            // Clean up random pool memory
            rtl_random_destroyPool(m_aRandomPool);
        }
    };

    RandomPool mRandomPool;

    OUString createIV();
#endif	// !NO_LIBO_MASTER_PASSWORD_FIX

    ::com::sun::star::uno::Sequence< ::com::sun::star::task::UserRecord > CopyToUserRecordSequence(
                                        const ::std::list< NamePassRecord >& original,
                                        const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& Handler )
                                                        throw(::com::sun::star::uno::RuntimeException);

    ::com::sun::star::task::UserRecord CopyToUserRecord(
                                        const NamePassRecord& aRecord,
                                        bool& io_bTryToDecode,
                                        const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& aHandler );

    ::com::sun::star::uno::Sequence< ::com::sun::star::task::UserRecord > FindUsr(
                                        const ::std::list< NamePassRecord >& userlist,
                                        const OUString& name,
                                        const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& Handler )
                                                        throw(::com::sun::star::uno::RuntimeException);
bool createUrlRecord(
    const PassMap::iterator & rIter,
    bool bName,
    const OUString & aName,
    const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& aHandler,
    ::com::sun::star::task::UrlRecord & rRec  )
        throw( ::com::sun::star::uno::RuntimeException );

::com::sun::star::task::UrlRecord find(
    const OUString& aURL,
    const OUString& aName,
    bool bName, // only needed to support empty user names
    const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& aHandler  ) throw(::com::sun::star::uno::RuntimeException);

    OUString GetDefaultMasterPassword();

    OUString RequestPasswordFromUser(
                    ::com::sun::star::task::PasswordRequestMode aRMode,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& xHandler );

    OUString GetMasterPassword( const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& Handler )
                                                        throw(::com::sun::star::uno::RuntimeException);

    void UpdateVector( const OUString& url, ::std::list< NamePassRecord >& toUpdate, NamePassRecord& rec, bool writeFile )
                                                        throw(::com::sun::star::uno::RuntimeException);

    void PrivateAdd( const OUString& aUrl,
                              const OUString& aUserName,
                              const ::com::sun::star::uno::Sequence< OUString >& aPasswords,
                              char  aMode,
                              const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& Handler )
                                                        throw(::com::sun::star::uno::RuntimeException);

#ifdef NO_LIBO_MASTER_PASSWORD_FIX
    ::std::vector< OUString > DecodePasswords( const OUString& aLine, const OUString& aMasterPassword )
#else	// NO_LIBO_MASTER_PASSWORD_FIX
    ::std::vector< OUString > DecodePasswords( const OUString& aLine, const OUString& aIV, const OUString& aMasterPassword )
#endif	// NO_LIBO_MASTER_PASSWORD_FIX
                                                        throw(::com::sun::star::uno::RuntimeException);

#ifdef NO_LIBO_MASTER_PASSWORD_FIX
    OUString EncodePasswords( ::std::vector< OUString > lines, const OUString& aMasterPassword )
#else	// NO_LIBO_MASTER_PASSWORD_FIX
    OUString EncodePasswords(const std::vector< OUString >& lines, const OUString& aIV, const OUString& aMasterPassword )
#endif	// NO_LIBO_MASTER_PASSWORD_FIX
                                                        throw(::com::sun::star::uno::RuntimeException);

public:
    PasswordContainer( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& );
    virtual ~PasswordContainer();

    virtual void SAL_CALL add( const OUString& aUrl,
                               const OUString& aUserName,
                               const ::com::sun::star::uno::Sequence< OUString >& aPasswords,
                               const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& Handler  )
                                                        throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL addPersistent( const OUString& aUrl,
                                            const OUString& aUserName,
                                         const ::com::sun::star::uno::Sequence< OUString >& aPasswords,
                                          const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& Handler  )
                                                        throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual ::com::sun::star::task::UrlRecord SAL_CALL
                            find( const OUString& aUrl,
                                  const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& Handler  )
                                                        throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual ::com::sun::star::task::UrlRecord SAL_CALL
                            findForName( const OUString& aUrl,
                                         const OUString& aUserName,
                                            const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& Handler  )
                                                        throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL remove( const OUString& aUrl,
                                  const OUString& aUserName )
                                                        throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL removePersistent( const OUString& aUrl,
                                            const OUString& aUserName )
                                                        throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL removeAllPersistent() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::task::UrlRecord > SAL_CALL
                            getAllPersistent( const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& Handler ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;


    // provide factory
    static OUString SAL_CALL        impl_getStaticImplementationName( ) throw(::com::sun::star::uno::RuntimeException);
    static ::com::sun::star::uno::Sequence< OUString > SAL_CALL
                    impl_getStaticSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);
    static ::com::sun::star::uno::Reference< ::com::sun::star::lang::XSingleServiceFactory > SAL_CALL
                    impl_createFactory( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& ServiceManager ) throw(::com::sun::star::uno::RuntimeException);
    static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
                    impl_createInstance( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceManager ) throw( ::com::sun::star::uno::RuntimeException );

    // XServiceInfo
    virtual OUString    SAL_CALL    getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL            supportsService( const OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL
                                        getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XEventListener
    virtual void SAL_CALL        disposing( const ::com::sun::star::lang::EventObject& Source )
                                    throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XMasterPasswordHandling
    virtual sal_Bool SAL_CALL authorizateWithMasterPassword( const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& xHandler )
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL changeMasterPassword( const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& xHandler ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeMasterPassword() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL hasMasterPassword(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL allowPersistentStoring( sal_Bool bAllow ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL isPersistentStoringAllowed(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XMasterPasswordHandling2
    virtual sal_Bool SAL_CALL useDefaultMasterPassword( const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& xHandler ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL isDefaultMasterPasswordUsed(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XUrlContainer
    virtual void SAL_CALL addUrl( const OUString& Url, sal_Bool MakePersistent ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual OUString SAL_CALL findUrl( const OUString& Url ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeUrl( const OUString& Url ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getUrls( sal_Bool OnlyPersistent ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    void            Notify();
};


class MasterPasswordRequest_Impl : public ucbhelper::InteractionRequest
{
    ::rtl::Reference< ucbhelper::InteractionSupplyAuthentication > m_xAuthSupplier;

public:
    MasterPasswordRequest_Impl( ::com::sun::star::task::PasswordRequestMode Mode );

    const ::rtl::Reference< ucbhelper::InteractionSupplyAuthentication > &
    getAuthenticationSupplier() const { return m_xAuthSupplier; }

};


class RW_SvMemoryStream : public SvMemoryStream {
public:
    RW_SvMemoryStream( void* Buf, sal_uLong Size, StreamMode eMode ):
            SvMemoryStream( Buf, Size, eMode){}

    RW_SvMemoryStream( sal_uLong InitSize=512, sal_uLong Resize=64 ):
            SvMemoryStream( InitSize, Resize ){}

    sal_uLong getActualSize(){ return nEndOfData; }
};



#endif // INCLUDED_SVL_SOURCE_INC_PASSWORDCONTAINER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
