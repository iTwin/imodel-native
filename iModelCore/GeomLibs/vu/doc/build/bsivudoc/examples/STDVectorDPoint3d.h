/*--------------------------------------------------------------------------------------+
|
|     $Source: vu/doc/build/bsivudoc/examples/STDVectorDPoint3d.h $
|
|  $Copyright: (c) 2010 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <msgeomstructs.h>
#include <vector>

//#define USE_MFC //Igor.Sokolov 28.08.2008: use MFC arrays
                //else bvector will be used

#ifdef USE_MFC

    class STDVectorDPoint3d : public CArray<DPoint3d>
    {
    public:
        STDVectorDPoint3d (void)                            {}
        STDVectorDPoint3d (const STDVectorDPoint3d& src)    { Append (src); ASSERT(0);/*avoid copy*/ }
    public:
        //required bvector methods reimplementation
        inline long size      (void)                 const  { return GetCount(); }
        inline bool empty     (void)                 const  { return GetCount()==0; }
        inline void push_back (const DPoint3d& pt)          { Add (pt); }
        inline void clear     (void)                        { RemoveAll(); }
    };//class STDVectorDPoint3d

#else //USE_MFC

    typedef bvector<DPoint3d> STDVectorDPoint3d;

#endif //#else //USE_MFC


typedef STDVectorDPoint3d &STDVectorDPoint3dR;



// Print xyz coordinates as microstation keyin sequences.
static void PrintDisconnectChainsAsKeyin (STDVectorDPoint3dR xyz)
    {
    int i = 0;
    int n = xyz.size ();
    while (i < n - 1)
        {
        while (i < n && xyz[i].isDisconnect ())
            i++;
        printf ("PLACE SMARTLINE\n");
        while (i < n && !xyz[i].isDisconnect ())
            {
            printf (" xy=%lg,%lg\n", xyz[i].x, xyz[i].y);
            i++;
            }
        i++;
        }
    }

// Add a point to the end of the DPoint3edArray
static void PushBack (STDVectorDPoint3dR Array, double x, double y, double z)
    {
    DPoint3d xyz;
    xyz.x = x;
    xyz.y = y;
    xyz.z = z;
    Array.push_back (xyz);
    }

// Add a disconnect to the end of the array.
static void PushBackDisconnect (STDVectorDPoint3dR Array)
    {
    DPoint3d xyz;
    xyz.initDisconnect ();
    Array.push_back (xyz);
    }
