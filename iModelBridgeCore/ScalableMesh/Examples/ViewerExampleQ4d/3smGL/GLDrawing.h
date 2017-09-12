#pragma once
#include <windows.h>  // for MS Windows
#include <GL/gl.h>  // GLUT, include glu.h and gl.h
#include <GL/glu.h>  // GLUT, include glu.h and gl.h

#include "SMDisplayMgr.h"
#include "CameraModel.h"

using namespace ScalableMesh;

class GLDrawing
    {
    public:
        GLDrawing();

        DPoint3d m_center3SM; // center of the ScalableMesh BBox

        // static member to please OpenGL functions
        static bool m_bCameraChanged;
        static bool m_bDrawBox;
        static int m_nDrawFaces;
        static bool m_bTextured;
        static bool m_bWireframe;
        static bool m_bUseLight;
        static bool m_bSyncCam;
        static bool m_bMove3SMCam;

        static IScalableMeshPtr m_ScalableMesh;
        static CameraModel m_Camera;

        static int refreshTime;        // refresh interval in milliseconds [NEW]
        static void initGL();           // lights

        // Drawing functions
        void CreateTexture(SmCachedDisplayTexture* cacheTex);

        void DrawMeshNodes(bvector<IScalableMeshCachedDisplayNodePtr>& _meshNodes, int displayElement, bool bWire); // Draw list of nodes
        void DrawMeshArray(SmCachedDisplayMesh* aNodeMesh); // draw one mesh node
        void DrawMeshPart(SmCachedDisplayMesh* aNodeMesh); // older, replaced by DrawMeshArray

        void DrawAxes();    // draw the World axis
        void DrawExtend(bvector<IScalableMeshCachedDisplayNodePtr>& _meshNodes); // draw the node extends
    };

