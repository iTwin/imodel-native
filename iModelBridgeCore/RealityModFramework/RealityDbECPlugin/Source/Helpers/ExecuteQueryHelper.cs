using System;
using System.Collections.Generic;
using System.Data.SqlClient;
using System.Linq;
using System.Text;
using System.Threading;
using Bentley.EC.Persistence;
using Bentley.EC.Persistence.Query;
using Bentley.EC.PluginBuilder.Modules;
using Bentley.ECObjects.Instance;
using Bentley.ECObjects.Schema;
using Bentley.ECSystem.Repository;
using Bentley.Exceptions;
using IndexECPlugin.Source.QueryProviders;

namespace IndexECPlugin.Source.Helpers
    {
    /// <summary>
    /// Helper class for querying data from external services.
    /// </summary>
    public static class ExecuteQueryHelper
        {
        //public static IEnumerable<IECInstance> ExecuteQuery(QueryModule sender,
        //                                                    RepositoryConnection connection,
        //                                                    ECQuery query,
        //                                                    ECQuerySettings querySettings,
        //                                                    string connectionString,
        //                                                    IDbConnectionCreator dbConnCreator )
        //    {
        //    try
        //        {
        //        string fullSchemaName = sender.ParentECPlugin.SchemaModule.GetSchemaFullNames(connection).First();
        //        IECSchema schema = sender.ParentECPlugin.SchemaModule.GetSchema(connection, fullSchemaName);
        //        SearchClass searchClass = query.SearchClasses.First();
        //            {
        //            Log.Logger.info("Executing query " + query.ID + " : " + query.ToECSqlString(0) + ", custom parameters : " + String.Join(",", query.ExtendedData.Select(x => x.ToString())));

        //            if ( (querySettings != null) && ((querySettings.LoadModifiers & LoadModifiers.IncludeStreamDescriptor) != LoadModifiers.None) )
        //            {
        //                switch ( searchClass.Class.Name )
        //                    {
        //                    case "PreparedPackage":
        //                        IECInstance packageInstance = searchClass.Class.CreateInstance();
        //                        ECInstanceIdExpression exp = query.WhereClause[0] as ECInstanceIdExpression;
        //                        packageInstance.InstanceId = exp.RightSideString;
        //                        RetrievalController.RetrievePackage(packageInstance, connectionString, dbConnCreator);
        //                        return new List<IECInstance> { packageInstance };

        //                    case "DownloadReport":

        //                        IECInstance DownloadReportInstance = searchClass.Class.CreateInstance();
        //                        ECInstanceIdExpression exp2 = query.WhereClause[0] as ECInstanceIdExpression;
        //                        DownloadReportInstance.InstanceId = exp2.RightSideString;
        //                        RetrievalController.RetrieveDownloadReport(DownloadReportInstance, connectionString, dbConnCreator);
        //                        return new List<IECInstance> { DownloadReportInstance };
        //                    default:
        //                        //We continue
        //                        break;
        //                    }

        //                }

        //            if(searchClass.Class.Name == "PackageStats")
        //                {
        //                if(!IndexECPlugin.IsBentleyEmployee(connection))
        //                    {
        //                    throw new UserFriendlyException("This operation is not permitted");
        //                    }
        //                SqlDbConnectionCreator sqlDbConnectionCreator = new SqlDbConnectionCreator();
        //                return Packager.ExtractStats(query, connectionString, schema, dbConnCreator);
        //                }



        //            if ( searchClass.Class.GetCustomAttributes("QueryType") == null || searchClass.Class.GetCustomAttributes("QueryType")["QueryType"].IsNull )
        //                {
        //                throw new UserFriendlyException(String.Format("The class {0} cannot be queried.", searchClass.Class.Name));
        //                }

        //            string[] sources;

        //            if ( query.ExtendedData.ContainsKey("source") )
        //                {
        //                sources = query.ExtendedData["source"].ToString().Split('&');
        //                }
        //            else
        //                {
        //                sources = new string[] { searchClass.Class.GetCustomAttributes("QueryType")["QueryType"].StringValue };
        //                }

        //            List<DataSource> sourcesList = new List<DataSource>();
        //            foreach ( string source in sources )
        //                {
        //                try
        //                    {
        //                    sourcesList.Add(SourceStringMap.StringToSource(source));
        //                    }
        //                catch ( NotImplementedException )
        //                    {
        //                    throw new UserFriendlyException("This source does not exist. Choose between " + SourceStringMap.GetAllSourceStrings());
        //                    }
        //                }
        //            List<Tuple<DataSource, IECQueryProvider>> queryProviderList = new List<Tuple<DataSource, IECQueryProvider>>();
        //            if ( sourcesList.Contains(DataSource.Index) || sourcesList.Contains(DataSource.All) )
        //                {
        //                queryProviderList.Add(new Tuple<DataSource, IECQueryProvider>(DataSource.Index, new SqlQueryProvider(query, querySettings, new DbQuerier(connectionString, dbConnCreator), schema)));
        //                }
        //            if ( sourcesList.Contains(DataSource.USGS) || sourcesList.Contains(DataSource.All) )
        //                {
        //                queryProviderList.Add(new Tuple<DataSource, IECQueryProvider>(DataSource.USGS, new UsgsSubAPIQueryProvider(query, querySettings, new DbQuerier(connectionString, dbConnCreator), schema)));
        //                }
        //            ////TODO: Reenable RDS when using All. This requires the operator "IN" to be usable in RDS (ideally) and some changes to the sub-index.
        //            if ( sourcesList.Contains(DataSource.RDS) /* || sources.Contains(DataSource.All)*/ )
        //                {
        //                queryProviderList.Add(new Tuple<DataSource, IECQueryProvider>(DataSource.RDS, new RdsAPIQueryProvider(query, querySettings, new DbQuerier(connectionString, dbConnCreator), schema, Convert.ToBase64String(Encoding.UTF8.GetBytes(GetToken(connection))))));
        //                }
        //            return QueryMultipleSources(queryProviderList);
        //            }
        //        }
        //    catch ( Exception e )
        //        {
        //        if ( e is System.Data.Common.DbException )
        //            {
        //            if ( e is SqlException )
        //                {
        //                var sqlEx = e as SqlException;
        //                Log.Logger.error(String.Format("Query {0} aborted. SqlException number : {3}. SqlException message : {1}. Stack Trace : {2}", query.ID, sqlEx.Message, sqlEx.StackTrace, sqlEx.Number));
        //                if ( sqlEx.Number == 10928 || sqlEx.Number == 10929 )
        //                    throw new EnvironmentalException(String.Format("The server is currently busy. Please try again later"));
        //                }
        //            else
        //                {
        //                Log.Logger.error(String.Format("Query {0} aborted. DbException message : {1}. Stack Trace : {2}", query.ID, e.Message, e.StackTrace));
        //                }
        //            Exception innerEx = e.InnerException;
        //            while ( innerEx != null )
        //                {
        //                Log.Logger.error(String.Format("Inner error message : {0}. Stack Trace : {1}", e.Message, e.StackTrace));
        //                innerEx = innerEx.InnerException;
        //                }
        //            throw new EnvironmentalException("The server has encountered a problem while processing your request. Please verify the syntax of your request. If the problem persists, the server may be down");
        //            }
        //        Log.Logger.error(String.Format("Query {0} aborted. Error message : {1}. Stack trace : {2}", query.ID, e.Message, e.StackTrace));
        //        if ( e is UserFriendlyException )
        //            {
        //            throw;
        //            }
        //        if ( e is EnvironmentalException )
        //            {
        //            throw;
        //            }
        //        else
        //            {
        //            throw new Exception("Internal Error.");
        //            }
        //        }
        //    }

        /// <summary>
        /// Queries ecinstances from multiple sources in a multithreaded manner.
        /// </summary>
        /// <param name="queryProviderList"></param>
        /// <returns></returns>
        public static IEnumerable<IECInstance> QueryMultipleSources (IEnumerable<Tuple<DataSource, IECQueryProvider>> queryProviderList)
            {
            List<IECInstance> instanceList = new List<IECInstance>();

            List<Exception> exceptions = new List<Exception>();
            Object exceptionsLock = new Object();
            Object resultsLock = new Object();
            List<Thread> threadList = new List<Thread>();
            foreach ( var queryProvider in queryProviderList )
                {

                threadList.Add(new Thread(() =>
                {
                    try
                        {
                        //InstanceOverrider instanceOverrider = new InstanceOverrider(new DbQuerier());
                        //InstanceComplement instanceComplement = new InstanceComplement(new DbQuerier());
                        var queryInstances = queryProvider.Item2.CreateInstanceList().ToList();
                        //instanceOverrider.Modify(indexInstances, DataSource.Index, ConnectionString);
                        //instanceComplement.Modify(indexInstances, DataSource.Index, ConnectionString);
                        lock ( resultsLock )
                            {
                            instanceList.AddRange(queryInstances);
                            }
                        }
                    catch ( Exception e )
                        {
                        lock ( exceptionsLock )
                            {
                            exceptions.Add(e);
                            Log.Logger.error(String.Format(queryProvider.Item1.ToString() + " query aborted. Error message : {0}. Stack trace : {1}", e.Message, e.StackTrace));
                            if ( e is AggregateException )
                                {
                                AggregateException ae = (AggregateException) e;
                                foreach ( Exception ie in ae.InnerExceptions )
                                    {
                                    Log.Logger.error(queryProvider.Item1.ToString() + " Aggregate exception message: " + ie.GetBaseException().Message);
                                    }

                                }
                            }
                        }
                }));
                }

            foreach(var thread in threadList)
                {
                thread.Start();
                }

            foreach(var thread in threadList)
                {
                thread.Join();
                }

            if ( exceptions.Count != 0 )
                {
                if ( exceptions.Any(e => e is UserFriendlyException) )
                    {
                    //We throw this exception, since it is caused by the user's wrong input
                    throw exceptions.First(e => e is UserFriendlyException);
                    }
                if ( instanceList.Count == 0 )
                    {
                    //We did not return any instance and there were errors. 
                    //In this case, we throw the first one (DbExceptions in priority). 
                    //
                    if ( exceptions.Any(e => e is System.Data.Common.DbException) )
                        {
                        throw exceptions.First(e => e is System.Data.Common.DbException);
                        }
                    throw exceptions.First();
                    }
                }

            return instanceList;
            }
        }
    }
