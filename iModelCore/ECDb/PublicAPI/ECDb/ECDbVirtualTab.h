/*---------------------------------------------------------------------------------------------
 * Copyright (c) Bentley Systems, Incorporated. All rights reserved.
 * See LICENSE.md in the repository root for full copyright notice.
 *--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECDb/ECDbTypes.h>
#include <ECDb/ECDb.h>
#include <BeSQLite/VirtualTab.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
//! Allow to implement table-value function in ECSQL
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct ECDbModule : public BeSQLite::DbModule {
   public:
    struct ECDbVirtualTable : public DbVirtualTable {
       public:
        struct ECDbCursor : public DbCursor {
           public:
            explicit ECDbCursor(ECDbVirtualTable& vt) : DbCursor(vt) {}
            virtual ~ECDbCursor() {};
        };

       public:
        explicit ECDbVirtualTable(ECDbModule& module) : DbVirtualTable(module) {}
        virtual ~ECDbVirtualTable() {}
    };

   private:
    Utf8String m_ecSchema;

   protected:
    ECDB_EXPORT DbResult _OnRegister() final;

   public:
    ECDbModule(ECDbR db, Utf8CP name, Utf8CP declaration, Utf8CP ecSchema) : DbModule(db, name, declaration), m_ecSchema(ecSchema) {}
    ECDbR GetECDb() { return reinterpret_cast<ECDbR>(GetDb()); }
    virtual ~ECDbModule() {}
};

END_BENTLEY_SQLITE_EC_NAMESPACE