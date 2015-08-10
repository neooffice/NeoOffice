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

TARGET=csstask
PACKAGE=com$/sun$/star$/task

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
	ClassifiedInteractionRequest.idl\
	DocumentMacroConfirmationRequest.idl\
	DocumentPasswordRequest.idl\
	ErrorCodeRequest.idl\
	ErrorCodeIOException.idl\
	FutureDocumentVersionProductUpdateRequest.idl\
	InteractionClassification.idl\
	InteractionHandler.idl\
	JobExecutor.idl\
	Job.idl\
	AsyncJob.idl\
	MasterPasswordRequest.idl\
	NoMasterException.idl\
	PasswordContainer.idl\
	PasswordRequest.idl\
	PasswordRequestMode.idl\
	UnsupportedOverwriteRequest.idl\
	UrlRecord.idl\
	UserRecord.idl\
	XAsyncJob.idl\
	XInteractionApprove.idl\
	XInteractionAskLater.idl\
	XInteractionDisapprove.idl\
	XInteractionPassword.idl\
	XJob.idl\
	XJobExecutor.idl\
	XJobListener.idl\
	XMasterPasswordHandling.idl\
	XMasterPasswordHandling2.idl\
	XPasswordContainer.idl\
	XStatusIndicator.idl\
	XStatusIndicatorFactory.idl\
	XStatusIndicatorSupplier.idl\
	XAbortChannel.idl\
	XInteractionRequestStringResolver.idl\
	InteractionRequestStringResolver.idl

.IF "$(UPD)" == "310"

IDLFILES += \
	DocumentMSPasswordRequest.idl \
	DocumentMSPasswordRequest2.idl \
	DocumentPasswordRequest2.idl \
	XInteractionPassword2.idl

UNOTYPES= \
	com.sun.star.task.DocumentMSPasswordRequest \
	com.sun.star.task.DocumentMSPasswordRequest2 \
	com.sun.star.task.DocumentPasswordRequest2 \
	com.sun.star.task.XInteractionPassword2

# Force creation of the IDL header files before the compiling source files
UNOUCRDEP=$(OUT)$/ucr$/$(TARGET).db

.ENDIF		# "$(UPD)" == "310"

# ------------------------------------------------------------------

.INCLUDE :  target.mk
.INCLUDE :  $(PRJ)$/util$/target.pmk
