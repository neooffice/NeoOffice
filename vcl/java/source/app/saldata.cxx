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

#define _SV_SALDATA_CXX

#ifndef _SV_SALDATA_HXX
#include <saldata.hxx>
#endif
#ifndef _SV_SALGDI_H
#include <salgdi.h>
#endif
#ifndef _SV_OUTFONT_HXX
#include <outfont.hxx>
#endif
#ifndef _SV_COM_SUN_STAR_VCL_VCLEVENT_HXX
#include <com/sun/star/vcl/VCLEvent.hxx>
#endif
#ifndef _SV_COM_SUN_STAR_VCL_VCLGRAPHICS_HXX
#include <com/sun/star/vcl/VCLGraphics.hxx>
#endif

using namespace rtl;
using namespace vcl;

// ========================================================================

SalData::SalData()
{
	mpFirstInstance	= NULL;
	mpFocusFrame = NULL;
	mnTimerInterval = 0;
	maTimeout.tv_sec = 0;
	maTimeout.tv_usec = 0;
	mpEventQueue = NULL;
	mpPresentationFrame = NULL;
	mbInNativeModalSheet = false;
	mpNativeModalSheetFrame = NULL;
	mpLastDragFrame = NULL;
	mbInSignalHandler = false;
	mbDoubleScrollbarArrows = false;
	mpCaptureFrame = NULL;

	// Set condition so that they don't block
	maNativeEventCondition.set();
	maLastPointerState.mnState = 0;
}

// ------------------------------------------------------------------------

SalData::~SalData()
{
	if ( mpEventQueue )
		delete mpEventQueue;

	for ( ::std::map< String, JavaImplFontData* >::const_iterator it = maFontNameMapping.begin(); it != maFontNameMapping.end(); ++it )
		delete it->second;

	while ( maPendingDocumentEventsList.size() )
	{
		delete maPendingDocumentEventsList.front();
		maPendingDocumentEventsList.pop_front();
	}
}
