#!/bin/bash -l
#SBATCH --job-name=HeatDiffusion+Ascent
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
module load hdf5/1.14.6-q54kcdk
module load libfabric/1.22.0-sw5dkak

mkdir -p $SCRATCH/insitu/datasets
cp $PWD/buildAscent-cuda/bin/diffusion2d $SCRATCH/insitu/diffusion2d_ascent
pushd $SCRATCH/insitu

srun --cpus-per-task=64 ./diffusion2d_ascent 8 40001
