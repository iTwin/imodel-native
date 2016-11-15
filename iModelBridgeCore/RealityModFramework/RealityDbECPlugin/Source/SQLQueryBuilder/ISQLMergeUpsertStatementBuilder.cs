using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Bentley.ECObjects.Schema;

namespace IndexECPlugin.Source
    {

    /// <summary>
    /// Base interface for merge upsert query builders
    /// </summary>
    public interface ISQLMergeUpsertStatementBuilder
        {

        /// <summary>
        /// Gets the Target table alias. The alias should be used like this: MERGE tableName as alias
        /// </summary>
        string TargetTableAlias{get;}

        /// <summary>
        /// Gets the Source table name. The name in this example is s: (USING (VALUES(...) ) as s(columnName1,columnName2,...)
        /// </summary>
        string SourceTableAlias{get;}

        /// <summary>
        /// Sets the table name (MERGE tableName)
        /// </summary>
        /// <param name="tableName">The table name</param>
        void SetMergeTableName (string tableName);

        /// <summary>
        /// Adds a column (USING (VALUES(...) ) as s(columnName1,columnName2,...). 
        /// Cannot be called after EndSettingColumnNames
        /// </summary>
        /// <param name="columnName">The name of the column in the database</param>
        /// <param name="ecType">The Type of the column</param>
        void AddColumnName (string columnName, IECType ecType);

        /// <summary>
        /// Adds a column of spatial type
        /// </summary>
        /// <param name="columnName">The name of the column in the database</param>
        void AddSpatialColumnName (string columnName);

        /// <summary>
        /// Adds a column of binary type
        /// </summary>
        /// <param name="columnName">The name of the column in the database</param>
        void AddBinaryColumnName (string columnName);

        /// <summary>
        /// Ends the phase of adding columns to the insert. AddColumnName cannot be called after this, 
        /// and AddRow cannot be called before this.
        /// </summary>
        void EndSettingColumnNames ();

        /// <summary>
        /// Set the ON statement (without the ON). Example : (ON) t.IdStr=s.IdStr and t.SubAPI=s.SubAPI
        /// </summary>
        /// <param name="onStatement">The ON statement, without the ON</param>
        void SetOnStatement (string onStatement);

        /// <summary>
        /// Sets the MATCHED conditional statement (without the WHEN MATCHED). Example: (WHEN MATCHED) AND (t.Complete='FALSE')
        /// </summary>
        /// <param name="matchedStatement">The MATCHED conditional statement (without the WHEN MATCHED)</param>
        void SetMatchedStatement (string matchedStatement);

        /// <summary>
        /// Sets the NOT MATCHED conditional statement (without the WHEN NOT MATCHED). Example: (WHEN NOT MATCHED) AND (t.Complete='FALSE')
        /// </summary>
        /// /// <param name="notMatchedStatement">The NOT MATCHED conditional statement (without the WHEN NOT MATCHED)</param>
        void SetNotMatchedStatement (string notMatchedStatement);

        /// <summary>
        /// Adds a row of values. Cannot be called before EndSettingColumnNames. Not all columns specified before are necessary, others will be null.
        /// </summary>
        /// <param name="rowOfValues">A dictionary containing the names of the column as keys and the value the row must contain as the value linked to the key.</param>
        void AddRow (Dictionary<string, object> rowOfValues);

        /// <summary>
        /// Creates the Sql statement according to the columns and rows added. Must be called after EndSettingColumnNames and after having added at least one row.
        /// </summary>
        /// <param name="paramNameValueMap">The map indicating the value and type of each parameter in the parameterized statement returned</param>
        /// <returns>The Sql statement</returns>
        string CreateSqlStatement (out IParamNameValueMap paramNameValueMap);

        }
    }
