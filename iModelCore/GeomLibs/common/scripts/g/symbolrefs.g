!
! Extract external names references from object files as reformatted
! by dumpbin/symbols.
!

! Look for the special line that tells the object file name...
\nDump of file <U>\n=@set{FileName;$1}

! UNDEF tells who we call from outside ....
\L UNDEF <U> \| <I>\N=@var{FileName} $2\n

?=