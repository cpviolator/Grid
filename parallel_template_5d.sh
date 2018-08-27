#!/bin/bash -l

L=4
LX=${L}
LY=${L}
LZ=${L}
LT=${L}
LS=15

MPIX=2
MPIY=2
MPIZ=2
MPIT=1
MPIS=1

BETA=4.95

#seed must be five, space separated integers
SEED_SERIAL="2 5 3 8 9"
SEED_PARALLEL="6 91 2 12 98"

TRAJ=20
THERM=10
RECORD=10
START=10

HMC_STEP_SIZE=20
HMC_TRAJ_LENGTH=1.0

LOG='Integrator'
START_TYPE='CheckpointStart'

COMMAND="mpirun -np 8 ./Test_hmc_WilsonGauge ${SEED_SERIAL} ${SEED_PARALLEL} 
		      			     ${BETA} {RECORD} ${HMC_STEP_SIZE} 
					     ${HMC_TRAJ_LENGTH}
					     --grid ${LX}.${LY}.${LZ}.${LT}.${LS} 
					     --mpi ${MPIX}.${MPIY}.${MPIZ}.${MPIT}.${MPIS} 
	        	       		     --Trajectories ${TRAJ} 
					     --Thermalizations ${THERM} 
					     --log ${LOG} "
#                                                  --StartingTrajectory ${START} --StartingType ${START_TYPE}"

echo ""
echo "Command given:"
echo ${COMMAND}
echo ""

${COMMAND}
