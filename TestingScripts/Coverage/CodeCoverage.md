# Code Coverage

## Development setup

### Steps

- Change the build strategy in the env.bat file to iModelCoreCoverage

- Run `bb build` to generate the code coverage report for all the different components of iModelCore

- You can also generate the code coverage reports of a single component by specifing the part in the `bb` command. Follow `Coverage.PartFile.xml` for more details on indiviual parts.

### Steps to generate excel report

- Change the build strategy in the env.bat file to iModelCoreCoverage

- To generate excel report you will have to first define `generateExcel` in your shared shell everytime. e.g. set generateExcel=1.

- To generate excel report we also need a python package named `XlsxWriter`. The quick command to install it would be `pip install XlsxWriter`. For more details visit [here](https://pypi.org/project/XlsxWriter/)

- Run `bb build` to generate the code coverage report for all the different components of iModelCore

- You can also generate the code coverage reports of a single component by specifing the part in the `bb` command. Follow `Coverage.PartFile.xml` for more details on indiviual parts.

### Special Note

`LastCoverageResults.log` files might be generated when we run code coverage. `DONOT FORGET TO REMOVE THEM BEFORE COMMITING` if they are generated.These files give us an overview of the overall coverage.

### Note - TODO:

Currently the test coverage we get is for files and for the number of lines. We can also extend the behaviour so that we get coverage also for the number of methods. The logic for that is not yet implemented properly and needs looking into in subsequent time.

Copyright (c) Bentley Systems, Incorporated. All rights reserved.
