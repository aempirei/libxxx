#include <xxx.hh>

namespace xxx {
    const predicate_quantifier q::star     ( 0, INT_MAX );
    const predicate_quantifier q::plus     ( 1, INT_MAX );
    const predicate_quantifier q::question ( 0, 1       );
    const predicate_quantifier q::one      ( 1, 1       );
}
