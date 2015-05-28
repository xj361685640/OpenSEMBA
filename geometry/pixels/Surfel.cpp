/*
 * Surfel.cpp
 *
 *  Created on: May 19, 2015
 *      Author: Miguel D. Ruiz-Cabello N.
 */

#include "Surfel.h"

bool Surfel::isInto (const CVecI3Frac& coordIntFractional_){
    CVecR3 pos_ = coordIntFractional_.getRelativePos();
    for(UInt n=0; n<3; n++){
        if(n==(UInt)(normId_-1)){
            if(MathUtils::notEqual(
                    pos_.val[n],(Real)coordIntFractional_.val[n])){
                return false;
            }
        }else if ( pos_.val[n]<(Real)coordIntFractional_.val[n] ||
                pos_.val[n]>((Real)coordIntFractional_.val[n]+1.0) ){
            return false;
        }
    }
    return true;
}

Surfel::Surfel(const CVecI3& pos, const CartesianDirection& norm) : Pixel(pos) {
    normId_ = norm;
}

bool Surfel::operator ==(const Surfel& rhs) const {
    return (Pixel::operator==(rhs) && rhs.normId_ == normId_);
}

string Surfel::toStr() const {
    stringstream res;
    res << CVecI3::toStr() << " Norm: " << Pixel::toStr(normId_);
    return res.str();
}

SurfelLinels Surfel::getLinels() const {
    SurfelLinels res;
    for (UInt i = 0; i < 4; i++) {
        res[i] = getLinel(i);
    }
    return res;
}

Linel Surfel::getLinel(const UInt s) const {
    CVecI3 pos = *this;
    UInt norm(normId_-1);
    if (s == 2) {
        pos((norm+2)%3)++;
    } else if (s == 3) {
        pos((norm+1)%3)++;
    }
    UInt dir = (norm + (s%2) + 1) % 3;
    return Linel(pos, CartesianDirection(dir+1));
}

bool Surfel::operator<(const Surfel& rhs) const {
    if (Pixel::operator<(rhs)) {
        return true;
    }
    if (Pixel::operator==(rhs)) {
        return normId_ < rhs.normId_;
    }
    return false;
}
