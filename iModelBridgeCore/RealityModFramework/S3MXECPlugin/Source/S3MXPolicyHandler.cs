using Bentley.EC.Persistence;
using Bentley.EC.PluginBuilder;

namespace S3MXECPlugin.Source
{
    internal static class IndexPolicyHandler
        {
        internal static void InitializeHandlers (ECPluginBuilder builder)
            {

            builder .SetPolicyAssertionSupport<PersistenceServicePolicy>(PersistenceServicePolicy.PolicyAssertionNames.Updateable, ApplyUpdatableAssertion)
                    .SetPolicyAssertionSupport<PersistenceServicePolicy>(PersistenceServicePolicy.PolicyAssertionNames.StreamBackable, ApplyStreambackableAssertion);

            }

        private static void ApplyStreambackableAssertion (DefaultPolicyModule sender, PersistenceServicePolicy policy, ECPolicyContext context)
            {
            // this is to download a Document file
            if (context.ECClass.Name == "Document")
                {
                policy.StreamBackable = new FileBackedPolicyAssertion(true);
                }
            }

        private static void ApplyUpdatableAssertion(DefaultPolicyModule sender, PersistenceServicePolicy policy, ECPolicyContext context)
        {
            // this is to upload a Document file
            if (context.ECClass.Name == "Document")
                {
                policy.Updateable = new UpdateablePolicyAssertion(true);
                }
        }
    }
}
