#include <xxx.hh>

namespace xxx {

    template <typename T> _s<T>::_s() : base_type() {
    }

    template <typename T> _s<T>::_s(const value_type& x) : base_type() {
        this->push_back(x);
    }

    template <typename T> _s<T>::_s(const value_type& x, _s&& xs) : base_type(xs) {
        this->push_front(x);
    }

    template <typename T> _s<T>::_s(const value_type& x, const _s& xs) : base_type(xs) {
        this->push_front(x);
    }

    template struct _s<rule>;
    template struct _s<predicate>;
}
