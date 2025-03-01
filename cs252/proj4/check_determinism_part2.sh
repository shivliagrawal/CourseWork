#!/bin/bash

PROGRAM="./part2"
NUM_RUNS=10
OXYGEN_ATOMS=10
NITROGEN_ATOMS=10

# Remove previous outputs
rm -f part2_output_*.txt

# Run the program multiple times
for i in $(seq 1 $NUM_RUNS); do
    $PROGRAM $OXYGEN_ATOMS $NITROGEN_ATOMS > part2_output_$i.txt
done

# Compare outputs
IDENTICAL=1
FIRST_OUTPUT="part2_output_1.txt"

for i in $(seq 2 $NUM_RUNS); do
    DIFF_OUTPUT=$(diff $FIRST_OUTPUT part2_output_$i.txt)
    if [ "$DIFF_OUTPUT" != "" ]; then
        IDENTICAL=0
        echo "Difference found between run 1 and run $i"
    fi
done

if [ $IDENTICAL -eq 1 ]; then
    echo "All outputs are identical. The program is deterministic."
else
    echo "Outputs differ between runs. The program is nondeterministic."
fi

