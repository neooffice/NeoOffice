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
 *  Copyright 2003 Planamesa Inc.
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

#ifndef _SV_COM_SUN_STAR_VCL_VCLPRINTJOB_HXX
#define	_SV_COM_SUN_STAR_VCL_VCLPRINTJOB_HXX

#include <java/lang/Object.hxx>
#include <sal/types.h>
#include <osl/mutex.h>
#include <vcl/jobset.h>

// Uncomment the following line to use native printing APIs
// #define USE_NATIVE_PRINTING

namespace vcl {

class com_sun_star_vcl_VCLGraphics;
class com_sun_star_vcl_VCLPageFormat;

class SAL_DLLPRIVATE com_sun_star_vcl_VCLPrintJob : public java_lang_Object
{
#ifdef USE_NATIVE_PRINTING
	::osl::Mutex		maMutex;
	void*				mpPrintPanel;
#endif	// USE_NATIVE_PRINTING

protected:
	static jclass		theClass;

public:
	static jclass		getMyClass();

#ifndef USE_NATIVE_PRINTING
						com_sun_star_vcl_VCLPrintJob( jobject myObj ) : java_lang_Object( myObj ) {}
#endif	// !USE_NATIVE_PRINTING
						com_sun_star_vcl_VCLPrintJob();
	virtual				~com_sun_star_vcl_VCLPrintJob();

	void				abortJob();
	void				dispose();
	void				endJob();
	void				endPage();
	XubString			getPageRange( com_sun_star_vcl_VCLPageFormat *_par0 );
	sal_Bool			isFinished();
	sal_Bool			startJob( com_sun_star_vcl_VCLPageFormat *_par0, ::rtl::OUString _par1, float _par2, sal_Bool _par3 );
	com_sun_star_vcl_VCLGraphics*	startPage( Orientation _par0 );
};

} // namespace vcl

#endif // _SV_COM_SUN_STAR_VCL_VCLPRINTJOB_HXX
