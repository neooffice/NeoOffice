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
 *  Edward Peterlin, January 2005
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2005 by Edward Peterlin (OPENSTEP@neooffice.org)
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
#ifndef _ABOUT_HXX
#define _ABOUT_HXX

// include ---------------------------------------------------------------

#ifndef _RESARY_HXX //autogen
#include <vcl/resary.hxx>
#endif
#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#ifndef _ACCEL_HXX //autogen
#include <vcl/accel.hxx>
#endif
#ifndef _LIST_HXX //autogen
#include <tools/list.hxx>
#endif
#ifndef _STDCTRL_HXX //autogen
#include <svtools/stdctrl.hxx>
#endif
#include "basedlgs.hxx"		// SfxModalDialog

DECLARE_LIST( AccelList, Accelerator* )

// class AboutDialog -----------------------------------------------------

class AboutDialog : public SfxModalDialog
{
private:
	OKButton    	aOKButton;
	Image			aAppLogo;

	FixedInfo   	aVersionText;
	FixedInfo   	aCopyrightText;

	ResStringArray	aDeveloperAry;
	String			aDevVersionStr;
	String 			aAccelStr;

	AccelList 		aAccelList;

	AutoTimer       aTimer;
	long            nOff;
	long            nEnd;

	BOOL            bNormal;

protected:
	virtual BOOL 	Close();
	virtual void 	Paint( const Rectangle& );

public:
	AboutDialog( Window* pParent, const ResId& rId, const String& rVerStr );
	~AboutDialog();

#ifdef MACOSX
	// determine if an about dialog is already being displayed
	static BOOL	DialogIsActive();
#endif

	DECL_LINK( TimerHdl, Timer * );
	DECL_LINK( AccelSelectHdl, Accelerator * );
};

#endif // #ifndef _ABOUT_HXX


