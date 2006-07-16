
#ifndef _JAVA_FOLDERPICKER_HXX_
#define _JAVA_FOLDERPICKER_HXX_

#ifndef _CPPUHELPER_COMPBASE3_HXX_
#include <cppuhelper/compbase3.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_UI_XFOLDERPICKER_HPP_
#include <com/sun/star/ui/dialogs/XFolderPicker.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XCANCELLABLE_HPP_
#include <com/sun/star/util/XCancellable.hpp>
#endif
#ifndef _RTL_USTRING_H_
#include <rtl/ustring.hxx>
#endif

class JavaFolderPicker : public ::cppu::WeakComponentImplHelper3< ::com::sun::star::ui::dialogs::XFolderPicker, ::com::sun::star::lang::XServiceInfo, ::com::sun::star::util::XCancellable >
{
	::osl::Mutex		maMutex;

public:
						JavaFolderPicker( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceMgr );
	virtual				~JavaFolderPicker();

	virtual void				SAL_CALL setTitle( const ::rtl::OUString& aTitle ) throw( ::com::sun::star::uno::RuntimeException );
	virtual sal_Int16			SAL_CALL execute() throw( ::com::sun::star::uno::RuntimeException );
	virtual void				SAL_CALL setDisplayDirectory( const ::rtl::OUString& rDirectory ) throw( com::sun::star::lang::IllegalArgumentException, com::sun::star::uno::RuntimeException );
	virtual ::rtl::OUString		SAL_CALL getDisplayDirectory() throw( com::sun::star::uno::RuntimeException );
	virtual ::rtl::OUString		SAL_CALL getDirectory() throw( com::sun::star::uno::RuntimeException );
	virtual void				SAL_CALL setDescription( const ::rtl::OUString& rDescription ) throw( com::sun::star::uno::RuntimeException );
	virtual ::rtl::OUString		SAL_CALL getImplementationName() throw( ::com::sun::star::uno::RuntimeException );
	virtual sal_Bool			SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw( ::com::sun::star::uno::RuntimeException );
	virtual ::com::sun::star::uno::Sequence< ::rtl::OUString >	SAL_CALL getSupportedServiceNames() throw( ::com::sun::star::uno::RuntimeException );
	virtual void				SAL_CALL cancel() throw( ::com::sun::star::uno::RuntimeException );
	virtual void				SAL_CALL disposing( const ::com::sun::star::lang::EventObject& aEvent ) throw( ::com::sun::star::uno::RuntimeException );
}; 

#endif	// _JAVA_FOLDERPICKER_HXX_
