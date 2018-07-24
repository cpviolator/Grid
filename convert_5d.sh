#!/bin/bash -l                                                                                                                                                                   

LX=8
LY=8
LZ=8
LT=8
LS=5

MPIX=1
MPIY=1
MPIZ=1
MPIT=1
MPIS=1

BETA=5.60

TRAJ=10
THERM=10
LOG='Integrator'
START_TYPE='CheckpointConvert'

COMMAND="./Test_hmc_WilsonGauge ${BETA} --grid ${LX}.${LY}.${LZ}.${LT}.${LS} --mpi ${MPIX}.${MPIY}.${MPIZ}.${MPIT}.${MPIS} 
	  	       		        --Trajectories ${TRAJ} --Thermalizations ${THERM} --log ${LOG} 
                                        --StartingTrajectory 10 --StartingType ${START_TYPE}"

echo ""
echo "Command given:"
echo ${COMMAND}
echo ""

${COMMAND}
