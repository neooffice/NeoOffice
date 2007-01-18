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
 *    Modified January 2007 by Patrick Luby. NeoOffice is distributed under
 *    GPL only under modification term 3 of the LGPL.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sfx2.hxx"


#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCONTAINER_HPP_
#include <com/sun/star/container/XContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_ELEMENTMODES_HPP_
#include <com/sun/star/embed/ElementModes.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_XTRANSACTEDOBJECT_HPP_
#include <com/sun/star/embed/XTransactedObject.hpp>
#endif

#ifndef __RSC //autogen
#include <tools/errinf.hxx>
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _RTL_URI_HXX_
#include <rtl/uri.hxx>
#endif

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

#include <cppuhelper/typeprovider.hxx>
#include "namecont.hxx"

#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _UTL_STREAM_WRAPPER_HXX_
#include <unotools/streamwrap.hxx>
#endif
#include <svtools/pathoptions.hxx>
#include <svtools/sfxecode.hxx>
#include <svtools/ehdl.hxx>
#include <basic/basmgr.hxx>


#ifndef _COM_SUN_STAR_XML_SAX_XEXTENDEDDOCUMENTHANDLER_HPP_
#include <com/sun/star/xml/sax/XExtendedDocumentHandler.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_SAX_XPARSER_HPP_
#include <com/sun/star/xml/sax/XParser.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_SAX_INPUTSOURCE_HPP_
#include <com/sun/star/xml/sax/InputSource.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XOUTPUTSTREAM_HPP_
#include <com/sun/star/io/XOutputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HPP_
#include <com/sun/star/io/XInputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XACTIVEDATASOURCE_HPP_
#include <com/sun/star/io/XActiveDataSource.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_DEPLOYMENTEXCEPTION_HPP_
#include <com/sun/star/uno/DeploymentException.hpp>
#endif

#include <comphelper/storagehelper.hxx>
#include <comphelper/anytostring.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <basic/sbmod.hxx>

using namespace com::sun::star::container;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::io;
using namespace com::sun::star::ucb;
using namespace com::sun::star::script;
using namespace com::sun::star::beans;
using namespace com::sun::star::xml::sax;
using namespace com::sun::star::util;
using namespace com::sun::star::task;
using namespace com::sun::star;
using namespace cppu;
using namespace rtl;
using namespace osl;


// #i34411: Flag for error handling during migration 
bool GbMigrationSuppressErrors = false;


//============================================================================
// Implementation class NameContainer_Impl

namespace SfxContainer_Impl
{
// Methods XElementAccess
Type NameContainer_Impl::getElementType()
	throw(RuntimeException)
{
	return mType;
}

sal_Bool NameContainer_Impl::hasElements()
	throw(RuntimeException)
{
	sal_Bool bRet = (mnElementCount > 0);
	return bRet;
}

// Methods XNameAccess
Any NameContainer_Impl::getByName( const OUString& aName )
	throw(NoSuchElementException, WrappedTargetException, RuntimeException)
{
	NameContainerNameMap::iterator aIt = mHashMap.find( aName );
	if( aIt == mHashMap.end() )
	{
		throw NoSuchElementException();
	}
	sal_Int32 iHashResult = (*aIt).second;
	Any aRetAny = mValues.getConstArray()[ iHashResult ];
	return aRetAny;
}

Sequence< OUString > NameContainer_Impl::getElementNames()
	throw(RuntimeException)
{
	return mNames;
}

sal_Bool NameContainer_Impl::hasByName( const OUString& aName )
	throw(RuntimeException)
{
	NameContainerNameMap::iterator aIt = mHashMap.find( aName );
	sal_Bool bRet = ( aIt != mHashMap.end() );
	return bRet;
}


// Methods XNameReplace
void NameContainer_Impl::replaceByName( const OUString& aName, const Any& aElement )
	throw(IllegalArgumentException, NoSuchElementException, WrappedTargetException, RuntimeException)
{
	Type aAnyType = aElement.getValueType();
	if( mType != aAnyType )
		throw IllegalArgumentException();

	NameContainerNameMap::iterator aIt = mHashMap.find( aName );
	if( aIt == mHashMap.end() )
	{
		throw NoSuchElementException();
	}
	sal_Int32 iHashResult = (*aIt).second;
	Any aOldElement = mValues.getConstArray()[ iHashResult ];
	mValues.getArray()[ iHashResult ] = aElement;


	// Fire event
	ContainerEvent aEvent;
	aEvent.Source = mpxEventSource;
	aEvent.Accessor <<= aName;
	aEvent.Element = aElement;
	aEvent.ReplacedElement = aOldElement;

	OInterfaceIteratorHelper aIterator( maListenerContainer );
	while( aIterator.hasMoreElements() )
	{
		Reference< XInterface > xIface = aIterator.next();
		Reference< XContainerListener > xListener( xIface, UNO_QUERY );
        try
        {
            xListener->elementReplaced( aEvent );
        }
        catch(RuntimeException&)
        {
            aIterator.remove();
        }
	}
}


// Methods XNameContainer
void NameContainer_Impl::insertByName( const OUString& aName, const Any& aElement )
	throw(IllegalArgumentException, ElementExistException, WrappedTargetException, RuntimeException)
{
	Type aAnyType = aElement.getValueType();
	if( mType != aAnyType )
		throw IllegalArgumentException();

	NameContainerNameMap::iterator aIt = mHashMap.find( aName );
	if( aIt != mHashMap.end() )
	{
		throw ElementExistException();
	}

	sal_Int32 nCount = mNames.getLength();
	mNames.realloc( nCount + 1 );
	mValues.realloc( nCount + 1 );
	mNames.getArray()[ nCount ] = aName;
	mValues.getArray()[ nCount ] = aElement;

	mHashMap[ aName ] = nCount;
	mnElementCount++;


	// Fire event
	ContainerEvent aEvent;
	aEvent.Source = mpxEventSource;
	aEvent.Accessor <<= aName;
	aEvent.Element = aElement;

	OInterfaceIteratorHelper aIterator( maListenerContainer );
	while( aIterator.hasMoreElements() )
	{
		Reference< XInterface > xIface = aIterator.next();
		Reference< XContainerListener > xListener( xIface, UNO_QUERY );
        try
        {
            xListener->elementInserted( aEvent );
        }
        catch(RuntimeException&)
        {
            aIterator.remove();
        }
	}
}

void NameContainer_Impl::removeByName( const OUString& Name )
	throw(NoSuchElementException, WrappedTargetException, RuntimeException)
{
	NameContainerNameMap::iterator aIt = mHashMap.find( Name );
	if( aIt == mHashMap.end() )
	{
		throw NoSuchElementException();
	}

	sal_Int32 iHashResult = (*aIt).second;
	Any aOldElement = mValues.getConstArray()[ iHashResult ];
	mHashMap.erase( aIt );
	sal_Int32 iLast = mNames.getLength() - 1;
	if( iLast != iHashResult )
	{
		OUString* pNames = mNames.getArray();
		Any* pValues = mValues.getArray();
		pNames[ iHashResult ] = pNames[ iLast ];
		pValues[ iHashResult ] = pValues[ iLast ];
		mHashMap[ pNames[ iHashResult ] ] = iHashResult;
	}
	mNames.realloc( iLast );
	mValues.realloc( iLast );
	mnElementCount--;


	// Fire event
	ContainerEvent aEvent;
	aEvent.Source = mpxEventSource;
	aEvent.Accessor <<= Name;
	aEvent.Element = aOldElement;

	OInterfaceIteratorHelper aIterator( maListenerContainer );
	while( aIterator.hasMoreElements() )
	{
		Reference< XInterface > xIface = aIterator.next();
		Reference< XContainerListener > xListener( xIface, UNO_QUERY );
        try
        {
            xListener->elementRemoved( aEvent );
        }
        catch(RuntimeException&)
        {
            aIterator.remove();
        }
	}
}


// Methods XContainer
void SAL_CALL NameContainer_Impl::addContainerListener( const Reference< XContainerListener >& xListener )
	throw (RuntimeException)
{
	if( !xListener.is() )
		throw RuntimeException();
	Reference< XInterface > xIface( xListener, UNO_QUERY );
	maListenerContainer.addInterface( xIface );
}

void SAL_CALL NameContainer_Impl::removeContainerListener( const Reference< XContainerListener >& xListener )
	throw (RuntimeException)
{
	if( !xListener.is() )
		throw RuntimeException();
	Reference< XInterface > xIface( xListener, UNO_QUERY );
	maListenerContainer.removeInterface( xIface );
}

}	// namespace SfxContainer_Impl


//============================================================================

// Implementation class SfxLibraryContainer_Impl

// Ctor
SfxLibraryContainer_Impl::SfxLibraryContainer_Impl( void )
	: maNameContainer( getCppuType( (Reference< XNameAccess >*) NULL ) )
	, mbModified( sal_False )
    , mbOldInfoFormat( sal_False )
    , mbOasis2OOoFormat( sal_False )
    , mpBasMgr( NULL )
    , mbOwnBasMgr( sal_False )
{
	mxMSF = comphelper::getProcessServiceFactory();
	if( !mxMSF.is() )
	{
		OSL_ENSURE( 0, "### couln't get ProcessServiceFactory\n" );
	}

	mxSFI = Reference< XSimpleFileAccess >( mxMSF->createInstance
		( OUString::createFromAscii( "com.sun.star.ucb.SimpleFileAccess" ) ), UNO_QUERY );
	if( !mxSFI.is() )
	{
		OSL_ENSURE( 0, "### couln't create SimpleFileAccess component\n" );
	}

	mxStringSubstitution = Reference< XStringSubstitution >( mxMSF->createInstance
		( OUString::createFromAscii( "com.sun.star.util.PathSubstitution" ) ), UNO_QUERY );
	if( !mxStringSubstitution.is() )
	{
		OSL_ENSURE( 0, "### couln't create PathSubstitution component\n" );
	}
}

SfxLibraryContainer_Impl::~SfxLibraryContainer_Impl()
{
    if( mbOwnBasMgr )
        delete mpBasMgr;
}

sal_Bool SfxLibraryContainer_Impl::isContainerModified()
{
	if ( mbModified )
		return sal_True;

	// the library container is not modified, go through the libraries and check whether they are modified
	Sequence< OUString > aNames = maNameContainer.getElementNames();
	const OUString* pNames = aNames.getConstArray();
	sal_Int32 nNameCount = aNames.getLength();

	for( sal_Int32 i = 0 ; i < nNameCount ; i++ )
	{
		OUString aName = pNames[ i ];
        SfxLibrary_Impl* pImplLib = getImplLib( aName );
		if( pImplLib->mbModified )
		{
			if ( aName.equals( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Standard") ) ) )
			{
				// this is a workaround that has to be implemented because
				// empty standard library should stay marked as modified
				// but should not be treated as modified while it is empty
				if ( pImplLib->hasElements() )
					return sal_True;
			}
			else
				return sal_True;
		}
	}

	return sal_False;
}

void SfxLibraryContainer_Impl::setContainerModified( sal_Bool bModified )
{
	mbModified = bModified;
}


static void checkAndCopyFileImpl( const INetURLObject& rSourceFolderInetObj,
								  const INetURLObject& rTargetFolderInetObj,
								  const OUString& rCheckFileName,
								  const OUString& rCheckExtension,
								  Reference< XSimpleFileAccess > xSFI )
{
	INetURLObject aTargetFolderInetObj( rTargetFolderInetObj );
	aTargetFolderInetObj.insertName( rCheckFileName, sal_True, INetURLObject::LAST_SEGMENT,
									 sal_True, INetURLObject::ENCODE_ALL );
	aTargetFolderInetObj.setExtension( rCheckExtension );
	OUString aTargetFile = aTargetFolderInetObj.GetMainURL( INetURLObject::NO_DECODE );
	if( !xSFI->exists( aTargetFile ) )
	{
		INetURLObject aSourceFolderInetObj( rSourceFolderInetObj );
		aSourceFolderInetObj.insertName( rCheckFileName, sal_True, INetURLObject::LAST_SEGMENT,
										 sal_True, INetURLObject::ENCODE_ALL );
		aSourceFolderInetObj.setExtension( rCheckExtension );
		OUString aSourceFile = aSourceFolderInetObj.GetMainURL( INetURLObject::NO_DECODE );
		xSFI->copy( aSourceFile, aTargetFile );
	}
}

static void createVariableURL( OUString& rStr, const OUString& rLibName,
							   const OUString& rInfoFileName, bool bUser )
{
	if( bUser )
		rStr = OUString::createFromAscii( "$(USER)/basic/" );
	else
		rStr = OUString::createFromAscii( "$(INST)/share/basic/" );

	rStr += rLibName;
	rStr += OUString::createFromAscii( "/" );
	rStr += rInfoFileName;
	rStr += OUString::createFromAscii( ".xlb/" );
}

sal_Bool SfxLibraryContainer_Impl::init(
    const OUString& aInitialisationParam,
    const OUString& aInfoFileName,
    const OUString& aOldInfoFileName,
    const OUString& aLibElementFileExtension,
    const OUString& aLibrariesDir,
	const uno::Reference< embed::XStorage >& xInitStorage )
{
	uno::Reference< embed::XStorage > xStorage = xInitStorage;

    maInitialisationParam = aInitialisationParam;
	maInfoFileName = aInfoFileName;
	maOldInfoFileName = aOldInfoFileName;
	maLibElementFileExtension = aLibElementFileExtension;
	maLibrariesDir = aLibrariesDir;

    meInitMode = DEFAULT;
    INetURLObject aInitUrlInetObj( maInitialisationParam );
    OUString aInitFileName = aInitUrlInetObj.GetMainURL( INetURLObject::NO_DECODE );
    if( aInitFileName.getLength() )
    {
        // We need a BasicManager to avoid problems
        StarBASIC* pBas = new StarBASIC();
        mpBasMgr = new BasicManager( pBas );
        mbOwnBasMgr = sal_True;

        OUString aExtension = aInitUrlInetObj.getExtension();
        if( aExtension.compareToAscii( "xlc" ) == COMPARE_EQUAL )
        {
            meInitMode = CONTAINER_INIT_FILE;
	        INetURLObject aLibPathInetObj( aInitUrlInetObj );
			aLibPathInetObj.removeSegment();
	        maLibraryPath = aLibPathInetObj.GetMainURL( INetURLObject::NO_DECODE );
        }
        else if( aExtension.compareToAscii( "xlb" ) == COMPARE_EQUAL )
        {
            meInitMode = LIBRARY_INIT_FILE;
        	uno::Reference< embed::XStorage > xDummyStor;
            ::xmlscript::LibDescriptor aLibDesc;
            sal_Bool bReadIndexFile = implLoadLibraryIndexFile( NULL, aLibDesc, xDummyStor, aInitFileName );
           	return bReadIndexFile;
        }
        else
        {
            // Decide between old and new document
            sal_Bool bOldStorage = SotStorage::IsOLEStorage( aInitFileName );
            if ( bOldStorage )
            {
                meInitMode = OLD_BASIC_STORAGE;
                importFromOldStorage( aInitFileName );
                return sal_True;
            }
            else
            {
                meInitMode = OFFICE_DOCUMENT;
                try
                {
                    xStorage = ::comphelper::OStorageHelper::GetStorageFromURL( aInitFileName, embed::ElementModes::READ );
                }
                catch ( uno::Exception& )
                {
                    // TODO: error handling
                }
//REMOVE	                xStorage = new SotStorage( sal_True, aInitFileName );
            }
        }
    }
    else
    {
        // Default pathes
        maLibraryPath = SvtPathOptions().GetBasicPath();
    }

	Reference< XParser > xParser( mxMSF->createInstance(
		OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.xml.sax.Parser") ) ), UNO_QUERY );
	if( !xParser.is() )
	{
		OSL_ENSURE( 0, "### couln't create sax parser component\n" );
		return sal_False;
	}

	uno::Reference< io::XInputStream > xInput;

	mxStorage = xStorage;
	sal_Bool bStorage = mxStorage.is();


	// #110009: Scope to force the StorageRefs to be destructed and
	// so the streams to be closed before the preload opertation
	{
	// #110009

	uno::Reference< embed::XStorage > xLibrariesStor;
	String aFileName;

	int nPassCount = 1;
	if( !bStorage && meInitMode == DEFAULT )
		nPassCount = 2;
	for( int nPass = 0 ; nPass < nPassCount ; nPass++ )
	{
		if( bStorage )
		{
			OSL_ENSURE( meInitMode == DEFAULT || meInitMode == OFFICE_DOCUMENT,
				"### Wrong InitMode for document\n" );
			try
			{
				uno::Reference< io::XStream > xStream;
				xLibrariesStor = xStorage->openStorageElement( maLibrariesDir, embed::ElementModes::READ );
//REMOVE				xLibrariesStor = xStorage->OpenSotStorage( maLibrariesDir, STREAM_READ | STREAM_NOCREATE );
                //if ( !xLibrariesStor.is() )
                    // TODO: the method must either return a storage or throw an exception
                    //throw uno::RuntimeException();

                if ( xLibrariesStor.is() )
                {
                    aFileName = maInfoFileName;
                    aFileName += String( RTL_CONSTASCII_USTRINGPARAM("-lc.xml") );

                    try
                    {
                        xStream = xLibrariesStor->openStreamElement( aFileName, embed::ElementModes::READ );
                    }
                    catch( uno::Exception& )
                    {}

                    if( !xStream.is() )
                    {
                        mbOldInfoFormat = true;

                        // Check old version
                        aFileName = maOldInfoFileName;
                        aFileName += String( RTL_CONSTASCII_USTRINGPARAM(".xml") );

                        try
                        {
                            xStream = xLibrariesStor->openStreamElement( aFileName, embed::ElementModes::READ );
                        }
                        catch( uno::Exception& )
                        {}

                        if( !xStream.is() )
                        {
                            // Check for EA2 document version with wrong extensions
                            aFileName = maOldInfoFileName;
                            aFileName += String( RTL_CONSTASCII_USTRINGPARAM(".xli") );
                            xStream = xLibrariesStor->openStreamElement( aFileName, embed::ElementModes::READ );
                        }
                    }
				}

				if ( xStream.is() )
					xInput = xStream->getInputStream();
			}
			catch( uno::Exception& )
			{
                // TODO: error handling?
			}
		}
		else
		{
			INetURLObject* pLibInfoInetObj = NULL;
			if( meInitMode == CONTAINER_INIT_FILE )
			{
				aFileName = aInitFileName;
			}
			else
			{
				if( nPass == 1 )
					pLibInfoInetObj = new INetURLObject( String(maLibraryPath).GetToken(0) );
				else
					pLibInfoInetObj = new INetURLObject( String(maLibraryPath).GetToken(1) );
				pLibInfoInetObj->insertName( maInfoFileName, sal_True, INetURLObject::LAST_SEGMENT, sal_True, INetURLObject::ENCODE_ALL );
				pLibInfoInetObj->setExtension( OUString( RTL_CONSTASCII_USTRINGPARAM("xlc") ) );
				aFileName = pLibInfoInetObj->GetMainURL( INetURLObject::NO_DECODE );
			}

			try
			{
				xInput = mxSFI->openFileRead( aFileName );
			}
			catch( Exception& )
			{
				xInput.clear();
                if( nPass == 0 )
                {
		            SfxErrorContext aEc( ERRCTX_SFX_LOADBASIC, aFileName );
                    ULONG nErrorCode = ERRCODE_IO_GENERAL;
                    ErrorHandler::HandleError( nErrorCode );
                }
			}

			// Old variant?
			if( !xInput.is() && nPass == 0 )
			{
				INetURLObject aLibInfoInetObj( String(maLibraryPath).GetToken(1) );
				aLibInfoInetObj.insertName( maOldInfoFileName, sal_True, INetURLObject::LAST_SEGMENT, sal_True, INetURLObject::ENCODE_ALL );
				aLibInfoInetObj.setExtension( OUString( RTL_CONSTASCII_USTRINGPARAM("xli") ) );
				aFileName = aLibInfoInetObj.GetMainURL( INetURLObject::NO_DECODE );

				try
				{
					xInput = mxSFI->openFileRead( aFileName );
					mbOldInfoFormat = true;
				}
				catch( Exception& )
				{
    				xInput.clear();
		            SfxErrorContext aEc( ERRCTX_SFX_LOADBASIC, aFileName );
                    ULONG nErrorCode = ERRCODE_IO_GENERAL;
                    ErrorHandler::HandleError( nErrorCode );
				}
			}

			delete pLibInfoInetObj;
		}

		if( xInput.is() )
        {
		    InputSource source;
		    source.aInputStream = xInput;
		    source.sSystemId 	= aFileName;

		    // start parsing
		    ::xmlscript::LibDescriptorArray* pLibArray = new ::xmlscript::LibDescriptorArray();

            try
            {
                xParser->setDocumentHandler( ::xmlscript::importLibraryContainer( pLibArray ) );
                xParser->parseStream( source );
            }
            catch ( xml::sax::SAXException& e )
            {
                (void) e; // avoid warning
                OSL_ENSURE( 0, OUStringToOString( e.Message, RTL_TEXTENCODING_ASCII_US ).getStr() );
                return sal_False;
            }
            catch ( io::IOException& e )
            {
                (void) e; // avoid warning
                OSL_ENSURE( 0, OUStringToOString( e.Message, RTL_TEXTENCODING_ASCII_US ).getStr() );
                return sal_False;
            }

		    sal_Int32 nLibCount = pLibArray->mnLibCount;
		    for( sal_Int32 i = 0 ; i < nLibCount ; i++ )
		    {
			    ::xmlscript::LibDescriptor& rLib = pLibArray->mpLibs[i];

			    // Check storage URL
			    OUString aStorageURL = rLib.aStorageURL;
			    if( !bStorage && !aStorageURL.getLength() && nPass == 0 )
			    {
					String aLibraryPath;
					if( meInitMode == CONTAINER_INIT_FILE )
						aLibraryPath = maLibraryPath;
					else
						aLibraryPath = String(maLibraryPath).GetToken(1);
					INetURLObject aInetObj( aLibraryPath );

				    aInetObj.insertName( rLib.aName, sal_True, INetURLObject::LAST_SEGMENT,
					    sal_True, INetURLObject::ENCODE_ALL );
				    OUString aLibDirPath = aInetObj.GetMainURL( INetURLObject::NO_DECODE );
				    if( mxSFI->isFolder( aLibDirPath ) )
				    {
						createVariableURL( rLib.aStorageURL, rLib.aName, maInfoFileName, true );
					    mbModified = sal_True;
				    }
				    else if( rLib.bLink )
				    {
					    // Check "share" path
					    INetURLObject aShareInetObj( String(maLibraryPath).GetToken(0) );
					    aShareInetObj.insertName( rLib.aName, sal_True, INetURLObject::LAST_SEGMENT,
						    sal_True, INetURLObject::ENCODE_ALL );
					    OUString aShareLibDirPath = aShareInetObj.GetMainURL( INetURLObject::NO_DECODE );
					    if( mxSFI->isFolder( aShareLibDirPath ) )
					    {
							createVariableURL( rLib.aStorageURL, rLib.aName, maInfoFileName, false );
						    mbModified = sal_True;
					    }
						else
						{
							// #i25537: Ignore lib if library folder does not really exist
							continue;
						}
				    }
			    }

#ifdef USE_JAVA
				// Finally put an end to those hard-coded WebWizard references
				// from OOo-1.1.x-based installations
				if ( rLib.aName == OUString::createFromAscii( "WebWizard" ) )
				{
					OUString aWebWizardScriptURL = OUString::createFromAscii( "/Contents/share/basic/WebWizard/script.xlb/" );
					sal_Int32 nIndex = rLib.aStorageURL.lastIndexOf( aWebWizardScriptURL );
					if ( nIndex >= 0 && nIndex == rLib.aStorageURL.getLength() - aWebWizardScriptURL.getLength() )
						continue;

					OUString aWebWizardDialogURL = OUString::createFromAscii( "/Contents/share/basic/WebWizard/dialog.xlb/" );
					nIndex = rLib.aStorageURL.lastIndexOf( aWebWizardDialogURL );
					if ( nIndex >= 0 && nIndex == rLib.aStorageURL.getLength() - aWebWizardDialogURL.getLength() )
						continue;
				}
#endif	// USE_JAVA

			    OUString aLibName = rLib.aName;

			    // If the same library name is used by the shared and the
			    // user lib container index files the user file wins
			    if( nPass == 1 && hasByName( aLibName ) )
				    continue;

			    SfxLibrary_Impl* pImplLib;
			    if( rLib.bLink )
			    {
				    Reference< XNameAccess > xLib =
					    createLibraryLink( aLibName, rLib.aStorageURL, rLib.bReadOnly );
				    pImplLib = static_cast< SfxLibrary_Impl* >( xLib.get() );
			    }
			    else
			    {
				    Reference< XNameContainer > xLib = createLibrary( aLibName );
				    pImplLib = static_cast< SfxLibrary_Impl* >( xLib.get() );
				    pImplLib->mbLoaded = sal_False;
				    pImplLib->mbReadOnly = rLib.bReadOnly;
				    if( !bStorage )
					    checkStorageURL( rLib.aStorageURL, pImplLib->maLibInfoFileURL,
                            pImplLib->maStorageURL, pImplLib->maUnexpandedStorageURL );
			    }
				mbModified = sal_False;

			    // Read library info files
			    if( !mbOldInfoFormat )
			    {
        		    uno::Reference< embed::XStorage > xLibraryStor;
          		    if( !pImplLib->mbInitialised && bStorage )
				    {
						try {
							xLibraryStor = xLibrariesStor->openStorageElement( rLib.aName,
																				embed::ElementModes::READ );
						}
						catch( uno::Exception& )
						{
						    OSL_ENSURE( 0, "### couln't open sub storage for library\n" );
						}
				    }

				    // Link is already initialised in createLibraryLink()
				    if( !pImplLib->mbInitialised && (!bStorage || xLibraryStor.is()) )
				    {
					    OUString aIndexFileName;
					    sal_Bool bLoaded = implLoadLibraryIndexFile( pImplLib, rLib, xLibraryStor, aIndexFileName );
					    if( bLoaded && aLibName != rLib.aName )
					    {
						    OSL_ENSURE( 0, "Different library names in library"
							    " container and library info files!\n" );
					    }
						if( GbMigrationSuppressErrors && !bLoaded )
							removeLibrary( aLibName );
				    }
			    }
			    else if( !bStorage )
			    {
				    // Write new index file immediately because otherwise
				    // the library elements will be lost when storing into
				    // the new info format
				    uno::Reference< embed::XStorage > xTmpStorage;
				    implStoreLibraryIndexFile( pImplLib, rLib, xTmpStorage );
			    }

			    implImportLibDescriptor( pImplLib, rLib );

			    if( nPass == 1 )
			    {
				    pImplLib->mbSharedIndexFile = sal_True;
				    pImplLib->mbReadOnly = sal_True;
			    }
		    }

		    // Keep flag for documents to force writing the new index files
		    if( !bStorage )
			    mbOldInfoFormat = sal_False;

		    delete pLibArray;
        }
		// Only in the first pass it's an error when no index file is found
		else if( nPass == 0 )
		{
			return sal_False;
		}
	}

	// #110009: END Scope to force the StorageRefs to be destructed
	}
	// #110009

	// #110009 Preload?
    {
	    Sequence< OUString > aNames = maNameContainer.getElementNames();
	    const OUString* pNames = aNames.getConstArray();
	    sal_Int32 nNameCount = aNames.getLength();
	    for( sal_Int32 i = 0 ; i < nNameCount ; i++ )
	    {
		    OUString aName = pNames[ i ];
            SfxLibrary_Impl* pImplLib = getImplLib( aName );
		    if( pImplLib->mbPreload )
			    loadLibrary( aName );
	    }
    }

    // #118803# upgrade installation 7.0 -> 8.0
    if( meInitMode == DEFAULT )
    {
        INetURLObject aUserBasicInetObj( String(maLibraryPath).GetToken(1) );
		OUString aStandardStr( RTL_CONSTASCII_USTRINGPARAM("Standard") );

		static char strPrevFolderName_1[] = "__basic_70";
		static char strPrevFolderName_2[] = "__basic_70_2";
        INetURLObject aPrevUserBasicInetObj_1( aUserBasicInetObj );
        aPrevUserBasicInetObj_1.removeSegment();
		INetURLObject aPrevUserBasicInetObj_2 = aPrevUserBasicInetObj_1;
	    aPrevUserBasicInetObj_1.Append( strPrevFolderName_1 );
	    aPrevUserBasicInetObj_2.Append( strPrevFolderName_2 );

		INetURLObject aPrevUserBasicInetObj = aPrevUserBasicInetObj_1;
        String aPrevFolder = aPrevUserBasicInetObj.GetMainURL( INetURLObject::NO_DECODE );
		bool bSecondTime = false;
	    if( mxSFI->isFolder( aPrevFolder ) )
		{
			// #110101 Check if Standard folder exists and is complete
	        INetURLObject aUserBasicStandardInetObj( aUserBasicInetObj );
			aUserBasicStandardInetObj.insertName( aStandardStr, sal_True, INetURLObject::LAST_SEGMENT,
												  sal_True, INetURLObject::ENCODE_ALL );
            INetURLObject aPrevUserBasicStandardInetObj( aPrevUserBasicInetObj );
		    aPrevUserBasicStandardInetObj.insertName( aStandardStr, sal_True, INetURLObject::LAST_SEGMENT,
                                                    sal_True, INetURLObject::ENCODE_ALL );
		    OUString aPrevStandardFolder = aPrevUserBasicStandardInetObj.GetMainURL( INetURLObject::NO_DECODE );
		    if( mxSFI->isFolder( aPrevStandardFolder ) )
			{
				OUString aXlbExtension( OUString( RTL_CONSTASCII_USTRINGPARAM("xlb") ) );
				OUString aCheckFileName;

				// Check if script.xlb exists
				aCheckFileName = OUString( RTL_CONSTASCII_USTRINGPARAM("script") );
				checkAndCopyFileImpl( aUserBasicStandardInetObj,
									  aPrevUserBasicStandardInetObj,
									  aCheckFileName, aXlbExtension, mxSFI );

				// Check if dialog.xlb exists
				aCheckFileName = OUString( RTL_CONSTASCII_USTRINGPARAM("dialog") );
				checkAndCopyFileImpl( aUserBasicStandardInetObj,
									  aPrevUserBasicStandardInetObj,
									  aCheckFileName, aXlbExtension, mxSFI );

				// Check if module1.xba exists
				OUString aXbaExtension( OUString( RTL_CONSTASCII_USTRINGPARAM("xba") ) );
				aCheckFileName = OUString( RTL_CONSTASCII_USTRINGPARAM("Module1") );
				checkAndCopyFileImpl( aUserBasicStandardInetObj,
									  aPrevUserBasicStandardInetObj,
									  aCheckFileName, aXbaExtension, mxSFI );
			}
			else
		    {
	            String aStandardFolder = aUserBasicStandardInetObj.GetMainURL( INetURLObject::NO_DECODE );
		        mxSFI->copy( aStandardFolder, aPrevStandardFolder );
			}

			String aPrevCopyToFolder = aPrevUserBasicInetObj_2.GetMainURL( INetURLObject::NO_DECODE );
	        mxSFI->copy( aPrevFolder, aPrevCopyToFolder );
		}
		else
		{
			bSecondTime = true;
			aPrevUserBasicInetObj = aPrevUserBasicInetObj_2;
			aPrevFolder = aPrevUserBasicInetObj.GetMainURL( INetURLObject::NO_DECODE );
		}
	    if( mxSFI->isFolder( aPrevFolder ) )
        {
            SfxLibraryContainer_Impl* pPrevCont = createInstanceImpl();
            Reference< XInterface > xRef = static_cast< XInterface* >( static_cast< OWeakObject* >(pPrevCont) );

            // Rename previous basic folder to make storage URLs correct during initialisation
            String aFolderUserBasic = aUserBasicInetObj.GetMainURL( INetURLObject::NO_DECODE );
            INetURLObject aUserBasicTmpInetObj( aUserBasicInetObj );
            aUserBasicTmpInetObj.removeSegment();
	        aUserBasicTmpInetObj.Append( "__basic_tmp" );
            String aFolderTmp = aUserBasicTmpInetObj.GetMainURL( INetURLObject::NO_DECODE );

			bool bMoveOk = true;
			try
				{ mxSFI->move( aFolderUserBasic, aFolderTmp ); }
			catch( CommandAbortedException& )
				{ bMoveOk = false; }
			catch( Exception& )
				{ bMoveOk = false; }
			if( bMoveOk )
			{
				try
					{ mxSFI->move( aPrevFolder, aFolderUserBasic ); }
				catch( CommandAbortedException& )
					{ bMoveOk = false; }
				catch( Exception& )
					{ bMoveOk = false; }
				if( !bMoveOk )
				{
					// Move back user/basic folder
					try
						{ mxSFI->move( aFolderTmp, aFolderUserBasic ); }
					catch( CommandAbortedException& ) {}
					catch( Exception& ) {}
				}
			}

			if( bMoveOk )
			{
				INetURLObject aPrevUserBasicLibInfoInetObj( aUserBasicInetObj );
				aPrevUserBasicLibInfoInetObj.insertName( maInfoFileName, sal_True, INetURLObject::LAST_SEGMENT,
													sal_True, INetURLObject::ENCODE_ALL );
				aPrevUserBasicLibInfoInetObj.setExtension( OUString( RTL_CONSTASCII_USTRINGPARAM("xlc") ) );
				OUString aLibInfoFileName = aPrevUserBasicLibInfoInetObj.GetMainURL( INetURLObject::NO_DECODE );
				Sequence<Any> aInitSeq( 1 );
				aInitSeq.getArray()[0] <<= aLibInfoFileName;
				GbMigrationSuppressErrors = true;
				pPrevCont->initialize( aInitSeq );
				GbMigrationSuppressErrors = false;

				// Rename folders back
				mxSFI->move( aFolderUserBasic, aPrevFolder );
				mxSFI->move( aFolderTmp, aFolderUserBasic );

				// Detect old share/basic folder to destinguish
				// between own links and links to wizards
				INetURLObject aPrevOrgShareFolderInetObj;
				if( pPrevCont->hasByName( aStandardStr ) )
				{
					SfxLibrary_Impl* pImplLib = pPrevCont->getImplLib( aStandardStr );
					aPrevOrgShareFolderInetObj = INetURLObject( pImplLib->maStorageURL );
					for( int i = 0 ; i < 3 ; ++ i )
						aPrevOrgShareFolderInetObj.removeSegment();
        			aPrevOrgShareFolderInetObj.Append( "share" );
        			aPrevOrgShareFolderInetObj.Append( "basic" );
				}

				Sequence< OUString > aNames = pPrevCont->getElementNames();
				const OUString* pNames = aNames.getConstArray();
				sal_Int32 nNameCount = aNames.getLength();

				for( sal_Int32 i = 0 ; i < nNameCount ; i++ )
				{
					OUString aLibName = pNames[ i ];
					if( hasByName( aLibName ) )
					{
						if( aLibName == aStandardStr )
						{
							SfxLibrary_Impl* pImplLib = getImplLib( aStandardStr );
							INetURLObject aStandardFolderInetObj( pImplLib->maStorageURL );
							String aStandardFolder = pImplLib->maStorageURL;
                			mxSFI->kill( aStandardFolder );
						}
						else
						{
							continue;
						}
					}

					SfxLibrary_Impl* pImplLib = pPrevCont->getImplLib( aLibName );
					if( pImplLib->mbLink )
					{
						INetURLObject aPrevLinkInetObj( pImplLib->maStorageURL );
						for( int j = 0 ; j < 3 ; ++ j )
							aPrevLinkInetObj.removeSegment();
        				aPrevLinkInetObj.Append( "share" );
        				aPrevLinkInetObj.Append( "basic" );
						if( aPrevLinkInetObj != aPrevOrgShareFolderInetObj )
							createLibraryLink( aLibName, pImplLib->maStorageURL, pImplLib->mbReadOnly );
					}
					else
					{
						// Move folder if not already done
						INetURLObject aUserBasicLibFolderInetObj( aUserBasicInetObj );
        				aUserBasicLibFolderInetObj.Append( aLibName );
						String aLibFolder = aUserBasicLibFolderInetObj.GetMainURL( INetURLObject::NO_DECODE );

						INetURLObject aPrevUserBasicLibFolderInetObj( aPrevUserBasicInetObj );
        				aPrevUserBasicLibFolderInetObj.Append( aLibName );
						String aPrevLibFolder = aPrevUserBasicLibFolderInetObj.GetMainURL( INetURLObject::NO_DECODE );

						if( mxSFI->isFolder( aPrevLibFolder ) && !mxSFI->isFolder( aLibFolder ) )
							mxSFI->move( aPrevLibFolder, aLibFolder );

						if( aLibName == aStandardStr )
                       		maNameContainer.removeByName( aLibName );

						// Create library
						Reference< XNameContainer > xLib = createLibrary( aLibName );
               			SfxLibrary_Impl* pNewLib = static_cast< SfxLibrary_Impl* >( xLib.get() );
						pNewLib->mbLoaded = false;
						pNewLib->mbModified = false;
						checkStorageURL( aLibFolder, pNewLib->maLibInfoFileURL,
							pNewLib->maStorageURL, pNewLib->maUnexpandedStorageURL );

						uno::Reference< embed::XStorage > xDummyStor;
						::xmlscript::LibDescriptor aLibDesc;
						/*sal_Bool bReadIndexFile =*/ implLoadLibraryIndexFile
							( pNewLib, aLibDesc, xDummyStor, pNewLib->maLibInfoFileURL );
						implImportLibDescriptor( pNewLib, aLibDesc );
					}
				}
   				mxSFI->kill( aPrevFolder );
			}	// if( bMoveOk )
        }
    }

	return sal_True;
}

// Handle maLibInfoFileURL and maStorageURL correctly
void SfxLibraryContainer_Impl::checkStorageURL( const OUString& aSourceURL,
    OUString& aLibInfoFileURL, OUString& aStorageURL, OUString& aUnexpandedStorageURL )
{
    OUString aExpandedSourceURL = expand_url( aSourceURL );
    if( aExpandedSourceURL != aSourceURL )
        aUnexpandedStorageURL = aSourceURL;

	INetURLObject aInetObj( aExpandedSourceURL );
    OUString aExtension = aInetObj.getExtension();
    if( aExtension.compareToAscii( "xlb" ) == COMPARE_EQUAL )
    {
        // URL to xlb file
		aLibInfoFileURL = aExpandedSourceURL;
        aInetObj.removeSegment();
		aStorageURL = aInetObj.GetMainURL( INetURLObject::NO_DECODE );
    }
    else
    {
        // URL to library folder
        aStorageURL = aExpandedSourceURL;
		aInetObj.insertName( maInfoFileName, sal_True, INetURLObject::LAST_SEGMENT, sal_True, INetURLObject::ENCODE_ALL );
		aInetObj.setExtension( OUString( RTL_CONSTASCII_USTRINGPARAM("xlb") ) );
		aLibInfoFileURL = aInetObj.GetMainURL( INetURLObject::NO_DECODE );
    }
}

SfxLibrary_Impl* SfxLibraryContainer_Impl::getImplLib( const String& rLibraryName )
{
	Any aLibAny = maNameContainer.getByName( rLibraryName ) ;
	Reference< XNameAccess > xNameAccess;
	aLibAny >>= xNameAccess;
	SfxLibrary_Impl* pImplLib = static_cast< SfxLibrary_Impl* >( xNameAccess.get() );
    return pImplLib;
}


// Storing with password encryption

// Empty implementation, avoids unneccesary implementation in dlgcont.cxx
sal_Bool SfxLibraryContainer_Impl::implStorePasswordLibrary( 
    SfxLibrary_Impl*,
    const OUString&, 
    const uno::Reference< embed::XStorage >&,
    const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& )

{
    return sal_False;
}

sal_Bool SfxLibraryContainer_Impl::implStorePasswordLibrary( 
    SfxLibrary_Impl* /*pLib*/, 
    const ::rtl::OUString& /*aName*/, 
    const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& /*xStorage*/,
    const ::rtl::OUString& /*aTargetURL*/, 
    const Reference< XSimpleFileAccess > /*xToUseSFI*/,
    const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& /*Handler*/ )
{
    return sal_False;
}

sal_Bool SfxLibraryContainer_Impl::implLoadPasswordLibrary( 
    SfxLibrary_Impl* /*pLib*/, 
    const OUString& /*Name*/, 
    sal_Bool /*bVerifyPasswordOnly*/ )
throw(WrappedTargetException, RuntimeException)
{
    return sal_True;
}


#define EXPAND_PROTOCOL "vnd.sun.star.expand"
#define OUSTR(x) ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(x) )

OUString SfxLibraryContainer_Impl::createAppLibraryFolder
    ( SfxLibrary_Impl* pLib, const OUString& aName )
{
	OUString aLibDirPath = pLib->maStorageURL;
	if( !aLibDirPath.getLength() )
    {
		INetURLObject aInetObj( String(maLibraryPath).GetToken(1) );
		aInetObj.insertName( aName, sal_True, INetURLObject::LAST_SEGMENT, sal_True, INetURLObject::ENCODE_ALL );
        checkStorageURL( aInetObj.GetMainURL( INetURLObject::NO_DECODE ), pLib->maLibInfoFileURL,
            pLib->maStorageURL, pLib->maUnexpandedStorageURL );
		aLibDirPath = pLib->maStorageURL;
    }

	if( !mxSFI->isFolder( aLibDirPath ) )
    {
	    try
	    {
		    mxSFI->createFolder( aLibDirPath );
        }
        catch( Exception& )
        {}
    }

    return aLibDirPath;
}

// Storing
void SfxLibraryContainer_Impl::implStoreLibrary( SfxLibrary_Impl* pLib,
	const OUString& aName, const uno::Reference< embed::XStorage >& xStorage )
{
	OUString aDummyLocation;
	Reference< XSimpleFileAccess > xDummySFA;
	implStoreLibrary( pLib, aName, xStorage, aDummyLocation, xDummySFA );
}

// New variant for library export
void SfxLibraryContainer_Impl::implStoreLibrary( SfxLibrary_Impl* pLib,
	const OUString& aName, const uno::Reference< embed::XStorage >& xStorage,
	const ::rtl::OUString& aTargetURL, Reference< XSimpleFileAccess > xToUseSFI )
{
	sal_Bool bLink = pLib->mbLink;
	sal_Bool bStorage = xStorage.is() && !bLink;

	Sequence< OUString > aElementNames = pLib->getElementNames();
	sal_Int32 nNameCount = aElementNames.getLength();
	const OUString* pNames = aElementNames.getConstArray();

	if( bStorage )
	{
		for( sal_Int32 i = 0 ; i < nNameCount ; i++ )
		{
			OUString aElementName = pNames[ i ];

			OUString aStreamName = aElementName;
			aStreamName += String( RTL_CONSTASCII_USTRINGPARAM(".xml") );

			Any aElement = pLib->getByName( aElementName );
			if( isLibraryElementValid( aElement ) )
			{
				try {
					uno::Reference< io::XStream > xElementStream = xStorage->openStreamElement(
																		aStreamName,
																		embed::ElementModes::READWRITE );
                    //if ( !xElementStream.is() )
                    //    throw uno::RuntimeException(); // TODO: method must either return the stream or throw an exception

					String aPropName( String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM("MediaType") ) );
					OUString aMime( RTL_CONSTASCII_USTRINGPARAM("text/xml") );

					uno::Reference< beans::XPropertySet > xProps( xElementStream, uno::UNO_QUERY );
					OSL_ENSURE( xProps.is(), "The StorageStream must implement XPropertySet interface!\n" );
                    //if ( !xProps.is() ) //TODO
                    //    throw uno::RuntimeException();

                    if ( xProps.is() )
                    {
                        xProps->setPropertyValue( aPropName, uno::makeAny( aMime ) );

                        // #87671 Allow encryption
//REMOVE	                        aPropName = String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM("Encrypted") );
                        aPropName = String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "UseCommonStoragePasswordEncryption" ) );
                        xProps->setPropertyValue( aPropName, uno::makeAny( sal_True ) );

                        Reference< XOutputStream > xOutput = xElementStream->getOutputStream();
                        writeLibraryElement( aElement, aElementName, xOutput );
						// writeLibraryElement closes the stream
                        // xOutput->closeOutput();
                    }
				}
				catch( uno::Exception& )
				{
					OSL_ENSURE( sal_False, "Problem during storing of library!\n" );
                    // TODO: error handling?
				}
			}
		}
	}
	else
	{
		// Export?
		bool bExport = aTargetURL.getLength();
		try
		{
			Reference< XSimpleFileAccess > xSFI = mxSFI;
			if( xToUseSFI.is() )
				xSFI = xToUseSFI;

            OUString aLibDirPath;
			if( bExport )
			{
				INetURLObject aInetObj( aTargetURL );
				aInetObj.insertName( aName, sal_True, INetURLObject::LAST_SEGMENT, sal_True, INetURLObject::ENCODE_ALL );
				aLibDirPath = aInetObj.GetMainURL( INetURLObject::NO_DECODE );

				if( !xSFI->isFolder( aLibDirPath ) )
					xSFI->createFolder( aLibDirPath );
			}
			else
			{
	            aLibDirPath = createAppLibraryFolder( pLib, aName );
			}

			for( sal_Int32 i = 0 ; i < nNameCount ; i++ )
			{
				OUString aElementName = pNames[ i ];

				INetURLObject aElementInetObj( aLibDirPath );
				aElementInetObj.insertName( aElementName, sal_False,
					INetURLObject::LAST_SEGMENT, sal_True, INetURLObject::ENCODE_ALL );
				aElementInetObj.setExtension( maLibElementFileExtension );
				String aElementPath( aElementInetObj.GetMainURL( INetURLObject::NO_DECODE ) );

				Any aElement = pLib->getByName( aElementName );
				if( isLibraryElementValid( aElement ) )
				{
					// TODO: Check modified
		            try
		            {
					    if( xSFI->exists( aElementPath ) )
						    xSFI->kill( aElementPath );
    					Reference< XOutputStream > xOutput = xSFI->openFileWrite( aElementPath );
					    writeLibraryElement( aElement, aElementName, xOutput );
					    xOutput->closeOutput();
                    }
            		catch( Exception& )
                    {
						if( bExport )
							throw;

			            SfxErrorContext aEc( ERRCTX_SFX_SAVEDOC, aElementPath );
                        ULONG nErrorCode = ERRCODE_IO_GENERAL;
                	    ErrorHandler::HandleError( nErrorCode );
                    }
				}
			}
		}
		catch( Exception& )
		{
			if( bExport )
				throw;
		}
	}
}

void SfxLibraryContainer_Impl::implStoreLibraryIndexFile( SfxLibrary_Impl* pLib,
	const ::xmlscript::LibDescriptor& rLib, const uno::Reference< embed::XStorage >& xStorage )
{
	OUString aDummyLocation;
	Reference< XSimpleFileAccess > xDummySFA;
	implStoreLibraryIndexFile( pLib, rLib, xStorage, aDummyLocation, xDummySFA );
}

// New variant for library export
void SfxLibraryContainer_Impl::implStoreLibraryIndexFile( SfxLibrary_Impl* pLib,
	const ::xmlscript::LibDescriptor& rLib, const uno::Reference< embed::XStorage >& xStorage,
	const ::rtl::OUString& aTargetURL, Reference< XSimpleFileAccess > xToUseSFI )
{
	// Create sax writer
	Reference< XExtendedDocumentHandler > xHandler(
		mxMSF->createInstance(
			OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.xml.sax.Writer") ) ), UNO_QUERY );
	if( !xHandler.is() )
	{
		OSL_ENSURE( 0, "### couln't create sax-writer component\n" );
		return;
	}

	sal_Bool bLink = pLib->mbLink;
	sal_Bool bStorage = xStorage.is() && !bLink;

	// Write info file
	uno::Reference< io::XOutputStream > xOut;
	uno::Reference< io::XStream > xInfoStream;
	if( bStorage )
	{
		OUString aStreamName( maInfoFileName );
		aStreamName += String( RTL_CONSTASCII_USTRINGPARAM("-lb.xml") );

		try {
			xInfoStream = xStorage->openStreamElement( aStreamName, embed::ElementModes::READWRITE );
            OSL_ENSURE( xInfoStream.is(), "No stream!\n" );
			uno::Reference< beans::XPropertySet > xProps( xInfoStream, uno::UNO_QUERY );
            //if ( !xProps.is() )
            //    throw uno::RuntimeException(); // TODO

            if ( xProps.is() )
            {
                String aPropName( String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM("MediaType") ) );
                OUString aMime( RTL_CONSTASCII_USTRINGPARAM("text/xml") );
                xProps->setPropertyValue( aPropName, uno::makeAny( aMime ) );

                // #87671 Allow encryption
//REMOVE	                aPropName = String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM("Encrypted") );
                aPropName = String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "UseCommonStoragePasswordEncryption" ) );
                xProps->setPropertyValue( aPropName, uno::makeAny( sal_True ) );

                xOut = xInfoStream->getOutputStream();
            }
		}
		catch( uno::Exception& )
		{
			OSL_ENSURE( sal_False, "Problem during storing of library index file!\n" );
            // TODO: error handling?
		}
	}
	else
	{
		// Export?
		bool bExport = aTargetURL.getLength();
		Reference< XSimpleFileAccess > xSFI = mxSFI;
		if( xToUseSFI.is() )
			xSFI = xToUseSFI;

        OUString aLibInfoPath;
		if( bExport )
		{
			INetURLObject aInetObj( aTargetURL );
			aInetObj.insertName( rLib.aName, sal_True, INetURLObject::LAST_SEGMENT, sal_True, INetURLObject::ENCODE_ALL );
			OUString aLibDirPath = aInetObj.GetMainURL( INetURLObject::NO_DECODE );
			if( !xSFI->isFolder( aLibDirPath ) )
				xSFI->createFolder( aLibDirPath );

			aInetObj.insertName( maInfoFileName, sal_True, INetURLObject::LAST_SEGMENT, sal_True, INetURLObject::ENCODE_ALL );
			aInetObj.setExtension( OUString( RTL_CONSTASCII_USTRINGPARAM("xlb") ) );
			aLibInfoPath = aInetObj.GetMainURL( INetURLObject::NO_DECODE );
		}
		else
		{
			createAppLibraryFolder( pLib, rLib.aName );
			aLibInfoPath = pLib->maLibInfoFileURL;
		}

		try
		{
		    if( xSFI->exists( aLibInfoPath ) )
			    xSFI->kill( aLibInfoPath );
		    xOut = xSFI->openFileWrite( aLibInfoPath );
        }
        catch( Exception& )
        {
			if( bExport )
				throw;

			SfxErrorContext aEc( ERRCTX_SFX_SAVEDOC, aLibInfoPath );
            ULONG nErrorCode = ERRCODE_IO_GENERAL;
            ErrorHandler::HandleError( nErrorCode );
        }
	}
	if( !xOut.is() )
	{
		OSL_ENSURE( 0, "### couln't open output stream\n" );
		return;
	}

	Reference< XActiveDataSource > xSource( xHandler, UNO_QUERY );
	xSource->setOutputStream( xOut );

    xmlscript::exportLibrary( xHandler, rLib );
}


sal_Bool SfxLibraryContainer_Impl::implLoadLibraryIndexFile(  SfxLibrary_Impl* pLib,
    ::xmlscript::LibDescriptor& rLib, const uno::Reference< embed::XStorage >& xStorage, const OUString& aIndexFileName )
{
	Reference< XParser > xParser( mxMSF->createInstance(
		OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.xml.sax.Parser") ) ), UNO_QUERY );
	if( !xParser.is() )
	{
		OSL_ENSURE( 0, "### couln't create sax parser component\n" );
		return sal_False;
	}

	sal_Bool bLink = sal_False;
	sal_Bool bStorage = sal_False;
    if( pLib )
    {
	    bLink = pLib->mbLink;
	    bStorage = xStorage.is() && !bLink;
    }

	// Read info file
	uno::Reference< io::XInputStream > xInput;
    String aLibInfoPath;
	if( bStorage )
	{
		aLibInfoPath = maInfoFileName;
		aLibInfoPath += String( RTL_CONSTASCII_USTRINGPARAM("-lb.xml") );

		try {
			uno::Reference< io::XStream > xInfoStream =
						xStorage->openStreamElement( aLibInfoPath, embed::ElementModes::READ );
			xInput = xInfoStream->getInputStream();
		}
		catch( uno::Exception& )
		{}
	}
	else
	{
		// Create Input stream
        //String aLibInfoPath; // attention: THIS PROBLEM MUST BE REVIEWED BY SCRIPTING OWNER!!!

        if( pLib )
        {
            createAppLibraryFolder( pLib, rLib.aName );
            aLibInfoPath = pLib->maLibInfoFileURL;
        }
        else
            aLibInfoPath = aIndexFileName;

		try
		{
			xInput = mxSFI->openFileRead( aLibInfoPath );
		}
		catch( Exception& )
		{
            xInput.clear();
			if( !GbMigrationSuppressErrors )
			{
				SfxErrorContext aEc( ERRCTX_SFX_LOADBASIC, aLibInfoPath );
				ULONG nErrorCode = ERRCODE_IO_GENERAL;
				ErrorHandler::HandleError( nErrorCode );
			}
		}
	}
	if( !xInput.is() )
	{
		// OSL_ENSURE( 0, "### couln't open input stream\n" );
		return sal_False;
	}

	InputSource source;
	source.aInputStream = xInput;
	source.sSystemId 	= aLibInfoPath;

	// start parsing
	try {
		xParser->setDocumentHandler( ::xmlscript::importLibrary( rLib ) );
		xParser->parseStream( source );
	}
	catch( Exception& )
	{
		// throw WrappedTargetException( OUString::createFromAscii( "parsing error!\n" ),
		//								Reference< XInterface >(),
		//								makeAny( e ) );
		OSL_ENSURE( 0, "Parsing error\n" );
		SfxErrorContext aEc( ERRCTX_SFX_LOADBASIC, aLibInfoPath );
        ULONG nErrorCode = ERRCODE_IO_GENERAL;
        ErrorHandler::HandleError( nErrorCode );
		return sal_False;
	}

    if( !pLib )
    {
		Reference< XNameContainer > xLib = createLibrary( rLib.aName );
		pLib = static_cast< SfxLibrary_Impl* >( xLib.get() );
		pLib->mbLoaded = sal_False;
        rLib.aStorageURL = aIndexFileName;
        checkStorageURL( rLib.aStorageURL, pLib->maLibInfoFileURL, pLib->maStorageURL,
            pLib->maUnexpandedStorageURL );

        implImportLibDescriptor( pLib, rLib );
    }

    return sal_True;
}

void SfxLibraryContainer_Impl::implImportLibDescriptor
    ( SfxLibrary_Impl* pLib, ::xmlscript::LibDescriptor& rLib )
{
    if( !pLib->mbInitialised )
    {
	    sal_Int32 nElementCount = rLib.aElementNames.getLength();
	    const OUString* pElementNames = rLib.aElementNames.getConstArray();
	    Any aDummyElement = createEmptyLibraryElement();
	    for( sal_Int32 i = 0 ; i < nElementCount ; i++ )
	    {
		    pLib->maNameContainer.insertByName( pElementNames[i], aDummyElement );
	    }
        pLib->mbPasswordProtected = rLib.bPasswordProtected;
        pLib->mbReadOnly = rLib.bReadOnly;
		pLib->mbPreload  = rLib.bPreload;
        pLib->mbModified = sal_False;

        pLib->mbInitialised = sal_True;
    }
}


void SfxLibraryContainer_Impl::storeLibraries( sal_Bool bComplete )
{
	uno::Reference< embed::XStorage > xStorage;
	storeLibraries_Impl( xStorage, bComplete );
}

void SfxLibraryContainer_Impl::storeLibrariesToStorage( const uno::Reference< embed::XStorage >& xStorage )
{
	sal_Bool bComplete = sal_True;
	storeLibraries_Impl( xStorage, bComplete );
}

bool SfxLibraryContainer_Impl::LegacyPsswdBinaryLimitExceeded( Sequence< rtl::OUString >& rNames )
{
	Sequence< OUString > aNames = maNameContainer.getElementNames();
	const OUString* pNames = aNames.getConstArray();
	sal_Int32 i, nNameCount = aNames.getLength();
	for( i = 0 ; i < nNameCount ; i++ )
	{
		SfxLibrary_Impl* pLib = getImplLib( pNames[ i ] );
		if( !pLib->mbSharedIndexFile  && pLib->mbPasswordProtected 
		)
		{
			StarBASIC* pBasicLib = NULL;
			if ( mpBasMgr && ( pBasicLib = mpBasMgr->GetLib( pNames[ i ] ) ) )
			{
				Sequence< OUString > aElementNames = pLib->getElementNames();
				sal_Int32 nLen = aElementNames.getLength();
				const OUString* pStr = aElementNames.getConstArray();
				Sequence< OUString > aBigModules( nLen );
				sal_Int32 nBigModules = 0;
				
				for( sal_Int32 index = 0 ; index < nLen ; index++ )
				{
					OUString aElementName = pStr[ index ];
					SbModule* pMod = pBasicLib->FindModule( aElementName );
					if ( pMod )
					{
						if ( pMod->ExceedsLegacyModuleSize() )
						{
							aBigModules[ nBigModules++ ] = aElementName;
						}
					}

				}
				if ( nBigModules )
				{
					aBigModules.realloc( nBigModules );	
					rNames = aBigModules;
					return true;
				}
			}

		}
	}
	return false;

}

// Methods of new XLibraryStorage interface?
void SfxLibraryContainer_Impl::storeLibraries_Impl( const uno::Reference< embed::XStorage >& xStorage, sal_Bool bComplete )
{
	Sequence< OUString > aNames = maNameContainer.getElementNames();
	const OUString* pNames = aNames.getConstArray();
	sal_Int32 i, nNameCount = aNames.getLength();

	// Don't count libs from shared index file
	sal_Int32 nLibsToSave = nNameCount;
	for( i = 0 ; i < nNameCount ; i++ )
	{
        SfxLibrary_Impl* pImplLib = getImplLib( pNames[ i ] );
		if( pImplLib->mbSharedIndexFile )
			nLibsToSave--;
	}
    if( !nLibsToSave )
        return;

	::xmlscript::LibDescriptorArray* pLibArray = new ::xmlscript::LibDescriptorArray( nLibsToSave );

	// Write to storage?
	sal_Bool bStorage = xStorage.is();
	uno::Reference< embed::XStorage > xLibrariesStor;
	uno::Reference< embed::XStorage > xSourceLibrariesStor;
	if( bStorage )
	{
		// first of all, clean the target library storage, since the storing procedure must do overwrite
		try {
			if ( xStorage->hasByName( maLibrariesDir ) )
				xStorage->removeElement( maLibrariesDir );
		}
		catch( uno::Exception& )
		{
			uno::Any exc( cppu::getCaughtException() ); // cppuhelper/exc_hlp.hxx
			::rtl::OUString msg( comphelper::anyToString(exc) ); // comphelper/anytostring.hxx
			OSL_ENSURE( 0, rtl::OUStringToOString( msg, RTL_TEXTENCODING_ASCII_US ).getStr() );
			return;
		}

        // Don't write if only empty standard lib exists
        if( nNameCount == 1 )
        {
            // Must be standard lib
		    Any aLibAny = maNameContainer.getByName( pNames[0] );
		    Reference< XNameAccess > xNameAccess;
		    aLibAny >>= xNameAccess;
            if( !xNameAccess->hasElements() )
                return;
        }

		try {
			xLibrariesStor = xStorage->openStorageElement( maLibrariesDir, embed::ElementModes::READWRITE );
			if ( !xLibrariesStor.is() )
				throw uno::RuntimeException();
		}
		catch( uno::Exception& )
		{
			uno::Any exc( cppu::getCaughtException() ); // cppuhelper/exc_hlp.hxx
			::rtl::OUString msg( comphelper::anyToString(exc) ); // comphelper/anytostring.hxx
			OSL_ENSURE( 0, rtl::OUStringToOString( msg, RTL_TEXTENCODING_ASCII_US ).getStr() );
			return;
		}

		try {
			xSourceLibrariesStor = mxStorage->openStorageElement( maLibrariesDir, embed::ElementModes::READ );
		}
		catch( uno::Exception& )
		{}
	}

	int iArray = 0;
	for( i = 0 ; i < nNameCount ; i++ )
	{
        SfxLibrary_Impl* pImplLib = getImplLib( pNames[ i ] );
		if( pImplLib->mbSharedIndexFile )
			continue;
		::xmlscript::LibDescriptor& rLib = pLibArray->mpLibs[iArray];
		rLib.aName = pNames[ i ];
		iArray++;

		rLib.bLink = pImplLib->mbLink;
		if( !bStorage || pImplLib->mbLink )
		{
			rLib.aStorageURL = ( pImplLib->maUnexpandedStorageURL.getLength() ) ?
				pImplLib->maUnexpandedStorageURL : pImplLib->maLibInfoFileURL;
		}
		rLib.bReadOnly = pImplLib->mbReadOnly;
		rLib.bPreload = pImplLib->mbPreload;
		rLib.bPasswordProtected = pImplLib->mbPasswordProtected;
		rLib.aElementNames = pImplLib->getElementNames();

		if( pImplLib->mbModified || bComplete )
		{
            // Can we copy the storage?
            if( !mbOldInfoFormat && !pImplLib->mbModified && !mbOasis2OOoFormat && xSourceLibrariesStor.is() )
            {
				try {
                	xSourceLibrariesStor->copyElementTo( rLib.aName, xLibrariesStor, rLib.aName );
				} catch( uno::Exception& )
				{
					OSL_ENSURE( sal_False, "Problem during storing of libraries!\n" );
                    // TODO: error handling?
				}
            }
            else
            {
				uno::Reference< embed::XStorage > xLibraryStor;
				if( bStorage )
				{
					try {
						xLibraryStor = xLibrariesStor->openStorageElement(
																		rLib.aName,
																		embed::ElementModes::READWRITE );
					}
					catch( uno::Exception& )
					{
						OSL_ENSURE( 0, "### couln't create sub storage for library\n" );
						return;
					}
				}

				// Maybe lib is not loaded?!
				if( bComplete )
					loadLibrary( rLib.aName );

    			if( pImplLib->mbPasswordProtected )
				    implStorePasswordLibrary( pImplLib, rLib.aName, xLibraryStor, uno::Reference< task::XInteractionHandler >() );
                    // TODO: Check return value
                else
				    implStoreLibrary( pImplLib, rLib.aName, xLibraryStor );

                implStoreLibraryIndexFile( pImplLib, rLib, xLibraryStor );
				if( bStorage )
				{
					try {
						uno::Reference< embed::XTransactedObject > xTransact( xLibraryStor, uno::UNO_QUERY );
                        OSL_ENSURE( xTransact.is(), "The storage must implement XTransactedObject!\n" );
						if ( !xTransact.is() )
							throw uno::RuntimeException();

						xTransact->commit();
					}
					catch( uno::Exception& )
					{
						OSL_ENSURE( sal_False, "Problem during storing of libraries!\n" );
                        // TODO: error handling
					}
				}
            }

			mbModified = sal_True;
			pImplLib->mbModified = sal_False;
		}

        // For container info ReadOnly refers to mbReadOnlyLink
		rLib.bReadOnly = pImplLib->mbReadOnlyLink;
	}

	if( !mbOldInfoFormat && !mbModified )
		return;
	mbModified = sal_False;
    mbOldInfoFormat = sal_False;

	// Write library container info
	// Create sax writer
	Reference< XExtendedDocumentHandler > xHandler(
		mxMSF->createInstance(
			OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.xml.sax.Writer") ) ), UNO_QUERY );
	if( !xHandler.is() )
	{
		OSL_ENSURE( 0, "### couln't create sax-writer component\n" );
		return;
	}

	// Write info file
	uno::Reference< io::XOutputStream > xOut;
	uno::Reference< io::XStream > xInfoStream;
	if( bStorage )
	{
		OUString aStreamName( maInfoFileName );
		aStreamName += String( RTL_CONSTASCII_USTRINGPARAM("-lc.xml") );

		try {
			xInfoStream = xLibrariesStor->openStreamElement( aStreamName, embed::ElementModes::READWRITE );
			uno::Reference< beans::XPropertySet > xProps( xInfoStream, uno::UNO_QUERY );
			OSL_ENSURE ( xProps.is(), "The stream must implement XPropertySet!\n" );
			if ( !xProps.is() )
				throw uno::RuntimeException();

			String aPropName( String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM("MediaType") ) );
			OUString aMime( RTL_CONSTASCII_USTRINGPARAM("text/xml") );
			xProps->setPropertyValue( aPropName, uno::makeAny( aMime ) );

            // #87671 Allow encryption
			aPropName = String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM("UseCommonStoragePasswordEncryption") );
			xProps->setPropertyValue( aPropName, uno::makeAny( sal_True ) );

			xOut = xInfoStream->getOutputStream();
		}
		catch( uno::Exception& )
		{
			ULONG nErrorCode = ERRCODE_IO_GENERAL;
			ErrorHandler::HandleError( nErrorCode );
		}
	}
	else
	{
		// Create Output stream
		INetURLObject aLibInfoInetObj( String(maLibraryPath).GetToken(1) );
		aLibInfoInetObj.insertName( maInfoFileName, sal_True, INetURLObject::LAST_SEGMENT, sal_True, INetURLObject::ENCODE_ALL );
		aLibInfoInetObj.setExtension( OUString( RTL_CONSTASCII_USTRINGPARAM("xlc") ) );
		String aLibInfoPath( aLibInfoInetObj.GetMainURL( INetURLObject::NO_DECODE ) );

		try
		{
		    if( mxSFI->exists( aLibInfoPath ) )
			    mxSFI->kill( aLibInfoPath );
		    xOut = mxSFI->openFileWrite( aLibInfoPath );
        }
        catch( Exception& )
        {
            xOut.clear();
			SfxErrorContext aEc( ERRCTX_SFX_SAVEDOC, aLibInfoPath );
            ULONG nErrorCode = ERRCODE_IO_GENERAL;
            ErrorHandler::HandleError( nErrorCode );
        }

	}
	if( !xOut.is() )
	{
		OSL_ENSURE( 0, "### couln't open output stream\n" );
		return;
	}

	Reference< XActiveDataSource > xSource( xHandler, UNO_QUERY );
	xSource->setOutputStream( xOut );

    try 
	{
		xmlscript::exportLibraryContainer( xHandler, pLibArray );
		if ( bStorage )
		{
            uno::Reference< embed::XTransactedObject > xTransact( xLibrariesStor, uno::UNO_QUERY );
            OSL_ENSURE( xTransact.is(), "The storage must implement XTransactedObject!\n" );
            if ( !xTransact.is() )
                throw uno::RuntimeException();

            xTransact->commit();
        }
    }
    catch( uno::Exception& )
    {
		OSL_ENSURE( sal_False, "Problem during storing of libraries!\n" );
        ULONG nErrorCode = ERRCODE_IO_GENERAL;
        ErrorHandler::HandleError( nErrorCode );
    }

	delete pLibArray;
}


// Methods XElementAccess
Type SfxLibraryContainer_Impl::getElementType()
	throw(RuntimeException)
{
	return maNameContainer.getElementType();
}

sal_Bool SfxLibraryContainer_Impl::hasElements()
	throw(RuntimeException)
{
	sal_Bool bRet = maNameContainer.hasElements();
	return bRet;
}

// Methods XNameAccess
Any SfxLibraryContainer_Impl::getByName( const OUString& aName )
	throw(NoSuchElementException, WrappedTargetException, RuntimeException)
{
	Any aRetAny = maNameContainer.getByName( aName ) ;
	return aRetAny;
}

Sequence< OUString > SfxLibraryContainer_Impl::getElementNames()
	throw(RuntimeException)
{
	return maNameContainer.getElementNames();
}

sal_Bool SfxLibraryContainer_Impl::hasByName( const OUString& aName )
	throw(RuntimeException)
{
	sal_Bool bRet = maNameContainer.hasByName( aName ) ;
	return bRet;
}

// Methods XLibraryContainer
Reference< XNameContainer > SAL_CALL SfxLibraryContainer_Impl::createLibrary( const OUString& Name )
		throw(IllegalArgumentException, ElementExistException, RuntimeException)
{
	SfxLibrary_Impl* pNewLib = implCreateLibrary();
    pNewLib->maLibElementFileExtension = maLibElementFileExtension;

	createVariableURL( pNewLib->maUnexpandedStorageURL, Name, maInfoFileName, true );

	Reference< XNameAccess > xNameAccess = static_cast< XNameAccess* >( pNewLib );
	Any aElement;
	aElement <<= xNameAccess;
	maNameContainer.insertByName( Name, aElement );
	mbModified = sal_True;
    Reference< XNameContainer > xRet( xNameAccess, UNO_QUERY );
	return xRet;
}

Reference< XNameAccess > SAL_CALL SfxLibraryContainer_Impl::createLibraryLink
	( const OUString& Name, const OUString& StorageURL, sal_Bool ReadOnly )
		throw(IllegalArgumentException, ElementExistException, RuntimeException)
{
    // TODO: Check other reasons to force ReadOnly status
	//if( !ReadOnly )
	//{
	//}

    OUString aLibInfoFileURL;
    OUString aLibDirURL;
    OUString aUnexpandedStorageURL;
    checkStorageURL( StorageURL, aLibInfoFileURL, aLibDirURL, aUnexpandedStorageURL );


	SfxLibrary_Impl* pNewLib = implCreateLibraryLink( aLibInfoFileURL, aLibDirURL, ReadOnly );
    pNewLib->maLibElementFileExtension = maLibElementFileExtension;
    pNewLib->maUnexpandedStorageURL = aUnexpandedStorageURL;

    OUString aInitFileName;
	uno::Reference< embed::XStorage > xDummyStor;
    ::xmlscript::LibDescriptor aLibDesc;
    /*sal_Bool bReadIndexFile = */implLoadLibraryIndexFile( pNewLib, aLibDesc, xDummyStor, aInitFileName );
    implImportLibDescriptor( pNewLib, aLibDesc );

	Reference< XNameAccess > xRet = static_cast< XNameAccess* >( pNewLib );
	Any aElement;
	aElement <<= xRet;
	maNameContainer.insertByName( Name, aElement );
	mbModified = sal_True;
	return xRet;
}

void SAL_CALL SfxLibraryContainer_Impl::removeLibrary( const OUString& Name )
	throw(NoSuchElementException, WrappedTargetException, RuntimeException)
{
    // Get and hold library before removing
	Any aLibAny = maNameContainer.getByName( Name ) ;
	Reference< XNameAccess > xNameAccess;
	aLibAny >>= xNameAccess;
	SfxLibrary_Impl* pImplLib = static_cast< SfxLibrary_Impl* >( xNameAccess.get() );
	if( pImplLib->mbReadOnly && !pImplLib->mbLink )
		throw IllegalArgumentException();

    // Remove from container
	maNameContainer.removeByName( Name );
	mbModified = sal_True;

    // Delete library files, but not for linked libraries
    if( !pImplLib->mbLink )
    {
	    if( mxStorage.is() )
            return;
	    if( xNameAccess->hasElements() )
	    {
		    Sequence< OUString > aNames = pImplLib->getElementNames();
		    sal_Int32 nNameCount = aNames.getLength();
		    const OUString* pNames = aNames.getConstArray();
		    for( sal_Int32 i = 0 ; i < nNameCount ; i++ )
		    {
			    OUString aElementName = pNames[ i ];
                pImplLib->removeByName( aElementName );
		    }
	    }

        // Delete index file
        createAppLibraryFolder( pImplLib, Name );
        String aLibInfoPath = pImplLib->maLibInfoFileURL;
		try
		{
		    if( mxSFI->exists( aLibInfoPath ) )
			    mxSFI->kill( aLibInfoPath );
        }
        catch( Exception& ) {}

        // Delete folder if empty
	    INetURLObject aInetObj( String(maLibraryPath).GetToken(1) );
	    aInetObj.insertName( Name, sal_True, INetURLObject::LAST_SEGMENT,
		    sal_True, INetURLObject::ENCODE_ALL );
	    OUString aLibDirPath = aInetObj.GetMainURL( INetURLObject::NO_DECODE );

	    try
	    {
	        if( mxSFI->isFolder( aLibDirPath ) )
	        {
                Sequence< OUString > aContentSeq = mxSFI->getFolderContents( aLibDirPath, true );
    		    sal_Int32 nCount = aContentSeq.getLength();
	            if( !nCount )
		            mxSFI->kill( aLibDirPath );
	        }
        }
        catch( Exception& )
        {
        }
    }
}

sal_Bool SAL_CALL SfxLibraryContainer_Impl::isLibraryLoaded( const OUString& Name )
	throw(NoSuchElementException, RuntimeException)
{
    SfxLibrary_Impl* pImplLib = getImplLib( Name );
	sal_Bool bRet = pImplLib->mbLoaded;
	return bRet;
}


void SAL_CALL SfxLibraryContainer_Impl::loadLibrary( const OUString& Name )
	throw(NoSuchElementException, WrappedTargetException, RuntimeException)
{
	Any aLibAny = maNameContainer.getByName( Name ) ;
	Reference< XNameAccess > xNameAccess;
	aLibAny >>= xNameAccess;
	SfxLibrary_Impl* pImplLib = static_cast< SfxLibrary_Impl* >( xNameAccess.get() );

    sal_Bool bLoaded = pImplLib->mbLoaded;
	pImplLib->mbLoaded = sal_True;
	if( !bLoaded && xNameAccess->hasElements() )
	{
        if( pImplLib->mbPasswordProtected )
        {
            implLoadPasswordLibrary( pImplLib, Name );
            return;
        }

		sal_Bool bLink = pImplLib->mbLink;
		sal_Bool bStorage = mxStorage.is() && !bLink;

		uno::Reference< embed::XStorage > xLibrariesStor;
		uno::Reference< embed::XStorage > xLibraryStor;
		if( bStorage )
		{
			try {
				xLibrariesStor = mxStorage->openStorageElement( maLibrariesDir, embed::ElementModes::READ );
				OSL_ENSURE( xLibrariesStor.is(), "The method must either throw exception or return a storage!\n" );
				if ( !xLibrariesStor.is() )
					throw uno::RuntimeException();

				xLibraryStor = xLibrariesStor->openStorageElement( Name, embed::ElementModes::READ );
				OSL_ENSURE( xLibraryStor.is(), "The method must either throw exception or return a storage!\n" );
				if ( !xLibrariesStor.is() )
					throw uno::RuntimeException();
			}
			catch( uno::Exception& )
			{
				OSL_ENSURE( 0, "### couln't open sub storage for library\n" );
				return;
			}
		}

		Sequence< OUString > aNames = pImplLib->getElementNames();
		sal_Int32 nNameCount = aNames.getLength();
		const OUString* pNames = aNames.getConstArray();
		for( sal_Int32 i = 0 ; i < nNameCount ; i++ )
		{
			OUString aElementName = pNames[ i ];

			OUString aFile;
			uno::Reference< io::XInputStream > xInStream;

			if( bStorage )
			{
				uno::Reference< io::XStream > xElementStream;

				aFile = aElementName;
				aFile += String( RTL_CONSTASCII_USTRINGPARAM(".xml") );

				try {
					xElementStream = xLibraryStor->openStreamElement( aFile, embed::ElementModes::READ );
				} catch( uno::Exception& )
				{}

				if( !xElementStream.is() )
				{
					// Check for EA2 document version with wrong extensions
					aFile = aElementName;
					aFile += String( RTL_CONSTASCII_USTRINGPARAM(".") );
					aFile += maLibElementFileExtension;
					try {
						xElementStream = xLibraryStor->openStreamElement( aFile, embed::ElementModes::READ );
					} catch( uno::Exception& )
					{}
				}

				if ( xElementStream.is() )
					xInStream = xElementStream->getInputStream();

				if ( !xInStream.is() )
				{
					OSL_ENSURE( 0, "### couln't open library element stream\n" );
					return;
				}
			}
			else
			{
		        String aLibDirPath = pImplLib->maStorageURL;
				INetURLObject aElementInetObj( aLibDirPath );
				aElementInetObj.insertName( aElementName, sal_False,
					INetURLObject::LAST_SEGMENT, sal_True, INetURLObject::ENCODE_ALL );
				aElementInetObj.setExtension( maLibElementFileExtension );
				aFile = aElementInetObj.GetMainURL( INetURLObject::NO_DECODE );
			}

			Any aAny = importLibraryElement( aFile, xInStream );
			if( pImplLib->hasByName( aElementName ) )
            {
                if( aAny.hasValue() )
				    pImplLib->maNameContainer.replaceByName( aElementName, aAny );
            }
			else
            {
				pImplLib->maNameContainer.insertByName( aElementName, aAny );
            }
		}

        pImplLib->mbModified = sal_False;
	}
}

// Methods XLibraryContainer2
sal_Bool SAL_CALL SfxLibraryContainer_Impl::isLibraryLink( const OUString& Name )
    throw (NoSuchElementException, RuntimeException)
{
    SfxLibrary_Impl* pImplLib = getImplLib( Name );
	sal_Bool bRet = pImplLib->mbLink;
	return bRet;
}

OUString SAL_CALL SfxLibraryContainer_Impl::getLibraryLinkURL( const OUString& Name )
    throw (IllegalArgumentException, NoSuchElementException, RuntimeException)
{
    SfxLibrary_Impl* pImplLib = getImplLib( Name );
	sal_Bool bLink = pImplLib->mbLink;
	if( !bLink )
		throw IllegalArgumentException();
    OUString aRetStr = pImplLib->maLibInfoFileURL;
    return aRetStr;
}

sal_Bool SAL_CALL SfxLibraryContainer_Impl::isLibraryReadOnly( const OUString& Name )
    throw (NoSuchElementException, RuntimeException)
{
    SfxLibrary_Impl* pImplLib = getImplLib( Name );
	sal_Bool bRet = pImplLib->mbReadOnly || (pImplLib->mbLink && pImplLib->mbReadOnlyLink);
	return bRet;
}

void SAL_CALL SfxLibraryContainer_Impl::setLibraryReadOnly( const OUString& Name, sal_Bool bReadOnly )
    throw (NoSuchElementException, RuntimeException)
{
    SfxLibrary_Impl* pImplLib = getImplLib( Name );
    if( pImplLib->mbLink )
    {
        if( pImplLib->mbReadOnlyLink != bReadOnly )
        {
            pImplLib->mbReadOnlyLink = bReadOnly;
            pImplLib->mbModified = sal_True;
            mbModified = sal_True;
        }
    }
    else
    {
        if( pImplLib->mbReadOnly != bReadOnly )
        {
	        pImplLib->mbReadOnly = bReadOnly;
            pImplLib->mbModified = sal_True;
        }
    }
}

void SAL_CALL SfxLibraryContainer_Impl::renameLibrary( const OUString& Name, const OUString& NewName )
    throw (NoSuchElementException, ElementExistException, RuntimeException)
{
	if( maNameContainer.hasByName( NewName ) )
		throw ElementExistException();

    // Get and hold library before removing
	Any aLibAny = maNameContainer.getByName( Name ) ;

	// #i24094 Maybe lib is not loaded!
	Reference< XNameAccess > xNameAccess;
	aLibAny >>= xNameAccess;
	SfxLibrary_Impl* pImplLib = static_cast< SfxLibrary_Impl* >( xNameAccess.get() );
	if( pImplLib->mbPasswordProtected && !pImplLib->mbPasswordVerified )
		return;		// Lib with unverified password cannot be renamed
	loadLibrary( Name );

    // Remove from container
	maNameContainer.removeByName( Name );
	mbModified = sal_True;

    // Rename library folder, but not for linked libraries
    bool bMovedSuccessful = true;

    // Rename files
    sal_Bool bStorage = mxStorage.is();
    if( !bStorage && !pImplLib->mbLink )
    {
        bMovedSuccessful = false;

	    OUString aLibDirPath = pImplLib->maStorageURL;

	    INetURLObject aDestInetObj( String(maLibraryPath).GetToken(1) );
	    aDestInetObj.insertName( NewName, sal_True, INetURLObject::LAST_SEGMENT,
		    sal_True, INetURLObject::ENCODE_ALL );
	    OUString aDestDirPath = aDestInetObj.GetMainURL( INetURLObject::NO_DECODE );

        // Store new URL
        OUString aLibInfoFileURL = pImplLib->maLibInfoFileURL;
        checkStorageURL( aDestDirPath, pImplLib->maLibInfoFileURL, pImplLib->maStorageURL,
            pImplLib->maUnexpandedStorageURL );

	    try
	    {
	        if( mxSFI->isFolder( aLibDirPath ) )
	        {
			    if( !mxSFI->isFolder( aDestDirPath ) )
				    mxSFI->createFolder( aDestDirPath );

                // Move index file
		        try
		        {
					if( mxSFI->exists( pImplLib->maLibInfoFileURL ) )
						mxSFI->kill( pImplLib->maLibInfoFileURL );
            	    mxSFI->move( aLibInfoFileURL, pImplLib->maLibInfoFileURL );
                }
            	catch( Exception& )
                {
                }

			    Sequence< OUString > aElementNames = xNameAccess->getElementNames();
			    sal_Int32 nNameCount = aElementNames.getLength();
			    const OUString* pNames = aElementNames.getConstArray();
			    for( sal_Int32 i = 0 ; i < nNameCount ; i++ )
			    {
				    OUString aElementName = pNames[ i ];

				    INetURLObject aElementInetObj( aLibDirPath );
				    aElementInetObj.insertName( aElementName, sal_False,
					    INetURLObject::LAST_SEGMENT, sal_True, INetURLObject::ENCODE_ALL );
				    aElementInetObj.setExtension( maLibElementFileExtension );
				    String aElementPath( aElementInetObj.GetMainURL( INetURLObject::NO_DECODE ) );

				    INetURLObject aElementDestInetObj( aDestDirPath );
				    aElementDestInetObj.insertName( aElementName, sal_False,
					    INetURLObject::LAST_SEGMENT, sal_True, INetURLObject::ENCODE_ALL );
				    aElementDestInetObj.setExtension( maLibElementFileExtension );
				    String aDestElementPath( aElementDestInetObj.GetMainURL( INetURLObject::NO_DECODE ) );

		            try
		            {
					    if( mxSFI->exists( aDestElementPath ) )
						    mxSFI->kill( aDestElementPath );
            	        mxSFI->move( aElementPath, aDestElementPath );
                    }
            		catch( Exception& )
                    {
                    }
			    }

                // Delete folder if empty
                Sequence< OUString > aContentSeq = mxSFI->getFolderContents( aLibDirPath, true );
    		    sal_Int32 nCount = aContentSeq.getLength();
	            if( !nCount )
                {
       	            mxSFI->kill( aLibDirPath );
                }

                bMovedSuccessful = true;
				pImplLib->mbModified = true;
	        }
        }
        catch( Exception& )
        {
            // Restore old library
        	maNameContainer.insertByName( Name, aLibAny ) ;
        }
    }

    if( bStorage && !pImplLib->mbLink )
		pImplLib->mbModified = true;

    if( bMovedSuccessful )
       	maNameContainer.insertByName( NewName, aLibAny ) ;

}


// Methods XLibraryContainerPassword
sal_Bool SAL_CALL SfxLibraryContainer_Impl::isLibraryPasswordProtected( const OUString& )
    throw (NoSuchElementException, RuntimeException)
{
	sal_Bool bRet = sal_False;
	return bRet;
}

sal_Bool SAL_CALL SfxLibraryContainer_Impl::isLibraryPasswordVerified( const OUString& )
    throw (IllegalArgumentException, NoSuchElementException, RuntimeException)
{
	throw IllegalArgumentException();
}

sal_Bool SAL_CALL SfxLibraryContainer_Impl::verifyLibraryPassword
    ( const OUString&, const OUString& )
        throw (IllegalArgumentException, NoSuchElementException, RuntimeException)
{
	throw IllegalArgumentException();
}

void SAL_CALL SfxLibraryContainer_Impl::changeLibraryPassword( 
    const OUString&, const OUString&, const OUString& )
        throw (IllegalArgumentException, NoSuchElementException, RuntimeException)
{
	throw IllegalArgumentException();
}

// Methods XContainer
void SAL_CALL SfxLibraryContainer_Impl::addContainerListener( const Reference< XContainerListener >& xListener )
	throw (RuntimeException)
{
	maNameContainer.setEventSource( static_cast< XInterface* >( (OWeakObject*)this ) );
	maNameContainer.addContainerListener( xListener );
}

void SAL_CALL SfxLibraryContainer_Impl::removeContainerListener( const Reference< XContainerListener >& xListener )
	throw (RuntimeException)
{
	maNameContainer.removeContainerListener( xListener );
}

// Methods XLibraryContainerExport
void SAL_CALL SfxLibraryContainer_Impl::exportLibrary( const OUString& Name, const OUString& URL, 
	const Reference< XInteractionHandler >& Handler ) 
		throw ( uno::Exception, NoSuchElementException, RuntimeException)
{
    SfxLibrary_Impl* pImplLib = getImplLib( Name );

	Reference< XSimpleFileAccess > xToUseSFI;
	if( Handler.is() )
	{
		xToUseSFI = Reference< XSimpleFileAccess >( mxMSF->createInstance
			( OUString::createFromAscii( "com.sun.star.ucb.SimpleFileAccess" ) ), UNO_QUERY );
		if( xToUseSFI.is() )
			xToUseSFI->setInteractionHandler( Handler );
	}

	// Maybe lib is not loaded?!
	loadLibrary( Name );

	uno::Reference< ::com::sun::star::embed::XStorage > xDummyStor;
    if( pImplLib->mbPasswordProtected )
		implStorePasswordLibrary( pImplLib, Name, xDummyStor, URL, xToUseSFI, Handler );
    else
		implStoreLibrary( pImplLib, Name, xDummyStor, URL, xToUseSFI );

	::xmlscript::LibDescriptor aLibDesc;
	aLibDesc.aName = Name;
	aLibDesc.bLink = false;				// Link status gets lost?
	aLibDesc.bReadOnly = pImplLib->mbReadOnly;
	aLibDesc.bPreload = false;			// Preload status gets lost?
	aLibDesc.bPasswordProtected = pImplLib->mbPasswordProtected;
	aLibDesc.aElementNames = pImplLib->getElementNames();

	implStoreLibraryIndexFile( pImplLib, aLibDesc, xDummyStor, URL, xToUseSFI );
}

OUString SfxLibraryContainer_Impl::expand_url( const OUString& url )
	throw(::com::sun::star::uno::RuntimeException)
{
    if (0 == url.compareToAscii( RTL_CONSTASCII_STRINGPARAM(EXPAND_PROTOCOL ":") ))
    {
        if( !mxMacroExpander.is() )
        {
            Reference< XPropertySet > xProps( mxMSF, UNO_QUERY );
            OSL_ASSERT( xProps.is() );
            if( xProps.is() )
            {
                Reference< XComponentContext > xContext;
                xProps->getPropertyValue(
                    OUString( RTL_CONSTASCII_USTRINGPARAM("DefaultContext") ) ) >>= xContext;
                OSL_ASSERT( xContext.is() );
                if( xContext.is() )
                {
                    Reference< util::XMacroExpander > xExpander;
                    xContext->getValueByName(
                        OUSTR("/singletons/com.sun.star.util.theMacroExpander") ) >>= xExpander;
                    if(! xExpander.is())
                    {
                        throw DeploymentException(
                            OUSTR("no macro expander singleton available!"), Reference< XInterface >() );
                    }
                    MutexGuard guard( Mutex::getGlobalMutex() );
                    if( !mxMacroExpander.is() )
                    {
                        mxMacroExpander = xExpander;
                    }
                }
            }
        }

        if( !mxMacroExpander.is() )
            return url;

        // cut protocol
        OUString macro( url.copy( sizeof (EXPAND_PROTOCOL ":") -1 ) );
        // decode uric class chars
        macro = Uri::decode( macro, rtl_UriDecodeWithCharset, RTL_TEXTENCODING_UTF8 );
        // expand macro string
        OUString ret( mxMacroExpander->expandMacros( macro ) );
        return ret;
    }
	else if( mxStringSubstitution.is() )
	{
		OUString ret( mxStringSubstitution->substituteVariables( url, false ) );
        return ret;
	}
    else
    {
        return url;
    }
}


//============================================================================

// Implementation class SfxLibrary_Impl

// Ctor
SfxLibrary_Impl::SfxLibrary_Impl( Type aType,
                                  Reference< XMultiServiceFactory > xMSF,
                                  Reference< XSimpleFileAccess > xSFI )
	: OComponentHelper( m_mutex )
    , mxMSF( xMSF )
    , mxSFI( xSFI )
	, maNameContainer( aType )
	, mbLoaded( sal_True )
	, mbModified( sal_True )
    , mbInitialised( sal_False )
    , mbLink( sal_False )
	, mbReadOnly( sal_False )
    , mbReadOnlyLink( sal_False )
	, mbPreload( sal_False )
	, mbPasswordProtected( sal_False )
	, mbPasswordVerified( sal_False )
    , mbDoc50Password( sal_False )
	, mbSharedIndexFile( sal_False )
{
}

SfxLibrary_Impl::SfxLibrary_Impl( Type aType,
                                  Reference< XMultiServiceFactory > xMSF,
                                  Reference< XSimpleFileAccess > xSFI ,
                                  const OUString& aLibInfoFileURL,
                                  const OUString& aStorageURL, sal_Bool ReadOnly )
	: OComponentHelper( m_mutex )
    , mxMSF( xMSF )
    , mxSFI( xSFI )
	, maNameContainer( aType )
	, mbLoaded( sal_False )
	, mbModified( sal_True )
    , mbInitialised( sal_False )
    , maLibInfoFileURL( aLibInfoFileURL )
	, maStorageURL( aStorageURL )
    , mbLink( sal_True )
	, mbReadOnly( sal_False )
    , mbReadOnlyLink( ReadOnly )
	, mbPreload( sal_False )
	, mbPasswordProtected( sal_False )
	, mbPasswordVerified( sal_False )
    , mbDoc50Password( sal_False )
	, mbSharedIndexFile( sal_False )
{
}


// Methods XInterface
Any SAL_CALL SfxLibrary_Impl::queryInterface( const Type& rType )
	throw( RuntimeException )
{
	Any aRet;

    /*
	if( mbReadOnly )
	{
		aRet = Any( ::cppu::queryInterface( rType,
			static_cast< XContainer * >( this ),
			static_cast< XNameAccess * >( this ) ) );
	}
	else
	{
    */
		aRet = Any( ::cppu::queryInterface( rType,
			static_cast< XContainer * >( this ),
			static_cast< XNameContainer * >( this ),
			static_cast< XNameAccess * >( this ) ) );
	//}
	if( !aRet.hasValue() )
		aRet = OComponentHelper::queryInterface( rType );
	return aRet;
}

// Methods XElementAccess
Type SfxLibrary_Impl::getElementType()
	throw(RuntimeException)
{
	return maNameContainer.getElementType();
}

sal_Bool SfxLibrary_Impl::hasElements()
	throw(RuntimeException)
{
	sal_Bool bRet = maNameContainer.hasElements();
	return bRet;
}

// Methods XNameAccess
Any SfxLibrary_Impl::getByName( const OUString& aName )
	throw(NoSuchElementException, WrappedTargetException, RuntimeException)
{
	Any aRetAny = maNameContainer.getByName( aName ) ;
	return aRetAny;
}

Sequence< OUString > SfxLibrary_Impl::getElementNames()
	throw(RuntimeException)
{
	return maNameContainer.getElementNames();
}

sal_Bool SfxLibrary_Impl::hasByName( const OUString& aName )
	throw(RuntimeException)
{
	sal_Bool bRet = maNameContainer.hasByName( aName );
	return bRet;
}

// Methods XNameReplace
void SfxLibrary_Impl::replaceByName( const OUString& aName, const Any& aElement )
	throw(IllegalArgumentException, NoSuchElementException, WrappedTargetException, RuntimeException)
{
	if( mbReadOnly || (mbLink && mbReadOnlyLink) )
        throw RuntimeException();

	maNameContainer.replaceByName( aName, aElement );
	mbModified = sal_True;
}


// Methods XNameContainer
void SfxLibrary_Impl::insertByName( const OUString& aName, const Any& aElement )
	throw(IllegalArgumentException, ElementExistException, WrappedTargetException, RuntimeException)
{
	if( mbReadOnly || (mbLink && mbReadOnlyLink) )
        throw RuntimeException();

	maNameContainer.insertByName( aName, aElement );
	mbModified = sal_True;
}

void SfxLibrary_Impl::removeByName( const OUString& Name )
	throw(NoSuchElementException, WrappedTargetException, RuntimeException)
{
	if( mbReadOnly || (mbLink && mbReadOnlyLink) )
        throw RuntimeException();

	maNameContainer.removeByName( Name );
	mbModified = sal_True;

    // Remove element file
	if( maStorageURL.getLength() )
	{
		INetURLObject aElementInetObj( maStorageURL );
		aElementInetObj.insertName( Name, sal_False,
			INetURLObject::LAST_SEGMENT, sal_True, INetURLObject::ENCODE_ALL );
		aElementInetObj.setExtension( maLibElementFileExtension );
		OUString aFile = aElementInetObj.GetMainURL( INetURLObject::NO_DECODE );

		try
		{
	        if( mxSFI->exists( aFile ) )
		        mxSFI->kill( aFile );
        }
        catch( Exception& )
        {
        }
	}

}

// XTypeProvider
Sequence< Type > SfxLibrary_Impl::getTypes()
	throw( RuntimeException )
{
	static OTypeCollection * s_pTypes_NameContainer = 0;
	{
		if( !s_pTypes_NameContainer )
		{
			MutexGuard aGuard( Mutex::getGlobalMutex() );
			if( !s_pTypes_NameContainer )
			{
				static OTypeCollection s_aTypes_NameContainer(
					::getCppuType( (const Reference< XNameContainer > *)0 ),
					::getCppuType( (const Reference< XContainer > *)0 ),
					OComponentHelper::getTypes() );
				s_pTypes_NameContainer = &s_aTypes_NameContainer;
			}
		}
		return s_pTypes_NameContainer->getTypes();
	}
}


Sequence< sal_Int8 > SfxLibrary_Impl::getImplementationId()
	throw( RuntimeException )
{
	static OImplementationId * s_pId_NameContainer = 0;
	{
		if( !s_pId_NameContainer )
		{
			MutexGuard aGuard( Mutex::getGlobalMutex() );
			if( !s_pId_NameContainer )
			{
				static OImplementationId s_aId_NameContainer;
				s_pId_NameContainer = &s_aId_NameContainer;
			}
		}
		return s_pId_NameContainer->getImplementationId();
	}
}


//============================================================================

// Methods XContainer
void SAL_CALL SfxLibrary_Impl::addContainerListener( const Reference< XContainerListener >& xListener )
	throw (RuntimeException)
{
	maNameContainer.setEventSource( static_cast< XInterface* >( (OWeakObject*)this ) );
	maNameContainer.addContainerListener( xListener );
}

void SAL_CALL SfxLibrary_Impl::removeContainerListener( const Reference< XContainerListener >& xListener )
	throw (RuntimeException)
{
	maNameContainer.removeContainerListener( xListener );
}

//============================================================================

