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
 * Modified February 2017 by Patrick Luby. NeoOffice is distributed under
 * GPL only under modification term 2 of the LGPL.
 *
 ************************************************************************/

#ifndef _VIEWIMP_HXX
#define _VIEWIMP_HXX

// include ---------------------------------------------------------------

#ifndef __SBX_SBXOBJ_HXX
#include <basic/sbxobj.hxx>
#endif
#include <sfx2/viewsh.hxx>
#include <sfx2/viewfrm.hxx>                  // SvBorder
#include <osl/mutex.hxx>
#include <cppuhelper/interfacecontainer.hxx>

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <svtools/acceleratorexecute.hxx>

// forward ---------------------------------------------------------------

class SfxOfficeDispatch;
class SfxBaseController;

typedef SfxShell* SfxShellPtr_Impl;
SV_DECL_PTRARR( SfxShellArr_Impl, SfxShellPtr_Impl, 4, 4 )

// struct SfxViewShell_Impl ----------------------------------------------

struct SfxViewShell_Impl
{
    ::osl::Mutex                aMutex;
    ::cppu::OInterfaceContainerHelper aInterceptorContainer;
	BOOL						bControllerSet;
	SfxShellArr_Impl			aArr;
	SvBorder                    aBorder;
	Size                        aOptimalSize;
	Size						aMargin;
	USHORT						nPrinterLocks;
	BOOL                        bUseObjectSize;
	BOOL						bCanPrint;
	BOOL                        bHasPrintOptions;
	BOOL						bPlugInsActive;
	BOOL						bIsShowView;
	BOOL                        bFrameSetImpl;
    BOOL                        bOwnsMenu;
    BOOL                        bGotOwnerShip;
    BOOL                        bGotFrameOwnerShip;
	SfxScrollingMode			eScroll;
	USHORT						nFamily;
    SfxBaseController*          pController;
    ::svt::AcceleratorExecute*  pAccExec;
	com::sun::star::uno::Sequence < com::sun::star::beans::PropertyValue > aPrintOpts;
#if defined USE_JAVA && defined MACOSX
	BOOL                        bInDoPrint;
#endif	// USE_JAVA && MACOSX

                                SfxViewShell_Impl()
                                  : aInterceptorContainer( aMutex )
                                  , pAccExec(0)
                                {}
};

#endif

