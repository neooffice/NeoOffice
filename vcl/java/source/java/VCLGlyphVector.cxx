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
 *  Patrick Luby, June 2004
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2004 by Patrick Luby (patrick.luby@planamesa.com)
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

#define _SV_COM_SUN_STAR_VCL_VCLGLYPHVECTOR_CXX

#ifndef _SV_COM_SUN_STAR_VCL_VCLGLYPHVECTOR_HXX
#include <com/sun/star/vcl/VCLGlyphVector.hxx>
#endif
#ifndef _SV_COM_SUN_STAR_VCL_VCLFONT_HXX
#include <com/sun/star/vcl/VCLFont.hxx>
#endif
#ifndef _SV_COM_SUN_STAR_VCL_VCLGRAPHICS_HXX
#include <com/sun/star/vcl/VCLGraphics.hxx>
#endif

using namespace vcl;

// ============================================================================

jclass com_sun_star_vcl_VCLGlyphVector::theClass = NULL;

// ----------------------------------------------------------------------------

jclass com_sun_star_vcl_VCLGlyphVector::getMyClass()
{
	if ( !theClass )
	{
		VCLThreadAttach t;
		if ( !t.pEnv ) return (jclass)NULL;
		jclass tempClass = t.pEnv->FindClass( "com/sun/star/vcl/VCLGlyphVector" );
		OSL_ENSURE( tempClass, "Java : FindClass not found!" );
		theClass = (jclass)t.pEnv->NewGlobalRef( tempClass );
	}
	return theClass;
}

// ----------------------------------------------------------------------------

com_sun_star_vcl_VCLGlyphVector::com_sun_star_vcl_VCLGlyphVector( com_sun_star_vcl_VCLGraphics *pGraphics, com_sun_star_vcl_VCLFont *pFont ) : java_lang_Object( (jobject)NULL )
{
	static jmethodID mID = NULL;
	VCLThreadAttach t;
	if ( !t.pEnv )
		return;
	if ( !mID )
	{
		char *cSignature = "(Lcom/sun/star/vcl/VCLGraphics;Lcom/sun/star/vcl/VCLFont;)V";
		mID = t.pEnv->GetMethodID( getMyClass(), "<init>", cSignature );
	}
	OSL_ENSURE( mID, "Unknown method id!" );
	jvalue args[2];
	args[0].l = pGraphics->getJavaObject();
	args[1].l = pFont->getJavaObject();
	jobject tempObj;
	tempObj = t.pEnv->NewObjectA( getMyClass(), mID, args );
	saveRef( tempObj );
}

// ----------------------------------------------------------------------------

void com_sun_star_vcl_VCLGlyphVector::layoutText( ImplLayoutArgs& rArgs )
{
	static jmethodID mID = NULL;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "([C)V";
			mID = t.pEnv->GetMethodID( getMyClass(), "layoutText", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			jsize elements( rArgs.mnEndCharPos - rArgs.mnMinCharPos );
			jcharArray chars = t.pEnv->NewCharArray( elements );
			t.pEnv->SetCharArrayRegion( chars, 0, elements, (jchar *)rArgs.mpStr + rArgs.mnMinCharPos );
			jvalue args[1];
			args[0].l = chars;
			t.pEnv->CallNonvirtualVoidMethodA( object, getMyClass(), mID, args );
		}
	}
}

// ----------------------------------------------------------------------------

void com_sun_star_vcl_VCLGlyphVector::drawText( long _par0, long _par1, int _par2, SalColor _par3 )
{
	static jmethodID mID = NULL;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "(IIII)V";
			mID = t.pEnv->GetMethodID( getMyClass(), "drawText", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			jvalue args[4];
			args[0].i = jint( _par0 );
			args[1].i = jint( _par1 );
			args[2].i = jint( _par2 );
			args[3].i = jint( _par3 );
			t.pEnv->CallNonvirtualVoidMethodA( object, getMyClass(), mID, args );
		}
	}
}
// ----------------------------------------------------------------------------

long com_sun_star_vcl_VCLGlyphVector::fillDXArray( long *_par0 )
{
	static jmethodID mID = NULL;
	long out = 0;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "()[I";
			mID = t.pEnv->GetMethodID( getMyClass(), "fillDXArray", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			jintArray tempObj = (jintArray)t.pEnv->CallNonvirtualObjectMethod( object, getMyClass(), mID );
			if ( tempObj )
			{
				jsize nElements = t.pEnv->GetArrayLength( tempObj );
				if ( nElements )
				{
					long nSize = nElements * sizeof( jint );
					jboolean bCopy( sal_False );
					jint *pPosBits = (jint *)t.pEnv->GetPrimitiveArrayCritical( tempObj, &bCopy );
					int i;
					for ( i = 0; i < nElements; i++)
						out += (long)pPosBits[ i ];
					if ( _par0 )
						memcpy( _par0, pPosBits, nSize );
					t.pEnv->ReleasePrimitiveArrayCritical( tempObj, (void *)pPosBits, JNI_ABORT );
				}
			}
		}
	}
	return out;
}
