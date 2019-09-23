Purpose of the tests in this folder is to create a high load on the presentation manager to help catch any random
deadlocks or crashes. The tests don't measure any performance metrics, so they can't be used for regression testing.
Also, take a lot of time, so they aren't supposed to be run as part of the build on a regular basis.