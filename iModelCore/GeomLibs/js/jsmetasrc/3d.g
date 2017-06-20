3DONLY\W(<BalancedText>)=@{$1}
2DONLY\W(<BalancedText>)=
SELECT_2D3D\W(<BalancedText>)\W(<BalancedText>)=@{$2}

(3DONLY\W<BalancedText>)=@{$1}
(2DONLY\W<BalancedText>)=

BalancedText:(#)=$0
BalancedTExt:\[#\]=$0
BalancedText:\{#\}=$0
BalancedText:?=?

