#!/bin/bash

exec="./Ateams"

for((i = $1; i <= $2; i++));
do
    if [ $i -le 9 ];
    then
        $exec -i dados/la0$i.prb $4 $5 $6 $7 $8 $9
    else
        $exec -i dados/la$i.prb $4 $5 $6 $7 $8 $9
    fi
done