for i in {1..8}
do
    ./simplexomp $i < input.dat >> saida_omp_100x100;
    ./simplexthreads $i < input.dat >> saida_threads_100x100;  
done
