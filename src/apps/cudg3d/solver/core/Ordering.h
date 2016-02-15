// OpenSEMBA
// Copyright (C) 2015 Salvador Gonzalez Garcia        (salva@ugr.es)
//                    Luis Manuel Diaz Angulo         (lmdiazangulo@semba.guru)
//                    Miguel David Ruiz-Cabello Nuñez (miguel@semba.guru)
//                    Daniel Mateos Romero            (damarro@semba.guru)
//
// This file is part of OpenSEMBA.
//
// OpenSEMBA is free software: you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License as published by the Free
// Software Foundation, either version 3 of the License, or (at your option)
// any later version.
//
// OpenSEMBA is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
// details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with OpenSEMBA. If not, see <http://www.gnu.org/licenses/>.
/*
 * Ordering.h
 *
 *  Created on: Jun 12, 2013
 *      Author: luis
 */

#ifndef ORDERING_H_
#define ORDERING_H_

#include "stdlib.h"
#include <assert.h>
#include <iostream>
#include <vector>
using namespace std;

#include "geometry/elements/Element.h"
#include "math/DynMatrix.h"


class Ordering {
public:
    UInt getGlobalSize() const;
    UInt getGlobalRelPosOfId(const ElementId id) const;
    UInt getRelPosOfId(const ElementId id) const;
    ElementId getIdOfGlobalRelPos(const UInt rp) const;
    ElementId getIdOfRelPos(const UInt rp) const;
    UInt getLocalSize() const;
    bool checkRelPosOfId() const;
    bool isLocalId(const ElementId id) const;
protected:
    Ordering();
    virtual ~Ordering();
    void setGlobalSize(const UInt globalSize_);
    void setLocalSizeAndOffset(
            const UInt localSize,
            const UInt localOffset);
    void buildRelPosOfIds(
            const DynMatrix<UInt>& list);
    void printOrderingInfo() const;
private:
    static UInt globalSize;
    static UInt localSize;
    static UInt localOffset;
    static ElementId offsetId;
    static ElementId* idOfRelPos;
    static UInt* relPosOfId;
    bool checkLocalIds(
            const vector<vector<ElementId> >& partIds,
            const UInt task);
};

#endif /* ORDERING_H_ */