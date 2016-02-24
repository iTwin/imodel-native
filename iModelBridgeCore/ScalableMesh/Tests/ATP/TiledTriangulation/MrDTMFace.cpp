/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/ATP/TiledTriangulation/MrDTMFace.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

//#include "DcStmCorePCH.h"
//#include "ScalableMeshATPPch.h"
#include "MrDTMFace.h"

//BEGIN_GEODTMAPP_NAMESPACE

void MeshFaceProperties::ComputeBoundingCircle (IBcDTMMeshFacePtr face)
{
    DPoint3d a = face->GetCoordinates(0), b = face->GetCoordinates(1), c= face->GetCoordinates(2);

    double circumCenterX = 0.0, circumCenterY = 0.0;
    //const bool useNewFormula = true;

    //if (useNewFormula)
    //{
    // Translate triangle to origin
    const double bpx = b.x-a.x; const double bpy = b.y-a.y;
    const double cpx = c.x-a.x; const double cpy = c.y-a.y;

    // Compute determinant
    const double D = 2.0*(bpx*cpy - bpy*cpx);

    // Compute circumcenter
    const double bpx2 = bpx*bpx;
    const double bpy2 = bpy*bpy;
    const double cpx2 = cpx*cpx;
    const double cpy2 = cpy*cpy;
    const double bpx2Plusbpy2 = bpx2 + bpy2;
    const double cpx2Pluscpy2 = cpx2 + cpy2;
    circumCenterX = (cpy*bpx2Plusbpy2 - bpy*cpx2Pluscpy2)/D;
    circumCenterY = (bpx*cpx2Pluscpy2 - cpx*bpx2Plusbpy2)/D;

    // Translate back to original position
    circumCenterX += a.x;
    circumCenterY += a.y;

    //}
    ////calculate triangle radius
    //double yDelta_a= b.y - a.y;
    //double xDelta_a= b.x - a.x;
    //double yDelta_b= c.y  - b.y;
    //double xDelta_b= c.x - b.x;

    //const double numeric_tolerance = 0.000000001;

    //double aSlope=fabs(xDelta_a) > numeric_tolerance ? yDelta_a/xDelta_a : 1;
    //double bSlope=fabs(xDelta_b) > numeric_tolerance ? yDelta_b/xDelta_b : 1;

    //if(aSlope == 1 && bSlope == 0 || aSlope == 0 && bSlope == 1)
    //{
    //  m_CircumCenterX = (c.x + a.x)/2;
    //  m_CircumCenterY = (c.y + a.y)/2;
    //}
    //else if(aSlope == 1 || aSlope == 0)
    //{
    //  double cSlope = (c.y - a.y) / (c.x - a.x);
    //  bSlope = yDelta_b / xDelta_b;
    //  m_CircumCenterX = (cSlope*bSlope*(a.y - b.y) + bSlope*(a.x + c.x)
    //          - cSlope*(c.x + b.x) )/(2* (bSlope-cSlope) );
    //  if(fabs(cSlope) > numeric_tolerance)
    //          m_CircumCenterY = -1*(m_CircumCenterX - (a.x + c.x)/2)/cSlope + (a.y+c.y)/2;
    //  else //use the other perpendicular
    //          m_CircumCenterY = -1*(m_CircumCenterX - (b.x + c.x)/2)/bSlope + (b.y + c.y)/2;
    //}
    //else if(bSlope == 1 || bSlope == 0)
    //{
    //  double cSlope = (a.y - c.y) / (a.x - c.x);
    //  m_CircumCenterX = (cSlope*aSlope*(c.y - b.y) + aSlope*(c.x + a.x)
    //          - cSlope*(a.x + b.x) )/(2* (aSlope-cSlope) );
    //  if(fabs(cSlope) > numeric_tolerance)
    //          m_CircumCenterY = -1*(m_CircumCenterX - (c.x+a.x)/2)/cSlope + (c.y+a.y)/2;
    //  else //use the other perpendicular
    //          m_CircumCenterY = -1*(m_CircumCenterX - (a.x + b.x)/2)/aSlope + (a.y + b.y)/2;
    //}
    //else
    //{
    //  m_CircumCenterX = (aSlope*bSlope*(a.y - c.y) + bSlope*(a.x + b.x)
    //          - aSlope*(b.x + c.x) )/(2* (bSlope-aSlope) );
    //  if(fabs(aSlope) > numeric_tolerance)
    //          m_CircumCenterY = -1*(m_CircumCenterX - (a.x+b.x)/2)/aSlope + (a.y+b.y)/2;
    //  else //use the other perpendicular
    //          m_CircumCenterY = -1*(m_CircumCenterX - (b.x + c.x)/2)/bSlope + (b.y + c.y)/2;
    //}

    //if (useNewFormula)  // compare results
    //{
    //  if ( fabs(circumCenterX - m_CircumCenterX) > 0.000001 )
    //  {
    //          double radiusA = pow(a.x - m_CircumCenterX, 2) + pow(a.y - m_CircumCenterY, 2);
    //          double radiusB = pow(b.x - m_CircumCenterX, 2) + pow(b.y - m_CircumCenterY, 2);
    //          double radiusC = pow(c.x - m_CircumCenterX, 2) + pow(c.y - m_CircumCenterY, 2);
    //          double radiusANew = pow(a.x - circumCenterX, 2) + pow(a.y - circumCenterY, 2);
    //          double radiusBNew = pow(b.x - circumCenterX, 2) + pow(b.y - circumCenterY, 2);
    //          double radiusCNew = pow(c.x - circumCenterX, 2) + pow(c.y - circumCenterY, 2);
    //          circumCenterX = m_CircumCenterX;
    //  }
    //  if ( fabs(circumCenterY - m_CircumCenterY) > 0.000001 )
    //  {
    //          circumCenterY = m_CircumCenterY;
    //  }
    //}
    m_CircumCenterX = circumCenterX;
    m_CircumCenterY = circumCenterY;
    m_CircumRadiusSquared = pow(a.x - m_CircumCenterX, 2) + pow(a.y - m_CircumCenterY, 2);
}

double MeshFaceProperties::GetCircumCenterX() const
{
    return m_CircumCenterX;
}

double MeshFaceProperties::GetCircumCenterY() const
{
    return m_CircumCenterY;
}

double MeshFaceProperties::GetCircumRadiusSquared() const
{
    return m_CircumRadiusSquared;
}

void FaceWithProperties::ComputeBoundingCircle()
{
    m_properties.ComputeBoundingCircle(m_face);
}

FaceWithProperties::FaceWithProperties()
: m_face(0)
    {
    }

FaceWithProperties::FaceWithProperties(const FaceWithProperties& face)
: m_face(face.m_face)
    {
    }

FaceWithProperties::~FaceWithProperties()
    {
    int i = 0;
    i = i;
    }

IBcDTMMeshFacePtr FaceWithProperties::GetFace() const
    {
    return m_face;
    }

void FaceWithProperties::SetFace(IBcDTMMeshFacePtr face)
{
    m_face = face;
}

const MeshFaceProperties& FaceWithProperties::GetProperties() const
    {
    return m_properties;
    }

//END_GEODTMAPP_NAMESPACE