!
! gema patterns that strip C/C++/Java comments
! DA 7/98
!

! Leave comment blocks inside strings
\"<t>\/\*<t>\*\/<t>\"=$0
\"<t>\/\/<t>\"=$0
\'<t>\/\*<t>\*\/<t>\'=$0
\'<t>\/\/<t>\'=$0

! C comments sandwiched by text
\/\*<t>\*\/<T>\N=$2

! C comments ending a line
\/\*<t>\*\/\W\N=

! C block comments sandwiched by newlines
\N\W\/\*<t>\*\/\W\n=

! java/C++ comments ending a line
\W\/\/<t>\N=

?=?