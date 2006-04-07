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

#ifndef _SV_COM_SUN_STAR_VCL_VCLGRAPHICS_HXX
#define	_SV_COM_SUN_STAR_VCL_VCLGRAPHICS_HXX

#ifndef _SV_JAVA_LANG_OBJECT_HXX
#include <java/lang/Object.hxx>
#endif
#ifndef _SV_SALGTYPE_HXX
#include <salgtype.hxx>
#endif
#ifndef _SV_GEN_HXX
#include <tools/gen.hxx>
#endif

namespace vcl {

class com_sun_star_vcl_VCLBitmap;
class com_sun_star_vcl_VCLFont;
class com_sun_star_vcl_VCLImage;

class com_sun_star_vcl_VCLGraphics : public java_lang_Object
{
protected:
	static jclass		theClass;

public:
	static void			beep();
	static jclass		getMyClass();

						com_sun_star_vcl_VCLGraphics( jobject myObj ) : java_lang_Object( myObj ) {};
	virtual				~com_sun_star_vcl_VCLGraphics() {};

	void				beginSetClipRegion();
	void				copyBits( const com_sun_star_vcl_VCLGraphics *_par0, long _par1, long _par2, long _par3, long _par4, long _par5, long _par6, long _par7, long _par8 );
	com_sun_star_vcl_VCLImage*	createImage();
	void				drawBitmap( const com_sun_star_vcl_VCLBitmap *_par0, long _par1, long _par2, long _par3, long _par4, long _par5, long _par6, long _par7, long _par8 );
	void				drawBitmap( const com_sun_star_vcl_VCLBitmap *_par0, const com_sun_star_vcl_VCLBitmap *_par1, long _par2, long _par3, long _par4, long _par5, long _par6, long _par7, long _par8, long _par9 );
	void				drawEPS( void *_par0, long _par1, long _par2, long _par3, long _par4, long _par5 );
	void				drawGlyphs( long _par0, long _par1, int _par2, long *_par3, long *_par4, com_sun_star_vcl_VCLFont *_par5, SalColor _par6, int _par7, int _par8, long _par9, long _par10 );
	void				drawLine( long _par0, long _par1, long _par2, long _par3, SalColor _par4 );
	void				drawMask( const com_sun_star_vcl_VCLBitmap *_par0, SalColor _par1, long _par2, long _par3, long _par4, long _par5, long _par6, long _par7, long _par8, long _par9 );
	void				drawPolygon( ULONG _par0, const SalPoint *_par1, SalColor _par2, sal_Bool _par3 );
	void				drawPolyline( ULONG _par0, const SalPoint *_par1, SalColor _par2 );
	void				drawPolyPolygon( ULONG _par0, const ULONG *_par1, PCONSTSALPOINT *_par2, SalColor _par3, sal_Bool _par4 );
	void				drawRect( long _par0, long _par1, long _par2, long _par3, SalColor _par4, sal_Bool _par5 );
	void				endSetClipRegion();
	USHORT				getBitCount();
	const Rectangle		getGlyphBounds( int _par0, com_sun_star_vcl_VCLFont *_par1, int _par2 );
	SalColor			getPixel( long _par0, long _par1 );
	const Size			getResolution();
	const Size			getScreenFontResolution();
	void				invert( long _par0, long _par1, long _par2, long _par3, SalInvert _par4 );
	void				invert( ULONG _par0, const SalPoint *_par1, SalInvert _par2 );
	void				resetClipRegion();
	void				resetGraphics();
	void				setPixel( long _par0, long _par1, SalColor _par2 );
	void				setXORMode( sal_Bool _par0 );
	void				unionClipRegion( long _par0, long _par1, long _par2, long _par3 );
#ifdef GENESIS_OF_THE_NEW_WEAPONS
	void				drawPushButton( long _par0, long _par1, long _par2, long _par3, ::rtl::OUString _par4, sal_Bool _par5, sal_Bool _par6, sal_Bool _par7, sal_Bool _par8 );
	USHORT				getPreferredPushButtonWidth( long _par0, long _par1, long _par2, long _par3, ::rtl::OUString _par4 );
	USHORT				getPreferredPushButtonHeight( long _par0, long _par1, long _par2, long _par3, ::rtl::OUString _par4 );
	void				drawRadioButton( long _par0, long _par1, long _par2, long _par3, ::rtl::OUString _par4, sal_Bool _par5, sal_Bool _par6, sal_Bool _par7, long _par8 );
	USHORT				getPreferredRadioButtonWidth( long _par0, long _par1, long _par2, long _par3, ::rtl::OUString _par4 );
	USHORT				getPreferredRadioButtonHeight( long _par0, long _par1, long _par2, long _par3, ::rtl::OUString _par4 );
#endif
};

} // namespace vcl

#endif // _SV_COM_SUN_STAR_VCL_VCLGRAPHICS_HXX
