/*--------------------------------------------------------------------------------------+
|
|   Code taken from Michael Garland's demo application called "QSlim" (version 1.0)
|   which intends to demonstrate an algorithm of mesh simplification based on
|   Garland and Heckbert(1997) "Surface Simplification Using Quadric Error Metrics".
|   The code of the demo is said to be in the public domain.
|   See: http://www.cs.cmu.edu/afs/cs/Web/People/garland/quadrics/qslim10.html
|
|   $Revision: 1.0 $
|       $Date: 2014/09/17 $
|     $Author: Christian.Cote $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include "Vec3.h"
#include "Buffer.h"
#include "Array3.h"

class ProxGrid_Cell : public buffer<Vec3 *>
{
    public:
        ProxGrid_Cell() : buffer<Vec3 *>(8){ }
};

class ProximityGrid
{
    array3<ProxGrid_Cell> cells;
    int xdiv, ydiv, zdiv;
    double cellsize;
    double cellsize2;
    Vec3 minVec, maxVec;
    void GetCellForPoint(const Vec3&, int *i, int *j, int *k);
    void CollectPoints(Vec3 *v, buffer<Vec3 *>& close, ProxGrid_Cell& cell);

public:

    ProximityGrid(const Vec3& minVec, const Vec3& maxVec, double dist);
    ~ProximityGrid() { cells.freeMemory(); }
    void AddPoint(Vec3 *);
    void RemovePoint(Vec3 *);
    void ProximalPoints(Vec3 *, buffer<Vec3 *>&);
};
