/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "DgnHandlersTests.h"

#define DGN_SQL_TEST_SCHEMA_NAME   "DgnPlatformTest"
#define DGN_SQL_TEST_SCHEMA_NAMEW L"DgnPlatformTest"
#define DGN_SQL_TEST_ROBOT_CLASS   "Robot"
#define DGN_SQL_TEST_OBSTACLE_CLASS "Obstacle"
#define DGN_SQL_TEST_TEST_UNIQUE_ASPECT_CLASS_NAME                  "DgnSqlTestUniqueAspect"
#define DGN_SQL_TEST_TEST_MULTI_ASPECT_CLASS_NAME                   "DgnSqlTestMultiAspect"

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
    static DgnClassId QueryClassId(DgnDbR db) {return DgnClassId(db.Schemas().GetClassId(DGN_SQL_TEST_SCHEMA_NAME, DGN_SQL_TEST_ROBOT_CLASS));}

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
    static DgnClassId QueryClassId(DgnDbR db) {return DgnClassId(db.Schemas().GetClassId(DGN_SQL_TEST_SCHEMA_NAME, DGN_SQL_TEST_OBSTACLE_CLASS));}

    //! Set the value of the "SomeProperty" property
    void SetSomeProperty(Utf8CP value);

    //! An Obstacle can have an associated "TestUniqueAspect". This method sets the value of the TestUniqueAspectProperty of that aspect, inserting the aspect if necessary.
    void SetTestUniqueAspect(Utf8CP itemPropertyValue);

    //! Get the value of the TestUniqueAspectProperty property of the Obstacle's TestUniqueAspect.
    Utf8String GetTestUniqueAspect() const;
    };

typedef RefCountedPtr<RobotElement>     RobotElementPtr;
typedef RefCountedPtr<ObstacleElement>  ObstacleElementPtr;
typedef RefCountedCPtr<RobotElement>     RobotElementCPtr;
typedef RefCountedCPtr<ObstacleElement>  ObstacleElementCPtr;

//=======================================================================================
// @bsiclass                                                     Sam.Wilson      01/15
//=======================================================================================
struct RobotElementHandler : dgn_ElementHandler::Physical
{
    ELEMENTHANDLER_DECLARE_MEMBERS ("Robot", RobotElement, RobotElementHandler, dgn_ElementHandler::Physical, )
};

//=======================================================================================
// @bsiclass                                                     Sam.Wilson      01/15
//=======================================================================================
struct ObstacleElementHandler : dgn_ElementHandler::Physical
{
    ELEMENTHANDLER_DECLARE_MEMBERS ("Obstacle", ObstacleElement, ObstacleElementHandler, dgn_ElementHandler::Physical, )
};

//=======================================================================================
//! A test Domain
// @bsiclass                                                     Sam.Wilson      01/15
//=======================================================================================
struct DgnSqlTestDomain : DgnDomain
{
    DOMAIN_DECLARE_MEMBERS(DgnSqlTestDomain, )

private:
    WCharCP _GetSchemaRelativePath() const override { return L"ECSchemas/" DGN_SQL_TEST_SCHEMA_NAMEW L".ecschema.xml"; }

public:
    DgnSqlTestDomain();
};

}; // namespace DgnSqlTestNamespace

