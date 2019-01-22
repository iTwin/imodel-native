# Napi

Napi is a C++ api for writing version-independent Node addons. The C++ api is a header-only implementation (see `Napi/napi.h`), that depends (only) on a pure-C set of functions defined in `Napi/node_api.h` each with the `napi_` prefix.

Generally, C++ programs include `<Napi/napi.h>` and don't directly access the C functions. Node implements and exports the C functions in a .lib file under Windows and exports them from the node executable under Unix.

## napi_stub.cpp

The file `napi_stub.cpp` exists for two reasons:

- We sometimes want to *stub out* Node when we write tests, or when we use our dlls in an environment where we don't need JavaScript
- On Windows, Electron exports the `napi_` symbols differently than Node does (from `electron.exe` instead of `node.exe`), and the electron executable is also usually renamed when distributing applications.

For this reason, rather than link all of our .dlls (that have a JavaScript dependency) with the Node/Electron delivered node.lib file we link our .dlls with a new "napi.dll" created from `napi_stub.cpp`. Then, napi.dll *forwards* the references to the `napi_xxx` symbols to the appropriate place. That way we don't have to have 2 versions of all of our .dlls that use the "Napi" header files.

## How `napi_stub.cpp` resolves the `napi_xxx` symbols

There are 2 ways that `napi_stub.cpp` resolves the `napi_xxx` symbols:

1. from the executable that loaded the addon (usually either `node.exe` or `electron.exe`).
2. from stubs when we run tests, or other places where we don't want a dependence on Node.

To complicate things further, the first case above is only relevant for Windows, because the Unix loader does not specify the source for an import. But, we still have the 2nd case under Unix.

In `node-addon-api.mke`, we compile `napi_stub.cpp` twice, first with no macros defined, then with `BUILD_FOR_NODE` define. The first build generates the .lib file that we use to link our .dlls under Windows, and the .a file we link with under Unix. When we compile for Node we *forward* all the exported symbols to `node.exe` (that's what the `FORWARD_NAPI_EXPORT` lines do).
Also, on Windows, we register a [delay-load hook](https://docs.microsoft.com/en-us/cpp/build/reference/notification-hooks?view=vs-2017) to redirect any `node.exe` references to use the loading executable. This way, the addon will still work even if the host executable is `electron.exe` or is renamed to anything else. For the "stub" build we simply implement each function to do nothing. That generates 2 different `napi.dll` files in our BuildContext.

## Making PartFiles that use the `node-addon-api` PartFile

1. Every Part that depends on Napi should include `napi-lib` as a SubPart.

2. The Parts that build tests should deliver the stub napi.dll by using `napi-stub` as a SubPart.

3. The BentleyBuild `<Product>`s that include `imodeljs.node` should:

Under Windows:

- Deliver the Node version of napi.dll via the `napi-dll-win` Part.

Under Unix:

- Do nothing. The imodeljs.node ".so" merely contains external references to these symbols that are resolved at load time. Unix doesn't specify the name of the module where the symbols are implemented, so we don't have to specify anything and there is no Part for napi.dll (because it comes from the .a generated in step 1.)

## Updating to a new version of Node

When we update to a new version of Node, unless they change something in Napi, there are **no changes** required. If they *do* happen to change/add/remove symbols in napi:

 1. copy the new header files into the Napi subdirectory and .lib files into the node subdirectory.
 2. the file `Napi/node_api.h` has the names/signatures of all the functions. Ensure that the signatures of the stubs in `napi_stubs.cpp` agree exactly.
 3. for the "forwarding" sections, it may be easier to do a `dumpbin /exports node.lib` from the Node delivery, then find all the symbols with the "napi_" prefix.