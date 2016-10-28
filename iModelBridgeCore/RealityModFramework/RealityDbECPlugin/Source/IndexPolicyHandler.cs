﻿/*-------------------------------------------------------------------------------------
|
|     $Source: RealityDbECPlugin/Source/IndexPolicyHandler.cs $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+-------------------------------------------------------------------------------------*/

using Bentley.EC.Persistence;
using Bentley.EC.PluginBuilder;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace IndexECPlugin.Source
    {
    internal static class IndexPolicyHandler
        {
        internal static void InitializeHandlers (ECPluginBuilder builder)
            {

            builder//.SetPolicyAssertionSupport(ECPluginBuilder.CommonPolicyAssertion.SupportsMaxResults)
                //.SetPolicyAssertionSupport(ECPluginBuilder.CommonPolicyAssertion.SupportsResultRangeOffset)
                //.SetPolicyAssertionSupport(ECPluginBuilder.CommonPolicyAssertion.SortableQueries)
            .SetPolicyAssertionSupport<PersistenceServicePolicy>(PersistenceServicePolicy.PolicyAssertionNames.StreamBackable, ApplyStreambackableAssertion)
            .SetPolicyAssertionSupport<PersistenceServicePolicy>(PersistenceServicePolicy.PolicyAssertionNames.Insertable, ApplyInsertableAssertion)
            .SetPolicyAssertionSupport<PersistenceServicePolicy>(PersistenceServicePolicy.PolicyAssertionNames.Updateable, ApplyUpdateableAssertion);

            }

        private static void ApplyStreambackableAssertion (DefaultPolicyModule sender, PersistenceServicePolicy policy, ECPolicyContext context)
            {
            //if(context.ECClass.Name == "USGSThumbnail")
            //{
            //    policy.StreamBackable = new FileBackedPolicyAssertion(true);
            //}

            if ( context.ECClass.Name == "PreparedPackage" )
                {
                policy.StreamBackable = new FileBackedPolicyAssertion(true);
                }

            if ( context.ECClass.Name == "Thumbnail" )
                {
                policy.StreamBackable = new FileBackedPolicyAssertion(true);
                }

            if ( context.ECClass.Name == "DownloadReport" )
                {
                policy.StreamBackable = new FileBackedPolicyAssertion(true);
                }
            }

        private static void ApplyInsertableAssertion (DefaultPolicyModule sender, PersistenceServicePolicy policy, ECPolicyContext context)
            {
            if ( context.ECClass.Name == "PackageRequest" )
                {
                policy.Insertable = new InsertablePolicyAssertion(true);
                }
            }

        private static void ApplyUpdateableAssertion (DefaultPolicyModule sender, PersistenceServicePolicy policy, ECPolicyContext context)
            {
            if ( context.ECClass.Name == "DownloadReport" )
                {
                policy.Updateable = new UpdateablePolicyAssertion(true);
                }
            }
        }
    }
