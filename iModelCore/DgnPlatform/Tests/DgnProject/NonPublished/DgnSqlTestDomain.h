/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/NonPublished/DgnSqlTestDomain.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnHandlersTests.h"

#define DGN_SQL_TEST_SCHEMA_NAME   "DgnPlatformTest"
#define DGN_SQL_TEST_SCHEMA_NAMEW L"DgnPlatformTest"
#define DGN_SQL_TEST_ROBOT_CLASS   "Robot"
#define DGN_SQL_TEST_OBSTACLE_CLASS "Obstacle"
#define DGN_SQL_TEST_TEST_UNIQUE_ASPECT_CLASS_NAME                  "TestUniqueAspect"
#define DGN_SQL_TEST_TEST_UNIQUE_ASPECT_TestUniqueAspectProperty    L"TestUniqueAspectProperty"
#define DGN_SQL_TEST_TEST_UNIQUE_ASPECT_TestUniqueAspectPropertyA   "TestUniqueAspectProperty"

namespace DgnSqlTestNamespace {

struct RobotElementHandler;
struct ObstacleElementHandler;

//=======================================================================================
// @bsiclass                                                    Sam.Wilson      05/15
//=======================================================================================
struct RobotElement : PhysicalElement
{
    DGNELEMENT_DECLARE_MEMBERS(DGN_SQL_TEST_ROBOT_CLASS, PhysicalElement)
private:
    friend struct RobotElementHandler;
    RobotElement(PhysicalElement::CreateParams const& params) : PhysicalElement(params) {;}
    RobotElement(SpatialModelR model, DgnCategoryId categoryId, DPoint3dCR origin, double yaw, DgnCode elementCode);
public:
    //! Factory method that creates an instance of a RobotElement
    //! @param[in] model Create the RobotElement in this SpatialModel
    //! @param[in] categoryId specifies the category for the RobotElement.
    static RefCountedPtr<RobotElement> Create(SpatialModelR model, DgnCategoryId categoryId, DPoint3dCR origin, double yaw, DgnCode elementCode)
        {return new RobotElement(model, categoryId, origin, yaw, elementCode);}

    //! Query the DgnClassId for the Robot ECClass in the specified DgnDb.
    static DgnClassId QueryClassId(DgnDbR db) {return DgnClassId(db.Schemas().GetECClassId(DGN_SQL_TEST_SCHEMA_NAME, DGN_SQL_TEST_ROBOT_CLASS));}

    void Translate(DVec3dCR offset);
};

//=======================================================================================
// @bsiclass                                                    Sam.Wilson      05/15
//=======================================================================================
struct ObstacleElement : PhysicalElement
{
    DGNELEMENT_DECLARE_MEMBERS(DGN_SQL_TEST_OBSTACLE_CLASS, PhysicalElement)

private:
    friend struct ObstacleElementHandler;
    ObstacleElement(PhysicalElement::CreateParams const& params) : PhysicalElement(params) {;}
    ObstacleElement(SpatialModelR model, DgnCategoryId categoryId, DPoint3dCR origin, double yaw, DgnCode elementCode);
public:
    //! Factory method that creates an instance of a ObstacleElement
    //! @param[in] model Create the ObstacleElement in this SpatialModel
    //! @param[in] categoryId specifies the category for the ObstacleElement.
    static RefCountedPtr<ObstacleElement> Create(SpatialModelR model, DgnCategoryId categoryId, DPoint3dCR origin, double yaw, DgnCode elementCode)
        {return new ObstacleElement(model, categoryId, origin, yaw, elementCode);}

    //! Query the DgnClassId for the Obstacle ECClass in the specified DgnDb.
    static DgnClassId QueryClassId(DgnDbR db) {return DgnClassId(db.Schemas().GetECClassId(DGN_SQL_TEST_SCHEMA_NAME, DGN_SQL_TEST_OBSTACLE_CLASS));}

    //! Set the value of the "SomeProperty" property
    void SetSomeProperty(DgnDbR db, Utf8CP value);

    //! An Obstacle can have an associated "TestUniqueAspect". This method sets the value of the TestUniqueAspect, inserting the item if necessary.
    void SetTestUniqueAspect(DgnDbR db, Utf8CP itemPropertyValue);
};

typedef RefCountedPtr<RobotElement>     RobotElementPtr;
typedef RefCountedPtr<ObstacleElement>  ObstacleElementPtr;
typedef RefCountedCPtr<RobotElement>     RobotElementCPtr;
typedef RefCountedCPtr<ObstacleElement>  ObstacleElementCPtr;

//=======================================================================================
// @bsiclass                                                     Sam.Wilson      01/15
//=======================================================================================
struct RobotElementHandler : dgn_ElementHandler::Geometric3d
{
    ELEMENTHANDLER_DECLARE_MEMBERS ("Robot", RobotElement, RobotElementHandler, dgn_ElementHandler::Geometric3d, )
};

//=======================================================================================
// @bsiclass                                                     Sam.Wilson      01/15
//=======================================================================================
struct ObstacleElementHandler : dgn_ElementHandler::Geometric3d
{
    ELEMENTHANDLER_DECLARE_MEMBERS ("Obstacle", ObstacleElement, ObstacleElementHandler, dgn_ElementHandler::Geometric3d, )
};

//=======================================================================================
//! A test Domain
// @bsiclass                                                     Sam.Wilson      01/15
//=======================================================================================
struct DgnSqlTestDomain : DgnDomain
    {
    DOMAIN_DECLARE_MEMBERS(DgnSqlTestDomain, )
public:
    DgnSqlTestDomain();

    static void ImportSchema(DgnDbR, BeFileNameCR schemasDir);
    };

}; // namespace DgnSqlTestNamespace

