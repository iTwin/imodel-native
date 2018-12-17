!
! pipe the output of "cvs log -rDEV_wip:" to this gema script in order to strip away
! files showing only one revision.
!
! The block of text for a single file in a cvs log stream starts with "RCS file" and ends with a line of equals.
! Capture each such block and pass it FINDMULTIREV:
\NRCS file<T>\N\=\=\=\=\=\=\=\=\=\=\==@FINDMULTIREV{$0}
?=

! A file that has only a single revision in the cvs log stream has exactly one line of dashes.
! If there are two or more revisions (i.e. two or more lines of dashes) send it through.
! Swallow everything else.
FINDMULTIREV:<T>\n---------------------<T>\n--------------------<T>=\n\n$0
FINDMULTIREV:?=
