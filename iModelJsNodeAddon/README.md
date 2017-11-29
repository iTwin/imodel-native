# imodeljsnodeaddonapi

The imodeljs node "addon" contains native code that is projected into JavaScript for use by imodeljs-core backend.

iModelJsNodeApi.d.ts contains the declaration of the iModelJs Node Addon APIs.

The addon itself is in a different package. In fact, there will be a separate package for each combination of node version and target platform that is supported. For example, @bentley/imodeljs-n_8_9-winx64 is the version of the addon that can be used on a Windows desktop machine running node v8.9.x. In addition, some apps may build custom versions of the addon.

## Who Knows What Addon to Use?

imodeljs-core does not have a package dependency on the addon.

Instead, the addon is delivered by the app. This allows the app to substitute a custom addon of its own. Or, an app can use the default addon. In either case, it is up to the app to know what addon it wants to use, to obtain that addon, and to include it in its installation package. The app must then tell imodeljs-core where the addon is by calling a method on the NodeAddon class in imodeljs-core. imodeljs-core will require the delivered addon (whatever it is) and use the classes and methods that it contains.

## Implementations and the Addon API

The imodeljs node addon defines classes, methods, and properties. The addon's API is *declared* in the iModelJsNodeApi.d.ts file. That is how imodeljs-core sees the API at build time. So, imodeljs-core is built to expect a particular addon API. The addon that is loaded and used at runtime must implement the expected API. imodeljs-core does a runtime check to verify this: the addon's version and the version of imodeljsnodeaddonapi must have the same first digit, and the second digit of the addon must be greater than or equal to the second digit of imodeljsnodeaddonapi.

## Versioning Rules

Increment the *first digit* of the version if you remove or modify the signature of any existing method, or if you otherwise change the contract of an existing method.

Increment the *second digit* of the version if you add new methods to the addon API.

Increment the *third digit* if you merely fix a bug or otherwise change the addon in a way that does not affect the API.

Obviously, if you change the addon's API in any way, you must update iModelJsNodeApi.d.ts to reflect the change.

imodeljs-core has a dependency on imodeljsnodeaddonapi. Moving up to a new version of the addon, requires two things:
1. Increment the version that imodeljs-core depends on.
2. Make sure that the app that uses imodeljs-core obtains and delivers a corresponding version of the addon.

Note: imodeljs-core depends on imodeljsnodeaddonapi in two places: in backend/package.json and test/package.json. Keep them consistent!

# Building and Publishing the iModelJs Node Addon

## Build

If you change the addon code itself or any library that it links with, build the addon like this: 

`bb -riModelJsNodeAddon -piModelJsNodeAddon:MakePackages b`

(You might need to use a rebuild command to force incremental rebuilds of individual parts.)

That will print a message like this: 

`npm publish %OutRoot%Winx64\packages\imodeljs-E_1_6_11-WinX64`
`npm publish %OutRoot%Winx64\packages\imodeljs-N_8_9-WinX64`
`npm publish %OutRoot%Winx64\packages\imodeljsnodeaddonapi`

That message identifies the (generated) npm packages that contain the addon, as well as the API package. This example is from a Windows build. Note that you produce two addon packages from the same source, one for use in a node app and another for an electron app. The API package is the same for both.

## Test 

To test changes to the native code before publishing, you must at a minimum install your local build of the addon into your local build of imodeljs-core and then run unit tests. 

The following shortcut works in the simple case where you just want to try out a local build of your native code along with your local build of imodeljs-core: 

`cd <localjsroot>\imodeljs-core`
`npm install --no-save  %OutRoot%Winx64\packages\imodeljs-N_8_9-WinX64`

(Assuming that you are testing on Windows. Use whatever path is printed by the build step above.)

WARNING: Make sure that you do not accidentally add imodeljs-N_8_2-WinX64 to dependencies in your packages.json file! 

## Publish 

When you are ready to publish the addon:
1. Edit %SrcRoot%iModelJsNodeAddon\package_version.txt and increment the version number. Don't forget to hg commit and hg push that. 
2. Rebuild the MakePackages part, as described above. 
3. Run the npm publish commands that are displayed by the MakePackages part. ***Run all of them.***

# How To Update To a New Version of Node 

First, get the node header files and the .lib for the desired version. 

If you haven't already, install node-gyp: 
`npm install -g node-gyp`

Tell node-gyp to install the version of nodejs that you want. 
`node-gyp install <nodevernum>`

That will install the headers and libs to the .node-gyp directory in your %homedrive%%homepath% directory. Copy the files from the relevant subdirectory to thirdparty\nodejs\node-gyp 

Next, copy that whole directory as-is to %SrcRoot%thirdparty\nodejs\node-gyp. Then, rename the copied directory by prefixing it with "N_" and replacing dots with underscores.
For example, suppose you want to update to 8.2.1. You would copy the %homedrive%%homepath%.node-gyp\8.2.1 to %SrcRoot%thirdparty\nodejs\node-gyp. You would then rename the copy to N_8_2_1. 

Finally, change the PartFiles 

Edit %SrcRoot%thirdparty\nodejs\nodejs.PartFile.xml and change the node-gyp part to pass -dNODE_GYP_VER=<nodever> to its mke file. 
Edit %SrcRoot%iModelJsNodeAddon\DgnPlatform.PartFile.xml and change the nodejs_ver name to <nodever_major.nodever_minor> in the iModelJsNodeAddon product directory. Note that we specify only major and minor version, not build number</verbatim> 
