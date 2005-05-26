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

#ifndef _SV_SALPRN_H
#define _SV_SALPRN_H

#ifndef _SV_SV_H
#include <sv.h>
#endif
#ifndef _SV_SALGDI_HXX
#include <salgdi.hxx>
#endif
#ifndef _SV_PRNTYPES_HXX
#include <prntypes.hxx>
#endif
#ifndef _SV_COM_SUN_STAR_VCL_VCLPAGEFORMAT_HXX
#include <com/sun/star/vcl/VCLPageFormat.hxx>
#endif

namespace vcl
{
class com_sun_star_vcl_VCLPrintJob;
}

// ----------------------
// - SalInfoPrinterData -
// ----------------------

class SalInfoPrinterData
{
	friend class	SalInfoPrinter;
	friend class	SalInstance;
	friend class	SalPrinter;

					SalInfoPrinterData();
					~SalInfoPrinterData();

	SalGraphics*	mpGraphics;
	BOOL			mbGraphics;
	::vcl::com_sun_star_vcl_VCLPageFormat*	mpVCLPageFormat;
};

// ------------------
// - SalPrinterData -
// ------------------

class SalPrinterData
{
	friend class	SalGraphics;
	friend class	SalPrinter;

					SalPrinterData();
					~SalPrinterData();

	BOOL			mbStarted;
	SalGraphics*	mpGraphics;
	BOOL			mbGraphics;
	::vcl::com_sun_star_vcl_VCLPrintJob*	mpVCLPrintJob;
	::vcl::com_sun_star_vcl_VCLPageFormat*	mpVCLPageFormat;
};

// ------------------
// - SalPrinterData -
// ------------------

class SalDriverData
{
	friend class	ImplJobSetup;
	friend class	JobSetup;
	friend class	SalInfoPrinter;
	friend class	SalInstance;
	friend class	SalPrinter;

	::vcl::com_sun_star_vcl_VCLPageFormat*	mpVCLPageFormat;

public:
					SalDriverData() : mpVCLPageFormat( NULL ) {}
					SalDriverData( SalDriverData *pData );
					~SalDriverData();
};

#endif // _SV_SALPRN_H
