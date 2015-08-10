#*************************************************************************
#
# Copyright 2008 by Sun Microsystems, Inc.
#
# $RCSfile$
#
# $Revision$
#
# This file is part of NeoOffice.
#
# NeoOffice is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 3
# only, as published by the Free Software Foundation.
#
# NeoOffice is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU General Public License
# version 3 along with NeoOffice.  If not, see
# <http://www.gnu.org/licenses/gpl-3.0.txt>
# for a copy of the GPLv3 License.
#
# Modified August 2015 by Patrick Luby. NeoOffice is distributed under
# GPL only under modification term 2 of the LGPL.
#
#*************************************************************************

PRJ=..$/..$/..$/..

PRJNAME=offapi

TARGET=cssembed
PACKAGE=com$/sun$/star$/embed

# --- Settings -----------------------------------------------------

.IF "$(UPD)" == "310"
.INCLUDE :  settings.mk
.ENDIF		# "$(UPD)" == "310"
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

.IF "$(UPD)" == "310"
# Add locally built types registry to cppumaker search path
UNOUCRRDB+=$(OUT)$/ucr$/$(TARGET).db
.ENDIF		# "$(UPD)" == "310"

# ------------------------------------------------------------------------

IDLFILES=\
	Actions.idl\
	Aspects.idl\
	BaseStorage.idl\
	ElementModes.idl\
	EmbedStates.idl\
	EmbedVerbs.idl\
	EmbedMapUnits.idl\
	EmbedMisc.idl\
	EmbedUpdateModes.idl\
	EmbeddedObjectDescriptor.idl\
	EntryInitModes.idl\
	DocumentCloser.idl\
	FileSystemStorage.idl\
	FileSystemStorageFactory.idl\
	InsertedObjectInfo.idl\
	InstanceLocker.idl\
	Storage.idl\
	StorageStream.idl\
	StorageFactory.idl\
	VerbAttributes.idl\
	VisualRepresentation.idl\
	VerbDescriptor.idl\
	XActionsApproval.idl\
	XPersistanceHolder.idl\
	XEmbeddedObject.idl\
	XVisualObject.idl\
	XCommonEmbedPersist.idl\
	XEmbedPersist.idl\
	XLinkageSupport.idl\
	XClassifiedObject.idl\
	XInplaceObject.idl\
	XEmbeddedClient.idl\
	XEmbedObjectClipboardCreator.idl\
	XEmbedObjectCreator.idl\
	XEmbedObjectFactory.idl\
	XLinkCreator.idl\
	XLinkFactory.idl\
	XEncryptionProtectedSource.idl\
	XInplaceClient.idl\
	XInsertObjectDialog.idl\
	XWindowSupplier.idl\
	XTransactedObject.idl\
	XTransactionBroadcaster.idl\
	XTransactionListener.idl\
	XRelationshipAccess.idl\
	XStateChangeBroadcaster.idl\
	XStateChangeListener.idl\
	XTransferableSupplier.idl\
	XComponentSupplier.idl\
	XStorage.idl\
	XStorageRawAccess.idl\
	XExtendedStorageStream.idl\
	XHierarchicalStorageAccess.idl\
	XHatchWindowController.idl\
	XHatchWindowFactory.idl\
	XHatchWindow.idl\
	XPackageStructureCreator.idl\
	XOptimizedStorage.idl\
	UnreachableStateException.idl\
	UseBackupException.idl\
	StateChangeInProgressException.idl\
	WrongStateException.idl\
	NoVisualAreaSizeException.idl\
	ObjectSaveVetoException.idl\
	InvalidStorageException.idl\
	LinkageMisuseException.idl\
	NeedsRunningStateException.idl\
	StorageWrappedTargetException.idl\
	OLESimpleStorage.idl\
	XOLESimpleStorage.idl

.IF "$(UPD)" == "310"

IDLFILES += \
	StorageFormats.idl \
	XEncryptionProtectedSource2.idl \
	XEncryptionProtectedStorage.idl \
	XHierarchicalStorageAccess2.idl \
	XStorage2.idl

UNOTYPES= \
	com.sun.star.embed.StorageFormats \
	com.sun.star.embed.XEncryptionProtectedSource2 \
	com.sun.star.embed.XEncryptionProtectedStorage \
	com.sun.star.embed.XHierarchicalStorageAccess2 \
	com.sun.star.embed.XStorage2

# Force creation of the IDL header files before the compiling source files
UNOUCRDEP=$(OUT)$/ucr$/$(TARGET).db

.ENDIF		# "$(UPD)" == "310"

# ------------------------------------------------------------------

.INCLUDE :  target.mk
.INCLUDE :  $(PRJ)$/util$/target.pmk
