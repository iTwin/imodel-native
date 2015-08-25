#pragma once

#include <amp.h>
#include <Bentley/Bentley.h>

using namespace concurrency;


float pointDistanceSquared(FPoint3d& p1, FPoint3d& p2) restrict(amp)
    {
    return (p1.x - p2.x)*(p1.x - p2.x) + (p1.y - p2.y)*(p1.y - p2.y) + (p1.z - p2.z)*(p1.z - p2.z);
    }

//see http://stackoverflow.com/questions/2049582/how-to-determine-a-point-in-a-triangle for getting the barycentric coordinates.
//I know we have code to do this, I thought a more condensed version might work better with AMP.
bool findTriangleInTriList(FPoint3d* tri, array_view<const float> triList, int size, FPoint3d& testPt) restrict(amp)
    {
    for (int i = 0; i < size; i += 9)
        {
        float denom = (-triList[i + 4] * triList[i + 6] + triList[i + 1] * (-triList[i + 3] + triList[i + 6]) + triList[i] * (triList[i + 4] - triList[i + 7]) + triList[i + 3] * triList[i + 7]);
        if (denom == 0.0f) continue;
        float area = 1.0f / 2.0f * denom;

        float s = 1.0f / (2.0f * area)*(triList[i + 1] * triList[i + 6] - triList[i] * triList[i + 7] + (triList[i + 7] - triList[i + 1])*testPt.x + (triList[i] - triList[i + 6])*testPt.y);
        float t = 1.0f / (2.0f * area)*(triList[i] * triList[i + 4] - triList[i + 1] * triList[i + 3] + (triList[i + 1] - triList[i + 4])*testPt.x + (triList[i + 3] - triList[i])*testPt.y);
        if (s >= 0.0f && t >= 0.0f && 1 - s - t >= 0.0f)
            {
            tri[0].x = triList[i];
            tri[0].y = triList[i + 1];
            tri[0].z = triList[i + 2];
            tri[1].x = triList[i + 3];
            tri[1].y = triList[i + 4];
            tri[1].z = triList[i + 5];
            tri[2].x = triList[i + 6];
            tri[2].y = triList[i + 7];
            tri[2].z = triList[i + 8];
            return true;
            }
        }
    return false;
    }

FPoint3d crossProduct(FPoint3d a, FPoint3d b) restrict(amp)
    {
    float xx = a.y * b.z - a.z * b.y;
    float yy = a.z * b.x - a.x * b.z;
    float zz = a.x * b.y - a.y * b.x;
    FPoint3d pt;
    pt.x = xx;
    pt.y = yy;
    pt.z = zz;
    return pt;
    }

float dotProduct(FPoint3d a, FPoint3d b) restrict(amp)
    {
    return (a.x * b.x + a.y * b.y + a.z * b.z);
    }

FPoint3d projectPtOnTriPlane(FPoint3d* tri, FPoint3d& testPt) restrict(amp)
    {
    FPoint3d origin, normal;
    origin = tri[0];
    FPoint3d vec01, vec02;
    vec01.x = tri[1].x - tri[0].x;
    vec01.y = tri[1].y - tri[0].y;
    vec01.z = tri[1].z - tri[0].z;

    vec02.x = tri[2].x - tri[0].x;
    vec02.y = tri[2].y - tri[0].y;
    vec02.z = tri[2].z - tri[0].z;
    normal = crossProduct(vec01, vec02);
    FPoint3d proj;
    FPoint3d V;
    float UdotU, UdotV, s = 0;
    V.x = testPt.x - origin.x;
    V.y = testPt.y - origin.y;
    V.z = testPt.z - origin.z;
    UdotU = dotProduct(normal, normal);
    UdotV = dotProduct(normal, V);
    if (UdotU != 0)
        {
        s = UdotV / UdotU;
        proj.x = testPt.x + normal.x * -s;
        proj.y = testPt.y + normal.y * -s;
        proj.z = testPt.z + normal.z * -s;
        }
    return proj;
    }

float crossProductMagnitude(FPoint3d a, FPoint3d b) restrict(amp)
    {
    FPoint3d pt = crossProduct(a, b);
    return fast_math::sqrt(pt.x*pt.x + pt.y*pt.y + pt.z*pt.z);
    }

#define PI_FLOAT 3.1415926535897932384626433f
bool evaluateSlopeCondition(FPoint3d* tri, FPoint3d& proj, FPoint3d& testPt, float allowedSlope) restrict(amp)
    {
    for (int i = 0; i < 3; i++)
        {
        FPoint3d triPoint;
        FPoint3d triProj;

        triPoint.x = tri[i].x - testPt.x;
        triPoint.y = tri[i].y - testPt.y;
        triPoint.z = tri[i].z - testPt.z;
        triProj.x = tri[i].x - proj.x;
        triProj.y = tri[i].y - proj.y;
        triProj.z = tri[i].z - proj.z;
        float cross, dot;
        cross = crossProductMagnitude(triProj, triPoint);
        dot = dotProduct(triProj, triPoint);
        float angle1 = cross == 0.0f && dot == 0.0f ? 0.0f : fast_math::atan2(cross, dot);
        float angleDeg1 = angle1 * 180.0f / PI_FLOAT;
        if ((angleDeg1) > allowedSlope)
            return false;
        }
    return true;
    }

bool evaluateHeightCondition(FPoint3d& testPt, FPoint3d&  proj, float allowedHeight, float density, float minZ) restrict(amp)
    {
    return (pointDistanceSquared(testPt, proj) <= allowedHeight*allowedHeight);
    }