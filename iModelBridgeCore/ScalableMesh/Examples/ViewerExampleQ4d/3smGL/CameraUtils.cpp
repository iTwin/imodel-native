#include "SMDisplayMgr.h"
using namespace ScalableMesh;



void SetCamera(DMatrix4d& matrix, double fov, double aspect, double zNear, double zFar, DPoint3d eye, DPoint3d target)
    {
    double proj[16] = { 0 };
    proj[0] = fov / aspect;
    proj[5] = fov;
    proj[10] = (zFar + zNear) / (zNear - zFar);
    proj[11] = -1;
    proj[15] = (2 * zFar*zNear) / (zNear - zFar);

    DMatrix4d projM;
    projM.InitFromColumnVectors(DVec3d::From(proj[0], proj[1], proj[2]), // model[3]==0 
        DVec3d::From(proj[4], proj[5], proj[6]),
        DVec3d::From(proj[8], proj[9], proj[10]),
        DVec3d::From(proj[12], proj[13], proj[14]));

    }