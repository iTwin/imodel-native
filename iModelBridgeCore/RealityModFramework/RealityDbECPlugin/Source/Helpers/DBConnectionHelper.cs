/*-------------------------------------------------------------------------------------
|
|     $Source: RealityDbECPlugin/Source/Helpers/DBConnectionHelper.cs $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+-------------------------------------------------------------------------------------*/

using System;
using System.Collections.Generic;
using System.Data;
using System.Data.Common;
using System.Data.SqlClient;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace IndexECPlugin.Source.Helpers
    {
    internal static class DbConnectionHelper
        {
        public static ISQLInsertStatementBuilder GetSqlInsertStatementBuilder(IDbConnection dbConnection)
            {
            if(dbConnection is SqlConnection)
                {
                return new SQLServerInsertStatementBuilder();
                }
            else
                {
                throw new NotImplementedException("This type of connection doesn't have an appropriate implementation of ISQLInsertStatementBuilder yet.");
                }
            }
        }
    }
