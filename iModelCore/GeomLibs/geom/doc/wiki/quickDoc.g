\/\/\W\@q<u>\n=@Block{\@q$1\n}

Block:\@q<I><u>\P\@q=@Fragment{\@q$1 $2\@_end}
Block:\@q<I><u>\P\n=@Fragment{\@q$1 $2\@_end}
Block:\n=
Block:\L<u>=(Block $0)

Fragment:\@qH1\I<U>\@_end=\n\n\! $1\n
Fragment:\@qH2 <U>\@_end=\n\n\!\! $1\n
Fragment:\@qH3\I<U>\@_end=\n\n\!\!\! $1\n

Fragment:\@qD\W\@_end=\n\|\| \|\|
Fragment:\@qD\I<U>\@_end=\n\|\| $1 \|\|
Fragment:\@qX\W\@_end= \|\|
Fragment:\@qX\I<U>\@_end= $1 \|\|

Fragment:\W\@qW\I<U>\@_end=\nWarning\: $1

Fragment:<u>=(Fragment $0)\n
\#ifdef flexwiki<u>\#endif=$1

\L\/\/flex\!<u>=\n\n\!$1\n\n
\L\/\/flex <I>\:<u>=\n\n$1\:$2\n
\L\/\/flex <u>=\n$1
\L\/\/flex\I<u>=\n$1

?=
