/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "interpre.hxx"
#include <rtl/strbuf.hxx>
#ifndef NO_LIBO_WEBSERVICE_LOADING_FIX
#include <formulacell.hxx>
#endif	// !NO_LIBO_WEBSERVICE_LOADING_FIX
#include <formula/errorcodes.hxx>
#ifndef NO_LIBO_WEBSERVICE_LOADING_FIX
#include <sfx2/bindings.hxx>
#endif	// !NO_LIBO_WEBSERVICE_LOADING_FIX
#include <svtools/miscopt.hxx>
#ifndef NO_LIBO_CVE_2018_6871_FIX
#include <tools/urlobj.hxx>
#endif	// !NO_LIBO_CVE_2018_6871_FIX

#include <com/sun/star/ucb/XSimpleFileAccess3.hpp>
#include <com/sun/star/ucb/SimpleFileAccess.hpp>
#include <com/sun/star/io/XInputStream.hpp>

#include "libxml/xpath.h"
#include <datastreamgettime.hxx>
#include <dpobject.hxx>
#include <document.hxx>
#ifndef NO_LIBO_WEBSERVICE_LOADING_FIX
#include <tokenarray.hxx>
#include <webservicelink.hxx>

#include <sc.hrc>
#endif	// !NO_LIBO_WEBSERVICE_LOADING_FIX

#include <boost/shared_ptr.hpp>
#include <cstring>

using namespace com::sun::star;

// TODO: Add new methods for ScInterpreter here.

void ScInterpreter::ScFilterXML()
{
    sal_uInt8 nParamCount = GetByte();
    if (MustHaveParamCount( nParamCount, 2 ) )
    {
        OUString aXPathExpression = GetString().getString();
        OUString aString = GetString().getString();
        if(aString.isEmpty() || aXPathExpression.isEmpty())
        {
            PushError( errNoValue );
            return;
        }

        OString aOXPathExpression = OUStringToOString( aXPathExpression, RTL_TEXTENCODING_UTF8 );
        const char* pXPathExpr = aOXPathExpression.getStr();
        OString aOString = OUStringToOString( aString, RTL_TEXTENCODING_UTF8 );
        const char* pXML = aOString.getStr();

        boost::shared_ptr<xmlParserCtxt> pContext(
                xmlNewParserCtxt(), xmlFreeParserCtxt );

        boost::shared_ptr<xmlDoc> pDoc( xmlParseMemory( pXML, aOString.getLength() ),
                xmlFreeDoc );

        if(!pDoc)
        {
            PushError( errNoValue );
            return;
        }

        boost::shared_ptr<xmlXPathContext> pXPathCtx( xmlXPathNewContext(pDoc.get()),
                xmlXPathFreeContext );

        boost::shared_ptr<xmlXPathObject> pXPathObj( xmlXPathEvalExpression(BAD_CAST(pXPathExpr), pXPathCtx.get()),
                xmlXPathFreeObject );

        if(!pXPathObj)
        {
            PushError( errNoValue );
            return;
        }

        rtl::OUString aResult;

        switch(pXPathObj->type)
        {
            case XPATH_UNDEFINED:
                break;
            case XPATH_NODESET:
                {
                    xmlNodeSetPtr pNodeSet = pXPathObj->nodesetval;
                    if(!pNodeSet)
                    {
                        PushError( errNoValue );
                        return;
                    }

                    size_t nSize = pNodeSet->nodeNr;
                    if( nSize >= 1 )
                    {
                        if(pNodeSet->nodeTab[0]->type == XML_NAMESPACE_DECL)
                        {
                            xmlNsPtr ns = (xmlNsPtr)pNodeSet->nodeTab[0];
                            xmlNodePtr cur = (xmlNodePtr)ns->next;
                            boost::shared_ptr<xmlChar> pChar2(xmlNodeGetContent(cur), xmlFree);
                            aResult = OStringToOUString(OString((char*)pChar2.get()), RTL_TEXTENCODING_UTF8);
                        }
                        else
                        {
                            xmlNodePtr cur = pNodeSet->nodeTab[0];
                            boost::shared_ptr<xmlChar> pChar2(xmlNodeGetContent(cur), xmlFree);
                            aResult = OStringToOUString(OString((char*)pChar2.get()), RTL_TEXTENCODING_UTF8);
                        }
                    }
                    else
                    {
                        PushError( errNoValue );
                        return;
                    }
                }
                PushString(aResult);
                break;
            case XPATH_BOOLEAN:
                {
                    bool bVal = pXPathObj->boolval != 0;
                    PushDouble(double(bVal));
                }
                break;
            case XPATH_NUMBER:
                {
                    double fVal = pXPathObj->floatval;
                    PushDouble(fVal);
                }
                break;
            case XPATH_STRING:
                PushString(OUString::createFromAscii((char*)pXPathObj->stringval));
                break;
            case XPATH_POINT:
                PushNoValue();
                break;
            case XPATH_RANGE:
                PushNoValue();
                break;
            case XPATH_LOCATIONSET:
                PushNoValue();
                break;
            case XPATH_USERS:
                PushNoValue();
                break;
            case XPATH_XSLT_TREE:
                PushNoValue();
                break;

        }
    }
}

#ifndef NO_LIBO_WEBSERVICE_LOADING_FIX

static ScWebServiceLink* lcl_GetWebServiceLink(const sfx2::LinkManager* pLinkMgr, const OUString& rURL)
{
    size_t nCount = pLinkMgr->GetLinks().size();
    for (size_t i=0; i<nCount; ++i)
    {
        ::sfx2::SvBaseLink* pBase = *pLinkMgr->GetLinks()[i];
        if (ScWebServiceLink* pLink = dynamic_cast<ScWebServiceLink*>(pBase))
        {
            if (pLink->GetURL() == rURL)
                return pLink;
        }
    }

    return nullptr;
}

static bool lcl_FunctionAccessLoadWebServiceLink( OUString& rResult, ScDocument* pDoc, const OUString& rURI )
{
    // For FunctionAccess service always force a changed data update.
    ScWebServiceLink aLink( pDoc, rURI);
    if (aLink.DataChanged( OUString(), css::uno::Any()) != sfx2::SvBaseLink::UpdateResult::SUCCESS)
        return false;

    if (!aLink.HasResult())
        return false;

    rResult = aLink.GetResult();

    return true;
}

#endif	// !NO_LIBO_WEBSERVICE_LOADING_FIX

void ScInterpreter::ScWebservice()
{
    sal_uInt8 nParamCount = GetByte();
    if (MustHaveParamCount( nParamCount, 1 ) )
    {
        OUString aURI = GetString().getString();

        if(aURI.isEmpty())
        {
            PushError( errNoValue );
            return;
        }

#ifndef NO_LIBO_CVE_2018_6871_FIX
        INetURLObject aObj(aURI, INET_PROT_FILE);
        INetProtocol eProtocol = aObj.GetProtocol();
        if (eProtocol != INET_PROT_HTTP && eProtocol != INET_PROT_HTTPS)
        {
            PushError( errNoValue );
            return;
        }
#endif	// !NO_LIBO_CVE_2018_6871_FIX

#ifdef NO_LIBO_WEBSERVICE_LOADING_FIX
        uno::Reference< ucb::XSimpleFileAccess3 > xFileAccess( ucb::SimpleFileAccess::create( comphelper::getProcessComponentContext() ), uno::UNO_QUERY );
        if(!xFileAccess.is())
#else	// NO_LIBO_WEBSERVICE_LOADING_FIX
        if (!mpLinkManager)
#endif	// NO_LIBO_WEBSERVICE_LOADING_FIX
        {
#ifdef NO_LIBO_WEBSERVICE_LOADING_FIX
            PushError( errNoValue );
#else	// NO_LIBO_WEBSERVICE_LOADING_FIX
            if (!pDok->IsFunctionAccess() || pDok->HasLinkFormulaNeedingCheck())
            {
                PushError( errNoValue );
            }
            else
            {
                OUString aResult;
                if (lcl_FunctionAccessLoadWebServiceLink( aResult, pDok, aURI))
                    PushString( aResult);
                else
                    PushError( errNoValue );
            }
#endif	// NO_LIBO_WEBSERVICE_LOADING_FIX
            return;
        }

#ifdef NO_LIBO_WEBSERVICE_LOADING_FIX
        uno::Reference< io::XInputStream > xStream;
        try {
            xStream = xFileAccess->openFileRead( aURI );
        }
        catch (...)
        {
            // don't let any exceptions pass
            PushError( errNoValue );
            return;
        }
        if ( !xStream.is() )
        {
            PushError( errNoValue );
            return;
        }
#else	// NO_LIBO_WEBSERVICE_LOADING_FIX
        // Need to reinterpret after loading (build links)
        if (rArr.IsRecalcModeNormal())
            rArr.SetExclusiveRecalcModeOnLoad();
#endif	// NO_LIBO_WEBSERVICE_LOADING_FIX

#ifdef NO_LIBO_WEBSERVICE_LOADING_FIX
        const sal_Int32 BUF_LEN = 8000;
        uno::Sequence< sal_Int8 > buffer( BUF_LEN );
        OStringBuffer aBuffer( 64000 );
#else	// NO_LIBO_WEBSERVICE_LOADING_FIX
        //  while the link is not evaluated, idle must be disabled (to avoid circular references)
        bool bOldEnabled = pDok->IsIdleEnabled();
        pDok->EnableIdle(false);
#endif	// NO_LIBO_WEBSERVICE_LOADING_FIX

#ifdef NO_LIBO_WEBSERVICE_LOADING_FIX
        sal_Int32 nRead = 0;
        while ( ( nRead = xStream->readBytes( buffer, BUF_LEN ) ) == BUF_LEN )
#else	// NO_LIBO_WEBSERVICE_LOADING_FIX
        // Get/ Create link object
        ScWebServiceLink* pLink = lcl_GetWebServiceLink(mpLinkManager, aURI);

        bool bWasError = (pMyFormulaCell && pMyFormulaCell->GetRawError() != 0);
        bool bLinkFormulaNeedingCheck = false;

        if (!pLink)
#endif	// NO_LIBO_WEBSERVICE_LOADING_FIX
        {
#ifdef NO_LIBO_WEBSERVICE_LOADING_FIX
            aBuffer.append( reinterpret_cast< const char* >( buffer.getConstArray() ), nRead );
        }
#else	// NO_LIBO_WEBSERVICE_LOADING_FIX
            pLink = new ScWebServiceLink(pDok, aURI);
            mpLinkManager->InsertFileLink(*pLink, OBJECT_CLIENT_FILE, aURI);
            if ( mpLinkManager->GetLinks().size() == 1 )                    // the first one?
            {
                SfxBindings* pBindings = pDok->GetViewBindings();
                if (pBindings)
                    pBindings->Invalidate( SID_LINKS );             // Link-Manager enabled
            }

            //if the document was just loaded, but the ScDdeLink entry was missing, then
            //don't update this link until the links are updated in response to the users
            //decision
            bLinkFormulaNeedingCheck = pDok->HasLinkFormulaNeedingCheck();
fprintf( stderr, "Here 0: %i\n", bLinkFormulaNeedingCheck );
            if (!bLinkFormulaNeedingCheck)
            {
fprintf( stderr, "Here 1\n" );
                pLink->Update();
            }
#endif	// NO_LIBO_WEBSERVICE_LOADING_FIX

#ifdef NO_LIBO_WEBSERVICE_LOADING_FIX
        if ( nRead > 0 )
#else	// NO_LIBO_WEBSERVICE_LOADING_FIX
            if (pMyFormulaCell)
            {
                // StartListening after the Update to avoid circular references
                pMyFormulaCell->StartListening(*pLink);
            }
        }
        else
#endif	// NO_LIBO_WEBSERVICE_LOADING_FIX
        {
#ifdef NO_LIBO_WEBSERVICE_LOADING_FIX
            aBuffer.append( reinterpret_cast< const char* >( buffer.getConstArray() ), nRead );
#else	// NO_LIBO_WEBSERVICE_LOADING_FIX
            if (pMyFormulaCell)
                pMyFormulaCell->StartListening(*pLink);
#endif	// NO_LIBO_WEBSERVICE_LOADING_FIX
        }

#ifdef NO_LIBO_WEBSERVICE_LOADING_FIX
        xStream->closeInput();
#else	// NO_LIBO_WEBSERVICE_LOADING_FIX
        //  If an new Error from Reschedule appears when the link is executed then reset the errorflag
        if (pMyFormulaCell && pMyFormulaCell->GetRawError() != 0 && !bWasError)
            pMyFormulaCell->SetErrCode(0);

        //  check the value
        if (pLink->HasResult())
            PushString(pLink->GetResult());
        else
        {
            // If this formula cell is recalculated just after load and the
            // expression is exactly WEBSERVICE("literal_URI") (i.e. no other
            // calculation involved, not even a cell reference) and a cached
            // result is set as hybrid string then use that as result value to
            // prevent a #VALUE! result due to the "Automatic update of
            // external links has been disabled."
            // This will work only once, as the new formula cell result won't
            // be a hybrid anymore.
            if (bLinkFormulaNeedingCheck && pMyFormulaCell && pMyFormulaCell->GetCode()->GetCodeLen() == 2 &&
                    pMyFormulaCell->HasHybridStringResult())
            {
                formula::FormulaToken const * const * pRPN = pMyFormulaCell->GetCode()->GetCode();
                if (pRPN[0]->GetType() == formula::svString && pRPN[1]->GetOpCode() == ocWebservice)
                    PushString( pMyFormulaCell->GetResultString());
                else
                    PushError(errNoValue);
            }
            else
                PushError(errNoValue);
        }
#endif	// NO_LIBO_WEBSERVICE_LOADING_FIX

#ifdef NO_LIBO_WEBSERVICE_LOADING_FIX
        OUString aContent = OStringToOUString( aBuffer.makeStringAndClear(), RTL_TEXTENCODING_UTF8 );
        PushString( aContent );
#else	// NO_LIBO_WEBSERVICE_LOADING_FIX
        pDok->EnableIdle(bOldEnabled);
        mpLinkManager->CloseCachedComps();
#endif	// NO_LIBO_WEBSERVICE_LOADING_FIX
    }
}

void ScInterpreter::ScDebugVar()
{
    // This is to be used by developers only!  Never document this for end
    // users.  This is a convenient way to extract arbitrary internal state to
    // a cell for easier debugging.

    SvtMiscOptions aMiscOptions;
    if (!aMiscOptions.IsExperimentalMode())
    {
        PushError(ScErrorCodes::errNoName);
        return;
    }

    if (!MustHaveParamCount(GetByte(), 1))
    {
        PushIllegalParameter();
        return;
    }

    rtl_uString* p = GetString().getDataIgnoreCase();
    if (!p)
    {
        PushIllegalParameter();
        return;
    }

    OUString aStrUpper(p);

    if (aStrUpper == "PIVOTCOUNT")
    {
        // Set the number of pivot tables in the document.

        double fVal = 0.0;
        if (pDok->HasPivotTable())
        {
            const ScDPCollection* pDPs = pDok->GetDPCollection();
            fVal = pDPs->GetCount();
        }
        PushDouble(fVal);
    }
    else if (aStrUpper == "DATASTREAM_IMPORT")
        PushDouble( sc::datastream_get_time( 0 ) );
    else if (aStrUpper == "DATASTREAM_RECALC")
        PushDouble( sc::datastream_get_time( 1 ) );
    else if (aStrUpper == "DATASTREAM_RENDER")
        PushDouble( sc::datastream_get_time( 2 ) );
    else
        PushIllegalParameter();
}

void ScInterpreter::ScErf()
{
    sal_uInt8 nParamCount = GetByte();
    if (MustHaveParamCount( nParamCount, 1 ) )
    {
        double x = GetDouble();
        PushDouble( ::rtl::math::erf( x ) );
    }
}

void ScInterpreter::ScErfc()
{
    sal_uInt8 nParamCount = GetByte();
    if (MustHaveParamCount( nParamCount, 1 ) )
    {
        double x = GetDouble();
        PushDouble( ::rtl::math::erfc( x ) );
    }
}

void ScInterpreter::ScColor()
{
    sal_uInt8 nParamCount = GetByte();
    if(MustHaveParamCount(nParamCount, 3, 4))
    {
        double nAlpha = 0;
        if(nParamCount == 4)
            nAlpha = rtl::math::approxFloor(GetDouble());

        if(nAlpha < 0 || nAlpha > 255)
        {
            PushIllegalArgument();
            return;
        }

        double nBlue = rtl::math::approxFloor(GetDouble());

        if(nBlue < 0 || nBlue > 255)
        {
            PushIllegalArgument();
            return;
        }

        double nGreen = rtl::math::approxFloor(GetDouble());

        if(nGreen < 0 || nGreen > 255)
        {
            PushIllegalArgument();
            return;
        }

        double nRed = rtl::math::approxFloor(GetDouble());

        if(nRed < 0 || nRed > 255)
        {
            PushIllegalArgument();
            return;
        }

        double nVal = 256*256*256*nAlpha + 256*256*nRed + 256*nGreen + nBlue;
        PushDouble(nVal);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
