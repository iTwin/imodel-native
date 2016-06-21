/*-------------------------------------------------------------------------------------
|
|     $Source: RealityDbECPlugin/Source/Helpers/MimicTableWriter.cs $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+-------------------------------------------------------------------------------------*/

using System;
using System.Collections.Generic;
using System.Data;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Bentley.EC.Persistence;
using Bentley.ECObjects.Instance;
using Bentley.ECObjects.Schema;
using Bentley.Exceptions;

namespace IndexECPlugin.Source.Helpers
    {
    /// <summary>
    /// Base class for writing in the database mimic tables
    /// </summary>
    public class MimicTableWriter : MimicTableAccessor
        {
        

        private bool m_setStream;

        /// <summary>
        /// Constructor for instance mimic writer. Sets the names of the properties of custom attributes used to query the database.
        /// </summary>
        /// <param name="canMimicStreamData">Sets the CanModifyStreamData property.</param>
        /// <param name="mimicTableNameProp">Sets the ModifierTableNameProp</param>
        /// <param name="mimicColumnNameProp">Sets the ModifierColumnNameProp</param>
        /// <param name="mimicJoinTableNameProp">Sets the ModifierJoinTableNameProp</param>
        /// <param name="mimicUniqueIdColumn">Sets the ModifierUniqueIdColumn</param>
        public MimicTableWriter (bool canMimicStreamData, string mimicTableNameProp, string mimicColumnNameProp, string mimicJoinTableNameProp, string mimicUniqueIdColumn)
            : base(canMimicStreamData, mimicTableNameProp, mimicColumnNameProp, mimicJoinTableNameProp, mimicUniqueIdColumn)
            {

            }

        /// <summary>
        /// This method builds the sql query to insert data in the mimic table. This could be rewritten as a SQL insert query builder class method.
        /// All instances must be of the same ECClass.
        /// </summary>
        /// <param name="instanceList">The instances to insert.</param>
        /// <param name="ecClass">The ECClass of the instances</param>
        /// <param name="sqlInsertQueryBuilder">The appropriate ISQLInsertQueryBuilder object to create the query</param>
        /// <param name="paramNameValueMap">The map containing each parameter's name and its corresponding value and DbType</param>
        /// <param name="additionalColumns">Represent additional columns to add in the insert and that are not part of the class. 
        /// Each tuple contains the name of the column, the type and the delegate function to obtain the value</param>
        /// <param name="deleteStatementConstructor">The delegate used to create a delete statement before the insert. 
        /// Used to clear old data before inserting the new data</param>>
        /// <returns>The query string</returns>
        public string CreateMimicSQLInsert (IEnumerable<IECInstance> instanceList, 
                                               IECClass ecClass, 
                                               ISQLInsertStatementBuilder sqlInsertQueryBuilder,
                                               out IParamNameValueMap paramNameValueMap, 
                                               IEnumerable<Tuple<string, IECType, Func<IECInstance, string>>> additionalColumns = null,
                                               Func<IECInstance, WhereStatementManager> deleteStatementConstructor = null)
            {
            m_setStream = false;

            IECInstance sqlEntity = ecClass.GetCustomAttributes("SQLEntity");
            sqlInsertQueryBuilder.SetInsertIntoTableName(sqlEntity.GetPropertyValue(MimicTableNameProp).StringValue);

            if(deleteStatementConstructor != null)
                {
                sqlInsertQueryBuilder.ActivateDeleteBeforeInsert();
                }

            IEnumerable<IECProperty> propList = ecClass.Properties(true);

            foreach(IECProperty prop in propList)
                {

                IECInstance mimicDbColumn = prop.GetCustomAttributes(MimicDbColumnCustomClassName);
                if(mimicDbColumn == null)
                    {
                    //This property doesn't have any column in the mimic table
                    continue;
                    }
                IECPropertyValue columnNamePropVal = mimicDbColumn.GetPropertyValue(MimicColumnNameProp);
                if(columnNamePropVal == null)
                    {
                    //This property doesn't have any column in the mimic table
                    continue;
                    }

                string columnName = columnNamePropVal.StringValue;

                if ((prop.GetCustomAttributes("DBColumn") != null) && 
                    (prop.GetCustomAttributes("DBColumn")["IsSpatial"] != null) &&
                     (!prop.GetCustomAttributes("DBColumn")["IsSpatial"].IsNull) &&
                     (prop.GetCustomAttributes("DBColumn")["IsSpatial"].StringValue.ToLower() == "true") )
                    {
                    sqlInsertQueryBuilder.AddSpatialColumnName(columnName);
                    }
                else
                    {
                    sqlInsertQueryBuilder.AddColumnName(columnName, prop.Type);
                    }
                }
            if ( additionalColumns != null )
                {
                foreach ( var additionalColumn in additionalColumns )
                    {
                    sqlInsertQueryBuilder.AddColumnName(additionalColumn.Item1, additionalColumn.Item2);
                    }
                }
            if ( CanMimicStreamData )
                {
                IECInstance fileHolderAttribute = ecClass.GetCustomAttributes("FileHolder");
                if ( (fileHolderAttribute != null) )
                    {
                    m_setStream = true;
                    sqlInsertQueryBuilder.AddBinaryColumnName(fileHolderAttribute["LocationHoldingColumn"].StringValue );
                    }
                }
            sqlInsertQueryBuilder.EndSettingColumnNames();
            foreach ( IECInstance instance in instanceList )
                {
                if ( instance.ClassDefinition.Name != ecClass.Name && !instance.ClassDefinition.BaseClasses.Any(c => c.Name == ecClass.Name) )
                    {
                    throw new ProgrammerException("An instance in the list is not of the specified class.");
                    }

                Dictionary<string, object> row = new Dictionary<string, object>();

                WhereStatementManager deleteStatementManager = null;
                if ( deleteStatementConstructor != null)
                    {
                    deleteStatementManager = deleteStatementConstructor(instance);
                    }

                foreach ( IECProperty prop in propList )
                    {
                    IECInstance mimicDbColumn = prop.GetCustomAttributes(MimicDbColumnCustomClassName);
                    if ( mimicDbColumn == null )
                        {
                        //This property doesn't have any column in the mimic table
                        continue;
                        }
                    IECPropertyValue columnNamePropVal = mimicDbColumn.GetPropertyValue(MimicColumnNameProp);
                    if ( columnNamePropVal == null )
                        {
                        //This property doesn't have any column in the mimic table
                        continue;
                        }
                    string columnName = columnNamePropVal.StringValue;
                    IECPropertyValue propValue = instance.GetPropertyValue(prop.Name);
                    if(propValue == null || propValue.IsNull)
                        {
                        row.Add(columnName, null);
                        }
                    else
                        {
                        row.Add(columnName, propValue.StringValue);
                        }
                    }
                if ( additionalColumns != null )
                    {
                    foreach ( Tuple<string, IECType, Func<IECInstance, string>> additionalColumn in additionalColumns )
                        {
                        row.Add(additionalColumn.Item1, additionalColumn.Item3(instance));
                        }
                    }
                if ( m_setStream )
                    {
                    StreamBackedDescriptor streamBackedDescriptor;
                    IECInstance fileHolderAttribute = ecClass.GetCustomAttributes("FileHolder");
                    String streamColumnName = fileHolderAttribute["LocationHoldingColumn"].StringValue;
                    if ( StreamBackedDescriptorAccessor.TryGetFrom(instance, out streamBackedDescriptor) )
                        {
                        using (MemoryStream mStream = new MemoryStream())
                            {
                            if ( streamBackedDescriptor.Stream.CanSeek )
                                {
                                streamBackedDescriptor.Stream.Position = 0;
                                }
                            streamBackedDescriptor.Stream.CopyTo(mStream);
                            row.Add(streamColumnName, mStream.ToArray());
                            }
                        }
                    else
                        {
                        row.Add(streamColumnName, null);
                        }
                    }
                sqlInsertQueryBuilder.AddRow(row, deleteStatementManager);
                }
            return sqlInsertQueryBuilder.CreateSqlStatement(out paramNameValueMap);
            }
        }
    }
