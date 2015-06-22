/*
 * Integrator.cpp
 *
 *  Created on: Feb 21, 2013
 *      Author: luis
 */

#include "Integrator.h"

Integrator::Integrator() {
	mindt = 0.0;
	nTiers = 0;
	tierRange = NULL;
	doLTS = true;
	growSmallerTiers = 0;
	maxNumOfTiers = 0;
	timeStepSize = 0.0;
	solver = NULL;
	partIds.clear();
}

Integrator::~Integrator() {
	for (UInt i = 0; i < nTiers; i++) {
		delete tierRange[i];
	}
}

void
Integrator::setSolver(DG* solver_) {
	solver = solver_;
}

UInt
Integrator::getNTiers() const {
	return nTiers;
}

Real
Integrator::getMaxDT() const {
	if (doLTS) {
		return (mindt / pow(getMaxTimeRatio(), Real(nTiers-1)));
	} else {
		return mindt;
	}
}

Real
Integrator::getMinDT() const {
	return mindt;
}

vector<vector<UInt> >
Integrator::getTiersIds() const {
	vector<vector<UInt> > res;
	for (UInt tier = 0; tier < nTiers; tier++) {
		vector<UInt> tierIds = getIdsOfTier(tier);
		res.push_back(tierIds);
	}
	return res;
}

vector<vector<UInt> >
Integrator::getStagesIds() const {
	vector<vector<UInt> > res;
	for (UInt stage = 0; stage < getNStages(); stage++) {
		vector<UInt> stageIds =	getIdsOfStage(stage);
		res.push_back(stageIds);
	}
	return res;
}

vector<vector<UInt> >
Integrator::getPartitionsIds() const {
	//assert(partIds.size() != 0);
	return partIds;
}

vector<pair<UInt, Int> >
Integrator::getComputationalWeights(const MeshVolume* msh) const {
	const Int curlFlops = 1; // TODO curlFlops are not considered.
	const Int fluxFlops = 0; // TODO fluxFlops are not considered.
	Int flops = curlFlops + fluxFlops;
	UInt nK = msh->nVolumeElements();
	vector<pair<UInt,Int> > idWgt;
	idWgt.reserve(nK);
	for (UInt e = 0; e < nK; e++) {
		pair<UInt,Int> aux;
		UInt id = getIdOfGlobalRelPos(e);
		aux.first = id;
		aux.second =
		 getNumOfIterationsPerBigTimeStep(e) * flops;
		idWgt.push_back(aux);
	}
	return idWgt;
}

pair<UInt, UInt>
Integrator::getRange(const UInt tier, const UInt stage) const {
	assert(tier < nTiers);
	assert(stage < getNStages());
	assert(tierRange != NULL);
	return tierRange[tier][stage];
}

UInt
Integrator::getNPartitions() const {
	return partIds.size();
}

void
Integrator::partitionate(
 const MeshVolume* msh,
 Comm* comm) {
	cout << " - Getting computational weights... " << flush;
	vector<pair<UInt,Int> > idWgt;
	idWgt = getComputationalWeights(msh);
	cout << "OK" << endl;
	vector<vector <UInt> > partId;
	cout << " - Obtaining partition ids... " << flush;
	partId =
	 msh->getPartitionsIds(comm->getNumberOfTasks(), idWgt);
	cout << "OK" << endl;
	cout << " - Setting partition sizes... " << flush;
	comm->setPartitionSizes(partId);
	cout << "OK" << endl;
	cout << " - Reordering partitions... " << flush;
	reorder(partId, comm->getLocalOffset(), comm->getLocalSize());
	cout << "OK" << endl;
}

void
Integrator::printInfo() const {
	cout << "--- SolverInfo ---" << endl;
	cout << "Min. time step: " << mindt*1E12 << " [ps]" << endl;
	cout << "Max. time step: " << getMaxDT()*1E12 << " [ps]" << endl;
	cout << "Number of tiers: " << nTiers << endl;
	if (nTiers > 1) {
		for (UInt i = 0; i < nTiers; i++) {
			cout << "# of Cells in tier " << i << ": "
			 << getNumberOfCellsInTier(i) << endl;
			for (UInt j = 0; j < getNStages(); j++) {
				cout << "--> Range stage #" << j << ": "
				 << getRange(i,j).first << ", "
				 << getRange(i,j).second << endl;
			}
		}
	}
}

void
Integrator::init(
 const MeshVolume& mesh,
 const PMGroup& pmGroup,
 const ArgumentsCudg3d* arg) {
	growSmallerTiers = arg->getGrowSmallerTiers();
	maxNumOfTiers = arg->getMaxNumberOfTiers();
	doLTS = !arg->isNoLTS();
	if (!doLTS) {
		maxNumOfTiers = 1;
	}
	cout << "- Building tier info... " << flush;
	buildTierInfo(mesh, pmGroup);
	cout << " OK" << endl;
	cout << "- Building Relative Positions of Ids... " << flush;
	buildRelPosOfIds(timeTierList);
	cout << " OK" << endl;
	cout << "- Building Tier Range... " << flush;
	tierRange = buildTierRange(tierRange, timeTierList);
	cout <<  "OK" << endl;
	//
	assert(timeStepSize != 0.0);
	assert(tierRange != NULL);
}

Real
Integrator::getMaxTimeStep(
 const Tet* tet,
 const PhysicalModel* mat) const {
	// Returns the maximum time step allowed for this cell.
	Real fS1 = 0.0;
	for (UInt f = 0; f < tet->numberOfFaces(); f++) {
		Real area = tet->getAreaOfFace(f);
		Real volume = tet->getVolume();
		Real fS2 = area / volume;
		if (fS2 > fS1) {
			fS1 = fS2;
		}
	}
	Real dt = cfl *  (1.0 / Real(fS1 * ORDER_N * ORDER_N));
	// Checks case of electrical dispersive materials.
	checkMaterialStabilityForDT(mat, dt);
	return (dt * timeStepSize);
}

UInt
Integrator::getNumberOfCellsInTier(const UInt tier) const {
	assert(tier < nTiers);
	assert(tierRange != NULL);
	UInt first = getRange(tier, 0).first;
	UInt last = getRange(tier, getNStages() - 1).second;
	UInt res = last - first;
	return res;
}

vector<UInt>
Integrator::getIdsOfTier(UInt tier) const {
	assert(tier < nTiers);
	UInt nK = timeTierList.nRows();
	vector<UInt> res;
	res.reserve(nK);
	for (UInt i = 0; i < nK; i++) {
		if (timeTierList(i,1) == tier) {
			res.push_back(timeTierList(i,0));
		}
	}
	return res;
}

vector<UInt>
Integrator::getIdsOfStage(UInt stage) const {
	assert(stage < getNStages());
	UInt nK = timeTierList.nRows();
	vector<UInt> res;
	res.reserve(nK);
	for (UInt i = 0; i < nK; i++) {
		if (timeTierList(i,2) == stage) {
			res.push_back(timeTierList(i,0));
		}
	}
	return res;
}

void
Integrator::reorder(
 const vector<vector<UInt> >& partitionId,
 const UInt localOffset,
 const UInt localSize) {
	partIds = partitionId;
	// Reorders timeTierList according to partitions.
	reorderTimeTierList(partitionId);
	// Now aux stores the final ordering considering partitions.
	buildRelPosOfIds(timeTierList);
	// Builds time tier range
	DynMatrix<UInt> reducedList(localSize,3);
	for (UInt i = 0; i < localSize; i++) {
		for (UInt j = 0; j < 3; j++) {
			reducedList(i,j) = timeTierList(i + localOffset, j);
		}
	}
	tierRange = buildTierRange(tierRange, reducedList);
}

void
Integrator::reorderTimeTierList(
 const vector<vector<UInt> >& partitionId) {
	UInt nK = timeTierList.nRows();
	DynMatrix<UInt> aux(nK, 5); // relPos - Ids - Part - Tier - Stage
	for (UInt k = 0; k < nK; k++) {
		aux(k, 0) = k;
		aux(k, 1) = timeTierList(k, 0);
		aux(k, 2) = 0; // Temporary assignation of partition.
		aux(k, 3) = timeTierList(k, 1); // Tier assignation.
		aux(k, 4) = timeTierList(k, 2);
	}
	aux.sortRows_omp(1, 1);
	UInt initId = aux(0, 1);
	UInt nParts = partitionId.size();
	for (UInt p = 0; p < nParts; p++) {
		for (UInt i = 0; i < partitionId[p].size(); i++) {
			UInt id = partitionId[p][i];
			aux(id - initId, 2) = p;
		}
	}
	aux.sortRows_omp(2, 4);
	for (UInt i = 0; i < nK; i++) {
		timeTierList(i, 0) = aux(i, 1);
		timeTierList(i, 1) = aux(i, 3);
		timeTierList(i, 2) = aux(i, 4);
	}
}

void
Integrator::buildTierInfo(
 const MeshVolume& mesh,
 const PMGroup& pmGroup) {
	assignTiersBasedOnMaxTimeStep(mesh, pmGroup);
	// Grows smallest tier regions for smoothing.
	if (nTiers > 1 && growSmallerTiers > 0) {
		growSmallestTierRegions(growSmallerTiers, mesh);
	}
	if (nTiers > 1) {
		assignStages(mesh);
	}
	// Ensures that all elem within same tier/stage are consecutive.
	if (nTiers > 1) {
		timeTierList.sortRows_omp(1,2);
		nTiers = timeTierList.maxValInCol(1) + 1;
	}
}

pair<UInt,UInt>**
Integrator::buildTierRange(
 pair<UInt,UInt> **range,
 const DynMatrix<UInt>& list) {
	UInt nK = list.nRows();
	static const UInt vS = 2;
	const UInt nStages = getNStages();
	// Allocates memory for tierRange.
	if (range != NULL) {
		for (UInt i = 0; i < nTiers; i++) {
			delete [] range[i];
		}
		delete [] range;
	}
	range = new pair<UInt,UInt>*[nTiers];
	for (UInt i = 0; i < nTiers; i++) {
		range[i] = new pair<UInt,UInt>[nStages];
	}
	// Assigns ranges for tier 0.
	if (nTiers == 1) {
		for (UInt i = 0; i < nStages; i++) {
			range[0][i].first = 0;
			range[0][i].second = nK;
		}
	} else {
		static const UInt nextKey[2] = {1, 0};
		UInt e2 = list.findFirstOcurrenceInColumns(nextKey,1,vS);
		for (UInt i = 0; i < nStages; i++) {
			range[0][i].first = 0;
			range[0][i].second = e2;
		}
	}
	//
	UInt key[2], nextKey[2];
	for (UInt tier = 1; tier < nTiers; tier++) {
		for (UInt stage = 0; stage < nStages; stage++) {
			key[0] = tier;
			key[1] = stage;
			UInt e1 = list.findFirstOcurrenceInColumns(key,1,vS);
			UInt e2;
			if (tier+1 != nTiers || stage+1 != nStages) {
				nextKey[0] = tier + (stage+1) / nStages;
				nextKey[1] = (stage+1) % nStages;
				e2 = list.findFirstOcurrenceInColumns(nextKey,1,vS);
			} else {
				e2 = nK;
			}
			range[tier][stage].first = e1;
			range[tier][stage].second = e2;
		}
	}
	return range;
}

void
Integrator::checkMaterialStabilityForDT(
 const PhysicalModel* mat,
 const Real dt) const {
   const PMVolumeDispersive* disp = dynamic_cast<const PMVolumeDispersive*>(mat);
   if (disp != NULL) {
	   for (UInt p = 0; p < disp->getPoleNumber(); p++) {
			Real polePeriod = 1.0 / abs(disp->getPole(p));
			if (polePeriod < dt) {
				cerr << endl << "ERROR@Integrator: "
				 << " Contains pole #" << p + 1
				 << " with value " << disp->getPole(p)
				 << " will cause an unstability for dt"
				 << dt << endl;
				mat->printInfo();
			}
		}
	}
}

void
Integrator::growSmallestTierRegions(
 const UInt toGrow,
 const MeshVolume& mesh) {
	UInt nK = mesh.elem_.nVolumeElements();
	timeTierList.sortRows_omp(0,0);
	for (UInt tier = 0; tier < nTiers-1; tier++) {
		// Creates a list with all the elements belonging to this tier.
		vector<UInt> elem;
		elem.reserve(nK);
		bool isInRegion;
		for (UInt k = 0; k < nK; k++) {
			isInRegion = (timeTierList(k,1) == tier);
			if (isInRegion) {
				elem.push_back(timeTierList(k,0));
			}
		}
		vector<UInt> grownElem, newNeigh, neigh;
		grownElem = elem;
		for (UInt stage = 0; stage < toGrow; stage++) {
			newNeigh = mesh.getAdjacentElements(grownElem);
			neigh.insert(
			 neigh.end(), newNeigh.begin(), newNeigh.end());
			grownElem.insert(
			 grownElem.end(), newNeigh.begin(), newNeigh.end());
		}
		for (UInt k = 0; k < neigh.size(); k++) {
			UInt id = neigh[k];
			UInt row =
			timeTierList.findFirstOcurrenceInColumns(&id,0,1);
			timeTierList(row,1) = tier;
		}
	}
	// Updates number of tiers.
	timeTierList.sortRows_omp(1,1);
	nTiers = timeTierList(nK-1,1) + 1;
}

void
Integrator::assignTiersBasedOnMaxTimeStep(
 const MeshVolume& mesh,
 const PMGroup& pmGroup) {
	UInt nK = mesh.elem_.nVolumeElements();
	DynMatrix<Real> dtList(nK, 4);
	mindt = 0.0;
	for (UInt k = 0; k < nK; k++) {
		const Tet* tet = mesh.elem_.tet[k];
		const PMVolume* mat =
		 pmGroup.getPMVolumeWithId(tet->getMatId());
		Real dt = getMaxTimeStep(tet, mat);
		if (mindt > dt || mindt == 0.0) {
			mindt = dt;
		}
		dtList(k,0) = tet->getId();
		dtList(k,1) = dt;
		dtList(k,2) = noTier;
		dtList(k,3) = getNStages() - 1;
	}
	if (maxNumOfTiers != 1) {
		dtList.sortRows_omp(1,1);
		Real maxdtList = dtList(nK-1,1);
		Real ratio = getMaxTimeRatio();
		nTiers = floor(log(mindt/maxdtList)/log(ratio)) + 1;
		if (maxNumOfTiers > 0 && nTiers > maxNumOfTiers) {
			nTiers = maxNumOfTiers;
		}
		for (UInt tier = 0; tier < nTiers; tier++) {
			Real inf = mindt / pow(ratio, Real(tier));
			Real sup;
			if (tier+1 == maxNumOfTiers) {
				sup = numeric_limits<Real>::max();
			} else {
				sup = mindt / pow(ratio, Real(tier + 1));
			}
			for (UInt k = 0; k < nK; k++) {
				if (dtList(k,1) >= inf && dtList(k,1) < sup) {
					dtList(k,2) = tier;
				}
			}
		}
	} else {
		nTiers = 1;
	}
	timeTierList.copy(dtList.eliminateColumns(1,1));
}

void
Integrator::assignStages(const MeshVolume& mesh) {
	if (nTiers == 1) {
		return;
	}
	// ----------- Reassigns tiers ------------------------------------
	UInt nK = mesh.elem_.nVolumeElements();
	timeTierList.sortRows_omp(0,0);
	const UInt nStages = getNStages();
	for (UInt tier = 0; tier < nTiers-2; tier++) {
		// Creates a list with all the elements belonging to this tier.
		vector<UInt> elem;
		elem.reserve(nK);
		bool isInRegion;
		for (UInt k = 0; k < nK; k++) {
			isInRegion = (timeTierList(k,1) == tier);
			if (isInRegion) {
				elem.push_back(timeTierList(k,0));
			}
		}
		vector<UInt> grownElem, newNeigh, neigh;
		grownElem = elem;
		for (UInt stage = 0; stage < (nStages * growStages); stage++) {
			newNeigh = mesh.getAdjacentElements(grownElem);
			neigh.insert(
			 neigh.end(), newNeigh.begin(), newNeigh.end());
			grownElem.insert(
			 grownElem.end(), newNeigh.begin(), newNeigh.end());
		}
		for (UInt k = 0; k < neigh.size(); k++) {
			UInt id = neigh[k];
			UInt row =
			 timeTierList.findFirstOcurrenceInColumns(&id,0,1);
			if (timeTierList(row,1) > tier + 1) {
				timeTierList(row,1) = tier + 1;
			}
		}
	}
	// ----------- Assigns stages -------------------------------------
	for (UInt tier = 0; tier < nTiers-1; tier++) {
		vector<UInt> elem;
		elem.reserve(nK);
		bool isInRegion;
		for (UInt k = 0; k < nK; k++) {
			isInRegion = (timeTierList(k,1) == tier);
			if (isInRegion) {
				elem.push_back(timeTierList(k,0));
			}
		}
		vector<UInt> grownElem, newNeigh, neigh;
		grownElem = elem;
		for (UInt stage = 0; stage < nStages; stage++) {
			for (UInt times = 0; times < growStages; times++) {
				newNeigh = mesh.getAdjacentElements(grownElem);
				grownElem.insert(
				 grownElem.end(), newNeigh.begin(), newNeigh.end());
				for (UInt k = 0; k < newNeigh.size(); k++) {
					UInt id = newNeigh[k];
					UInt row =
					 timeTierList.findFirstOcurrenceInColumns(&id,0,1);
					if (row < nK) {
						if (timeTierList(row,1) > tier) {
							timeTierList(row,2) = stage;
						}
					}
				}
			}
		}
	}
}

vector<pair<UInt, UInt> >
Integrator::getIdPartitionVector(
 const vector<vector<UInt> >& pId) const {
	assert(pId.size() != 0);
	vector<pair<UInt,UInt> > res;
	UInt nPart = pId.size();
	UInt gSize = 0;
	for (UInt p = 0; p < nPart; p++) {
		gSize += pId[p].size();
	}
	res.reserve(gSize);
	pair<UInt,UInt> idPart;
	for (UInt p = 0; p < nPart; p++) {
		for (UInt k = 0; k < pId[p].size(); k++) {
			idPart.first = pId[p][k];
			idPart.second = p;
			res.push_back(idPart);
		}
	}
	return res;
}

//void
//Integrator::processStopRequest() {
//	// Terminates the program storing the fields if key is pressed.
//	nonblock(NB_ENABLE);
//	Int endingRequest = 0;
//	endingRequest = kbhit();
//	if (endingRequest != 0 && fgetc(stdin)=='q') {
//		cout << "Key 'q' was pressed, terminating." << endl;
//		// Writes fields for resume.
//		cout << " - Writing resume files." << endl;
//		const Real *fPtrs[6];
//		getFieldPtrs(fPtrs);
//		out->writeResumeFile(fPtrs, tStep, nK);
//		cout << " - Program was succesfully terminated." << endl;
//		exit(EXIT_SUCCESS);
//	}
//	nonblock(NB_DISABLE);
//}

//Int
//Integrator::kbhit()	{
//	struct timeval tv;
//	fd_set fds;
//	tv.tv_sec = 0;
//	tv.tv_usec = 0;
//	FD_ZERO(&fds);
//	FD_SET(STDIN_FILENO, &fds); //STDIN_FILENO is 0
//	select(STDIN_FILENO+1, &fds, NULL, NULL, &tv);
//	return FD_ISSET(STDIN_FILENO, &fds);
//}

//void
//Integrator::nonblock(Int state) {
//	struct termios ttystate;
//	//get the terminal state
//	tcgetattr(STDIN_FILENO, &ttystate);
//	if (state == NB_ENABLE) {
//		//turn off canonical mode
//		ttystate.c_lflag &= ~ICANON;
//		//minimum of number input read.
//		ttystate.c_cc[VMIN] = 1;
//	}
//	else if (state == NB_DISABLE) {
//		//turn on canonical mode
//		ttystate.c_lflag |= ICANON;
//	}
//	//set the terminal attributes.
//	tcsetattr(STDIN_FILENO, TCSANOW, &ttystate);
//}
