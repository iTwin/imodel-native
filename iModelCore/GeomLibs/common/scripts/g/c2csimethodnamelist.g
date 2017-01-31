!
! Pluck out c-to-csharp renames
!

@define{@read{@getenv{GEMADIR}g\\domains.g}}
@define{@read{@getenv{GEMADIR}g\\needswork.g}}

!                           $1=csName   $2=comments        #3=retType #4=cName $5=arglist $6=body
\/\*\WCSIMETHOD\W(\W<I>\W)\W\*\/<u>\n\WPublic <I> <I>\W(<arglist>)=\
\\B\=\@FuncToIMethod@ArgListTypeCharacter{$5}\{$4,$1\}\n

\/\*\WCSIMETHOD_\W(\W<I>\W)\W\*\/<u>\n\WPublic <I> <I>\W(<arglist>)=\
\\B\=\@FuncToIMethod@ArgListTypeCharacter{$5}\{$4,$1\}\n

?=

ArgListTypeCharacter:<u>,<u>=B@end
ArgListTypeCharacter:<u>=A@end
