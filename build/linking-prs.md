# Description
If you are working in imodel-native, you will need to go through a PR validation process to ensure all of your changes work correctly and pass all tests. Part of these tests include the validation tests in the itwinjs-core GitHub repository. In the event that your PR in imodel-native requires changes in itwinjs-core, then you will need to follow this document to correctly link your prs together so that you can run all the correct tests.

# PR Validation Steps
## How to Use
### If you have a single PR in imodel-native:
- No extra steps required, the PR validation pipeline will trigger automatically as usual
- Your PR will test against master branch of itwinjs-core

### If you have a PRs in imodel-native and itwinjs-core that are dependent on each other:
1. Create a PR for your changes in itwinjs-core, and copy the link the to new PR
1. Create a PR for your changes in imodel-native place the link for your itwinjs-core PR in the description of the imodel-native PR as follows:
    - `itwinjs-core: https://github.com/iTwin/itwinjs-core/pull/PULL-NUM`
1. Go pack to you itwinjs-core PR and update the description to have your native PR in it.
    - `imodel-native: https://github.com/iTwin/imodel-native/pull/PULL-NUM`
1. If you already created the PRs just add this links to your PR description and then in the imodel-native PR comment `/azp run imodel-native` to retrigger the pipelines.
1. Wait until your pipeline succeeds and *both* PRs are approved.
1. Once both PRs are approved merge the imodel-native PR. This will create a new release with your change and automatically add the new release of the addon to your itwinjs-core PR. (This may take a few hours to build and publish your changes)

1. Once you see the new addon is published and committed to your itwinjs-core PR, wait for tests to run and merge PR.

# Solution to most Errors
- If certain checks fail and need to be rerun, the solution is most likely commenting on the PR `/azp run imodel-native` because that will re run your pipeline native pipeline.
- If you notice your itwinjs-core PR is failing at first, this is intended. Your native PRs will test your changes in itwinjs-core. Then once it does your native PRs can merge. After merging it will publish a new addon (this will take a while because it needs to rebuild native). Then after the new addon is published it will update your itwinjs-core PR and then core should pass all its builds
- 

## Expected Behavior
### For a native and core change
1. The native pipeline should begin and build to completion. The `Build itwinjs-core` Stage should not be skipped
1. There is currently no solution for stopping the iTwinJs Pipeline, so that will run as if nothing is linked, and will most likely fail.
1. Upon completion of the native pipeline, the status check on your core PR should be updated
1. The builds for you itwinjs-core PR should still fail. Once your native PR merges, a release will be triggered and that release will be integrated into your pr.
    - This is a safety precaution so people cannot merge a change to itwinjs-core pr before the release of their new addon.