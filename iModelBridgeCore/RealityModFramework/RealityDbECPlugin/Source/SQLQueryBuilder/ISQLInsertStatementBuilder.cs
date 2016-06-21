/*-------------------------------------------------------------------------------------
|
|     $Source: RealityDbECPlugin/Source/SQLQueryBuilder/ISQLInsertStatementBuilder.cs $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+-------------------------------------------------------------------------------------*/

using System;
using System.Collections.Generic;
using System.Data;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Bentley.ECObjects.Schema;

namespace IndexECPlugin.Source
    {

    /// <summary>
    /// Base interface for insert query builders
    /// </summary>
    public interface ISQLInsertStatementBuilder
        {

        /// <summary>
        /// Sets the table name (INSERT INTO tableName )
        /// </summary>
        /// <param name="tableName">The table name</param>
        void SetInsertIntoTableName (string tableName);
        

        //If it's not possible to map the IECType to a database type (because one IECType could represent many database types),
        //an AddColumn method could be implemented with this signature :
        //void AddColumn (IECProperty prop);
        //The Properties should also have new custom attributes set to indicate what is the associated database type 
        //(for each type of database, if there are many). For now, the AddColumn methods we have now are enough...

        ///// <summary>
        ///// Adds a delete statement before each insert : DELETE FROM tableName WHERE whereColumnName=value. 
        ///// This should be used to prevent conflit with pre-existing values, and if the information deleted is not important.
        ///// Typically, you should use the Id column as whereColumnName to prevent unwanted deletion of rows.
        ///// </summary>
        ///// <param name="whereColumnName">The name of the column on which to execute the test</param>
        //void AddDeleteStatement (string whereColumnName);

        /// <summary>
        /// Calling this method will enable the user to include a delete statement before each insert
        /// </summary>
        void ActivateDeleteBeforeInsert ();

        /// <summary>
        /// Adds a column (INSERT INTO tableName (columnName1, columnName2, ...)). 
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

        ///// <summary>
        ///// Adds multiple column names at the same time
        ///// </summary>
        ///// <param name="columnNames"></param>
        //void AddColumnNames (IEnumerable<string> columnNames);

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

        ///// <summary>
        ///// Adds a row of values. Cannot be called before EndSettingColumnNames. Not all columns specified before are necessary, others will be null.
        ///// </summary>
        ///// <param name="rowOfValues">A dictionary containing the names of the column as keys and the value the row must contain as the value linked to the key.</param>
        //void AddRow (Dictionary<string, object> rowOfValues);

        /// <summary>
        /// Adds a row of values. Cannot be called before EndSettingColumnNames. Not all columns specified before are necessary, others will be null.
        /// </summary>
        /// <param name="rowOfValues">A dictionary containing the names of the column as keys and the value the row must contain as the value linked to the key.</param>
        /// <param name="deleteStatementManager">The delete statement to include before the insert. 
        /// Can be null if delete is not activated.</param>
        void AddRow (Dictionary<string, object> rowOfValues, WhereStatementManager deleteStatementManager);


        /// <summary>
        /// Creates the Sql statement according to the columns and rows added. Must be called after EndSettingColumnNames and after having added at least one row.
        /// </summary>
        /// <param name="paramNameValueMap">The map indicating the value and type of each parameter in the parameterized statement returned</param>
        /// <returns>The Sql statement</returns>
        string CreateSqlStatement (out IParamNameValueMap paramNameValueMap);


        }

    /// <summary>
    /// This class enables an ISQLInsertStatementBuilder to build a where statement put its parameters in the paramNameValueMap.
    /// The WhereStatement must begin after the WHERE (ex : DELETE FROM tableName WHERE [WhereStatement]). The parameters to include in the paramNameValueMap must be added using
    /// AddParameter. The name of each parameter should be unique and should be a unique string of character in the WhereStatement, 
    /// as any occurence of it in the WhereStatement will be replaced by a unique name in the paramNameValueMap.
    /// </summary>
    public class WhereStatementManager
        {

        /// <summary>
        /// The where statement. Must begin after the WHERE in the where statement (DELETE FROM tableName WHERE [WhereStatement]).
        /// Should resemble the following string : IdStr = @instId@ AND Complete = @isComplete@;
        /// @instId@, @isComplete@ are, in this example, parameter names. These should be added, along with their value and type, using AddParameter
        /// </summary>
        public string WhereStatement
            {
            get;
            set;
            }

        /// <summary>
        /// The list of parameters names
        /// </summary>
        public List<string> ParameterNames
            {
            get;
            private set;
            }

        /// <summary>
        /// The list of parameter types
        /// </summary>
        public List<IECType> ParameterTypes
            {
            get;
            private set;
            }

        /// <summary>
        /// The list of parameter values
        /// </summary>

        public List<object> ParameterValues
            {
            get;
            private set;
            }

        /// <summary>
        /// WhereStatementManager constructor
        /// </summary>
        public WhereStatementManager ()
            {
            WhereStatement = null;
            ParameterNames = new List<string>();
            ParameterTypes = new List<IECType>();
            ParameterValues = new List<object>();
            }

        /// <summary>
        /// Specifies a parameter in the WhereStatement that should be included in the paramNameValueMap.
        /// </summary>
        /// <param name="parameterName">The parameter name as written in the WhereStatement</param>
        /// <param name="parameterType">The type of the parameter</param>
        /// <param name="parameterValue">The value of the parameter</param>
        public void AddParameter (string parameterName, IECType parameterType, object parameterValue)
            {
            ParameterNames.Add(parameterName);
            ParameterTypes.Add(parameterType);
            ParameterValues.Add(parameterValue);
            }

        }
        
    }
