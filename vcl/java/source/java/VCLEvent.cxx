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
		char *cSignature = "(ILcom/sun/star/vcl/VCLFrame;J)V";
		mID = t.pEnv->GetMethodID( getMyClass(), "<init>", cSignature );
	}
	OSL_ENSURE( mID, "Unknown method id!" );
	jvalue args[3];
	args[0].i = jint( nID );
	args[1].l = pFrame->maFrameData.mpVCLFrame->getJavaObject();
	args[2].j = jlong( pData );
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
			SalFrame *pFrame = pSalData->mpFirstFrame;
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
	if ( !pFrame )
		return;

	switch ( nID )
	{
		case SALEVENT_CLOSE:
		case SALEVENT_GETFOCUS:
		case SALEVENT_LOSEFOCUS:
		{
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
			SalKeyEvent *pKeyEvent = (SalKeyEvent *)pData;
			if ( !pKeyEvent )
			{
				pKeyEvent = new SalKeyEvent();
				pKeyEvent->mnTime = getWhen();
				pKeyEvent->mnCode = getModifiers();
				pKeyEvent->mnCharCode = 0;
				pKeyEvent->mnRepeat = 0;
			}
			dispatchEvent( nID, pFrame, pKeyEvent );
			delete pKeyEvent;
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
					pMouseEvent->mnButton = nModifiers;
			}
			dispatchEvent( nID, pFrame, pMouseEvent );
			delete pMouseEvent;
			return;
		}
		case SALEVENT_MOVE:
		case SALEVENT_MOVERESIZE:
		case SALEVENT_RESIZE:
		{
			Size aOldSize( pFrame->maGeometry.nWidth, pFrame->maGeometry.nHeight );
			Rectangle *pPosSize = (Rectangle *)pData;
			if ( !pPosSize )
			{
				// Update size
				pPosSize = new Rectangle( getBounds() );
			}
			pFrame->maGeometry.nX = pPosSize->nLeft + pFrame->maGeometry.nLeftDecoration;
			pFrame->maGeometry.nY = pPosSize->nTop + pFrame->maGeometry.nTopDecoration;
			pFrame->maGeometry.nWidth = pPosSize->GetWidth() - pFrame->maGeometry.nLeftDecoration - pFrame->maGeometry.nRightDecoration;
			pFrame->maGeometry.nHeight = pPosSize->GetHeight() - pFrame->maGeometry.nTopDecoration - pFrame->maGeometry.nBottomDecoration;
			dispatchEvent( nID, pFrame, NULL );
			delete pPosSize;
			// Invoke a paint event if the size has changed
			if ( pFrame->maGeometry.nWidth != aOldSize.Width() || pFrame->maGeometry.nHeight != aOldSize.Height() )
			{
				SalPaintEvent *pPaintEvent = new SalPaintEvent();
				pPaintEvent->mnBoundX = 0;
				pPaintEvent->mnBoundY = 0;
				pPaintEvent->mnBoundWidth = pFrame->maGeometry.nWidth + pFrame->maGeometry.nLeftDecoration;
				pPaintEvent->mnBoundHeight = pFrame->maGeometry.nHeight + pFrame->maGeometry.nTopDecoration;
				dispatchEvent( SALEVENT_PAINT, pFrame, pPaintEvent );
				delete pPaintEvent;
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
				pPaintEvent->mnBoundX = aUpdateRect.nLeft - pFrame->maGeometry.nLeftDecoration;
				pPaintEvent->mnBoundY = aUpdateRect.nTop - pFrame->maGeometry.nTopDecoration;
				pPaintEvent->mnBoundWidth = aUpdateRect.GetWidth() + pFrame->maGeometry.nLeftDecoration;
				pPaintEvent->mnBoundHeight = aUpdateRect.GetHeight() + pFrame->maGeometry.nTopDecoration;
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
		pFrame->maFrameData.mpProc( pFrame->maFrameData.mpInst, pFrame, nID, pData );
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

void *com_sun_star_vcl_VCLEvent::getData()
{
	static jmethodID mID = NULL;
	void *out = NULL;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "()J";
			mID = t.pEnv->GetMethodID( getMyClass(), "getData", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			out = (void *)t.pEnv->CallNonvirtualLongMethod( object, getMyClass(), mID );
		}
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
			char *cSignature = "()J";
			mID = t.pEnv->GetMethodID( getMyClass(), "getFrame", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			out = (SalFrame *)t.pEnv->CallNonvirtualLongMethod( object, getMyClass(), mID );
		}
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
		{
			out = (USHORT)t.pEnv->CallNonvirtualCharMethod( object, getMyClass(), mID );
		}
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
		{
			out = (USHORT)t.pEnv->CallNonvirtualCharMethod( object, getMyClass(), mID );
		}
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
		{
			out = (USHORT)t.pEnv->CallNonvirtualIntMethod( object, getMyClass(), mID );
		}
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
		{
			out = (USHORT)t.pEnv->CallNonvirtualIntMethod( object, getMyClass(), mID );
		}
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
		{
			out = (ULONG)t.pEnv->CallNonvirtualIntMethod( object, getMyClass(), mID );
		}
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
		{
			out = (long)t.pEnv->CallNonvirtualIntMethod( object, getMyClass(), mID );
		}
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
		{
			out = (long)t.pEnv->CallNonvirtualIntMethod( object, getMyClass(), mID );
		}
	}
	return out;
}

// ----------------------------------------------------------------------------

sal_Bool com_sun_star_vcl_VCLEvent::isAWTEvent()
{
	static jmethodID mID = NULL;
	sal_Bool out = sal_False;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "()Z";
			mID = t.pEnv->GetMethodID( getMyClass(), "isAWTEvent", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			out = (sal_Bool)t.pEnv->CallNonvirtualBooleanMethod( object, getMyClass(), mID );
		}
	}
	return out;
}
