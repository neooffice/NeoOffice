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

package com.sun.star.vcl.macosx;

import com.sun.star.vcl.VCLEvent;
import com.sun.star.vcl.VCLEventQueue;

import com.apple.mrj.MRJApplicationUtils;
import com.apple.mrj.MRJAboutHandler;

/** 
 * A Java class that implements the <code>MRJAboutHandler</code> interface.
 * <p>
 * @version 	$Revision$ $Date$
 * @author 	    $Author$
 */
public class VCLAboutHandler implements MRJAboutHandler {

	/**
	 * The event queue.
	 */
	private VCLEventQueue queue = null;

	/**
	 * Constructs a new <code>VCLAboutHandler</code> instance.
	 * 
	 * @param q the event queue to post events to
	 */
	public VCLAboutHandler(VCLEventQueue q) {

		queue = q;

		// Register an instance of this class as the quit handler
		MRJApplicationUtils.registerAboutHandler(this);

	}

	/**
	 * Called to handle about operations.
	 */
	public void handleAbout() {

		queue.postCachedEvent(new VCLEvent(VCLEvent.SALEVENT_ABOUT, null, 0));

	}

}
