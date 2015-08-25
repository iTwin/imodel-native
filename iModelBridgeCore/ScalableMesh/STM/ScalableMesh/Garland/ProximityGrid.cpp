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

#include <ScalableMeshPCH.h>
#include <iostream>
#include "TypesAndDefinitions.h"
#include "Vec3.h"
#include "Buffer.h"
#include "Array3.h"
#include "ProximityGrid.h"

ProximityGrid::ProximityGrid(const Vec3& lo, const Vec3& hi, double dist)
{
    cellsize = dist;
    cellsize2 = dist*dist;

    minVec = lo;
    maxVec = hi;

    xdiv = (int)ceil((maxVec[X] - minVec[X]) / dist);
    ydiv = (int)ceil((maxVec[Y] - minVec[Y]) / dist);
    zdiv = (int)ceil((maxVec[Z] - minVec[Z]) / dist);

    cells.init(xdiv, ydiv, zdiv);
}

void ProximityGrid::GetCellForPoint(const Vec3& v, int *i_out, int *j_out, int *k_out)
{
    int i = (int)floor((v[X] - minVec[X]) / cellsize);
    int j = (int)floor((v[Y] - minVec[Y]) / cellsize);
    int k = (int)floor((v[Z] - minVec[Z]) / cellsize);

    // In case we hit the max bounds
    if (i == xdiv) i--;
    if (j == ydiv) j--;
    if (k == zdiv) k--;

    *i_out = i;
    *j_out = j;
    *k_out = k;
}

void ProximityGrid::AddPoint(Vec3 *v)
{
    int i, j, k;
    GetCellForPoint(*v, &i, &j, &k);
    ProxGrid_Cell& cell = cells(i, j, k);
    cell.add(v);
}

void ProximityGrid::RemovePoint(Vec3 *v)
{
    int i, j, k;
    GetCellForPoint(*v, &i, &j, &k);
    ProxGrid_Cell& cell = cells(i, j, k);
    int index = cell.find(v);
    if (index >= 0)
        cell.remove(index);
    else
        cerr << "WARNING: ProximityGrid -- removing non-member point." << endl;
}

void ProximityGrid::CollectPoints(Vec3 *v, buffer<Vec3 *>& close, ProxGrid_Cell& cell)
{
    for (int i = 0; i<cell.length(); i++)
    {
        Vec3 *u = cell(i);
        if (u != v && norm2(*u - *v) < cellsize2)
            close.add(u);
    }
}

void ProximityGrid::ProximalPoints(Vec3 *v, buffer<Vec3 *>& close)
{
    int i, j, k;
    GetCellForPoint(*v, &i, &j, &k);

    for (int dk = -1; dk<2; dk++)
        for (int dj = -1; dj<2; dj++)
            for (int di = -1; di<2; di++)
            {
                if (i + di >= 0 && j + dj >= 0 && k + dk >= 0
                    && i + di<cells.width()
                    && j + dj<cells.height()
                    && k + dk<cells.depth())
                    {
                        CollectPoints(v, close, cells(i + di, j + dj, k + dk));
                    }
            }
}