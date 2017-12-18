# @bentley/imodeljs-nodeaddonapi

<p style="text-align: center;">
<b><i><mark>
<mark><em>The addon and its api are for the private use of imodeljs-backend only.</em></mark>
</b></i></mark>
</p>


The iModelJs node addon contains native code that is projected into JavaScript. The @bentley/imodeljs-nodeaddonapi package declares the classes, methods, and properties -- the API -- that the addon implements.

The addon and its api declaration are in separate packages. In fact, there are many addon packages, one for each combination of node version and target platform that is supported. For example, @bentley/imodeljs-n_8_9-win32-x64 is the version of the addon that can be used on a Windows desktop machine running 64-bit node v8.9.x. In addition, some apps may build custom versions of the addon. There is only one api package.

imodeljs-backend does not have a package dependency on any addon package. Instead, addons are delivered by apps. An app can depend on and deliver the standard node addon. Or, an electron app can depend on and deliver the standard electron addon. Or, an app can build and deliver a custom-built addon. In any case, it is up to the app to know what addon it wants to use, to obtain that addon, and to include the addon in its installation package. The app must then load the addon and register it with imodeljs-backend. imodeljs-backend will look for the native classes and methods that it needs in the registered addon.

imodeljs-backend does have a package dependency on imodeljs-nodeaddonapi and is built to expect a particular addon API. The addon that is loaded and used at runtime must implement the expected API. imodeljs-backend checks compatibility at run time.

**Key point:** The app delivers the addon. imodeljs-backend depends on a version of the addon api.

# iModelJsNodeAddon

The addon and its api are not in git. The code for the addon and the declaration of the api are in a Mercurial (hg) repository called iModelJsNodeAddon. The addon is built by BentleyBuild. 

The addon and api packages are generated. See the iModelJsNodeAddon::MakePackages part.

The generated addon and api packages are published by PRG using BentleyBuild.

The addon api is defined by imodeljs-nodeaddonapi.d.ts. When you write addon code, you define JavaScript classes and methods using the native node_addon_api. You must also declare the classes and methods in the imodeljs-nodeaddonapi.d.ts file. Yes, you must define the API twice -- once in native code and again in the d.ts file -- there is nothing to make this automatic or to check consistency -- you must do this manually.

imodeljs-nodeaddonapi.d.ts contains the (internal) documentation for the addon. This README file is also part of the addon api package, and it contains these build instructions.

The package version number for the addon and its api are stored in the file iModelJsNodeAddon/package_version.txt. Parts in iModelJsNode read the package version from this file and inject it into the addon binaries and into the addon and api packages.

# Changing and Publishing the Addon

**Key point:** The addon packages and the api package must all be published with the same version number.

**Key point:** The addon and imodeljs-nodeaddonapi.d.ts must be in sync.

## Build

You can build the addon like this:

`bb -s"iModelJsNodeAddon;BuildAll" b`

To force a rebuild of individual parts, do this:

`bb -s"iModelJsNodeAddon;BuildAll" re DgnPlatformDLL <other libraries...> iModelJsNodeAddonLib* MakePackages`

## Version

The package version number for the addon and its api are stored in the file iModelJsNodeAddon/package_version.txt. This is used to generate the packages. So, you must update the version number in this file when you change the addon. Specifically:

Increment the *first digit* of the version if you remove or modify the signature of any existing class or method, or if you otherwise change the contract of an existing method.

Increment the *second digit* of the version if you add new classes, methods, or properties to the addon API.

Increment the *third digit* if you merely fix a bug or otherwise change the implementation of the addon in a way that does not affect the API or the contract of any method.

If you change only the documentation content in the d.ts or README files, then increment the third digit of the package version.

## Update the Declarations

If you change the addon's API in native code, e.g., by adding a method, you must also update iModelJsNodeAddonApi.d.ts to reflect the change.

## Burn in the Version

You must rebuild the addon itself and the packages whenever you change the version. Note that the version number is *burned into the code*. This allows imodeljs-backend to do its runtime version-compatibility check. So, it's not enough just to regenerate the packages -- you must also rebuild the addon. After changing the version number, re-build like this:

```
bb -s"iModelJsNodeAddon;BuildAll" re iModelJsNodeAddonLib* MakePackages -c
bb -s"iModelJsNodeAddon;BuildAll" re iModelJsNodeAddonLib* MakePackages
```

## Test

To test changes to the native code before publishing, you can install your local build of the api and the addon into your local build of imodeljs-core and run unit tests.

For example:

```
cd source\backend
call npm install --no-save  %OutRoot%Winx64\packages\imodeljs-nodeaddonapi
cd ..\test
call npm install --no-save  %OutRoot%Winx64\packages\imodeljs-nodeaddonapi %OutRoot%Winx64\packages\imodeljs-n_8_9-win32-x64
```

This example is for a windows build. Use whatever package names are displayed by the iModelJsNodeAddon::MakePackages part.

## Publish

When you bb build the MakePackages part,

`bb -s"iModelJsNodeAddon;BuildAll" re MakePackages`

That will print publishing instructions. For example:

``` bat
npm publish %OutRoot%Winx64\packages\imodeljs-e_1_6_11-win32-x64
npm publish %OutRoot%Winx64\packages\imodeljs-n_8_9-win32-x64
npm publish %OutRoot%Winx64\packages\imodeljs-nodeaddonapi
```

That message identifies the (generated) npm packages that contain the addon, as well as the API package. This example is from a Windows build. The messages from a Linux or MacOS build will be similar, but will show names that are specific to those platforms. Note MakePackages produces several packages from the same source. In the case of a Windows build, there is one addon package for use in a node app and another for an electron app. The API package is the same for both. All of the packages must be published together.

Run the npm publish commands that are displayed by the MakePackages part.

PRG must ensure that there is a version of the addon for each platform for a given version. When a new version is committed to the iModelNodeJsAddon repository, (***TBD***: who?) must stamp it, and PRG must pull the source on that stamp for each platform and publish the result.

## Package Dependencies

**Key point:** imodeljs-backend must depend on a specific minor version of the api.

**Key point:** The minor version of imodeljs-backend must advance with the minor version of the api that it uses.

**Key point:** Apps must depend on a specific minor version of imodeljs-backend.

imodeljs-backend requires that apps deliver a compatible version of the addon. That means that, if imodeljs-backend moves up to a newer minor version of the api, then apps must move up to a newer version of the addon ... if they want to use the newer backend. 

We must use semantic versioning to control when a newer minor version of the addon is required, and locking on to minor version is the key to doing this. modeljs-backend/package.json must use a ~ range, not a ^ range, for its dependency on imodeljs-nodeaddonapi. That will allow imodeljs-backend to float to new patches of the existing addon but not to newer minor versions. It will take an explicit change to the dependency to move up to a newer minor version. That's half of the solution. We also need a way to advance the minor version requirement of imodeljs-backend without breaking apps. The solution is a) to advance the minor version of imodeljs-backend itself whenever it changes to require a newer minor version of the api, and b) require apps to depend on a minor version of imodeljs-backend. 

This scheme allows us to version the addon api in steps, with all downstream consumers opting in when they are ready. In a nutshell, if you add new methods to the addon and you want imodeljs-backend to consume them, the upgrade process is:
1. Publish a new version of the addon and api packages with a higher minor version.
1. Once they have landed, imodeljs-backend can move up:
  a. Change imodeljs-backend/package.json to depend on the higher minor version of the api.
  b. NPM UPDATE
  c. Change the minor version of the imodeljs-backend package itself.
  d. Publish imodeljs-backend.
1. Once that has landed, apps can move up:
  a. Change their package.json to depend on the newer minor version of the addon.
  b. NPM UPDATE

Note: imodeljs-backend depends on imodeljs-nodeaddonapi in two places: in backend/package.json and test/package.json. Keep them consistent!

### How imodeljs-backend Checks Version Compatibility

imodeljs-core/backend/NodeAddonRegistry.registerAddon verifies that the loaded addon implements the api that imodeljs-backend expects. There are three tests: 1) The addon and the api must be the same generation (same major version). 2) The addon must include all of the classes and methods that the backend expects and may include new classes that the backend is not yet using (same or greater minor version). And, 3) the addon must include all required bug fixes and may include more recent bug fixes (same or higher patch version). 

# APPENDIX: How to Move to a New Version of Node

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
