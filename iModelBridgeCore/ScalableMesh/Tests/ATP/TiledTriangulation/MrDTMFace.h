/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/ATP/TiledTriangulation/MrDTMFace.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once
#include <TerrainModel/Core/bcDTMClass.h>
#include <TerrainModel/TerrainModel.h>

USING_NAMESPACE_BENTLEY_TERRAINMODEL
//BEGIN_GEODTMAPP_NAMESPACE


struct FaceWithProperties;
struct MeshFaceProperties;

typedef RefCountedPtr<BcDTMMeshFace> IBcDTMMeshFacePtr;
typedef RefCountedPtr<FaceWithProperties> FaceWithPropertiesPtr;

inline IBcDTMMeshFacePtr GetMeshFacePtr(BcDTMMeshFace* pMeshFace)
    {
    IBcDTMMeshFacePtr meshFacePtr;

    meshFacePtr = pMeshFace;

    pMeshFace->Release();

    return meshFacePtr;
    }

template<class POINT> struct PointComparer2D
    {
    public:
        bool operator()(const POINT& point1, const POINT& point2) const
            {
            if(point1.x < point2.x) return true;
            if(point2.x < point1.x) return false;
            if(point1.y < point2.y) return true;
            if(point2.y < point1.y) return false;
            return false;
            };
    };

struct MeshFaceProperties
    {
    private:
        double m_CircumCenterX;
        double m_CircumCenterY;
        double m_CircumRadiusSquared;
    public:
        void   ComputeBoundingCircle (IBcDTMMeshFacePtr face);
        double GetCircumCenterX() const;
        double GetCircumCenterY() const;
        double GetCircumRadiusSquared() const;
    };

struct FaceWithProperties : public RefCountedBase
    {
    private:
        MeshFaceProperties m_properties;
        IBcDTMMeshFacePtr m_face;
    public:

        FaceWithProperties();
        FaceWithProperties(const FaceWithProperties& face);
        virtual ~FaceWithProperties();

        void ComputeBoundingCircle();
        void SetFace(IBcDTMMeshFacePtr face);
        IBcDTMMeshFacePtr GetFace() const;
        const MeshFaceProperties& GetProperties() const;
    };

//END_GEODTMAPP_NAMESPACE