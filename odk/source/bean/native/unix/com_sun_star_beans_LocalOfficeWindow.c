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
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
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
 *  =================================================
 *  Modified March 2004 by Patrick Luby. SISSL Removed. NeoOffice is
 *  distributed under GPL only under modification term 3 of the LGPL.
 *
 *  Contributor(s): _______________________________________
 *
 ************************************************************************/

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Intrinsic.h>

#ifdef USE_JAVA
#include "jni.h"
#else	// USE_JAVA
#include "jawt.h"
#include "jawt_md.h"
#endif	// USE_JAVA

#include "../inc/com_sun_star_beans_LocalOfficeWindow.h"

#if defined assert
#undef assert
#endif

#define assert(X) if (!X) { (*env)->ThrowNew(env, (*env)->FindClass(env, "java/lang/RuntimeException"), "assertion failed"); return;}


#define SYSTEM_WIN32   1
#define SYSTEM_WIN16   2
#define SYSTEM_JAVA    3
#define SYSTEM_OS2     4
#define SYSTEM_MAC     5
#define SYSTEM_XWINDOW 6

/*****************************************************************************/
/*
 * Class:     com_sun_star_beans_LocalOfficeWindow
 * Method:    getNativeWindowSystemType
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_sun_star_beans_LocalOfficeWindow_getNativeWindowSystemType
  (JNIEnv * env, jobject obj_this)
{
#ifdef USE_JAVA
    return (SYSTEM_JAVA);
#else	// USE_JAVA
    return (SYSTEM_XWINDOW);
#endif	// USE_JAVA
}


/*****************************************************************************/
/*
 * Class:     com_sun_star_beans_LocalOfficeWindow
 * Method:    getNativeWindow
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL Java_com_sun_star_beans_LocalOfficeWindow_getNativeWindow
  (JNIEnv * env, jobject obj_this)
{
#ifdef USE_JAVA
	jlong out = 0;

	static jmethodID mID = NULL;
	if ( !mID )
	{
		char *cSignature = "()Ljava/awt/Container;";
		mID = (*env)->GetMethodID( env, (*env)->GetObjectClass( env, obj_this ), "getParent", cSignature );
	}
	if ( mID )
		out = (jlong)((*env)->CallObjectMethod( env, obj_this, mID ));

	return out;
#else	// USE_JAVA
	jboolean result;
	jint lock;

	JAWT awt;
	JAWT_DrawingSurface* ds;
	JAWT_DrawingSurfaceInfo* dsi;
	JAWT_X11DrawingSurfaceInfo* dsi_x11;

	Drawable drawable;
	Display* display;

	/* Get the AWT */
	awt.version = JAWT_VERSION_1_3;
	result = JAWT_GetAWT(env, &awt);
	assert(result != JNI_FALSE);
	
								/* Get the drawing surface */
	if ((ds = awt.GetDrawingSurface(env, obj_this)) == NULL)
		return 0L;
        
	/* Lock the drawing surface */
	lock = ds->Lock(ds);
	assert((lock & JAWT_LOCK_ERROR) == 0);
	
	/* Get the drawing surface info */
	dsi = ds->GetDrawingSurfaceInfo(ds);

	/* Get the platform-specific drawing info */
	dsi_x11 = (JAWT_X11DrawingSurfaceInfo*)dsi->platformInfo;
	
	drawable = dsi_x11->drawable;
	display  = dsi_x11->display;

	/* Free the drawing surface info */
	ds->FreeDrawingSurfaceInfo(dsi);
	/* Unlock the drawing surface */
	ds->Unlock(ds);
	/* Free the drawing surface */
	awt.FreeDrawingSurface(ds);

	return ((jlong)drawable);
#endif	// USE_JAVA
}
