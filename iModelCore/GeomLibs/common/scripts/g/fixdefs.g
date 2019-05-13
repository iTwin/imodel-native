\/\/include\W(<T>)\W\n=$0@out{@fixup{@read{@REROUTE{$1}}}}
\/\*include\W(<T>)\W\*\/\W\n=$0@out{@fixup{@read{@REROUTE{$1}}}}
?=?


REROUTE:include=methods
REROUTE:\/msj\/=@getenv{MSJ}
REROUTE:?=?

! Miscellaneous code fixups.
! For mjava, reinstate the jmdl_const and null conversions.
! (But that will never be needed. Right?)
!
fixup:\N \/\*\*\N=\N\
\/\*-------------------------------------------------------------------------\*\/\/\*\*
!fixup:\Iconst\I=jmdl_const
fixup:bsihdr=bsimethod
!fixup:NULL=null
fixup:?=?