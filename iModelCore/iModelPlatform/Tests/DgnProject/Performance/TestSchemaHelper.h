/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECObjects/ECObjectsAPI.h>
#include "../BackDoor/PublicAPI/BackDoor/DgnProject/BackDoor.h"

BEGIN_DGNDB_UNIT_TESTS_NAMESPACE

//=======================================================================================    
// @bsiclass
//=======================================================================================    
struct TestSchemaHelper
    {
private:
    static Utf8String GetComplexTestSchemaXml ();
    static Utf8String GetTestSchemaXml (int integerPropCount, int stringPropCount);
    static ECN::ECSchemaPtr DeserializeSchema (ECN::ECSchemaReadContextPtr& schemaReadContext, Utf8StringCR schemaXml);

public:
    static Utf8CP const TESTSCHEMA_NAME;
    static WCharCP const TESTCLASS_NAME;

    //!---------------------------------------------------------------------------------
    //! Creates the complex test schema.
    //! @remarks This is the same schema used by DgnEC ATPs. Only for PIPE_Extra the property Tag was
    //!          removed as the class already inherited a property TAG from TAGGED_ITEM.
    //! @param[out] schemaReadContext Schema read context
    //! @returns Created test schema
    //!---------------------------------------------------------------------------------
    static ECN::ECSchemaPtr CreateComplexTestSchema (ECN::ECSchemaReadContextPtr& schemaReadContext);
    
    //!---------------------------------------------------------------------------------
    //! Creates a test schema with varying numbers of properties.
    //! @remarks This is the same schema used by DgnEC ATPs. 
    //! @param[out] schemaReadContext Schema read context
    //! @param[in] integerPropCount number of integer properties the schema should have
    //! @param[in] stringPropCount number of string properties the schema should have
    //! @returns Created test schema
    //!---------------------------------------------------------------------------------
    static ECN::ECSchemaPtr CreateTestSchema (ECN::ECSchemaReadContextPtr& schemaReadContext, int integerPropCount, int stringPropCount);
    };

END_DGNDB_UNIT_TESTS_NAMESPACE
