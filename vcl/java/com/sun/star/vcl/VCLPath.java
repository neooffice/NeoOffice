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
 *  Patrick Luby, March 2010
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2010 Planamesa Inc.
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

import java.awt.Shape;
import java.awt.geom.GeneralPath;

/** 
 * The Java class that implements the convenience methods for accessing Java
 * path information.
 * <p>
 * @version 	$Revision$ $Date$
 * @author 	    $Author$
 */
public final class VCLPath {

	/**
	 * Cached path.
	 */
	private GeneralPath path = new GeneralPath();

	/**
	 * Constructs a new <code>VCLPath</code> instance.
	 */
	public VCLPath() {}

	/**
	 * Closes the path.
	 */
	public void closePath() {

		path.closePath();

	}

	/**
	 * Extends a bezier curve to the path.
	 *
	 * @param firstControlX the first control X coordinate
	 * @param firstControlY the first control Y coordinate
	 * @param secondControlX the second control X coordinate
	 * @param secondControlY the second control Y coordinate
	 * @param x the X coordinate to extend a curve to
	 * @param y the Y coordinate to extend a curve to
	 */
	public void curveTo(float firstControlX, float firstControlY, float secondControlX, float secondControlY, float x, float y) {

		path.curveTo(firstControlX, firstControlY, secondControlX, secondControlY, x, y);

	}

	/**
	 * Extends a line in the path.
	 *
	 * @param x the X coordinate to extend a line to
	 * @param y the Y coordinate to extend a line to
	 */
	public void lineTo(float x, float y) {

		path.lineTo(x, y);

	}

	/**
	 * Moves the current point in the path.
	 *
	 * @param x the X coordinate to move to
	 * @param y the Y coordinate to move to
	 */
	public void moveTo(float x, float y) {

		path.moveTo(x, y);

	}

	/**
	 * Returns the path as a shape.
	 *
	 * @return the path as a shape
	 */
	Shape getShape() {

		return path;

	}

}
