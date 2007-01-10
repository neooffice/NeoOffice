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
 *    Modified June 2006 by Patrick Luby. NeoOffice is distributed under
 *    GPL only under modification term 3 of the LGPL.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_desktop.hxx"

#ifndef _DESKTOP_BASICMIGRATION_HXX_
#include "basicmigration.hxx"
#endif

#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _UTL_BOOTSTRAP_HXX
#include <unotools/bootstrap.hxx>
#endif


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;


//.........................................................................
namespace migration
{
//.........................................................................


    static ::rtl::OUString sSourceUserBasic = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "/user/basic" ) );
    static ::rtl::OUString sTargetUserBasic = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "/user/__basic_70" ) );


    // =============================================================================
    // component operations
    // =============================================================================

    ::rtl::OUString BasicMigration_getImplementationName()
    {
        static ::rtl::OUString* pImplName = 0;
	    if ( !pImplName )
	    {
            ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
            if ( !pImplName )
		    {
                static ::rtl::OUString aImplName( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.desktop.migration.Basic" ) );
			    pImplName = &aImplName;
		    }
	    }
	    return *pImplName;
    }

    // -----------------------------------------------------------------------------

    Sequence< ::rtl::OUString > BasicMigration_getSupportedServiceNames()
    {
        static Sequence< ::rtl::OUString >* pNames = 0;
	    if ( !pNames )
	    {
            ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
		    if ( !pNames )
		    {
                static Sequence< ::rtl::OUString > aNames(1);
                aNames.getArray()[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.migration.Basic" ) );
                pNames = &aNames;
		    }
	    }
	    return *pNames;
    }

    // =============================================================================
    // BasicMigration
    // =============================================================================

    BasicMigration::BasicMigration()
    {
    }

    // -----------------------------------------------------------------------------

    BasicMigration::~BasicMigration()
    {
    }

    // -----------------------------------------------------------------------------

    TStringVectorPtr BasicMigration::getFiles( const ::rtl::OUString& rBaseURL ) const
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
                ::osl::FileStatus aFileStatus( FileStatusMask_Type | FileStatusMask_FileURL );
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

    // -----------------------------------------------------------------------------

    ::osl::FileBase::RC BasicMigration::checkAndCreateDirectory( INetURLObject& rDirURL )
    {
        ::osl::FileBase::RC aResult = ::osl::Directory::create( rDirURL.GetMainURL( INetURLObject::DECODE_TO_IURI ) );
        if ( aResult == ::osl::FileBase::E_NOENT )
        {
            INetURLObject aBaseURL( rDirURL );
            aBaseURL.removeSegment();
            checkAndCreateDirectory( aBaseURL );
            return ::osl::Directory::create( rDirURL.GetMainURL( INetURLObject::DECODE_TO_IURI ) );
        }
        else
        {
#ifdef USE_JAVA
            // Fix bug 1544 by ensuring that destination directory is
            // readable, writable, and executable
            ::osl::FileStatus aDirStatus( FileStatusMask_Attributes );
            ::osl::DirectoryItem aDirItem;
            ::osl::DirectoryItem::get( rDirURL.GetMainURL( INetURLObject::DECODE_TO_IURI ), aDirItem );
            aDirItem.getFileStatus( aDirStatus );
            ::osl::File::setAttributes( rDirURL.GetMainURL( INetURLObject::DECODE_TO_IURI ), Attribute_OwnRead | Attribute_OwnWrite | Attribute_OwnExe | aDirStatus.getAttributes() );
#endif	// USE_JAVA

            return aResult;
        }
    }       

    // -----------------------------------------------------------------------------

    void BasicMigration::copyFiles()
    {
        ::rtl::OUString sTargetDir;
        ::utl::Bootstrap::PathStatus aStatus = ::utl::Bootstrap::locateUserInstallation( sTargetDir );
        if ( aStatus == ::utl::Bootstrap::PATH_EXISTS )
        {
            sTargetDir += sTargetUserBasic;
            TStringVectorPtr aFileList = getFiles( m_sSourceDir );
            TStringVector::const_iterator aI = aFileList->begin();
            while ( aI != aFileList->end() )
            {                
                ::rtl::OUString sLocalName = aI->copy( m_sSourceDir.getLength() );
                ::rtl::OUString sTargetName = sTargetDir + sLocalName;
                INetURLObject aURL( sTargetName );
                aURL.removeSegment();
                checkAndCreateDirectory( aURL );            
                ::osl::FileBase::RC aResult = ::osl::File::copy( *aI, sTargetName );
                if ( aResult != ::osl::FileBase::E_None )
                {
                    ::rtl::OString aMsg( "BasicMigration::copyFiles: cannot copy " );
                    aMsg += ::rtl::OUStringToOString( *aI, RTL_TEXTENCODING_UTF8 ) + " to "
                         +  ::rtl::OUStringToOString( sTargetName, RTL_TEXTENCODING_UTF8 );
                    OSL_ENSURE( sal_False, aMsg.getStr() );
                }
#ifdef USE_JAVA
                else
                {
                    // Fix bug 1544 by ensuring that destination file is
                    // readable and writable
                    ::osl::FileStatus aTargetFileStatus( FileStatusMask_Attributes );
                    ::osl::DirectoryItem aDirItem;
                    ::osl::DirectoryItem::get( sTargetName, aDirItem );
                    aDirItem.getFileStatus( aTargetFileStatus );
                    ::osl::File::setAttributes( sTargetName, Attribute_OwnRead | Attribute_OwnWrite | aTargetFileStatus.getAttributes() );
                }
#endif	// USE_JAVA
                ++aI;
            }
        } 
        else
        {
            OSL_ENSURE( sal_False, "BasicMigration::copyFiles: no user installation!" );
        }
    }

    // -----------------------------------------------------------------------------
    // XServiceInfo
    // -----------------------------------------------------------------------------

    ::rtl::OUString BasicMigration::getImplementationName() throw (RuntimeException)
    {
        return BasicMigration_getImplementationName();
    }

    // -----------------------------------------------------------------------------

    sal_Bool BasicMigration::supportsService( const ::rtl::OUString& rServiceName ) throw (RuntimeException)
    {
	    Sequence< ::rtl::OUString > aNames( getSupportedServiceNames() );
	    const ::rtl::OUString* pNames = aNames.getConstArray();
	    const ::rtl::OUString* pEnd = pNames + aNames.getLength();
	    for ( ; pNames != pEnd && !pNames->equals( rServiceName ); ++pNames )
		    ;

	    return pNames != pEnd;
    }

    // -----------------------------------------------------------------------------

    Sequence< ::rtl::OUString > BasicMigration::getSupportedServiceNames() throw (RuntimeException)
    {
        return BasicMigration_getSupportedServiceNames();
    }

    // -----------------------------------------------------------------------------
    // XInitialization
    // -----------------------------------------------------------------------------

    void BasicMigration::initialize( const Sequence< Any >& aArguments ) throw (Exception, RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        const Any* pIter = aArguments.getConstArray();
        const Any* pEnd = pIter + aArguments.getLength();
        for ( ; pIter != pEnd ; ++pIter )
        {
            beans::NamedValue aValue;
            *pIter >>= aValue;
            if ( aValue.Name.equalsAscii( "UserData" ) )
            {
                if ( !(aValue.Value >>= m_sSourceDir) )
                {
                    OSL_ENSURE( false, "BasicMigration::initialize: argument UserData has wrong type!" );
                }
                m_sSourceDir += sSourceUserBasic;
                break;
            }
        }
    }

    // -----------------------------------------------------------------------------
    // XJob
    // -----------------------------------------------------------------------------

    Any BasicMigration::execute( const Sequence< beans::NamedValue >& )
        throw (lang::IllegalArgumentException, Exception, RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        copyFiles();

        return Any();
    }

    // =============================================================================
    // component operations
    // =============================================================================

    Reference< XInterface > SAL_CALL BasicMigration_create(
        Reference< XComponentContext > const & )
        SAL_THROW( () )
    {
        return static_cast< lang::XTypeProvider * >( new BasicMigration() );
    }

    // -----------------------------------------------------------------------------

//.........................................................................
}	// namespace migration
//.........................................................................
