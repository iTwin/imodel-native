/*-------------------------------------------------------------------------------------
|
|     $Source: RealityDbECPlugin/Source/Helpers/TableDescriptor.cs $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+-------------------------------------------------------------------------------------*/

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace IndexECPlugin.Source.Helpers
    {
    //
    /// <summary>
    /// A Type to describe a table and the alias given to it in the from/join statement.
    /// </summary>
    public class TableDescriptor
        {
        /// <summary>
        /// TO BE DONE
        /// </summary>
        public string Name
            {
            get;
            set;
            }
        /// <summary>
        /// TO BE DONE
        /// </summary>
        public string Alias
            {
            get;
            set;
            }



        /// <summary>
        /// TO BE DONE
        /// </summary>
        public TableDescriptor FirstTable
            {
            get;
            private set;
            }

        /// <summary>
        /// Name of the column in m_tableJoined that is used as a key to join this table to the query
        /// </summary>
        public string FirstTableKey
            {
            get;
            private set;
            }


        /// <summary>
        /// Name of the column in this table (of name m_name)
        /// </summary>
        public string TableKey
            {
            get;
            private set;
            }

        /// <summary>
        /// Contructor for table descriptor
        /// </summary>
        /// <param name="name"></param>
        /// <param name="alias"></param>
        public TableDescriptor (string name, string alias)
            {
            Name = name;
            Alias = alias;

            FirstTable = null;
            }


        /// <summary>
        /// TO BE DONE
        /// </summary>
        /// <param name="firstTable"></param>
        /// <param name="firstTableKey"></param>
        /// <param name="tableKey"></param>
        public void SetTableJoined (TableDescriptor firstTable, string firstTableKey, string tableKey)
            {
            FirstTable = firstTable;
            FirstTableKey = firstTableKey;
            TableKey = tableKey;
            }

        /// <summary>
        /// Checks if the table descriptor is equivalent to another one. This means that 
        /// all of its properties except its alias are equal and the hierarchy of table joined
        /// is the same.
        /// </summary>
        /// <param name="table"></param>
        /// <returns></returns>
        public bool IsEqualTo (TableDescriptor table)
            {
            if ( table.Name == Name &&
                table.TableKey == TableKey &&
                table.FirstTableKey == FirstTableKey )
                {
                if ( FirstTable == null && table.FirstTable == null )
                    {
                    return true;
                    }
                if ( FirstTable != null && table.FirstTable != null )
                    {
                    return FirstTable.IsEqualTo(table.FirstTable);
                    }
                }
            return false;
            }

        }
    }
