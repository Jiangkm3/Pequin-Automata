#! /bin/bash
cd vprexocompiler &&
g++ -std=c++11 src/compiler.cpp -o bin/compiler &&

echo -e "\n\n+------------------------------+" &&
echo -e "| Generating c file... |" &&
echo -e "+------------------------------+\n" &&
echo -e "--\nCompiling find_min:"
./bin/compiler ./data/find_min &&
echo -e "--\nCompiling merging:"
./bin/compiler ./data/merging &&
echo -e "--\nCompiling binary_search:"
./bin/compiler ./data/binary_search &&
echo -e "--\nCompiling kmp_search:"
./bin/compiler ./data/kmp_search &&
echo -e "--\nCompiling next_permutation:"
./bin/compiler ./data/next_permutation &&
echo -e "--\nCompiling dutch_flag:"
./bin/compiler ./data/dutch_flag &&
echo -e "--\nCompiling rr_sequence_find:"
./bin/compiler ./data/rr_sequence_find &&
echo -e "--\nCompiling sum_of_powers:"
./bin/compiler ./data/sum_of_powers &&
echo -e "--\nCompiling 2d_convex_hull:"
./bin/compiler ./data/td_convex_hull &&
echo -e "--\nCompiling msc:"
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