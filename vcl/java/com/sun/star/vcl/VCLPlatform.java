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

package com.sun.star.vcl;

/** 
 * A Java class that provides convenience methods for accessing runtime
 * platform information.
 * <p>
 * @version 	$Revision$ $Date$
 * @author 	    $Author$
 */
public class VCLPlatform {

	/**
	 * The default platform.
	 */
	public final static int PLATFORM_UNKNOWN = 0;

	/**
	 * The Mac OS X platform.
	 */
	public final static int PLATFORM_MACOSX = 1;

	/**
	 * Cached platform.
	 */
	private static int platform = VCLPlatform.PLATFORM_UNKNOWN;

	/**
	 * Initialize the platform.
	 */
	static {

		String os = System.getProperty("os.name").toLowerCase();

		if (os.startsWith("mac os x"))
			VCLPlatform.platform = VCLPlatform.PLATFORM_MACOSX;

	}

	/**
	 * Returns the platform.
	 *
	 * @return the platform constant
	 */
	static int getPlatform() {

		return platform;

	}

}
