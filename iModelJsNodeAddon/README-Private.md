# @bentley/imodeljs-native-platform-api

Rolling a new version of the native platform code is a two-step process:
1. *Change the native platform* -- Change the native code and update package_version.txt in iModelNodeAddon.
2. *Adopt the native platform* -- Change the imodeljs-native-platform-api.ts file in imodeljs-core/core/backend and change TypeScript code as necessary to react to API changes.

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

Before requesting a PRG build, you must update the version number of the native platform package(s). The package version number for the implementation of the native platform is stored in one place, in the file:
```
iModelJsNodeAddon/package_version.txt
```

The native platform's version number is a standard, 3-part semantic version number. Update according to semver rules.

BentleyBuild parts in iModelJsNodeAddon read the version number from this file and inject it into the native platform binaries and into the generated native platform package.json files.

FYI The native platform package version number is also burned into the native code. This allows imodeljs-backend to do a version-compatibility check at runtime. It is not necessary to to burn in a new version number as part of your testing. If for some reason you want to do this, you must re-build like this after changing package_version.txt:

``` cmd
bb -s "iModelJsNodeAddon;BuildAll" re iModelJsNodeAddonLib* MakePackages -c
bb -s "iModelJsNodeAddon;BuildAll" re iModelJsNodeAddonLib* MakePackages
```

## 3. Publish

Request a PRG build of the native platform packages. Specify the new version number in your request. Wait for the result.

# Testing Native Platform Changes and Corresponding Backend Changes

You test the native platform by calling it from TypeScript.

Install your local build of the native platform. There is a `installNativePlatform` script in this directory for each supported platform.

Update imodeljs-core/core/backend/imodeljs-native-platform-api.ts to reflect the changes made to the API implemented by native code.

Update .ts files in imodeljs-core/core/backend as necessary to react to changes in the native platform API.

Run TypeScript tests and sample apps.

# Adopting a New Version of the Native Platform packages

After the native platform packages have been built by PRG and published, you can update imodeljs-backend to depend on the new version.

The native platform version number appears in only once place in all of imodeljs, in `imodeljs-core/core/backend/package.json`. So, to move imodeljs-backend to a new version of the native platform, edit this file and specify the new version number.

Then, rush update and rush build.

Make sure tests are still passing.

Push.

# How to Move to a New Version of Node

If you need to support a newer version of node, do the following:

If you haven't already, install node-gyp:
`npm install -g node-gyp`

Tell node-gyp to install the version of nodejs that you want. Suppose, for example, you want to install v10.13.0. You would do this:
```
node-gyp install 10.13.0
```

That will install headers and libs to a directory called `%homedrive%%homepath%\.node-gyp\10.13.0`

Copy the following files from there into `%SrcRoot%\thirdparty\node-addon-api\napi-v10`:
* include\node\node_api.h
* include\node\node_api_types.h
* x64\node.lib

# How to move to a new version of the napi headers

If you need a newer version of napi.h
```
npm install -g node-addon-api
```

That will install the napi package in the `%homedrive%%homepath%\AppData\Roaming\npm\node_modules\node-addon-api` directory.

Copy the following files from there into `%SrcRoot%\thirdparty\node-addon-api\napi-v10`:
* napi.h
* napi-inl.h
* napi-inl.deprecated.h

# How to Move to a New Version of Electron

If you need to support a newer version of Electron, do the following:

Tell node-gyp to install the version of electron that you want. For example, to install version 2.0.8, do this:
```
node-gyp install --target=2.0.8 --arch=x64 --dist-url=https://atom.io/download/electron
```

That will install the lib you need to the `%homedrive%%homepath%\.node-gyp\iojs-2.0.8` directory.

Copy *only* the following file from there into `%SrcRoot%\thirdparty\node-addon-api\napi-v10`:
* x64\iojs.lib
