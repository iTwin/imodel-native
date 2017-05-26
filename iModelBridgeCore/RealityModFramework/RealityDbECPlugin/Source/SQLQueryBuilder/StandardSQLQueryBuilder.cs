﻿/*-------------------------------------------------------------------------------------
|
|     $Source: RealityDbECPlugin/Source/SQLQueryBuilder/StandardSQLQueryBuilder.cs $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+-------------------------------------------------------------------------------------*/

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using IndexECPlugin.Source.Helpers;

namespace IndexECPlugin.Source
    {
    /// <summary>
    /// SQLQueryBuilder class used to create non paged queries.
    /// </summary>
    public class StandardSQLQueryBuilder : SQLQueryBuilder
        {
        /// <summary>
        /// StandardSQLQueryBuilder constructor
        /// </summary>
        public StandardSQLQueryBuilder ()
            : base()
            {

            }

        /// <summary>
        /// Builds the query according to the clauses added
        /// </summary>
        /// <returns>The SQL query string</returns>
        override public string BuildQuery (out DataReadingHelper dataReadingHelper)
            {
            string completeSelectStr = "SELECT TOP 1000 ";
            completeSelectStr += String.Join(", ", m_sqlSelectClause.ToArray());

            completeSelectStr += " ";


            string completeFromStr = "FROM " + m_sqlFromClause.Name + " " + m_sqlFromClause.Alias + " ";

            string completeTableHint = "";

            if(!String.IsNullOrWhiteSpace(m_tableHint))
                {
                completeTableHint = " WITH(INDEX(" + m_tableHint + ")) ";
                }
                

            //string completeLeftJoinClause = String.Join(" ", m_sqlLeftJoinClause.ToArray());
            string completeLeftJoinClause = "";
            foreach ( var table in m_sqlLeftJoinClause )
                {
                completeLeftJoinClause += "LEFT JOIN " + table.Name + " " + table.Alias
                    + " ON " + table.FirstTable.Alias + "." + table.FirstTableKey + " = " + table.Alias + "." + table.TableKey + " ";
                }


            completeLeftJoinClause += " ";

            string completeWhereClause = "";
            if ( !String.IsNullOrWhiteSpace(m_sqlWhereClause) )
                {
                completeWhereClause = "WHERE " + m_sqlWhereClause + " ";
                }

            string completeOrderByClause = "";
            if ( m_sqlOrderByClause.Count != 0 )
                {
                completeOrderByClause = "ORDER BY ";
                completeOrderByClause += String.Join(", ", m_sqlOrderByClause.ToArray());
                }

            dataReadingHelper = m_dataReadingHelper;

            return completeSelectStr + completeFromStr + completeTableHint + completeLeftJoinClause + completeWhereClause + completeOrderByClause + ";";
            }
        }
    }
