# ECPresentation Library

## Development setup

### Recommended tools

We recommend using Visual Studio for the library development. To help with the setup, there's a solution file `ECPresentation.sln` next to this README. Projects in the solution use BentleyBuild environment variables to detect include paths, so Visual Studio has to be started from shell after running the `env.bat` file to set up environment.

### Recommended build commands

Generally, when making ECPresentation library changes it's not necessary to always build everything. Below is the list of build commands that should help
developers avoid building too much.

- `bb re -s ecpresentation-gtest` builds our library tests and all their dependencies, including the library itself. Use when building tests for the first time.

- `bb re -s imodeljsmakepackages` builds the imodeljs-native addon and all its dependencies, including our library. Use when building addon for the first time.

- `bb re ecpresentation-library` builds just our library. Use after making source code changes to our library.
  - When building for native tests, using this command to rebuild the library is enough when only source (\*.cpp) files are changed. When there are changes in headers (\*.h) included in the tests, it's recommended to also remove `{platform out root}/build/ECPresentation/UnitTests-NonPublished` and rebuild the tests.

  - When build for native addon, the following are necessary to re-create the addon package:
    ```
    bb re -c imodeljsmakepackages
    bb re iModelJsNative-Dynamic iModelJsMakePackages iModelJsApiDeclarations
    ```

- It's recommended to use "Rebuild" command on a test project after making test changes to rebuild the tests.

BentleyBuild notes:

- `re` is an alias for `rebuild`.
- The `-s` option means "also build dependencies".

It's always recommended to do a clean build after pulling changes, unless you're sure they're not significant and won't affect the result.

### Visual Studio projects setup

The Visual Studio solution (`ECPresentation.sln` next to this README) contains 4 projects:

- `ECPresentation` contains source code for the library itself.
- `ECPresentationTests` contains source code for unit and integration tests.
- `ECPresentationTests-Performance` contains source code for performance tests. **This is not part of CI build.**
- `ECPresentationTests-Stress` contains source code for stress tests. **This is not part of CI build.**

Project command meanings:
- **Build** builds the project and all its dependencies.
- **Rebuild** rebuilds just the project, but not its dependencies. Will fail if dependencies aren't built beforehand.
- **Clean** removes build output of the project (doesn't touch dependencies).

## Developer guidelines

### Logging

All logging should be done using the `Diagnostics` APIs.

We got 2 types of logs:

- **Editor** - logs for someone who writes presentation rules.
- **Dev** - logs for a developer who works on ECPresentation library.

Here are some general advice on choosing the right logging severity:

Editor severity | When to use
----------------|------------------------------------------------------------------------
ERROR           | Error that prevents us from producing a correct result.
WARN            | Likely (but not necessarily) and error, something that makes us produce and invalid result.
INFO            | Something that influences the end result, but is not an error.

Dev severity    | When to use
----------------|------------------------------------------------------------------------
ERROR           | Something that indicates a bug in the library. Results in an assertion failure, email notification to library devs. Since this is our bug, we should still attempt to produce at least a partial result.
WARN            | Something unusual, but doesn't prevent us from producing valid results.
INFO            | High level logs like request parameters, resulting values or performance information. Shouldn't be used excessively to avoid affecting performance.
DEBUG           | Lower level logs about stuff that substantially affects results. Might be used a lot. Might reduce library performance, but not as much as TRACE.
TRACE           | Lowest level logs. Could be used to log a message for every code branch. Substantially reduces library performance.

Guidelines on choosing the right **dev** severity when logging **editor** logs:

Editor severity | Dev severity
----------------|----------------
ERROR           |  DEBUG / INFO / WARN
WARN            |  DEBUG
INFO            |  TRACE / DEBUG

Copyright (c) Bentley Systems, Incorporated. All rights reserved.
