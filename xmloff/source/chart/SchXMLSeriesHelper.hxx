/*************************************************************************
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * $RCSfile$
 * $Revision$
 *
 * This file is part of NeoOffice.
 *
 * NeoOffice is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * NeoOffice is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU General Public License
 * version 3 along with NeoOffice.  If not, see
 * <http://www.gnu.org/licenses/gpl-3.0.txt>
 * for a copy of the GPLv3 License.
 *
 * Modified October 2014 by Patrick Luby. NeoOffice is distributed under
 * GPL only under modification term 2 of the LGPL.
 *
 ************************************************************************/
#ifndef _XMLOFF_SCH_XML_SERIESHELPER_HXX
#define _XMLOFF_SCH_XML_SERIESHELPER_HXX

#if SUPD == 310

#include <xmloff/SchXMLSeriesHelper.hxx>

#else	// SUPD == 310

#include <com/sun/star/chart2/data/XDataSequence.hpp>
#include <com/sun/star/chart2/data/XDataSource.hpp>
#include <com/sun/star/chart2/XDataSeries.hpp>
#include <com/sun/star/chart2/XDiagram.hpp>
#include <com/sun/star/frame/XModel.hpp>

#include <vector>
#include <map>

class SchXMLSeriesHelper
{
public:
    static ::std::vector< ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XDataSeries > >
            getDataSeriesFromDiagram(
                const ::com::sun::star::uno::Reference<
                    ::com::sun::star::chart2::XDiagram > & xDiagram );
    static ::std::map< ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::XDataSeries >, sal_Int32 > 
            getDataSeriesIndexMapFromDiagram(
                const ::com::sun::star::uno::Reference<
                    ::com::sun::star::chart2::XDiagram > & xDiagram );

    static bool isCandleStickSeries(
                  const ::com::sun::star::uno::Reference<
                    ::com::sun::star::chart2::XDataSeries >& xSeries
                , const ::com::sun::star::uno::Reference<
                    ::com::sun::star::frame::XModel >& xChartModel  );

    static ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::XDataSeries > getFirstCandleStickSeries(
                    const ::com::sun::star::uno::Reference<
                        ::com::sun::star::chart2::XDiagram > & xDiagram  );

    static ::com::sun::star::uno::Reference<
                ::com::sun::star::beans::XPropertySet > createOldAPISeriesPropertySet(
                    const ::com::sun::star::uno::Reference<
                        ::com::sun::star::chart2::XDataSeries >& xSeries
                    , const ::com::sun::star::uno::Reference<
                        ::com::sun::star::frame::XModel >& xChartModel );

    static ::com::sun::star::uno::Reference<
                ::com::sun::star::beans::XPropertySet > createOldAPIDataPointPropertySet(
                    const ::com::sun::star::uno::Reference<
                        ::com::sun::star::chart2::XDataSeries >& xSeries
                    , sal_Int32 nPointIndex
                    , const ::com::sun::star::uno::Reference<
                        ::com::sun::star::frame::XModel >& xChartModel );
};

#endif	// SUPD == 310

// _XMLOFF_SCH_XML_SERIESHELPER_HXX
#endif
