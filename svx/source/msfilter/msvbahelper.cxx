/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: 
 * $Revision: 
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include <svx/msvbahelper.hxx>
#include <basic/sbx.hxx>
#include <basic/sbstar.hxx>
#include <basic/basmgr.hxx>
#include <basic/sbmod.hxx>
#include <basic/sbmeth.hxx>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/document/XDocumentProperties.hpp>
#include <com/sun/star/document/XDocumentInfoSupplier.hpp>
#if SUPD == 310
#include <com/sun/star/lang/XUnoTunnel.hpp>
#endif	// SUPD == 310
#include <tools/urlobj.hxx>
#include <osl/file.hxx>

#if SUPD == 310
#include <com/sun/star/awt/KeyModifier.hpp>
#include <svtools/acceleratorexecute.hxx>
#include <com/sun/star/ui/XUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/ui/XUIConfigurationManager.hpp>
#include <map>
#include <vcl/keycodes.hxx>
#endif	// SUPD == 310

using namespace ::com::sun::star;

const static rtl::OUString sUrlPart0 = rtl::OUString::createFromAscii( "vnd.sun.star.script:");
const static rtl::OUString sUrlPart1 = rtl::OUString::createFromAscii( "?language=Basic&location=document"); 

namespace ooo { namespace vba {

String makeMacroURL( const String& sMacroName )
{
	return sUrlPart0.concat( sMacroName ).concat( sUrlPart1 ) ;
}

SfxObjectShell* findShellForUrl( const rtl::OUString& sMacroURLOrPath )
{
    SfxObjectShell* pFoundShell=NULL;
    SfxObjectShell* pShell = SfxObjectShell::GetFirst();
    INetURLObject aObj;
    aObj.SetURL( sMacroURLOrPath );
    bool bIsURL = aObj.GetProtocol() != INET_PROT_NOT_VALID;
    rtl::OUString aURL;
    if ( bIsURL )
        aURL = sMacroURLOrPath;
    else
    {
        osl::FileBase::getFileURLFromSystemPath( sMacroURLOrPath, aURL );
        aObj.SetURL( aURL );
    }    
    OSL_TRACE("Trying to find shell for url %s", rtl::OUStringToOString( aURL, RTL_TEXTENCODING_UTF8 ).getStr() );
    while ( pShell )
    {

        uno::Reference< frame::XModel > xModel = pShell->GetModel();
        // are we searching for a template? if so we have to cater for the
        // fact that in openoffice a document opened from a template is always
        // a new document :/
        if ( xModel.is() )
        {
            OSL_TRACE("shell 0x%x has model with url %s and we look for %s", pShell
                , rtl::OUStringToOString( xModel->getURL(), RTL_TEXTENCODING_UTF8 ).getStr() 
                , rtl::OUStringToOString( aURL, RTL_TEXTENCODING_UTF8 ).getStr() 
            );
            if ( sMacroURLOrPath.endsWithIgnoreAsciiCaseAsciiL( ".dot", 4 ) )
            {
                uno::Reference< document::XDocumentInfoSupplier > xDocInfoSupp( xModel, uno::UNO_QUERY );
                if( xDocInfoSupp.is() )
                {
                    uno::Reference< document::XDocumentPropertiesSupplier > xDocPropSupp( xDocInfoSupp->getDocumentInfo(), uno::UNO_QUERY_THROW );
                    uno::Reference< document::XDocumentProperties > xDocProps( xDocPropSupp->getDocumentProperties(), uno::UNO_QUERY_THROW );
                    rtl::OUString sCurrName = xDocProps->getTemplateName();
                    if( sMacroURLOrPath.lastIndexOf( sCurrName ) >= 0 )
                    {
                        pFoundShell = pShell; 
                        break;
                    }
                }
            }
            else
            {
                if ( aURL.equals( xModel->getURL() ) )
                {
                    pFoundShell = pShell; 
                    break;
                }
            }
        }
        pShell = SfxObjectShell::GetNext( *pShell );
    }
    return pFoundShell;
}

// sMod can be empty ( but we really need the library to search in )
// if sMod is empty and a macro is found then sMod is updated
bool hasMacro( SfxObjectShell* pShell, const String& sLibrary, String& sMod, const String& sMacro )
{
    bool bFound = false;
    if ( sLibrary.Len() && sMacro.Len() )
    {
        OSL_TRACE("** Searching for %s.%s in library %s"
            ,rtl::OUStringToOString( sMod, RTL_TEXTENCODING_UTF8 ).getStr()
            ,rtl::OUStringToOString( sMacro, RTL_TEXTENCODING_UTF8 ).getStr()
            ,rtl::OUStringToOString( sLibrary, RTL_TEXTENCODING_UTF8 ).getStr() );
        BasicManager* pBasicMgr = pShell-> GetBasicManager();
        if ( pBasicMgr )
        {
            StarBASIC* pBasic = pBasicMgr->GetLib( sLibrary );
            if ( !pBasic )
            {
                USHORT nId = pBasicMgr->GetLibId( sLibrary );
                pBasicMgr->LoadLib( nId );
                pBasic = pBasicMgr->GetLib( sLibrary );
            }
            if ( pBasic )
            {
                if ( sMod.Len() ) // we wish to find the macro is a specific module
                {
                    SbModule* pModule = pBasic->FindModule( sMod );
                    if ( pModule )
                    {
                        SbxArray* pMethods = pModule->GetMethods();
                        if ( pMethods )
                        {
                            SbMethod* pMethod = static_cast< SbMethod* >( pMethods->Find( sMacro, SbxCLASS_METHOD ) );
                            if ( pMethod )
                              bFound = true;
                        }
                    }
                }
                else if( SbMethod* pMethod = dynamic_cast< SbMethod* >( pBasic->Find( sMacro, SbxCLASS_METHOD ) ) )
                {
                    if( SbModule* pModule = pMethod->GetModule() )
                    {
                        sMod = pModule->GetName();
                        bFound = true;
                    }
                }
            }
        }
    }
    return bFound;
}
void parseMacro( const rtl::OUString& sMacro, String& sContainer, String& sModule, String& sProcedure )
{
    sal_Int32 nMacroDot = sMacro.lastIndexOf( '.' );
    
    if ( nMacroDot != -1 )
    {
        sProcedure = sMacro.copy( nMacroDot + 1 );
 
        sal_Int32 nContainerDot = sMacro.lastIndexOf( '.',  nMacroDot - 1 );
        if ( nContainerDot != -1 )
        {
            sModule = sMacro.copy( nContainerDot + 1, nMacroDot - nContainerDot - 1 );
            sContainer = sMacro.copy( 0, nContainerDot );
        }
        else
            sModule = sMacro.copy( 0, nMacroDot );
    }
    else
       sProcedure = sMacro;
}

VBAMacroResolvedInfo resolveVBAMacro( SfxObjectShell* pShell, const rtl::OUString& MacroName, bool bSearchGlobalTemplates )
{
    VBAMacroResolvedInfo aRes;
    if ( !pShell )
        return aRes;
    aRes.SetMacroDocContext( pShell );
    // parse the macro name
    sal_Int32 nDocSepIndex = MacroName.indexOfAsciiL( "!", 1 );
    String sMacroUrl = MacroName;

    String sContainer;
    String sModule;
    String sProcedure;
    
    if( nDocSepIndex > 0 )
    {
        // macro specified by document name
        // find document shell for document name and call ourselves 
        // recursively

        // assume for now that the document name is *this* document
        String sDocUrlOrPath = MacroName.copy( 0, nDocSepIndex );
        sMacroUrl = MacroName.copy( nDocSepIndex + 1 );
        OSL_TRACE("doc search, current shell is 0x%x", pShell );
        SfxObjectShell* pFoundShell = findShellForUrl( sDocUrlOrPath );
        OSL_TRACE("doc search, after find, found shell is 0x%x", pFoundShell );
        aRes = resolveVBAMacro( pFoundShell, sMacroUrl ); 
    }    
    else
    {
        // macro is contained in 'this' document ( or code imported from a template
        // where that template is a global template or perhaps the template this
        // document is created from ) 
    
        // macro format = Container.Module.Procedure
        parseMacro( MacroName, sContainer, sModule, sProcedure ); 
        uno::Reference< lang::XMultiServiceFactory> xSF( pShell->GetModel(), uno::UNO_QUERY);
        uno::Reference< container::XNameContainer > xPrjNameCache;
        if ( xSF.is() )
            xPrjNameCache.set( xSF->createInstance( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "ooo.vba.VBAProjectNameProvider" ) ) ), uno::UNO_QUERY );
    
        std::vector< rtl::OUString > sSearchList; 

        if ( sContainer.Len() > 0 )
        { 
            // get the Project associated with the Container
            if ( xPrjNameCache.is() )
            {
                if ( xPrjNameCache->hasByName( sContainer ) )
                {
                    rtl::OUString sProject;
                    xPrjNameCache->getByName( sContainer ) >>= sProject;
                    sContainer = sProject;
                }
            }
            sSearchList.push_back( sContainer ); // First Lib to search
        }
        else
        {
            // Ok, if we have no Container specified then we need to search them in order, this document, template this document created from, global templates, 
            // get the name of Project/Library for 'this' document
            rtl::OUString sThisProject;
            BasicManager* pBasicMgr = pShell-> GetBasicManager();
            if ( pBasicMgr )
            {
                if ( pBasicMgr->GetName().Len() )
                   sThisProject = pBasicMgr->GetName();
                else // cater for the case where VBA is not enabled
                   sThisProject = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Standard") );
            }
            sSearchList.push_back( sThisProject ); // First Lib to search
            if ( xPrjNameCache.is() )
            {
                // is this document created from a template?
                uno::Reference< document::XDocumentInfoSupplier > xDocInfoSupp( pShell->GetModel(), uno::UNO_QUERY_THROW );
                uno::Reference< document::XDocumentPropertiesSupplier > xDocPropSupp( xDocInfoSupp->getDocumentInfo(), uno::UNO_QUERY_THROW );
                uno::Reference< document::XDocumentProperties > xDocProps( xDocPropSupp->getDocumentProperties(), uno::UNO_QUERY_THROW );
            
                rtl::OUString sCreatedFrom = xDocProps->getTemplateURL();
                if ( sCreatedFrom.getLength() )
                {
                    INetURLObject aObj;
                    aObj.SetURL( sCreatedFrom );
                    bool bIsURL = aObj.GetProtocol() != INET_PROT_NOT_VALID;
                    rtl::OUString aURL;
                    if ( bIsURL )
                        aURL = sCreatedFrom;
                    else
                    {
                        osl::FileBase::getFileURLFromSystemPath( sCreatedFrom, aURL );
                        aObj.SetURL( aURL );
                    }        
                    sCreatedFrom =  aObj.GetLastName();
                } 
                
                sal_Int32 nIndex =  sCreatedFrom.lastIndexOf( '.' );
                if ( nIndex != -1 )
                    sCreatedFrom = sCreatedFrom.copy( 0, nIndex );
        
                rtl::OUString sPrj;
                if ( sCreatedFrom.getLength() && xPrjNameCache->hasByName( sCreatedFrom ) )
                {
                    xPrjNameCache->getByName( sCreatedFrom ) >>= sPrj;
                    // Make sure we don't double up with this project
                    if ( !sPrj.equals( sThisProject ) )
                        sSearchList.push_back( sPrj );
                }
        
                // get list of global template Names
                uno::Sequence< rtl::OUString > sTemplateNames = xPrjNameCache->getElementNames();
                sal_Int32 nLen = sTemplateNames.getLength();
                for ( sal_Int32 index = 0; ( bSearchGlobalTemplates && index < nLen ); ++index )
                {
                    
                    if ( !sCreatedFrom.equals( sTemplateNames[ index ] ) )
                    {
                        if ( xPrjNameCache->hasByName( sTemplateNames[ index ] ) )
                        {
                            xPrjNameCache->getByName( sTemplateNames[ index ] ) >>= sPrj;
                            // Make sure we don't double up with this project
                            if ( !sPrj.equals( sThisProject ) )
                                sSearchList.push_back( sPrj );
                        }
                    }
        
                }
            }
        }
        std::vector< rtl::OUString >::iterator it_end = sSearchList.end();
        for ( std::vector< rtl::OUString >::iterator it = sSearchList.begin(); it != it_end; ++it )
        {
            bool bRes = hasMacro( pShell, *it, sModule, sProcedure );
            if ( bRes )
            {
                aRes.SetResolved( true );
                aRes.SetMacroDocContext( pShell );
                sContainer = *it;
                break;
            }
        }
    }
    aRes.SetResolvedMacro( sProcedure.Insert( '.', 0 ).Insert( sModule, 0).Insert( '.', 0 ).Insert( sContainer, 0 ) );

    return aRes;
}

// Treat the args as possible inouts ( convertion at bottom of method )
sal_Bool executeMacro( SfxObjectShell* pShell, const String& sMacroName, uno::Sequence< uno::Any >& aArgs, uno::Any& /*aRet*/, const uno::Any& aCaller )
{
    sal_Bool bRes = sal_False;
    if ( !pShell )
        return bRes;
    rtl::OUString sUrl = makeMacroURL( sMacroName );

    uno::Sequence< sal_Int16 > aOutArgsIndex;
    uno::Sequence< uno::Any > aOutArgs;

    try
    {    
        uno::Reference< script::provider::XScriptProvider > xScriptProvider;
        uno::Reference< script::provider::XScriptProviderSupplier > xSPS( pShell->GetModel(), uno::UNO_QUERY_THROW );

        xScriptProvider.set( xSPS->getScriptProvider(), uno::UNO_QUERY_THROW );

        uno::Reference< script::provider::XScript > xScript( xScriptProvider->getScript( sUrl ), uno::UNO_QUERY_THROW );

        if ( aCaller.hasValue() )
        {
            uno::Reference< beans::XPropertySet > xProps( xScript, uno::UNO_QUERY );
            if ( xProps.is() )
            {
                uno::Sequence< uno::Any > aCallerHack(1);
                aCallerHack[ 0 ] = aCaller;
                xProps->setPropertyValue( rtl::OUString::createFromAscii( "Caller" ), uno::makeAny( aCallerHack ) );
            }
        }


        xScript->invoke( aArgs, aOutArgsIndex, aOutArgs  );
        
        sal_Int32 nLen = aOutArgs.getLength();
        // convert any out params to seem like they were inouts
        if ( nLen )
        {
            for ( sal_Int32 index=0; index < nLen; ++index )
            {
                sal_Int32 nOutIndex = aOutArgsIndex[ index ];
                aArgs[ nOutIndex ] = aOutArgs[ index ];
           }
        }

        bRes = sal_True;
    }
    catch ( uno::Exception& e )
    {
       bRes = sal_False;
    }
    return bRes; 
}

#if SUPD == 310

bool getModifier( char c, sal_uInt16& mod )
{
    static const char modifiers[] = "+^%";
    static const sal_uInt16 KEY_MODS[] = {KEY_SHIFT, KEY_MOD1, KEY_MOD2};

    for ( unsigned int i=0; i<SAL_N_ELEMENTS(KEY_MODS); ++i )
    {
        if ( c == modifiers[i] )
        {
            mod = mod | KEY_MODS[ i ];
            return true;
        }
    }
    return false;
}

typedef std::map< OUString, sal_uInt16 > MSKeyCodeMap;

sal_uInt16 parseChar( char c ) throw ( uno::RuntimeException )
{
    sal_uInt16 nVclKey = 0;
    // do we care about locale here for isupper etc. ? probably not
    if ( isalpha( c ) )
    {
        nVclKey |= ( toupper( c ) - 'A' ) + KEY_A;
        if ( isupper( c ) )
            nVclKey |= KEY_SHIFT;
    }
    else if ( isdigit( c ) )
        nVclKey |= ( c  - '0' ) + KEY_0;
    else if ( c == '~' ) // special case
        nVclKey = KEY_RETURN;
    else if ( c == ' ' ) // special case
        nVclKey = KEY_SPACE;
    else // I guess we have a problem ( but not sure if locale specific keys might come into play here )
        throw uno::RuntimeException();
    return nVclKey;
}

struct KeyCodeEntry
{
   const char* sName;
   sal_uInt16 nCode;
};

KeyCodeEntry aMSKeyCodesData[] = {
    { "BACKSPACE", KEY_BACKSPACE },
    { "BS", KEY_BACKSPACE },
    { "DELETE", KEY_DELETE },
    { "DEL", KEY_DELETE },
    { "DOWN", KEY_DOWN },
    { "UP", KEY_UP },
    { "LEFT", KEY_LEFT },
    { "RIGHT", KEY_RIGHT },
    { "END", KEY_END },
    { "ESCAPE", KEY_ESCAPE },
    { "ESC", KEY_ESCAPE },
    { "HELP", KEY_HELP },
    { "HOME", KEY_HOME },
    { "PGDN", KEY_PAGEDOWN },
    { "PGUP", KEY_PAGEUP },
    { "INSERT", KEY_INSERT },
#if SUPD != 310
    { "SCROLLLOCK", KEY_SCROLLLOCK },
    { "NUMLOCK", KEY_NUMLOCK },
#endif	// SUPD != 310
    { "TAB", KEY_TAB },
    { "F1", KEY_F1 },
    { "F2", KEY_F2 },
    { "F3", KEY_F3 },
    { "F4", KEY_F4 },
    { "F5", KEY_F5 },
    { "F6", KEY_F6 },
    { "F7", KEY_F7 },
    { "F8", KEY_F8 },
    { "F9", KEY_F1 },
    { "F10", KEY_F10 },
    { "F11", KEY_F11 },
    { "F12", KEY_F12 },
    { "F13", KEY_F13 },
    { "F14", KEY_F14 },
    { "F15", KEY_F15 },
};

awt::KeyEvent parseKeyEvent( const OUString& Key ) throw ( uno::RuntimeException )
{
    static MSKeyCodeMap msKeyCodes;
    if ( msKeyCodes.empty() )
    {
        for ( unsigned int i = 0; i < SAL_N_ELEMENTS( aMSKeyCodesData ); ++i )
        {
            msKeyCodes[ OUString::createFromAscii( aMSKeyCodesData[ i ].sName ) ] = aMSKeyCodesData[ i ].nCode;
        }
    }
    OUString sKeyCode;
    sal_uInt16 nVclKey = 0;

    // parse the modifier if any
    for ( int i=0; i<Key.getLength(); ++i )
    {
        if ( ! getModifier( Key[ i ], nVclKey ) )
        {
            sKeyCode = Key.copy( i );
            break;
        }
    }

    // check if keycode is surrounded by '{}', if so scoop out the contents
    // else it should be just one char of ( 'a-z,A-Z,0-9' )
    if ( sKeyCode.getLength() == 1 ) // ( a single char )
    {
        char c = (char)( sKeyCode[ 0 ] );
        nVclKey |= parseChar( c );
    }
    else // key should be enclosed in '{}'
    {
        if ( sKeyCode.getLength() < 3 ||  !( sKeyCode[0] == '{' && sKeyCode[sKeyCode.getLength() - 1 ] == '}' ) )
            throw uno::RuntimeException();

        sKeyCode = sKeyCode.copy(1, sKeyCode.getLength() - 2 );

        if ( sKeyCode.getLength() == 1 )
            nVclKey |= parseChar( (char)( sKeyCode[ 0 ] ) );
        else
        {
            MSKeyCodeMap::iterator it = msKeyCodes.find( sKeyCode );
            if ( it == msKeyCodes.end() ) // unknown or unsupported
                throw uno::RuntimeException();
            nVclKey |= it->second;
        }
    }

    awt::KeyEvent aKeyEvent = svt::AcceleratorExecute::st_VCLKey2AWTKey( KeyCode( nVclKey ) );
    return aKeyEvent;
}

void applyShortCutKeyBinding ( const uno::Reference< frame::XModel >& rxModel, const awt::KeyEvent& rKeyEvent, const OUString& rMacroName ) throw (uno::RuntimeException)
{
    OUString MacroName( rMacroName );
    if ( !MacroName.isEmpty() )
    {
        OUString aMacroName = MacroName.trim();
        if( aMacroName.startsWith("!") )
            MacroName = aMacroName.copy(1).trim();
        SfxObjectShell* pShell = NULL;
        if ( rxModel.is() )
        {
            uno::Reference< lang::XUnoTunnel >  xObjShellTunnel( rxModel, uno::UNO_QUERY_THROW );
            pShell = reinterpret_cast<SfxObjectShell*>( xObjShellTunnel->getSomething(SfxObjectShell::getUnoTunnelId()));
            if ( !pShell )
                throw uno::RuntimeException();
        }
#if SUPD == 310
        VBAMacroResolvedInfo aMacroInfo = resolveVBAMacro( pShell, aMacroName );
        if( !aMacroInfo.IsResolved() )
#else	// SUPD == 310
        MacroResolvedInfo aMacroInfo = resolveVBAMacro( pShell, aMacroName );
        if( !aMacroInfo.mbFound )
#endif	// SUPD == 310
            throw uno::RuntimeException( "The procedure doesn't exist", uno::Reference< uno::XInterface >() );
#if SUPD == 310
       MacroName = aMacroInfo.ResolvedMacro();
#else	// SUPD == 310
       MacroName = aMacroInfo.msResolvedMacro;
#endif	// SUPD == 310
    }
    uno::Reference< ui::XUIConfigurationManagerSupplier > xCfgSupplier(rxModel, uno::UNO_QUERY_THROW);
    uno::Reference< ui::XUIConfigurationManager > xCfgMgr = xCfgSupplier->getUIConfigurationManager();

    uno::Reference< ui::XAcceleratorConfiguration > xAcc( xCfgMgr->getShortCutManager(), uno::UNO_QUERY_THROW );
    if ( MacroName.isEmpty() )
        // I believe this should really restore the [application] default. Since
        // afaik we don't actually setup application default bindings on import
        // we don't even know what the 'default' would be for this key
        xAcc->removeKeyEvent( rKeyEvent );
    else
        xAcc->setKeyEvent( rKeyEvent, ooo::vba::makeMacroURL( MacroName ) );

}

#endif	// SUPD == 310

} } // vba // ooo
