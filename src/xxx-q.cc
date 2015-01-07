#include <xxx.hh>

namespace xxx {

    const predicate_quantifier q::star     ( 0, SIZE_MAX );
    const predicate_quantifier q::plus     ( 1, SIZE_MAX );
    const predicate_quantifier q::question ( 0, 1        );
    const predicate_quantifier q::zero     ( 0, 0        );
    const predicate_quantifier q::one      ( 1, 1        );

    predicate_quantifier q::upper(size_t max) {
        return predicate_quantifier(0, max);
    }

    predicate_quantifier q::lower(size_t min) {
        return predicate_quantifier(min, SIZE_MAX);
    }

}
