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
 * This file incorporates work covered by the following license notice:
 * 
 *   Modified March 2016 by Patrick Luby. NeoOffice is only distributed
 *   under the GNU General Public License, Version 3 as allowed by Section 4
 *   of the Apache License, Version 2.0.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * 
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_extensions.hxx"
#include <com/sun/star/xml/xpath/XXPathAPI.hpp>

#include "updateprotocol.hxx"
#include "updatecheckconfig.hxx"

#ifndef _COM_SUN_STAR_DEPLOYMENT_UPDATEINFORMATINENTRY_HPP_
#include <com/sun/star/deployment/UpdateInformationEntry.hpp>
#endif
#include <com/sun/star/deployment/XPackageInformationProvider.hpp>


#include <rtl/ref.hxx>
#include <rtl/uri.hxx>
#include <rtl/strbuf.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/bootstrap.hxx>
#include <osl/process.h>

#include <cppuhelper/implbase1.hxx>

#ifdef USE_JAVA

// Uncomment this to disable patch checking for non-admin users
// #define USE_NATIVE_ADMIN_USER_CHECK

#ifdef MACOSX

#include <dlfcn.h>

#include <premac.h>
#include <CoreServices/CoreServices.h>
#include <postmac.h>

typedef OSErr Gestalt_Type( OSType selector, SInt32 *response );

#endif	// MACOSX

#endif	// USE_JAVA

namespace css = com::sun::star ;
namespace container = css::container ;
namespace deployment = css::deployment ;
namespace lang = css::lang ;
namespace uno = css::uno ;
namespace task = css::task ;
namespace xml = css::xml ;

#define UNISTRING(s) rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(s))

#ifdef USE_JAVA

static const rtl::OUString GetOSVersion()
{
    static rtl::OUString aOSVersion;
    static bool nInitialized = false;

    if ( !nInitialized )
    {
#ifdef MACOSX
        void *pLib = dlopen( NULL, RTLD_LAZY | RTLD_LOCAL );
        if ( pLib )
        {
            Gestalt_Type *pGestalt = (Gestalt_Type *)dlsym( pLib, "Gestalt" );
            if ( pGestalt )
            {
                SInt32 nMajor = 0;
                SInt32 nMinor = 0;
                SInt32 nBugFix = 0;
                pGestalt( gestaltSystemVersionMajor, &nMajor);
                pGestalt( gestaltSystemVersionMinor, &nMinor);
                pGestalt( gestaltSystemVersionBugFix, &nBugFix);
                aOSVersion = rtl::OUString::valueOf( (sal_Int32)nMajor );
                aOSVersion += UNISTRING( "." );
                aOSVersion += rtl::OUString::valueOf( (sal_Int32)nMinor );
                aOSVersion += UNISTRING( "." );
                aOSVersion += rtl::OUString::valueOf( (sal_Int32)nBugFix );
            }

            dlclose( pLib );
        }
#endif	// MACOSX

        nInitialized = true;
    }

    return aOSVersion;
}

#endif	// USE_JAVA

//------------------------------------------------------------------------------

static bool 
getBootstrapData(
    uno::Sequence< ::rtl::OUString > & rRepositoryList,
    ::rtl::OUString & rBuildID,
    ::rtl::OUString & rInstallSetID)
{
#ifdef USE_JAVA
    rBuildID = UNISTRING( "${$BRAND_BASE_DIR/program/" SAL_CONFIGFILE("version") ":ProductBuildid}" );
#else	// USE_JAVA
    rBuildID = UNISTRING( "${$OOO_BASE_DIR/program/" SAL_CONFIGFILE("version") ":ProductBuildid}" );
#endif	// USE_JAVA
    rtl::Bootstrap::expandMacros( rBuildID );
    if ( ! rBuildID.getLength() )
        return false;

    rInstallSetID = UNISTRING( "${$OOO_BASE_DIR/program/" SAL_CONFIGFILE("version") ":UpdateID}" );
    rtl::Bootstrap::expandMacros( rInstallSetID );
    if ( ! rInstallSetID.getLength() )
        return false;

    rtl::OUString aValue( UNISTRING( "${$OOO_BASE_DIR/program/" SAL_CONFIGFILE("version") ":UpdateURL}" ) );
    rtl::Bootstrap::expandMacros( aValue );

#ifdef USE_JAVA
    for (sal_Int32 i = 0;;)
    {
        i = aValue.indexOfAsciiL(RTL_CONSTASCII_STRINGPARAM("<OSVERSION>"), i);
        if (i == -1) {
            break;
        }
        rtl::OUString aOSVersion = GetOSVersion();
        aValue = aValue.replaceAt(i, RTL_CONSTASCII_LENGTH("<OSVERSION>"), aOSVersion);
        i += aOSVersion.getLength();
    }
#endif	// USE_JAVA

    if( aValue.getLength() > 0 )
    {
        rRepositoryList.realloc(1);
        rRepositoryList[0] = aValue;
    }

    return true;
}

//------------------------------------------------------------------------------

// Returns 'true' if successfully connected to the update server
bool 
checkForUpdates(
    UpdateInfo& o_rUpdateInfo,
    uno::Reference< uno::XComponentContext > const & rxContext,
    uno::Reference< task::XInteractionHandler > const & rxInteractionHandler,
    const uno::Reference< deployment::XUpdateInformationProvider >& rUpdateInfoProvider)
{
    OSL_TRACE("checking for updates ..\n");
   
    ::rtl::OUString myArch;
    ::rtl::OUString myOS;
  
    rtl::Bootstrap::get(UNISTRING("_OS"), myOS);
    rtl::Bootstrap::get(UNISTRING("_ARCH"), myArch);
    
    uno::Sequence< ::rtl::OUString > aRepositoryList;
    ::rtl::OUString aBuildID;
    ::rtl::OUString aInstallSetID;
    
    if( ! ( getBootstrapData(aRepositoryList, aBuildID, aInstallSetID) && (aRepositoryList.getLength() > 0) ) )
    	return false;

#if defined USE_NATIVE_ADMIN_USER_CHECK && defined MACOSX
    // If there is a local admin group and the user is not in it, do not run
    // update check as the user must have admin privileges to install updates
    CFArrayRef aGroupIdentities = NULL;
    CSIdentityQueryRef aGroupQuery = CSIdentityQueryCreateForName(NULL, CFSTR("admin"), kCSIdentityQueryStringEquals, kCSIdentityClassGroup, CSGetLocalIdentityAuthority());
    if (aGroupQuery)
    {
        if (CSIdentityQueryExecute(aGroupQuery, 0, NULL))
            aGroupIdentities = CSIdentityQueryCopyResults(aGroupQuery);
        CFRelease(aGroupQuery);
    }
  
    if (aGroupIdentities)
    {
        bool bIsAdminUser = false;
        CFArrayRef aUserIdentities = NULL;
        CSIdentityQueryRef aUserQuery = CSIdentityQueryCreateForCurrentUser(NULL);
        if (aUserQuery)
        {
            if (CSIdentityQueryExecute(aUserQuery, 0, NULL))
                aUserIdentities = CSIdentityQueryCopyResults(aUserQuery);
            CFRelease(aUserQuery);
        }

        if (aUserIdentities)
        {
            CFIndex nGroupCount = CFArrayGetCount(aGroupIdentities);
            CFIndex nUserCount = CFArrayGetCount(aUserIdentities);
            for (CFIndex i = 0; !bIsAdminUser && i < nGroupCount; i++)
            {
                const CSIdentityRef aGroupIdentity = (const CSIdentityRef)CFArrayGetValueAtIndex(aGroupIdentities, i);
                if (aGroupIdentity)
                {
                    for (CFIndex j = 0; !bIsAdminUser && j < nUserCount; j++)
                    {
                        const CSIdentityRef aUserIdentity = (const CSIdentityRef)CFArrayGetValueAtIndex(aUserIdentities, j);
                        if (aUserIdentity && CSIdentityIsMemberOfGroup(aUserIdentity, aGroupIdentity))
                            bIsAdminUser = true;
                    }
                }
            }

            CFRelease(aUserIdentities);
        }

        CFRelease(aGroupIdentities);

        if (!bIsAdminUser)
    		return true;
    }
#endif	// USE_NATIVE_ADMIN_USER_CHECK && MACOSX

    if( !rxContext.is() )
        throw uno::RuntimeException( 
            UNISTRING( "checkForUpdates: empty component context" ), uno::Reference< uno::XInterface >() );
            
    OSL_ASSERT( rxContext->getServiceManager().is() );
			
    // XPath implementation 
    uno::Reference< xml::xpath::XXPathAPI > xXPath( 
        rxContext->getServiceManager()->createInstanceWithContext( UNISTRING( "com.sun.star.xml.xpath.XPathAPI" ), rxContext ), 
        uno::UNO_QUERY_THROW);    
    
    xXPath->registerNS( UNISTRING("inst"), UNISTRING("http://installation.openoffice.org/description") );

    if( rxInteractionHandler.is() )
        rUpdateInfoProvider->setInteractionHandler(rxInteractionHandler);
	
    try
    {
		uno::Reference< container::XEnumeration > aUpdateInfoEnumeration =
            rUpdateInfoProvider->getUpdateInformationEnumeration( aRepositoryList, aInstallSetID );

        if ( !aUpdateInfoEnumeration.is() )
            return false; // something went wrong ..

        rtl::OUStringBuffer aBuffer;
        aBuffer.appendAscii("/child::inst:description[inst:os=\'");
        aBuffer.append( myOS );
        aBuffer.appendAscii("\' and inst:arch=\'");
        aBuffer.append( myArch );
        aBuffer.appendAscii("\' and inst:buildid>");
        aBuffer.append( aBuildID );
        aBuffer.appendAscii("]");
        
        rtl::OUString aXPathExpression = aBuffer.makeStringAndClear();

        while( aUpdateInfoEnumeration->hasMoreElements() )
        {
            deployment::UpdateInformationEntry aEntry;
            
            if( aUpdateInfoEnumeration->nextElement() >>= aEntry )
            {
                uno::Reference< xml::dom::XNode > xNode( aEntry.UpdateDocument.get() );
                uno::Reference< xml::dom::XNodeList > xNodeList;
                try {
                    xNodeList = xXPath->selectNodeList(xNode, aXPathExpression
                        + UNISTRING("/inst:update/attribute::src"));
                } catch (css::xml::xpath::XPathException &) {
                    // ignore
                }

/*                
                o_rUpdateInfo.Sources.push_back( DownloadSource(true, 
                    UNISTRING("http://openoffice.bouncer.osuosl.org/?product=OpenOffice.org&os=solarissparcwjre&lang=en-US&version=2.2.1") ) );
*/
                
                sal_Int32 i, imax = xNodeList->getLength();
                for( i = 0; i < imax; ++i )
                {
                    uno::Reference< xml::dom::XNode > xNode2( xNodeList->item(i) );
                
                    if( xNode2.is() )
                    {
                        uno::Reference< xml::dom::XElement > xParent(xNode2->getParentNode(), uno::UNO_QUERY_THROW);
                        rtl::OUString aType = xParent->getAttribute(UNISTRING("type"));
                        bool bIsDirect = ( sal_False == aType.equalsIgnoreAsciiCaseAscii("text/html") );
                    
                        o_rUpdateInfo.Sources.push_back( DownloadSource(bIsDirect, xNode2->getNodeValue()) );
                    }
                }

                uno::Reference< xml::dom::XNode > xNode2;
                try {
                    xNode2 = xXPath->selectSingleNode(xNode, aXPathExpression
                        + UNISTRING("/inst:version/text()"));
                } catch (css::xml::xpath::XPathException &) {
                    // ignore
                }

                if( xNode2.is() )
                    o_rUpdateInfo.Version = xNode2->getNodeValue();

                try {
                    xNode2 = xXPath->selectSingleNode(xNode, aXPathExpression
                        + UNISTRING("/inst:buildid/text()"));
                } catch (css::xml::xpath::XPathException &) {
                    // ignore
                }

                if( xNode2.is() )
                    o_rUpdateInfo.BuildId = xNode2->getNodeValue();

                o_rUpdateInfo.Description = aEntry.Description;

                // Release Notes
                try {
                    xNodeList = xXPath->selectNodeList(xNode, aXPathExpression
                        + UNISTRING("/inst:relnote"));
                } catch (css::xml::xpath::XPathException &) {
                    // ignore
                }
                imax = xNodeList->getLength();
                for( i = 0; i < imax; ++i )
                {
                    uno::Reference< xml::dom::XElement > xRelNote(xNodeList->item(i), uno::UNO_QUERY);
                    if( xRelNote.is() )
                    {
                        sal_Int32 pos = xRelNote->getAttribute(UNISTRING("pos")).toInt32();
                        
                        ReleaseNote aRelNote((sal_uInt8) pos, xRelNote->getAttribute(UNISTRING("src")));
                        
                        if( xRelNote->hasAttribute(UNISTRING("src2")) )
                        {
                            pos = xRelNote->getAttribute(UNISTRING("pos2")).toInt32();
                            aRelNote.Pos2 = (sal_Int8) pos;
                            aRelNote.URL2 = xRelNote->getAttribute(UNISTRING("src2"));
                        }
                        
                        o_rUpdateInfo.ReleaseNotes.push_back(aRelNote);
                    }
                }
/*
                o_rUpdateInfo.ReleaseNotes.push_back(
                    ReleaseNote(1, UNISTRING("http://qa.openoffice.org/tests/online_update_test.html"))
                );
*/
                
                if( o_rUpdateInfo.Sources.size() > 0 )
                    return true;
            }
        }
    }
    catch( ... )
    {
        return false;
    }
    
    return true;
}

//------------------------------------------------------------------------------
bool storeExtensionUpdateInfos( const uno::Reference< uno::XComponentContext > & rxContext,
                                const uno::Sequence< uno::Sequence< rtl::OUString > > &rUpdateInfos )
{
    bool bNotify = false;

    if ( rUpdateInfos.hasElements() )
    {
        rtl::Reference< UpdateCheckConfig > aConfig = UpdateCheckConfig::get( rxContext );

        for ( sal_Int32 i = rUpdateInfos.getLength() - 1; i >= 0; i-- )
        {
            bNotify |= aConfig->storeExtensionVersion( rUpdateInfos[i][0], rUpdateInfos[i][1] );
        }
    }
    return bNotify;
}

//------------------------------------------------------------------------------
// Returns 'true' if there are updates for any extension

bool checkForExtensionUpdates( const uno::Reference< uno::XComponentContext > & rxContext )
{
    uno::Sequence< uno::Sequence< rtl::OUString > > aUpdateList;

    uno::Reference< deployment::XPackageInformationProvider > xInfoProvider;
    try
    {
        uno::Any aValue( rxContext->getValueByName(
                UNISTRING( "/singletons/com.sun.star.deployment.PackageInformationProvider" ) ) );
        OSL_VERIFY( aValue >>= xInfoProvider );
    }
    catch( const uno::Exception& )
    {
        OSL_ENSURE( false, "checkForExtensionUpdates: could not create the PackageInformationProvider!" );
    }

    if ( !xInfoProvider.is() ) return false;

    aUpdateList = xInfoProvider->isUpdateAvailable( ::rtl::OUString() );
    bool bNotify = storeExtensionUpdateInfos( rxContext, aUpdateList );

    return bNotify;
}

//------------------------------------------------------------------------------
// Returns 'true' if there are any pending updates for any extension (offline check)

bool checkForPendingUpdates( const uno::Reference< uno::XComponentContext > & rxContext )
{
    uno::Sequence< uno::Sequence< rtl::OUString > > aExtensionList;
    uno::Reference< deployment::XPackageInformationProvider > xInfoProvider;
    try
    {
        uno::Any aValue( rxContext->getValueByName(
                UNISTRING( "/singletons/com.sun.star.deployment.PackageInformationProvider" ) ) );
        OSL_VERIFY( aValue >>= xInfoProvider );
    }
    catch( const uno::Exception& )
    {
        OSL_ENSURE( false, "checkForExtensionUpdates: could not create the PackageInformationProvider!" );
    }

    if ( !xInfoProvider.is() ) return false;

    bool bPendingUpdateFound = false;

    aExtensionList = xInfoProvider->getExtensionList();
    if ( aExtensionList.hasElements() )
    {
        rtl::Reference< UpdateCheckConfig > aConfig = UpdateCheckConfig::get( rxContext );

        for ( sal_Int32 i = aExtensionList.getLength() - 1; i >= 0; i-- )
        {
            bPendingUpdateFound = aConfig->checkExtensionVersion( aExtensionList[i][0], aExtensionList[i][1] );
            if ( bPendingUpdateFound )
                break;
        }
    }

    return bPendingUpdateFound;
}
