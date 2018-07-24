#!/bin/bash -l                                                                                                                                                                   

LX=4
LY=4
LZ=4
LT=4
LS=15

MPIX=2
MPIY=2
MPIZ=2
MPIT=1
MPIS=1

BETA=4.80

TRAJ=10
THERM=10
LOG='Integrator'
START_TYPE='CheckpointStart'

COMMAND="mpirun -np 8 ./Test_hmc_WilsonGauge 4.80 --grid ${LX}.${LY}.${LZ}.${LT}.${LS} --mpi ${MPIX}.${MPIY}.${MPIZ}.${MPIT}.${MPIS} 
	        			          --Trajectories ${TRAJ} --Thermalizations ${THERM} --log ${LOG} "
#                                                  --StartingTrajectory 10 --StartingType ${START_TYPE}"

echo ""
echo "Command given:"
echo ${COMMAND}
echo ""

${COMMAND}
