cd projects/SEED

# Compile
g++-4.8 -std=c++11 func/nvn.cpp -o NvN2.0 -l boost_program_options
g++-4.8 -DNDEBUG -std=c++11 func/model.cpp -o SEED-2.3 -l boost_program_options
g++-4.8 -DNDEBUG -std=c++11 func/model.cpp -lboost_program_options -o exec/SEED-2.3.1 

# Run full AvR model
cd res; runid=3; mkdir $runid; cd $runid
batch-job ../../AvR2.2

# Generate code for TACC
rid=1000
for aspan in 2 10 100; do
for jspan in 0.5 2 5; do
for a in 0.2 0.5 0.8; do
for theta in 2 5 10; do
for lag in 2 5 10; do
for E in 10 50 100; do
for gr in 0 0.2 0.857; do
echo ./SEED-2.3.1 --aspan $aspan --jspan $jspan --a $a --theta $theta --lag $lag --E $E --gr $gr --rid $rid
rid=$(($rid+1));
done
done
done
done
done
done
done | split -d -a1 -l2176 - sweep

scp sweep* addi416@stampede.tacc.utexas.edu:~

# One extra run on antarctica to even out nodes
cd res
../SEED-2.3 --aspan 100 --jspan 5 --a 0.8 --theta 10 --lag 10 --E 100 --U 1 --gr 0.857 --rid 7560

# Syntax for batch-job time SEED...
batch-job '{ time exec/SEED-2.3.1 --rid 100;} 2> time.100'


# On TACC
cdh
module load git
git pull ...
mv sweep* $WORK/
module load gcc
module load boost
gcc -DNDEBUG -std=c++11 seed/func/model.cpp -I$TACC_BOOST_INC -L$TACC_BOOST_LIB -lboost_program_options -o $WORK/SEED-2.3

cdw
module load launcher
sbatch launcher0.slurm
sbatch launcher1.slurm
sbatch launcher2.slurm
sbatch launcher3.slurm
squeue -u addi416



# Extras

 mkdir $runid
 cd $runid
 mkdir init
 batch-job ../../SEED1.1 --lag $lag --aspan $aspan
 cd ..
 run_id=$(($run_id+1))
done
done	
cd ..


g++-4.8 -std=c++11 func/network.cpp -o network -larmadillo
cd res/5; ../../network; cd ../..








# Wait until jobs complete
find res/ -maxdepth 1 -type f -name "*p" -exec Rscript func/process.R {} \;
for run_id in {1..175}; do python func/graph.py res/"$run_id"tf; done
find res/ -maxdepth 1 -type f -name "*tb" -exec python func/graph.py {} \;
find res/ -maxdepth 1 -type f -name "*v" -exec python func/graph.py {} \;

rm doc/agg_params
touch doc/agg_params
echo -e "run_id m n N lag aspan jspan baby mama papa" >> doc/agg_params
for run_id in {176..187}; do 
	pfile=res/"$run_id"p
	echo "$run_id"$(cat $pfile | tr -d '\n' | sed 's/[a-z,=,N]//g') >> doc/agg_params
done 

g++-4.8 -std=c++11 func/join_count.cpp
for run_id in {176..187}; do ./a.out $run_id > res/"$run_id"jc; done
for run_id in {176..187}; do ./a.out $run_id >> doc/join_count; done

./SEED1.1


