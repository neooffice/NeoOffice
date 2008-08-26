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

PRJ=..$/..$/..

PRJNAME=oox

TARGET=api
PACKAGE=com$/sun$/star

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# ------------------------------------------------------------------------

UNOTYPES=\
	com.sun.star.chart.DataLabelPlacement \
	com.sun.star.chart.ErrorBarStyle \
	com.sun.star.chart2.AxisType \
	com.sun.star.chart2.AxisOrientation \
	com.sun.star.chart2.AxisPosition \
	com.sun.star.chart2.Break \
	com.sun.star.chart2.CoordinateSystemTypeID \
	com.sun.star.chart2.CurveStyle \
	com.sun.star.chart2.DataPointGeometry3D \
	com.sun.star.chart2.DataPointLabel \
	com.sun.star.chart2.ExplicitIncrementData \
	com.sun.star.chart2.ExplicitScaleData \
	com.sun.star.chart2.ExplicitSubIncrement \
	com.sun.star.chart2.FillBitmap \
	com.sun.star.chart2.IncrementData \
	com.sun.star.chart2.InterpretedData \
	com.sun.star.chart2.LegendExpansion \
	com.sun.star.chart2.LegendPosition \
	com.sun.star.chart2.LegendSymbolStyle \
	com.sun.star.chart2.LightSource \
	com.sun.star.chart2.PieChartOffsetMode \
	com.sun.star.chart2.RelativePosition \
	com.sun.star.chart2.RelativeSize \
	com.sun.star.chart2.ScaleData \
	com.sun.star.chart2.StackingDirection \
	com.sun.star.chart2.SubIncrement \
	com.sun.star.chart2.Symbol \
	com.sun.star.chart2.SymbolStyle \
	com.sun.star.chart2.TickmarkStyle \
	com.sun.star.chart2.TransparencyStyle \
	com.sun.star.chart2.ViewLegendEntry \
	com.sun.star.chart2.XAxis \
	com.sun.star.chart2.XCoordinateSystem \
	com.sun.star.chart2.XCoordinateSystemContainer \
	com.sun.star.chart2.XChartDocument \
	com.sun.star.chart2.XChartShape \
	com.sun.star.chart2.XChartShapeContainer \
	com.sun.star.chart2.XChartType \
	com.sun.star.chart2.XChartTypeContainer \
	com.sun.star.chart2.XChartTypeManager \
	com.sun.star.chart2.XChartTypeTemplate \
	com.sun.star.chart2.XColorScheme \
	com.sun.star.chart2.XDataInterpreter \
	com.sun.star.chart2.XDataSeries \
	com.sun.star.chart2.XDataSeriesContainer \
	com.sun.star.chart2.XDiagram \
	com.sun.star.chart2.XDiagramProvider \
	com.sun.star.chart2.XFormattedString \
	com.sun.star.chart2.XInternalDataProvider \
	com.sun.star.chart2.XLabeled \
	com.sun.star.chart2.XLegend \
	com.sun.star.chart2.XLegendEntry \
	com.sun.star.chart2.XLegendSymbolProvider \
	com.sun.star.chart2.XPlotter \
	com.sun.star.chart2.XRegressionCurve \
	com.sun.star.chart2.XRegressionCurveCalculator \
	com.sun.star.chart2.XRegressionCurveContainer \
	com.sun.star.chart2.XScaling \
	com.sun.star.chart2.XTarget \
	com.sun.star.chart2.XTitle \
	com.sun.star.chart2.XTitled \
	com.sun.star.chart2.XTransformation \
	com.sun.star.chart2.XUndoManager \
	com.sun.star.chart2.XUndoSupplier \
	com.sun.star.chart2.XUndoHelper \
	com.sun.star.chart2.data.DataSequenceRole \
	com.sun.star.chart2.data.HighlightedRange \
	com.sun.star.chart2.data.LabelOrigin \
	com.sun.star.chart2.data.XDataProvider \
	com.sun.star.chart2.data.XDataReceiver \
	com.sun.star.chart2.data.XDataSequence \
	com.sun.star.chart2.data.XDataSink \
	com.sun.star.chart2.data.XDataSource \
	com.sun.star.chart2.data.XLabeledDataSequence \
	com.sun.star.chart2.data.XNumericalDataSequence \
	com.sun.star.chart2.data.XRangeHighlighter \
	com.sun.star.chart2.data.XRangeXMLConversion \
	com.sun.star.chart2.data.XTextualDataSequence \
	com.sun.star.chart2.data.XDatabaseDataProvider \
	com.sun.star.document.XDocumentProperties \
	com.sun.star.document.XOOXMLDocumentPropertiesImporter \
	com.sun.star.drawing.XEnhancedCustomShapeDefaulter \
	com.sun.star.graphic.XGraphicTransformer \
	com.sun.star.sheet.AccessibleCell \
	com.sun.star.sheet.AccessibleCsvCell \
	com.sun.star.sheet.AccessibleCsvRuler \
	com.sun.star.sheet.AccessibleCsvTable \
	com.sun.star.sheet.AccessiblePageHeaderFooterAreasView \
	com.sun.star.sheet.AccessibleSpreadsheet \
	com.sun.star.sheet.AccessibleSpreadsheetDocumentView \
	com.sun.star.sheet.AccessibleSpreadsheetPageView \
	com.sun.star.sheet.AddIn \
	com.sun.star.sheet.AddressConvention \
	com.sun.star.sheet.CellAnnotation \
	com.sun.star.sheet.CellAnnotationShape \
	com.sun.star.sheet.CellAnnotations \
	com.sun.star.sheet.CellAnnotationsEnumeration \
	com.sun.star.sheet.CellAreaLink \
	com.sun.star.sheet.CellAreaLinks \
	com.sun.star.sheet.CellAreaLinksEnumeration \
	com.sun.star.sheet.CellFormatRanges \
	com.sun.star.sheet.CellFormatRangesEnumeration \
	com.sun.star.sheet.Cells \
	com.sun.star.sheet.CellsEnumeration \
	com.sun.star.sheet.ComplexReference \
	com.sun.star.sheet.ConsolidationDescriptor \
	com.sun.star.sheet.DDELink \
	com.sun.star.sheet.DDELinkMode \
	com.sun.star.sheet.DDELinks \
	com.sun.star.sheet.DDELinksEnumeration \
	com.sun.star.sheet.DataPilotDescriptor \
	com.sun.star.sheet.DataPilotField \
	com.sun.star.sheet.DataPilotFieldFilter \
	com.sun.star.sheet.DataPilotFieldGroup \
	com.sun.star.sheet.DataPilotFieldGroupItem \
	com.sun.star.sheet.DataPilotFieldGroups \
	com.sun.star.sheet.DataPilotFields \
	com.sun.star.sheet.DataPilotFieldsEnumeration \
	com.sun.star.sheet.DataPilotItem \
	com.sun.star.sheet.DataPilotItems \
	com.sun.star.sheet.DataPilotItemsEnumeration \
	com.sun.star.sheet.DataPilotOutputRangeType \
	com.sun.star.sheet.DataPilotSource \
	com.sun.star.sheet.DataPilotSourceDimension \
	com.sun.star.sheet.DataPilotSourceDimensions \
	com.sun.star.sheet.DataPilotSourceHierarchies \
	com.sun.star.sheet.DataPilotSourceHierarchy \
	com.sun.star.sheet.DataPilotSourceLevel \
	com.sun.star.sheet.DataPilotSourceLevels \
	com.sun.star.sheet.DataPilotSourceMember \
	com.sun.star.sheet.DataPilotSourceMembers \
	com.sun.star.sheet.DataPilotTable \
	com.sun.star.sheet.DataPilotTableHeaderData \
	com.sun.star.sheet.DataPilotTablePositionData \
	com.sun.star.sheet.DataPilotTablePositionType \
	com.sun.star.sheet.DataPilotTableResultData \
	com.sun.star.sheet.DataPilotTables \
	com.sun.star.sheet.DataPilotTablesEnumeration \
	com.sun.star.sheet.DatabaseImportDescriptor \
	com.sun.star.sheet.DatabaseRange \
	com.sun.star.sheet.DatabaseRanges \
	com.sun.star.sheet.DatabaseRangesEnumeration \
	com.sun.star.sheet.DocumentSettings \
	com.sun.star.sheet.FormulaLanguage \
	com.sun.star.sheet.FormulaMapGroup \
	com.sun.star.sheet.FormulaMapGroupSpecialOffset \
	com.sun.star.sheet.FormulaOpCodeMapEntry \
	com.sun.star.sheet.FormulaParser \
	com.sun.star.sheet.FormulaToken \
	com.sun.star.sheet.FunctionAccess \
	com.sun.star.sheet.FunctionDescription \
	com.sun.star.sheet.FunctionDescriptionEnumeration \
	com.sun.star.sheet.FunctionDescriptions \
	com.sun.star.sheet.GlobalSheetSettings \
	com.sun.star.sheet.HeaderFooterContent \
	com.sun.star.sheet.LabelRange \
	com.sun.star.sheet.LabelRanges \
	com.sun.star.sheet.LabelRangesEnumeration \
	com.sun.star.sheet.NamedRange \
	com.sun.star.sheet.NamedRanges \
	com.sun.star.sheet.NamedRangesEnumeration \
	com.sun.star.sheet.RangeSelectionArguments \
	com.sun.star.sheet.RecentFunctions \
	com.sun.star.sheet.ReferenceFlags \
	com.sun.star.sheet.Scenario \
	com.sun.star.sheet.Scenarios \
	com.sun.star.sheet.ScenariosEnumeration \
	com.sun.star.sheet.Shape \
	com.sun.star.sheet.SheetCell \
	com.sun.star.sheet.SheetCellCursor \
	com.sun.star.sheet.SheetCellRange \
	com.sun.star.sheet.SheetCellRanges \
	com.sun.star.sheet.SheetCellRangesEnumeration \
	com.sun.star.sheet.SheetFilterDescriptor \
	com.sun.star.sheet.SheetLink \
	com.sun.star.sheet.SheetLinks \
	com.sun.star.sheet.SheetLinksEnumeration \
	com.sun.star.sheet.SheetRangesQuery \
	com.sun.star.sheet.SheetSortDescriptor \
	com.sun.star.sheet.SheetSortDescriptor2 \
	com.sun.star.sheet.SingleReference \
	com.sun.star.sheet.Solver \
	com.sun.star.sheet.SolverConstraint \
	com.sun.star.sheet.SolverConstraintOperator \
	com.sun.star.sheet.Spreadsheet \
	com.sun.star.sheet.SpreadsheetDocument \
	com.sun.star.sheet.SpreadsheetDocumentSettings \
	com.sun.star.sheet.SpreadsheetDrawPage \
	com.sun.star.sheet.SpreadsheetView \
	com.sun.star.sheet.SpreadsheetViewPane \
	com.sun.star.sheet.SpreadsheetViewPanesEnumeration \
	com.sun.star.sheet.SpreadsheetViewSettings \
	com.sun.star.sheet.Spreadsheets \
	com.sun.star.sheet.SpreadsheetsEnumeration \
	com.sun.star.sheet.SubTotalDescriptor \
	com.sun.star.sheet.SubTotalField \
	com.sun.star.sheet.SubTotalFieldsEnumeration \
	com.sun.star.sheet.TableAutoFormat \
	com.sun.star.sheet.TableAutoFormatEnumeration \
	com.sun.star.sheet.TableAutoFormatField \
	com.sun.star.sheet.TableAutoFormats \
	com.sun.star.sheet.TableAutoFormatsEnumeration \
	com.sun.star.sheet.TableCellStyle \
	com.sun.star.sheet.TableConditionalEntry \
	com.sun.star.sheet.TableConditionalEntryEnumeration \
	com.sun.star.sheet.TableConditionalFormat \
	com.sun.star.sheet.TablePageStyle \
	com.sun.star.sheet.TableValidation \
	com.sun.star.sheet.UniqueCellFormatRanges \
	com.sun.star.sheet.UniqueCellFormatRangesEnumeration \
	com.sun.star.sheet.VolatileResult \
	com.sun.star.sheet.XArrayFormulaTokens \
	com.sun.star.sheet.XCellRangesAccess \
	com.sun.star.sheet.XDDELinkResults \
	com.sun.star.sheet.XDDELinks \
	com.sun.star.sheet.XDataPilotTable2 \
	com.sun.star.sheet.XDrillDownDataSupplier \
	com.sun.star.sheet.XExternalSheetName \
	com.sun.star.sheet.XFormulaOpCodeMapper \
	com.sun.star.sheet.XFormulaParser \
	com.sun.star.sheet.XFormulaTokens \
	com.sun.star.sheet.XMultiFormulaTokens \
	com.sun.star.sheet.XSolver \
	com.sun.star.sheet.XSolverDescription \
	com.sun.star.table.XTable \
	com.sun.star.table.XMergeableCellRange \
	com.sun.star.xml.Attribute \
	com.sun.star.xml.sax.XSAXSerializable \
	com.sun.star.xml.sax.XFastParser \
	com.sun.star.xml.sax.XFastDocumentHandler \
	com.sun.star.xml.sax.XFastContextHandler \
	com.sun.star.xml.sax.XFastShapeContextHandler \
	com.sun.star.xml.sax.XFastTokenHandler \
	com.sun.star.xml.sax.XFastAttributeList \
	com.sun.star.xml.sax.FastToken \
	com.sun.star.xml.sax.FastShapeContextHandler \
	com.sun.star.xml.sax.FastTokenHandler 

UNOUCROUT=$(OUT)$/inc
UNOUCRRDB+=$(shell -+$(FIND) $(OUT)$/ucr -name "*.db")

# ------------------------------------------------------------------

.INCLUDE :  target.mk
.INCLUDE :  $(PRJ)$/util$/target.pmk
