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

#ifndef _JAVA_DTRANS_JAVA_LANG_THROWABLE_HXX
#define _JAVA_DTRANS_JAVA_LANG_THROWABLE_HXX

#ifndef _JAVA_DTRANS_JAVA_LANG_OBJECT_HXX
#include <java/lang/Object.hxx>
#endif

namespace java {

namespace dtrans {

class java_lang_Throwable : public java_lang_Object
{
protected:
	static jclass		theClass;

public:
	static jclass		getMyClass();

						java_lang_Throwable( jobject myObj ) : java_lang_Object( myObj ) {}
	virtual				~java_lang_Throwable() {};

	::rtl::OUString		getMessage() const;
	::rtl::OUString		getLocalizedMessage() const;
	::rtl::OUString		toString() const;
};

} // namespace dtrans

} // namespace java

#endif // _JAVA_DTRANS_JAVA_LANG_THROWABLE_HXX
