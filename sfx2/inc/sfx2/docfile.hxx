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
 * Modified January 2010 by Patrick Luby. NeoOffice is distributed under
 * GPL only under modification term 2 of the LGPL.
 *
 ************************************************************************/
#ifndef _SFXDOCFILE_HXX
#define _SFXDOCFILE_HXX

#include <com/sun/star/io/XSeekable.hpp>
#include "sal/config.h"
#include "sfx2/dllapi.h"
#include "sal/types.h"
#include <com/sun/star/util/RevisionTag.hpp>
#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/ucb/XContent.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <tools/stream.hxx>
#include <tools/string.hxx>
#include <tools/list.hxx>
#include <svtools/lstner.hxx>

#include <tools/globname.hxx>
#include <svtools/cancel.hxx>
#include <cppuhelper/weak.hxx>
#include <ucbhelper/content.hxx>

class SvKeyValueIterator;
class SfxObjectFactory;
class SfxFilter;
class SfxMedium_Impl;
class INetURLObject;
class SfxObjectShell;
class SfxFrame;
class Timer;
class SfxItemSet;
class DateTime;
class SvStringsDtor;
class SvEaMgr;
class SfxPoolCancelManager_Impl;

#define SFX_TFPRIO_SYNCHRON                        0
#define SFX_TFPRIO_DOC                            10
#define SFX_TFPRIO_VISIBLE_LOWRES_GRAPHIC         20
#define SFX_TFPRIO_VISIBLE_HIGHRES_GRAPHIC        21
#define SFX_TFPRIO_PLUGINS                        40
#define SFX_TFPRIO_INVISIBLE_LOWRES_GRAPHIC       50
#define SFX_TFPRIO_INVISIBLE_HIGHRES_GRAPHIC      51
#define SFX_TFPRIO_DOWNLOADS                      60

#define S2BS(s) ByteString( s, RTL_TEXTENCODING_MS_1252 )

//____________________________________________________________________________________________________________________________________
//	defines for namespaces
//____________________________________________________________________________________________________________________________________

#define	OUSTRING					::rtl::OUString
#define	XMULTISERVICEFACTORY		::com::sun::star::lang::XMultiServiceFactory
#define	XSERVICEINFO				::com::sun::star::lang::XServiceInfo
#define	OWEAKOBJECT					::cppu::OWeakObject
#define	REFERENCE					::com::sun::star::uno::Reference
#define	XINTERFACE					::com::sun::star::uno::XInterface
#define	SEQUENCE					::com::sun::star::uno::Sequence
#define	EXCEPTION					::com::sun::star::uno::Exception
#define	RUNTIMEEXCEPTION			::com::sun::star::uno::RuntimeException
#define	ANY							::com::sun::star::uno::Any

class SFX2_DLLPUBLIC SfxMedium : public SvRefBase
{
	sal_uInt32          eError;
	sal_Bool            bDirect:1,
						bRoot:1,
						bSetFilter:1,
						bTriedStorage;
	StreamMode          nStorOpenMode;
	INetURLObject*      pURLObj;
	String              aName;
	SvGlobalName        aFilterClass;
	SvStream*			pInStream;
    SvStream*           pOutStream;
//REMOVE		SvStorageRef        aStorage;
	const SfxFilter*	pFilter;
	SfxItemSet*			pSet;
	SfxMedium_Impl*		pImp;
	String           	aLogicName;
	String           	aLongName;
	sal_Bool            bRemote;

    sal_Bool            m_bIsReadOnly;
    com::sun::star::uno::Reference<com::sun::star::io::XInputStream>
    m_xInputStreamToLoadFrom;

#if _SOLAR__PRIVATE
    SAL_DLLPRIVATE void SetIsRemote_Impl();
    SAL_DLLPRIVATE void CloseInStream_Impl();
    SAL_DLLPRIVATE sal_Bool CloseOutStream_Impl();
	SAL_DLLPRIVATE void CloseStreams_Impl();
	DECL_DLLPRIVATE_STATIC_LINK( SfxMedium, UCBHdl_Impl, sal_uInt32 * );

	SAL_DLLPRIVATE void SetPasswordToStorage_Impl();
#endif

public:

	SvCompatWeakHdl*    GetHdl();

						SfxMedium();
						SfxMedium( const String &rName,
								   StreamMode nOpenMode,
                                   sal_Bool bDirect=FALSE,
								   const SfxFilter *pFilter = 0,
								   SfxItemSet *pSet = 0 );

                        SfxMedium( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage,
                                    const String& rBaseURL,
                                    const SfxItemSet* pSet=0,
                                    sal_Bool bRoot = sal_False );

                        SfxMedium( const SfxMedium &rMedium, sal_Bool bCreateTemporary = sal_False );
                        SfxMedium( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aArgs );

						~SfxMedium();

    void                UseInteractionHandler( BOOL );
    ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >
						GetInteractionHandler();

    void setStreamToLoadFrom(const com::sun::star::uno::Reference<com::sun::star::io::XInputStream>& xInputStream,sal_Bool bIsReadOnly )
    { m_xInputStreamToLoadFrom = xInputStream; m_bIsReadOnly = bIsReadOnly; }

    void                SetLoadTargetFrame(SfxFrame* pFrame );
	SfxFrame*           GetLoadTargetFrame() const;
	void                CancelTransfers();

	void                SetReferer( const String& rRefer );
	const String&       GetReferer( ) const;
	sal_Bool            Exists( sal_Bool bForceSession = sal_True );
	void			    SetFilter(const SfxFilter *pFlt, sal_Bool bResetOrig = sal_False);
	const SfxFilter *   GetFilter() const { return pFilter; }
	const SfxFilter *   GetOrigFilter( sal_Bool bNotCurrent = sal_False ) const;
	const String&       GetOrigURL() const;
	SfxItemSet	*		GetItemSet() const;
	void				SetItemSet(SfxItemSet *pSet);
	void                Close();
    void                CloseAndRelease();
	void                ReOpen();
	void                CompleteReOpen();
	const String&       GetName() const {return aLogicName;}
#if defined SINIX && defined GCC && defined C272
	const INetURLObject& GetURLObject();
#else
	const INetURLObject& GetURLObject() const;
#endif

    void                CheckFileDate( const ::com::sun::star::util::DateTime& aInitDate );
    sal_Bool            DocNeedsFileDateCheck();
    ::com::sun::star::util::DateTime GetInitFileDate( sal_Bool bIgnoreOldValue );

    ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XContent > GetContent() const;
	const String&       GetPhysicalName( sal_Bool bForceCreateTempIfRemote = sal_True ) const;
	void                SetTemporary( sal_Bool bTemp );
    sal_Bool            IsTemporary() const;
    sal_Bool            IsRemote();
	sal_Bool            IsOpen() const; // { return aStorage.Is() || pInStream; }
	void                StartDownload();
	void                DownLoad( const Link& aLink = Link());
	void                SetDoneLink( const Link& rLink );
	Link                GetDoneLink( ) const;
	void                SetDataAvailableLink( const Link& rLink );
	Link                GetDataAvailableLink( ) const;

	void                SetClassFilter( const SvGlobalName & rFilterClass );

	sal_uInt32          GetMIMEAndRedirect( String& );
	sal_uInt32          GetErrorCode() const;
	sal_uInt32          GetError() const
						{ return ERRCODE_TOERROR(GetErrorCode()); }
	sal_uInt32			GetLastStorageCreationState();

	void                SetError( sal_uInt32 nError ) { eError = nError; }

	void                CloseInStream();
	sal_Bool            CloseOutStream();

	sal_Bool            IsRoot() const { return bRoot; }
	void				CloseStorage();

	StreamMode			GetOpenMode() const { return nStorOpenMode; }
    void                SetOpenMode( StreamMode nStorOpen, sal_Bool bDirect, sal_Bool bDontClose = sal_False );
	sal_Bool			IsDirect() const { return bDirect? sal_True: sal_False; }

	SvStream*           GetInStream();
	SvStream*           GetOutStream();

	SvEaMgr*			GetEaMgr();

	sal_Bool            Commit();
	sal_Bool            TryStorage();
	SAL_DLLPRIVATE ErrCode Unpack_Impl( const String& );
    sal_Bool            IsStorage();

    sal_Int8            ShowLockedDocumentDialog( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aData, sal_Bool bIsLoading, sal_Bool bOwnLock );
    sal_Bool            LockOrigFileOnDemand( sal_Bool bLoading, sal_Bool bNoUI );
    void                UnlockFile();

	::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage > GetStorage();
    ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage > GetOutputStorage();
	const SvGlobalName& GetClassFilter();
	void				ResetError();
	sal_Bool            UsesCache() const;
	void                SetUsesCache( sal_Bool );
	sal_Bool            IsExpired() const;
	void                SetName( const String& rName, sal_Bool bSetOrigURL = sal_False );
	void                SetDontCreateCancellable();
	sal_Bool			IsAllowedForExternalBrowser() const;
	long				GetFileVersion() const;

    const com::sun::star::uno::Sequence < com::sun::star::util::RevisionTag >&
                        GetVersionList( bool _bNoReload = false );
	sal_Bool			IsReadOnly();

    ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >  GetInputStream();

	void				CreateTempFile();
	void				CreateTempFileNoCopy();
	void				TryToSwitchToRepairedTemp();
    ::rtl::OUString     SwitchDocumentToTempFile();
    sal_Bool            SwitchDocumentToFile( ::rtl::OUString aURL );

	::rtl::OUString		GetCharset();
	void				SetCharset( ::rtl::OUString );
    ::rtl::OUString     GetBaseURL( bool bForSaving=false );

    sal_Bool            SupportsActiveStreaming( const rtl::OUString &rName ) const;
#if defined USE_JAVA && defined MACOSX
	void				CheckForMovedFile( SfxObjectShell *pDoc );
#endif	// USE_JAVA && MACOSX

#if _SOLAR__PRIVATE
//REMOVE		// the storage will be truncated, if it is still not open then the stream will be truncated
//REMOVE	    ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage > GetOutputStorage_Impl();
	SAL_DLLPRIVATE ::rtl::OUString GetOutputStorageURL_Impl();
    SAL_DLLPRIVATE BOOL HasStorage_Impl() const;

	SAL_DLLPRIVATE sal_Bool BasedOnOriginalFile_Impl();
	SAL_DLLPRIVATE void StorageBackup_Impl();
	SAL_DLLPRIVATE ::rtl::OUString GetBackup_Impl();

	SAL_DLLPRIVATE ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage > GetLastCommitReadStorage_Impl();
	SAL_DLLPRIVATE void CloseReadStorage_Impl();

	// the storage that will be returned by the medium on GetStorage request
	SAL_DLLPRIVATE void SetStorage_Impl( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xNewStorage );

    SAL_DLLPRIVATE ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > GetInputStream_Impl();
	SAL_DLLPRIVATE void CloseAndReleaseStreams_Impl();
//REMOVE	    SvStorage*          GetStorage_Impl( BOOL bUCBStorage );
	SAL_DLLPRIVATE void RefreshName_Impl();
    SAL_DLLPRIVATE sal_uInt16 AddVersion_Impl( com::sun::star::util::RevisionTag& rVersion );
	SAL_DLLPRIVATE sal_Bool TransferVersionList_Impl( SfxMedium& rMedium );
	SAL_DLLPRIVATE sal_Bool SaveVersionList_Impl( sal_Bool bUseXML );
    SAL_DLLPRIVATE sal_Bool RemoveVersion_Impl( const ::rtl::OUString& rVersion );
	SAL_DLLPRIVATE SfxPoolCancelManager_Impl* GetCancelManager_Impl() const;
	SAL_DLLPRIVATE void SetCancelManager_Impl( SfxPoolCancelManager_Impl* pMgr );

	SAL_DLLPRIVATE void SetExpired_Impl( const DateTime& rDateTime );
	SAL_DLLPRIVATE SvKeyValueIterator* GetHeaderAttributes_Impl();
	SAL_DLLPRIVATE const String& GetPreRedirectedURL() const;
	SAL_DLLPRIVATE void SetOrigFilter_Impl( const SfxFilter* pFilter );

	// Diese Protokolle liefern MIME Typen
	SAL_DLLPRIVATE sal_Bool SupportsMIME_Impl() const;

	SAL_DLLPRIVATE void Init_Impl();
	SAL_DLLPRIVATE void ForceSynchronStream_Impl( sal_Bool bSynchron );

	SAL_DLLPRIVATE void GetMedium_Impl();
	SAL_DLLPRIVATE sal_Bool TryDirectTransfer( const ::rtl::OUString& aURL, SfxItemSet& aTargetSet );
	SAL_DLLPRIVATE void Transfer_Impl();
	SAL_DLLPRIVATE void CreateFileStream();
	SAL_DLLPRIVATE void SetUpdatePickList(sal_Bool);
	SAL_DLLPRIVATE sal_Bool IsUpdatePickList() const;

//REMOVE		void                SetStorage_Impl( SvStorage* pStor );
	SAL_DLLPRIVATE void SetLongName(const String &rName)
						{ aLongName = rName; }
	SAL_DLLPRIVATE const String & GetLongName() const { return aLongName; }
	SAL_DLLPRIVATE ErrCode CheckOpenMode_Impl( sal_Bool bSilent, sal_Bool bAllowRO = sal_True );
	SAL_DLLPRIVATE sal_Bool IsDownloadDone_Impl();
    SAL_DLLPRIVATE sal_Bool IsPreview_Impl();
	SAL_DLLPRIVATE void ClearBackup_Impl();
    SAL_DLLPRIVATE void Done_Impl( ErrCode );
    SAL_DLLPRIVATE void DataAvailable_Impl();
    SAL_DLLPRIVATE void Cancel_Impl();
	SAL_DLLPRIVATE void SetPhysicalName_Impl(const String& rName);
	SAL_DLLPRIVATE void MoveTempTo_Impl( SfxMedium* pMedium );
    SAL_DLLPRIVATE void CanDisposeStorage_Impl( sal_Bool bDisposeStorage );
    SAL_DLLPRIVATE sal_Bool WillDisposeStorageOnClose_Impl();

	SAL_DLLPRIVATE void DoBackup_Impl();
	SAL_DLLPRIVATE void DoInternalBackup_Impl( const ::ucbhelper::Content& aOriginalContent );
	SAL_DLLPRIVATE void DoInternalBackup_Impl( const ::ucbhelper::Content& aOriginalContent,
												const String& aPrefix,
												const String& aExtension,
												const String& aDestDir );

	SAL_DLLPRIVATE sal_Bool UseBackupToRestore_Impl( ::ucbhelper::Content& aOriginalContent,
					 		const ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XCommandEnvironment >& xComEnv );

	SAL_DLLPRIVATE sal_Bool StorageCommit_Impl();

	SAL_DLLPRIVATE sal_Bool TransactedTransferForFS_Impl( const INetURLObject& aSource,
					 		const INetURLObject& aDest,
					 		const ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XCommandEnvironment >& xComEnv );

	SAL_DLLPRIVATE sal_Bool SignContents_Impl( sal_Bool bScriptingContent );

	// the following two methods must be used and make sence only during saving currently
	// TODO/LATER: in future the signature state should be controlled by the medium not by the document
	//             in this case the methods will be used generally, and might need to be renamed
	SAL_DLLPRIVATE sal_uInt16 GetCachedSignatureState_Impl();
	SAL_DLLPRIVATE void       SetCachedSignatureState_Impl( sal_uInt16 nState );
#endif

    static com::sun::star::uno::Sequence < com::sun::star::util::RevisionTag > GetVersionList(
					const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage );
	static sal_Bool EqualURLs( const ::rtl::OUString& aFirstURL, const ::rtl::OUString& aSecondURL );
	static ::rtl::OUString CreateTempCopyWithExt( const ::rtl::OUString& aURL );
};

SV_DECL_IMPL_REF( SfxMedium )
SV_DECL_COMPAT_WEAK( SfxMedium )

#ifndef SFXMEDIUM_LIST
#define SFXMEDIUM_LIST
DECLARE_LIST( SfxMediumList, SfxMedium* )
#endif

/*========================================================================
 *
 * SvKeyValue.
 *
 *======================================================================*/

#ifndef COPYCTOR_API
#define COPYCTOR_API(C) C (const C&); C& operator= (const C&)
#endif
SV_DECL_REF(SvKeyValueIterator)

class SvKeyValue
{
	/** Representation.
	*/
	String m_aKey;
	String m_aValue;

public:
	/** Construction.
	*/
	SvKeyValue (void)
	{}

	SvKeyValue (const String &rKey, const String &rValue)
		: m_aKey (rKey), m_aValue (rValue)
	{}

	SvKeyValue (const SvKeyValue &rOther)
		: m_aKey (rOther.m_aKey), m_aValue (rOther.m_aValue)
	{}

	/** Assignment.
	*/
	SvKeyValue& operator= (SvKeyValue &rOther)
	{
		m_aKey   = rOther.m_aKey;
		m_aValue = rOther.m_aValue;
		return *this;
	}

	/** Operation.
	*/
	const String& GetKey   (void) const { return m_aKey; }
	const String& GetValue (void) const { return m_aValue; }

	void SetKey   (const String &rKey  ) { m_aKey = rKey; }
	void SetValue (const String &rValue) { m_aValue = rValue; }
};

/*========================================================================
 *
 * SvKeyValueIterator.
 *
 *======================================================================*/
class SvKeyValueList_Impl;
class SFX2_DLLPUBLIC SvKeyValueIterator : public SvRefBase
{
	/** Representation.
	*/
	SvKeyValueList_Impl* m_pList;
	USHORT               m_nPos;

	/** Not implemented.
	*/
	COPYCTOR_API(SvKeyValueIterator);

public:
	/** Construction/Destruction.
	*/
	SvKeyValueIterator (void);
	virtual ~SvKeyValueIterator (void);

	/** Operation.
	*/
	virtual BOOL GetFirst (SvKeyValue &rKeyVal);
	virtual BOOL GetNext  (SvKeyValue &rKeyVal);
	virtual void Append   (const SvKeyValue &rKeyVal);
};

SV_IMPL_REF(SvKeyValueIterator);



#endif

