! If the first thing in the file is a comment containing "opyright",
! rip it out and replace by a new notice:
\B\W\/\*<u>opyright<u>\*\/=@read{@getenv{GEMADIR}\\data\\copyright.txt}
! Otherwise just insert one...
\B=@read{@getenv{GEMADIR}\\data\\copyright.txt}

! We know the new notice contains some CVS keywords, so we'll rip them
! out anywhere else..........
\L\$Logfile\:<u>\$=
\L\$Workfile\:<u>\$=
\L\$Source\:<u>\$=
\L\$RCSfile\:<u>\$=
\L\$Revision\:<u>\$=
\L\$Date\:<u>\$=
\L\$Author\:<u>\$=
?=?