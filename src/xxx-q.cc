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
        namespace q {
                const predicate_quantifier star(0,INT_MAX);
                const predicate_quantifier plus(1,INT_MAX);
                const predicate_quantifier zero(0,0);
                const predicate_quantifier question(0,1);
                const predicate_quantifier one(1,1);
        }
}
