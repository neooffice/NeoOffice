/*************************************************************************
 *
 *  $RCSfile$
 *
 *  $Revision$
 *
 *  last change: $Author$ $Date$
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU General Public License Version 2.1
 *
 *  Patrick Luby, June 2003
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2003 by Patrick Luby (patrick.luby@planamesa.com)
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _SV_JAVA_LANG_OBJECT_HXX
#define	_SV_JAVA_LANG_OBJECT_HXX

#ifndef _SV_JAVA_TOOLS_HXX
#include <java/tools.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _SALJAVA_H
#include <saljava.h>
#endif

namespace vcl {

class VCLThreadAttach
{
protected:
	void				AttachThread();
	void				DetachThread();
	sal_Bool			StartJava();
	void				ThrowException();

public:
	JNIEnv*				pEnv;

						VCLThreadAttach();
						~VCLThreadAttach();
};

class java_lang_Class;

class java_lang_Object
{

						java_lang_Object& operator = (java_lang_Object&) { return *this;};
						java_lang_Object(java_lang_Object&) {};

protected:
	static jclass		theClass;

	jobject				object;

public:
	static jclass		getMyClass();

						java_lang_Object( jobject myObj );
	virtual				~java_lang_Object();

	void				saveRef( jobject myObj );
	jobject				getJavaObject() const { return object; }
	java_lang_Object*	getWrapper() { return this; }

	java_lang_Class*	getClass();

	::rtl::OUString		toString();
};

} // namespace vcl
#endif //_SV_JAVA_LANG_OBJECT_HXX
