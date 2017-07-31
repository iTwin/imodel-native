/*-------------------------------------------------------------------------------------
|
|     $Source: RealityDbECPlugin/Source/QueryProviders/IECQueryProvider.cs $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+-------------------------------------------------------------------------------------*/

using Bentley.ECObjects.Instance;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace IndexECPlugin.Source.QueryProviders
    {
    /// <summary>
    /// This interface serves as canvas for queries from different origins
    /// </summary>
    public interface IECQueryProvider
        {
        /// <summary>
        /// Queries a datasource and returns ecinstances created from the data.
        /// </summary>
        /// <returns></returns>
        IEnumerable<IECInstance> CreateInstanceList ();

        //We should add something to prepare packages
        }
    }
