#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <climits>
#include <cassert>

#include <unistd.h>

#include <stdexcept>

#include <xxx.hh>

namespace xxx {

        predicate::predicate() : quantifier(q::one), modifier(predicate_modifier::push) {
        }

        std::string predicate::str() {
                return "<PREDICATE>";
        }
}
