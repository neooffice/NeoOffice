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

import java.awt.Dimension;
import java.awt.Frame;
import java.awt.Insets;
import java.awt.Toolkit;

/** 
 * The Java class that implements the convenience methods for accessing
 * screen information.
 * <p>
 * @version 	$Revision$ $Date$
 * @author 	    $Author$
 */
public final class VCLScreen {

	/**
	 * The minimum screen resolution
	 */
	public final static int MIN_PRINTER_RESOLUTION = 72;

	/**
	 * The minimum screen resolution
	 */
	public final static int MIN_SCREEN_RESOLUTION = 96;

	/**
	 * The cached frame insets.
	 */
	private static Insets frameInsets = null;

	/**
	 * The cached screen size.
	 */
	private static Dimension screenSize = null;

	/**
	 * Initialize screen size and frame insets.
	 */
	static {

		screenSize = Toolkit.getDefaultToolkit().getScreenSize();

		Frame f = new Frame();
		f.addNotify();
		frameInsets = f.getInsets();
		if (VCLPlatform.getPlatform() == VCLPlatform.PLATFORM_MACOSX)
			screenSize.height -= frameInsets.top;
		f.removeNotify();

	}

	/**
	 * Gets the <code>Frame</code> insets.
	 *
	 * @return the <code>Frame</code> insets
	 */
	static Insets getFrameInsets() {

		return frameInsets;

	}

	/**
	 * Gets the size of the screen.
	 *
	 * @return the size of the screen
	 */
	public static Dimension getScreenSize() {

		return screenSize;

	}

}
