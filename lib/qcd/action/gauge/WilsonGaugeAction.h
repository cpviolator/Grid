/*************************************************************************************

Grid physics library, www.github.com/paboyle/Grid

Source file: ./lib/qcd/action/gauge/WilsonGaugeAction.h

Copyright (C) 2015

Author: Azusa Yamaguchi <ayamaguc@staffmail.ed.ac.uk>
Author: Peter Boyle <paboyle@ph.ed.ac.uk>
Author: neo <cossu@post.kek.jp>
Author: paboyle <paboyle@ph.ed.ac.uk>
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
#ifndef QCD_WILSON_GAUGE_ACTION_H
#define QCD_WILSON_GAUGE_ACTION_H

namespace Grid {
namespace QCD {

////////////////////////////////////////////////////////////////////////
// Wilson Gauge Action .. should I template the Nc etc..
////////////////////////////////////////////////////////////////////////
template <class Gimpl>
class WilsonGaugeAction : public Action<typename Gimpl::GaugeField> {
 public:  
  INHERIT_GIMPL_TYPES(Gimpl);

  /////////////////////////// constructors
  explicit WilsonGaugeAction(RealD beta_):beta(beta_){};

  virtual std::string action_name() {return "WilsonGaugeAction";}

  virtual std::string LogParameters(){
    std::stringstream sstream;
    sstream << GridLogMessage << "[WilsonGaugeAction] Beta: " << beta << std::endl;
    return sstream.str();
  }

  virtual void refresh(const GaugeField &U,
                       GridParallelRNG &pRNG){};  // noop as no pseudoferms

  virtual RealD S(const GaugeField &U) {
    RealD plaq = WilsonLoops<Gimpl>::avgPlaquette(U);
    RealD vol = U._grid->gSites();
    RealD action = beta * (1.0 - plaq) * (Nd * (Nd - 1.0)) * vol * 0.5;
    return action;
  };

  virtual void deriv(const GaugeField &U, GaugeField &dSdU) {
    // not optimal implementation FIXME
    // extend Ta to include Lorentz indexes

    RealD factor = 0.5 * beta / RealD(Nc);

    GaugeLinkField Umu(U._grid);
    GaugeLinkField dSdU_mu(U._grid);
    for (int mu = 0; mu < Nd; mu++) {
      Umu = PeekIndex<LorentzIndex>(U, mu);

      // Staple in direction mu
      WilsonLoops<Gimpl>::Staple(dSdU_mu, U, mu);
      dSdU_mu = Ta(Umu * dSdU_mu) * factor;

      PokeIndex<LorentzIndex>(dSdU, dSdU_mu, mu);
    }
  }
private:
  RealD beta;  
 };

    //DMH
    //Added G.C.'s 5D HMC    
    template <class Gimpl>
      class WilsonGaugeAction5D : public Action<typename Gimpl::GaugeField> {
    public:
      INHERIT_GIMPL_TYPES(Gimpl);
      
    private:
      std::vector<RealD> b_bulk;    // bulk couplings
      std::vector<RealD> b_fifthD;  // extra dimension couplings
      GridBase *grid;
      LatticeComplex beta_fifthD;
      LatticeComplex beta_fifthD_shifted;
      LatticeComplex beta_bulk;
      
      int bulk_volume;
      
    public:
      
      virtual std::string action_name() {return "WilsonGaugeAction5D";}
      
      virtual std::string LogParameters(){
	std::stringstream sstream;
	//sstream << GridLogMessage << "[WilsonGaugeAction5D] BetaBulk: " << b_bulk   << std::endl;
	//sstream << GridLogMessage << "[WilsonGaugeAction5D] Beta5D:"    << b_fifthD << std::endl;
	return sstream.str();
      }
      
      explicit WilsonGaugeAction5D(std::vector<RealD> bulk, std::vector<RealD> fifthD,
				   GridBase *_grid)
	: b_bulk(bulk),
        b_fifthD(fifthD),
        grid(_grid),
        beta_fifthD(grid),
        beta_fifthD_shifted(grid),
        beta_bulk(grid) {

	  //DMH? Required
	  int Ndim = 5;
	  
	  std::vector<int> FullDim = grid->GlobalDimensions();
	  bulk_volume = 1;
	  for (int s = 0; s < Ndim - 1; s++) {	    
	    bulk_volume *= FullDim[s];
	  }
	  
	  LatticeComplex temp(grid);
	  
	  Lattice<iScalar<vInteger> > coor(grid);
	  
	  LatticeCoordinate(coor, Ndim - 1);

	  //Number of slices in extra dim
	  int Nex = FullDim[Ndim - 1];
	  assert(b_bulk.size() == Nex);
	  assert(b_fifthD.size() == Nex);

	  //populate extra dim beta in all slices
	  beta_fifthD = zero;
	  for (int tau = 0; tau < Nex; tau++) {
	    temp = b_fifthD[tau];
	    beta_fifthD = where(coor == tau, temp, beta_fifthD);
	  }
	  
	  beta_fifthD_shifted = Cshift(beta_fifthD, Ndim - 1, -1);
	  
	  beta_bulk = zero;
	  for (int tau = 0; tau < Nex; tau++) {
	    temp = b_bulk[tau];
	    beta_bulk = where(coor == tau, temp, beta_bulk);
	  }
	};
      
     
      
      virtual void refresh(const GaugeField &U,
			   GridParallelRNG &pRNG){};  // noop as no pseudoferms
      
      virtual RealD S(const GaugeField &Umu) {

	int Ndim = 5;  //DMH? Required
	conformable(grid, Umu._grid);
	
	std::vector<GaugeLinkField> U(Ndim, grid);
      
	for (int mu = 0; mu < Ndim; mu++) {
	  U[mu] = PeekIndex<LorentzIndex>(Umu, mu);
	}
	
	LatticeComplex dirPlaq(grid);
	LatticeComplex Plaq(grid);
	
	LatticeComplex SumdirPlaq(grid);
	
	RealD OneOnNc = 1.0 / Real(Nc);
	
	/////////////
	// Lower dim plaquettes
	/////////////
	Plaq = zero;
	SumdirPlaq = zero;
	for (int mu = 1; mu < Ndim - 1; mu++) {
	  for (int nu = 0; nu < mu; nu++) {
	    WilsonLoops<Gimpl>::traceDirPlaquette(dirPlaq, U, mu, nu);
	    SumdirPlaq += dirPlaq;
	    Plaq = Plaq + (1.0 - dirPlaq * OneOnNc) * beta_bulk;
	  }
	}

    
	double faces = (1.0 * (Ndim - 1) * (Ndim - 2)) / 2.0;
	SumdirPlaq *= OneOnNc / (RealD(bulk_volume) * faces);

	// print slices in the extra dimension
	int Nex = grid->_fdimensions[Ndim - 1];
	std::vector<TComplex> plaq_ex(Nex);
	sliceSum(SumdirPlaq, plaq_ex, Ndim - 1);
	for (int ex = 0; ex < Nex; ex++)
	  std::cout << GridLogMessage << "Bulk plaq[" << ex
		    << "] = " << TensorRemove(plaq_ex[ex]).real() << std::endl;

	/////////////
	// Extra dimension
	/////////////
	{
	  int mu = Ndim - 1;
	  for (int nu = 0; nu < mu; nu++) {
	    WilsonLoops<Gimpl>::traceDirPlaquette(dirPlaq, U, mu, nu);
	    //DMH
	    //std::cout << "nu=" << nu << " beta=" << beta_fifthD << std::endl;
	    Plaq = Plaq + (1.0 - dirPlaq * OneOnNc) * beta_fifthD;
	  }
	}
	
	TComplex Tp = sum(Plaq);
	Complex p = TensorRemove(Tp);
	RealD action = p.real();
	
	return action;
      };
      
      virtual void deriv(const GaugeField &U, GaugeField &dSdU) {
	// not optimal implementation FIXME
	// extend Ta to include Lorentz indexes

	// G.C.
	// for the higher dimension plaquettes take the upper plaq of the
	// 4d slice and multiply by beta[s] and the lower and multiply by beta[s-1]
	// derivative of links mu = 0, ... Nd-1 inside plaq (mu,5)
	// for these I need upper and lower staples separated
	// each multiplied with their own beta
	// derivative of links mu = 5
	// living on the same slice, share the same beta

	conformable(grid, U._grid);
	//int Ndim = Nd;  // change later
	int Ndim = 5;     //DMH
	RealD factor = 0.5 / RealD(Nc);
    
	GaugeLinkField Umu(grid);
	GaugeLinkField dSdU_mu(grid);
	GaugeLinkField staple(grid);

	//Loop over a direction
	for (int mu = 0; mu < Ndim; mu++) {
	  Umu = PeekIndex<LorentzIndex>(U, mu);
	  dSdU_mu = zero;

	  //Loop over an orthogonal direction
	  for (int nu = 0; nu < Ndim; nu++) {
	    if (nu != mu && mu <Ndim-1) {
	      
	      if ((mu < (Ndim - 1)) && (nu < (Ndim - 1))) {
		// Spacelike case apply beta space
		WilsonLoops<Gimpl>::Staple(staple, U, mu, nu);
		staple = staple * beta_bulk;
		dSdU_mu += staple;

	      } else if (mu == (Ndim - 1)) {
		// nu space; mu time link
		assert(nu < (Ndim - 1));
		assert(mu == (Ndim - 1));

		// mu==tau dir link deriv, nu spatial
		WilsonLoops<Gimpl>::Staple(staple, U, mu, nu);
		staple = staple * beta_fifthD;
		dSdU_mu += staple;

	      } else {
		assert(mu < (Ndim - 1));
		assert(nu == (Ndim - 1));

		// nu time; mu space link

		// staple forwards in tau
		WilsonLoops<Gimpl>::StapleUpper(staple, U, mu, nu);
		staple = staple * beta_fifthD;
		dSdU_mu += staple;

		// staple backwards in tau
		WilsonLoops<Gimpl>::StapleLower(staple, U, mu, nu);
		staple = staple * beta_fifthD_shifted;
		dSdU_mu += staple;
	      }
	    }
	  }

	  dSdU_mu = Ta(Umu * dSdU_mu) * factor;
	  PokeIndex<LorentzIndex>(dSdU, dSdU_mu, mu);
	}
      };
    };
    

}
}

#endif
