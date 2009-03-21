/*************************************************************************
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * $RCSfile$
 * $Revision$
 *
 * This file is part of NeoOffice.
 *
 * NeoOffice is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * NeoOffice is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU General Public License
 * version 3 along with NeoOffice.  If not, see
 * <http://www.gnu.org/licenses/gpl-3.0.txt>
 * for a copy of the GPLv3 License.
 *
 * Modified March 2009 by Patrick Luby. NeoOffice is distributed under
 * GPL only under modification term 2 of the LGPL.
 *
 ************************************************************************/

#ifndef CONFIGMGR_TREEACCESSOR_HXX
#define CONFIGMGR_TREEACCESSOR_HXX

#include "nodeaccess.hxx"
#include "treefragment.hxx"

#ifndef INCLUDED_CSTDDEF
#include <cstddef>
#define INCLUDED_CSTDDEF
#endif
#include <builddata.hxx>

namespace configmgr
{
// -----------------------------------------------------------------------------	
       
// -----------------------------------------------------------------------------	
    namespace data
    {
    // -------------------------------------------------------------------------
        /** class that mediates access to the data of a tree fragment
        */
        class TreeAccessor
	    {
        public:
            TreeAccessor(sharable::TreeFragment *_aTreeRef)
                : m_pTree(_aTreeRef) {}
            TreeAccessor(const sharable::TreeFragment * _pTree) 
                : m_pTree((sharable::TreeFragment *)_pTree) {}

            inline configuration::Name getName() const
#ifdef USE_JAVA
                { return configuration::makeName( m_pTree ? m_pTree->getName() : ::rtl::OUString(),
                                                  configuration::Name::NoValidate() ); }
#else	// USE_JAVA
                { return configuration::makeName( m_pTree->getName(),
                                                  configuration::Name::NoValidate() ); }
#endif	// USE_JAVA

            NodeAccess getRootNode() const
                { return NodeAccess(m_pTree ? const_cast<sharable::Node *>(m_pTree->nodes) : NULL); }

            TreeAddress copyTree() const
                { return data::buildTree(*this); }
            static void freeTree(TreeAddress _aTree)
                { data::destroyTree(_aTree); }

            // make it look like a pointer ...
            operator sharable::TreeFragment *() const { return (sharable::TreeFragment *)m_pTree; }
            sharable::TreeFragment* operator->() const { return m_pTree; }
            bool operator == (sharable::TreeFragment *pTree) const { return pTree == m_pTree; }
            bool operator != (sharable::TreeFragment *pTree) const { return pTree != m_pTree; }

        private:
            TreeAddress  m_pTree;
        };
    // -------------------------------------------------------------------------
    }
// -----------------------------------------------------------------------------	
} // namespace configmgr

#endif // CONFIGMGR_TREEACCESSOR_HXX

