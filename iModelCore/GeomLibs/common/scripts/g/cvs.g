!
! gema patterns to pluck out cvs header C-style comment block
! (the one with at least 4 instances of "$")
!

\/\**\*\/=@FilterCVSHeader{$0}
?=

FilterCVSHeader:\/\**\$*\$*\$*\$*\*\/=$0\n
FilterCVSHeader:?=
