#!/bin/bash

PROGRAM="./part2"
OXYGEN_ATOMS=100
NITROGEN_ATOMS=100
NUM_RUNS=10
ACTIVATION_ENERGIES=(0 20 40 60 80 100)

# Remove previous data files
rm -f activation_energy_results.txt

echo "ActivationEnergy_NO2 ActivationEnergy_O3 Avg_NO2_Reactions Avg_O3_Reactions" > activation_energy_results.txt

for AENO2 in "${ACTIVATION_ENERGIES[@]}"; do
    for AEO3 in "${ACTIVATION_ENERGIES[@]}"; do
        TOTAL_NO2=0
        TOTAL_O3=0
        for (( run=1; run<=NUM_RUNS; run++ )); do
            OUTPUT_FILE="output_${AENO2}_${AEO3}_run${run}.txt"
            $PROGRAM $OXYGEN_ATOMS $NITROGEN_ATOMS $AENO2 $AEO3 > $OUTPUT_FILE

            # Count the number of NO2 and O3 reactions
            NO2_COUNT=$(grep -c "produce two molecules of NO2" $OUTPUT_FILE)
            O3_COUNT=$(grep -c "produce two molecules of O3" $OUTPUT_FILE)

            TOTAL_NO2=$((TOTAL_NO2 + NO2_COUNT))
            TOTAL_O3=$((TOTAL_O3 + O3_COUNT))

            # Clean up the output file
            rm -f $OUTPUT_FILE
        done

        # Calculate average reactions over runs
        AVG_NO2=$((TOTAL_NO2 / NUM_RUNS))
        AVG_O3=$((TOTAL_O3 / NUM_RUNS))

        # Write results to file
        echo "$AENO2 $AEO3 $AVG_NO2 $AVG_O3" >> activation_energy_results.txt
    done
done

echo "Testing complete. Results saved in activation_energy_results.txt"

