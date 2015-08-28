/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/NonPublished/DgnSqlTestDomain.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnHandlersTests.h"

#define DGN_SQL_TEST_SCHEMA_NAME   "DgnPlatformTest"
#define DGN_SQL_TEST_SCHEMA_NAMEW L"DgnPlatformTest"
#define DGN_SQL_TEST_ROBOT_CLASS   "Robot"
#define DGN_SQL_TEST_OBSTACLE_CLASS "Obstacle"
#define DGN_SQL_TEST_TEST_ITEM_CLASS_NAME                       "TestItem"
#define DGN_SQL_TEST_TEST_ITEM_TestItemProperty               L"TestItemProperty"
#define DGN_SQL_TEST_TEST_ITEM_TestItemPropertyA               "TestItemProperty"

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
    RobotElement(PhysicalModelR model, DgnCategoryId categoryId, DPoint3dCR origin, double yaw, Code elementCode);
public:
    //! Factory method that creates an instance of a RobotElement
    //! @param[in] model Create the RobotElement in this PhysicalModel
    //! @param[in] categoryId specifies the category for the RobotElement.
    static RefCountedPtr<RobotElement> Create(PhysicalModelR model, DgnCategoryId categoryId, DPoint3dCR origin, double yaw, DgnElement::Code elementCode)
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
    ObstacleElement(PhysicalModelR model, DgnCategoryId categoryId, DPoint3dCR origin, double yaw, DgnElement::Code elementCode);
public:
    //! Factory method that creates an instance of a ObstacleElement
    //! @param[in] model Create the ObstacleElement in this PhysicalModel
    //! @param[in] categoryId specifies the category for the ObstacleElement.
    static RefCountedPtr<ObstacleElement> Create(PhysicalModelR model, DgnCategoryId categoryId, DPoint3dCR origin, double yaw, DgnElement::Code elementCode)
        {return new ObstacleElement(model, categoryId, origin, yaw, elementCode);}

    //! Query the DgnClassId for the Obstacle ECClass in the specified DgnDb.
    static DgnClassId QueryClassId(DgnDbR db) {return DgnClassId(db.Schemas().GetECClassId(DGN_SQL_TEST_SCHEMA_NAME, DGN_SQL_TEST_OBSTACLE_CLASS));}

    //! Set the value of the "SomeProperty" property
    void SetSomeProperty(DgnDbR db, Utf8CP value);

    //! An Obstacle can have an associated "TestItem". This method sets the value of the TestItem, inserting the item if necessary.
    void SetTestItem(DgnDbR db, Utf8CP itemPropertyValue);
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
public:
    DgnSqlTestDomain();

    static void RegisterDomainAndImportSchema(DgnDbR, BeFileNameCR schemasDir);
    };

}; // namespace DgnSqlTestNamespace

