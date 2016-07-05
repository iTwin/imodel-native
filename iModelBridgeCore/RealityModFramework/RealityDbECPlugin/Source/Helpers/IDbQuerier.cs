using System;
using System.Collections.Generic;
using System.Data;
using System.Data.SqlClient;
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

    /// <summary>
    /// An interface for communicating with a database. Its primary purpose is for testing, by mocking.
    /// </summary>
    public interface IDbQuerier
        {

        /// <summary>
        /// Query the database and transforms the output into ECInstances.
        /// </summary>
        /// <param name="sqlCommandString">The command string containing the query.</param>
        /// <param name="dataReadingHelper">The DataReadingHelper object, usually returned by the same object that created the command string.</param>
        /// <param name="paramNameValueMap">The IParamNameValueMap object, usually returned by the same object that created the command string.</param>
        /// <param name="ecClass">The class of the ECInstances created.</param>
        /// <param name="propertyList">The list of properties that will be filled in the ECInstances.</param>
        /// <param name="dbConnection">The DbConnection object necessary to communicate with the database.</param>
        /// <param name="nonInstanceDataColumnList">The list of queried columns that are not reflected by ECProperties in the ECInstances. 
        ///   These infos will be put in the extended data of the instances</param>
        /// <param name="createDefaultThumbnail">If the call for thumbnail data, this indicates if the thumbnail returned will be the default image.</param>
        /// <returns>The queried instances</returns>
        List<IECInstance> QueryDbForInstances
            (string sqlCommandString,
            DataReadingHelper dataReadingHelper,
            IParamNameValueMap paramNameValueMap,
            IECClass ecClass, IEnumerable<IECProperty> propertyList,
            IDbConnection dbConnection,
            IEnumerable<string> nonInstanceDataColumnList = null,
            bool createDefaultThumbnail = true);

        /// <summary>
        /// Executes a non-query command to a database
        /// </summary>
        /// <param name="sqlCommandString">The command string.</param>
        /// <param name="paramNameValueMap">The IParamNameValueMap object, usually returned by the same object that created the command string.</param>
        /// <param name="dbConnection">The DbConnection object necessary to communicate with the database.</param>
        /// <returns>The return code of the command.</returns>
        int ExecuteNonQueryInDb
           (string sqlCommandString,
           IParamNameValueMap paramNameValueMap,
           IDbConnection dbConnection
           );
        }

    /// <summary>
    /// A class for communicating with a database.
    /// </summary>
    public class DbQuerier : IDbQuerier
        {

        /// <summary>
        /// Default constructor
        /// </summary>
        public DbQuerier()
            {
            }


        /// <summary>
        /// Query the database and transforms the output into ECInstances.
        /// </summary>
        /// <param name="sqlCommandString">The command string containing the query.</param>
        /// <param name="dataReadingHelper">The DataReadingHelper object, usually returned by the same object that created the command string.</param>
        /// <param name="paramNameValueMap">The IParamNameValueMap object, usually returned by the same object that created the command string.</param>
        /// <param name="ecClass">The class of the ECInstances created.</param>
        /// <param name="propertyList">The list of properties that will be filled in the ECInstances.</param>
        /// <param name="dbConnection">The DbConnection object necessary to communicate with the database.</param>
        /// <param name="nonInstanceDataColumnList">The list of queried columns that are not reflected by ECProperties in the ECInstances. 
        ///   These infos will be put in the extended data of the instances</param>
        /// <param name="createDefaultThumbnail">If the call for thumbnail data, this indicates if the thumbnail returned will be the default image.</param>
        /// <returns></returns>
        public List<IECInstance> QueryDbForInstances
            (string sqlCommandString,
            DataReadingHelper dataReadingHelper,
            IParamNameValueMap paramNameValueMap,
            IECClass ecClass, IEnumerable<IECProperty> propertyList,
            IDbConnection dbConnection,
            IEnumerable<string> nonInstanceDataColumnList = null,
            bool createDefaultThumbnail = true)
            {
            List<IECInstance> instanceList = new List<IECInstance>();

            dbConnection.Open();

            using ( IDbCommand dbCommand = dbConnection.CreateCommand() )
                {

                dbCommand.CommandText = sqlCommandString;
                dbCommand.CommandType = CommandType.Text;
                dbCommand.Connection = dbConnection;

                SetDbCommandParameters(paramNameValueMap, dbCommand);


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

                        if ( nonInstanceDataColumnList != null )
                            {
                            foreach ( string columnName in nonInstanceDataColumnList )
                                {
                                int? nonInstanceDataColumn = dataReadingHelper.getNonPropertyDataColumn(columnName);
                                if ( nonInstanceDataColumn.HasValue )
                                    {
                                    object columnData = reader[nonInstanceDataColumn.Value];
                                    instance.ExtendedDataValueSetter.Add(new KeyValuePair<string, object>(columnName, columnData));
                                    }
                                }
                            }

                        foreach ( IECProperty prop in propertyList )
                            {
                            if ( ecClass.Contains(prop.Name) )
                                {
                                IECPropertyValue instancePropertyValue = instance[prop.Name];
                                IECInstance dbColumn = prop.GetCustomAttributes("DBColumn");

                                //if ( dbColumn == null )
                                //    {
                                //    //This is not a column in the sql table. We skip it...
                                //    continue;
                                //    }

                                int? columnNumber = dataReadingHelper.getInstanceDataColumn(prop);

                                if ( !columnNumber.HasValue )
                                    {
                                    // We did not query this property. We skip it...
                                    continue;
                                    //throw new ProgrammerException("There should have been a column number.");
                                    }

                                IECPropertyValue isSpatial = dbColumn != null ? dbColumn["IsSpatial"] : null;
                                if ( isSpatial == null || isSpatial.IsNull || isSpatial.StringValue.ToLower() == "false" )
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

        /// <summary>
        /// Executes a non-query command to a database
        /// </summary>
        /// <param name="sqlCommandString">The command string.</param>
        /// <param name="paramNameValueMap">The IParamNameValueMap object, usually returned by the same object that created the command string.</param>
        /// <param name="dbConnection">The DbConnection object necessary to communicate with the database.</param>
        /// <returns>The return code of the command.</returns>
        public int ExecuteNonQueryInDb
            (string sqlCommandString,
            IParamNameValueMap paramNameValueMap,
            IDbConnection dbConnection
            )
            {
            dbConnection.Open();

            int result;
            using ( IDbCommand dbCommand = dbConnection.CreateCommand() )
                {

                dbCommand.CommandText = sqlCommandString;
                dbCommand.CommandType = CommandType.Text;
                dbCommand.Connection = dbConnection;

                SetDbCommandParameters(paramNameValueMap, dbCommand);

                result = dbCommand.ExecuteNonQuery();
                }

            dbConnection.Close();
            return result;

            }

        private void SetDbCommandParameters (IParamNameValueMap paramNameValueMap, IDbCommand dbCommand)
            {
            if ( paramNameValueMap is GenericParamNameValueMap )
                {

                GenericParamNameValueMap genericParamNameValueMap = paramNameValueMap as GenericParamNameValueMap;

                foreach ( KeyValuePair<string, Tuple<object, DbType>> paramNameValue in genericParamNameValueMap )
                    {
                    IDbDataParameter param = dbCommand.CreateParameter();
                    param.DbType = paramNameValue.Value.Item2;
                    param.ParameterName = paramNameValue.Key;
                    if ( paramNameValue.Value.Item1 != null )
                        {
                        param.Value = paramNameValue.Value.Item1;
                        }
                    else
                        {
                        param.Value = DBNull.Value;
                        }
                    dbCommand.Parameters.Add(param);
                    }
                }
            else if ( paramNameValueMap is SqlServerParamNameValueMap )
                {
                SqlCommand sqlCommand = dbCommand as SqlCommand;
                if ( sqlCommand == null )
                    {
                    throw new ProgrammerException("SqlServerParamNameValueMap should only be used with SqlCommand");
                    }
                SqlServerParamNameValueMap sqlServerParamNameValueMap = paramNameValueMap as SqlServerParamNameValueMap;

                foreach ( KeyValuePair<string, Tuple<object, SqlDbType>> paramNameValue in sqlServerParamNameValueMap )
                    {
                    SqlParameter param = sqlCommand.CreateParameter();
                    param.SqlDbType = paramNameValue.Value.Item2;
                    if ( param.SqlDbType == SqlDbType.Udt )
                        {
                        param.UdtTypeName = "GEOMETRY";
                        }
                    param.ParameterName = paramNameValue.Key;
                    if ( paramNameValue.Value.Item1 != null )
                        {
                        param.Value = paramNameValue.Value.Item1;
                        }
                    else
                        {
                        param.Value = DBNull.Value;
                        }
                    sqlCommand.Parameters.Add(param);
                    }
                }
            else
                {
                throw new NotImplementedException("This type of IParamNameValueMap is not implemented yet.");
                }
            }
        }
    }
