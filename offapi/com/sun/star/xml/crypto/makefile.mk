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

#i20156 - new file for xmlsecurity module

PRJ=..$/..$/..$/..$/..

PRJNAME=offapi

TARGET=xsec-crypto
PACKAGE=com$/sun$/star$/xml$/crypto

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
    XXMLSecurityTemplate.idl     \
    XXMLSignature.idl     \
    XXMLSignatureTemplate.idl     \
    XXMLEncryption.idl     \
    XXMLEncryptionTemplate.idl     \
    XXMLSecurityContext.idl     \
    XSecurityEnvironment.idl     \
    XSEInitializer.idl     \
    XMLSignature.idl     \
    XMLSignatureTemplate.idl     \
    XMLEncryption.idl     \
    XMLEncryptionTemplate.idl     \
    XMLSecurityContext.idl     \
    SecurityEnvironment.idl     \
    SEInitializer.idl     \
    XMLSignatureException.idl     \
    XMLEncryptionException.idl     \
    XUriBinding.idl    \
    SecurityOperationStatus.idl

.IF "$(UPD)" == "310"

IDLFILES += \
	CipherID.idl \
	DigestID.idl \
	XCipherContext.idl \
	XCipherContextSupplier.idl \
	XDigestContext.idl \
	XDigestContextSupplier.idl \
	XNSSInitializer.idl

UNOTYPES= \
	com.sun.star.xml.crypto.CipherID \
	com.sun.star.xml.crypto.DigestID \
	com.sun.star.xml.crypto.XCipherContext \
	com.sun.star.xml.crypto.XCipherContextSupplier \
	com.sun.star.xml.crypto.XDigestContext \
	com.sun.star.xml.crypto.XDigestContextSupplier \
	com.sun.star.xml.crypto.XNSSInitializer

# Force creation of the IDL header files before the compiling source files
UNOUCRDEP=$(OUT)$/ucr$/$(TARGET).db

.ENDIF		# "$(UPD)" == "310"

# ------------------------------------------------------------------

.INCLUDE :  target.mk
.INCLUDE :  $(PRJ)$/util$/target.pmk
