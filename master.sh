cd projects/SEED

# Compile
g++-4.8 -std=c++11 func/nvn.cpp -o NvN2.0 -l boost_program_options
g++-4.8 -DNDEBUG -std=c++11 func/model.cpp -o AvR2.2 -l boost_program_options

# Run full AvR model
cd res; runid=3; mkdir $runid; cd $runid
batch-job ../../AvR2.2

# Generate code for TACC
rid=0
for aspan in 2 10 100; do
for jspan in 0.5 2 5; do
for a in 0.2 0.5 0.8; do
for theta in 2 5 10; do
for lag in 2 5 10; do
for E in 10 50 100; do
for U in 0 0.5 1; do
for gr in 0 0.2 0.857; do
echo AvR2.2 --aspan $aspan --jspan $jspan --a $a --theta $theta --lag $lag --E $E --U $U --gr $gr --rid $rid >> seed_commands.sh;
rid=$(($rid+1));
done
done
done
done
done
done
done
done


# On TACC
module load launcher
sbatch launcher.slurm




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


