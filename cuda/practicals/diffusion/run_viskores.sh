#!/bin/bash -l
#SBATCH --job-name=HeatDiffusion+Viskores
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=1
#SBATCH --gpus-per-node=1
#SBATCH --ntasks=1
#SBATCH --time=00:09:59
#SBATCH --account=csstaff
#SBATCH --partition=debug
#SBATCH --uenv=prgenv-gnu/25.6:v1
#SBATCH --view=default

#SBATCH --cpus-per-task=64
#SBATCH --hint=nomultithread

#module load cray-mpich cuda/q2
export LD_LIBRARY_PATH=`spack location -i cuda@12.8.1`/lib64:$LD_LIBRARY_PATH

mkdir $SCRATCH/datasets/Viskores
pushd $SCRATCH/datasets/Viskores
rm    $SCRATCH/datasets/Viskores/viskores*png

srun --cpus-per-task=64 /users/jfavre/Projects/SummerUniversity2024/cuda/practicals/diffusion/buildViskores/bin/diffusion2d 8 15001 
