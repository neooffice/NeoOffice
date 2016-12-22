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
 *		 - GNU General Public License Version 2.1
 *
 *  Patrick Luby, February 2006
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2006 Planamesa Inc.
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

#ifndef _SV_SALSYS_H
#define _SV_SALSYS_H

#include "salsys.hxx"

// -----------------
// - JavaSalSystem -
// -----------------

class JavaSalSystem : public SalSystem
{
public:
							JavaSalSystem();
	virtual					~JavaSalSystem();

	virtual unsigned int	GetDisplayScreenCount() SAL_OVERRIDE;
	virtual bool			IsUnifiedDisplay() SAL_OVERRIDE { return false; }
	virtual unsigned int	GetDisplayBuiltInScreen() SAL_OVERRIDE;
	virtual Rectangle		GetDisplayScreenPosSizePixel( unsigned int nScreen ) SAL_OVERRIDE;
	virtual OUString		GetDisplayScreenName( unsigned int nScreen ) SAL_OVERRIDE;
	virtual int				ShowNativeMessageBox( const OUString& rTitle, const OUString& rMessage, int nButtonCombination, int nDefaultButton, bool bUseResources ) SAL_OVERRIDE;
};

#endif // _SV_SALSYS_H
