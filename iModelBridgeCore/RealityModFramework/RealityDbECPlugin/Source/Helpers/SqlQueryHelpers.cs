using System;
using System.Collections.Generic;
using System.Data;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;
using Bentley.EC.Persistence;
using Bentley.ECObjects.Instance;
using Bentley.ECObjects.Schema;
using Bentley.Exceptions;

namespace IndexECPlugin.Source.Helpers
    {
    static internal class SqlQueryHelpers
        {
        /// <summary>
        /// This method's purpose is to join recursively all tables in the hierarchy of a class, up to a specified base class
        /// </summary>
        /// <param name="queriedClass">The derived class of which we need the base class table joined</param>
        /// <param name="tempTable1">The table descriptor of the queried class</param>
        /// <param name="finalBaseClass">The specified final base class to have its table joined</param>
        /// <param name="sqlQueryBuilder">The SQLQueryBuilder used to create the query</param>
        /// <param name="tac">The table alias creator used with the query</param>
        /// <param name="modifyTablePropertyName">Indicates the name of the property used to indicate which table to use. 
        /// <param name="uniqueIdColumn">Indicates the name of the property containing the uniqueId Column used link the derived and base tables.</param>
        ///   The property is located in SQLEntity custom attribute. The default property used is FromTableName</param>
        /// <returns>The final class' table descriptor</returns>
        static public TableDescriptor JoinBaseTables 
            (IECClass queriedClass, 
             TableDescriptor tempTable1, 
             IECClass finalBaseClass, 
             SQLQueryBuilder sqlQueryBuilder, 
             TableAliasCreator tac, 
             string modifyTablePropertyName = null,
             string uniqueIdColumn = null)
            {
            if ( queriedClass != finalBaseClass )
                {
                if ( queriedClass.BaseClasses.Count() != 1 )
                    {
                    throw new ProgrammerException("IndexECPlugin only supports classes that have at most one base class.");
                    }

                IECClass baseClass = queriedClass.BaseClasses[0];
                if ( queriedClass.GetCustomAttributes("SQLEntity").GetPropertyValue("FromTableName").StringValue !=
                    baseClass.GetCustomAttributes("SQLEntity").GetPropertyValue("FromTableName").StringValue )
                    {

                    string baseClassKeyColumn;
                    string derivedClassKeyColumn;

                    if ( uniqueIdColumn != null)
                        {
                        var baseClassUniqueIdColumn = baseClass.GetCustomAttributes("SQLEntity").GetPropertyValue(uniqueIdColumn);
                        var derivedClassUniqueIdColumn = queriedClass.GetCustomAttributes("SQLEntity").GetPropertyValue(uniqueIdColumn);
                        if(baseClassUniqueIdColumn == null || derivedClassUniqueIdColumn == null)
                            {
                            throw new ProgrammerException(String.Format("The schema does not contain {2} properties for the {0} and/or {1} class(es)", baseClass.Name, queriedClass.Name, uniqueIdColumn));
                            }
                        baseClassKeyColumn = baseClassUniqueIdColumn.StringValue;
                        derivedClassKeyColumn = derivedClassUniqueIdColumn.StringValue;
                        }
                    else if ( queriedClass.GetCustomAttributes("DerivedClassLinker") != null )
                        {
                        //The DerivedClassLinker was never used. We could remove it.
                        string baseClassKeyPropertyName = queriedClass.GetCustomAttributes("DerivedClassLinker").GetPropertyValue("BaseClassKeyProperty").StringValue;
                        string derivedClassKeyPropertyName = queriedClass.GetCustomAttributes("DerivedClassLinker").GetPropertyValue("DerivedClassKeyProperty").StringValue;

                        baseClassKeyColumn = baseClass[baseClassKeyPropertyName].GetCustomAttributes("DBColumn")["ColumnName"].StringValue;
                        derivedClassKeyColumn = queriedClass[derivedClassKeyPropertyName].GetCustomAttributes("DBColumn")["ColumnName"].StringValue;
                        }

                    else
                        {
                        string baseClassKeyPropertyName = baseClass.GetCustomAttributes("SQLEntity").GetPropertyValue("InstanceIDProperty").StringValue;
                        string derivedClassKeyPropertyName = queriedClass.GetCustomAttributes("SQLEntity").GetPropertyValue("InstanceIDProperty").StringValue;

                        baseClassKeyColumn = baseClass[baseClassKeyPropertyName].GetCustomAttributes("DBColumn")["ColumnName"].StringValue;
                        derivedClassKeyColumn = queriedClass[derivedClassKeyPropertyName].GetCustomAttributes("DBColumn")["ColumnName"].StringValue;
                        }

                    //We have to join the two different tables
                    TableDescriptor newTable;
                    if ( modifyTablePropertyName == null )
                        {
                        newTable = new TableDescriptor(baseClass.GetCustomAttributes("SQLEntity").GetPropertyValue("FromTableName").StringValue, tac.GetNewTableAlias());
                        }
                    else
                        {
                        newTable = new TableDescriptor(baseClass.GetCustomAttributes("SQLEntity").GetPropertyValue(modifyTablePropertyName).StringValue, tac.GetNewTableAlias());
                        }

                    newTable.SetTableJoined(tempTable1, derivedClassKeyColumn, baseClassKeyColumn);

                    TableDescriptor similarTable;
                    bool joinSuccessful = sqlQueryBuilder.AddLeftJoinClause(newTable, out similarTable);
                    if ( !joinSuccessful )
                        {
                        //tempTable1 = similarTable;
                        return JoinBaseTables(baseClass, similarTable, finalBaseClass, sqlQueryBuilder, tac, modifyTablePropertyName);
                        }
                    else
                        {
                        //tempTable1 = newTable;
                        return JoinBaseTables(baseClass, newTable, finalBaseClass, sqlQueryBuilder, tac, modifyTablePropertyName);
                        }
                    }

                return JoinBaseTables(baseClass, tempTable1, finalBaseClass, sqlQueryBuilder, tac, modifyTablePropertyName);
                }

            return tempTable1;

            }

        static public List<IECInstance> QueryDbForInstances 
            (string sqlCommandString, 
            DataReadingHelper dataReadingHelper, 
            Dictionary<string, Tuple<string, DbType>> paramNameValueMap, 
            IECClass ecClass, IEnumerable<IECProperty> propertyList, 
            IDbConnection dbConnection, 
            bool createDefaultThumbnail = true)
            {
            List<IECInstance> instanceList = new List<IECInstance>();

            dbConnection.Open();

            using ( IDbCommand dbCommand = dbConnection.CreateCommand() )
                {

                dbCommand.CommandText = sqlCommandString;
                dbCommand.CommandType = CommandType.Text;
                dbCommand.Connection = dbConnection;

                foreach ( KeyValuePair<string, Tuple<string, DbType>> paramNameValue in paramNameValueMap )
                    {
                    IDbDataParameter param = dbCommand.CreateParameter();
                    param.DbType = paramNameValue.Value.Item2;
                    param.ParameterName = paramNameValue.Key;
                    param.Value = paramNameValue.Value.Item1;
                    dbCommand.Parameters.Add(param);
                    }

                using ( IDataReader reader = dbCommand.ExecuteReader() )
                    {

                    while ( reader.Read() )
                        {
                        IECInstance instance = ecClass.CreateInstance();
                        int? streamDataColumn = dataReadingHelper.getStreamDataColumn();
                        if ( streamDataColumn.HasValue )
                            {
                            Byte[] byteArray = reader[streamDataColumn.Value] as byte[];

                            MemoryStream mStream;

                            if ( byteArray != null )
                                {
                                mStream = new MemoryStream(byteArray);
                                StreamBackedDescriptor desc = new StreamBackedDescriptor(mStream, "Thumbnail", mStream.Length, DateTime.UtcNow);
                                StreamBackedDescriptorAccessor.SetIn(instance, desc);
                                }
                            else
                                {
                                if ( createDefaultThumbnail )
                                    {
                                    mStream = new MemoryStream();
                                    Assembly.GetExecutingAssembly().GetManifestResourceStream("NoImage.jpg").CopyTo(mStream);
                                    StreamBackedDescriptor desc = new StreamBackedDescriptor(mStream, "Thumbnail", mStream.Length, DateTime.UtcNow);
                                    StreamBackedDescriptorAccessor.SetIn(instance, desc);
                                    }
                                }

                            }

                        int? relatedInstanceIdColumn = dataReadingHelper.getRelatedInstanceIdColumn();

                        if ( relatedInstanceIdColumn.HasValue )
                            {
                            string relatedInstanceId = reader[relatedInstanceIdColumn.Value] as string;
                            instance.ExtendedDataValueSetter.Add(new KeyValuePair<string, object>("relInstID", relatedInstanceId));
                            }

                        foreach ( IECProperty prop in propertyList )
                            {
                            if ( ecClass.Contains(prop.Name) )
                                {
                                IECPropertyValue instancePropertyValue = instance[prop.Name];
                                IECInstance dbColumn = prop.GetCustomAttributes("DBColumn");

                                if ( dbColumn == null )
                                    {
                                    //This is not a column in the sql table. We skip it...
                                    continue;
                                    }

                                int? columnNumber = dataReadingHelper.getInstanceDataColumn(prop);

                                if ( !columnNumber.HasValue )
                                    {
                                    throw new ProgrammerException("There should have been a column number.");
                                    }

                                var isSpatial = dbColumn["IsSpatial"];
                                if ( isSpatial.IsNull || isSpatial.StringValue.ToLower() == "false" )
                                    {

                                    if ( !reader.IsDBNull(columnNumber.Value) )
                                        {
                                        ECToSQLMap.SQLReaderToECProperty(instancePropertyValue, reader, columnNumber.Value);
                                        }
                                    }
                                else
                                    {
                                    if ( !ECTypeHelper.IsString(instancePropertyValue.Type) )
                                        {
                                        throw new ProgrammerException(String.Format("The property {0} tagged as spatial must be declared as a string in the ECSchema.", prop.Name));
                                        }
                                    else
                                        {
                                        if ( !reader.IsDBNull(columnNumber.Value) && !reader.IsDBNull(columnNumber.Value + 1) )
                                            {
                                            string WKTString = reader.GetString(columnNumber.Value);

                                            //DbGeometry geom = DbGeometry.FromText(WKTString);

                                            //int SRID = reader.GetInt32(i);

                                            //instancePropertyValue.StringValue = "{ \"points\" : " + DbGeometryHelpers.ExtractPointsLongLat(geom) + ", \"coordinate_system\" : \"" + reader.GetInt32(i) + "\" }";
                                            instancePropertyValue.StringValue = "{ \"points\" : " + DbGeometryHelpers.ExtractOuterShellPointsFromWKTPolygon(WKTString) + ", \"coordinate_system\" : \"" + reader.GetInt32(columnNumber.Value + 1) + "\" }";
                                            }
                                        }
                                    }
                                }
                            }


                        string IDProperty = ecClass.GetCustomAttributes("SQLEntity").GetPropertyValue("InstanceIDProperty").StringValue;
                        instance.InstanceId = instance.GetInstanceStringValue(IDProperty, "");

                        instanceList.Add(instance);

                        }
                    }
                }
            dbConnection.Close();
            return instanceList;
            }

        }
    }
