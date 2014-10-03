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
 *************************************************************/



#ifndef OOX_HELPER_ZIPSTORAGE_HXX
#define OOX_HELPER_ZIPSTORAGE_HXX

#include "oox/helper/storagebase.hxx"

namespace com { namespace sun { namespace star {
    namespace uno { class XComponentContext; }
} } }

namespace oox {

// ============================================================================

/** Implements stream access for ZIP storages containing XML streams. */
class ZipStorage : public StorageBase
{
public:
    explicit            ZipStorage(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& rxInStream );

    explicit            ZipStorage(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream >& rxStream );

    virtual             ~ZipStorage();

private:
    explicit            ZipStorage(
                            const ZipStorage& rParentStorage,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& rxStorage,
                            const ::rtl::OUString& rElementName );

    /** Returns true, if the object represents a valid storage. */
    virtual bool        implIsStorage() const;

    /** Returns the com.sun.star.embed.XStorage interface of the current storage. */
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >
                        implGetXStorage() const;

    /** Returns the names of all elements of this storage. */
    virtual void        implGetElementNames( ::std::vector< ::rtl::OUString >& orElementNames ) const;

    /** Opens and returns the specified sub storage from the storage. */
    virtual StorageRef  implOpenSubStorage( const ::rtl::OUString& rElementName, bool bCreateMissing );

    /** Opens and returns the specified input stream from the storage. */
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >
                        implOpenInputStream( const ::rtl::OUString& rElementName );

    /** Opens and returns the specified output stream from the storage. */
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream >
                        implOpenOutputStream( const ::rtl::OUString& rElementName );

    /** Commits the current storage. */
    virtual void        implCommit() const;

private:
    ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >
                        mxStorage;      /// Storage based on input or output stream.
};

// ============================================================================

} // namespace oox

#endif
