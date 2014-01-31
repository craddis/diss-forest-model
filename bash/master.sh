python func/gen_bash.py
g++-4.8 -std=c++11 func/model.cpp
bash/runs.sh
find res/ -maxdepth 1 -type f -name "*p" -exec Rscript func/process.R {} \;
