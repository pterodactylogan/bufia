#!/bin/sh

#SBATCH --partition=short
#SBATCH --job-name=bufia_slurm_test
#SBATCH --output=bufia_nmp_slurm_test.log
#SBATCH --nodes=1 
#SBATCH --ntasks=4
#SBATCH --cpus-per-task=8 ### 48 threads (cores) per rank (node)

module load slurm
module load fujitsu/compiler/4.8
export OMP_NUM_THREADS=1
export OMP_DISPLAY_ENV=true
export OMP_DISPLAY_AFFINITY=true
export OMP_AFFINITY_FORMAT="Thread Affinity: %0.3L %.8n %.15{thread_affinity} %.12H"
export OMP_PROC_BIND=true

FCC /lustre/projects/heinz-group/bufia/bufia/cpp/bufia_init_utils.cc /lustre/projects/heinz-group/bufia/bufia/cpp/bufia_algorithm_utils.cc /lustre/projects/heinz-group/bufia/bufia/cpp/factor.cc /lustre/projects/heinz-group/bufia/bufia/cpp/bufia.cc

export OMP_PLACES=cores

time ./a.out /lustre/projects/heinz-group/bufia/bufia/sanity/parupa.features.csv /lustre/projects/heinz-group/bufia/bufia/sanity/parupa600.train
