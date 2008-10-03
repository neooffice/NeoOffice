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

#ifndef _SV_JAVA_LANG_OBJECT_HXX
#include <java/lang/Object.hxx>
#endif
#ifndef _SV_JOBSET_H
#include <vcl/jobset.h>
#endif

namespace vcl {

class com_sun_star_vcl_VCLGraphics;
class com_sun_star_vcl_VCLPageFormat;

class com_sun_star_vcl_VCLPrintJob : public java_lang_Object
{
protected:
	static jclass		theClass;

public:
	static jclass		getMyClass();

						com_sun_star_vcl_VCLPrintJob( jobject myObj ) : java_lang_Object( myObj ) {}
						com_sun_star_vcl_VCLPrintJob();
	virtual				~com_sun_star_vcl_VCLPrintJob() {};

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
