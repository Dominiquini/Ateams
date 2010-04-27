#!/bin/bash
#Executa o teste $1, $2 vezes cada, variando o parametro $3 de $4 ate $5 com incremento $6

exec="./Ateams"

for((j = $4; j <= $5; j+=$6));
do
    for((i = 0; i < $2; i++));
    do
        echo "$exec -i dados/la$1.prb $3 $j $7 $8 $9 -r resultados/$3_$j.res"
        $exec -i dados/la$1.prb $3 $j $7 $8 $9 -r resultados/$3_$j.res
    done
done
