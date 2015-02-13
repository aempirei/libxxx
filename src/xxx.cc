#include <xxx.hh>

namespace xxx {

    template <typename T> fold<T>::fold() : base_type() {
    }

    template <typename T> fold<T>::fold(const value_type& x) : base_type() {
        this->push_back(x);
    }

    template <typename T> fold<T>::fold(const value_type& x, fold&& xs) : base_type(xs) {
        this->push_front(x);
    }

    template <typename T> fold<T>::fold(const value_type& x, const fold& xs) : base_type(xs) {
        this->push_front(x);
    }

    template struct fold<std::list<predicate>>;
    template struct fold<std::list<rule>>;
    template struct fold<std::list<tree>>;
    template struct fold<std::list<var>>;
}
