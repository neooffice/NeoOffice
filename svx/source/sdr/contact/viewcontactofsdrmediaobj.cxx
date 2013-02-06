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
 * Modified February 2013 by Patrick Luby. NeoOffice is distributed under
 * GPL only under modification term 2 of the LGPL.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#include <svx/sdr/contact/viewcontactofsdrmediaobj.hxx>
#include <svx/svdomedia.hxx>
#include <svx/sdr/contact/viewobjectcontactofsdrmediaobj.hxx>
#include <drawinglayer/primitive2d/mediaprimitive2d.hxx>

namespace sdr { namespace contact {

// ----------------------------
// - ViewContactOfSdrMediaObj -
// ----------------------------

ViewContactOfSdrMediaObj::ViewContactOfSdrMediaObj( SdrMediaObj& rMediaObj ) :
	ViewContactOfSdrObj( rMediaObj )
{
}

// ------------------------------------------------------------------------------

ViewContactOfSdrMediaObj::~ViewContactOfSdrMediaObj()
{
}

// ------------------------------------------------------------------------------

ViewObjectContact& ViewContactOfSdrMediaObj::CreateObjectSpecificViewObjectContact(ObjectContact& rObjectContact)
{
	return *( new ViewObjectContactOfSdrMediaObj( rObjectContact, *this, static_cast< SdrMediaObj& >( GetSdrObject() ).getMediaProperties() ) );
}

// ------------------------------------------------------------------------------

bool ViewContactOfSdrMediaObj::hasPreferredSize() const
{
	// #i71805# Since we may have a whole bunch of VOCs here, make a loop
	// return true if all have their preferred size
	const sal_uInt32 nCount(getViewObjectContactCount());
	bool bRetval(true);

	for(sal_uInt32 a(0); bRetval && a < nCount; a++)
	{
		ViewObjectContact* pCandidate = getViewObjectContact(a);

		if(pCandidate && !static_cast< ViewObjectContactOfSdrMediaObj* >(pCandidate)->hasPreferredSize())
		{
			bRetval = false;
		}
	}

	return bRetval;
}

// ------------------------------------------------------------------------------

Size ViewContactOfSdrMediaObj::getPreferredSize() const
{
	// #i71805# Since we may have a whole bunch of VOCs here, make a loop
	// return first useful size -> the size from the first which is visualized as a window
	const sal_uInt32 nCount(getViewObjectContactCount());

	for(sal_uInt32 a(0); a < nCount; a++)
	{
		ViewObjectContact* pCandidate = getViewObjectContact(a);
		Size aSize(pCandidate ? static_cast< ViewObjectContactOfSdrMediaObj* >(pCandidate)->getPreferredSize() : Size());
		
		if(0 != aSize.getWidth() || 0 != aSize.getHeight())
		{
			return aSize;
		}
	}

	return Size();
}

// ------------------------------------------------------------------------------
		
void ViewContactOfSdrMediaObj::updateMediaItem( ::avmedia::MediaItem& rItem ) const
{
	// #i71805# Since we may have a whole bunch of VOCs here, make a loop
	const sal_uInt32 nCount(getViewObjectContactCount());

	for(sal_uInt32 a(0); a < nCount; a++)
	{
		ViewObjectContact* pCandidate = getViewObjectContact(a);

		if(pCandidate)
		{
			static_cast< ViewObjectContactOfSdrMediaObj* >(pCandidate)->updateMediaItem(rItem);
		}
	}
}

// ------------------------------------------------------------------------------
			
void ViewContactOfSdrMediaObj::executeMediaItem( const ::avmedia::MediaItem& rItem )
{
	const sal_uInt32 nCount(getViewObjectContactCount());

	for(sal_uInt32 a(0); a < nCount; a++)
	{
		ViewObjectContact* pCandidate = getViewObjectContact(a);

		if(pCandidate)
		{
			static_cast< ViewObjectContactOfSdrMediaObj* >(pCandidate)->executeMediaItem(rItem);
		}
	}
}

// ------------------------------------------------------------------------------

void ViewContactOfSdrMediaObj::mediaPropertiesChanged( const ::avmedia::MediaItem& rNewState )
{
	static_cast< SdrMediaObj& >(GetSdrObject()).mediaPropertiesChanged(rNewState);
}

}} // end of namespace sdr::contact

namespace sdr
{
	namespace contact
	{
		drawinglayer::primitive2d::Primitive2DSequence ViewContactOfSdrMediaObj::createViewIndependentPrimitive2DSequence() const
		{
			// create range using the model data directly. This is in SdrTextObj::aRect which i will access using
            // GetGeoRect() to not trigger any calculations. It's the unrotated geometry which is okay for MediaObjects ATM.
			const Rectangle& rRectangle(GetSdrMediaObj().GetGeoRect());
			const basegfx::B2DRange aRange(rRectangle.Left(), rRectangle.Top(), rRectangle.Right(), rRectangle.Bottom());

			// create object transform
			basegfx::B2DHomMatrix aTransform;
			aTransform.set(0, 0, aRange.getWidth());
			aTransform.set(1, 1, aRange.getHeight());
			aTransform.set(0, 2, aRange.getMinX());
			aTransform.set(1, 2, aRange.getMinY());

			// create media primitive
#if defined USE_JAVA && defined MACOSX
			const basegfx::BColor aBackgroundColor(1.0f, 1.0f, 1.0f);
#else	// USE_JAVA && MACOSX
			const basegfx::BColor aBackgroundColor(67.0 / 255.0, 67.0 / 255.0, 67.0 / 255.0);
#endif	// USE_JAVA && MACOSX
			const rtl::OUString& rURL(GetSdrMediaObj().getURL());
			const sal_uInt32 nPixelBorder(4L);
			const drawinglayer::primitive2d::Primitive2DReference xRetval(new drawinglayer::primitive2d::MediaPrimitive2D(
				aTransform, rURL, aBackgroundColor, nPixelBorder));

			return drawinglayer::primitive2d::Primitive2DSequence(&xRetval, 1);
		}
	} // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// eof
