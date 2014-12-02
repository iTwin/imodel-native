/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Published/IEditInterface_Test.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnHandlersTests.h"

USING_NAMESPACE_BENTLEY_DGNPLATFORM

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                                     Evan.Williams  11/09
+---------------+---------------+---------------+---------------+---------------+------*/
class IEditInterfaceTest : public GenericDgnModelTestFixture
{
protected:
EditElementHandle      m_eeh;

public:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Evan.Williams  11/09
+---------------+---------------+---------------+---------------+---------------+------*/
IEditInterfaceTest()
    : GenericDgnModelTestFixture (__FILE__, true)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Evan.Williams  11/09
+---------------+---------------+---------------+---------------+---------------+------*/
virtual ~IEditInterfaceTest ()
    {
    m_eeh.Invalidate();
    }
};

#if defined (NEEDS_WORK_DGNITEM)
/*---------------------------------------------------------------------------------**//**
* Create and modify a BsplineSurface using SetBsplineSurface() on IBSplineSurfaceEdit.
* Will flip a BsplineSurface and elongate it.
* @bsimethod                                                    Evan.Williams  12/09
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(IEditInterfaceTest, IBsplineSurfaceEdit)
    {
    //SETUP: Create a upward paraboloid-shaped BsplineSurface with a square base and pass to m_eeh
    DPoint3d points[] = {{1000.0, 3000.0, 500.0},
                         {2000.0, 3000.0, 0.0},
                         {3000.0, 3000.0, 500.0},
                         {1000.0, 2000.0, 0.0},
                         {2000.0, 2000.0, -5000.0},
                         {3000.0, 2000.0, 0.0},
                         {1000.0, 1000.0, 500.0},
                         {2000.0, 1000.0, 0.0},
                         {3000.0, 1000.0, 500.0}};

    bvector<DPoint3d> surfPoints;
    for(size_t i = 0; i < _countof(points); i++)
        surfPoints.push_back(points[i]);
    
    MSBsplineSurface surface;
    surface.Populate (surfPoints, NULL, NULL, 3, 3, false, NULL, 3, 3, false, false);
    
    ASSERT_EQ (SUCCESS, BSplineSurfaceHandler::CreateBSplineSurfaceElement (m_eeh, NULL, surface, *GetDgnModelP ()));
    
    //EDIT: Flip the original surface so that it is opening downward and elongate it to extend to z=10000.0
    DPoint3d editPoints[] = {{1000.0, 3000.0, -500.0},
                             {2000.0, 3000.0, 0.0},
                             {3000.0, 3000.0, -500.0},
                             {1000.0, 2000.0, 0.0},
                             {2000.0, 2000.0, 10000.0},
                             {3000.0, 2000.0, 0.0},
                             {1000.0, 1000.0, -500.0},
                             {2000.0, 1000.0, 0.0},
                             {3000.0, 1000.0, -500.0}};
    
    bvector<DPoint3d> editSurfPoints;
    for(size_t i = 0; i < _countof(editPoints); i++)
        editSurfPoints.push_back(editPoints[i]);
    
    MSBsplineSurface editSurface;
    editSurface.Populate (editSurfPoints, NULL, NULL, 3, 3, false, NULL, 3, 3, false, false);
    
    IBsplineSurfaceEdit* editor = dynamic_cast<IBsplineSurfaceEdit*>(&m_eeh.GetHandler());
    editor->SetBsplineSurface(m_eeh, editSurface);
    
    //CHECK: Make sure the changes specified in the editor carried over by cross-checking the poles
    MSBsplineSurfacePtr checkSurface;
    editor->GetBsplineSurface(m_eeh, checkSurface);
    
    for(size_t i = 0; i < _countof(editPoints); i++)
        {
        EXPECT_EQ(editSurface.poles[i].x, checkSurface->poles[i].x);
        EXPECT_EQ(editSurface.poles[i].y, checkSurface->poles[i].y);
        EXPECT_EQ(editSurface.poles[i].z, checkSurface->poles[i].z);
        }

    surface.ReleaseMem();
    editSurface.ReleaseMem();
    }
#endif

#if defined (DGNPLATFORM_WIP_ISolidPrimitiveQuery)
/*---------------------------------------------------------------------------------**//**
* Create and modify a cone using SetConeData()
* @bsimethod                                                    Evan.Williams  12/09
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(IEditInterfaceTest, IConeEdit)
    {
    //SETUP: Create a pointed and capped cone facing +z with bottom radius 500.0 and pass m_eeh.
    DPoint3d    top = {0.0, 0.0, 1000.0};
    DPoint3d    bottom = {0.0, 0.0, 0.0};
    RotMatrix rMatrix;
    rMatrix.InitIdentity ();
    
    ASSERT_EQ(SUCCESS, ConeHandler::CreateConeElement(m_eeh, NULL, 0.0, 500.0, top, bottom, rMatrix, true, *GetDgnModelP()));
    m_eeh.AddToModel (/*GetDgnModelP()*/);
    
    //EDIT: Transform the previous cone into a circular cylinder with major axis (1000.0, 1000.0), 0.0 < z < 5000.0
    //      Both ends have a radius of 1000.0.
    DPoint3d    editTop = {1000.0, 1000.0, 5000.0};
    DPoint3d    editBottom = {1000.0, 1000.0, 0.0};
    double      editTopRadius = 1000.0;
    double      editBottomRadius = 1000.0;
    
    IConeEdit* editor = dynamic_cast<IConeEdit*> (&m_eeh.GetHandler());
    editor->SetConeData(m_eeh, rMatrix, editTop, editBottom, editTopRadius, editBottomRadius); //turning the cone into a cylinder and displacing it into the first octant
    
    //CHECK: Ensure that the cone in m_eeh has all the parameters specified by the editor
    DPoint3d checkTop;
    DPoint3d checkBottom;
    double r0 = 0;
    double r1 = 0;
    editor->GetConeData(m_eeh, NULL, &checkTop, &checkBottom, &r0, &r1);
    
    EXPECT_EQ(editTop.x, checkTop.x);
    EXPECT_EQ(editTop.y, checkTop.y);
    EXPECT_EQ(editTop.z, checkTop.z);
    EXPECT_EQ(editBottom.x, checkBottom.x);
    EXPECT_EQ(editBottom.y, checkBottom.y);
    EXPECT_EQ(editBottom.z, checkBottom.z);
    EXPECT_EQ(editTopRadius, r0);
    EXPECT_EQ(editBottomRadius, r1);
    }
#endif

#if defined (NEEDS_WORK_DGNITEM)
/*---------------------------------------------------------------------------------**//**
* Create and modify a Mesh using SetMeshData() on IMeshEdit
* Turn a basic square mesh into a more complex one.
* @bsimethod                                                    Evan.Williams  12/09
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(IEditInterfaceTest, IMeshEdit)
    {
    //SETUP: Create a basic square mesh and pass to m_eeh
    PolyfaceHeaderPtr header = PolyfaceHeader::New ();
    header->SetMeshStyle (MESH_ELM_STYLE_INDEXED_FACE_LOOPS);
    header->SetTwoSided (false);
    header->SetNumPerFace (0);
    
    DPoint3d points[] = {{1000.0, 1000.0, 0.0},
                         {2000.0, 1000.0, 0.0},
                         {2000.0, 2000.0, 0.0},
                         {1000.0, 2000.0, 0.0}};
    
    int indices[] = {1, 2, 3, 4};
    
    header->Point().SetActive (true);
    header->PointIndex().SetActive (true);
    for (int i = 0; i < _countof (indices); i++)
        header->PointIndex().push_back (indices[i]);
    for (int i = 0; i < _countof (points); i++)
        header->Point ().push_back (points[i]);
    ASSERT_EQ(SUCCESS, MeshHeaderHandler::CreateMeshElement (m_eeh, NULL, *header, Is3d(), *GetDgnModelP()));
    
    //EDIT: Use IMeshEdit to transform the basic square mesh into a much more complex one.
    PolyfaceHeaderPtr editHeader = PolyfaceHeader::New();
    editHeader->SetMeshStyle (MESH_ELM_STYLE_INDEXED_FACE_LOOPS);
    editHeader->SetTwoSided (false);
    editHeader->SetNumPerFace (0);
    
    DPoint3d editPoints[] = {{1000.0, 3000.0, 500.0},
                             {2000.0, 3000.0, 0.0},
                             {3000.0, 3000.0, 500.0},
                             {1000.0, 2000.0, 0.0},
                             {2000.0, 2000.0, -5000.0},
                             {3000.0, 2000.0, 0.0},
                             {1000.0, 1000.0, 500.0},
                             {2000.0, 1000.0, 0.0},
                             {3000.0, 1000.0, 500.0}};
    
    int editIndices [] = {0,1,2,3,4,5,6,7};
    
    editHeader->Point().SetActive (true);
    editHeader->PointIndex().SetActive (true);
    for (int i = 0; i < _countof (editIndices); i++)
        editHeader->PointIndex().push_back (editIndices[i]);
    for (int i = 0; i < _countof (editPoints); i++)
        editHeader->Point ().push_back (editPoints[i]);
    
    IMeshEdit* editor = dynamic_cast<IMeshEdit*> (&m_eeh.GetHandler());
    editor->SetMeshData(m_eeh, *editHeader);
    
    //CHECK: Cross-check the edit and check PolyfaceHeaders to make sure that the edit was carried out properly.
    PolyfaceHeaderPtr checkHeader = PolyfaceHeader::New();
    editor->GetMeshData(m_eeh, checkHeader);
    
    for(size_t i = 0; i < checkHeader->Point().size(); i++)
        {
        EXPECT_EQ(editHeader->Point().at(i).x, checkHeader->Point().at(i).x);
        EXPECT_EQ(editHeader->Point().at(i).y, checkHeader->Point().at(i).y);
        EXPECT_EQ(editHeader->Point().at(i).z, checkHeader->Point().at(i).z);
        }
    for(size_t i = 0; i < checkHeader->PointIndex().size(); i++)
        {
        EXPECT_EQ(editHeader->PointIndex().at(i), checkHeader->PointIndex().at(i));
        EXPECT_EQ(editHeader->PointIndex().at(i), checkHeader->PointIndex().at(i));
        EXPECT_EQ(editHeader->PointIndex().at(i), checkHeader->PointIndex().at(i));
        }
    }

#endif
