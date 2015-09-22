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
 * Linel.h
 *
 *  Created on: May 19, 2015
 *      Author: mdebi
 */

#ifndef SRC_MESHER_FDTD_MESHCONF_LINEL_H_
#define SRC_MESHER_FDTD_MESHCONF_LINEL_H_

#include "Pixel.h"

class Linel: public Pixel {
public:
    Linel();
    Linel(const CVecI3& pos, const CartesianDirection& dir);
    virtual ~Linel();

    bool isInto (const CVecI3Fractional& coordIntFractional_)const;

    bool operator==(const Linel& rhs) const;
    bool operator<(const Linel& rhs) const;

    CartesianDirection  getDirId ()const{return dirId_;}
    CartesianDirection& DirId (){return dirId_;}
    void setDirId (const CartesianDirection& _dirId){dirId_ = _dirId;}

    string toStr() const;
    friend std::ostream& operator<<(ostream& os, const Linel& vec) {
       return os << vec.toStr();
    }

private:
    CartesianDirection dirId_;
};

#endif /* SRC_MESHER_FDTD_MESHCONF_LINEL_H_ */
