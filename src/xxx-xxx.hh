#pragma once
#define R(s) rule(rule::regex_type(s))

namespace xxx {
	namespace local {

		using M = predicate_modifier;

		transform_function document_transform_1;
		transform_function entry_transform_1;
		transform_function entry_transform_2;
		transform_function line_transform_1;
		transform_function modifier_transform_1;
		transform_function name_transform_1;
		transform_function predicate_transform_1;
		transform_function predicates_transform_1;
		transform_function predicates_transform_2;
		transform_function quantifier_transform_1;
		transform_function recursive_transform_1;
		transform_function regex_transform_1;
		transform_function regexre_transform_1;
		transform_function rules_transform_1;
		transform_function rules_transform_2;

		xxx::grammar grammar = xxx::grammar({
			{ "_"         , { R("\\A[ \\t]+") } },
			{ "beq"       , { R("\\A&=") } },
			{ "ceq"       , { R("\\A:=") } },
			{ "document"  , { rule() << "line" << q::star << "eof" << M::discard >> document_transform_1 } },
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
			{ "predicate" , { rule() << "modifier" << q::question << "name" << "quantifier" << q::question >> predicate_transform_1 } },
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

		void document_transform_1(ast *a, void *x) {
			// [line]
            using void_ptr = void *;

            xxx::grammar *g = new xxx::grammar();

            void_ptr& result = *(void_ptr *)x;

            result = (void_ptr)g;

            for(auto& child : a->children) {
                entry_type entry;
                child.match_rule->transform(&child, &entry);
                if(g->find(entry.first) == g->end())
                    g->operator[](entry.first) = {};
                auto& rs = g->at(entry.first);
                rs.insert(rs.end(), entry.second.begin(), entry.second.end());
            }
		}

		void entry_transform_1(ast *a, void *x) {
			// recursive
            auto& child = a->children.front();
            child.match_rule->transform(&child, x);
		}

		void entry_transform_2(ast *a, void *x) {
			// regex
            auto& child = a->children.front();
            child.match_rule->transform(&child, x);
		}

		void line_transform_1(ast *a, void *x) {
			// entry
            auto& child = a->children.front();
            child.match_rule->transform(&child, x);
		}

		void modifier_transform_1(ast *a, void *x) {
			// std::string
            predicate_modifier& m = *(predicate_modifier *)x;
            m = (predicate_modifier)a->match[0];
		}

		void name_transform_1(ast *a, void *x) {
			// std::string
            std::string& s = *(std::string *)x;
            s = a->match;
		}

		void predicate_transform_1(ast *a, void *x) {
			// modifier? name quantifier?
		}

		void predicates_transform_1(ast *a, void *x) {
			// predicate predicates
		}

		void predicates_transform_2(ast *a, void *x) {
			// predicate
		}

        void quantifier_transform_1(ast *a, void *x) {
            // std::string
            predicate_quantifier& p = *(predicate_quantifier *)x;
            p = a->match == "*" ? q::star :
                a->match == "+" ? q::plus :
                a->match == "?" ? q::question :
                                  q::one;
		}

        void recursive_transform_1(ast *a, void *x) {
            // name rules
            entry_type& entry = *(entry_type *)x;
            auto& name_child = a->children.front();
            auto& rules_child = a->children.back();
            name_child.match_rule->transform(&name_child, &entry.first);
            rules_child.match_rule->transform(&rules_child, &entry.second);
		}

		void regex_transform_1(ast *a, void *x) {
			// name regexre
            entry_type& entry = *(entry_type *)x;
            auto& name_child = a->children.front();
            auto& regexre_child = a->children.back();
            rule::regex_type re;
            name_child.match_rule->transform(&name_child, &entry.first);
            regexre_child.match_rule->transform(&regexre_child, &re);
            entry.second.push_back(re);
		}

		void regexre_transform_1(ast *a, void *x) {
			// std::string
            rule::regex_type& re = *(rule::regex_type *)x;
            re = "\\A" + a->match;
		}

		void rules_transform_1(ast *a, void *x) {
			// predicates rules
            rules& rs = *(rules *)x;
            auto& predicates_child = a->children.front();
            auto& rules_child = a->children.back();
            predicates ps;
            predicates_child.match_rule->transform(&predicates_child, &ps);
            rs.push_back(ps);
            rules_child.match_rule->transform(&rules_child, &rs);
		}

		void rules_transform_2(ast *a, void *x) {
			// predicates
            rules& rs = *(rules *)x;
            auto& child = a->children.front();
            predicates ps;
            child.match_rule->transform(&child, &ps);
            rs.push_back(ps);
		}

	}
}

#undef R
