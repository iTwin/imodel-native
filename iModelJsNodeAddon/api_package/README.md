# @bentley/imodeljs-nodeaddonapi

<p style="text-align: center;">
<b><i><mark>
<mark><em>The addon and its api are for the private use of imodeljs-backend only.</em></mark>
</b></i></mark>
</p>


The iModelJs node addon contains native code that is projected into JavaScript. The @bentley/imodeljs-nodeaddonapi package declares the classes, methods, and properties -- the API -- that the addon implements.

The addon and its api declaration are in separate packages. In fact, there are many addon packages, one for each combination of node version and target platform that is supported. For example, @bentley/imodeljs-n_8_9-win32-x64 is the version of the addon that can be used on a Windows desktop machine running 64-bit node v8.9.x. In addition, some apps may build custom versions of the addon. There is only one api package.

imodeljs-backend does have a package dependency on imodeljs-nodeaddonapi and is built to expect a particular addon API. The addon that is loaded and used at runtime must implement the expected API. imodeljs-backend checks compatibility at run time.

imodeljs-backend has a package dependency on the addon packages, and this dependency specifies a compatible version. This relieves the app of the responsibility for getting a compatible addon.

# iModelJsNodeAddon

The addon and its api are not in git. The code for the addon and the declaration of the api are in the hg repository called iModelJsNodeAddon. The addon is built by BentleyBuild. 

The addon and api packages are generated. See the iModelJsNodeAddon::MakePackages part.

The generated addon and api packages are published by PRG using BentleyBuild.

The addon api is defined by imodeljs-nodeaddonapi.d.ts. When you write addon code, you define JS classes and methods using the native node_addon_api. You must also declare the classes and methods in the imodeljs-nodeaddonapi.d.ts file. Yes, you must define the API twice -- once in native code and again in the d.ts file -- there is nothing to make this automatic or to check consistency -- you must do this manually.

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
bb "iModelJsNodeAddon;BuildAll" re iModelJsNodeAddonLib* MakePackages -c
bb "iModelJsNodeAddon;BuildAll" re iModelJsNodeAddonLib* MakePackages
```

## Test

To test changes to the native code before publishing, you can install your local build of the api and the addon into your local build of imodeljs-core and run unit tests.

When you bb build the MakePackages part,

`bb -s"iModelJsNodeAddon;BuildAll" re MakePackages`

That will print the location of the generated packages. For example:

``` bat
%OutRoot%Winx64\packages\imodeljs-e_1_6_11-win32-x64
%OutRoot%Winx64\packages\imodeljs-n_8_9-win32-x64
%OutRoot%Winx64\packages\imodeljs-nodeaddonapi
%OutRoot%Winx64\packages\imodeljs-nodeaddon
%OutRoot%Winx64\packages\imodeljs-electronaddon
```

That message identifies the (generated) npm packages that contain the addon, as well as the API package. This example is from a Windows build. The messages from a Linux or MacOS build will be similar, but will show names that are specific to those platforms. Note MakePackages produces several packages from the same source. In the case of a Windows build, there is one addon package for use in a node app and another for an electron app. The API package is the same for both.

NB: The version number in package_version.txt must be the old version number. Change package_version.txt *after* testing.

Continuing this example, suppose your imodeljs git repository is here:

```
\imjs\imodeljs-core
```

On Windows you would install your local build of the addon like this:

```
if .%ImodelJsRoot% == . goto :missingvar

REM These installs point the aggregator packages to the local builds of the platform-specific addon packages.
REM Note: The names of the platform-specific addon packages are platform-, cpu-, and node/electron version-specific.
cd %OutRoot%Winx64\packages\imodeljs-nodeaddon
call npm install --no-save  %OutRoot%Winx64\packages\imodeljs-nodeaddonapi %OutRoot%Winx64\packages\imodeljs-n_8_9-win32-x64
cd %OutRoot%Winx64\packages\imodeljs-electronaddon
call npm install --no-save  %OutRoot%Winx64\packages\imodeljs-nodeaddonapi %OutRoot%Winx64\packages\imodeljs-e_1_6_11-win32-x64

REM Note: You must re-run the above install command whenever you rebuild the MakePackages part, as that will re-create 
REM         the %OutRoot%Winx64\packages\imodeljs-nodeaddon and %OutRoot%Winx64\packages\imodeljs-electronaddon directories.

REM Next, install the addon aggregator and api packages in imodeljs. 
REM Note that you must install these packages in each package that depends on them.
REM Note that you do not install the platform-specific addons in this step. They are nested in the aggregator packages.

cd %ImodelJsRoot%imodeljs-core\source\backend
call npm install --no-save  %OutRoot%Winx64\packages\imodeljs-nodeaddonapi %OutRoot%Winx64\packages\imodeljs-nodeaddon %OutRoot%Winx64\packages\imodeljs-electronaddon
cd %ImodelJsRoot%imodeljs-core\source\test
call npm install --no-save  %OutRoot%Winx64\packages\imodeljs-nodeaddonapi %OutRoot%Winx64\packages\imodeljs-nodeaddon
cd %ImodelJsRoot%imodeljs-core\source\testbed
call npm install --no-save  %OutRoot%Winx64\packages\imodeljs-nodeaddonapi %OutRoot%Winx64\packages\imodeljs-electronaddon
cd %ImodelJsRoot%imodeljs-core

goto :xit

:missingvar
echo Define ImodelJsRoot to point to the parent directory that contains imodeljs-core. For example: set ImodelJsRoot=d:\imjs\

:xit

```

On Linux:

```
export ImodelJsRoot=<The parent directory of imodeljs-core>
cd $OutRoot/LinuxX64/packages/imodeljs-nodeaddon
npm install --no-save  $OutRoot/LinuxX64/packages/imodeljs-nodeaddonapi $OutRoot/LinuxX64/packages/imodeljs-n_8_9-linux-x64
cd $OutRoot/LinuxX64/packages/imodeljs-electronaddon
npm install --no-save  $OutRoot/LinuxX64/packages/imodeljs-electronaddonapi $OutRoot/LinuxX64/packages/imodeljs-e_1_6_11-linux-x64
cd $ImodelJsRoot/imodeljs-core/source/backend
npm install --no-save  $OutRoot/LinuxX64/packages/imodeljs-nodeaddonapi $OutRoot/LinuxX64/packages/imodeljs-nodeaddon $OutRoot/LinuxX64/packages/imodeljs-electronaddon
cd $ImodelJsRoot/imodeljs-core/source/test
npm install --no-save  $OutRoot/LinuxX64/packages/imodeljs-nodeaddonapi $OutRoot/LinuxX64/packages/imodeljs-nodeaddon
cd $ImodelJsRoot/imodeljs-core/source/testbed
npm install --no-save  $OutRoot/LinuxX64/packages/imodeljs-nodeaddonapi $OutRoot/LinuxX64/packages/imodeljs-electronaddon
cd $ImodelJsRoot/imodeljs-core

```

To test other platforms or other versions, use the names of the generated packages that are displayed by the MakePackages part.

# Publishing the addon

The addon packages are published by PRG, not by developers.

## Package Dependencies

**Key point:** imodeljs-backend must depend on a specific minor version of the api.

**Key point:** The minor version of imodeljs-backend must advance with the minor version of the api that it uses.

**Key point:** Apps must depend on a specific minor version of imodeljs-backend.

This scheme allows us to version the addon api in steps, with all downstream consumers opting in when they are ready. In a nutshell, if you add new methods to the addon and you want imodeljs-backend to consume them, the upgrade process is:
1. Publish a new version of the addon and api packages with a higher minor version.
1. Once they have landed, imodeljs-backend can move up:
  a. Change imodeljs-backend/package.json to depend on the higher minor version of the addon and the api.
  b. NPM INSTALL
  c. Change the minor version of the imodeljs-backend package itself.
  d. Publish imodeljs-backend.
1. Once that has landed, apps can move up:
  a. Change their package.json to depend on the newer minor version of imodeljs-backend.
  b. NPM INSTALL

Note: imodeljs-backend depends on imodeljs-nodeaddon and imodeljs-nodeaddonapi in three places: in backend/package.json, test/package.json, and testbed/package.json. Keep them consistent!

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
