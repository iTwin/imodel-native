# @bentley/imodeljs-native

Rolling a new version of the native platform code is a two-step process:
1. *Change the native platform* -- Change the native code and update package_version.txt in iModelNodeAddon.
2. *Adopt the native platform* -- Change the IModelJsNative.ts file in imodeljs-core/core/backend and change TypeScript code as necessary to react to API changes.

These steps are described in more detail in the sections below.

# Changing and Publishing the Native Platform Packages

## 1. Change, Build and Test

Change the C++ files in iModelJsNodeAddon. Use the apis in <Napi\napi.h>

Build the native platform like this:

`bb -s"iModelJsNodeAddon;BuildAll" b`

To force a rebuild of individual parts, do this:

`bb -s"iModelJsNodeAddon;BuildAll" re DgnPlatformDLL iModelJs*

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
bb -s "iModelJsNodeAddon;BuildAll" re iModelJs*  -c
bb -s "iModelJsNodeAddon;BuildAll" re iModelJ*
```

## 3. Publish

Request a PRG build of the native platform packages. Specify the new version number in your request. Wait for the result.

# Testing Native Platform Changes and Corresponding Backend Changes

You test the native platform by calling it from TypeScript.

Install your local build of the native platform. There is a `installNativePlatform` script in this directory for each supported platform.

Update imodeljs/core/backend/IModelJsNative.ts to reflect the changes made to the API implemented by native code.

Update .ts files in imodeljs-core/core/backend as necessary to react to changes in the native platform API.

Run TypeScript tests and sample apps.

# Adopting a New Version of the Native Platform packages

After the native platform packages have been built by PRG and published, you can update imodeljs-backend to depend on the new version.

The native platform version number appears in only once place in all of imodeljs, in `imodeljs/core/backend/package.json`. So, to move imodeljs-backend to a new version of the native platform, edit this file and specify the new version number.

Then, rush update and rush build.

Make sure tests are still passing.

Push.

# Rules to Check in Code Reviews

*	Don’t make N-API calls or JS callbacks during JS GC. 
    * The only time that our addon is in danger of doing something that it should not while JavaScript GC is in progress is when our native objects are being destroyed.
    * Every subclass of ObjectWrap that we write must have a destructor, and the first line of the destructor must be this macro: OBJECT_WRAP_DTOR_DISABLE_JS_CALLS. 
    * Don’t deliberately invoke JS callbacks in a destructor.
    * A destructor can invoke native logging functions but note that output will be deferred.
    * If in doubt, call JsInterop::IsJsExecutionDisabled. The will return true (JS is disabled) in the scope of the RAII object created by OBJECT_WRAP_DTOR_DISABLE_JS_CALLS.

* Don’t invoke JS callbacks while JS exceptions are pending.
    * This can happen only in native code that makes a series of N-API calls or JS callbacks or if it deliberately throws a JS Error.
    * Code that throws a JS Error should return immediately. The argument-checking macros do this for you.
    * Code that makes a series of N-API calls or JS callbacks must check JsInterop::IsJsExecutionDisabled before each one.
    * An Napi::AsyncWorker subclass can assume that the context is valid when its OnOK, OnError, and OnComplete callbacks are invoked by N-API.
    * If in doubt, call JsInterop::IsJsExecutionDisabled. That will return true (JS is disabled) when a JS exception is pending.

* Don’t invoke JS callbacks or N-API functions in other threads.
    * This might be a danger if you add Napi calls or JS callbacks to helper classes.
    * Remember that ASyncWorker::OnExecute is invoked in a background thread. Don’t try to access JS or N-API.
    * If in doubt, call JsInterop::IsJsExecutionDisabled. That will return true (JS is disabled) for all but the main thread.
    * We have not vetted and have no experience with the so-called “threadsafe” portion of N-API. If you want to pioneer this, then please do a careful job and write tests to prove that it is safe. Nothing is harder to debug that threading errors.

* Object Lifetime management
    * If you are just implementing a native method or an ASyncWorker callback, you should not have to worry about handle scopes. N-API manages this for you.
    * If you write code that is not invoked directly by N-API and that does create JS objects or make callbacks, you must read and understand this:  https://nodejs.org/api/n-api.html#n_api_object_lifetime_management
    * If you write code that holds JS object references in C++ object member variables or in static globals, you must read and understand this: https://nodejs.org/api/n-api.html#n_api_references_to_objects_with_a_lifespan_longer_than_that_of_the_native_method
    * See the comment on the SET_CONSTRUCTOR macro.
