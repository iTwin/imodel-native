3DONLY\W(<BalancedText>)=@{$1}
2DONLY\W(<BalancedText>)=
SELECT_2D3D\W(<BalancedText>)\W(<BalancedText>)=@{$2}

BalancedText:(#)=$0
BalancedTExt:\[#\]=$0
BalancedText:\{#\}=$0
BalancedText:?=?

