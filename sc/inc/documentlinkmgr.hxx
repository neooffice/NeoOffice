/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_INC_DOCUMENTLINKMGR_HXX
#define INCLUDED_SC_INC_DOCUMENTLINKMGR_HXX

#include <boost/noncopyable.hpp>
#include <rtl/ustring.hxx>

class ScDocument;
class SfxObjectShell;
namespace vcl { class Window; }

namespace sfx2 {

class LinkManager;

}

namespace sc {

class DataStream;
struct DocumentLinkManagerImpl;

class DocumentLinkManager : boost::noncopyable
{
    DocumentLinkManagerImpl* mpImpl;

public:
    DocumentLinkManager( ScDocument& rDoc, SfxObjectShell* pShell );
    ~DocumentLinkManager();

    void setDataStream( DataStream* p );
    DataStream* getDataStream();
    const DataStream* getDataStream() const;

    /**
     * @param bCreate if true, create a new link manager instance in case one
     *                does not exist.
     *
     * @return link manager instance.
     */
    sfx2::LinkManager* getLinkManager( bool bCreate = true );

    const sfx2::LinkManager* getExistingLinkManager() const;

    bool idleCheckLinks();

    bool hasDdeLinks() const;
#ifdef NO_LIBO_WEBSERVICE_LOADING_FIX
    bool hasDdeOrOleLinks() const;
#else	// NO_LIBO_WEBSERVICE_LOADING_FIX
    bool hasDdeOrOleOrWebServiceLinks() const;
#endif	// NO_LIBO_WEBSERVICE_LOADING_FIX
 
#ifdef NO_LIBO_WEBSERVICE_LOADING_FIX
    bool updateDdeOrOleLinks(vcl::Window* pWin);
#else	// NO_LIBO_WEBSERVICE_LOADING_FIX
    bool updateDdeOrOleOrWebServiceLinks(vcl::Window* pWin);
#endif	// NO_LIBO_WEBSERVICE_LOADING_FIX

    bool updateDdeLink( const OUString& rAppl, const OUString& rTopic, const OUString& rItem );

    size_t getDdeLinkCount() const;

    void disconnectDdeLinks();
private:
#ifdef NO_LIBO_WEBSERVICE_LOADING_FIX
    bool hasDdeOrOleLinks(bool bDde, bool bOle) const;
#else	// NO_LIBO_WEBSERVICE_LOADING_FIX
    bool hasDdeOrOleOrWebServiceLinks(bool bDde, bool bOle, bool bWebService) const;
#endif	// NO_LIBO_WEBSERVICE_LOADING_FIX
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
