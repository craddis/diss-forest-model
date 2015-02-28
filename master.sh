cd projects/SEED

# Compile
g++-4.8 -std=c++11 func/nvn.cpp -o NvN2.0 -l boost_program_options
g++-4.8 -std=c++11 func/model.cpp -o AvR2.0 -l boost_program_options

# Run full AvR model
cd res; runid=1; mkdir $runid; cd $runid
batch-job ../../AvR2.0 --A 40 --lag 10 --steps 100 --years 100 --tol 0.12 --a 0.5 --aspan 10 --jspan 3 --E 100 --f 1.0







# Extras
cd res; runid=2
for lag in 10; do
for aspan in 100; do
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


