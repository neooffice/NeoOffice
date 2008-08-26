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

PRJ=..$/..$/..$/..

PRJNAME=oox

TARGET=sheet
PACKAGE=com$/sun$/star$/sheet

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# ------------------------------------------------------------------------

IDLFILES= \
	AccessibleCell.idl \
	AccessibleCsvCell.idl \
	AccessibleCsvRuler.idl \
	AccessibleCsvTable.idl \
	AccessiblePageHeaderFooterAreasView.idl \
	AccessibleSpreadsheet.idl \
	AccessibleSpreadsheetDocumentView.idl \
	AccessibleSpreadsheetPageView.idl \
	AddIn.idl \
	AddressConvention.idl \
	CellAnnotation.idl \
	CellAnnotationShape.idl \
	CellAnnotations.idl \
	CellAnnotationsEnumeration.idl \
	CellAreaLink.idl \
	CellAreaLinks.idl \
	CellAreaLinksEnumeration.idl \
	CellFormatRanges.idl \
	CellFormatRangesEnumeration.idl \
	Cells.idl \
	CellsEnumeration.idl \
	ComplexReference.idl \
	ConsolidationDescriptor.idl \
	DDELink.idl \
	DDELinkMode.idl \
	DDELinks.idl \
	DDELinksEnumeration.idl \
	DataPilotDescriptor.idl \
	DataPilotField.idl \
	DataPilotFieldFilter.idl \
	DataPilotFieldGroup.idl \
	DataPilotFieldGroupItem.idl \
	DataPilotFieldGroups.idl \
	DataPilotFields.idl \
	DataPilotFieldsEnumeration.idl \
	DataPilotItem.idl \
	DataPilotItems.idl \
	DataPilotItemsEnumeration.idl \
	DataPilotOutputRangeType.idl \
	DataPilotSource.idl \
	DataPilotSourceDimension.idl \
	DataPilotSourceDimensions.idl \
	DataPilotSourceHierarchies.idl \
	DataPilotSourceHierarchy.idl \
	DataPilotSourceLevel.idl \
	DataPilotSourceLevels.idl \
	DataPilotSourceMember.idl \
	DataPilotSourceMembers.idl \
	DataPilotTable.idl \
	DataPilotTableHeaderData.idl \
	DataPilotTablePositionData.idl \
	DataPilotTablePositionType.idl \
	DataPilotTableResultData.idl \
	DataPilotTables.idl \
	DataPilotTablesEnumeration.idl \
	DatabaseImportDescriptor.idl \
	DatabaseRange.idl \
	DatabaseRanges.idl \
	DatabaseRangesEnumeration.idl \
	DocumentSettings.idl \
	FormulaLanguage.idl \
	FormulaMapGroup.idl \
	FormulaMapGroupSpecialOffset.idl \
	FormulaOpCodeMapEntry.idl \
	FormulaParser.idl \
	FormulaToken.idl \
	FunctionAccess.idl \
	FunctionDescription.idl \
	FunctionDescriptionEnumeration.idl \
	FunctionDescriptions.idl \
	GlobalSheetSettings.idl \
	HeaderFooterContent.idl \
	LabelRange.idl \
	LabelRanges.idl \
	LabelRangesEnumeration.idl \
	NamedRange.idl \
	NamedRanges.idl \
	NamedRangesEnumeration.idl \
	RangeSelectionArguments.idl \
	RecentFunctions.idl \
	ReferenceFlags.idl \
	Scenario.idl \
	Scenarios.idl \
	ScenariosEnumeration.idl \
	Shape.idl \
	SheetCell.idl \
	SheetCellCursor.idl \
	SheetCellRange.idl \
	SheetCellRanges.idl \
	SheetCellRangesEnumeration.idl \
	SheetFilterDescriptor.idl \
	SheetLink.idl \
	SheetLinks.idl \
	SheetLinksEnumeration.idl \
	SheetRangesQuery.idl \
	SheetSortDescriptor.idl \
	SheetSortDescriptor2.idl \
	SingleReference.idl \
	Solver.idl \
	SolverConstraint.idl \
	SolverConstraintOperator.idl \
	Spreadsheet.idl \
	SpreadsheetDocument.idl \
	SpreadsheetDocumentSettings.idl \
	SpreadsheetDrawPage.idl \
	SpreadsheetView.idl \
	SpreadsheetViewPane.idl \
	SpreadsheetViewPanesEnumeration.idl \
	SpreadsheetViewSettings.idl \
	Spreadsheets.idl \
	SpreadsheetsEnumeration.idl \
	SubTotalDescriptor.idl \
	SubTotalField.idl \
	SubTotalFieldsEnumeration.idl \
	TableAutoFormat.idl \
	TableAutoFormatEnumeration.idl \
	TableAutoFormatField.idl \
	TableAutoFormats.idl \
	TableAutoFormatsEnumeration.idl \
	TableCellStyle.idl \
	TableConditionalEntry.idl \
	TableConditionalEntryEnumeration.idl \
	TableConditionalFormat.idl \
	TablePageStyle.idl \
	TableValidation.idl \
	UniqueCellFormatRanges.idl \
	UniqueCellFormatRangesEnumeration.idl \
	VolatileResult.idl \
	XArrayFormulaTokens.idl \
	XCellRangesAccess.idl \
	XDDELinkResults.idl \
	XDDELinks.idl \
	XDataPilotTable2.idl \
	XDrillDownDataSupplier.idl \
	XExternalSheetName.idl \
	XFormulaOpCodeMapper.idl \
	XFormulaParser.idl \
	XFormulaTokens.idl \
	XMultiFormulaTokens.idl \
	XSolver.idl \
	XSolverDescription.idl

# ------------------------------------------------------------------

.INCLUDE :  target.mk
.INCLUDE :  $(PRJ)$/util$/target.pmk
