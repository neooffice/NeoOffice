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
 *  Patrick Luby, July 2006
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2006 by Patrick Luby (patrick.luby@planamesa.com)
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

#ifndef _JAVA_SERVICE_HXX_
#define _JAVA_SERVICE_HXX_

// Service names
#define FILE_PICKER_SERVICE_NAME "com.sun.star.ui.dialogs.SystemFilePicker"
#define FOLDER_PICKER_SERVICE_NAME "com.sun.star.ui.dialogs.SystemFolderPicker"

// Implementation names		
#define FILE_PICKER_IMPL_NAME "com.sun.star.ui.dialogs.JavaFilePicker"
#define FOLDER_PICKER_IMPL_NAME "com.sun.star.ui.dialogs.JavaFolderPicker"

// The registry key names
#define FILE_PICKER_REGKEY_NAME "/com.sun.star.ui.dialogs.JavaFilePicker/UNO/SERVICES/com.sun.star.ui.dialogs.SystemFilePicker"
#define FOLDER_PICKER_REGKEY_NAME  "/com.sun.star.ui.dialogs.JavaFolderPicker/UNO/SERVICES/com.sun.star.ui.dialogs.SystemFolderPicker"

#endif	// _JAVA_SERVICE_HXX_
