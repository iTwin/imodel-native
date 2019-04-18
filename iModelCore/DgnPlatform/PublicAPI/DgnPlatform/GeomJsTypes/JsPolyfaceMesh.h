/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__
// This file is included into GeomJsApi.h and relies on it for #include and various JsXxx classes.
#pragma once

#ifndef _JSPOLYFACEMESH_H_
#define _JSPOLYFACEMESH_H_

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
// @bsiclass                                                    Eariln.Lutz     08/15
//=======================================================================================
struct JsPolyfaceMesh: JsGeometry
{
PolyfaceHeaderPtr m_data;

public:
    PolyfaceHeaderPtr GetPolyfaceHeaderPtr () override {return m_data;}
    IGeometryPtr GetIGeometryPtr () override {return IGeometry::Create (m_data);}

    JsPolyfaceMesh (PolyfaceHeaderPtr const &header) { m_data = header;}
    JsPolyfaceMeshP Clone () override
        {
        PolyfaceHeaderPtr clone = PolyfaceHeader::New ();
        m_data->CopyTo (*clone);
        return new JsPolyfaceMesh (clone);
        }

    JsPolyfaceMeshP AsPolyfaceMesh () override {return this;}

    JsPolyfaceMesh ()
        {
        m_data = PolyfaceHeader::CreateVariableSizeIndexed ();
        }
    //void SetTwoSided (bool value} { m_data->SetTwoSided (value);}
    bool GetTwoSided () {return m_data->GetTwoSided ();}

    static JsPolyfaceMeshP CreateVariableSizeIndexed ()
        {
        PolyfaceHeaderPtr header = PolyfaceHeader::CreateVariableSizeIndexed ();
        return new JsPolyfaceMesh (header);
        }
    
    static JsPolyfaceMeshP CreateFixedIndexed (double aNumPerFace)
        {
        int numPerFace = (int)floor (aNumPerFace);
        if (numPerFace < 0)
            numPerFace = 0;
        PolyfaceHeaderPtr header = PolyfaceHeader::CreateFixedBlockIndexed (numPerFace);
        return new JsPolyfaceMesh (header);
        }

    static JsPolyfaceMeshP CreateQuadGrid (double numPerRow)
        {
        PolyfaceHeaderPtr header = PolyfaceHeader::CreateQuadGrid ((int)numPerRow);
        return new JsPolyfaceMesh (header);
        }

    static JsPolyfaceMeshP CreateTriangleGrid (double numPerRow)
        {
        PolyfaceHeaderPtr header = PolyfaceHeader::CreateTriangleGrid ((int)numPerRow);
        return new JsPolyfaceMesh (header);
        }

    static JsPolyfaceMeshP CreateCoordinateTriangleMesh ()
        {
        PolyfaceHeaderPtr header = PolyfaceHeader::CreateFixedBlockCoordinates (3);
        return new JsPolyfaceMesh (header);
        }

    static JsPolyfaceMeshP CreateCoordinateQuadMesh ()
        {
        PolyfaceHeaderPtr header = PolyfaceHeader::CreateFixedBlockCoordinates (4);
        return new JsPolyfaceMesh (header);
        }

    double PolyfaceMeshStyle (){return (double)m_data->GetMeshStyle ();}

    // Single-value AddXxx methods -- note that Append method on blocked arrays does SetActive () !!!
    void AddPoint (JsDPoint3dP data){m_data->Point ().Append (data->Get ());}

    void AddNormal (JsDVector3dP data){m_data->Normal ().Append (data->Get ());}
    void AddParam (JsDPoint2dP data){m_data->Param ().Append (data->Get ());}
    void AddIntColor(double aColor){m_data->Param ().Append ((int)aColor);}

    void AddPointIndex (double aData){m_data->PointIndex ().Append ((int)aData);}
    void AddNormalIndex (double aData){m_data->NormalIndex ().Append ((int)aData);}
    void AddParamndex (double aData){m_data->ParamIndex ().Append ((int)aData);}
    void AddColorIndex (double aData){m_data->ColorIndex ().Append ((int)aData);}

    JsDRange3dP GetRange ()
        {
        DRange3d range = m_data->PointRange ();
        return new JsDRange3d (range);
        }

    double GetTightTolerance (){return m_data->GetTightTolerance ();}
    double GetMediumTolerance (){return m_data->GetMediumTolerance ();}
    bool IsClosedByEdgePairing (){return m_data->IsClosedByEdgePairing ();}
    bool HasFacets(){return m_data->HasFacets();}
    bool HasConvexFacets(){return m_data->HasConvexFacets();}
    double GetFacetCount (){return (double)m_data->GetNumFacet ();}
    double GetLargestCoordinate (){return m_data->LargestCoordinate ();}



};

END_BENTLEY_DGNPLATFORM_NAMESPACE

#endif//ndef _JSPOLYFACEMESH_H_

