/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__PUBLISH_SECTION_START__*/
#include <ECObjects/ECSchema.h>
#include <ECObjects/ECContext.h>

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

//+===============+===============+===============+===============+===============+======
// Converts EC3 attributes to EC2 custom attributes.  Use the CustomECSchemaConverter to do 
// EC2 custom attribute to EC3 attribute conversion.
// <remarks>
// This class does the following conversions:
// <list type="bullet">
// <item>Creates UnitSpecification and DisplayUnitSpecifications for each property with a KindOfQuantity applied</item>
// </list>
// </remarks>
// @bsiclass                                                    Colin.Kerr   01/2018
//+===============+===============+===============+===============+===============+======
struct ECSchemaDownConverter : NonCopyableClass
    {
    private:
        ECSchemaDownConverter() = delete;
        ECSchemaDownConverter(const ECSchemaDownConverter& rhs) = delete;
        ECSchemaDownConverter & operator= (const ECSchemaDownConverter& rhs) = delete;
    public:

        //! Traverses the schema supplied and converts EC3 concepts back to EC2 custom attributes
        //! @param[in] schema   The schema to traverse
        ECOBJECTS_EXPORT static bool Convert(ECSchemaR schema);
    };

END_BENTLEY_ECOBJECT_NAMESPACE
