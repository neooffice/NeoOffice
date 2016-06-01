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

#ifndef INCLUDED_GRAPHICHELPERS_HXX
#define INCLUDED_GRAPHICHELPERS_HXX

#include "PropertyMap.hxx"

#include <WriterFilterDllApi.hxx>
#include <resourcemodel/LoggedResources.hxx>

#include <boost/shared_ptr.hpp>

namespace writerfilter {
namespace dmapper
{

class WRITERFILTER_DLLPRIVATE PositionHandler: public LoggedProperties
{
public:
#ifdef NO_LIBO_4_1_GRAPHICS_POSITION_FIXES
    PositionHandler( );
#else	// NO_LIBO_4_1_GRAPHICS_POSITION_FIXES
    PositionHandler( bool vertical );
#endif	// NO_LIBO_4_1_GRAPHICS_POSITION_FIXES
    ~PositionHandler( );
#ifndef NO_LIBO_4_1_GRAPHICS_POSITION_FIXES
    static void setPositionOffset(const ::rtl::OUString & sText, bool vertical);
    static void setAlignH(const ::rtl::OUString & sText);
    static void setAlignV(const ::rtl::OUString & sText);
#endif	// !NO_LIBO_4_1_GRAPHICS_POSITION_FIXES

    sal_Int16 m_nOrient;
    sal_Int16 m_nRelation;
    sal_Int32 m_nPosition;

 private:
    virtual void lcl_attribute( Id aName, Value& rVal );
    virtual void lcl_sprm( Sprm& rSprm );
#ifndef NO_LIBO_4_1_GRAPHICS_POSITION_FIXES
    static int savedPositionOffsetV, savedPositionOffsetH;
    static int savedAlignV, savedAlignH;
#endif	// !NO_LIBO_4_1_GRAPHICS_POSITION_FIXES
}; 
typedef boost::shared_ptr<PositionHandler> PositionHandlerPtr;

class WRITERFILTER_DLLPRIVATE WrapHandler: public LoggedProperties
{
public:
    WrapHandler( );
    ~WrapHandler( );

    sal_Int32 m_nType;
    sal_Int32 m_nSide;

    sal_Int32 getWrapMode( );

 private:
    virtual void lcl_attribute( Id aName, Value& rVal );
    virtual void lcl_sprm( Sprm& rSprm );
};
typedef boost::shared_ptr<WrapHandler> WrapHandlerPtr;

} }

#endif
