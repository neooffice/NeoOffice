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

#define _SV_COM_SUN_STAR_VCL_VCLTEXTLAYOUT_CXX

#ifndef _SV_COM_SUN_STAR_VCL_VCLTEXTLAYOUT_HXX
#include <com/sun/star/vcl/VCLTextLayout.hxx>
#endif
#ifndef _SV_COM_SUN_STAR_VCL_VCLFONT_HXX
#include <com/sun/star/vcl/VCLFont.hxx>
#endif
#ifndef _SV_COM_SUN_STAR_VCL_VCLGRAPHICS_HXX
#include <com/sun/star/vcl/VCLGraphics.hxx>
#endif

#include <unicode/ushape.h>

using namespace rtl;
using namespace vcl;

// ============================================================================

jclass com_sun_star_vcl_VCLTextLayout::theClass = NULL;

// ----------------------------------------------------------------------------

jclass com_sun_star_vcl_VCLTextLayout::getMyClass()
{
	if ( !theClass )
	{
		VCLThreadAttach t;
		if ( !t.pEnv ) return (jclass)NULL;
		jclass tempClass = t.pEnv->FindClass( "com/sun/star/vcl/VCLTextLayout" );
		OSL_ENSURE( tempClass, "Java : FindClass not found!" );
		theClass = (jclass)t.pEnv->NewGlobalRef( tempClass );
	}
	return theClass;
}

// ----------------------------------------------------------------------------

com_sun_star_vcl_VCLTextLayout::com_sun_star_vcl_VCLTextLayout( com_sun_star_vcl_VCLGraphics *pGraphics, com_sun_star_vcl_VCLFont *pFont ) : java_lang_Object( (jobject)NULL )
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

void com_sun_star_vcl_VCLTextLayout::drawText( long _par0, long _par1, SalColor _par2 )
{
	static jmethodID mID = NULL;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "(III)V";
			mID = t.pEnv->GetMethodID( getMyClass(), "drawText", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			jvalue args[3];
			args[0].i = jint( _par0 );
			args[1].i = jint( _par1 );
			args[2].i = jint( _par2 );
			t.pEnv->CallNonvirtualVoidMethodA( object, getMyClass(), mID, args );
		}
	}
}

// ----------------------------------------------------------------------------

long com_sun_star_vcl_VCLTextLayout::fillDXArray( long *_par0 )
{
	static jmethodID mIDGetWidth = NULL;
	long out = 0;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mIDGetWidth )
		{
			char *cSignature = "()I";
			mIDGetWidth = t.pEnv->GetMethodID( getMyClass(), "getWidth", cSignature );
		}
		OSL_ENSURE( mIDGetWidth, "Unknown method id!" );
		if ( mIDGetWidth )
		{
			out = (long)t.pEnv->CallNonvirtualIntMethod( object, getMyClass(), mIDGetWidth );
			if ( out && _par0 )
			{
				static jmethodID mIDGetDXArray = NULL;
				if ( !mIDGetDXArray )
				{
					char *cSignature = "()[I";
					mIDGetDXArray = t.pEnv->GetMethodID( getMyClass(), "getDXArray", cSignature );
				}
				OSL_ENSURE( mIDGetDXArray, "Unknown method id!" );
				if ( mIDGetDXArray )
				{
					BOOL bDXArray = FALSE;
					jintArray tempObj = (jintArray)t.pEnv->CallNonvirtualObjectMethod( object, getMyClass(), mIDGetDXArray );
					if ( tempObj )
					{
						jsize nElements = t.pEnv->GetArrayLength( tempObj );
						if ( nElements )
						{
							bDXArray = TRUE;	
							long nSize = nElements * sizeof( jint );
							jboolean bCopy( sal_False );
							jint *pPosBits = (jint *)t.pEnv->GetPrimitiveArrayCritical( tempObj, &bCopy );
							memcpy( _par0, pPosBits, nSize );
							t.pEnv->ReleasePrimitiveArrayCritical( tempObj, (void *)pPosBits, JNI_ABORT );
						}
					}

					if ( !bDXArray )
						out = 0;
				}
			}
		}
	}
	return out;
}

// ----------------------------------------------------------------------------

bool com_sun_star_vcl_VCLTextLayout::getBounds( Rectangle& _par0 )
{
	static jmethodID mID = NULL;
	static jfieldID fIDX = NULL;
	static jfieldID fIDY = NULL;
	static jfieldID fIDWidth = NULL;
	static jfieldID fIDHeight = NULL;
	bool out = false;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "()Ljava/awt/Rectangle;";
			mID = t.pEnv->GetMethodID( getMyClass(), "getBounds", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			jobject tempObj = t.pEnv->CallNonvirtualObjectMethod( object, getMyClass(), mID );
			if ( tempObj )
			{
				jclass tempObjClass = t.pEnv->GetObjectClass( tempObj );
				if ( !fIDX )
				{
					char *cSignature = "I";
					fIDX = t.pEnv->GetFieldID( tempObjClass, "x", cSignature );
				}
				OSL_ENSURE( fIDX, "Unknown field id!" );
				if ( !fIDY )
				{
					char *cSignature = "I";
					fIDY  = t.pEnv->GetFieldID( tempObjClass, "y", cSignature );
				}
				OSL_ENSURE( fIDY, "Unknown field id!" );
				if ( !fIDWidth )
				{
					char *cSignature = "I";
					fIDWidth = t.pEnv->GetFieldID( tempObjClass, "width", cSignature );
				}
				OSL_ENSURE( fIDWidth, "Unknown field id!" );
				if ( !fIDHeight )
				{
					char *cSignature = "I";
					fIDHeight = t.pEnv->GetFieldID( tempObjClass, "height", cSignature );
				}
				OSL_ENSURE( fIDHeight, "Unknown field id!" );
				if ( fIDX && fIDY && fIDWidth && fIDHeight )
				{
					_par0 = Rectangle( Point( (long)t.pEnv->GetIntField( tempObj, fIDX ), (long)t.pEnv->GetIntField( tempObj, fIDY ) ), Size( (long)t.pEnv->GetIntField( tempObj, fIDWidth ), (long)t.pEnv->GetIntField( tempObj, fIDHeight ) ) );
					out = true;
				}
			}
		}
	}
	return out;
}

// ----------------------------------------------------------------------------

void com_sun_star_vcl_VCLTextLayout::getCaretPositions( int _par0, long *_par1 )
{
	static jmethodID mID = NULL;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "()[I";
			mID = t.pEnv->GetMethodID( getMyClass(), "getCaretPositions", cSignature );
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
					jboolean bCopy( sal_False );
					jint *pPosBits = (jint *)t.pEnv->GetPrimitiveArrayCritical( tempObj, &bCopy );
					if ( _par1 )
						memcpy( _par1, pPosBits, _par0 * sizeof( jint* ) );
					t.pEnv->ReleasePrimitiveArrayCritical( tempObj, (void *)pPosBits, JNI_ABORT );
				}
			}
		}
	}
}

// ----------------------------------------------------------------------------

int com_sun_star_vcl_VCLTextLayout::getTextBreak( long _par0, long _par1, int _par2 )
{
	static jmethodID mID = NULL;
	int out = 0;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "(III)I";
			mID = t.pEnv->GetMethodID( getMyClass(), "getTextBreak", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			jvalue args[3];
			args[0].i = jint( _par0 );
			args[1].i = jint( _par1 );
			args[2].i = jint( _par2 );
			out = (int)t.pEnv->CallNonvirtualIntMethodA( object, getMyClass(), mID, args );
		}
	}
	return out;
}

// ----------------------------------------------------------------------------

void com_sun_star_vcl_VCLTextLayout::justify( long _par0 )
{
	static jmethodID mID = NULL;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "(I)V";
			mID = t.pEnv->GetMethodID( getMyClass(), "justify", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			jvalue args[1];
			args[0].i = jint( _par0 );
			t.pEnv->CallNonvirtualVoidMethodA( object, getMyClass(), mID, args );
		}
	}
}

// ----------------------------------------------------------------------------

void com_sun_star_vcl_VCLTextLayout::layoutText( ImplLayoutArgs& _par0 )
{
	static jmethodID mID = NULL;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "([CII[I)V";
			mID = t.pEnv->GetMethodID( getMyClass(), "layoutText", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			int nElements = 0;
			bool bVertical = ( ( _par0.mnFlags & SAL_LAYOUT_VERTICAL ) != 0 );
			bool bRTL;
			int i, j;

			// Calculate number of characters
			_par0.ResetPos();
			while ( _par0.GetNextRun( &i, &j, &bRTL ) )
			{
				int nRunLen = j - i;
				if ( bRTL && !bVertical )
				{
					// Calculate number of Arabic characters
					UErrorCode aErr = U_ZERO_ERROR;
					nElements += u_shapeArabic( _par0.mpStr + i, nRunLen, NULL, 0, U_SHAPE_LETTERS_SHAPE, &aErr );
				}
				else
				{
					nElements += nRunLen;
				}
			}

			jcharArray chars = t.pEnv->NewCharArray( nElements );
			jintArray flags = t.pEnv->NewIntArray( nElements );

			jboolean bCopy( sal_False );
			jchar *pCharBits = (jchar *)t.pEnv->GetPrimitiveArrayCritical( chars, &bCopy );
			bCopy = sal_False;
			jint *pFlagBits = (jint *)t.pEnv->GetPrimitiveArrayCritical( flags, &bCopy );

			int nElementsCopied = 0;
			_par0.ResetPos();
			while ( _par0.GetNextRun( &i, &j, &bRTL ) )
			{
				int nRunLen = j - i;
				if ( bRTL && !bVertical )
				{
					// Substitute Arabic characters
					UErrorCode aErr = U_ZERO_ERROR;
					int nShapedChars = u_shapeArabic( _par0.mpStr + i, nRunLen, pCharBits + nElementsCopied, nElements - nElementsCopied, U_SHAPE_LETTERS_SHAPE, &aErr );

					int nFlags = _par0.mnFlags | SAL_LAYOUT_BIDI_RTL;
					for ( i = 0 ; i < nShapedChars ; i++ )
					{
						// Reverse RTL chars
						int k = i + nElementsCopied;
						int l = nElementsCopied + nShapedChars - i - 1;
						if ( k < l )
						{
							pCharBits[ k ] ^= pCharBits[ l ];
							pCharBits[ l ] ^= pCharBits[ k ];
							pCharBits[ k ] ^= pCharBits[ l ];
						}
						pFlagBits[ k ] = nFlags;
					}
					nElementsCopied += nShapedChars;
				}
				else
				{
					for ( ; i < j ; i++ )
					{
						int k = i - _par0.mnMinCharPos;
						pCharBits[ k ] = _par0.mpStr[ i ];
						pFlagBits[ k ] = _par0.mnFlags;
					}
					nElementsCopied += nRunLen;
				}
			}

			t.pEnv->ReleasePrimitiveArrayCritical( flags, (void *)pFlagBits, 0 );
			t.pEnv->ReleasePrimitiveArrayCritical( chars, (void *)pCharBits, 0 );

			jvalue args[4];
			args[0].l = chars;
			args[1].i = _par0.mnMinCharPos;
			args[2].i = _par0.mnEndCharPos;
			args[3].l = flags;
			t.pEnv->CallNonvirtualVoidMethodA( object, getMyClass(), mID, args );
		}
	}
}

// ----------------------------------------------------------------------------

void com_sun_star_vcl_VCLTextLayout::setDXArray( const long *_par0, int _par1 )
{
	static jmethodID mID = NULL;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "([I)V";
			mID = t.pEnv->GetMethodID( getMyClass(), "setDXArray", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			if ( _par0 && _par1 )
			{
				jintArray pAdvances = t.pEnv->NewIntArray( _par1 );
				jboolean bCopy( sal_False );
				long *pAdvanceBits = (long *)t.pEnv->GetPrimitiveArrayCritical( pAdvances, &bCopy );
				for ( int i = 0 ; i < _par1; i++ )
					memcpy( pAdvanceBits, _par0, _par1 * sizeof( long ) );
				t.pEnv->ReleasePrimitiveArrayCritical( pAdvances, (void *)pAdvanceBits, 0 );
				jvalue args[1];
				args[0].l = pAdvances;
				t.pEnv->CallNonvirtualVoidMethodA( object, getMyClass(), mID, args );
			}
		}
	}
}
