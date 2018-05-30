# @bentley/imodeljs-native-platform-api

<p style="text-align: center;">
<b><i><mark>
<mark><em>The native platform and its API are for the private use of imodeljs-backend only.</em></mark>
</b></i></mark>
</p>


The iModelJs native library for node contains native code that is projected into JavaScript and is loaded by nodejs or electron. The @bentley/imodeljs-native-platform-api package declares the classes, methods, and properties -- the API -- that the native platform implements.

The native platform and its API declaration are in separate packages. In fact, there are many native platform packages, one for each combination of node version and target platform that is supported. For example, @bentley/imodeljs-n_8-win32-x64 is the version of the native platform that can be used on a Windows desktop machine running 64-bit node v8.x.x. In addition, some apps may build custom versions of the native platform. There is only one API package.

imodeljs-backend depends directly on imodeljs-native-platform-api, as it is built to expect a particular native platform API. The native platform that is loaded and used at runtime must implement the expected API. imodeljs-backend checks compatibility at run time.

Ultimately, it is up to an app to deliver and load a native platform at run time. The loaded native platform must implement at least the API that imodeljs-backend requires.

# iModelJsNodeAddon

The native platform and its API are not in git. The code for the native platform and the declaration of the API are in the hg repository called iModelJsNodeAddon. The native platform is built by BentleyBuild.

The native platform and API packages are generated. See the iModelJsNodeAddon::MakePackages part.

The generated native platform and API packages are published by PRG using BentleyBuild.

The native platform API is defined by imodeljs-native-platform-api.d.ts. When you write native platform code, you define JS classes and methods using N-API. You must also declare the classes and methods in the imodeljs-native-platform-api.d.ts file. Yes, you must define the API twice -- once in native code and again in the d.ts file -- there is nothing to make this automatic or to check consistency -- you must do this manually.

imodeljs-native-platform-api.d.ts contains the (internal) documentation for the native platform. This README file is also part of the native platform API package, and it contains these build instructions.

The package version number for the native platform and its API are stored in the file iModelJsNodeAddon/package_version.txt. Parts in iModelJsNode read the package version from this file and inject it into the native platform binaries and into the native platform and API packages.

# Changing and Publishing the Addon

**Key point:** The native platform packages and the API package must all be published with the same version number.
**Key point:** The native platform and imodeljs-native-platform-api.d.ts must be in sync.

## Build

You can build the native platform like this:

`bb -s"iModelJsNodeAddon;BuildAll" b`

To force a rebuild of individual parts, do this:

`bb -s"iModelJsNodeAddon;BuildAll" re DgnPlatformDLL <other libraries...> iModelJsNodeAddonLib* MakePackages`

## Version

The package version number for the native platform and its API are stored in the file iModelJsNodeAddon/package_version.txt. This is used to generate the packages. So, you must update the version number in this file when you change the native platform. Specifically:

Increment the *first digit* of the version if you remove or modify the signature of any existing class or method, or if you otherwise change the contract of an existing method.

Increment the *second digit* of the version if you add new classes, methods, or properties to the native platform API.

Increment the *third digit* if you merely fix a bug or otherwise change the implementation of the native platform in a way that does not affect the API or the contract of any method.

If you change only the documentation content in the d.ts or README files, then increment the third digit of the package version.

## Update the Declarations

If you change the native platform's API in native code, e.g., by adding a method, you must also update imodeljs-native-platform-api.d.ts to reflect the change.

## Burn in the Version

You must rebuild the native platform itself and the packages whenever you change the version. Note that the version number is *burned into the code*. This allows imodeljs-backend to do its runtime version-compatibility check. So, it's not enough just to regenerate the packages -- you must also rebuild the native platform. After changing the version number, re-build like this:

```
bb "iModelJsNodeAddon;BuildAll" re iModelJsNodeAddonLib* MakePackages -c
bb "iModelJsNodeAddon;BuildAll" re iModelJsNodeAddonLib* MakePackages
```

## Test

To test changes to the native code before publishing, you can install your local build of the API and the native platform into your local build of imodeljs-core and run unit tests.

When you bb build the MakePackages part,

`bb -s"iModelJsNodeAddon;BuildAll" re MakePackages`

That will print the location of the generated packages. For example:

``` bat
%OutRoot%Winx64\packages\imodeljs-e_2-win32-x64
%OutRoot%Winx64\packages\imodeljs-n_8-win32-x64
%OutRoot%Winx64\packages\imodeljs-native-platform-api
```

That message identifies the (generated) npm packages that contain the native platform, as well as the API package. This example is from a Windows build. The messages from a Linux or MacOS build will be similar, but will show names that are specific to those platforms. Note MakePackages produces several packages from the same source. In the case of a Windows build, there is one native platform package for node apps and another for electron apps. The API package is the same for both.

Continuing this example, suppose your imodeljs git repository is here:

```
\imjs\imodeljs-core
```

On Windows you would install your local build of the native platform by using the `installnativeplatform.bat` file in the root directory of imodeljs-core.
There are similar .sh scripts for Linux and MacOS.

Note: to get back to a clean install, do this:
```
rush install -c
```

To test other platforms or other versions, use the names of the generated packages that are displayed by the MakePackages part.

# Publishing the native platform

The native platform packages are published by PRG, not by developers.

### How imodeljs-backend Checks Version Compatibility

imodeljs-core/backend/AddonRegistry.registerAddon verifies that the loaded native platform implements the API that imodeljs-backend expects. There are three tests: 1) The native platform and the API must be the same generation (same major version). 2) The native platform must include all of the classes and methods that the backend expects and may include new classes that the backend is not yet using (same or greater minor version). And, 3) the native platform must include all required bug fixes and may include more recent bug fixes (same or higher patch version).

# APPENDIX: How to Move to a New Version of Node

The native platform is still specific to a major.minor version of nodejs. (That will change after we move to node v9 next year.) If you need to support a newer version of node, do the following:

### Get new node-gyp package, if necessary

node-gyp contains the node header files and the .lib for the desired version. You do not always have to update node-gyp when you update node.

If you haven't already, install node-gyp:
`npm install -g node-gyp`

Tell node-gyp to install the version of nodejs that you want.
`node-gyp install <nodevernum>`

That will install the headers and libs to the .node-gyp directory in your %homedrive%%homepath% directory. Copy the files from the relevant subdirectory to thirdparty\node-addon-api\node-gyp

### Update thirdparty/nodejs/node-gyp, if necessary

If you updated node-gyp from npm, then you must update our saved copy of it, as follows:

1. Copy that whole directory to `%SrcRoot%thirdparty\node-addon-api\node-gyp`, creating a new subdirectory with the same name as the origin.
2. Rename the new subdirectory by prefixing it with "N_" and replacing dots with underscores.

For example, suppose you want to update to 8.9.2 for node addons. You would copy the `%homedrive%%homepath%.node-gyp\8.9.2` to `%SrcRoot%thirdparty\node-addon-api\node-gyp`. That would create a subdirectory called `8.9.2`. You would then rename the copy to `n_8_2`.

### Change Partfiles

Finally, change the PartFiles that refer to the version of the node API that is used to build the native platform.

thirdparty\node-addon-api\node-addon-api.PartFile.xml
iModelJsNodeAddon\iModelJsNodeAddon.PartFile.xml