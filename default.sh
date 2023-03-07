#! /bin/bash
TIMEOUT=$1

cd vprexocompiler &&
g++ -std=c++11 src/compiler.cpp -o bin/compiler &&

echo -e "\n\n+------------------------------+" &&
echo -e "| Generating c file... |" &&
echo -e "+------------------------------+\n" &&
./bin/compiler ./data/find_min &&
./bin/compiler ./data/merging &&
./bin/compiler ./data/binary_search &&
./bin/compiler ./data/kmp_search &&
./bin/compiler ./data/next_permutation &&
./bin/compiler ./data/dutch_flag &&
./bin/compiler ./data/rr_sequence_find &&
./bin/compiler ./data/sum_of_powers &&
./bin/compiler ./data/td_convex_hull &&
./bin/compiler ./data/msc &&
cd .. &&
cp vprexocompiler/data/*_ti.c pepper/skeletons/ &&
cp vprexocompiler/data/*_te.c pepper/skeletons/ &&

echo -e "\n\n+-------------------------+" &&
echo -e "| Testing in Pequin... |" &&
echo -e "+-------------------------+\n" &&
cd pepper &&
python3 exec.py --all &&
python3 latex_generator.py