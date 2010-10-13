#!/bin/bash

exec="./Ateams"
num=$#
rep=`eval echo '${'$num'}'`


echo


if [ -e $rep ]
then
    rep=1
else
    num=$(($num-1))
fi


echo "Number Of Instances: $num"
echo "Repetitions: $rep"

echo
eval "make install"
echo


for((i = 1; i <= $num; i++));
do
    dado='${'$i'}'

    for((j = 0; j < $rep; j++));
    do
        eval "echo Running: $dado"
        eval "$exec -i $dado"
    done
done
