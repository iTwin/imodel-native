The TypeScript API exposed by the ecschema-ops addon package.

#Tests
The tests subdirectory contains tests written in TypeScript. These are tests of the methods exported by the ecschema-ops addon. They are written in TypeScript both for convenience and to better mimic the way ecschema-ops will see and use the addon's API.

To run these scripts, just run node.exe lib/test/index.js

To debug the native code, the developer must run node.exe under the debugger.

*Note:* To make it simple to launch the tests under the debugger and debug the native code, these scripts and the native code run in a single process. Therefore, these scripts do not use mocha or any other test framework.
