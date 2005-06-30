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
public final class VCLPlatform {

	/**
	 * The Java 1.2 constant.
	 */
	public final static int JAVA_VERSION_1_2 = 0x00010002;

	/**
	 * The Java 1.3 constant.
	 */
	public final static int JAVA_VERSION_1_3 = 0x00010003;

	/**
	 * The Java 1.4 constant.
	 */
	public final static int JAVA_VERSION_1_4 = 0x00010004;

	/**
	 * The Java 1.5 constant.
	 */
	public final static int JAVA_VERSION_1_5 = 0x00010005;

	/**
	 * Cached Java version.
	 */
	private static int javaVersion = JAVA_VERSION_1_2;

	/**
	 * Initialize the platform and Java version.
	 */
	static {

		try {
			Class.forName("java.lang.StrictMath");
			javaVersion = JAVA_VERSION_1_3;
			Class.forName("java.lang.CharSequence");
			javaVersion = JAVA_VERSION_1_4;
			Class.forName("java.lang.Readable");
			javaVersion = JAVA_VERSION_1_5;
		}
		catch (Throwable t) {}

	}

	/**
	 * Returns the Java version.
	 *
	 * @return the Java version
	 */
	static int getJavaVersion() {

		return javaVersion;

	}

}
