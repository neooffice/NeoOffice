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

#ifndef _SV_COM_SUN_STAR_VCL_VCLMENU_HXX
#define _SV_COM_SUN_STAR_VCL_VCLMENU_HXX

#ifndef _SV_JAVA_LANG_OBJECT_HXX
#include <java/lang/Object.hxx>
#endif
#ifndef _SV_GEN_HXX
#include <tools/gen.hxx>
#endif

namespace vcl {

class com_sun_star_vcl_VCLMenuItemData;

class com_sun_star_vcl_VCLMenu : public java_lang_Object
{
protected:
        static jclass				theClass;

public:
        static jclass				getMyClass();
        
                                                com_sun_star_vcl_VCLMenu( jobject myObj ) : java_lang_Object( myObj ) {};
                                
                                                com_sun_star_vcl_VCLMenu( );
        virtual					~com_sun_star_vcl_VCLMenu() { };
	
	void					dispose( );
        
        com_sun_star_vcl_VCLMenuItemData *	getMenuItemDataObject();
        
        void		insertItem(com_sun_star_vcl_VCLMenuItemData *_par0, int _par1);
        void		removeItem(int _par0);
        void		checkItem(int _par0, bool _par1);
        void		enableItem(int _par0, bool _par1);
        void		attachSubmenu(com_sun_star_vcl_VCLMenuItemData *_par0, int par1);
};

} // namespace vcl

#endif // _SV_COM_SUN_STAR_VCL_VCLMENU_HXX