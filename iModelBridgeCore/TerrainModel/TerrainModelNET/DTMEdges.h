/*--------------------------------------------------------------------------------------+
|
|     $Source: TerrainModelNET/DTMEdges.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include ".\Bentley.Civil.DTM.h"
using namespace System;

BEGIN_BENTLEY_TERRAINMODELNET_NAMESPACE

//=======================================================================================
/// <summary>
/// Class that represents a edge.
/// </summary>    
/// <author>Sylvain.Pucci</author>                              <date>08/2005</date>
//=======================================================================================
public ref struct Edges: System::IDisposable
    {
    private: 
        BcDTMEdgesP m_edges;
        ~Edges();
        !Edges();

    internal: 

        //=======================================================================================
        /// <summary>
        /// Initializes a new instance of the Edges class.
        /// </summary>                
        /// <author>Sylvain.Pucci</author>                              <date>08/2005</date>
        //=======================================================================================
        Edges (BcDTMEdgesP edges);

    public: 

        //=======================================================================================
        /// <summary>
        /// Gets the number of edges.
        /// </summary>                
        /// <author>Sylvain.Pucci</author>                              <date>08/2005</date>
        //=======================================================================================
        property int Count
            {
            int get();
            }

        //=======================================================================================
        /// <summary>
        /// Gets the edges.
        /// </summary>                
        /// <author>Sylvain.Pucci</author>                              <date>08/2005</date>
        //=======================================================================================
        array<BGEO::DPoint3d>^ GetEdge(int index);
    };

END_BENTLEY_TERRAINMODELNET_NAMESPACE
