#!/usr/bin/env bash

#SBATCH --array=0-3999
#SBATCH --job-name=rtp
#SBATCH --output=output/output-%A_%a.txt
#SBATCH --mem=800MB
#SBATCH --time=0-4:0:0
#SBATCH --nodes=1
#SBATCH --cpus-per-task=1
#SBATCH --partition=main

export EXPERIMENT_NAME="${today}/${SLURM_JOB_NAME}" 

rate_reset_values=( 0.01 0.62 1.23 1.85 2.46 3.07 3.68 4.29 4.91 5.52 6.13 6.74 7.35 7.97 8.58 9.19 9.8 10.41 11.03 11.64 12.25 12.86 13.47 14.09 14.7 15.31 15.92 16.54 17.15 17.76 18.37 18.98 19.6 20.21 20.82 21.43 22.04 22.66 23.27 23.88 24.49 25.1 25.72 26.33 26.94 27.55 28.16 28.78 29.39 30.0 )
rate_switch_values=( 1.0 2.0 3.0 4.0 5.0 )
init_pos_values=( 0.01 0.1 0.25 0.5 )
reset_pos_values=( 0.01 0.1 0.25 0.5 )

trial=${SLURM_ARRAY_TASK_ID}
rate_reset=${rate_reset_values[$(( trial % ${#rate_reset_values[@]} ))]}
trial=$(( trial / ${#rate_reset_values[@]} ))
rate_switch=${rate_switch_values[$(( trial % ${#rate_switch_values[@]} ))]}
trial=$(( trial / ${#rate_switch_values[@]} ))
init_pos=${init_pos_values[$(( trial % ${#init_pos_values[@]} ))]}
trial=$(( trial / ${#init_pos_values[@]} ))
reset_pos=${reset_pos_values[$(( trial % ${#reset_pos_values[@]} ))]}

export OMP_NUM_THREADS=1
export OMP_PROC_BIND=spread

## use ${rate_reset}, ${rate_switch}, ${init_pos}, ${reset_pos} below

mkdir -p results/${EXPERIMENT_NAME}
./bin/rtp_sim ${rate_reset} ${rate_switch} --v 2.0 --init_pos ${init_pos} --reset_pos ${reset_pos} --num_simulations 20000

# After all array tasks complete, submit the dependent job
if [ "${SLURM_ARRAY_TASK_ID}" -eq "${SLURM_ARRAY_TASK_MAX}" ]; then
    # Wait for all other array tasks to complete
    # Get the parent job ID without the array task ID
    parent_jobid=${SLURM_ARRAY_JOB_ID}
    
    # Submit the dependent job with dependency on the entire array
fi
