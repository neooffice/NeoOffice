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

import java.awt.Menu;
import java.awt.peer.MenuPeer;
import com.sun.star.vcl.VCLMenuItemData;
import com.sun.star.vcl.VCLMenuBar;
import java.lang.IllegalArgumentException;
import com.sun.star.vcl.VCLMenuItemData.AWTPeersInvalidatedException;
import java.lang.System;

/**
 * The Java class that implements wrapper methods for managing AWT menus linked to VCL menus.
 * The bulk of the AWT code is handled by the VCLMenu's associated VCLMenuItemData object, so
 * many of these methods are used as convenient mapping routines between the C++ SalMenu class
 * and any additional internal Java routines that are needed.
 * <p>
 * @see VCLMenuItemData
 * @version 	$Revision$ $Date$
 * @author 	    $Author$
 */
public final class VCLMenu {
    /**
     * VCLMenuItemData object associated with this menu
     */
    private VCLMenuItemData menuData=null;
    
    private boolean disposed=false;
    
    /**
     * Construct a new VCLMenu instance.
     *
     * @param title
     */
    public VCLMenu( ) {
        menuData=new VCLMenuItemData( "", false, (short)0, 0 );
        try
        {
            menuData.makeMenu();
        }
        catch (AWTPeersInvalidatedException e)
        {
            // no peers yet
        }
    }
    
    /**
     * Free up any information used by the VCLMenu object to allow it to be
     * garbage collected faster.  Note that disposing the object invalidates
     * any reference previously returned by getMenuItemDataObject.
     */
    public void dispose( ) {
	menuData.dispose();
	menuData=null;
	disposed=true;
    }
     
    /**
     * Return the VCLMenuItemData object associate with the menu.
     *
     * @return VCLMenuItemData reference
     */
    public VCLMenuItemData getMenuItemDataObject( ) {
	if(disposed)
	    System.err.println("getMenuItemDataObject() invoked on disposed menu!");
     	return(menuData);
    }
   
    /**
     * Add a new menu item into the menu
     *
     * @param newItem	item to add
     * @param nPos	position in which the item should be added
     */
    public void insertItem(VCLMenuItemData newItem, int nPos) {
        if((nPos < 0) || (nPos == 65535))
            nPos=menuData.getNumMenuItems();
        
        try
        {
            menuData.addMenuItem(newItem, nPos);
        }
        catch (IllegalArgumentException e)
        {
            System.err.println("Illegal argument exception in VCLMenu.insertItem");
        }
        catch (AWTPeersInvalidatedException e)
        {
            menuData.refreshAWTPeersInParentMenus();
        }
    }
    
    /**
     * Remove an item from the menu
     *
     * @param nPos	position of the item to remove
     */
    public void removeItem(int nPos) {
        try
        {
            menuData.removeMenuItem(nPos);
        }
        catch (IllegalArgumentException e)
        {
            System.err.println("Illegal argument exception in VCLMenu.removeItem");
        }
    }
    
    /**
     * Change the checkmark state of an item in a menu
     *
     * @param nPos	position of the item to check/uncheck
     * @param bCheck	new checkmark state of the item
     */
    public void checkItem(int nPos, boolean bCheck) {
        VCLMenuItemData item=null;
	try
        {
            item=(VCLMenuItemData)menuData.getMenuItem(nPos);
            item.setChecked(bCheck);
        }
        catch (IllegalArgumentException e)
        {
            System.err.println("Illegal argument exception in VCLMenu.checkItem");
        }
        catch (AWTPeersInvalidatedException e)
        {
	    // checkbox items can't be top level menus, so we only have to
	    // worry about reinserting new peers into their parent menus
            if(item!=null)
		item.refreshAWTPeersInParentMenus();
        }
    }
    
    /**
     * Change the active state of an item in a menu
     *
     * @param nPos	position of the item to enable or disable
     * @param bEnable	new enabled state of the item
     */
    public void enableItem(int nPos, boolean bEnable) {
        try
        {
            VCLMenuItemData item=(VCLMenuItemData)menuData.getMenuItem(nPos);
            item.setEnabled(bEnable);
        }
        catch (IllegalArgumentException e)
        {
            System.err.println("Illegal argument exception in VCLMenu.checkItem");
        }
    }
    
    /**
     * Attach a submenu to an item in the menu
     * 
     * @param newMenu		contents of the new menu item
     * @param nPos		position where the submenu should be attached
     */
    public void attachSubmenu(VCLMenuItemData newMenu, int nPos) {
	if(newMenu==null)
	    System.err.println("New menu is null");
	    
        try
        {
            VCLMenuItemData item=null;
	    item=(VCLMenuItemData)menuData.getMenuItem(nPos);
	    if(item==null) {
		System.err.println("Item is null");
		return;
	    }
	    
	    if(item.getDelegate()==newMenu) {
		// no need to reassociate the menu if its item is already the
		// delegate
		return;
	    }
		
	    newMenu.setTitle(item.getTitle());
            newMenu.setEnabled(item.getEnabled());
            item.unregisterAllAWTPeers();
            item.setDelegate(newMenu);
	    
	    item.refreshAWTPeersInParentMenus();
        }
        catch (Exception e)
        {
            System.err.println("Error in attaching submenu "+e);
        }
    }
}
