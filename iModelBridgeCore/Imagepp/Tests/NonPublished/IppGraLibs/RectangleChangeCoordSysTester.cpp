#include "../imagepptestpch.h"
#include "NonLinearTransforms.h"

static HFCPtr<HGF2DWorldCluster> s_pHMRWorld(new HGFHMRStdWorldCluster());

TEST(RectangleChangeCoordSysTester, CompareHGF2DRectangleAndHVEShape)
    {
    // Create the coordinate systems
    HFCPtr<HGF2DCoordSys> pSourceCoordSys = s_pHMRWorld->GetCoordSysReference(HGF2DWorld_UNKNOWNWORLD);

    HGF2DStretch transfoModel1(HGF2DDisplacement(-1930.96, 92.74), 21.31095, 21.31095);
    HGF2DStretch transfoModel2(HGF2DDisplacement(-180, -90), 0.01666667, 0.01666667);
    ThirdDegreeTransfoModel transfoModel3;
    HGF2DStretch transfoModel4(HGF2DDisplacement(0.0, 0.0), 1.0, -1.0);
    HGF2DStretch transfoModel5(HGF2DDisplacement(337.5, 168.75), 1.875, -1.875);

    HFCPtr<HGF2DTransfoModel> pFinalTransfoModel = transfoModel3.ComposeInverseWithDirectOf(transfoModel4);
    pFinalTransfoModel = pFinalTransfoModel->ComposeInverseWithDirectOf(transfoModel5);
    pFinalTransfoModel = transfoModel2.ComposeInverseWithDirectOf(*pFinalTransfoModel);
    pFinalTransfoModel = transfoModel1.ComposeInverseWithDirectOf(*pFinalTransfoModel);

    HFCPtr<HGF2DCoordSys> pDestCoordSys = new HGF2DCoordSys(*pFinalTransfoModel, pSourceCoordSys);

    // Declare the rectangle in the destination coord sys
    double minX = 512;
    double minY = 0;
    double maxX = 1024;
    double maxY = 256;

    //Transform via HGF2DRectangle
    HGF2DRectangle currentExtentHGF(minX, minY, maxX, maxY);
    HFCPtr<HGF2DTransfoModel> transfoModel = pDestCoordSys->GetTransfoModelTo(pSourceCoordSys);
    HFCPtr<HGF2DShape> resultShape = currentExtentHGF.AllocTransformDirect(*pFinalTransfoModel);
    HGF2DLiteExtent finalExtentHGFRectangle = resultShape->GetExtent();

    //Transform via HVEShape
    HVEShape extentShapeHVE(minX, minY, maxX, maxY, pDestCoordSys);
    extentShapeHVE.ChangeCoordSys(pSourceCoordSys);
    HGF2DExtent finalExtentHVEShape = extentShapeHVE.GetExtent();

    //Compare results
    ASSERT_EQ(finalExtentHVEShape.GetXMin(), finalExtentHGFRectangle.GetXMin());
    ASSERT_EQ(finalExtentHVEShape.GetYMin(), finalExtentHGFRectangle.GetYMin());
    ASSERT_EQ(finalExtentHVEShape.GetXMax(), finalExtentHGFRectangle.GetXMax());
    ASSERT_EQ(finalExtentHVEShape.GetYMax(), finalExtentHGFRectangle.GetYMax());
    }

