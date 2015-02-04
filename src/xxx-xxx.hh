#pragma once
#define R(s) rule(rule::terminal_type(s))

namespace xxx {
	namespace local {

		using M = predicate_modifier;
        using Q = predicate_quantifier;

		transform_function   document_transform_1;
		transform_function      entry_transform_1;
		transform_function      entry_transform_2;
		transform_function       line_transform_1;
		transform_function   modifier_transform_1;
		transform_function       name_transform_1;
		transform_function  predicate_transform_1;
		transform_function predicates_transform_1;
		transform_function predicates_transform_2;
		transform_function quantifier_transform_1;
		transform_function  recursive_transform_1;
		transform_function      regex_transform_1;
		transform_function    regexre_transform_1;
		transform_function      rules_transform_1;
		transform_function      rules_transform_2;

		grammar spec = grammar({
			{ "_"         , { R("\\A[ \\t]+") } },
			{ "beq"       , { R("\\A&=") } },
			{ "ceq"       , { R("\\A:=") } },
			{ "document"  , { rule() << "line" << Q::star << "eof" << M::discard >> document_transform_1 } },
			{ "entry"     , {
				rule() << "recursive" >> entry_transform_1,
				rule() << "regex" >> entry_transform_2,
			} },
			{ "eof"       , { R("\\A\\z") } },
			{ "eol"       , { R("\\A\\s*(?:$|\\z)") } },
			{ "fs"        , { R("\\A\\/") } },
			{ "leq"       , { R("\\A\"=") } },
			{ "line"      , { rule() << "ws" << M::discard << "entry" << "eol" << M::discard >> line_transform_1 } },
			{ "modifier"  , { R("\\A[!^>~]") >> modifier_transform_1 } },
			{ "name"      , { R("\\A\\w+") >> name_transform_1 } },
			{ "predicate" , { rule() << "modifier" << Q::question << "name" << "quantifier" << Q::question >> predicate_transform_1 } },
			{ "predicates", {
				rule() << "predicate" << "_" << M::discard << "predicates" >> predicates_transform_1,
				rule() << "predicate" >> predicates_transform_2,
			} },
			{ "qm"        , { R("\\A\\?") } },
			{ "quantifier", { R("\\A[*?+]") >> quantifier_transform_1 } },
			{ "recursive" , { rule() << "name" << "_" << M::discard << "ceq" << M::discard << "_" << M::discard << "rules" >> recursive_transform_1 } },
			{ "regex"     , { rule() << "name" << "_" << M::discard << "req" << M::discard << "_" << M::discard << "regexre" >> regex_transform_1 } },
			{ "regexre"   , { R("\\A\\/((?:\\/|[^\\/\\n])*)\\/\\s*$") >> regexre_transform_1 } },
			{ "req"       , { R("\\A~=") } },
			{ "rules"     , {
				rule() << "predicates" << "_" << M::discard << "fs" << M::discard << "_" << M::discard << "rules" >> rules_transform_1,
				rule() << "predicates" >> rules_transform_2,
			} },
			{ "ws"        , { R("\\A\\s*") } },
		});

        using entry_type = std::pair<var,rules>;

		void document_transform_1(tree *a, void *x) {

			// [line] : entry* -> grammar :: insert
            // [line]
            // FIXME: change this into a pass-thru so that grammar is built differentially via concatenation at the child

            grammar *g = (grammar *)x;

            g->clear();

            for(auto& child : a->children) {

                entry_type entry;

                child.transform(&entry);

                if(g->find(entry.first) == g->end())
                    g->operator[](entry.first) = {};

                auto& rs = g->at(entry.first);

                rs.insert(rs.end(), entry.second.begin(), entry.second.end());
            }
		}

		void entry_transform_1(tree *a, void *x) {
			// recursive
            a->children.front().transform(x);
		}

		void entry_transform_2(tree *a, void *x) {
			// regex
            a->children.front().transform(x);
		}

		void line_transform_1(tree *a, void *x) {
			// entry
            // entry :: entry -> grammar :: insert
            // FIXME: make as this shit above
            a->children.front().transform(x);
		}

		void modifier_transform_1(tree *a, void *x) {
			// std::string :: std::string -> predicate_modifier
            *(predicate_modifier *)x = predicate_modifier(a->match[0]);
		}

		void name_transform_1(tree *a, void *x) {
			// std::string :: std::string -> std::string
            *(std::string *)x = a->match;
		}

		void predicate_transform_1(tree *a, void *x) {
			// modifier? name quantifier? :: modifier name quantifier -> predicate
            // FIXME: rework as cartesian product construction
            auto iter = a->children.begin();

            if(iter->match_name() == "modifier") {
                iter->transform(&((predicate *)x)->modifier);
                iter++;
            }

            iter->transform(&((predicate *)x)->name);

            if(++iter != a->children.end())
                iter->transform(&((predicate *)x)->quantifier);
		}

		void predicates_transform_1(tree *a, void *x) {
			// predicate predicates :: predicate predicates -> predicates
            predicates_transform_2(a,x);
            a->children.back().transform(x);
		}

		void predicates_transform_2(tree *a, void *x) {
			// predicate :: predicate -> predicates :: push_back
            predicate p;
            a->children.front().transform(&p);
            ((predicates *)x)->push_back(p);
		}

        void quantifier_transform_1(tree *a, void *x) {
            // std::string :: std::string -> predicate_quantifier
            *(predicate_quantifier *)x = a->match == "*" ? Q::star :
                                         a->match == "+" ? Q::plus :
                                         a->match == "?" ? Q::question :
                                                           Q::one;
		}

        void recursive_transform_1(tree *a, void *x) {
            // name rules :: name rules -> entry
            entry_type& entry = *(entry_type *)x;
            a->children.front().transform(&entry.first);
            a->children.back().transform(&entry.second);
		}

        void regex_transform_1(tree *a, void *x) {
            // name regexre :: name regex -> entry
            entry_type& entry = *(entry_type *)x;
            rule::terminal_type re;
            a->children.front().transform(&entry.first);
            a->children.back().transform(&re);
            entry.second.push_back(re);
		}

		void regexre_transform_1(tree *a, void *x) {
			// std::string :: std::string -> regex
            *(rule::terminal_type *)x = "\\A" + a->match;
		}

		void rules_transform_1(tree *a, void *x) {
			// predicates rules :: predicates rules -> rules
            rules_transform_2(a,x);
            a->children.back().transform(x);
		}

		void rules_transform_2(tree *a, void *x) {
			// predicates :: predicates -> rules :: push_back
            predicates ps;
            a->children.front().transform(&ps);
            ((rules *)x)->push_back(ps);
		}

	}
}

#undef R
