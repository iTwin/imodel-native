# Description
For most Pull requests on imodel-native and imodel-native-internal the traditional PR validation build pipeline will run, and the functionality of the pipeline does not change. In these cases if you have a PR in imodel-native, then the pipeline will pull in the main branch for imodel-native-internal and proceed to build the native addon on all platforms. Same logic applies if there is a change only in imodel-native-internal.

However, if there are two related PRs in both imodel-native and imodel-native-internal, than the PR branches from both repos must be pulled into the build pipeline.

Since both PRs will trigger identical builds, to reduce wasted machine time, only the imodel-native-internal pipeline will fully run. So the imodel-native-internal pipeline is the driving force for these situations.

Also if a developer has a change in itwinjs-core and the native repos (one or both), then the changes in all repos need to be pulled in and tested.

# Validation for Native Repos
## How to Use
### If you have a single PR in imodel-native or imodel-native-internal
-  No extra steps required, the PR validation pipeline will trigger automatically as usual
- The branch for the opposing native repo will default to main

### If you have changes in both imodel-native and imodel-native-internal
##### Steps 1 and 2 can be done simultaneously for best results.
1. Create a PR for your changes in imodel-native-internal, place the link for your imodel-native PR in the description of the internal PR as follows:
    - `imodel-native: https://github.com/iTwin/imodel-native/pull/PULL-NUM`
1. Create a PR for your changes in imodel-native, place the link for your imodel-native-internal PR in the description of the public PR as follows:
    - `imodel-native-internal: https://github.com/iTwin/imodel-native-internal/pull/PULL-NUM`

  - **Special Note for 3.x**: if these changes are on 3.x branch please use the link to the BRANCH and not the PR. This was the original way PR val was written, and the 3.x branch has not been updated yet.
 
If the PRs for both repos were already made before placing the links in the description of the opposing repo, add the links in the description as shown above, and re-trigger the pipelines by commenting `/azp run PIPELINE_NAME` in your PR. `PIPELINE_NAME` is `imodel-native` or `imodel-native-internal` depending on the repo the PR is created in. You should be notified that an azure bot has triggered the pipeline

Once both PRs have passed CI it is Safe to merge. **Do NOT merge either PR until both are ready** 

Avoid re-triggering any pipeline runs in azure that were initially triggered by a PR. This messes up the branch name so that the pipeline runs incorrectly. Instead trigger the pipeline using `/azp run PIPELINE_NAME` as mentioned above.

## Expected Behavior
1. The imodel-native-internal pipeline should begin and in build to completion.
1. The imodel-native pipeline should start, but initially fail after running its first stage (see image below)
1. Upon completion of the imodel-native-internal pipeline, the status check on your imodel-native GH PR should update and allow merging

### Notes:
- To validate the pipeline correctly parsed the description you should see the name of your branch and the link to your branch here:
    - Job `Get PRs`
        - Step `Set Linked PR`
    - note: if you see `/home/vsts/work/_temp/dde4afce-0d0f-47ce-bf9c-4ba8aa881773.sh: line 13: nativePR: command not found
` then no PR is linked
  - if you did not link a PR then you can ignore this message. 
  - if you did intend to link a PR then the regex to find your PR did not see a match. Double check your syntax in the PR description. 
   Regex:
     - `^(imodel-native(|-internal): https://github.com/iTwin/imodel-native(|-internal)/pull/*)`

# Validation for Native and Core repo
## How to use
1. Create a PR for your changes in imodel-native (or native-internal depending on where your changes are), place the link for your itwinjs-core PR in the description of the internal PR as follows:
    - `itwinjs-core: https://github.com/iTwin/itwinjs-core/pull/PULL-NUM`
1. Create a PR for your changes in itwinjs-core, place the link for your native PR in the description of the itwinjs-core PR as follows:
    - `imodel-native: https://github.com/iTwin/imodel-native/pull/PULL-NUM`
1. If you have changes in all 3 repos make sure to add the links to both native and core PRs to both native PRs. In the case of all 3 repos having changes, the most important place to have both PRs linked is in the imodel-native-internal PR since that is where the pipeline will run to completion and validate your other PRs.

1. Because these PRs may most likely not be created simultaneously it would be best to manually trigger the pipelines using `/azp run PIPELINE_NAME` where `PIPELINE_NAME` is `imodel-native`, `imodel-native-internal`, or `iTwin.js` depending on the Repo the PR is in. In this case the iTwin.js pipeline should be skipped since that will not be able to build against your native changes. The addon will be built in your native pipeline.

# Solution to most Errors
- If certain checks fail and need to be rerun, the solution is most likely `/azp run imodel-native-internal` because that is the driving pipeline for changes that require all three repos. If you only have changes in `imodel-native` and `itwinjs-core` then run `/azp run imodel-native`
- If you notice your itwinjs-core PR is failing at first, this is intended. Your native PRs will test your changes in itwinjs-core. Then once it does your native PRs can merge. After merging it will publish a new addon (this will take a while because it needs to rebuild native). Then after the new addon is published it will update your itwinjs-core PR and then core should pass all its builds

### If you have changes only in native then the pipeline will default to test the native addon against the itwinjs-core master branch

## Expected Behavior
### For a native and core change
1. The native pipeline should begin and build to completion. The `Build itwinjs-core` Stage should not be skipped
1. There is currently no solution for stopping the iTwinJs Pipeline, so that will run as if nothing is linked.
1. Upon completion of the native pipeline, the status check on your core PR should be updated
1. The builds for you itwinjs-core PR should still fail. Once your native PR merges, a release will be triggered and that release will be integrated into your pr.
    - This is a safety precaution so people cannot merge a change to itwinjs-core pr before the release of their new addon.
    - You can track the status of the release [here](https://dev.azure.com/bentleycs/iModelTechnologies/_build?view=runs&tagFilter=iModelJsNodeAddonRelease).
### For a change in all 3 Repos
1. The imodel-native-internal pipeline should begin and in build to completion.
1. The imodel-native pipeline should start, but initially fail after running its first stage (see image below)
1. The iTwinJs pipeline will begin, but may run into issues as mentioned above
1. Upon completion of the imodel-native-internal pipeline, the status checks on your imodel-native and itwinjs-core GH PR should update and allow merging
    - same rules apply here for waiting for your new addon to be published.