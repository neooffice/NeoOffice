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

#ifndef _SV_JAVA_LANG_CLASS_HXX
#define _SV_JAVA_LANG_CLASS_HXX

#ifndef _SV_JAVA_LANG_OBJECT_HXX
#include <java/lang/Object.hxx>
#endif

namespace vcl {

class java_lang_Class : public java_lang_Object
{
protected:
	static jclass		theClass;

public:
	static jclass		getMyClass();
	static java_lang_Class* forName( const ::rtl::OUString &_par0 );

						java_lang_Class( jobject myObj ) : java_lang_Object( myObj ) {}
	virtual				~java_lang_Class() {};

	sal_Bool			isAssignableFrom( java_lang_Class * _par0 );
	java_lang_Object*	newInstance();
	::rtl::OUString		getName();
	jobject				newInstanceObject();
};

} // namespace vcl

#endif // _SV_JAVA_LANG_CLASS_HXX
