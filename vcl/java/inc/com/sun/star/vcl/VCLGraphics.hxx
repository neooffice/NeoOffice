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

class com_sun_star_vcl_VCLFont;
class com_sun_star_vcl_VCLBitmap;
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

	void				addToFlush( long _par0, long _par1, long _par2, long _par3 );
	void				beginSetClipRegion();
	void				copyBits( const com_sun_star_vcl_VCLGraphics *_par0, long _par1, long _par2, long _par3, long _par4, long _par5, long _par6, long _par7, long _par8 );
	void				drawBitmap( const com_sun_star_vcl_VCLBitmap *_par0, long _par1, long _par2, long _par3, long _par4, long _par5, long _par6, long _par7, long _par8 );
	void				drawBitmap( const com_sun_star_vcl_VCLBitmap *_par0, const com_sun_star_vcl_VCLBitmap *_par1, long _par2, long _par3, long _par4, long _par5, long _par6, long _par7, long _par8, long _par9 );
	void				drawLine( long _par0, long _par1, long _par2, long _par3, SalColor _par4 );
	void				drawMask( const com_sun_star_vcl_VCLBitmap *_par0, SalColor _par1, long _par2, long _par3, long _par4, long _par5, long _par6, long _par7, long _par8, long _par9 );
	void				drawPolygon( ULONG _par0, const long *_par1, const long *_par2, SalColor _par3, sal_Bool _par4 );
	void				drawPolyline( ULONG _par0, const long *_par1, const long *_par2, SalColor _par3 );
	void				drawPolyPolygon( ULONG _par0, const ULONG *_par1, long **_par2, long **_par3, SalColor _par4, sal_Bool _par5 );
	void				drawRect( long _par0, long _par1, long _par2, long _par3, SalColor _par4, sal_Bool _par5 );
	void				drawTextLayout( void *_par0, SalColor _par1 );
	void				endSetClipRegion();
	USHORT				getBitCount();
	const Size			getGlyphSize( const sal_Unicode _par0, com_sun_star_vcl_VCLFont *_par1 );
	com_sun_star_vcl_VCLImage *getImage();
	java_lang_Object*	getGraphics();
	void*				getNativeGraphics();
	SalColor			getPixel( long _par0, long _par1 );
	const Size			getResolution();
	const Size			getScreenFontResolution();
	void				invert( long _par0, long _par1, long _par2, long _par3, SalInvert _par4 );
	void				invert( ULONG _par0, const long *_par1, const long *_par2, SalInvert _par3 );
	void				releaseNativeGraphics( void *_par0 );
	void				resetClipRegion();
	void				resetGraphics();
	void				setLineAntialiasing( sal_Bool _par0 );
	void				setPixel( long _par0, long _par1, SalColor _par2 );
	void				setXORMode( sal_Bool _par0 );
	void				unionClipRegion( long _par0, long _par1, long _par2, long _par3 );
};

} // namespace vcl

#endif // _SV_COM_SUN_STAR_VCL_VCLGRAPHICS_HXX
