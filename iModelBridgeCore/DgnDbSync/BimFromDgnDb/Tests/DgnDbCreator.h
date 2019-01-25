/*--------------------------------------------------------------------------------------+
|
|     $Source: BimFromDgnDb/Tests/DgnDbCreator.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <BimFromDgnDb/BimFromDgnDb.h>

namespace BentleyG0601
    {
    namespace Dgn
        {
        namespace BimFromDgnDbTest
            {
            struct DgnDbCreatorImpl;
            }
        }
    }

#ifdef __BIM_FROM_DGNDB_BUILD__
#define DGNDB_CREATOR_EXPORT EXPORT_ATTRIBUTE
#endif

struct DgnDbCreator
    {
    private:
        BentleyG0601::Dgn::BimFromDgnDbTest::DgnDbCreatorImpl *m_creator;

    public:
        DGNDB_CREATOR_EXPORT DgnDbCreator(char const*  inputName);

        DGNDB_CREATOR_EXPORT void ImportSchema(char const*  schemaXml);
        DGNDB_CREATOR_EXPORT void AddElement();

    };
