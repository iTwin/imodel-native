using System;
using System.Collections.Generic;
using System.Data;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Bentley.EC.Persistence.Query;
using Bentley.ECObjects.Schema;

namespace IndexECPlugin.Source
    {
    public interface ISQLDeleteStatementBuilder
        {

        /// <summary>
        /// Sets the table name (DELETE FROM tableName )
        /// </summary>
        /// <param name="tableName">The table name</param>
        void SetDeleteFromTableName (string tableName);

        /// <summary>
        /// Adds a where clause. 
        /// </summary>
        /// <param name="columnName">The name of the column used in the condition</param>
        /// <param name="op">Relational operator used in the condition</param>
        /// <param name="rightSideString">Right side of the condition encoded as a string</param>
        /// <param name="ecType">The IECType of the data</param>
        void AddWhereClause (string columnName, RelationalOperator op, string rightSideString, IECType ecType);

        /// <summary>
        /// Creates the SQL statement according to the information given.
        /// </summary>
        /// <param name="paramNameValueMap">The map indicating the value and type of each parameter in the parameterized statement returned</param>
        /// <returns>The SQL statement</returns>
        string CreateSqlStatement (out IParamNameValueMap paramNameValueMap);
        }
    }
