/*************************************************************************************

Grid physics library, www.github.com/paboyle/Grid

Source file: ./tests/Test_hmc_WilsonFermionGauge.cc

Copyright (C) 2015

Author: Peter Boyle <pabobyle@ph.ed.ac.uk>
Author: neo <cossu@post.kek.jp>
Author: Guido Cossu <guido.cossu@ed.ac.uk>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

See the full license in the file "LICENSE" in the top level distribution
directory
*************************************************************************************/
/*  END LEGAL */
#include <Grid/Grid.h>

int main(int argc, char **argv) {
  using namespace Grid;
  using namespace Grid::QCD;

  Grid_init(&argc, &argv);
  GridLogLayout();

   // Typedefs to simplify notation
  typedef GenericHMCRunner<MinimumNorm2> HMCWrapper;  // Uses the default minimum norm
  HMCWrapper TheHMC;

  // DMH: Construct a Five Dim Grid
  // Grid from the command line
  TheHMC.Resources.AddFiveDimGrid("gauge");
  // Possibile to create the module by hand 
  // hardcoding parameters or using a Reader

  // DMH: Get information about the 5D grid.
  auto UGrid = TheHMC.Resources.GetCartesian("gauge");
   
  RNGModuleParameters RNGpar;
  for(int i=0; i<5; i++) {
    RNGpar.serial_seeds   += std::to_string(atoi(argv[i])) + " ";
    RNGpar.parallel_seeds += std::to_string(atoi(argv[i+5])) + " ";
  }
  
  TheHMC.Resources.SetRNGSeeds(RNGpar);
  
  RealD Beta = 4.95; //DMH: A default value
  Beta = atof(argv[11]);

  // Checkpointer definition
  CheckpointerParameters CPparams;  
  CPparams.config_prefix = "ckpoint_lat";
  CPparams.rng_prefix = "ckpoint_rng";
  CPparams.saveInterval = atoi(argv[12]);
  CPparams.format = "IEEE64BIG";
  
  //DMH: Binary data is simpler to hack.
  TheHMC.Resources.LoadBinaryCheckpointer(CPparams);
  
  // Construct observables
  // here there is too much indirection 
  typedef PlaquetteMod<HMCWrapper::ImplPolicy> PlaqObs;
  TheHMC.Resources.AddObservable<PlaqObs>();

  //Topology done in 4D code
  /*
  typedef TopologicalChargeMod<HMCWrapper::ImplPolicy> QObs;
  TopologyObsParameters TopParams;
  TopParams.interval = 5;
  TopParams.do_smearing = false; //DMH: We do physics on the 4D slices
  TopParams.Smearing.steps = 200;
  TopParams.Smearing.step_size = 0.01;
  TopParams.Smearing.meas_interval = 50;
  TopParams.Smearing.maxTau = 2.0; 
  TheHMC.Resources.AddObservable<QObs>(TopParams);
  */
  //////////////////////////////////////////////

  // Gauge action
  int Ls = UGrid->_fdimensions[4]; //DMH: Get the 5D extent from the Grid.  
  std::vector<RealD> betat(Ls,Beta);
  std::vector<RealD> betas(Ls,Beta);
  
  betat[Ls-1] = 0.0;  // for the open boundary conditions
  std::cout << GridLogMessage << "Betas: " << betas << std::endl;
  std::cout << GridLogMessage << "Betat: " << betat << std::endl;
  WilsonGaugeAction5DR Waction(betas, betat, UGrid);
  
  ActionLevel<HMCWrapper::Field> Level1(1);
  Level1.push_back(&Waction);
  //Level1.push_back(WGMod.getPtr());
  TheHMC.TheAction.push_back(Level1);
  /////////////////////////////////////////////////////////////
  
  // HMC parameters are serialisable 
  TheHMC.Parameters.MD.MDsteps = atoi(argv[13]);
  TheHMC.Parameters.MD.trajL   = atof(argv[14]);

  TheHMC.ReadCommandLine(argc, argv); // these can be parameters from file

  TheHMC.Run();  // no smearing
  
  Grid_finalize();

} // main
