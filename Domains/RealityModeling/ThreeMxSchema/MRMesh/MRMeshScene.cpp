/*--------------------------------------------------------------------------------------+
|
|     $Source: ThreeMxSchema/MRMesh/MRMeshScene.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "..\ThreeMxSchemaInternal.h"

USING_NAMESPACE_BENTLEY_DGNPLATFORM


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   MRMeshScene::Initialize (S3SceneInfo const& sceneInfo, WCharCP fileName)
    {
    m_fileName  = BeFileName (fileName);
    m_sceneName = WString (sceneInfo.sceneName.c_str(), false);
    m_srs       = WString (sceneInfo.SRS.c_str(), false);

    if (3 == sceneInfo.SRSOrigin.size ())
        m_srsOrigin.InitFromArray (&sceneInfo.SRSOrigin.front ());
    else
        m_srsOrigin.Zero ();

    BeFileName   scenePath (BeFileName::DevAndDir, m_fileName.c_str());
    for (bvector<std::string>::const_iterator child = sceneInfo.meshChildren.begin (); child != sceneInfo.meshChildren.end (); child++)
        {
        MRMeshNodePtr       childNode = MRMeshNode::Create ();

        if (SUCCESS == MRMeshCacheManager::GetManager().SynchronousRead (*childNode, MRMeshUtil::ConstructNodeName (*child, &scenePath)))
            {
            childNode->LoadUntilDisplayable ();
            m_children.push_back (childNode);
            }
        }
    return SUCCESS;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
static void drawBoundingSpheres (MRMeshNodeR node, ViewContextR viewContext)
    {
    if (node.IsLoaded())
        {
        if (node.IsDisplayable())
            node.DrawBoundingSphere (viewContext);

        for (bvector<MRMeshNodePtr>::iterator child = node.m_children.begin (); child != node.m_children.end (); child++)
            drawBoundingSpheres (**child, viewContext);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void        MRMeshScene::DrawBoundingSpheres (ViewContextR viewContext)
    {
    for (bvector<MRMeshNodePtr>::iterator child = m_children.begin (); child != m_children.end (); child++)
        drawBoundingSpheres (**child, viewContext);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void    MRMeshScene::Draw (ViewContextR viewContext, MRMeshContextCR MeshContext)
    {
    for (bvector<MRMeshNodePtr>::iterator child = m_children.begin (); child != m_children.end (); child++)
        {
        if (viewContext.CheckStop())
            return;

        (*child)->Draw (viewContext, MeshContext);
        }
    }


/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void MRMeshScene::DrawMeshes (IDrawGeomP drawGeom, TransformCR transform)
    {
    for (bvector<MRMeshNodePtr>::iterator child = m_children.begin (); child != m_children.end (); child++)
        (*child)->DrawMeshes (drawGeom, transform);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
size_t      MRMeshScene::GetMeshMemorySize () const
    {
    size_t      memorySize = 0;

    for (bvector<MRMeshNodePtr>::const_iterator child = m_children.begin (); child != m_children.end (); child++)
        memorySize += (*child)->GetMeshMemorySize();

    return memorySize;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
size_t      MRMeshScene::GetTextureMemorySize () const
    {
    size_t      memorySize = 0;

    for (bvector<MRMeshNodePtr>::const_iterator child = m_children.begin (); child != m_children.end (); child++)
        memorySize += (*child)->GetTextureMemorySize();

    return memorySize;
    }


/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
size_t      MRMeshScene::GetNodeCount () const
    {
    size_t      count = 1;

    for (bvector<MRMeshNodePtr>::const_iterator child = m_children.begin (); child != m_children.end (); child++)
        count += (*child)->GetNodeCount ();

    return count;
    }


/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
size_t      MRMeshScene::GetMaxDepth () const
    {
    size_t maxChildDepth = 0, childDepth;

    for (bvector<MRMeshNodePtr>::const_iterator child = m_children.begin (); child != m_children.end (); child++)
        if ((childDepth = (*child)->GetMaxDepth()) > maxChildDepth)
            maxChildDepth = childDepth;

    return 1 + maxChildDepth;
    }











