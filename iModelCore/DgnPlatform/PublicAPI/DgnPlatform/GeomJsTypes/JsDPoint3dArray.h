/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__
// This file is included into GeomJsApi.h and relies on it for #include and various JsXxx classes.
#pragma once

#ifndef _JSDPOINT3DARRAY_H_
#define _JSDPOINT3DARRAY_H_

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
struct JsDPoint3dArray : BeProjectedRefCounted
{
private:
    bvector<DPoint3d> m_data;
public:   
    JsDPoint3dArray() {}
    static JsDPoint3dArrayP Create (bvector<DPoint3d> const *data)
        {
        return data != nullptr ? new JsDPoint3dArray (*data) : nullptr;
        }
    JsDPoint3dArray(bvector<DPoint3d> const &source) {m_data = source;}
    JsDPoint3dArrayP Clone (){ return new JsDPoint3dArray (m_data);}
    bvector<DPoint3d> & GetRef () {return m_data;}
    void Add (JsDPoint3dP in){m_data.push_back (in->Get ());}
    void AddXY (double x, double y){m_data.push_back (DPoint3d::From (x,y,0.0));}
    void AddXYZ (double x, double y, double z){m_data.push_back (DPoint3d::From (x,y,z));}
    void Clear (){m_data.clear ();}
    JsDPoint3dP At (double number)
        {
        size_t index;
        if (TryDoubleToIndex (number, m_data.size (), index))
            {
            return new JsDPoint3d (m_data[index]);
            }
        return nullptr;
        }
    double Size (){ return (double)m_data.size ();}
    void TransformInPlace (JsTransformP transform)
        {
        transform->Get().Multiply (m_data, m_data);
        }
    void Append (JsDPoint3dArrayP other)
        {
        for (DPoint3d &xyz : other->m_data)
            m_data.push_back (xyz);
        }

};

//=======================================================================================
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
struct JsDoubleArray : BeProjectedRefCounted
{
private:
    bvector<double> m_data;
public:   
    JsDoubleArray() {}
    JsDoubleArray(bvector<double> const &source) {m_data = source;}
    JsDoubleArrayP Clone (){ return new JsDoubleArray (m_data);}
    bvector<double> & GetRef () {return m_data;}
    void Add (double in){m_data.push_back (in);}
    void Clear (){m_data.clear ();}
    double At (double number)
        {
        size_t index;
        if (TryDoubleToIndex (number, m_data.size (), index))
            {
            return m_data[index];
            }
        return 0.0;
        }
    double Size (){ return (double)m_data.size ();}

    void Append (JsDoubleArrayP other)
        {
        for (double &a : other->m_data)
            m_data.push_back (a);
        }
};

//=======================================================================================
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
struct JsGeometryNode : BeProjectedRefCounted
{
private:
    GeometryNodePtr m_data;
public:   
    JsGeometryNode () {m_data = GeometryNode::Create ();}

    JsGeometryNode (GeometryNodePtr node) {m_data = node;}
    GeometryNodePtr GetGeometryNodePtr (){return m_data;}
    void AddGeometry (JsGeometryP g)
        {
        auto gPtr = g->GetIGeometryPtr ();
        m_data->AddGeometry (gPtr);
        }
    void AddMemberWithTransform (JsGeometryNodeP node, JsTransformP transform) {m_data->AddMember (node->m_data, transform->Get ());}

    void ClearGeometry (){m_data->ClearGeometry ();}
    void ClearMembers (){m_data->ClearMembers ();}

    JsGeometryP GeometryAt (double number)
        {
        size_t index;
        if (TryDoubleToIndex (number, m_data->Geometry ().size (), index))
            {
            return JsGeometry::CreateStronglyTypedJsGeometry (m_data->Geometry()[index]);
            }
        return nullptr;
        }

    JsGeometryNodeP MemberAt (double number)
        {
        size_t index;
        if (TryDoubleToIndex (number, m_data->Members ().size (), index))
            {
            return new JsGeometryNode (m_data->Members()[index].GetMember ());
            }
        return nullptr;
        }

    JsTransformP MemberTransformAt (double number)
        {
        size_t index;
        if (TryDoubleToIndex (number, m_data->Members ().size (), index))
            {
            return new JsTransform (m_data->Members()[index].GetTransform ());
            }
        return nullptr;
        }


    JsGeometryNodeP Flatten ()
        {
        return new JsGeometryNode (m_data->Flatten ());
        }

    double GeometrySize (){ return (double)m_data->Geometry().size ();}
    double MemberSize   (){ return (double)m_data->Members().size ();}

    bool IsSameStructureAndGeometry (JsGeometryNodeP other)
        {
        return m_data->IsSameStructureAndGeometry (*other->m_data, 0.0);
        }
};

END_BENTLEY_DGNPLATFORM_NAMESPACE

#endif//ndef _JSDPOINT3DARRAY_H_

