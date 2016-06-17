using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace IndexECPlugin.Source.Helpers
    {
    internal class TableAliasCreator
        {        
        int m_tabNumber = 0;
        public string GetNewTableAlias ()
            {
            return String.Format("tab{0}", m_tabNumber++);
            }
        }
    }
