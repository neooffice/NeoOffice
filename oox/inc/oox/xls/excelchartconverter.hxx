/**************************************************************
 * 
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 * 
 * This file incorporates work covered by the following license notice:
 *
 *   Portions of this file are part of the LibreOffice project.
 *
 *   This Source Code Form is subject to the terms of the Mozilla Public
 *   License, v. 2.0. If a copy of the MPL was not distributed with this
 *   file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 *************************************************************/



#ifndef OOX_XLS_EXCELCHARTCONVERTER_HXX
#define OOX_XLS_EXCELCHARTCONVERTER_HXX

#include "oox/drawingml/chart/chartconverter.hxx"
#include "oox/xls/workbookhelper.hxx"

namespace oox {
namespace xls {

// ============================================================================

class ExcelChartConverter : public ::oox::drawingml::chart::ChartConverter, public WorkbookHelper
{
public:
    explicit            ExcelChartConverter( const WorkbookHelper& rHelper );
    virtual             ~ExcelChartConverter();

    /** Creates an external data provider that is able to use spreadsheet data. */
    virtual void        createDataProvider(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartDocument >& rxChartDoc );

    /** Creates a data sequence from the passed formula. */
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XDataSequence >
                        createDataSequence(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XDataProvider >& rxDataProvider,
#if SUPD == 310
            const oox::drawingml::chart::DataSequenceModel& rDataSeq, const OUString& rRole ) SAL_OVERRIDE;
#else	// SUPD == 310
                            const ::oox::drawingml::chart::DataSequenceModel& rDataSeq );
#endif	// SUPD == 310
};

// ============================================================================

} // namespace xls
} // namespace oox

#endif
