#!/bin/env bash

#SBATCH --array=0-3124
#SBATCH --job-name=rtp-test
#SBATCH --output=output/output-%j.txt
#SBATCH --mem=1800MB
#SBATCH --time=0-0:5:0
#SBATCH --nodes=1
#SBATCH --cpus-per-task=1
#SBATCH --partition=main

rate_reset_values=( 0.1 1.35 2.59 3.84 5.08 6.33 7.57 8.82 10.07 11.31 12.56 13.8 15.05 16.3 17.54 18.79 20.03 21.28 22.52 23.77 25.02 26.26 27.51 28.75 30.0 )
rate_switch_values=( 1.0 2.0 3.0 4.0 5.0 )
init_pos_values=( 0.1 0.25 0.5 0.75 0.99 )
reset_pos_values=( 0.1 0.25 0.5 0.75 0.99 )

trial=${SLURM_ARRAY_TASK_ID}
rate_reset=${rate_reset_values[$(( trial % ${#rate_reset_values[@]} ))]}
trial=$(( trial / ${#rate_reset_values[@]} ))
rate_switch=${rate_switch_values[$(( trial % ${#rate_switch_values[@]} ))]}
trial=$(( trial / ${#rate_switch_values[@]} ))
init_pos=${init_pos_values[$(( trial % ${#init_pos_values[@]} ))]}
trial=$(( trial / ${#init_pos_values[@]} ))
reset_pos=${reset_pos_values[$(( trial % ${#reset_pos_values[@]} ))]}



trial=${SLURM_ARRAY_TASK_ID}
rate_reset=${rate_reset_values[$(( trial % ${#rate_reset_values[@]} ))]}
trial=$(( trial / ${#rate_reset_values[@]} ))
rate_switch=${rate_switch_values[$(( trial % ${#rate_switch_values[@]} ))]}


## use ${rate_reset}, ${rate_switch} below

mkdir -p results/${SLURM_JOB_NAME}/

EXPERIMENT_NAME=${SLURM_JOB_NAME} ./bin/rtp_sim --v 5.0 --num_simulations 10000 --init_pos ${init_pos} --reset_pos ${reset_pos} ${rate_reset} ${rate_switch}
