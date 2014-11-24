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

#include <tools/stream.hxx>
#include <poolfmt.hxx>
#include <shellio.hxx>
#include <ndtxt.hxx>
#include <doc.hxx>
#include <docsh.hxx>
#include <pam.hxx>
#include <swerror.h>

#if SUPD == 310
#include <unoobj.hxx>
#else	// SUPD == 310
#include <unotextrange.hxx>
#endif	// SUPD == 310

#include <unotools/streamwrap.hxx>
#include <comphelper/processfactory.hxx>

#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/document/XImporter.hpp>
#include <com/sun/star/document/XExporter.hpp>
#include <com/sun/star/text/XTextRange.hpp>

#if SUPD == 310
#include <sal/log.hxx>
#endif	// SUPD == 310

using namespace ::com::sun::star;

/// Glue class to call RtfImport as an internal filter, needed by copy&paste support.
class SwRTFReader : public Reader
{
#if SUPD == 310
    virtual ULONG Read( SwDoc &, const String& rBaseURL, SwPaM &,const String &) SAL_OVERRIDE;
#else	// SUPD == 310
    virtual sal_uLong Read( SwDoc &, const OUString& rBaseURL, SwPaM &,const OUString &) SAL_OVERRIDE;
#endif	// SUPD == 310
};

#if SUPD == 310
ULONG SwRTFReader::Read( SwDoc &rDoc, const String& /*rBaseURL*/, SwPaM& rPam, const String &)
#else	// SUPD == 310
sal_uLong SwRTFReader::Read( SwDoc &rDoc, const OUString& /*rBaseURL*/, SwPaM& rPam, const OUString &)
#endif	// SUPD == 310
{
    if (!pStrm)
        return ERR_SWG_READ_ERROR;

    // We want to work in an empty paragraph.
    // Step 1: XTextRange will be updated when content is inserted, so we know
    // the end position.
    const uno::Reference<text::XTextRange> xInsertPosition =
#if SUPD == 310
        SwXTextRange::CreateTextRangeFromPosition(&rDoc, *rPam.GetPoint(), 0);
#else	// SUPD == 310
        SwXTextRange::CreateXTextRange(rDoc, *rPam.GetPoint(), 0);
#endif	// SUPD == 310
    SwNodeIndex *pSttNdIdx = new SwNodeIndex(rDoc.GetNodes());
    const SwPosition* pPos = rPam.GetPoint();

    // Step 2: Split once and remember the node that has been split.
    rDoc.SplitNode( *pPos, false );
    *pSttNdIdx = pPos->nNode.GetIndex()-1;

    // Step 3: Split again.
    rDoc.SplitNode( *pPos, false );

    // Step 4: Insert all content into the new node
    rPam.Move( fnMoveBackward );
    rDoc.SetTxtFmtColl
        ( rPam, rDoc.GetTxtCollFromPool(RES_POOLCOLL_STANDARD, false ) );

    SwDocShell *pDocShell(rDoc.GetDocShell());
    uno::Reference<lang::XMultiServiceFactory> xMultiServiceFactory(comphelper::getProcessServiceFactory());
    uno::Reference<uno::XInterface> xInterface(xMultiServiceFactory->createInstance(
        "com.sun.star.comp.Writer.RtfFilter"), uno::UNO_QUERY_THROW);

    uno::Reference<document::XImporter> xImporter(xInterface, uno::UNO_QUERY_THROW);
    uno::Reference<lang::XComponent> xDstDoc(pDocShell->GetModel(), uno::UNO_QUERY_THROW);
    xImporter->setTargetDocument(xDstDoc);

    const uno::Reference<text::XTextRange> xInsertTextRange =
#if SUPD == 310
        SwXTextRange::CreateTextRangeFromPosition(&rDoc, *rPam.GetPoint(), 0);
#else	// SUPD == 310
        SwXTextRange::CreateXTextRange(rDoc, *rPam.GetPoint(), 0);
#endif	// SUPD == 310

    uno::Reference<document::XFilter> xFilter(xInterface, uno::UNO_QUERY_THROW);
    uno::Sequence<beans::PropertyValue> aDescriptor(3);
    aDescriptor[0].Name = "InputStream";
    uno::Reference<io::XStream> xStream(new utl::OStreamWrapper(*pStrm));
    aDescriptor[0].Value <<= xStream;
    aDescriptor[1].Name = "InsertMode";
    aDescriptor[1].Value <<= sal_True;
    aDescriptor[2].Name = "TextInsertModeRange";
    aDescriptor[2].Value <<= xInsertTextRange;
#if SUPD == 310
    ULONG ret(0);
#else	// SUPD == 310
    sal_uLong ret(0);
#endif	// SUPD == 310
    try {
        xFilter->filter(aDescriptor);
    }
    catch (uno::Exception const& e)
    {
        SAL_WARN("sw.rtf", "SwRTFReader::Read(): exception: " << e.Message);
        ret = ERR_SWG_READ_ERROR;
    }

    // Clean up the fake paragraphs.
    SwUnoInternalPaM aPam(rDoc);
#if SUPD == 310
    SwXTextRange::XTextRangeToSwPaM(aPam, xInsertPosition);
#else	// SUPD == 310
    ::sw::XTextRangeToSwPaM(aPam, xInsertPosition);
#endif	// SUPD == 310
    if (pSttNdIdx->GetIndex())
    {
        // If we are in insert mode, join the split node that is in front
        // of the new content with the first new node. Or in other words:
        // Revert the first split node.
        SwTxtNode* pTxtNode = pSttNdIdx->GetNode().GetTxtNode();
        SwNodeIndex aNxtIdx( *pSttNdIdx );
        if( pTxtNode && pTxtNode->CanJoinNext( &aNxtIdx ) &&
                pSttNdIdx->GetIndex() + 1 == aNxtIdx.GetIndex() )
        {
            // If the PaM points to the first new node, move the PaM to the
            // end of the previous node.
            if( aPam.GetPoint()->nNode == aNxtIdx )
            {
                aPam.GetPoint()->nNode = *pSttNdIdx;
                aPam.GetPoint()->nContent.Assign( pTxtNode,
#if SUPD == 310
                        pTxtNode->GetTxt().Len() );
#else	// SUPD == 310
                        pTxtNode->GetTxt().getLength() );
#endif	// SUPD == 310
            }
            // If the first new node isn't empty, convert  the node's text
            // attributes into hints. Otherwise, set the new node's
            // paragraph style at the previous (empty) node.
            SwTxtNode* pDelNd = aNxtIdx.GetNode().GetTxtNode();
#if SUPD == 310
            if (pTxtNode->GetTxt().Len())
#else	// SUPD == 310
            if (pTxtNode->GetTxt().getLength())
#endif	// SUPD == 310
                pDelNd->FmtToTxtAttr( pTxtNode );
            else
                pTxtNode->ChgFmtColl( pDelNd->GetTxtColl() );
            pTxtNode->JoinNext();
        }
    }

    return ret;
}

extern "C" SAL_DLLPUBLIC_EXPORT Reader* SAL_CALL ImportRTF()
{
    return new SwRTFReader();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
