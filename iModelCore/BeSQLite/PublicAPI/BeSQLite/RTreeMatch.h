/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/BeSQLite/RTreeMatch.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include "BeSQLite.h"
#include <algorithm>

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

BEGIN_BENTLEY_SQLITE_NAMESPACE

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   10/11
//=======================================================================================
struct RTree2dVal
{
    double m_minx, m_maxx, m_miny, m_maxy;

    RTree2dVal() {Invalidate();}
    RTree2dVal (DRange2dCR range) {m_minx=range.low.x; m_maxx=range.high.x; m_miny=range.low.y; m_maxy=range.high.y;}
    void Invalidate() {m_minx=m_miny=1; m_maxx=m_maxy=-1;}
    double Margin() const {return (m_maxx-m_minx) + (m_maxy-m_miny);}
    void ToRange (DRange2dR range) const {range.low.x=m_minx; range.low.y=m_miny; range.high.x=m_maxx; range.high.y=m_maxy;}
    void ToRange (DRange3dR range) const {range.low.x=m_minx; range.low.y=m_miny; range.high.x=m_maxx; range.high.y=m_maxy; range.low.z=range.high.z=0.0;}
    bool IsValid() const {return m_maxx>=m_minx && m_maxx>=m_minx;}
    void Union(RTree2dVal const& other) {m_minx=std::min(m_minx,other.m_minx); m_miny=std::min(m_miny,other.m_miny); m_maxx=std::max(m_maxx,other.m_maxx); m_maxy=std::max(m_maxy,other.m_maxy);}
    bool Contains(RTree2dVal const& other) const    {return m_minx<=other.m_minx && m_miny<=other.m_miny && m_maxx>=other.m_maxx && m_maxy>=other.m_maxy;}
    bool Intersects (RTree2dVal const& other) const {return m_minx<=other.m_maxx && m_maxx>=other.m_minx && m_miny<=other.m_maxy && m_maxy>=other.m_miny;}
};

typedef RTree2dVal const* RTree2dValCP;
typedef RTree2dVal*       RTree2dValP;

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   10/11
//=======================================================================================
struct RTree3dVal
{
    double m_minx, m_maxx, m_miny, m_maxy, m_minz, m_maxz;

    RTree3dVal() {Invalidate();}
    RTree3dVal (DRange3dCR range) {FromRange(range);}
    void Invalidate() {m_minx=m_miny=m_minz=1; m_maxx=m_maxy=m_maxz=-1;}
    double Margin() const {return (m_maxx-m_minx) + (m_maxy-m_miny) + (m_maxz-m_minz);}
    double Margin2d() const {return (m_maxx-m_minx) + (m_maxy-m_miny);}
    void ToRange (DRange3dR range) const {range.low.x=m_minx; range.low.y=m_miny; range.low.z=m_minz; range.high.x=m_maxx; range.high.y=m_maxy; range.high.z=m_maxz;}
    void FromRange (DRange3dCR range) {m_minx=range.low.x; m_maxx=range.high.x; m_miny=range.low.y; m_maxy=range.high.y; m_minz=range.low.z; m_maxz=range.high.z;}
    bool IsValid() const {return m_maxx>=m_minx && m_maxy>=m_miny && m_maxz>=m_minz;}
    void Union(RTree3dVal const& other) {m_minx=std::min(m_minx,other.m_minx); m_miny=std::min(m_miny,other.m_miny); m_minz=std::min(m_minz,other.m_minz);
                                         m_maxx=std::max(m_maxx,other.m_maxx); m_maxy=std::max(m_maxy,other.m_maxy); m_maxz=std::max(m_maxz,other.m_maxz);}
    bool Contains(RTree3dVal const& other) const    {return m_minx<=other.m_minx && m_miny<=other.m_miny && m_minz<=other.m_minz &&
                                                            m_maxx>=other.m_maxx && m_maxy>=other.m_maxy && m_maxz>=other.m_maxz;}
    bool Intersects (RTree3dVal const& other) const {return m_minx<=other.m_maxx && m_maxx>=other.m_minx &&
                                                            m_miny<=other.m_maxy && m_maxy>=other.m_miny &&
                                                            m_minz<=other.m_maxz && m_maxz>=other.m_minz;}
    bool Intersection (RTree3dVal const& left, RTree3dVal const& right) 
                                                            {
                                                            m_minx = left.m_minx > right.m_minx ? left.m_minx : right.m_minx;
                                                            m_miny = left.m_miny > right.m_miny ? left.m_miny : right.m_miny;
                                                            m_minz = left.m_minz > right.m_minz ? left.m_minz : right.m_minz;
                                                            m_maxx = left.m_maxx < right.m_maxx ? left.m_maxx : right.m_maxx;
                                                            m_maxy = left.m_maxy < right.m_maxy ? left.m_maxy : right.m_maxy;
                                                            m_maxz = left.m_maxz < right.m_maxz ? left.m_maxz : right.m_maxz;
                                                            return IsValid();
                                                            }
    bool Intersects2d (RTree3dVal const& other) const {return m_minx<= other.m_maxx && m_maxx>= other.m_minx &&
                                                            m_miny<= other.m_maxy && m_maxy>= other.m_miny ;}
};

typedef RTree3dVal const& RTree3dValCR;
typedef RTree3dVal const* RTree3dValCP;
typedef RTree3dVal*       RTree3dValP;

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   12/11
//=======================================================================================
struct RTree3dBoundsTest : RTreeAcceptFunction::Tester
{
    RTree3dVal  m_bounds;

    RTree3dBoundsTest (DbR db) : Tester(db) {}
    void _StepRange(DbFunction::Context&, int nArgs, DbValue* args) override {}
    int _TestRange(QueryInfo const& info) override
        {
//__PUBLISH_SECTION_END__
        BeAssert (6 == info.m_nCoord);
//__PUBLISH_SECTION_START__
        info.m_within = Within::Outside;
        RTree3dValCP pt = (RTree3dValCP) info.m_coords;

        if (!m_bounds.IsValid())
            m_bounds = *pt;
        else
            m_bounds.Union (*pt);

        return  BE_SQLITE_OK;
        }
};

END_BENTLEY_SQLITE_NAMESPACE
