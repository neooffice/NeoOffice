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
 
import java.lang.String;
import java.lang.System;
import java.util.Vector;
import java.util.Stack;
import java.util.Enumeration;
import java.lang.Exception;
import java.lang.IllegalArgumentException;
import java.awt.Menu;
import java.awt.MenuItem;
import java.awt.CheckboxMenuItem;
import java.awt.event.ActionListener;
import java.awt.event.ActionEvent;
import java.awt.MenuShortcut;
import com.sun.star.vcl.VCLEvent;
import com.sun.star.vcl.VCLEventQueue;
import com.sun.star.vcl.VCLMenuBar;
import com.sun.star.vcl.VCLFrame;

 /**
  * Instances of this class are used to hold information needed to construct a menu item
  * or menu corresponding to VCL menu items.  Menus are considered to be Vectors of these
  * items.
  */
public final class VCLMenuItemData {
    /**
     * Delagate object that performs work for us, if applicable
     */
    private VCLMenuItemData delegate = null;
    
    /**
     * Set the delegate object for this instance
     *
     * @param d	new delegate
     */
    public void setDelegate(VCLMenuItemData d) {
        delegate=d;
    }
    
    /**
     * Unicode string that corresponds to the title
     */
    private String title=new String();
    
    /**
     * Indicates whether the keyboard shortcut has been set
     */
    private boolean keyboardShortcutSet=false;
    
    /**
     * Keyboard shortcut to use
     */
    private int keyboardShortcut=0;
    
    /**
     * True if shift should be used as a modifier with the shortcut, false if
     * not.
     */
    private boolean keyboardShortcutUseShift=false;
    
    /**
     * Identifier that is used in Sal events to identify this specific menu item in VCL events
     */
    private short vclID=0;
    
    /**
     * Cookie needed to make the association between the C++ VCL Menu object that is spawning
     * Sal events.  Used in conjunction with the id.  Note that this is actually a void *
     * pointer (!).  For now we'll use the int Java type which i s32 bits
     */
    private int vclMenuCookie=0;
    
    /**
     * True if this item is a separator, false if not
     */
    private boolean isSeparator=false;
    
    /**
     * True if this item is a submenu, false if not
     */
    private boolean isSubmenu=false;
    
    /**
     * If the item is a submenu, Vector containing all of the menu items comprising the menu.
     * The items are stored as VCLMenuItemData references.
     */
    private java.util.Vector menuItems=new Vector();
    
    /**
     * True if this item is enabled, false if not
     */
    private boolean isEnabled=true;
    
    /**
     * True if this item is a checkbox, false if not
     */
    private boolean isCheckbox=false;
    
    /**
     * True if this item is checked, false if it is not.  If the item is checked, it should also be a checkbox
     */
    private boolean isChecked=false;
    
    /**
     * Vector of AWT objects that have been generated for this set of menu item data and are being managed
     * by it.
     */
    private java.util.Vector awtPeers=new Vector();
    
    /**
     * Construct and initialize a new <b>VCLMenuItemData</b> instance
     *
     * @param newTitle		initial value of the title of the menu item
     * @param separator		true if the item is to be a separator, false if it is to be any other type of item
     * @param id		identifier associated with this item that's require for posting VCL events
     * @param cookie		cookie used in Sal events to tie the id to a specific menu
     */
    public VCLMenuItemData(String newTitle, boolean separator, short id, int cookie) {
        if(separator)
        {
            isSeparator=true;
            title=new String("-");
        }
        else
            title=new String(newTitle);
        vclID=id;
        vclMenuCookie=cookie;
    }
    
    /**
     * Clean up as many references to this item and its peers as we can to allow
     * garbage collection and destruction of their heavyweight peers.
     *
     * Note that this does not instruct an item's delegate object to be
     * disposed;  delegates must manually be disposed.
     */
    public void dispose() {
	unregisterAllAWTPeers();
	menuItems=null;
	title=null;
    }
    
    /**
     * Exception thrown by any of the methods when the method results in a change that invalidates any
     * AWT mirror objects generated from this menu item data are invalid and must be regenerated.
     */
    public class AWTPeersInvalidatedException extends java.lang.Exception { }
    
    /**
     * Fetch the current title of the menu item.
     */
    public String getTitle() {
        if(delegate!=null)
            return(delegate.getTitle());
            
        return(title);
    };
    
    /**
     * Change the title of the menu item.  Any AWT peers will be automatically updated.
     *
     * @param newTitle		new title
     */
    synchronized public void setTitle(String newTitle) {
        if(delegate!=null) {
            delegate.setTitle(newTitle);
            return;
        }
        
        if(!isSeparator) {
            title=new String(newTitle);
            Enumeration e=awtPeers.elements();
            while(e.hasMoreElements()) {
                MenuItem m=(MenuItem)e.nextElement();
                m.setLabel(title);
            }
        }
    }
    
    /**
     * Change the keyboard shortcut of the menu item.  Any AWT peers will be automatically updated.
     *
     * @param key	VCL keycode of the new key to use as the shortcut
     * @param useShift  true if the shift key should additionally be required
     *			for the shortcut, false if just command is needed
     *			as modifier.
     */
    synchronized public void setKeyboardShortcut(int key, boolean useShift) {
        if(delegate!=null) {
            delegate.setKeyboardShortcut(key, useShift);
            return;
        }
        
        int newShortcut=VCLEvent.convertVCLKeyCode(key);
        if(newShortcut!=0) {
            keyboardShortcut=newShortcut;
            keyboardShortcutSet=true;
	    keyboardShortcutUseShift=useShift;
            if(!awtPeers.isEmpty()) {
                Enumeration e=awtPeers.elements();
                while(e.hasMoreElements()) {
                    MenuItem m=(MenuItem)e.nextElement();
                    m.setShortcut(new MenuShortcut(keyboardShortcut, keyboardShortcutUseShift));
                }
            }
        }
    }
    
    /**
     * Fetch the current enabled state of the menu item.
     */
    public boolean getEnabled() {
        if(delegate!=null)
            return(delegate.getEnabled());
            
        return(isEnabled);
    }
    
    /**
     * Change the current enabled state of the menu item.  Any AWT peers will be automatically updated.
     *
     * @param newEnabled	true if the item should be enabled, false if it should be disabled
     */
    synchronized public void setEnabled(boolean newEnabled) {
        if(delegate!=null) {
            delegate.setEnabled(newEnabled);
            return;
        }
        
        isEnabled=newEnabled;
        Enumeration e=awtPeers.elements();
        while(e.hasMoreElements()) {
            MenuItem m=(MenuItem)e.nextElement();
            if(isEnabled)
                m.enable();
            else
                m.disable();
        }
    }
    
    /**
     * Get the id corresponding to the menu item that should be used to refer to this menu item in VCL
     * events.
     */
    public short getVCLID() {
        if(delegate!=null)
            return(delegate.getVCLID());
            
        return(vclID);
    }
    
    /**
     * Get the cookie corresponding to the menu item that should be used to refer to this menu in VCL
     * events.
     */
    public int getVCLCookie() {
        if(delegate!=null)
            return(delegate.getVCLCookie());
            
        return(vclMenuCookie);
    }
    
    /**
     * Fetch the current checked state of the menu item
     */
    public boolean getChecked() {
        if(delegate!=null)
            return(delegate.getChecked());
            
        return(isChecked);
    }
    
    /**
     * Change the current checked state of the menu item.  Note that changing the checked state may have
     * side effects that invalidate the AWT peers.  Some items, such as separators and items that are actually
     * submenus, cannot be checked.
     *
     * @param newCheck	true if the item should be checked, false if unchecked
     * @throws AWTPeersInvalidatedException to indicate that the AWT peers are no longer valid and need to be
     * 	regenerated
     * @throws IllegalArgumentException to indicate that the checked state does not apply if other characteristics
     *	of the menu item make it inappropriate
     */
    synchronized public void setChecked(boolean newCheck) throws AWTPeersInvalidatedException, IllegalArgumentException {
        if(delegate!=null) {
            delegate.setChecked(newCheck);
            return;
        }
        
        if(newCheck && (isSeparator || isSubmenu))
            throw new IllegalArgumentException();
        
        isChecked=newCheck;
        if(!isCheckbox)
        {
            if(isChecked)
            {
                isCheckbox=true;
                
		// we were just set to checked, so we need to use instances of CheckMenuItem AWT objects instead
                // of regular MenuItems.  We need to invalidate our peers.
                
                if(!awtPeers.isEmpty())
                {
                    awtPeers.clear();
                    throw new AWTPeersInvalidatedException();
                }
            }
        }
        else
        {
            // change state of our checkbox peers
            
            Enumeration e=awtPeers.elements();
            while(e.hasMoreElements())
            {
                CheckboxMenuItem cMI=(CheckboxMenuItem)e.nextElement();
                cMI.setState(isChecked);
            }
        }
    }
    
    /**
     * Determine if the item is currently a menu or a regular item.  Adding the first menu item will
     * change an object into a menu
     *
     * @return true if object is a menu, false if just a single item
     */
    public boolean isMenu() {
        if(delegate!=null)
            return(delegate.isMenu());
        
        return(isSubmenu);
    }
    
    /**
     * Mark a menu item as designated for a submenu prior to the insertion of any elements into it.
     * This should only be used by VCLMenu constructors.
     *
     * @throws AWTPeersInvalidatedException
     */
    protected void makeMenu() throws AWTPeersInvalidatedException {
        if(delegate!=null) {
            delegate.makeMenu();
            return;
        }
        isSubmenu=true;
        if(!awtPeers.isEmpty())
        {
            awtPeers.clear();
            throw new AWTPeersInvalidatedException();
        }
    }
    
    /**
     * Add in a new menu item at a particular position in the menu.  Note that adding a menu item may
     * result in invalidating the AWT peers as it indicates the item must in fact be a menu.
     *
     * @param newItem	item to be added into the menu
     * @param nPos	position at which the item should be inserted.  Any item in that position or
     *			occuring after that position will be pushed down in the menu order
     * @throws IllegalArgumentException if the item cannot be added.  Separators or checked items cannot
     *		be transfigured into menus
     * @throws AWTPeersInvalidatedException if any underlying peers had to be destroyed due to the change
     */
    synchronized public void addMenuItem(VCLMenuItemData newItem, int nPos) throws IllegalArgumentException, AWTPeersInvalidatedException {
        if(delegate!=null) {
            delegate.addMenuItem(newItem, nPos);
            return;
        }
        
        if((nPos < 0) || (nPos == 65535))
            nPos=menuItems.size();
        
        if(isSeparator || isCheckbox || (nPos > menuItems.size()))
            throw new IllegalArgumentException();
        
        menuItems.insertElementAt(newItem, nPos);
        if(!isSubmenu)
        {
            if(!awtPeers.isEmpty())
            {
                awtPeers.clear();
                throw new AWTPeersInvalidatedException();
            }
        }
        else
        {
            Enumeration e=awtPeers.elements();
            while(e.hasMoreElements())
            {
                Menu m=(Menu)e.nextElement();
                
                if(nPos==menuItems.size()-1)
                {
                    // we can just append onto the end
                    
                    m.add((MenuItem)newItem.createAWTPeer());
                }
                else
                {
                    // we can't insert items in the middle of AWT menus, so we need to remove all of the
                    // existing items and reinsert them all
                    
                    Stack s=new Stack();
                    for(int i=m.countItems()-1; i>=0; i--)
                    {
                        s.push(m.getItem(i));
                        m.remove(i);
                        if(i==nPos)
                            s.push(newItem.createAWTPeer());
                    }
                    
                    while(!s.empty())
                    {
                        m.add((MenuItem)s.pop());
                    }
                }
            }
        }
    }
    
    /**
     * Remove a menu item at a particular position.  This only applies for menu style menu items.
     *
     * @param nPos	position of item to delete
     * @throws IllegalArgumentException if the menu item is of the incorrect type
     */
    synchronized public void removeMenuItem(int nPos) throws IllegalArgumentException {
        if(delegate!=null) {
            delegate.removeMenuItem(nPos);
            return;
        }
        
        if((nPos < 0) || (nPos == 65535))
            nPos=menuItems.size();
        
        if(!isSubmenu || (isSubmenu && (nPos >= menuItems.size())))
            throw new IllegalArgumentException();
        
        menuItems.removeElementAt(nPos);
        if(!awtPeers.isEmpty())
        {
            Enumeration e=awtPeers.elements();
            while(e.hasMoreElements())
            {
                Menu m=(Menu)e.nextElement();
                m.remove(nPos);
            }
        }
    }
    
    /**
     * Retrieve a menu item at a particular position.  This only applies for menu style menu items.
     *
     * @param nPos	position of item to retrieve
     * @throws IllegalArgumentException if the menu item is of the incorrect type
     */
    public VCLMenuItemData getMenuItem(int nPos) throws IllegalArgumentException {
        if(delegate!=null) {
            return(delegate.getMenuItem(nPos));
        }
        
        if(!isSubmenu || (isSubmenu && (nPos >= menuItems.size())))
            throw new IllegalArgumentException();
        
        return((VCLMenuItemData)menuItems.elementAt(nPos));
    }
    
    /**
     * Fetch the number of menu items in this menu
     *
     * @return number of menu item elements
     * @throws IllegalArgumentException if the item is not a menu
     */
    public int getNumMenuItems() throws IllegalArgumentException {
        if(delegate!=null) {
            return(delegate.getNumMenuItems());
        }
        
        if(!isSubmenu)
            throw new IllegalArgumentException();
        
        return(menuItems.size());
    }
    
    /**
     * Subclass of AWT MenuItem that allows the item to respond to AWT menu selections by posting
     * appropriate events into the VCL event queue.
     *
     * @see MenuItem
     * @see VCLEventQueue
     */
    static final class VCLAWTMenuItem extends MenuItem implements ActionListener {
        /**
         * Menu item data associated with this AWT item
         */
        private VCLMenuItemData d;
        
        /**
         * Construct a new <code>VCLAWTMenuItem</code> instance
         *
         * @param title		initial title of the menu item (may be changed later)
         * @param data		VCLMenuItemData holding the information needed to bind the AWT item to a VCL item
         */
        public VCLAWTMenuItem(String title, VCLMenuItemData data) {
            super(title);
            d=data;
            addActionListener(this);
        }
        
        /**
         * Respond to menu item choices by posting appropriate events into the queue.
         *
         * @param e	event spawning this action.  Ignored.
         */
        public void actionPerformed(ActionEvent e) {
            VCLMenuBar mb=VCLMenuBar.findVCLMenuBar(this);
            if(mb!=null) {
                mb.getEventQueue().postCachedEvent(new VCLEvent(VCLEvent.SALEVENT_MENUCOMMAND, mb.getFrame(), d.getVCLID(), d.getVCLCookie()));
            }
            else
            {
                System.err.println("MenuItem chosen, but no VCLFrame target found!");
            }
        }
    }
    
    /**
     * Subclass of AWT CheckboxMenuItem that allows the item to respond to AWT menu selections by posting
     * appropriate events into the VCL event queue.
     *
     * @see CheckboxMenuItem
     * @see VCLEventQueue
     */
    static final class VCLAWTCheckboxMenuItem extends CheckboxMenuItem implements ActionListener {
        /**
         * Menu item data associated with this AWT item
         */
        private VCLMenuItemData d;
        
        /**
         * Construct a new <code>VCLAWTCheckboxMenuItem</code> instance
         *
         * @param title		initial title of the menu item (may be changed later)
         * @param data		VCLMenuItemData holding the information needed to bind the AWT item to a VCL item
         */
        public VCLAWTCheckboxMenuItem(String title, VCLMenuItemData data) {
            super(title);
            d=data;
            addActionListener(this);
        }
        
        /**
         * Respond to menu item choices by posting appropriate events into the queue.
         *
         * @param e	event spawning this action.  Ignored.
         */
        public void actionPerformed(ActionEvent e) {
            VCLMenuBar mb=VCLMenuBar.findVCLMenuBar(this);
            if(mb!=null) {
                mb.getEventQueue().postCachedEvent(new VCLEvent(VCLEvent.SALEVENT_MENUCOMMAND, mb.getFrame(), d.getVCLID(), d.getVCLCookie()));
            }
            else
            {
                System.err.println("MenuItem chosen, but no VCLFrame target found!");
            }
        }
    }
    
    /**
     * Using all of the current settings, construct an AWT object that is most appropriate for these settings.
     * While the resulting object is guaranteed to be a reference to a MenuItem or one of its subclasses, up to
     * three types may be returned:  MenuItem, CheckboxMenuItem, and Menu.  The returned object will be added
     * onto internal queues to be automatically synchronized with changes made to this menu item.
     *
     * @return AWT MenuItem for this object
     */
    synchronized public Object createAWTPeer() {
        if(delegate!=null)
            return(delegate.createAWTPeer());
            
        Object toReturn=null;
        
        if(isCheckbox)
        {
            VCLAWTCheckboxMenuItem cmi=new VCLAWTCheckboxMenuItem(getTitle(), this);
            if(getEnabled())
                cmi.enable();
            else
                cmi.disable();
            cmi.setState(getChecked());
            if(keyboardShortcutSet)
                cmi.setShortcut(new MenuShortcut(keyboardShortcut, keyboardShortcutUseShift));
            toReturn=(Object)cmi;
        }
        else if(isSubmenu)
        {
            Menu mn=new Menu(getTitle());
            if(getEnabled())
                mn.enable();
            else
                mn.disable();
            Enumeration items=menuItems.elements();
            while(items.hasMoreElements()) {
                VCLMenuItemData i=(VCLMenuItemData)items.nextElement();
                mn.add((MenuItem)i.createAWTPeer());
            }
            toReturn=(Object)mn;
        }
        else if(isSeparator)
        {
            // separator is a menu item with a label of a dash
            MenuItem sep=new MenuItem("-");
            toReturn=(Object)sep;
        }
        else
        {
            VCLAWTMenuItem mi=new VCLAWTMenuItem(getTitle(), this);
            if(getEnabled())
                mi.enable();
            else
                mi.disable();
            if(keyboardShortcutSet)
                mi.setShortcut(new MenuShortcut(keyboardShortcut, keyboardShortcutUseShift));
            toReturn=(Object)mi;
        }
        
        // add the new peer onto our internal tracking lists
        
        if(toReturn!=null)
            awtPeers.add(toReturn);
        
        return(toReturn);
    }
    
    /**
     * Unregister a single AWT peer.  This indicates that the AWT object is no longer being used and
     * does not need to be tracked any longer.
     *
     * @param o		peer object to stop managing
     */
    synchronized public void unregisterAWTPeer(Object o) {
        if(delegate!=null) {
            delegate.unregisterAWTPeer(o);
            return;
        }
        
        awtPeers.removeElement(o);
    }
    
    /**
     * Unregister all AWT peer objects that have been created from the menu item data.  If the
     * item data corresponds to a submenu, all of the submenu peers will also be unregistered.
     */
    synchronized public void unregisterAllAWTPeers() {
        if(delegate!=null) {
            delegate.unregisterAllAWTPeers();
            return;
        }
        
	// remove notifiers to allow GC to reclaim these objects quicker
	
	Enumeration peers=awtPeers.elements();
	while(peers.hasMoreElements()) {
	    MenuItem mi=(MenuItem)peers.nextElement();
	    if(mi instanceof VCLAWTMenuItem) {
		mi.removeActionListener((VCLAWTMenuItem)mi);
	    } else if(mi instanceof VCLAWTCheckboxMenuItem) {
		mi.removeActionListener((VCLAWTCheckboxMenuItem)mi);
	    }
	}
	
        awtPeers.removeAllElements();
        if(isSubmenu) {
            Enumeration e=menuItems.elements();
            while(e.hasMoreElements()) {
                ((VCLMenuItemData)e.nextElement()).unregisterAllAWTPeers();
            }
        }
    }
}