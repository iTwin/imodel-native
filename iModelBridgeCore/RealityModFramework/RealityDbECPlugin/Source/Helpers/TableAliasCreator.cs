/*-------------------------------------------------------------------------------------
|
|     $Source: RealityDbECPlugin/Source/Helpers/TableAliasCreator.cs $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+-------------------------------------------------------------------------------------*/

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace IndexECPlugin.Source.Helpers
    {
    /// <summary>
    /// Utilitary class for managing the aliases used in Sql queries commands.
    /// </summary>
    public class TableAliasCreator
        {        
        int m_tabNumber = 0;

        /// <summary>
        /// Gets a new alias for a table.
        /// </summary>
        /// <returns></returns>
        public string GetNewTableAlias ()
            {
            return String.Format("tab{0}", m_tabNumber++);
            }
        }
    }
