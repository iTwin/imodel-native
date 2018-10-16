# @bentley/imodeljs-native-platform-api

Rolling a new version of the native platform code is a two-step process:
1. *Change the native platform* -- Change the native code and update package_version.txt in iModelNodeAddon.
2. *Adopt the native platform* -- Change the imodeljs-native-platform-api.d.ts file in imodeljs-core/core/backend and change TypeScript code as necessary to react to API changes.

These steps are described in more detail in the sections below.

# Changing and Publishing the Native Platform Packages

## 1. Change, Build and Test

Change the C++ files in iModelJsNodeAddon. Use only N-API (https://nodejs.org/api/addons.html). Do not use the v8 or "Nan" APIs. We generally use the N-API C++ wrapper classes.

Build the native platform like this:

`bb -s"iModelJsNodeAddon;BuildAll" b`

To force a rebuild of individual parts, do this:

`bb -s"iModelJsNodeAddon;BuildAll" re DgnPlatformDLL iModelJsNodeAddonLib* MakePackages`

***See below for testing. You should test before updating package_version.txt and before requesting a PRG build.***

## 2. Update package_version.txt

Before requesting a PRG build, you must update the version number of the native platform package(s).

The package version number for the *implementation* of the native platform is stored in `iModelJsNodeAddon/package_version.txt`. BentleyBuild parts in iModelJsNodeAddon read the version number from this file and inject it into the native platform binaries and into the generated native platform package.json files.

Increment the *first digit* of the version if you remove or modify the signature of any existing class or method, or if you otherwise change the contract of an existing method.

Increment the *second digit* of the version if you add new classes, methods, or properties to the native platform API.

Increment the *third digit* if you merely fix a bug or otherwise change the implementation of the native platform in a way that does not affect the API or the contract of any method.

FYI The native platform package version number is also burned into the native code. This allows imodeljs-backend to do a version-compatibility check at runtime. It is not necessary to to burn in a new version number as part of your testing. If for some reason you want to do this, you must re-build like this after changing package_version.txt:

``` cmd
bb -s "iModelJsNodeAddon;BuildAll" re iModelJsNodeAddonLib* MakePackages -c
bb -s "iModelJsNodeAddon;BuildAll" re iModelJsNodeAddonLib* MakePackages
```

## 3. Publish

Request a PRG build of the native platform packages. Specify the new version number in your request. Wait for the result.

# Testing Native Platform Changes and Corresponding Backend Changes

You test the native platform by calling it from TypeScript.

First, install your local build of the native platform. There is a `installNativePlatform` script in this directory for each supported platform. There are scripts for each supported platform.

Next, update imodeljs-core/core/backend/imodeljs-native-platform-api.d.ts to reflect the changes made to the API implemented by native code. (Do not change the version number yet.)

Finally, update .ts files in imodeljs-core/core/backend as necessary to react to changes in the native platform API and then run TypeScript tests and sample apps.

# Adopting a New Version of the Native Platform packages

The native platform version number appears in *two places* in imodeljs-core/core/backend: In `package.json` and in `imodeljs-native-platform-api.d.ts`. To move to a new version of the native platform, you must update both places to use the new version number.

Then, rush install and then rush build.

Make sure tests are still passing.

Push.

# How to Move to a New Version of Node

If you need to support a newer version of node, do the following:

If you haven't already, install node-gyp:
`npm install -g node-gyp`

Tell node-gyp to install the version of nodejs that you want.
```
node-gyp install <nodevernum>
```

That will install the headers and libs to the .node-gyp directory in your %homedrive%%homepath% directory.

Copy the files to the appropriate subdirectory of thirdparty\node-addon-api\node-gyp. The name of the target directory in thirdparty\node-addon-api\node-gyp should be N_v, where v is the major node version number.

Finally, change thirdparty\node-addon-api\node-addon-api.PartFile.xml and update the part that refers to the version of the node API that is used to build the native platform.

# How to Move to a New Version of Electron

If you need to support a newer version of node, do the following:

Tell node-gyp to install the version of electron that you want. For example, to install version 2.0.8, do this:
```
node-gyp install --target=2.0.8 --arch=x64 --dist-url=https://atom.io/download/electron
```

That will install the headers and libs to the %homedrive%%homepath%\.node-gyp\iojs-2.0.8 directory.

Copy the files to the appropriate subdirectory of thirdparty\node-addon-api\node-gyp, as follows:

The name of the target directory in thirdparty\node-addon-api\node-gyp should be E_v, where v is the major electron version number.

Copy all of the files in the following directories into the target directory:
* %homedrive%%homepath%\.node-gyp\iojs-2.0.8\src
* %homedrive%%homepath%\.node-gyp\iojs-2.0.8\deps\v8\include
* %homedrive%%homepath%\.node-gyp\iojs-2.0.8\deps\uv\include

Copy the the following directories as directories into the target directory:
* %homedrive%%homepath%\.node-gyp\iojs-2.0.8\deps\v8\include\libplatform


Finally, change thirdparty\node-addon-api\node-addon-api.PartFile.xml and update the part that refers to the version of the electron API that is used to build the native platform.
