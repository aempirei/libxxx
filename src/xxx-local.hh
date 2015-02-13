#pragma once

namespace xxx {
    namespace local {
        using document = grammar;
        using regex = rule::terminal_type;
        using repl = std::string;
        using entry = std::pair<var,rules>;
        using pm = predicate_modifier;
        using pq = predicate_quantifier;
        template <typename T> static inline typename T::value_type head(const T& s) {
            return s.front();
        }
    }
}
