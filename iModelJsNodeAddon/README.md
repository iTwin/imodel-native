# @bentley/imodeljs-nodeaddonapi

The iModelJs node addon contains native code that is projected into JavaScript for use by imodeljs-core backend. The @bentley/imodeljs-nodeaddonapi package declares the classes, methods, and properties -- the API -- that the addon implements.

The addon and api declaration are in separte packages. In fact, there will be many versions of the addon, one for each combination of node version and target platform that is supported, and each will be in its own package. For example, @bentley/imodeljs-n_8_9-winx64 is the version of the addon that can be used on a Windows desktop machine running node v8.9.x. In addition, some apps may build custom versions of the addon.

imodeljs-core does not have a package dependency on the addon. Instead, the addon is delivered by the app. This allows the app to substitute a custom addon of its own. Or, an app can use the default addon. In either case, it is up to the app to know what addon it wants to use, to obtain that addon, and to include it in its installation package. The app must then tell imodeljs-core where the addon is by calling a method on the NodeAddon class in imodeljs-core. imodeljs-core will `require` the delivered addon (whatever it is) and use the classes and methods that it contains.

imodeljs-core does have a package dependency on imodeljs-nodeaddonapi and is built to expect a particular addon API. The addon that is loaded and used at runtime must implement the expected API, as explained below.

Note: imodeljs-core depends on imodeljs-nodeaddonapi in two places: in backend/package.json and test/package.json. Keep them consistent!

## Versioning Rules

Increment the *first digit* of the version if you remove or modify the signature of any existing class or method, or if you otherwise change the contract of an existing method.

Increment the *second digit* of the version if you add new classes, methods, or properties to the addon API.

Increment the *third digit* if you merely fix a bug or otherwise change the implementation of the addon in a way that does not affect the API or the contract of any method.

Obviously, if you change the addon's API in any way, you must update iModelJsNodeApi.d.ts to reflect the change.

In order to verify that the addon is compatible, imodeljs-core compares the version of the addon, *addon*, with the version of imodeljs-nodeaddonapi from which imodeljs-core was built, *api*:
1. The first digit of *addon* must be equal to the first digit of *api*, and
2. The second digit of *addon* must be greater than or equal to the second digit of *api*.

From these versioning rules, it is clear that an app use an existing version of imodeljs-core with a newer version of the addon, as long as the API remains compatible. To use a newer, incompatible version of the addon, the app must must move to a newer version of imodeljs-core.

*TBD: How can an app tell, just by looking at the imodeljs-core package on npm.bentley.com, what version of the api is required by it?*

# Building the Addon

You can build the addon like this:

`bb -siModelJsNodeAddon b`

If you are making changes to the libraries that are used by the addon, such as DgnPlatform.dll, then you must add BuildAll to your build strategy, in order to build and link with your local build, rather than with LKGs, like this:

`bb -s"imodeljs-nodeaddonapi;BuildAll" b`

To force a rebuild of individual parts, do this:

`bb "imodeljs-nodeaddonapi;BuildAll" re DgnPlatformDLL <other libraries...> iModelJsNodeAddonLib* MakePackages`

The MakePackages part will print a message like this: 

``` bat
npm publish %OutRoot%Winx64\packages\imodeljs-E_1_6_11-WinX64
npm publish %OutRoot%Winx64\packages\imodeljs-N_8_9-WinX64
npm publish %OutRoot%Winx64\packages\imodeljs-nodeaddonapi
```

That message identifies the (generated) npm packages that contain the addon, as well as the API package. This example is from a Windows build. The messages from a Linux or MacOS build will be similar, but will show names that are specific to those platforms. Note MakePackages produces several packages from the same source. In the case of a Windows build, there is one addon package for use in a node app and another for an electron app. The API package is the same for both. All of the packages must be published together.

But, before publishing ...

# Test the Addon

To test changes to the native code before publishing, you must at a minimum install your local build of the addon into imodeljs-core and then run unit tests, as follows:

``` bat
cd <localjsroot>\imodeljs-core
cd source\backend
npm install --no-save  %OutRoot%Winx64\packages\imodeljs-nodeaddonapi
npm install --no-save  %OutRoot%Winx64\packages\imodeljs-N_8_9-WinX64
cd ..\test
npm install --no-save  %OutRoot%Winx64\packages\imodeljs-nodeaddonapi
npm install --no-save  %OutRoot%Winx64\packages\imodeljs-N_8_9-WinX64
```

Here is the same set of commands in the form of a .bat file:

``` bat
cd <localjsroot>\imodeljs-core
cd source\backend
call npm install --no-save  %OutRoot%Winx64\packages\imodeljs-nodeaddonapi
call npm install --no-save  %OutRoot%Winx64\packages\imodeljs-N_8_9-WinX64
cd ..\test
call npm install --no-save  %OutRoot%Winx64\packages\imodeljs-nodeaddonapi
call npm install --no-save  %OutRoot%Winx64\packages\imodeljs-N_8_9-WinX64
```

This example is for a Windows node app. Use appropriate paths, as printed by the MakePackages part in your local build.

Note the `--no-save` argument. Make sure that you do not accidentally add imodeljs-N_8_2-WinX64 to dependencies in your packages.json file! imodeljs-core must not have a hard dependency on any particular addon package!
Also, even though imodeljs-core does depend on imodeljs-nodeaddonapi, make sure that you don't add a dependency that refers to a local file.

Then go back up to the imodeljs-core parent directory and rebuild and run the tests.

# Publish the Addon

### Change the Package Version

The package version that is used for both the addon(s) and the corresponding imodeljs-nodeaddonapi is contained in a text file called `%SrcRoot%imodeljs-nodeaddonapi\package_version.txt`. Edit this file to change the version number as appropriate. Don't forget to hg commit and hg push that. 

### Rebuild the packages

After changing the version number, you must rebuild the MakePackages part:

`bb -siModelJsNodeAddon re MakePackages`

### Publish

Run the npm publish commands that are displayed by the MakePackages part. Don't forget to run *all* of the publish commands.

# Move to New Version of Node

The addon is still specific to a major.minor version of nodejs. (That will change after we move to node v9 next year.) If you need to support a newer version of node, do the following:

### Get New node-gyp Package

Get the node header files and the .lib for the desired version. 

If you haven't already, install node-gyp: 
`npm install -g node-gyp`

Tell node-gyp to install the version of nodejs that you want. 
`node-gyp install <nodevernum>`

That will install the headers and libs to the .node-gyp directory in your %homedrive%%homepath% directory. Copy the files from the relevant subdirectory to thirdparty\nodejs\node-gyp 

### Update thirdparty/nodejs/node-gyp

1. Copy that whole directory to `%SrcRoot%thirdparty\nodejs\node-gyp`, creating a new subdirectory with the same name as the origin.
2. Rename the new subdirectory by prefixing it with "N_" and replacing dots with underscores. 

For example, suppose you want to update to 8.9.2 for node addons. You would copy the `%homedrive%%homepath%.node-gyp\8.9.2` to `%SrcRoot%thirdparty\nodejs\node-gyp`. That would create a subdirectory called `8.9.2`. You would then rename the copy to `N_8_9_2`. 

### Change Partfiles

Finally, change the PartFiles that refer to the version of the node api that is used to build the addon. 

1. Edit `%SrcRoot%thirdparty\nodejs\napi\node-addon-api.mke` and change the value of the macros such as `nodeIncludes` that you see that refer to the version-specific node-gyp includes and libs. 

2. Edit `%SrcRoot%imodeljs-nodeaddonapi\imodeljs-nodeaddonapi.PartFile.xml` and change the node version that you see in the various `iModelJsNode_node_module` bindings. Note that we specify only major and minor version, not build number.
