/*
 * SolverCurvedFace.cpp
 *
 *  Created on: Mar 21, 2013
 *      Author: luis
 */

#ifndef SOLVERCURVEDFACE_H_
#include "DGCurvedFace.h"
#endif

DGCurvedFace::DGCurvedFace() {
	solverPosition = 0;
	impPAv = 0.0;
	admPAv = 0.0;
	imp1Av = 0.0;
	adm1Av = 0.0;
	rhsEx = NULL;
	rhsEy = NULL;
	rhsEz = NULL;
	rhsHx = NULL;
	rhsHy = NULL;
	rhsHz = NULL;
	dEx = NULL;
	dEy = NULL;
	dEz = NULL;
	dHx = NULL;
	dHy = NULL;
	dHz = NULL;
	dresEx = NULL;
	dresEy = NULL;
	dresEz = NULL;
	dresHx = NULL;
	dresHy = NULL;
	dresHz = NULL;
}

DGCurvedFace::DGCurvedFace(
 const CellTet<ORDER_N>* cell,
 const uint face,
 const uint solverPosition_,
 FieldD3& rhsE, FieldD3& rhsH,
 const FieldD3& dE, const FieldD3& dH,
 const FieldD3& dresE, const FieldD3& dresH,
 const double impP_,
 const double admP_,
 const double impAv_,
 const double admAv_) {
	solverPosition = solverPosition_;
	// Pointers to RHS and jumps --------------------------------------
	uint pos = solverPosition_ * np;
	rhsEx = &rhsE.set(x)[pos];
	rhsEy = &rhsE.set(y)[pos];
	rhsEz = &rhsE.set(z)[pos];
	rhsHx = &rhsH.set(x)[pos];
	rhsHy = &rhsH.set(y)[pos];
	rhsHz = &rhsH.set(z)[pos];
	pos = solverPosition_ * nfp * 4 + face * nfp;
	dEx = &dE(x)[pos];
	dEy = &dE(y)[pos];
	dEz = &dE(z)[pos];
	dHx = &dH(x)[pos];
	dHy = &dH(y)[pos];
	dHz = &dH(z)[pos];
	dresEx = &dresE(x)[pos];
	dresEy = &dresE(y)[pos];
	dresEz = &dresE(z)[pos];
	dresHx = &dresH(x)[pos];
	dresHy = &dresH(y)[pos];
	dresHz = &dresH(z)[pos];
	// Impedances, admitances -----------------------------------------
	impPAv = impP_ / impAv_ / 2.0;
	admPAv = admP_ / admAv_ / 2.0;
	imp1Av = 1.0 / impAv_ / 2.0;
	adm1Av = 1.0 / admAv_ / 2.0;
	// Computes LIFT matrices for curved elements ---------------------
	StaMatrix<double,np,nfp> LIFTn[3], LIFTcn[3], LIFTrn[3];
	cell->getCurvedLIFTnormal(LIFTn, LIFTcn, LIFTrn, face);
	LIFTn[0].convertToArray(MATRICES_ROW_MAJOR, nx);
	LIFTn[1].convertToArray(MATRICES_ROW_MAJOR, ny);
	LIFTn[2].convertToArray(MATRICES_ROW_MAJOR, nz);
	LIFTcn[0].convertToArray(MATRICES_ROW_MAJOR, cnx);
	LIFTcn[1].convertToArray(MATRICES_ROW_MAJOR, cny);
	LIFTcn[2].convertToArray(MATRICES_ROW_MAJOR, cnz);
	LIFTrn[0].convertToArray(MATRICES_ROW_MAJOR, rnx);
	LIFTrn[1].convertToArray(MATRICES_ROW_MAJOR, rny);
	LIFTrn[2].convertToArray(MATRICES_ROW_MAJOR, rnz);
}

DGCurvedFace::~DGCurvedFace() {

}

DGCurvedFace&
DGCurvedFace::operator=(const DGCurvedFace& rhs) {
	if (&rhs == this) {
		return *this;
	}
	solverPosition = rhs.solverPosition;
	rhsEx = rhs.rhsEx;
	rhsEy = rhs.rhsEy;
	rhsEz = rhs.rhsEz;
	rhsHx = rhs.rhsHx;
	rhsHy = rhs.rhsHy;
	rhsHz = rhs.rhsHz;
	dEx = rhs.dEx;
	dEy = rhs.dEy;
	dEz = rhs.dEz;
	dHx = rhs.dHx;
	dHy = rhs.dHy;
	dHz = rhs.dHz;
	dresEx = rhs.dresEx;
	dresEy = rhs.dresEy;
	dresEz = rhs.dresEz;
	dresHx = rhs.dresHx;
	dresHy = rhs.dresHy;
	dresHz = rhs.dresHz;
	impPAv = rhs.impPAv;
	admPAv = rhs.admPAv;
	imp1Av = rhs.imp1Av;
	adm1Av = rhs.adm1Av;
	for (uint i = 0; i < np*nfp; i++) {
		nx[i] = rhs.nx[i];
		ny[i] = rhs.ny[i];
		nz[i] = rhs.nz[i];
		rnx[i] = rhs.rnx[i];
		rny[i] = rhs.rny[i];
		rnz[i] = rhs.rnz[i];
		cnx[i] = rhs.cnx[i];
		cny[i] = rhs.cny[i];
		cnz[i] = rhs.cnz[i];
	}
	return *this;
}

void
DGCurvedFace::addFluxToRHSElectric(
 const double upwinding, const bool useResForUpw) {
	if  (upwinding == 1.0 && !useResForUpw) {
		add_am_v_prod<double,np,nfp>(rhsEx,ny,dHz,-impPAv);
		add_am_v_prod<double,np,nfp>(rhsEx,nz,dHy,impPAv);
		add_am_v_prod<double,np,nfp>(rhsEx,cnx,dEy,imp1Av);
		add_am_v_prod<double,np,nfp>(rhsEx,cnz,dEz,imp1Av);
		add_am_v_prod<double,np,nfp>(rhsEx,rnx,dEx,-imp1Av);
		// ----------------------------------------------------------------
		add_am_v_prod<double,np,nfp>(rhsEy,nz,dHx,-impPAv);
		add_am_v_prod<double,np,nfp>(rhsEy,nx,dHz,impPAv);
		add_am_v_prod<double,np,nfp>(rhsEy,cny,dEz,imp1Av);
		add_am_v_prod<double,np,nfp>(rhsEy,cnx,dEx,imp1Av);
		add_am_v_prod<double,np,nfp>(rhsEy,rny,dEy,-imp1Av);
		// ----------------------------------------------------------------
		add_am_v_prod<double,np,nfp>(rhsEz,nx,dHy,-impPAv);
		add_am_v_prod<double,np,nfp>(rhsEz,ny,dHx,impPAv);
		add_am_v_prod<double,np,nfp>(rhsEz,cnz,dEx,imp1Av);
		add_am_v_prod<double,np,nfp>(rhsEz,cny,dEy,imp1Av);
		add_am_v_prod<double,np,nfp>(rhsEz,rnz,dEz,-imp1Av);
		return;
	} else if (upwinding == 1.0 && useResForUpw) {
		add_am_v_prod<double,np,nfp>(rhsEx,ny,dHz,-impPAv);
		add_am_v_prod<double,np,nfp>(rhsEx,nz,dHy,impPAv);
		add_am_v_prod<double,np,nfp>(rhsEx,cnx,dresEy,imp1Av);
		add_am_v_prod<double,np,nfp>(rhsEx,cnz,dresEz,imp1Av);
		add_am_v_prod<double,np,nfp>(rhsEx,rnx,dresEx,-imp1Av);
		// ----------------------------------------------------------------
		add_am_v_prod<double,np,nfp>(rhsEy,nz,dHx,-impPAv);
		add_am_v_prod<double,np,nfp>(rhsEy,nx,dHz,impPAv);
		add_am_v_prod<double,np,nfp>(rhsEy,cny,dresEz,imp1Av);
		add_am_v_prod<double,np,nfp>(rhsEy,cnx,dresEx,imp1Av);
		add_am_v_prod<double,np,nfp>(rhsEy,rny,dresEy,-imp1Av);
		// ----------------------------------------------------------------
		add_am_v_prod<double,np,nfp>(rhsEz,nx,dHy,-impPAv);
		add_am_v_prod<double,np,nfp>(rhsEz,ny,dHx,impPAv);
		add_am_v_prod<double,np,nfp>(rhsEz,cnz,dresEx,imp1Av);
		add_am_v_prod<double,np,nfp>(rhsEz,cny,dresEy,imp1Av);
		add_am_v_prod<double,np,nfp>(rhsEz,rnz,dresEz,-imp1Av);
		return;
	} else if (upwinding == 0.0) {
		add_am_v_prod<double,np,nfp>(rhsEx,ny,dHz,-impPAv);
		add_am_v_prod<double,np,nfp>(rhsEx,nz,dHy,impPAv);
		// ------------------------------------------------------------
		add_am_v_prod<double,np,nfp>(rhsEy,nz,dHx,-impPAv);
		add_am_v_prod<double,np,nfp>(rhsEy,nx,dHz,impPAv);
		// ------------------------------------------------------------
		add_am_v_prod<double,np,nfp>(rhsEz,nx,dHy,-impPAv);
		add_am_v_prod<double,np,nfp>(rhsEz,ny,dHx,impPAv);
		return;
	} else if (!useResForUpw) {
		add_am_v_prod<double,np,nfp>(rhsEx,ny,dHz,-impPAv);
		add_am_v_prod<double,np,nfp>(rhsEx,nz,dHy,impPAv);
		add_am_v_prod<double,np,nfp>(rhsEx,cnx,dEy,imp1Av*upwinding);
		add_am_v_prod<double,np,nfp>(rhsEx,cnz,dEz,imp1Av*upwinding);
		add_am_v_prod<double,np,nfp>(rhsEx,rnx,dEx,-imp1Av*upwinding);
		// ----------------------------------------------------------------
		add_am_v_prod<double,np,nfp>(rhsEy,nz,dHx,-impPAv);
		add_am_v_prod<double,np,nfp>(rhsEy,nx,dHz,impPAv);
		add_am_v_prod<double,np,nfp>(rhsEy,cny,dEz,imp1Av*upwinding);
		add_am_v_prod<double,np,nfp>(rhsEy,cnx,dEx,imp1Av*upwinding);
		add_am_v_prod<double,np,nfp>(rhsEy,rny,dEy,-imp1Av*upwinding);
		// ----------------------------------------------------------------
		add_am_v_prod<double,np,nfp>(rhsEz,nx,dHy,-impPAv);
		add_am_v_prod<double,np,nfp>(rhsEz,ny,dHx,impPAv);
		add_am_v_prod<double,np,nfp>(rhsEz,cnz,dEx,imp1Av*upwinding);
		add_am_v_prod<double,np,nfp>(rhsEz,cny,dEy,imp1Av*upwinding);
		add_am_v_prod<double,np,nfp>(rhsEz,rnz,dEz,-imp1Av*upwinding);
		return;
	} else {
		add_am_v_prod<double,np,nfp>(rhsEx,ny,dHz,-impPAv);
		add_am_v_prod<double,np,nfp>(rhsEx,nz,dHy,impPAv);
		add_am_v_prod<double,np,nfp>(rhsEx,cnx,dresEy,imp1Av*upwinding);
		add_am_v_prod<double,np,nfp>(rhsEx,cnz,dresEz,imp1Av*upwinding);
		add_am_v_prod<double,np,nfp>(rhsEx,rnx,dresEx,-imp1Av*upwinding);
		// ----------------------------------------------------------------
		add_am_v_prod<double,np,nfp>(rhsEy,nz,dHx,-impPAv);
		add_am_v_prod<double,np,nfp>(rhsEy,nx,dHz,impPAv);
		add_am_v_prod<double,np,nfp>(rhsEy,cny,dresEz,imp1Av*upwinding);
		add_am_v_prod<double,np,nfp>(rhsEy,cnx,dresEx,imp1Av*upwinding);
		add_am_v_prod<double,np,nfp>(rhsEy,rny,dresEy,-imp1Av*upwinding);
		// ----------------------------------------------------------------
		add_am_v_prod<double,np,nfp>(rhsEz,nx,dHy,-impPAv);
		add_am_v_prod<double,np,nfp>(rhsEz,ny,dHx,impPAv);
		add_am_v_prod<double,np,nfp>(rhsEz,cnz,dresEx,imp1Av*upwinding);
		add_am_v_prod<double,np,nfp>(rhsEz,cny,dresEy,imp1Av*upwinding);
		add_am_v_prod<double,np,nfp>(rhsEz,rnz,dresEz,-imp1Av*upwinding);
		return;
	}
}

void
DGCurvedFace::addFluxToRHSMagnetic(
 const double upwinding,
 const bool useResForUpw) {
	if (upwinding == 1.0 && !useResForUpw) {
		add_am_v_prod<double,np,nfp>(rhsHx,ny,dEz,admPAv);
		add_am_v_prod<double,np,nfp>(rhsHx,nz,dEy,-admPAv);
		add_am_v_prod<double,np,nfp>(rhsHx,cnx,dHy,adm1Av);
		add_am_v_prod<double,np,nfp>(rhsHx,cnz,dHz,adm1Av);
		add_am_v_prod<double,np,nfp>(rhsHx,rnx,dHx,-adm1Av);
		// ----------------------------------------------------------------
		add_am_v_prod<double,np,nfp>(rhsHy,nz,dEx,admPAv);
		add_am_v_prod<double,np,nfp>(rhsHy,nx,dEz,-admPAv);
		add_am_v_prod<double,np,nfp>(rhsHy,cny,dHz,adm1Av);
		add_am_v_prod<double,np,nfp>(rhsHy,cnx,dHx,adm1Av);
		add_am_v_prod<double,np,nfp>(rhsHy,rny,dHy,-adm1Av);
		// ----------------------------------------------------------------
		add_am_v_prod<double,np,nfp>(rhsHz,nx,dEy,admPAv);
		add_am_v_prod<double,np,nfp>(rhsHz,ny,dEx,-admPAv);
		add_am_v_prod<double,np,nfp>(rhsHz,cnz,dHx,adm1Av);
		add_am_v_prod<double,np,nfp>(rhsHz,cny,dHy,adm1Av);
		add_am_v_prod<double,np,nfp>(rhsHz,rnz,dHz,-adm1Av);
		return;
	} else if (upwinding == 1.0 && useResForUpw) {
		add_am_v_prod<double,np,nfp>(rhsHx,ny,dEz,admPAv);
		add_am_v_prod<double,np,nfp>(rhsHx,nz,dEy,-admPAv);
		add_am_v_prod<double,np,nfp>(rhsHx,cnx,dresHy,adm1Av);
		add_am_v_prod<double,np,nfp>(rhsHx,cnz,dresHz,adm1Av);
		add_am_v_prod<double,np,nfp>(rhsHx,rnx,dresHx,-adm1Av);
		// ----------------------------------------------------------------
		add_am_v_prod<double,np,nfp>(rhsHy,nz,dEx,admPAv);
		add_am_v_prod<double,np,nfp>(rhsHy,nx,dEz,-admPAv);
		add_am_v_prod<double,np,nfp>(rhsHy,cny,dresHz,adm1Av);
		add_am_v_prod<double,np,nfp>(rhsHy,cnx,dresHx,adm1Av);
		add_am_v_prod<double,np,nfp>(rhsHy,rny,dresHy,-adm1Av);
		// ----------------------------------------------------------------
		add_am_v_prod<double,np,nfp>(rhsHz,nx,dEy,admPAv);
		add_am_v_prod<double,np,nfp>(rhsHz,ny,dEx,-admPAv);
		add_am_v_prod<double,np,nfp>(rhsHz,cnz,dresHx,adm1Av);
		add_am_v_prod<double,np,nfp>(rhsHz,cny,dresHy,adm1Av);
		add_am_v_prod<double,np,nfp>(rhsHz,rnz,dresHz,-adm1Av);
		return;
	} else if (upwinding == 0.0) {
		add_am_v_prod<double,np,nfp>(rhsHx,ny,dEz,admPAv);
		add_am_v_prod<double,np,nfp>(rhsHx,nz,dEy,-admPAv);
		// ----------------------------------------------------------------
		add_am_v_prod<double,np,nfp>(rhsHy,nz,dEx,admPAv);
		add_am_v_prod<double,np,nfp>(rhsHy,nx,dEz,-admPAv);
		// ----------------------------------------------------------------
		add_am_v_prod<double,np,nfp>(rhsHz,nx,dEy,admPAv);
		add_am_v_prod<double,np,nfp>(rhsHz,ny,dEx,-admPAv);
		return;
	} else if (!useResForUpw) {
		add_am_v_prod<double,np,nfp>(rhsHx,ny,dEz,admPAv);
		add_am_v_prod<double,np,nfp>(rhsHx,nz,dEy,-admPAv);
		add_am_v_prod<double,np,nfp>(rhsHx,cnx,dHy,adm1Av*upwinding);
		add_am_v_prod<double,np,nfp>(rhsHx,cnz,dHz,adm1Av*upwinding);
		add_am_v_prod<double,np,nfp>(rhsHx,rnx,dHx,-adm1Av*upwinding);
		// ----------------------------------------------------------------
		add_am_v_prod<double,np,nfp>(rhsHy,nz,dEx,admPAv);
		add_am_v_prod<double,np,nfp>(rhsHy,nx,dEz,-admPAv);
		add_am_v_prod<double,np,nfp>(rhsHy,cny,dHz,adm1Av*upwinding);
		add_am_v_prod<double,np,nfp>(rhsHy,cnx,dHx,adm1Av*upwinding);
		add_am_v_prod<double,np,nfp>(rhsHy,rny,dHy,-adm1Av*upwinding);
		// ----------------------------------------------------------------
		add_am_v_prod<double,np,nfp>(rhsHz,nx,dEy,admPAv);
		add_am_v_prod<double,np,nfp>(rhsHz,ny,dEx,-admPAv);
		add_am_v_prod<double,np,nfp>(rhsHz,cnz,dHx,adm1Av*upwinding);
		add_am_v_prod<double,np,nfp>(rhsHz,cny,dHy,adm1Av*upwinding);
		add_am_v_prod<double,np,nfp>(rhsHz,rnz,dHz,-adm1Av*upwinding);
		return;
	} else {
		add_am_v_prod<double,np,nfp>(rhsHx,ny,dEz,admPAv);
		add_am_v_prod<double,np,nfp>(rhsHx,nz,dEy,-admPAv);
		add_am_v_prod<double,np,nfp>(rhsHx,cnx,dresHy,adm1Av*upwinding);
		add_am_v_prod<double,np,nfp>(rhsHx,cnz,dresHz,adm1Av*upwinding);
		add_am_v_prod<double,np,nfp>(rhsHx,rnx,dresHx,-adm1Av*upwinding);
		// ----------------------------------------------------------------
		add_am_v_prod<double,np,nfp>(rhsHy,nz,dEx,admPAv);
		add_am_v_prod<double,np,nfp>(rhsHy,nx,dEz,-admPAv);
		add_am_v_prod<double,np,nfp>(rhsHy,cny,dresHz,adm1Av*upwinding);
		add_am_v_prod<double,np,nfp>(rhsHy,cnx,dresHx,adm1Av*upwinding);
		add_am_v_prod<double,np,nfp>(rhsHy,rny,dresHy,-adm1Av*upwinding);
		// ----------------------------------------------------------------
		add_am_v_prod<double,np,nfp>(rhsHz,nx,dEy,admPAv);
		add_am_v_prod<double,np,nfp>(rhsHz,ny,dEx,-admPAv);
		add_am_v_prod<double,np,nfp>(rhsHz,cnz,dresHx,adm1Av*upwinding);
		add_am_v_prod<double,np,nfp>(rhsHz,cny,dresHy,adm1Av*upwinding);
		add_am_v_prod<double,np,nfp>(rhsHz,rnz,dresHz,-adm1Av*upwinding);
		return;
	}
}
