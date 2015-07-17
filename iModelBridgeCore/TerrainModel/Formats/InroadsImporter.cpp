/*--------------------------------------------------------------------------------------+
|
|     $Source: Formats/InroadsImporter.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/WString.h>
#include <list>
#include <TerrainModel/Formats/InroadsImporter.h>
#include <Bentley/BeFileName.h>
#include <Bentley/BeFile.h>
#include <Geom\GeomApi.h>
#include "TriangulationPreserver.h"

#include "InroadsTM\InroadsTM.h"

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_TERRAINMODEL

InroadsImporter::InroadsImporter (WCharCP filename) : SingleTerrainImporter (filename)
        {
        }

bool InroadsImporter::IsFileSupported (WCharCP filename)
    {
    if (BeFileName::GetExtension (filename).CompareToI (L"dtm") == 0)
        {
        return true;
        }
        return false;
    }

BENTLEYDTMFORMATS_EXPORT InroadsImporterPtr InroadsImporter::Create (WCharCP filename)
    {
    if (InroadsImporter::IsFileSupported (filename))
        {
        return new InroadsImporter (filename);
        }
    return nullptr;
    }

WCharCP InroadsImporter::_GetFileUnitString () const
    {
    return SingleTerrainImporter::_GetFileUnitString ();
    }


FileUnit InroadsImporter::_GetFileUnit () const
    {
    return SingleTerrainImporter::_GetFileUnit ();
    }

struct InroadsTriangulationPreserver : TriangulationPreserver
    {
    bmap<struct CIVdtmpnt*, long> m_points;
    long m_nextPtNum;
    long m_numTriangles;

    InroadsTriangulationPreserver (BcDTMR dtm) : TriangulationPreserver (dtm)
        {
        m_nextPtNum = 0;
        m_numTriangles = 0;
        }

    int SendTriangleCallback (CIVdtmtin* t)
        {
        m_numTriangles++;
        if (!HasPoint (t->op1))
            AddPoint (t->p1->cor, t->op1);
        if (!HasPoint (t->op2))
            AddPoint (t->p2->cor, t->op2);
        if (!HasPoint (t->op3))
            AddPoint (t->p3->cor, t->op3);

        int ptIds[3];
        ptIds[0] = t->op1;
        ptIds[1] = t->op2;
        ptIds[2] = t->op3;

        AddTriangle (ptIds, 3);
        return SUCCESS;
        }
    };
struct ImporterArg
    {
    const InroadsImporter* importer;
    BcDTMP dtm;
    bool foundPerimeter;
    InroadsTriangulationPreserver* adjustTriangulation;
    };

static void AddFeature (void* dat, bvector<DPoint3d>& points, DTMFeatureType type, struct CIVdtmftr* ftrP)
    {
    if (points.size() > 0)
        {
        ImporterArg* pArg = (ImporterArg*)dat;
        wchar_t style[CIV_C_NAMSIZ+1];
        memset(style, 0, sizeof(style));
        if (ftrP->numStyles > 0)
            memcpy (style, ftrP->s1->nam, sizeof(ftrP->s1->nam));

        if (ftrP->flg & DTM_C_FTRTIN)
            {
            if (pArg->importer->GetCallback())
                pArg->importer->GetCallback()->AddFeature (DTM_NULL_FEATURE_ID, L"", style, ftrP->nam, ftrP->des, type, &points[0], points.size ());
            }
        else
            {
            if (type == DTMFeatureType::RandomSpots)
                pArg->dtm->AddPoints (&points[0], (int)points.size ());
            else
                {
                DTMUserTag dtmUserTag = 0;
                DTMFeatureId id;
                if (type == DTMFeatureType::GroupSpots)
                    pArg->dtm->AddPointFeature (&points[0], (int)points.size (), dtmUserTag, &id);
                else
                    pArg->dtm->AddLinearFeature (type, &points[0], (int)points.size (), dtmUserTag, &id);
                if (pArg->importer->GetCallback())
                    pArg->importer->GetCallback()->AddFeature (id, L"", style, ftrP->nam, ftrP->des, type, &points[0], points.size ());
                if (type == DTMFeatureType::Hull)
                    pArg->foundPerimeter = true;
                }
            }
        points.clear ();
        }
    }

static int SendFeaturesCallback (void* dat, struct CIVdtmsrf* srf, int ftrTyp, struct CIVdtmftr* ftrP)
    {
    if (ftrP->numPnts > 0)
        {
        DTMFeatureType type = DTMFeatureType::None;

        switch (ftrTyp)
            {
            case DTM_C_DTMREGFTR:
                if (wcslen (ftrP->nam) > 0)
                    type = DTMFeatureType::GroupSpots;
                else
                    type = DTMFeatureType::RandomSpots;
                break;

            case DTM_C_DTMBRKFTR:
                type = DTMFeatureType::Breakline;
                break;

            case DTM_C_DTMINTFTR:
                type = DTMFeatureType::Void;
                break;

            case DTM_C_DTMEXTFTR:
                type = DTMFeatureType::Hull;
                break;

            case DTM_C_DTMCTRFTR:
                type = DTMFeatureType::ContourLine;
                break;
            default:
                type = DTMFeatureType::None;
                break;
            }

        bvector<DPoint3d> points;
        if (ftrP->p1)
            {
            for (int i = 0; i < ftrP->numPnts; i++)
                {
                if (i > 0 && (ftrP->p1[i].flg & DTM_C_PNTPUD) == FALSE && !(ftrTyp != DTM_C_DTMREGFTR || ftrTyp != DTM_C_DTMEXTFTR))
                    {
                    AddFeature (dat, points, type, ftrP);
                    }

                if ((ftrP->p1[i].flg & DTM_C_PNTDEL) == FALSE)
                    {
                    points.push_back (ftrP->p1[i].cor);
                    }
                }

            AddFeature (dat, points, type, ftrP);
            }
        else
            {
            ftrP->p1 = nullptr;
            }
        }

    return SUCCESS;
    }

static int SendTriangleCallback (void* dat, long, DPoint3d*,  struct CIVdtmtin* t, unsigned long)
    {
    ImporterArg* pArg = (ImporterArg*)dat;
    // Check for colinear triangle.
    if (bcdtmMath_sideOf (t->p1->cor.x, t->p1->cor.y, t->p3->cor.x, t->p3->cor.y, t->p2->cor.x, t->p2->cor.y) == 0)
        return DTM_SUCCESS;

    return pArg->adjustTriangulation->SendTriangleCallback (t);
    }

static void AddSurfacePerimeter (ImporterArg& arg, struct CIVdtmsrf* pSrf)
    {
    DPoint3d* vrtsP = NULL;
    long numVrts = 0;
    if (inroadsTM_getSurfacePerimeter (&numVrts, &vrtsP, pSrf) == SUCCESS)
        {
        DTMFeatureId id;
        BcDTMPtr dtm = BcDTM::Create();
        dtm->AddLinearFeature (DTMFeatureType::Breakline, vrtsP, numVrts, &id);
        dtm->Triangulate ();
        dtm->RemoveNoneFeatureHullLines ();
        DTMPointArray pts;
        if (SUCCESS == dtm->GetBoundary (pts))
            arg.dtm->AddLinearFeature (DTMFeatureType::Hull, pts.data(), (int)pts.size(), &id);
        else
            arg.dtm->AddLinearFeature (DTMFeatureType::Hull, vrtsP, numVrts, &id);

        if (arg.importer->GetCallback())
            arg.importer->GetCallback()->AddFeature (id, L"", L"", L"", L"", DTMFeatureType::Hull, vrtsP, numVrts);
        }
    if (vrtsP) free (vrtsP);
    }


ImportedTerrain InroadsImporter::_ImportTerrain (WCharCP name) const
    {
    if (name == m_name)
        {
        struct CIVdtmsrf* pSrf = NULL;
        if (SUCCESS == inroadsTM_load (&pSrf, NULL, NULL, (wchar_t*)m_fileName.GetWCharCP ()))
            {
            BcDTMPtr dtm = BcDTM::Create ();
            dtm->SetCleanUpOptions (DTMCleanupFlags::All);
            dtm->SetTriangulationParameters (0.001, 0.001, 0, 100);
            if (!m_callback || !m_callback->StartTerrain (name, L"", dtm))
                {
                ImporterArg arg;
                arg.dtm = dtm.get();
                arg.importer = this;
                arg.foundPerimeter = false;
                inroadsTM_sendAllFeatures (NULL, pSrf, DTM_C_NOBREK, 0, SendFeaturesCallback, (void*)&arg);
                if (!arg.foundPerimeter)
                    AddSurfacePerimeter (arg, pSrf);

                InroadsTriangulationPreserver adjustTriangulation (*dtm);
                arg.adjustTriangulation = &adjustTriangulation;
                inroadsTM_sendAllTriangles (NULL, pSrf, DTM_C_NOBREK, SendTriangleCallback, &arg);
                adjustTriangulation.Finish ();
                inroadsTM_deleteSurface (NULL, pSrf, FALSE);
                if (m_callback)
                    m_callback->EndTerrain (name, dtm.get ());
                }
            return ImportedTerrain (dtm.get (), name, nullptr, true);
            }
        }
    return ImportedTerrain (nullptr, name, nullptr, false);
    }
 