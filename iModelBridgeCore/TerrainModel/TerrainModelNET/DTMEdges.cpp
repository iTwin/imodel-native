/*--------------------------------------------------------------------------------------+
|
|     $Source: TerrainModelNET/DTMEdges.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "StdAfx.h"
#include < vcclr.h >
#include ".\dtmedges.h"
#using <mscorlib.dll>

using namespace System;

BEGIN_BENTLEY_TERRAINMODELNET_NAMESPACE

//=======================================================================================
// @bsimethod                                               Sylvain.Pucci      5/2007
//=======================================================================================
Edges::Edges (BcDTMEdgesP edges)
    {
    m_edges = edges;
    m_edges->AddRef();
    }

//=======================================================================================
// @bsimethod                                               Sylvain.Pucci      5/2007
//=======================================================================================
int Edges::Count::get()
    {
    if (m_edges == NULL)
        {
        throw gcnew System::Exception("Edges was disposed");
        }
    return m_edges->GetEdgeCount();
    }

//=======================================================================================
// @bsimethod                                               Sylvain.Pucci      5/2007
//=======================================================================================
Edges::~Edges()
    {
    this->!Edges();
    System::GC::SuppressFinalize(this);
    }

//=======================================================================================
// @bsimethod                                               Sylvain.Pucci      5/2007
//=======================================================================================
Edges::!Edges()
    {
    if (m_edges != NULL)
        {
        m_edges->Release();
        }
    }

//=======================================================================================
// @bsimethod                                               Sylvain.Pucci      5/2007
//=======================================================================================
array<BGEO::DPoint3d>^ Edges::GetEdge(int index) 
    {
    array<BGEO::DPoint3d>^ res = gcnew array<BGEO::DPoint3d>(2);

    DPoint3d pt;
    m_edges->GetEdgeStartPoint(index, pt);
    res[0] = BGEO::DPoint3d(pt.x, pt.y, pt.z);
    m_edges->GetEdgeEndPoint(index, pt);
    res[1] = BGEO::DPoint3d(pt.x, pt.y, pt.z);

    return res;
    }

END_BENTLEY_TERRAINMODELNET_NAMESPACE
