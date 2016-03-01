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



#ifndef _LINGU2_SPELLIMP_HXX_
#define _LINGU2_SPELLIMP_HXX_

#include <uno/lbnames.h>			// CPPU_CURRENT_LANGUAGE_BINDING_NAME macro, which specify the environment type
#include <cppuhelper/implbase1.hxx>	// helper for implementations
#if defined USE_JAVA && defined MACOSX
#include <cppuhelper/implbase7.hxx>	// helper for implementations
#else	// USE_JAVA && MACOSX
#include <cppuhelper/implbase6.hxx>	// helper for implementations
#endif	// USE_JAVA && MACOSX
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceDisplayName.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/PropertyValues.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/linguistic2/XSpellChecker.hpp>
#include <com/sun/star/linguistic2/XSearchableDictionaryList.hpp>
#include <com/sun/star/linguistic2/XLinguServiceEventBroadcaster.hpp>
#include <tools/table.hxx>

#include <linguistic/misc.hxx>
#include <linguistic/lngprophelp.hxx>

#include <lingutil.hxx>

#if defined USE_JAVA && defined MACOSX

#include <com/sun/star/linguistic2/XProofreader.hpp>

#include <map>
#include "sspellimp_cocoa.h"
 
#endif	// USE_JAVA && MACOSX

using namespace ::rtl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::linguistic2;


///////////////////////////////////////////////////////////////////////////


class SpellChecker :
#if defined USE_JAVA && defined MACOSX
	public cppu::WeakImplHelper7
#else	// USE_JAVA && MACOSX
	public cppu::WeakImplHelper6
#endif	// USE_JAVA && MACOSX
	<
		XSpellChecker,
		XLinguServiceEventBroadcaster,
		XInitialization,
		XComponent,
		XServiceInfo,
		XServiceDisplayName
#if defined USE_JAVA && defined MACOSX
		, XProofreader
#endif	// USE_JAVA && MACOSX
	>
{
	Sequence< Locale >                 aSuppLocales;
    Hunspell **                        aDicts;
    rtl_TextEncoding *                 aDEncs;
    Locale *                           aDLocs;
    OUString *                         aDNames;
    sal_Int32                          numdict;

#if defined USE_JAVA && defined MACOSX
	CFArrayRef								maLocales;
	::std::map< ::rtl::OUString, CFStringRef >	maPrimaryNativeLocaleMap;
	::std::map< ::rtl::OUString, CFStringRef >	maSecondaryNativeLocaleMap;
#endif	// USE_JAVA && MACOSX
	::cppu::OInterfaceContainerHelper		aEvtListeners;
	Reference< XPropertyChangeListener >	xPropHelper;
    linguistic::PropertyHelper_Spell *      pPropHelper;
	sal_Bool									bDisposing;

	// disallow copy-constructor and assignment-operator for now
	SpellChecker(const SpellChecker &);
	SpellChecker & operator = (const SpellChecker &);

    linguistic::PropertyHelper_Spell &  GetPropHelper_Impl();
    linguistic::PropertyHelper_Spell &  GetPropHelper()
	{
		return pPropHelper ? *pPropHelper : GetPropHelper_Impl();
	}

	sal_Int16	GetSpellFailure( const OUString &rWord, const Locale &rLocale );
    Reference< XSpellAlternatives > GetProposals( const OUString &rWord, const Locale &rLocale );

public:
	SpellChecker();
	virtual ~SpellChecker();

	// XSupportedLocales (for XSpellChecker)
    virtual Sequence< Locale > SAL_CALL getLocales() throw(RuntimeException);
    virtual sal_Bool SAL_CALL hasLocale( const Locale& rLocale ) throw(RuntimeException);

	// XSpellChecker
    virtual sal_Bool SAL_CALL isValid( const OUString& rWord, const Locale& rLocale, const PropertyValues& rProperties ) throw(IllegalArgumentException, RuntimeException);
    virtual Reference< XSpellAlternatives > SAL_CALL spell( const OUString& rWord, const Locale& rLocale, const PropertyValues& rProperties ) throw(IllegalArgumentException, RuntimeException);

    // XLinguServiceEventBroadcaster
    virtual sal_Bool SAL_CALL addLinguServiceEventListener( const Reference< XLinguServiceEventListener >& rxLstnr ) throw(RuntimeException);
    virtual sal_Bool SAL_CALL removeLinguServiceEventListener( const Reference< XLinguServiceEventListener >& rxLstnr ) throw(RuntimeException);
	
	// XServiceDisplayName
    virtual OUString SAL_CALL getServiceDisplayName( const Locale& rLocale ) throw(RuntimeException);

	// XInitialization
    virtual void SAL_CALL initialize( const Sequence< Any >& rArguments ) throw(Exception, RuntimeException);

	// XComponent
    virtual void SAL_CALL dispose() throw(RuntimeException);
    virtual void SAL_CALL addEventListener( const Reference< XEventListener >& rxListener ) throw(RuntimeException);
    virtual void SAL_CALL removeEventListener( const Reference< XEventListener >& rxListener ) throw(RuntimeException);

	// XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw(RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const OUString& rServiceName ) throw(RuntimeException);
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames() throw(RuntimeException);

#if defined USE_JAVA && defined MACOSX
	// XProofreader
	virtual sal_Bool SAL_CALL isSpellChecker() throw(RuntimeException);
	virtual ProofreadingResult SAL_CALL doProofreading( const OUString& aDocumentIdentifier, const OUString& aText, const Locale& aLocale, sal_Int32 nStartOfSentencePosition, sal_Int32 nSuggestedBehindEndOfSentencePosition, const Sequence< PropertyValue >& aProperties ) throw (IllegalArgumentException, RuntimeException);
	virtual void SAL_CALL ignoreRule( const OUString& aRuleIdentifier, const Locale& aLocale ) throw (IllegalArgumentException, RuntimeException);
	virtual void SAL_CALL resetIgnoreRules() throw(RuntimeException);
#endif	// USE_JAVA && MACOSX


    static inline OUString  getImplementationName_Static() throw();
    static Sequence< OUString > getSupportedServiceNames_Static() throw();
};

inline OUString SpellChecker::getImplementationName_Static() throw()
{
	return A2OU( "org.openoffice.lingu.MySpellSpellChecker" );
}


///////////////////////////////////////////////////////////////////////////

#endif

