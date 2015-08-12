/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#ifndef INCLUDED_GRAPHICHELPERS_HXX
#define INCLUDED_GRAPHICHELPERS_HXX

#include "PropertyMap.hxx"

#include <WriterFilterDllApi.hxx>
#include <resourcemodel/WW8ResourceModel.hxx>

#include <boost/shared_ptr.hpp>

namespace writerfilter {
namespace dmapper
{

class WRITERFILTER_DLLPRIVATE PositionHandler: public Properties
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

    virtual void attribute( Id aName, Value& rVal );
    virtual void sprm( Sprm& rSprm );
#ifndef NO_LIBO_4_1_GRAPHICS_POSITION_FIXES
    static int savedPositionOffsetV, savedPositionOffsetH;
    static int savedAlignV, savedAlignH;
#endif	// !NO_LIBO_4_1_GRAPHICS_POSITION_FIXES
}; 
typedef boost::shared_ptr<PositionHandler> PositionHandlerPtr;

class WRITERFILTER_DLLPRIVATE WrapHandler: public Properties
{
public:
    WrapHandler( );
    ~WrapHandler( );

    sal_Int32 m_nType;
    sal_Int32 m_nSide;

    sal_Int32 getWrapMode( );

    virtual void attribute( Id aName, Value& rVal );
    virtual void sprm( Sprm& rSprm );
};
typedef boost::shared_ptr<WrapHandler> WrapHandlerPtr;

} }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
