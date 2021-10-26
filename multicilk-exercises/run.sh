export CILK_CONFIG1="nworkers=4;cpuset=0,1,2,3"
export CILK_CONFIG2="nworkers=4;cpuset=4,5,6,7"
./$1 $2

