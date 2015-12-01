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

#include "Polygon.h"

namespace SEMBA {
namespace Geometry {
namespace Element {

Polygon::Polygon() {

}

Polygon::Polygon(const Id id,
                 const std::vector<const CoordR3*>& v,
                 const Layer* lay,
                 const Model* mat)
:   Identifiable<Id>(id),
    Elem(lay, mat) {
    
    const Size vSize = v.size();
    assert(v.size() >= 5); // Polygons of 3 or 4 vertices are treated as triangles or quads.
    v_.resize(vSize);
    for (Size i = 0; i < vSize; i++) {
        v_[i] = v[i];
    }
}

Polygon::Polygon(const Polygon& rhs)
:   Identifiable<Id>(rhs),
    Elem(rhs) {
    
    v_ = rhs.v_;
}

Polygon::~Polygon() {
    
}

Size Polygon::numberOfFaces() const {
    return v_.size();
}

Size Polygon::numberOfVertices() const {
    return numberOfCoordinates();
}

Size Polygon::numberOfCoordinates() const {
    return v_.size();
}

Size
Polygon::numberOfSideVertices(const Size f = 0) const {
    return 2;
}

Size
Polygon::numberOfSideCoordinates(const Size f = 0) const {
    return 2;
}

const CoordR3* Polygon::getV(const Size i) const {
    assert(i < numberOfCoordinates());
    return v_[i];
}

const CoordR3* Polygon::getSideV(const Size f,
                                 const Size i) const {
    
    return v_[(f + i) % numberOfCoordinates()];
}

const CoordR3* Polygon::getVertex(const Size i) const {
    return getV(i);
}

const CoordR3* Polygon::getSideVertex(const Size f, const Size i) const {
    return getSideV(f,i);
}

Math::Real Polygon::getArea() const {
    throw std::logic_error("Polygon::getArea not implemented");
}

void Polygon::setV(const Size i, const CoordR3* coord) {
    assert(i < numberOfCoordinates());
    v_[i] = coord;
}

void Polygon::printInfo() const {
    std::cout<< "--- Polygon info ---" << std::endl
             << "Number of coordinates: " << numberOfCoordinates() << std::endl;
    std::cout<< "Id: " << getId() << ", MatId: " << getMatId() << std::endl;
    for (Size i = 0; i < numberOfCoordinates(); i++) {
        std::cout<< "#" << i << ": ";
        v_[i]->printInfo();
    }
    std::cout<< "--- End of polygon info ---" << std::endl;
}

} /* namespace Element */
} /* namespace Geometry */
} /* namespace SEMBA */
