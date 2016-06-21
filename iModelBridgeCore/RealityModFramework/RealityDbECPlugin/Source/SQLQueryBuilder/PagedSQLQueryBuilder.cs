/*-------------------------------------------------------------------------------------
|
|     $Source: RealityDbECPlugin/Source/SQLQueryBuilder/PagedSQLQueryBuilder.cs $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+-------------------------------------------------------------------------------------*/

using Bentley.Exceptions;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using IndexECPlugin.Source.Helpers;

namespace IndexECPlugin.Source
    {
    /// <summary>
    /// SQLQueryBuilder class used to create paged queries.
    /// </summary>
    public class PagedSQLQueryBuilder : SQLQueryBuilder
        {
        private int m_colNumber;
        private int m_lowerBound;
        private int m_upperBound;

        private List<string> m_aliasList;

        /// <summary>
        /// PagedSQLQueryBuilder constructor
        /// </summary>
        /// <param name="lowerBound">Lower bound (included) of the instance rows returned</param>
        /// <param name="upperBound">Upper bound (included) of the instance rows returned</param>
        public PagedSQLQueryBuilder (int lowerBound, int upperBound)
            : base()
            {
            m_colNumber = 0;

            //m_lowerBound = skip + 1;

            //m_upperBound = m_lowerBound + top - 1;

            m_lowerBound = lowerBound;
            m_upperBound = upperBound;

            m_aliasList = new List<string>();
            }

        /// <summary>
        /// Builds the query according to the clauses added
        /// </summary>
        /// <returns>The SQL query string</returns>
        override public string BuildQuery (out DataReadingHelper dataReadingHelper)
            {
            string innerRequestAlias = "Results";

            if ( m_sqlOrderByClause.Count == 0 )
                {
                throw new UserFriendlyException("Please include an OrderBy statement in order for the paging to work.");
                }

            string innerOrderByClause = "ORDER BY ";
            innerOrderByClause += String.Join(", ", m_sqlOrderByClause.ToArray());

            string innerSelectString = "SELECT ROW_NUMBER() OVER ( " + innerOrderByClause + " ) AS RowNum";
            foreach ( string selectedColumn in m_sqlSelectClause )
                {
                string alias = GetNewColAlias();
                m_aliasList.Add(alias);
                innerSelectString += ", " + selectedColumn + " AS " + alias;
                }
            innerSelectString += " ";

            string innerFromStr = "FROM " + m_sqlFromClause.Name + " " + m_sqlFromClause.Alias + " ";

            //string innerLeftJoinClause = String.Join(" ", m_sqlLeftJoinClause.ToArray()) + " ";
            string innerLeftJoinClause = "";
            foreach ( var table in m_sqlLeftJoinClause )
                {
                innerLeftJoinClause += "LEFT JOIN " + AddBrackets(table.Name) + " " + table.Alias
                    + " ON " + AddBrackets(table.FirstTable.Alias) + "." + AddBrackets(table.FirstTableKey) + " = " + AddBrackets(table.Alias) + "." + AddBrackets(table.TableKey) + " ";
                }


            string innerWhereClause = "";
            if ( !String.IsNullOrWhiteSpace(m_sqlWhereClause) )
                {
                innerWhereClause = "WHERE " + m_sqlWhereClause + " ";
                }

            string completeInnerString = innerSelectString + innerFromStr + innerLeftJoinClause + innerWhereClause;

            string outerSelect = "";
            foreach ( string alias in m_aliasList )
                {
                if ( outerSelect == "" )
                    {
                    outerSelect = "SELECT ";
                    }
                else
                    {
                    outerSelect += ", ";
                    }

                outerSelect += innerRequestAlias + "." + alias;
                }
            outerSelect += " ";

            string outerFrom = "FROM ( " + completeInnerString + " ) AS " + innerRequestAlias + " ";

            string outerWhere = "WHERE RowNum >= " + m_lowerBound + " AND RowNum <= " + m_upperBound + " ORDER BY RowNum";

            dataReadingHelper = m_dataReadingHelper;

            return outerSelect + outerFrom + outerWhere;
            }

        //Generates a new unique nolumn
        private string GetNewColAlias ()
            {
            return String.Format("col{0}", m_colNumber++);
            }
        }
    }
