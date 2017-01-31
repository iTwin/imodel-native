!
! Extract external names defs  from object files as reformatted
! by dumpbin/symbols.
!

! Look for the special line that tells the object file name...
\nDump of file <U>\n=@set{FileName;$1}

! SECTnnn tells who we export ....
\L SECT<d> notype External \| <I>\N=$2\=@var{FileName}\n
\L SECT<d> notype () External \| <I>\N=$2\=@var{FileName}\n
?=