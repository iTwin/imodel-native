## How to generate results
Generating the results can be done overnight if the work is split between multiple processes. Usually,
splitting the work in 4 parts is sufficient to run all tests in a reasonable time. To do so, use the -p
option of GCoordApproxTester.

1. Open four terminals in the directory where GCoordApproxTester is built.
2. In the first terminal, enter the command "GCoordApproxTester -p 1 4" (meaning part 1 of 4).
3. Start parts 2, 3 and 4 in the other terminals, using the same command, but replacing the 1 by the number of the part you want to start.
4. Wait a few hours to let the four processes the time to complete. This will generate two .csv files for each process: a result file and an error file.

## How to analyse results
The .csv file generated are too large to be merged and opened in excel. The best way to have results all in one place is
to import them into a database. The simplest way to do that is to use Microsoft Access, has it is already installed part of
the office suite. In Access, import the four .csv file one by one into the same table. Then, it is possible (altough it is slow)
to sort the entire DB according to a column or to run SQL querries on the database.
