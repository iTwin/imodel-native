/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <BimFromDgnDb/BimFromDgnDb.h>

namespace BentleyG0601
    {
    namespace Dgn
        {
        namespace DgnDb0601ToJsonTest
            {
            struct DgnDbCreatorImpl;
            }
        }
    }

#ifdef __BIM_FROM_DGNDB_BUILD__
#define DGNDB_CREATOR_EXPORT EXPORT_ATTRIBUTE
#endif

#if !defined (DGNDB_CREATOR_EXPORT)
#define DGNDB_CREATOR_EXPORT  IMPORT_ATTRIBUTE
#endif

struct DgnDbCreator
    {
    private:
        BentleyG0601::Dgn::DgnDb0601ToJsonTest::DgnDbCreatorImpl *m_creator;

    public:
        DGNDB_CREATOR_EXPORT DgnDbCreator(char const*  inputName);

        DGNDB_CREATOR_EXPORT bool CreateDgnDb();
        DGNDB_CREATOR_EXPORT bool ImportSchema(char const*  schemaXml);
        DGNDB_CREATOR_EXPORT bool AddElement(const char* schemaName, char const* instanceXml);

    };
