#include "ww8toolbar.hxx"
#include <rtl/ustrbuf.hxx>
#include <stdarg.h>
#include <com/sun/star/ui/XUIConfigurationPersistence.hpp>
#include <com/sun/star/ui/XUIConfigurationPersistence.hpp>
#include <com/sun/star/ui/XModuleUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XSingleComponentFactory.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/ui/XImageManager.hpp>
#include <com/sun/star/ui/ItemType.hpp>
#include <fstream>
#include <comphelper/processfactory.hxx>
#include <vcl/graph.hxx>
#include <map>
using namespace com::sun::star;

typedef std::map< sal_Int16, rtl::OUString > IdToString;

class MSOWordCommandConvertor : public MSOCommandConvertor
{
   IdToString msoToOOcmd;
   IdToString tcidToOOcmd;
public:
    MSOWordCommandConvertor();
    virtual rtl::OUString MSOCommandToOOCommand( sal_Int16 msoCmd );
    virtual rtl::OUString MSOTCIDToOOCommand( sal_Int16 key );
};

MSOWordCommandConvertor::MSOWordCommandConvertor()
{
    // mso command id to ooo command string
    // #FIXME and *HUNDREDS* of id's to added here
    msoToOOcmd[ 0x20b ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(".uno:CloseDoc") );
    msoToOOcmd[ 0x50 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(".uno:Open") );

   // mso tcid to ooo command string
    // #FIXME and *HUNDREDS* of id's to added here
   tcidToOOcmd[ 0x9d9 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(".uno:Print") );
}

rtl::OUString MSOWordCommandConvertor::MSOCommandToOOCommand( sal_Int16 key )
{
    rtl::OUString sResult;
    IdToString::iterator it = msoToOOcmd.find( key );
    if ( it != msoToOOcmd.end() )
        sResult = it->second;
    return sResult;
}

rtl::OUString MSOWordCommandConvertor::MSOTCIDToOOCommand( sal_Int16 key )
{
    rtl::OUString sResult;
    IdToString::iterator it = tcidToOOcmd.find( key );
    if ( it != tcidToOOcmd.end() )
        sResult = it->second;
    return sResult;
}


CTBWrapper::CTBWrapper( bool bReadId ) : Tcg255SubStruct( bReadId )
,reserved2(0)
,reserved3(0)
,reserved4(0)
,reserved5(0)
,cbTBD(0)
,cCust(0)
,cbDTBC(0)
,rtbdc(0)
{
}

CTBWrapper::~CTBWrapper()
{
}

Customization* CTBWrapper::GetCustomizaton( sal_Int16 index )
{
    if ( index < 0 || index >= rCustomizations.size() )
        return NULL;
    return &rCustomizations[ index ]; 
}

bool CTBWrapper::Read( SvStream* pS )
{
    OSL_TRACE("CTBWrapper::Read() stream pos 0x%x", pS->Tell() );
    nOffSet = pS->Tell();
    Tcg255SubStruct::Read( pS );
    *pS >> reserved2 >> reserved3 >> reserved4 >> reserved5;
    *pS >> cbTBD >> cCust >> cbDTBC;
    if ( cbDTBC )
    {
        // cbDTBC is the size in bytes of the TBC array
        // but the size of a TBC element is dynamic ( and this relates to TBDelta's
        int nStart = pS->Tell();

        int bytesRead = 0;
        int bytesToRead = cbDTBC - bytesRead; 
        // cbDTBC specifies the size ( in bytes ) taken by an array ( of unspecified size )
        // of TBC records ( TBC records have dynamic length, so we need to check our position
        // after each read )
        do
        {
            TBC aTBC;
            if ( !aTBC.Read( pS ) )
                return false;
            rtbdc.push_back( aTBC );
            bytesToRead = cbDTBC - ( pS->Tell() - nStart ); 
        } while ( bytesToRead > 0 );
    }
    if ( cCust )
    {
        for ( sal_Int32 index = 0; index < cCust; ++index )
        {
            Customization aCust( this );
            if ( !aCust.Read( pS ) )
                return false;
            rCustomizations.push_back( aCust );
        } 
    } 
    std::vector< sal_Int16 >::iterator it_end = dropDownMenuIndices.end();
    for ( std::vector< sal_Int16 >::iterator it = dropDownMenuIndices.begin(); it != it_end; ++it )
    {
        rCustomizations[ *it ].bIsDroppedMenuTB = true;
    }
    return true;
}

TBC* CTBWrapper::GetTBCAtOffset( sal_uInt32 nStreamOffset )
{
    for ( std::vector< TBC >::iterator it = rtbdc.begin(); it != rtbdc.end(); ++it )
    {
        if ( (*it).GetOffset() == nStreamOffset )
            return &(*it);
    }
    return NULL;
}

void CTBWrapper::Print( FILE* fp )
{
    Indent a;
    indent_printf(fp,"[ 0x%x ] CTBWrapper - dump\n", nOffSet );
    bool bRes = ( ch == 0x12 && reserved2 == 0x0 && reserved3 == 0x7 && reserved4 == 0x6 && reserved5 == 0xC );
    if ( bRes )
        indent_printf(fp,"  sanity check ( first 8 bytes conform )\n");
    else 
    {
        indent_printf(fp,"    reserved1(0x%x)\n",ch);
        indent_printf(fp,"    reserved2(0x%x)\n",reserved2);
        indent_printf(fp,"    reserved3(0x%x)\n",reserved3);
        indent_printf(fp,"    reserved4(0x%x)\n",reserved4);
        indent_printf(fp,"    reserved5(0x%x)\n",reserved5);
        indent_printf(fp,"Quiting dump");
        return;
    }
    indent_printf(fp,"  size of TBDelta structures 0x%x\n", cbTBD );
    indent_printf(fp,"  cCust: no. of cCust structures 0x%x\n",cCust);
    indent_printf(fp,"  cbDTBC: no. of bytes in rtbdc array 0x%x\n", static_cast< unsigned int >( cbDTBC ));

    sal_Int32 index = 0;

    for ( std::vector< TBC >::iterator it = rtbdc.begin(); it != rtbdc.end(); ++it, ++index )
    {
        indent_printf(fp,"  Dumping rtbdc[%d]\n", static_cast< int >( index ));
        Indent b;
        it->Print( fp );
    }

    index = 0;

    for ( std::vector< Customization >::iterator it = rCustomizations.begin(); it != rCustomizations.end(); ++it, ++index )
    {
        indent_printf(fp,"  Dumping custimization [%d]\n", static_cast< int >( index ));
        Indent c;
        it->Print(fp);
    }
}

bool CTBWrapper::ImportCustomToolBar( SfxObjectShell& rDocSh )
{
    
    for ( std::vector< Customization >::iterator it = rCustomizations.begin(); it != rCustomizations.end(); ++it )
    {
        uno::Reference< lang::XMultiServiceFactory > xMSF( ::comphelper::getProcessServiceFactory(), uno::UNO_QUERY_THROW );
        uno::Reference< ui::XModuleUIConfigurationManagerSupplier > xAppCfgSupp( xMSF->createInstance( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.ui.ModuleUIConfigurationManagerSupplier" ) ) ), uno::UNO_QUERY_THROW ); 
        CustomToolBarImportHelper helper( rDocSh, xAppCfgSupp->getUIConfigurationManager( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.text.TextDocument" ) ) ) );
        helper.setMSOCommandMap( new  MSOWordCommandConvertor() );
        if ( !(*it).ImportCustomToolBar( helper ) )
            return false;
    }
    return false;
}

Customization::Customization( CTBWrapper* wrapper ) : tbidForTBD( 0 )
,reserved1( 0 )
, ctbds( 0 )
, pWrapper( wrapper )
, bIsDroppedMenuTB( false )
{
}

Customization::~Customization()
{
}

bool Customization::Read( SvStream *pS)
{
    OSL_TRACE("Custimization::Read() stream pos 0x%x", pS->Tell() );
    nOffSet = pS->Tell();
    *pS >> tbidForTBD >> reserved1 >> ctbds;
    if ( tbidForTBD )
    {
        for ( sal_Int32 index = 0; index < ctbds; ++index )
        {
            TBDelta aTBDelta;
            if (!aTBDelta.Read( pS ) )
                return false;
            customizationDataTBDelta.push_back( aTBDelta );
            // Only set the drop down for menu's associated with standard toolbar
            if ( aTBDelta.ControlDropsToolBar() && tbidForTBD == 0x25 )
                pWrapper->InsertDropIndex( aTBDelta.CustomizationIndex() );
        }
    }
    else
    {
        customizationDataCTB.reset( new CTB() );
        if ( !customizationDataCTB->Read( pS ) )
                return false;
    }
    return true;
}

void Customization::Print( FILE* fp )
{
    Indent a;
    indent_printf( fp,"[ 0x%x ] Customization -- dump \n", nOffSet );
    indent_printf( fp,"  tbidForTBD 0x%x ( should be 0 for CTBs )\n", static_cast< unsigned int >( tbidForTBD ));
    indent_printf( fp,"  reserved1 0x%x \n", reserved1);
    indent_printf( fp,"  ctbds - number of customisations %d(0x%x) \n", ctbds, ctbds );
    if ( !tbidForTBD && !ctbds )
        customizationDataCTB->Print( fp );
    else
    {
        const char* pToolBar = NULL;
        switch ( tbidForTBD )
        {
            case 0x9:
                pToolBar = "Standard";
                break;
            case 0x25:
                pToolBar = "Builtin-Menu";
                break;
            default:
                pToolBar = "Unknown toolbar";
                break;
        }
        
        indent_printf( fp,"  TBDelta(s) are associated with %s toolbar.\n", pToolBar);
        std::vector< TBDelta >::iterator it = customizationDataTBDelta.begin();
        for ( sal_Int32 index = 0; index < ctbds; ++it,++index )
            it->Print( fp );
    }
    
}

bool Customization::ImportMenu( const uno::Reference< container::XIndexContainer >& xIndexContainer, CustomToolBarImportHelper& helper )
{
    if ( !customizationDataCTB.get() )
        return false;
    return customizationDataCTB->ImportMenu( xIndexContainer, helper );
}

bool Customization::ImportCustomToolBar( CustomToolBarImportHelper& helper )
{
    if ( bIsDroppedMenuTB )
        return true; // ignore ( will be processed by the ImportMenu )
    if ( tbidForTBD == 0x25 )  // we can handle in a limited way additions the built-in menu bar
    {
        for ( std::vector< TBDelta >::iterator it = customizationDataTBDelta.begin(); it != customizationDataTBDelta.end(); ++it )
        {
            // for each new menu ( control that drops a toolbar )
            // import a toolbar
            if ( it->ControlIsInserted() && it->ControlDropsToolBar() )
            {
                Customization* pCust = pWrapper->GetCustomizaton( it->CustomizationIndex() );
                if ( pCust )
                {
                    // currently only support built-in menu
                    rtl::OUString sMenuBar( RTL_CONSTASCII_USTRINGPARAM("private:resource/menubar/") );
                    if ( tbidForTBD != 0x25 )
                        return false;

                    sMenuBar = sMenuBar.concat( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("menubar") ) );
                    // Get menu name
                    TBC* pTBC = pWrapper->GetTBCAtOffset( it->TBCStreamOffset() );
                    if ( !pTBC )
                        return false;
                    rtl::OUString sMenuName = pTBC->GetCustomText();
                    sMenuName = sMenuName.replace('&','~');

                    // see if the document has already setting for the menubar
                
                    uno::Reference< container::XIndexContainer > xIndexContainer;
                    bool bHasSettings = false;
                    if ( helper.getCfgManager()->hasSettings( sMenuBar ) )
                    {
                        xIndexContainer.set( helper.getCfgManager()->getSettings( sMenuBar, sal_True ), uno::UNO_QUERY_THROW );
                        bHasSettings = true;
                    }
                    else
                    {
                        if ( helper.getAppCfgManager()->hasSettings( sMenuBar ) )
                            xIndexContainer.set( helper.getAppCfgManager()->getSettings( sMenuBar, sal_True ), uno::UNO_QUERY_THROW );
                        else 
                            xIndexContainer.set( helper.getAppCfgManager()->createSettings(), uno::UNO_QUERY_THROW );
                    }
               
                    uno::Reference< lang::XSingleComponentFactory > xSCF( xIndexContainer, uno::UNO_QUERY_THROW );
                    uno::Reference< beans::XPropertySet > xProps( ::comphelper::getProcessServiceFactory(), uno::UNO_QUERY_THROW );
                    uno::Reference< uno::XComponentContext > xContext(  xProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DefaultContext" ))), uno::UNO_QUERY_THROW );
                    // create the popup menu
                    uno::Sequence< beans::PropertyValue > aPopupMenu( 4 );
                    aPopupMenu[0].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("CommandURL") );
                    aPopupMenu[0].Value = uno::makeAny( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("vnd.openoffice.org:") ) + sMenuName );
                    aPopupMenu[1].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Label") );
                    aPopupMenu[1].Value <<= sMenuName;
                    aPopupMenu[2].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Type" ) );
                    aPopupMenu[2].Value <<= sal_Int32( 0 );
                    aPopupMenu[3].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ItemDescriptorContainer") );
                    uno::Reference< container::XIndexContainer > xMenuContainer( xSCF->createInstanceWithContext( xContext ), uno::UNO_QUERY_THROW );
                    aPopupMenu[3].Value <<= xMenuContainer;
                    if ( !pCust->ImportMenu( xMenuContainer, helper ) )
                        return false;
                    OSL_TRACE("** there are %d menu items on the bar, inserting after that", xIndexContainer->getCount() );
                    xIndexContainer->insertByIndex( xIndexContainer->getCount(), uno::makeAny( aPopupMenu ) );

                    if ( bHasSettings )
                        helper.getCfgManager()->replaceSettings( sMenuBar, uno::Reference< container::XIndexAccess >( xIndexContainer, uno::UNO_QUERY_THROW ) );
                    else
                        helper.getCfgManager()->insertSettings( sMenuBar, uno::Reference< container::XIndexAccess >( xIndexContainer, uno::UNO_QUERY_THROW ) );
                }
            }
        }
        return true;
    }
    if ( !customizationDataCTB.get() )
        return false;
    return customizationDataCTB->ImportCustomToolBar( helper );
}

TBDelta::TBDelta() : doprfatendFlags(0)
,ibts(0)
,cidNext(0)
,cid(0)
,fc(0)
,cbTBC(0)
{
}

bool TBDelta::ControlIsModified()
{
    return ( ( doprfatendFlags & 0x3 ) == 0x2 );
}

bool TBDelta::ControlIsInserted()
{
    return ( ( doprfatendFlags & 0x3 ) == 0x1 );
}

bool TBDelta::ControlIsChanged()
{
    return ( ( doprfatendFlags & 0x3 ) == 0x1 );
}

bool TBDelta::ControlDropsToolBar()
{
    return !( CiTBDE & 0x8000 );
}

sal_Int32 TBDelta::TBCStreamOffset()
{
    return fc;
}

sal_Int16 TBDelta::CustomizationIndex()
{
    sal_Int16 nIndex = CiTBDE;
    nIndex = nIndex >> 1;
    nIndex &= 0x1ff; // only 13 bits are relevant
    return nIndex;
}

bool TBDelta::Read(SvStream *pS)
{
    OSL_TRACE("TBDelta::Read() stream pos 0x%x", pS->Tell() );
    nOffSet = pS->Tell();
    *pS >> doprfatendFlags >> ibts >> cidNext >> cid >> fc ;
    *pS >> CiTBDE >> cbTBC;
    return true;
}

void TBDelta::Print( FILE* fp )
{
    // Like most of the debug output, it's raw and little ( no )
    // interpretation of the data is output ( e.g. flag values etc. )
    indent_printf( fp, "[ 0x%x ] TBDelta -- dump\n", nOffSet );
    indent_printf( fp, " doprfatendFlags 0x%x\n",doprfatendFlags );
    
    indent_printf( fp, " ibts 0x%x\n",ibts );
    indent_printf( fp, " cidNext 0x%x\n", static_cast< unsigned int >( cidNext ) );
    indent_printf( fp, " cid 0x%x\n", static_cast< unsigned int >( cid ) );
    indent_printf( fp, " fc 0x%x\n", static_cast< unsigned int >( fc ) );
    indent_printf( fp, " CiTBDE 0x%x\n",CiTBDE );
    indent_printf( fp, " cbTBC 0x%x\n", cbTBC );
    if ( ControlDropsToolBar() )
    {
        indent_printf( fp, " this delta is associated with a control that drops a menu toolbar\n", cbTBC );
        indent_printf( fp, " the menu toolbar drops the toolbar defined at index[%d] in the rCustomizations array of the CTBWRAPPER that contains this TBDelta\n", CustomizationIndex() );
    }
}

CTB::CTB() : cbTBData( 0 )
,iWCTBl( 0 )
,reserved( 0 )
,unused( 0 )
,cCtls( 0 )
{
}

CTB::~CTB()
{
}

bool CTB::Read( SvStream *pS)
{
    OSL_TRACE("CTB::Read() stream pos 0x%x", pS->Tell() );
    nOffSet = pS->Tell();
    if ( !name.Read( pS ) )
        return false;
    *pS >> cbTBData;
    // sal_Int32 nTBSize = cbTBData - sizeof(rVisualData) - 12;
    if ( !tb.Read( pS ) )
        return false;
    pS->Read( &rVisualData, sizeof( rVisualData ) );

    *pS >> iWCTBl >> reserved >> unused >> cCtls;
    
    if ( cCtls )
    {
        for ( sal_Int32 index = 0; index < cCtls; ++index )
        {
            TBC aTBC;
            if ( !aTBC.Read( pS ) )
                return false;
            rTBC.push_back( aTBC );
        }
    }
    return true;
}

void
CTB::Print( FILE* fp )
{
    Indent a;
    indent_printf(fp, "[ 0x%x ] CTB - dump\n", nOffSet );
    indent_printf(fp, "  name %s\n", rtl::OUStringToOString( name.getString(), RTL_TEXTENCODING_UTF8 ).getStr() );
    indent_printf(fp, "  cbTBData size, in bytes, of this structure excluding the name, cCtls, and rTBC fields.  %x\n", static_cast< unsigned int >( cbTBData ) );
    
    tb.Print(fp);
    indent_printf(fp, "  iWCTBl 0x%x reserved 0x%x unused 0x%x cCtls( toolbar controls ) 0x%x \n", static_cast< unsigned int >( iWCTBl ), reserved, unused, static_cast< unsigned int >( cCtls ) );
    if ( cCtls )
    {
        for ( sal_Int32 index = 0; index < cCtls; ++index )
        {
        
            indent_printf(fp, "  dumping toolbar control 0x%x\n", static_cast< unsigned int >( index ) );
            rTBC[ index ].Print( fp );
        }
    }
}

bool CTB::ImportCustomToolBar( CustomToolBarImportHelper& helper )
{
    static rtl::OUString sToolbarPrefix( RTL_CONSTASCII_USTRINGPARAM( "private:resource/toolbar/custom_" ) );
    bool bRes = false;
    try
    {
        // #FIXME this is a bogus check ( but is seems to work )
        // the fNeedsPositioning seems always to be set for toolbars that
        // have been deleted ( sofar I fail to find something else to indicate
        // not to read them ) - e.g. We can have CTB records which relate to
        // deleted items ( usually associated with popups )
        if ( !tb.IsEnabled() || tb.NeedsPositioning() )
            return true;  // didn't fail, just ignoring
        // Create default setting
        uno::Reference< container::XIndexContainer > xIndexContainer( helper.getCfgManager()->createSettings(), uno::UNO_QUERY_THROW );
        uno::Reference< container::XIndexAccess > xIndexAccess( xIndexContainer, uno::UNO_QUERY_THROW );
        uno::Reference< beans::XPropertySet > xProps( xIndexContainer, uno::UNO_QUERY_THROW ); 
        
        // set UI name for toolbar
        xProps->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("UIName") ), uno::makeAny( name.getString() ) ); 
    
        rtl::OUString sToolBarName = sToolbarPrefix.concat( name.getString() );
        for ( std::vector< TBC >::iterator it =  rTBC.begin(); it != rTBC.end(); ++it )
        {
            // createToolBar item for control
            if ( !it->ImportToolBarControl( xIndexContainer, helper ) )
                return false;
        }
    
        OSL_TRACE("Name of toolbar :-/ %s", rtl::OUStringToOString( sToolBarName, RTL_TEXTENCODING_UTF8 ).getStr() );
    
        helper.getCfgManager()->insertSettings( sToolBarName, xIndexAccess );
        helper.applyIcons();
    
        uno::Reference< ui::XUIConfigurationPersistence > xPersistence( helper.getCfgManager()->getImageManager(), uno::UNO_QUERY_THROW );
        xPersistence->store();
    
        xPersistence.set( helper.getCfgManager(), uno::UNO_QUERY_THROW );
        xPersistence->store();
        bRes = true;
    }
    catch( uno::Exception& )
    {
        bRes = false;
    }
    return bRes;
}

bool CTB::ImportMenu( const css::uno::Reference< css::container::XIndexContainer >& xIndexContainer, CustomToolBarImportHelper& rHelper )
{
    for ( std::vector< TBC >::iterator it =  rTBC.begin(); it != rTBC.end(); ++it )
    {
        // createToolBar item for control
        if ( !it->ImportToolBarControl( xIndexContainer, rHelper ) )
            return false;
    }
    return true;
}

TBC::TBC()
{
}

TBC::~TBC()
{
}

bool TBC::Read( SvStream *pS )
{
    OSL_TRACE("TBC::Read() stream pos 0x%x", pS->Tell() );
    nOffSet = pS->Tell();
    if ( !tbch.Read( pS ) )
        return false;
    if ( tbch.getTcID() != 0x1 && tbch.getTcID() != 0x1051 )
    {
        cid.reset( new sal_uInt32 );
        *pS >> *cid;
    }
    // MUST exist if tbch.tct is not equal to 0x16
    if ( tbch.getTct() != 0x16 )
    {
        tbcd.reset(  new TBCData( tbch ) );
        if ( !tbcd->Read( pS ) )
            return false;
    }
    return true;
}

void TBC::Print( FILE* fp )
{
    Indent a;
    indent_printf(fp,"[ 0x%x ] TBC -- dump\n", nOffSet );
    indent_printf(fp,"  dumping header ( TBCHeader )\n");
    tbch.Print( fp );
    if ( cid.get() )
        indent_printf(fp,"  cid = 0x%x\n", static_cast< unsigned int >( *cid ) );
    if ( tbcd.get() )
    {
        indent_printf(fp,"  dumping toolbar data TBCData \n");
        tbcd->Print(fp);
    }
}

bool
TBC::ImportToolBarControl( const css::uno::Reference< css::container::XIndexContainer >& toolbarcontainer, CustomToolBarImportHelper& helper )
{
    // cmtFci       0x1 Command based on a built-in command. See CidFci.
    // cmtMacro     0x2 Macro command. See CidMacro.
    // cmtAllocated 0x3 Allocated command. See CidAllocated.
    // cmtNil       0x7 No command. See Cid.
    bool bBuiltin = false;
    sal_uInt16 cmdId = 0;
    if  ( cid.get() )
    {
        sal_uInt16 arg2 = ( *( cid.get() ) & 0xFFFF );

        sal_uInt8 cmt = ( arg2 & 0x7 );
        arg2 = ( arg2 >> 3 );

        switch ( cmt )
        {
            case 1:
                OSL_TRACE("cmt is cmtFci builtin command 0x%x", arg2);
                bBuiltin = true;
                cmdId = arg2;
                break;
            case 2:
                OSL_TRACE("cmt is cmtMacro macro 0x%x", arg2);
                break;
            case 3:
                OSL_TRACE("cmt is cmtAllocated [???] 0x%x", arg2);
                break;
            case 7:
                OSL_TRACE("cmt is cmNill no-phing 0x%x", arg2);
                break;
            default:
                OSL_TRACE("illegal 0x%x", cmt);
                break;
        }
    }

    if ( tbcd.get() )
    {
        std::vector< css::beans::PropertyValue > props;
        if ( bBuiltin )
        {
            rtl::OUString sCommand = helper.MSOCommandToOOCommand( cmdId );
            if ( sCommand.getLength() > 0 )
            {
                beans::PropertyValue aProp;
 
                aProp.Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("CommandURL") );
                aProp.Value <<= sCommand;
                props.push_back( aProp ); 
            }
         
        }
        bool bBeginGroup = false;
        if ( ! tbcd->ImportToolBarControl( toolbarcontainer, helper, props, bBeginGroup ) )
            return false;

        if ( bBeginGroup )
        {
            // insert spacer
            uno::Sequence< beans::PropertyValue > sProps( 1 );
            sProps[ 0 ].Name =  rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Type") );
            sProps[ 0 ].Value = uno::makeAny( ui::ItemType::SEPARATOR_LINE ); 
            toolbarcontainer->insertByIndex( toolbarcontainer->getCount(), uno::makeAny( sProps ) );       
        }
 
        uno::Sequence< beans::PropertyValue > sProps( props.size() );
        beans::PropertyValue* pProp = sProps.getArray();
   
        for ( std::vector< css::beans::PropertyValue >::iterator it = props.begin(); it != props.end(); ++it, ++pProp )
            *pProp = *it;

        toolbarcontainer->insertByIndex( toolbarcontainer->getCount(), uno::makeAny( sProps ) );        
    }
    return true;
}

rtl::OUString 
TBC::GetCustomText()
{
    rtl::OUString sCustomText;
    if ( tbcd.get() )
        sCustomText = tbcd->getGeneralInfo().CustomText();
    return sCustomText;
   
    
}

bool 
Xst::Read( SvStream* pS )
{
    OSL_TRACE("Xst::Read() stream pos 0x%x", pS->Tell() );
    nOffSet = pS->Tell();
    sal_Int16 nChars = 0;
    *pS >> nChars;
    sString = readUnicodeString( pS, static_cast< sal_Int32 >( nChars  ) );
    return true;
}

void
Xst::Print( FILE* fp )
{
    Indent a;
    indent_printf( fp, "[ 0x%x ] Xst -- dump\n", nOffSet );
    indent_printf( fp, " %s",  rtl::OUStringToOString( sString, RTL_TEXTENCODING_UTF8 ).getStr() );
}

Tcg::Tcg() : nTcgVer( 255 )
{
}

bool Tcg::Read(SvStream *pS)
{
    OSL_TRACE("Tcg::Read() stream pos 0x%x", pS->Tell() );
    nOffSet = pS->Tell();
    *pS >> nTcgVer;
    if ( nTcgVer != (sal_Int8)255 )
        return false;
    tcg.reset( new Tcg255() );
    return tcg->Read( pS );
}

void Tcg::Print( FILE* fp )
{
    Indent a(true);
    indent_printf(fp, "[ 0x%x ] Tcg - dump %d\n", nOffSet, nTcgVer);
    indent_printf(fp,"  nTcgVer %d\n", nTcgVer);
#ifdef USE_JAVA
    // Fix bug 3629 by checking for a NULL pointer
    if ( !tcg.get() )
        tcg.reset( new Tcg255() );
#endif	// USE_JAVA
    tcg->Print( fp );
}

bool Tcg::ImportCustomToolBar( SfxObjectShell& rDocSh )
{
#ifdef USE_JAVA
    // Fix bug 3629 by checking for a NULL pointer
    if ( !tcg.get() )
        tcg.reset( new Tcg255() );
#endif	// USE_JAVA
    return tcg->ImportCustomToolBar( rDocSh );
}

Tcg255::Tcg255()
{
}

Tcg255::~Tcg255()
{
    std::vector< Tcg255SubStruct* >::iterator it = rgtcgData.begin();
    for ( ; it != rgtcgData.end(); ++it )
        delete *it;
}

bool Tcg255::processSubStruct( sal_uInt8 nId, SvStream *pS )
{
     Tcg255SubStruct* pSubStruct = NULL;
     switch ( nId )
     {
         case 0x1:
         {
             pSubStruct = new PlfMcd( false ); // don't read the id
             break;
         }
         case 0x2: 
         {
             pSubStruct = new PlfAcd( false );
             break;
         }
         case 0x3: 
         case 0x4: 
         {
             pSubStruct = new PlfKme( false );
             break;
         }
         case 0x10: 
         {
             pSubStruct = new TcgSttbf( false );
             break;
         }
         case 0x11: 
         {
             pSubStruct = new MacroNames( false );
             break;
         }
         case 0x12: 
         {
             pSubStruct = new CTBWrapper( false );
             break;
         }
         default:
             OSL_TRACE("Unknown id 0x%x",nId);
             return false;
    }
    pSubStruct->ch = nId;
    if ( !pSubStruct->Read( pS ) )
        return false;
    rgtcgData.push_back( pSubStruct );
    return true;
}

bool Tcg255::ImportCustomToolBar( SfxObjectShell& rDocSh )
{
    // Find the CTBWrapper
    for ( std::vector< Tcg255SubStruct* >::const_iterator it = rgtcgData.begin(); it != rgtcgData.end(); ++it )
    {
        if ( (*it)->id() == 0x12 )
        {
            // not so great, shouldn't really have to do a horror casting    
            CTBWrapper* pCTBWrapper =  dynamic_cast< CTBWrapper* > ( *it );
            if ( pCTBWrapper )
            {
                if ( !pCTBWrapper->ImportCustomToolBar( rDocSh ) )
                    return false;
            }
        }
    }
    return true;
}


bool Tcg255::Read(SvStream *pS)
{
    OSL_TRACE("Tcg255::Read() stream pos 0x%x", pS->Tell() );
    nOffSet = pS->Tell();
    sal_uInt8 nId = 0; // 
    *pS >> nId;
    while (  nId != 0x40  )
    {
        if ( !processSubStruct( nId, pS ) )
            return false;
        *pS >> nId;
    } 
    return true;
    // Peek at  
}

void Tcg255::Print( FILE* fp)
{
    Indent a;
    indent_printf(fp, "[ 0x%x ] Tcg255 - dump\n", nOffSet );
    indent_printf(fp, "  contains %d sub records\n", rgtcgData.size() );
    std::vector< Tcg255SubStruct* >::iterator it = rgtcgData.begin(); 
    std::vector< Tcg255SubStruct* >::iterator it_end = rgtcgData.end(); 
    
    for( sal_Int32 count = 1; it != it_end ; ++it, ++count )
    {
        Indent b;
        indent_printf(fp, "  [%d] Tcg255SubStruct \n", static_cast< unsigned int >( count ) );
        (*it)->Print(fp);
    }
}


Tcg255SubStruct::Tcg255SubStruct( bool bReadId ) : mbReadId( bReadId ), ch(0)
{
}

bool Tcg255SubStruct::Read(SvStream *pS)
{
    OSL_TRACE("Tcg255SubStruct::Read() stream pos 0x%x", pS->Tell() );
    nOffSet = pS->Tell();
    if ( mbReadId )
        *pS >> ch;
    return true;
}

PlfMcd::PlfMcd( bool bReadId ): Tcg255SubStruct( bReadId ), rgmcd( NULL )
{
}
PlfMcd::~PlfMcd()
{
    if ( rgmcd )
        delete[] rgmcd;
}

bool PlfMcd::Read(SvStream *pS)
{
    OSL_TRACE("PffMcd::Read() stream pos 0x%x", pS->Tell() );
    nOffSet = pS->Tell();
    Tcg255SubStruct::Read( pS );
    *pS >> iMac;
    if ( iMac )
    { 
        rgmcd = new MCD[ iMac ];
        for ( sal_Int32 index = 0; index < iMac; ++index )
        {
            if ( !rgmcd[ index ].Read( pS ) )
                return false; 
        } 
    }
    return true;
}

void PlfMcd::Print( FILE* fp )
{
    Indent a;
    indent_printf(fp, "[ 0x%x ] PlfMcd ( Tcg255SubStruct ) - dump\n", nOffSet );
    indent_printf(fp, " contains %d MCD records\n", static_cast<int>( iMac ) );
    for ( sal_Int32 count=0; count < iMac; ++count )
    {
        Indent b;
        indent_printf(fp, "[%d] MCD\n", static_cast< int >( count ) );
        rgmcd[ count ].Print( fp );
    }
    
}

PlfAcd::PlfAcd( bool bReadId ) : Tcg255SubStruct( bReadId )
,iMac(0)
,rgacd(NULL)
{
}


PlfAcd::~PlfAcd()
{
    if ( rgacd )
        delete[] rgacd;
}

bool PlfAcd::Read( SvStream *pS)
{
    OSL_TRACE("PffAcd::Read() stream pos 0x%x", pS->Tell() );
    nOffSet = pS->Tell();
    Tcg255SubStruct::Read( pS );
    *pS >> iMac;
    if ( iMac )
    {
        rgacd = new Acd[ iMac ];
        for ( sal_Int32 index = 0; index < iMac; ++index )
        {
            if ( !rgacd[ index ].Read( pS ) ) 
                return false;
        }
    }
    return true;
}
void PlfAcd::Print( FILE* fp )
{
    Indent a;
    indent_printf(fp, "[ 0x%x ] PlfAcd ( Tcg255SubStruct ) - dump\n", nOffSet );
    indent_printf(fp, " contains %d ACD records\n", static_cast< int >( iMac ) );
    for ( sal_Int32 count=0; count < iMac; ++count )
    {
        Indent b;
        indent_printf(fp, "[%d] ACD\n", static_cast< int >( count ) );
        rgacd[ count ].Print( fp );
    }
    
}

PlfKme::PlfKme( bool bReadId ) : Tcg255SubStruct( bReadId )
,iMac( 0 )
,rgkme( NULL )
{
}

PlfKme::~PlfKme()
{
    if ( rgkme )
        delete[] rgkme;
}

bool PlfKme::Read(SvStream *pS)
{
    OSL_TRACE("PlfKme::Read() stream pos 0x%x", pS->Tell() );
    nOffSet = pS->Tell();
    Tcg255SubStruct::Read( pS );
    *pS >> iMac;
    if ( iMac )
    {
        rgkme = new Kme[ iMac ];
        for( sal_Int32 index=0; index<iMac; ++index )
        {
            if ( !rgkme[ index ].Read( pS ) )
                return false;
        }
    }
    return true;
}
 
void PlfKme::Print( FILE* fp )
{
    Indent a;
    indent_printf(fp, "[ 0x%x ] PlfKme ( Tcg255SubStruct ) - dump\n", nOffSet );
    indent_printf(fp, " contains %d Kme records\n", static_cast< int >( iMac ) );
    for ( sal_Int32 count=0; count < iMac; ++count )
    {
        Indent b;
        indent_printf(fp, "[%d] Kme\n", static_cast< int >( count ) );
        rgkme[ count ].Print( fp );
    }
    
}

TcgSttbf::TcgSttbf( bool bReadId ) : Tcg255SubStruct( bReadId )
{
}

bool TcgSttbf::Read( SvStream *pS) 
{
    OSL_TRACE("TcgSttbf::Read() stream pos 0x%x", pS->Tell() );
    nOffSet = pS->Tell();
    Tcg255SubStruct::Read( pS );
    return sttbf.Read( pS );
}

void TcgSttbf::Print( FILE* fp )
{
    Indent a;
    indent_printf(fp,"[ 0x%x ] TcgSttbf - dump\n", nOffSet );
    sttbf.Print( fp );
}

TcgSttbfCore::TcgSttbfCore() : fExtend( 0 )
,cData( 0 )
,cbExtra( 0 )
,dataItems( NULL )
{
}

TcgSttbfCore::~TcgSttbfCore()
{
    if ( dataItems )
        delete[] dataItems;
}

bool TcgSttbfCore::Read( SvStream* pS )
{
    OSL_TRACE("TcgSttbfCore::Read() stream pos 0x%x", pS->Tell() );
    nOffSet = pS->Tell();
    *pS >> fExtend >> cData >> cbExtra;
    if ( cData )
    {
        dataItems = new SBBItem[ cData ];
        for ( sal_Int32 index = 0; index < cData; ++index )
        {
            *pS >> dataItems[ index ].cchData;
            dataItems[ index ].data = readUnicodeString( pS, dataItems[ index ].cchData );
            *pS >> dataItems[ index ].extraData;
        }
    }
    return true;
}

void TcgSttbfCore::Print( FILE* fp )
{
    Indent a;
    indent_printf( fp, "[ 0x%x ] TcgSttbfCore - dump\n");
    indent_printf( fp, " fExtend 0x%x [expected 0xFFFF ]\n", fExtend );
    indent_printf( fp, " cbExtra 0x%x [expected 0x02 ]\n", cbExtra );
    indent_printf( fp, " cData no. or string data items %d (0x%x)\n", cData, cData );
    
    if ( cData )
    {
        for ( sal_Int32 index = 0; index < cData; ++index )
            indent_printf(fp,"   string dataItem[ %d(0x%x) ] has name %s and if referenced %d times.\n", static_cast< int >( index ), static_cast< unsigned int >( index ), rtl::OUStringToOString( dataItems[ index ].data, RTL_TEXTENCODING_UTF8 ).getStr(), dataItems[ index ].extraData );
    }

}
MacroNames::MacroNames( bool bReadId ) : Tcg255SubStruct( bReadId )
,iMac( 0 )
,rgNames( NULL )
{
}

MacroNames::~MacroNames()
{
    if ( rgNames )
        delete[] rgNames;
}

bool MacroNames::Read( SvStream *pS)
{
    OSL_TRACE("MacroNames::Read() stream pos 0x%x", pS->Tell() );
    nOffSet = pS->Tell();
    Tcg255SubStruct::Read( pS );
    *pS >> iMac;
    if ( iMac )
    {
        rgNames = new MacroName[ iMac ]; 
        for ( sal_Int32 index = 0; index < iMac; ++index )
        {
            if ( !rgNames[ index ].Read( pS ) )
                return false;
        }
    }
    return true;
}

void MacroNames::Print( FILE* fp )
{
    Indent a;
    indent_printf(fp, "[ 0x%x ] MacroNames ( Tcg255SubStruct ) - dump\n");
    indent_printf(fp, " contains %d MacroName records\n", iMac );
    for ( sal_Int32 count=0; count < iMac; ++count )
    {
        Indent b;
        indent_printf(fp, "[%d] MacroName\n", static_cast<int>( count ) );
        rgNames[ count ].Print( fp );
    }
    
}

MacroName::MacroName():ibst(0)
{
}


bool MacroName::Read(SvStream *pS)
{
    OSL_TRACE("MacroName::Read() stream pos 0x%x", pS->Tell() );
    nOffSet = pS->Tell();
    *pS >> ibst;
    return xstz.Read( pS );
}

void MacroName::Print( FILE* fp )
{
    Indent a;
    indent_printf( fp, "[ 0x%x ] MacroName - dump");
    indent_printf( fp,"  index - 0x%x has associated following record\n", ibst );
    xstz.Print( fp );
}



Xstz::Xstz():chTerm(0)
{
}

bool 
Xstz::Read(SvStream *pS)
{
    OSL_TRACE("Xstz::Read() stream pos 0x%x", pS->Tell() );
    nOffSet = pS->Tell();
    if ( !xst.Read( pS ) )
        return false;
    *pS >> chTerm;
    if ( chTerm != 0 ) // should be an assert 
        return false;
    return true;
}

void Xstz::Print( FILE* fp )
{
    Indent a;
    indent_printf(fp,"[ 0x%x ] Xstz -- dump\n", nOffSet );
    indent_printf(fp,"  Xst\n");
    xst.Print( fp ); 
    indent_printf(fp,"  chterm 0x%x ( should be zero )\n", chTerm);
}

Kme::Kme() : reserved1(0)
,reserved2(0)
,kcm1(0)
,kcm2(0)
,kt(0)
,param(0)
{
}

Kme::~Kme()
{
}

bool
Kme::Read(SvStream *pS)
{
    OSL_TRACE("Kme::Read() stream pos 0x%x", pS->Tell() );
    nOffSet = pS->Tell();
    *pS >> reserved1 >> reserved2 >> kcm1 >> kcm2 >> kt >> param;
    return true;
}

void Kme::Print( FILE* fp )
{
    Indent a;
    
   indent_printf( fp, "[ 0x%x ] Kme - dump\n", nOffSet );
   indent_printf( fp, " reserved1 0x%x [expected 0x0 ]\n", reserved1 );
   indent_printf( fp, " reserved2 0x%x [expected 0x0 ]\n", reserved2 );
   indent_printf( fp, " kcm1 0x%x [shortcut key]\n", kcm1 );
   indent_printf( fp, " kcm2 0x%x [shortcut key]\n", kcm2 );
   indent_printf( fp, " kt 0x%x \n", kt );
   indent_printf( fp, " param 0x%x \n", static_cast< unsigned int >( param ) );
}

Acd::Acd() : ibst( 0 )
, fciBasedOnABC( 0 )
{
}

bool Acd::Read(SvStream *pS)
{
    OSL_TRACE("Acd::Read() stream pos 0x%x", pS->Tell() );
    nOffSet = pS->Tell();
    *pS >> ibst >> fciBasedOnABC;
    return true;
}

void Acd::Print( FILE* fp )
{
    Indent a;
    indent_printf( fp,"[ 0x%x ] ACD - dump\n", nOffSet );
    // #TODO flesh out interpretation of these values
    indent_printf( fp,"  ibst 0x%x\n", ibst);
    indent_printf( fp,"  fciBaseObABC 0x%x\n", fciBasedOnABC);
}

MCD::MCD() :  reserved1(0x56)
,reserved2( 0 )
,ibst( 0 )
,ibstName( 0 )
,reserved3( 0xFFFF )
,reserved4( 0 )
,reserved5( 0 )
,reserved6( 0 )
,reserved7( 0 )
{
}

bool  MCD::Read(SvStream *pS)
{
    OSL_TRACE("MCD::Read() stream pos 0x%x", pS->Tell() );
    nOffSet = pS->Tell();
    *pS >> reserved1 >> reserved2 >> ibst >> ibstName >> reserved3;
    *pS >> reserved4 >> reserved5 >> reserved6 >> reserved7;
    return true;
}

void MCD::Print( FILE* fp )
{
    Indent a;
    indent_printf( fp, "[ 0x%x ] MCD - dump\n", nOffSet );
    indent_printf( fp, " reserved1 0x%x [expected 0x56 ]\n", reserved1 );
    indent_printf( fp, " reserved2 0x%x [expected 0x0 ]\n", reserved2 );
    indent_printf( fp, " ibst 0x%x specifies macro with MacroName.xstz = 0x%x\n", ibst, ibst );
    indent_printf( fp, " ibstName 0x%x index into command string table ( TcgSttbf.sttbf )\n", ibstName );

    indent_printf( fp, " reserved3 0x%x [expected 0xFFFF ]\n", reserved3 );
    indent_printf( fp, " reserved4 0x%x\n", static_cast< unsigned int >( reserved4 ) );
    indent_printf( fp, " reserved5 0x%x [expected 0x0 ]\n", static_cast< unsigned int >( reserved5 ) );
    indent_printf( fp, " reserved6 0x%x\n", static_cast< unsigned int >( reserved6 ) );
    indent_printf( fp, " reserved7 0x%x\n", static_cast< unsigned int >( reserved7 ) );
}

