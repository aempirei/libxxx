#pragma once

namespace xxx {
    namespace local {
        using document = grammar;
        using composite = predicates;
        using regex = rule::terminal_type;
        using terminal = regex;
        using repl = std::string;
        using entry = std::pair<var,rules>;
        using pm = predicate_modifier;
        using pq = predicate_quantifier;
        static inline char head(const std::string& s) {
            return s.front();
        }
    }
}
