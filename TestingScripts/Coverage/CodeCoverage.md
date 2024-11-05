# Code Coverage

## Development setup

### Steps

- Change the build strategy in the env.bat file to iModelCoreCoverage

- Run `bb build` to generate the code coverage report for all the different components of iModelCore

- You can also generate the code coverage reports of a single component by specifing the part in the `bb` command. Follow `Coverage.PartFile.xml` for more details on indiviual parts.

### Steps to generate excel report

- Change the build strategy in the env.bat file to iModelCoreCoverage

- Run `bb build` to generate the code coverage report for all the different components of iModelCore

- You can also generate the code coverage reports of a single component by specifing the part in the `bb` command. Follow `Coverage.PartFile.xml` for more details on indiviual parts.

- To generate excel report for a particular part just don't pass any `BentleyBuildMakeOptions` in the specific part of the `Coverage.PartFile.xml` part file.

- To generate excel report you will have to first define `Win10SdkDir` in your shared shell everytime. e.g. set Win10SdkDir=C:\Program Files (x86)\Windows Kits\10\ [This might differ, just we need to find where dbh.exe exists in our machine. For dbh.exe to be present we need to have `Windows Software Development Kit` installed with Windows Debugger checked].

- To generate excel report we also need a python package named `XlsxWriter`. The quick command to install it would be `pip install XlsxWriter`. For more details visit [here](https://pypi.org/project/XlsxWriter/)

### Note - TODO:

Currently the test coverage we get is for files and for the number of lines. We can also extend the behaviour so that we get coverage also for the number of methods. The logic for that is not yet implemented properly and needs looking into in subsequent time.

Copyright (c) Bentley Systems, Incorporated. All rights reserved.
