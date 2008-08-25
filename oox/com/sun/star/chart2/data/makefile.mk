#*************************************************************************
#
# Copyright 2008 by Planamesa Inc.
#
# NeoOffice - a multi-platform office productivity suite
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
#*************************************************************************

PRJ=..$/..$/..$/..$/..

PRJNAME=oox

TARGET=chart2data
PACKAGE=com$/sun$/star$/chart2$/data

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# ------------------------------------------------------------------------

IDLFILES= \
	DataSequenceRole.idl \
	HighlightedRange.idl \
	LabelOrigin.idl \
	XDataProvider.idl \
	XDataReceiver.idl \
	XDataSequence.idl \
	XDataSink.idl \
	XDataSource.idl \
	XLabeledDataSequence.idl \
	XNumericalDataSequence.idl \
	XRangeHighlighter.idl \
	XRangeXMLConversion.idl \
	XTextualDataSequence.idl \
	XDatabaseDataProvider.idl

# ------------------------------------------------------------------

.INCLUDE :  target.mk
.INCLUDE :  $(PRJ)$/util$/target.pmk
