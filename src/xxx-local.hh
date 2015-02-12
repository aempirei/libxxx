#pragma once

namespace xxx {
    namespace local {
        using document = grammar;
        using composite = predicates;
        using regex = rule::terminal_type;
        using terminal = regex;
        using repl = std::string;
        using entry = std::pair<var,rules>;
        using m = predicate_modifier;
        static inline char head(const std::string& s) {
            return s.front();
        }
    }
}
