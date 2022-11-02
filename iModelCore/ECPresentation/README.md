# ECPresentation Library

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