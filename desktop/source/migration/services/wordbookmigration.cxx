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

#include "wordbookmigration.hxx"
#include <cppuhelper/supportsservice.hxx>
#include <tools/urlobj.hxx>
#include <unotools/bootstrap.hxx>
#include <unotools/ucbstreamhelper.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;


namespace migration
{
    // component operations


    OUString WordbookMigration_getImplementationName()
    {
        return OUString( "com.sun.star.comp.desktop.migration.Wordbooks" );
    }


    Sequence< OUString > WordbookMigration_getSupportedServiceNames()
    {
        return Sequence< OUString > { "com.sun.star.migration.Wordbooks" };
    }


    // WordbookMigration


    WordbookMigration::WordbookMigration()
    {
    }


    WordbookMigration::~WordbookMigration()
    {
    }


    TStringVectorPtr WordbookMigration::getFiles( const OUString& rBaseURL ) const
    {
        TStringVectorPtr aResult( new TStringVector );
        ::osl::Directory aDir( rBaseURL);

        if ( aDir.open() == ::osl::FileBase::E_None )
        {
            // iterate over directory content
            TStringVector aSubDirs;
            ::osl::DirectoryItem aItem;
            while ( aDir.getNextItem( aItem ) == ::osl::FileBase::E_None )
            {
                ::osl::FileStatus aFileStatus( osl_FileStatus_Mask_Type | osl_FileStatus_Mask_FileURL );
                if ( aItem.getFileStatus( aFileStatus ) == ::osl::FileBase::E_None )
                {
                    if ( aFileStatus.getFileType() == ::osl::FileStatus::Directory )
                        aSubDirs.push_back( aFileStatus.getFileURL() );
                    else
                        aResult->push_back( aFileStatus.getFileURL() );
                }
            }

            // iterate recursive over subfolders
            TStringVector::const_iterator aI = aSubDirs.begin();
            while ( aI != aSubDirs.end() )
            {
                TStringVectorPtr aSubResult = getFiles( *aI );
                aResult->insert( aResult->end(), aSubResult->begin(), aSubResult->end() );
                ++aI;
            }
        }

        return aResult;
    }


    void WordbookMigration::checkAndCreateDirectory( INetURLObject& rDirURL )
    {
        ::osl::FileBase::RC aResult = ::osl::Directory::create( rDirURL.GetMainURL( INetURLObject::DecodeMechanism::ToIUri ) );
        if ( aResult == ::osl::FileBase::E_NOENT )
        {
            INetURLObject aBaseURL( rDirURL );
            aBaseURL.removeSegment();
            checkAndCreateDirectory( aBaseURL );
            ::osl::Directory::create( rDirURL.GetMainURL( INetURLObject::DecodeMechanism::ToIUri ) );
        }
#ifdef USE_JAVA
        else
        {
            // Fix bug 1544 by ensuring that destination directory is
            // readable, writable, and executable
            ::osl::FileStatus aDirStatus( osl_FileStatus_Mask_Attributes );
            ::osl::DirectoryItem aDirItem;
            ::osl::DirectoryItem::get( rDirURL.GetMainURL( INetURLObject::DecodeMechanism::ToIUri ), aDirItem );
            aDirItem.getFileStatus( aDirStatus );
            ::osl::File::setAttributes( rDirURL.GetMainURL( INetURLObject::DecodeMechanism::ToIUri ), osl_File_Attribute_OwnRead | osl_File_Attribute_OwnWrite | osl_File_Attribute_OwnExe | aDirStatus.getAttributes() );
        }
#endif	// USE_JAVA
    }

#define MAX_HEADER_LENGTH 16
bool IsUserWordbook( const OUString& rFile )
{
    bool bRet = false;
    SvStream* pStream = ::utl::UcbStreamHelper::CreateStream( rFile, StreamMode::STD_READ );
    if ( pStream && !pStream->GetError() )
    {
        static const sal_Char* const pVerOOo7    = "OOoUserDict1";
        sal_uInt64 const nSniffPos = pStream->Tell();
        static std::size_t nVerOOo7Len = sal::static_int_cast< std::size_t >(strlen( pVerOOo7 ));
        sal_Char pMagicHeader[MAX_HEADER_LENGTH];
        pMagicHeader[ nVerOOo7Len ] = '\0';
        if ((pStream->ReadBytes(static_cast<void *>(pMagicHeader), nVerOOo7Len) == nVerOOo7Len))
        {
            if ( !strcmp(pMagicHeader, pVerOOo7) )
                bRet = true;
            else
            {
                sal_uInt16 nLen;
                pStream->Seek (nSniffPos);
                pStream->ReadUInt16( nLen );
                if ( nLen < MAX_HEADER_LENGTH )
                {
                   pStream->ReadBytes(pMagicHeader, nLen);
                   pMagicHeader[nLen] = '\0';
                    if ( !strcmp(pMagicHeader, "WBSWG2")
                     ||  !strcmp(pMagicHeader, "WBSWG5")
                     ||  !strcmp(pMagicHeader, "WBSWG6") )
                    bRet = true;
                }
            }
        }
    }

    delete pStream;
    return bRet;
}


    void WordbookMigration::copyFiles()
    {
        OUString sTargetDir;
        ::utl::Bootstrap::PathStatus aStatus = ::utl::Bootstrap::locateUserInstallation( sTargetDir );
        if ( aStatus == ::utl::Bootstrap::PATH_EXISTS )
        {
            sTargetDir += "/user/wordbook";
            TStringVectorPtr aFileList = getFiles( m_sSourceDir );
            TStringVector::const_iterator aI = aFileList->begin();
            while ( aI != aFileList->end() )
            {
                if (IsUserWordbook(*aI) )
                {
                    OUString sSourceLocalName = aI->copy( m_sSourceDir.getLength() );
                    OUString sTargetName = sTargetDir + sSourceLocalName;
                    INetURLObject aURL( sTargetName );
                    aURL.removeSegment();
                    checkAndCreateDirectory( aURL );
                    ::osl::FileBase::RC aResult = ::osl::File::copy( *aI, sTargetName );
                    if ( aResult != ::osl::FileBase::E_None )
                    {
                        OString aMsg = "WordbookMigration::copyFiles: cannot copy "
                                     + OUStringToOString( *aI, RTL_TEXTENCODING_UTF8 )
                                     + " to "
                                     + OUStringToOString( sTargetName, RTL_TEXTENCODING_UTF8 );
                        OSL_FAIL( aMsg.getStr() );
                    }
#ifdef USE_JAVA
                    else
                    {
                        // Fix bug 1544 by ensuring that destination file is
                        // readable and writable
                        ::osl::FileStatus aTargetFileStatus( osl_FileStatus_Mask_Attributes );
                        ::osl::DirectoryItem aDirItem;
                        ::osl::DirectoryItem::get( sTargetName, aDirItem );
                        aDirItem.getFileStatus( aTargetFileStatus );
                        ::osl::File::setAttributes( sTargetName, osl_File_Attribute_OwnRead | osl_File_Attribute_OwnWrite | aTargetFileStatus.getAttributes() );
                    }
#endif	// USE_JAVA
                }
                ++aI;
            }
        }
        else
        {
            OSL_FAIL( "WordbookMigration::copyFiles: no user installation!" );
        }
    }


    // XServiceInfo


    OUString WordbookMigration::getImplementationName()
    {
        return WordbookMigration_getImplementationName();
    }


    sal_Bool WordbookMigration::supportsService(OUString const & ServiceName)
    {
        return cppu::supportsService(this, ServiceName);
    }


    Sequence< OUString > WordbookMigration::getSupportedServiceNames()
    {
        return WordbookMigration_getSupportedServiceNames();
    }


    // XInitialization


    void WordbookMigration::initialize( const Sequence< Any >& aArguments )
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        const Any* pIter = aArguments.getConstArray();
        const Any* pEnd = pIter + aArguments.getLength();
        for ( ; pIter != pEnd ; ++pIter )
        {
            beans::NamedValue aValue;
            *pIter >>= aValue;
            if ( aValue.Name == "UserData" )
            {
                if ( !(aValue.Value >>= m_sSourceDir) )
                {
                    OSL_FAIL( "WordbookMigration::initialize: argument UserData has wrong type!" );
                }
                m_sSourceDir += "/user/wordbook";
                break;
            }
        }
    }


    // XJob


    Any WordbookMigration::execute( const Sequence< beans::NamedValue >& )
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        copyFiles();

        return Any();
    }


    // component operations


    Reference< XInterface > SAL_CALL WordbookMigration_create(
        Reference< XComponentContext > const & )
    {
        return static_cast< lang::XTypeProvider * >( new WordbookMigration() );
    }


}   // namespace migration


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
