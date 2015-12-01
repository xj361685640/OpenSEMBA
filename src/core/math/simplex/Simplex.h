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
/* =============== Simplex procedures =========================================
 * PURPOSE:
 * - Simplex indices numeration methods.
 * - Simplex Lagrange's polynomial related methods.
========================================================================= */

#ifndef SEMBA_MATH_SIMPLEX_SIMPLEX_H_
#define SEMBA_MATH_SIMPLEX_SIMPLEX_H_

#include <stdexcept>
#include <vector>

#include "math/vector/Cartesian.h"
#include "math/function/Polynomial.h"

#ifndef SIMPLEX_CUBATURE_ORDER
#   define SIMPLEX_CUBATURE_ORDER 5 // Tested to work up to 7.
#   if SIMPLEX_CUBATURE_ORDER != 5
#       warning "Simplex cubature order is not standard."
#   endif
#endif

namespace SEMBA {
namespace Math {
namespace Simplex {

class Simplex {
public:
    Simplex();
    virtual ~Simplex();
    virtual Real getCda(UInt i, UInt j, UInt k) const;
    virtual const Function::Polynomial<Real>& getLagr(const UInt i) const;
    virtual const Function::Polynomial<Real>& getDLagr(const UInt i,
                                                       const UInt f) const;
    virtual UInt nodeIndex(const UInt i, const UInt j) const;
    virtual UInt cubatureNodeIndex(const UInt i, const UInt j) const;
    virtual const CVecR4 cubatureCoordinate(const UInt c) const {
        throw std::logic_error("Simplex::cubatureCoordinate not implemented");
    }
protected:
    static const UInt cubatureOrder = SIMPLEX_CUBATURE_ORDER;
    Function::Polynomial<Real> silvesterPol(const UInt m,
                                            const UInt n) const;
    void lagrangePolynomials(Function::Polynomial<Real>* lagr,
                             const UInt n,
                             const UInt np,
                             const UInt nsc) const;
    void cubatureLagrangePolynomials(Function::Polynomial<Real>* lagr,
                                     const UInt n,
                                     const UInt np,
                                     const UInt nsc) const;
    Real integrate(const Function::Polynomial<Real> pol,
                   const UInt dimension,
                   const Real sizeFactor) const;
    UInt factorial(UInt n) const;
};

} /* namespace Simplex */
} /* namespace Math */
} /* namespace SEMBA */

#endif /* SEMBA_MATH_SIMPLEX_SIMPLEX_H_ */
