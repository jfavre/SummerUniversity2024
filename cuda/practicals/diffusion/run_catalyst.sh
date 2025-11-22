#!/bin/bash -l
#SBATCH --job-name=HeatDiffusion+Catalyst
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=1
#SBATCH --gpus-per-node=1
#SBATCH --ntasks=1
#SBATCH --time=00:09:59
#SBATCH --account=csstaff
#SBATCH --partition=normal
#SBATCH --uenv=paraview/6.0.1:2156754389
#SBATCH --view=default

#SBATCH --cpus-per-task=64
#SBATCH --hint=nomultithread

export CATALYST_IMPLEMENTATION_NAME=paraview

mkdir -p $SCRATCH/insitu/datasets
cp $PWD/buildCatalyst/bin/diffusion2d $SCRATCH/insitu/diffusion2d_catalyst
cp $PWD/catalyst_pipeline.py $SCRATCH/insitu
pushd $SCRATCH/insitu

srun --cpus-per-task=64 ./diffusion2d_catalyst 8 40001 catalyst_pipeline.py
