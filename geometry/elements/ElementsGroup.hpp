/*
 * ElementsGroup.hpp
 *
 *  Created on: Mar 28, 2013
 *      Author: luis
 */

#include "ElementsGroup.h"

template<typename E>
ElementsGroup<E>::ElementsGroup() {

}

template<typename E>
ElementsGroup<E>::ElementsGroup(const vector<E*>& elems, bool ownership)
:   GroupId<E, ElementId>(elems, ownership) {

}

template<typename E>
ElementsGroup<E>::ElementsGroup(const Group<E>& rhs)
:   GroupId<E, ElementId>(rhs) {

}

template<typename E>
ElementsGroup<E>::~ElementsGroup() {

}

template<typename E>
ElementsGroup<E>& ElementsGroup<E>::operator=(const Group<E>& rhs) {
    if (this == &rhs) {
        return *this;
    }

    GroupId<E, ElementId>::operator=(rhs);

    return *this;
}

template<typename E>
bool ElementsGroup<E>::isLinear() const {
    return (this->template sizeOf<Tri6> () == 0 &&
            this->template sizeOf<Tet10>() == 0);
}

template<typename E>
bool ElementsGroup<E>::areTetrahedrons(const vector<ElementId>& elemId) const {
    UInt nE = elemId.size();
    for (UInt i = 0; i < nE; i++) {
        if (!this->getPtrToId(elemId[i])->template is<Tet>()) {
            return false;
        }
    }
    return true;
}

template<typename E>
bool ElementsGroup<E>::areTriangles(const vector<ElementId>& elemId) const {
    UInt nE = elemId.size();
    for (UInt i = 0; i < nE; i++) {
        if (!this->getPtrToId(elemId[i])->template is<Tri>()) {
            return false;
        }
    }
    return true;
}

template<typename E>
ElementsGroup<E> ElementsGroup<E>::get(const ElementBase::Type type) const {
    vector<E*> res;
    res.reserve(this->size());
    for (UInt i = 0; i < this->size(); i++) {
        if (this->element_[i]->getType() == type) {
            res.push_back(this->element_[i]);
        }
    }
    return ElementsGroup<E>(res, false);
}

template<typename E>
ElementsGroup<E> ElementsGroup<E>::get(const MatId matId) const {
    vector<MatId> aux;
    aux.push_back(matId);
    return get(aux);
}

template<typename E>
ElementsGroup<E> ElementsGroup<E>::get(const vector<MatId>& matIds_) const {
    set<MatId> matIds(matIds_.begin(), matIds_.end());
    vector<E*> res;
    res.reserve(this->size());
    for (UInt i = 0; i < this->size(); i++) {
        if (matIds.count(this->element_[i]->getMatId()) == 1) {
            res.push_back(this->element_[i]);
        }
    }
    return ElementsGroup<E>(res, false);
}

template<typename E>
ElementsGroup<E> ElementsGroup<E>::get(const LayerId layerId) const {
    vector<LayerId> aux;
    aux.push_back(layerId);
    return get(aux);
}

template<typename E>
ElementsGroup<E> ElementsGroup<E>::get(const vector<LayerId>& layIds_) const {
    set<LayerId> layIds(layIds_.begin(), layIds_.end());
    vector<E*> res;
    res.reserve(this->size());
    for (UInt i = 0; i < this->size(); i++) {
        if (layIds.count(this->element_[i]->getLayerId()) == 1) {
            res.push_back(this->element_[i]);
        }
    }
    return ElementsGroup<E>(res, false);
}

template<typename E>
ElementsGroup<E> ElementsGroup<E>::get(const MatId   matId,
                                       const LayerId layId) const {

    return get(matId).get(layId);
}

template<typename E>
ElementsGroup<E> ElementsGroup<E>::get(const ElementBase::Type type,
                                       const MatId   matId,
                                       const LayerId layId) const {

    return get(type).get(matId, layId);
}

template<typename E>
vector<ElementId> ElementsGroup<E>::getIds() const {

    vector<ElementId> res;
    res.reserve(this->size());
    for (UInt i = 0; i < this->size(); i++) {
        res.push_back(this->element_[i]->getId());
    }
    return res;
}

template<typename E>
vector<ElementId> ElementsGroup<E>::getHexIds() const {
    vector<const HexR8*> hex8 =
        this->template getVectorOf<HexR8>();
    const UInt nK = hex8.size();
    vector<ElementId> res(nK);
    for (UInt i = 0; i < nK; i++) {
        res[i] = hex8[i]->getId();
    }
    return res;
}

template<typename E>
vector<ElementId> ElementsGroup<E>::getIdsWithMaterialId(
    const MatId matId) const {

    vector<ElementId> res;
    res.reserve(this->size());
    for (UInt i = 0; i < this->size(); i++) {
        if (this->element_[i]->getMatId() == matId) {
            res.push_back(this->element_[i]->getId());
        }
    }
    return res;
}

template<typename E>
vector<ElementId> ElementsGroup<E>::getIdsWithoutMaterialId(
    const MatId matId) const {

    vector<ElementId> res;
    res.reserve(this->size());
    for (UInt i = 0; i < this->size(); i++) {
        if (this->element_[i]->getMatId() != matId) {
            res.push_back(this->element_[i]->getId());
        }
    }
    return res;
}

template<typename E>
void ElementsGroup<E>::add(E* newElem, bool newId) {
    return GroupId<E, ElementId>::add(newElem, newId);
}

template<typename E>
void ElementsGroup<E>::add(vector<E*>& newElems, bool newId) {
    return GroupId<E, ElementId>::add(newElems, newId);
}

template<typename E>
vector<ElementId> ElementsGroup<E>::add(const CoordinateGroup<>& coord,
                                        const vector<HexR8>& newHex) {
    CoordinateId vId[8];
    MatId matId;
    vector<ElementId> res;
    for (UInt i = 0; i < newHex.size(); i++) {
        // Determines coordinates ids.
        // PERFORMANCE This is O(N^2). It can be improved by creating a
        //               lexicographically sorted list of coordinates positions.
        for (UInt j = 0; j < 8; j++) {
            vId[j] = newHex[i].getV(j)->getId();
        }
        matId = newHex[i].getMatId();
        HexR8* newhex =
            new HexR8(coord, ElementId(0), vId, LayerId(0), matId);
        if(newhex->is<E>()) {
            this->add(newhex->template castTo<E>(), true);
            res.push_back(newhex->getId());
        } else {
            delete newhex;
        }
    }
    return res;
}

template<typename E>
map<LayerId, ElementsGroup<E> > ElementsGroup<E>::separateLayers() const {
    map<LayerId, ElementsGroup<E> > res;
    for (UInt i = 0; i < this->size(); i++) {
        const LayerId layerId = this->element_[i]->getLayerId();
        typename map<LayerId, ElementsGroup<E> >::iterator it =
            res.find(layerId);
        if (it == res.end()) {
            pair<LayerId, ElementsGroup<E> > newEntry;
            newEntry.first = layerId;
            newEntry.second.add(this->element_[i]);
            res.insert(newEntry);
        } else {
            it->second.add(this->element_[i]);
        }
    }
    return res;
}

template<typename E>
ElementsGroup<E> ElementsGroup<E>::removeElementsWithMatId(
    const MatId matId) const {

    vector<E*> elems;
    elems.reserve(this->size());
    for (UInt i = 0; i < this->size(); i++) {
        if (this->element_[i]->getMatId() != matId) {
            elems.push_back(this->element_[i]->clone()->template castTo<E>());
        }
    }
    return ElementsGroup<E>(elems, false);
}

template<typename E>
void ElementsGroup<E>::reassignPointers(const CoordinateGroup<>& vNew) {
    for (UInt i = 0; i < this->size(); i++) {
        if (this->element_[i]->template is<ElemR>()) {
            ElemR* elem = this->element_[i]->template castTo<ElemR>();
            for (UInt j = 0; j < elem->numberOfCoordinates(); j++) {
                CoordinateId vId = elem->getV(j)->getId();
                const CoordinateBase* coord = vNew.getPtrToId(vId);
                if (coord == NULL) {
                    cerr << "ERROR @ ElementsGroup<E>::reassignPointers(): "
                         << "Coordinate in new CoordinateGroup inexistent"
                         << endl;
                    assert(false);
                    exit(EXIT_FAILURE);
                }
                if (!coord->is<CoordR3>()) {
                    cerr << "ERROR @ ElementsGroup<E>::reassignPointers(): "
                         << "Coordinate in new CoordinateGroup is not a valid Coordinate"
                         << endl;
                    assert(false);
                    exit(EXIT_FAILURE);
                }
                elem->setV(j, coord->castTo<CoordR3>());
            }
        }
    }
}

template<typename E>
void ElementsGroup<E>::linearize() {
    if (isLinear()) {
        return;
    }
    if(!this->ownership_) {
        cerr << "ERROR @ ElementsGroup::linearize(): "
             << "Forbidden to linearize without ownership "
             << "of elements on it" << endl;
        assert(false);
        exit(EXIT_FAILURE);
    }

    for(UInt i = 0; i < this->size(); i++) {
        if (this->element_[i]->template is<Tri6> ()) {
            Tri6* tmpPtr = this->element_[i]->template castTo<Tri6>();
            this->element_[i] = tmpPtr->linearize();
            delete tmpPtr;
        }
        if (this->element_[i]->template is<Tet10> ()) {
            Tet10* tmpPtr = this->element_[i]->template castTo<Tet10>();
            this->element_[i] = tmpPtr->linearize();
            delete tmpPtr;
        }
    }
}

template<typename E>
void ElementsGroup<E>::printInfo() const {
    cout << "--- Elements Group info ---" << endl;
    cout << "Total number of elements: " << this->size() << endl;
    for(UInt i = 0; i < this->size(); i++) {
        this->element_[i]->printInfo();
        cout << endl;
    }
}
