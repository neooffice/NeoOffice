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

#define _SV_COM_SUN_STAR_VCL_VCLEVENT_CXX

#ifndef _SV_COM_SUN_STAR_VCL_VCLEVENT_HXX
#include <com/sun/star/vcl/VCLEvent.hxx>
#endif
#ifndef _SV_COM_SUN_STAR_VCL_VCLFRAME_HXX
#include <com/sun/star/vcl/VCLFrame.hxx>
#endif
#ifndef _SV_COM_SUN_STAR_VCL_VCLGRAPHICS_HXX
#include <com/sun/star/vcl/VCLGraphics.hxx>
#endif
#ifndef _SV_SALDATA_HXX
#include <saldata.hxx>
#endif
#ifndef _SV_SALFRAME_HXX
#include <salframe.hxx>
#endif
#ifndef _SV_EVENT_HXX
#include <event.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <svapp.hxx>
#endif

#ifdef MACOSX

#include <premac.h>
#include <QuickTime/QuickTime.h>
#include <postmac.h>

using namespace rtl;

#endif	// MACOSX

using namespace vcl;

// ============================================================================

jclass com_sun_star_vcl_VCLEvent::theClass = NULL;

// ----------------------------------------------------------------------------

jclass com_sun_star_vcl_VCLEvent::getMyClass()
{
	if ( !theClass )
	{
		VCLThreadAttach t;
		if ( !t.pEnv ) return (jclass)NULL;
		jclass tempClass = t.pEnv->FindClass( "com/sun/star/vcl/VCLEvent" );
		OSL_ENSURE( tempClass, "Java : FindClass not found!" );
		theClass = (jclass)t.pEnv->NewGlobalRef( tempClass );
	}
	return theClass;
}

// ----------------------------------------------------------------------------

com_sun_star_vcl_VCLEvent::com_sun_star_vcl_VCLEvent( USHORT nID, const SalFrame *pFrame, void *pData ) : java_lang_Object( (jobject)NULL )
{
	static jmethodID mID = NULL;
	VCLThreadAttach t;
	if ( !t.pEnv )
		return;
	if ( !mID )
	{
		char *cSignature = "(ILcom/sun/star/vcl/VCLFrame;I)V";
		mID = t.pEnv->GetMethodID( getMyClass(), "<init>", cSignature );
	}
	OSL_ENSURE( mID, "Unknown method id!" );
	jvalue args[3];
	args[0].i = jint( nID );
	args[1].l = pFrame->maFrameData.mpVCLFrame->getJavaObject();
	args[2].i = jint( pData );
	jobject tempObj;
	tempObj = t.pEnv->NewObjectA( getMyClass(), mID, args );
	saveRef( tempObj );
}

// ----------------------------------------------------------------------------

void com_sun_star_vcl_VCLEvent::dispatch()
{
	USHORT nID = getID();
	void *pData = getData();
	SalData *pSalData = GetSalData();

	// Handle events that do not need a SalFrame pointer
	switch ( nID )
	{
		case SALEVENT_SHUTDOWN:
		{
			SalFrame *pFrame = pSalData->maFrameList.front();
			if ( pFrame )
				dispatchEvent( nID, pFrame, NULL );
			return;
		}
		case SALEVENT_OPENDOCUMENT:
		{
			String aEmptyStr;
			ApplicationEvent aAppEvt( aEmptyStr, aEmptyStr, APPEVENT_OPEN_STRING, getPath() );
			ImplGetSVData()->mpApp->AppEvent( aAppEvt );

			return;
		}
		case SALEVENT_PRINTDOCUMENT:
		{
			String aEmptyStr;
			ApplicationEvent aAppEvt( aEmptyStr, aEmptyStr, APPEVENT_PRINT_STRING, getPath() );
			ImplGetSVData()->mpApp->AppEvent( aAppEvt );

			return;
		}
	}
	
	// Handle events that require a SalFrame pointer
	SalFrame *pFrame = getFrame();

	switch ( nID )
	{
		case SALEVENT_CLOSE:
		{
			dispatchEvent( nID, pFrame, NULL );
			return;
		}
		case SALEVENT_ENDEXTTEXTINPUT:
		{
			SalExtTextInputEvent *pInputEvent = (SalExtTextInputEvent *)pData;
			if ( pInputEvent )
				delete pInputEvent;
			dispatchEvent( nID, pFrame, NULL );
			return;
		}
		case SALEVENT_EXTTEXTINPUT:
		{
			XubString aText( getText() );
			ULONG nLen = aText.Len();
			ULONG nCommitted = getCommittedCharacterCount();
			USHORT *pAttributes = getTextAttributes();
			SalExtTextInputEvent *pInputEvent = (SalExtTextInputEvent *)pData;
			if ( !pInputEvent )
			{
				pInputEvent = new SalExtTextInputEvent();
				pInputEvent->mnTime = getWhen();
				pInputEvent->maText = aText;
				pInputEvent->mpTextAttr = pAttributes;
				pInputEvent->mnCursorPos = nLen;
				pInputEvent->mnDeltaStart = 0;
				pInputEvent->mbOnlyCursor = FALSE;
				pInputEvent->mnCursorFlags = 0;
			}
			dispatchEvent( nID, pFrame, pInputEvent );
			delete pInputEvent;
			if ( pAttributes )
				rtl_freeMemory( pAttributes );
			// If there is no text, the character is committed
			if ( nLen == nCommitted )
				dispatchEvent( SALEVENT_ENDEXTTEXTINPUT, pFrame, NULL );
			return;
		}
		case SALEVENT_GETFOCUS:
		{
			pSalData->mpFocusFrame = pFrame;
			dispatchEvent( nID, pFrame, NULL );

			// In presentation mode, don't let presentation window lose focus
			if ( pSalData->mpPresentationFrame && pSalData->mpPresentationFrame != pFrame )
				pSalData->mpPresentationFrame->ToTop( SAL_FRAME_TOTOP_FOREGROUNDTASK );
			return;
		}
		case SALEVENT_LOSEFOCUS:
		{
			if ( pSalData->mpFocusFrame == pFrame )
				pSalData->mpFocusFrame = NULL;
			dispatchEvent( nID, pFrame, NULL );
			return;
		}
		case SALEVENT_KEYINPUT:
		case SALEVENT_KEYUP:
		{
			SalKeyEvent *pKeyEvent = (SalKeyEvent *)pData;
			if ( !pKeyEvent )
			{
				pKeyEvent = new SalKeyEvent();
				pKeyEvent->mnTime = getWhen();
				pKeyEvent->mnCode = getKeyCode() | getModifiers();
				pKeyEvent->mnCharCode = getKeyChar();
				pKeyEvent->mnRepeat = 0;
			}
			dispatchEvent( nID, pFrame, pKeyEvent );
			delete pKeyEvent;
			return;
		}
		case SALEVENT_KEYMODCHANGE:
		{
			SalKeyModEvent *pKeyModEvent = (SalKeyModEvent *)pData;
			if ( !pKeyModEvent )
			{
				pKeyModEvent = new SalKeyModEvent();
				pKeyModEvent->mnTime = getWhen();
				pKeyModEvent->mnCode = getModifiers();
			}
			dispatchEvent( nID, pFrame, pKeyModEvent );
			delete pKeyModEvent;
			return;
		}
		case SALEVENT_MOUSEBUTTONDOWN:
		case SALEVENT_MOUSEBUTTONUP:
		case SALEVENT_MOUSELEAVE:
		case SALEVENT_MOUSEMOVE:
		{
			SalMouseEvent *pMouseEvent = (SalMouseEvent *)pData;
			if ( !pMouseEvent )
			{
				pMouseEvent = new SalMouseEvent();
				pMouseEvent->mnTime = getWhen();
				pMouseEvent->mnX = getX();
				pMouseEvent->mnY = getY();
				USHORT nModifiers = getModifiers();
				pMouseEvent->mnCode = nModifiers;
				if ( nID == SALEVENT_MOUSELEAVE || nID == SALEVENT_MOUSEMOVE )
					pMouseEvent->mnButton = 0;
				else
					pMouseEvent->mnButton = nModifiers & ( MOUSE_LEFT | MOUSE_MIDDLE | MOUSE_RIGHT );
			}
			dispatchEvent( nID, pFrame, pMouseEvent );
			delete pMouseEvent;
			return;
		}
		case SALEVENT_MOVE:
		case SALEVENT_MOVERESIZE:
		case SALEVENT_RESIZE:
		{
			Rectangle *pPosSize = (Rectangle *)pData;
			Size aOldSize;
			if ( pFrame )
			{
				aOldSize = Size( pFrame->maGeometry.nWidth, pFrame->maGeometry.nHeight );
				if ( !pPosSize )
				{
					// Update size
					pPosSize = new Rectangle( getBounds() );
				}
				pFrame->maGeometry.nX = pPosSize->nLeft + pFrame->maGeometry.nLeftDecoration;
				pFrame->maGeometry.nY = pPosSize->nTop + pFrame->maGeometry.nTopDecoration;
				pFrame->maGeometry.nWidth = pPosSize->GetWidth() - pFrame->maGeometry.nLeftDecoration - pFrame->maGeometry.nRightDecoration;
				pFrame->maGeometry.nHeight = pPosSize->GetHeight() - pFrame->maGeometry.nTopDecoration - pFrame->maGeometry.nBottomDecoration;
				// Reset graphics if the size has changed before dispatching
				if ( pFrame->maGeometry.nWidth != aOldSize.Width() || pFrame->maGeometry.nHeight != aOldSize.Height() )
				{
					com_sun_star_vcl_VCLGraphics *pVCLGraphics = pFrame->maFrameData.mpVCLFrame->getGraphics();
					if ( pVCLGraphics )
					{
						pVCLGraphics->resetGraphics();
						delete pVCLGraphics;
					}
				}
			}
			dispatchEvent( nID, pFrame, NULL );
			delete pPosSize;
			if ( pFrame )
			{
				// Invoke a paint event if the size has changed
				if ( pFrame->maGeometry.nWidth != aOldSize.Width() || pFrame->maGeometry.nHeight != aOldSize.Height() )
				{
					SalPaintEvent *pPaintEvent = new SalPaintEvent();
					pPaintEvent->mnBoundX = 0;
					pPaintEvent->mnBoundY = 0;
					pPaintEvent->mnBoundWidth = pFrame->maGeometry.nWidth;
					pPaintEvent->mnBoundHeight = pFrame->maGeometry.nHeight;
					com_sun_star_vcl_VCLEvent aVCLPaintEvent( SALEVENT_PAINT, pFrame, (void *)pPaintEvent );
					GetSalData()->mpEventQueue->postCachedEvent( &aVCLPaintEvent );
				}
			}
			return;
		}
		case SALEVENT_PAINT:
		{
			SalPaintEvent *pPaintEvent = (SalPaintEvent *)pData;
			if ( !pPaintEvent )
			{
				// Get paint region
				const Rectangle &aUpdateRect = getUpdateRect();
				pPaintEvent = new SalPaintEvent();
				if ( pFrame )
				{
					pPaintEvent->mnBoundX = aUpdateRect.nLeft;
					pPaintEvent->mnBoundY = aUpdateRect.nTop;
					pPaintEvent->mnBoundWidth = aUpdateRect.GetWidth();
					pPaintEvent->mnBoundHeight = aUpdateRect.GetHeight();
				}
			}
			dispatchEvent( nID, pFrame, pPaintEvent );
			delete pPaintEvent;
			return;
		}
		case SALEVENT_USEREVENT:
			dispatchEvent( nID, pFrame, pData );
			return;
	}

#ifdef DEBUG
	fprintf( stderr, "Unhandled Event: %i\n", nID );
#endif
}

// ----------------------------------------------------------------------------

void com_sun_star_vcl_VCLEvent::dispatchEvent( USHORT nID, SalFrame *pFrame, void *pData )
{
	if ( pFrame && pFrame->maFrameData.mpProc )
	{
		pFrame->maFrameData.mpProc( pFrame->maFrameData.mpInst, pFrame, nID, pData );

		// Flush the window's buffer to the native window
		SalData *pSalData = GetSalData();
		for ( ::std::list< SalFrame* >::const_iterator it = pSalData->maFrameList.begin(); it != pSalData->maFrameList.end(); ++it )
		{
			if ( pFrame == *it )
			{
				pFrame->Flush();
				break;
			}
		}
	}
}

// ----------------------------------------------------------------------------

const Rectangle com_sun_star_vcl_VCLEvent::getBounds()
{
	static jmethodID mID = NULL;
	static jfieldID fIDX = NULL;
	static jfieldID fIDY = NULL;
	static jfieldID fIDWidth = NULL;
	static jfieldID fIDHeight = NULL;
	Rectangle out( Point( 0, 0 ), Size( 0, 0 ) );
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
					fIDY = t.pEnv->GetFieldID( tempObjClass, "y", cSignature );
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
					Point aPoint( (long)t.pEnv->GetIntField( tempObj, fIDX ), (long)t.pEnv->GetIntField( tempObj, fIDY ) );
					Size aSize( (long)t.pEnv->GetIntField( tempObj, fIDWidth ), (long)t.pEnv->GetIntField( tempObj, fIDHeight ) );
					out = Rectangle( aPoint, aSize );
				}
			}
		}
	}
	return out;
}

// ----------------------------------------------------------------------------

ULONG com_sun_star_vcl_VCLEvent::getCommittedCharacterCount()
{
	static jmethodID mID = NULL;
	ULONG out = 0;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "()I";
			mID = t.pEnv->GetMethodID( getMyClass(), "getCommittedCharacterCount", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
			out = (ULONG)t.pEnv->CallNonvirtualIntMethod( object, getMyClass(), mID );
	}
	return out;
}

// ----------------------------------------------------------------------------

void *com_sun_star_vcl_VCLEvent::getData()
{
	static jmethodID mID = NULL;
	void *out = NULL;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "()I";
			mID = t.pEnv->GetMethodID( getMyClass(), "getData", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
			out = (void *)t.pEnv->CallNonvirtualIntMethod( object, getMyClass(), mID );
	}
	return out;
}

// ----------------------------------------------------------------------------

SalFrame *com_sun_star_vcl_VCLEvent::getFrame()
{
	static jmethodID mID = NULL;
	SalFrame *out = NULL;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "()I";
			mID = t.pEnv->GetMethodID( getMyClass(), "getFrame", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
			out = (SalFrame *)t.pEnv->CallNonvirtualIntMethod( object, getMyClass(), mID );
	}
	return out;
}

// ----------------------------------------------------------------------------

USHORT com_sun_star_vcl_VCLEvent::getKeyChar()
{
	static jmethodID mID = NULL;
	USHORT out = 0;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "()C";
			mID = t.pEnv->GetMethodID( getMyClass(), "getKeyChar", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
			out = (USHORT)t.pEnv->CallNonvirtualCharMethod( object, getMyClass(), mID );
	}
	return out;
}

// ----------------------------------------------------------------------------

USHORT com_sun_star_vcl_VCLEvent::getKeyCode()
{
	static jmethodID mID = NULL;
	USHORT out = 0;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "()I";
			mID = t.pEnv->GetMethodID( getMyClass(), "getKeyCode", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
			out = (USHORT)t.pEnv->CallNonvirtualCharMethod( object, getMyClass(), mID );
	}
	return out;
}

// ----------------------------------------------------------------------------

USHORT com_sun_star_vcl_VCLEvent::getID()
{
	static jmethodID mID = NULL;
	USHORT out = 0;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "()I";
			mID = t.pEnv->GetMethodID( getMyClass(), "getID", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
			out = (USHORT)t.pEnv->CallNonvirtualIntMethod( object, getMyClass(), mID );
	}
	return out;
}

// ----------------------------------------------------------------------------

USHORT com_sun_star_vcl_VCLEvent::getModifiers()
{
	static jmethodID mID = NULL;
	USHORT out = 0;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "()I";
			mID = t.pEnv->GetMethodID( getMyClass(), "getModifiers", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
			out = (USHORT)t.pEnv->CallNonvirtualIntMethod( object, getMyClass(), mID );
	}
	return out;
}

// ----------------------------------------------------------------------------

::rtl::OUString com_sun_star_vcl_VCLEvent::getPath()
{
	static jmethodID mID = NULL;
	::rtl::OUString out;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "()Ljava/lang/String;";
			mID = t.pEnv->GetMethodID( getMyClass(), "getPath", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			jstring tempObj = (jstring)t.pEnv->CallNonvirtualObjectMethod( object, getMyClass(), mID );
			if ( tempObj )
				out = JavaString2String( t.pEnv, tempObj );
		}
	}
	return out;
}

// ----------------------------------------------------------------------------

::rtl::OUString com_sun_star_vcl_VCLEvent::getText()
{
	static jmethodID mID = NULL;
	::rtl::OUString out;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "()Ljava/lang/String;";
			mID = t.pEnv->GetMethodID( getMyClass(), "getText", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			jstring tempObj = (jstring)t.pEnv->CallNonvirtualObjectMethod( object, getMyClass(), mID );
			if ( tempObj )
				out = JavaString2String( t.pEnv, tempObj );
		}
	}
	return out;
}

// ----------------------------------------------------------------------------

USHORT *com_sun_star_vcl_VCLEvent::getTextAttributes()
{
	static jmethodID mID = NULL;
	USHORT *out = NULL;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "()[I";
			mID = t.pEnv->GetMethodID( getMyClass(), "getTextAttributes", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			jintArray tempObj = (jintArray)t.pEnv->CallNonvirtualObjectMethod( object, getMyClass(), mID );
			if ( tempObj )
			{
				jsize nAttributes = t.pEnv->GetArrayLength( tempObj );
				if ( nAttributes )
				{
					jboolean bCopy( sal_False );
					jint *pAttributes = (jint *)t.pEnv->GetPrimitiveArrayCritical( tempObj, &bCopy );
					out = (USHORT *)rtl_allocateMemory( sizeof( USHORT* ) * nAttributes );
					if ( out )
					{
						for ( jsize i = 0; i < nAttributes; i++)
							out[i] = pAttributes[i];
					}
					t.pEnv->ReleasePrimitiveArrayCritical( tempObj, (void *)pAttributes, JNI_ABORT );
				}
			}
		}
	}
	return out;
}

// ----------------------------------------------------------------------------

const Rectangle com_sun_star_vcl_VCLEvent::getUpdateRect()
{
	static jmethodID mID = NULL;
	static jfieldID fIDX = NULL;
	static jfieldID fIDY = NULL;
	static jfieldID fIDWidth = NULL;
	static jfieldID fIDHeight = NULL;
	Rectangle out( Point( 0, 0 ), Size( 0, 0 ) );
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "()Ljava/awt/Rectangle;";
			mID = t.pEnv->GetMethodID( getMyClass(), "getUpdateRect", cSignature );
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
					fIDY = t.pEnv->GetFieldID( tempObjClass, "y", cSignature );
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
					Point aPoint( (long)t.pEnv->GetIntField( tempObj, fIDX ), (long)t.pEnv->GetIntField( tempObj, fIDY ) );
					Size aSize( (long)t.pEnv->GetIntField( tempObj, fIDWidth ), (long)t.pEnv->GetIntField( tempObj, fIDHeight ) );
					out = Rectangle( aPoint, aSize );
				}
			}
		}
	}
	return out;
}

// ----------------------------------------------------------------------------

ULONG com_sun_star_vcl_VCLEvent::getWhen()
{
	static jmethodID mID = NULL;
	ULONG out = 0;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "()J";
			mID = t.pEnv->GetMethodID( getMyClass(), "getWhen", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
			out = (ULONG)t.pEnv->CallNonvirtualLongMethod( object, getMyClass(), mID );
	}
	return out;
}

// ----------------------------------------------------------------------------

long com_sun_star_vcl_VCLEvent::getX()
{
	static jmethodID mID = NULL;
	long out = 0;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "()I";
			mID = t.pEnv->GetMethodID( getMyClass(), "getX", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
			out = (long)t.pEnv->CallNonvirtualIntMethod( object, getMyClass(), mID );
	}
	return out;
}

// ----------------------------------------------------------------------------

long com_sun_star_vcl_VCLEvent::getY()
{
	static jmethodID mID = NULL;
	long out = 0;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "()I";
			mID = t.pEnv->GetMethodID( getMyClass(), "getY", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
			out = (long)t.pEnv->CallNonvirtualIntMethod( object, getMyClass(), mID );
	}
	return out;
}
