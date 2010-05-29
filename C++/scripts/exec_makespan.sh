#!/bin/bash
#Executa os testes la$1.prb ate la$2.prb, $3 vezes cada, passando ao programa o melhor makespan conhecido

exec="./Ateams"

if [ $1 -le $2 ];
then
    for((j = 0; j < $3; j++));
    do
        for((i = $1; i <= $2; i++));
        do
	    k=`sed -n -e ${i}p resultados/bestMakespan.lawrence`
            if [ $i -le 9 ];
            then
                echo "$exec -i dados/la0$i.prb --makespanBest $k $4 $5 $6 $7 $8 $9"
                $exec -i dados/la0$i.prb --makespanBest $k $4 $5 $6 $7 $8 $9
            else
                echo "$exec -i dados/la$i.prb --makespanBest $k $4 $5 $6 $7 $8 $9"
                $exec -i dados/la$i.prb --makespanBest $k $4 $5 $6 $7 $8 $9
            fi
        done
    done
else
    for((j = 0; j < $3; j++));
    do
        for((i = $1; i >= $2; i--));
        do
	    k=`sed -n -e ${i}p resultados/bestMakespan.lawrence`
            if [ $i -le 9 ];
            then
                echo "$exec -i dados/la0$i.prb --makespanBest $k $4 $5 $6 $7 $8 $9"
                $exec -i dados/la0$i.prb --makespanBest $k $4 $5 $6 $7 $8 $9
            else
                echo "$exec -i dados/la$i.prb --makespanBest $k $4 $5 $6 $7 $8 $9"
                $exec -i dados/la$i.prb --makespanBest $k $4 $5 $6 $7 $8 $9
            fi
        done
    done
fi
