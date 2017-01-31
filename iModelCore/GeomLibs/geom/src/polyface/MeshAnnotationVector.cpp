/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/polyface/MeshAnnotationVector.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

//========================================================================
// return index to indicated key.  Search is linear, but in reverse order so repeated references go fast.
// returns SIZE_MAX if key not found
size_t MeshAnnotationVector::Find (Utf8CP key)
    {
    for (size_t i = size (); i-- > 0;)
        {
        if (at (i).m_description.CompareToI (key) == 0)
            return i;
        }
    return SIZE_MAX;
    }
//========================================================================
size_t MeshAnnotationVector::FindOrAdd (Utf8CP key)
    {
    size_t index = Find (key);
    if (index == SIZE_MAX)
        {
        index = size ();
        push_back (MeshAnnotation (key));
        }
    return index;
    }

//========================================================================
// Return index where recorded (SIZE_MAX if not recorded)
size_t MeshAnnotationVector::Assert (bool condition, Utf8CP key)
    {
    size_t index = SIZE_MAX;
    if (condition)
        {
        m_totalPass++;
        if (m_recordAllTestDescriptions)
            {
            index = FindOrAdd (key);
            at (index).IncrementPass ();
            }
        }
    else
        {
        m_totalFail++;
        index = FindOrAdd (key);
        at (index).IncrementFail ();
        }
    return index;
    }

//========================================================================
void MeshAnnotationVector::Assert (bool condition, Utf8CP key, size_t index, size_t tag)
    {
    size_t k = Assert (condition, key);
    if (k != SIZE_MAX && !condition)
        at (k).Record (index, tag);
    }

//========================================================================
void MeshAnnotationVector::Assert (bool condition, Utf8CP key, size_t index, DPoint3dCR tag)
    {
    size_t k = Assert (condition, key);
    if (k != SIZE_MAX && !condition)
        at (k).Record (index, tag);
    }

//========================================================================
void MeshAnnotationVector::Assert (bool condition, Utf8CP key, size_t index, int tag)
    {
    size_t k = Assert (condition, key);
    if (k != SIZE_MAX && !condition)
        at (k).Record (index, tag);
    }


END_BENTLEY_GEOMETRY_NAMESPACE
