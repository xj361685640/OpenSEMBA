
#ifndef SEMBA_ARGUMENT_VALUEMULTI_H_
#define SEMBA_ARGUMENT_VALUEMULTI_H_

#include "ValueBase.h"

namespace SEMBA {
namespace Argument {

class ValueMulti : public virtual ValueBase {
public:
    ValueMulti() {}
    virtual ~ValueMulti() {}

    virtual void parse(Object&,
                       std::vector<std::list<std::string>>&,
                       std::vector<std::list<std::string>>&) const;

    virtual void parseSingle(Object&,
                             std::list<std::string>&,
                             std::list<std::string>&) const = 0;
};

} /* namespace Argument */
} /* namespace SEMBA */

#endif /* SEMBA_ARGUMENT_VALUEMULTI_H_ */
