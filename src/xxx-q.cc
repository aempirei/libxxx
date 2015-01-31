#include <xxx.hh>

namespace xxx {

    const predicate_quantifier Q::star     ( 0, SIZE_MAX );
    const predicate_quantifier Q::plus     ( 1, SIZE_MAX );
    const predicate_quantifier Q::question ( 0, 1        );
    const predicate_quantifier Q::one      ( 1, 1        );

    predicate_quantifier Q::upper(size_t max) {
        return predicate_quantifier(0, max);
    }

    predicate_quantifier Q::lower(size_t min) {
        return predicate_quantifier(min, SIZE_MAX);
    }

}
