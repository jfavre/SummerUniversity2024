#!/bin/bash -l
#SBATCH --job-name=VTKm-HeatDiffusion
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=1
#SBATCH --ntasks=1
#SBATCH --time=00:09:59
#SBATCH --account=csstaff
#SBATCH --partition=debug
#SBATCH --uenv=insitu_ascent/0.9.5:2109123735@daint
#SBATCH --view=modules
#SBATCH --cpus-per-task=64
#SBATCH --gpus-per-task=1
#SBATCH --hint=nomultithread

module load ascent/0.9.5-5tvprek
module load cray-mpich/8.1.32-fvq4yfa
module load cuda/12.8.1-fel3gie
module load gcc/13.4.0-yrhdyox
module load libfabric/1.22.0-sw5dkak

mkdir -p $SCRATCH/insitu/datasets
cp $PWD/buildVTKm/bin/diffusion2d $SCRATCH/insitu/diffusion2d_vtkm
pushd $SCRATCH/insitu

srun --cpus-per-task=64 ./diffusion2d_vtkm 8 40001
