/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 * 
 *   Modified October 2016 by Patrick Luby. NeoOffice is only distributed
 *   under the GNU General Public License, Version 3 as allowed by Section 3.3
 *   of the Mozilla Public License, v. 2.0.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef INCLUDED_AVMEDIA_SOURCE_QUICKTIME_QUICKTIMECOMMON_HXX
#define INCLUDED_AVMEDIA_SOURCE_QUICKTIME_QUICKTIMECOMMON_HXX

#ifdef MACOSX
#include <premac.h>
#import <Cocoa/Cocoa.h>
#if MACOSX_SDK_VERSION < 101200
#import <QTKit/QTKit.h>
#endif	// MACOSX_SDK_VERSION < 101200
#include <postmac.h>
#endif
#include <osl/mutex.hxx>
#include <rtl/ustring.hxx>
#include <tools/stream.hxx>
#include <cppuhelper/weak.hxx>
#include <cppuhelper/factory.hxx>

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/awt/KeyModifier.hpp>
#include <com/sun/star/awt/MouseButton.hpp>
#include <com/sun/star/media/XManager.hpp>

#define AVMEDIA_QUICKTIME_MANAGER_IMPLEMENTATIONNAME "com.sun.star.comp.avmedia.Manager_QuickTime"
#define AVMEDIA_QUICKTIME_MANAGER_SERVICENAME "com.sun.star.media.Manager_QuickTime"

#define AVMEDIA_QUICKTIME_PLAYER_IMPLEMENTATIONNAME "com.sun.star.comp.avmedia.Player_QuickTime"
#define AVMEDIA_QUICKTIME_PLAYER_SERVICENAME "com.sun.star.media.Player_QuickTime"

#define AVMEDIA_QUICKTIME_WINDOW_IMPLEMENTATIONNAME "com.sun.star.comp.avmedia.Window_QuickTime"
#define AVMEDIA_QUICKTIME_WINDOW_SERVICENAME "com.sun.star.media.Window_QuickTime"

#ifdef USE_JAVA
#define AVMEDIA_QUICKTIME_FRAMEGRABBER_IMPLEMENTATIONNAME "com.sun.star.comp.avmedia.FrameGrabber_Quicktime"
#define AVMEDIA_QUICKTIME_FRAMEGRABBER_SERVICENAME "com.sun.star.media.FrameGrabber_Quicktime"
#endif	// USE_JAVA

#endif // _QUICKTIMECOMMOM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
