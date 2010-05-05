#!/bin/bash
#Executa os testes la$1.prb ate la$2.prb, $3 vezes cada

exec="./Ateams"

if [ $i -le $2 ];
then
    for((j = 0; j < $3; j++));
    do
        for((i = $1; i <= $2; i++));
        do
            if [ $i -le 9 ];
            then
                echo "$exec -i dados/la0$i.prb $4 $5 $6 $7 $8 $9"
                $exec -i dados/la0$i.prb $4 $5 $6 $7 $8 $9
            else
                echo "$exec -i dados/la$i.prb $4 $5 $6 $7 $8 $9"
                $exec -i dados/la$i.prb $4 $5 $6 $7 $8 $9
            fi
        done
    done
else
    for((j = 0; j < $3; j++));
    do
        for((i = $1; i >= $2; i--));
        do
            if [ $i -le 9 ];
            then
                echo "$exec -i dados/la0$i.prb $4 $5 $6 $7 $8 $9"
                $exec -i dados/la0$i.prb $4 $5 $6 $7 $8 $9
            else
                echo "$exec -i dados/la$i.prb $4 $5 $6 $7 $8 $9"
                $exec -i dados/la$i.prb $4 $5 $6 $7 $8 $9
            fi
        done
    done
fi
