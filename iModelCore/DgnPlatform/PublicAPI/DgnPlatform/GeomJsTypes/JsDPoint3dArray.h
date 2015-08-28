/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/GeomJsTypes/JsDPoint3dArray.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__
// This file is included into GeomJsApi.h and relies on it for #include and various JsXxx classes.
#pragma once

#ifndef _JSDPOINT3DARRAY_H_
#define _JSDPOINT3DARRAY_H_

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
struct JsDPoint3dArray : RefCountedBase
{
private:
    bvector<DPoint3d> m_data;
    bool TryDoubleToIndex (double a, size_t &index)
        {
        index = 0;
        if (a < 0.0)
            return false;
        index = (size_t) a;
        if (index < m_data.size ())
            return true;
        index = SIZE_MAX;
        return false;
        }
public:   
    JsDPoint3dArray() {}
    JsDPoint3dArray(bvector<DPoint3d> const &source) {m_data = source;}
    JsDPoint3dArrayP Clone (){ return new JsDPoint3dArray (m_data);}
    bvector<DPoint3d> & Get () {return m_data;}
    void Add (JsDPoint3dP in){m_data.push_back (in->Get ());}
    void Clear (){m_data.clear ();}
    JsDPoint3dP At (double number)
        {
        size_t index;
        if (TryDoubleToIndex (number, index))
            {
            return new JsDPoint3d (m_data[index]);
            }
        return nullptr;
        }
    double Size (){ return (double)m_data.size ();}

    void Append (JsDPoint3dArrayP other)
        {
        for (DPoint3d &xyz : other->m_data)
            m_data.push_back (xyz);
        }

};
END_BENTLEY_DGNPLATFORM_NAMESPACE

#endif//ndef _JSDPOINT3DARRAY_H_

