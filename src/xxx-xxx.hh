#pragma once
#define R(s) rule(rule::terminal_type(s))

namespace xxx {
	namespace local {

		using M = predicate_modifier;
        using Q = predicate_quantifier;

		transform_function        line_transform_1;
		transform_function    modifier_transform_1;
		transform_function        name_transform_1;
		transform_function   predicate_transform_1;
		transform_function  predicates_transform_1;
		transform_function  predicates_transform_2;
		transform_function  quantifier_transform_1;
		transform_function   recursive_transform_1;
		transform_function       regex_transform_1;
		transform_function     regexre_transform_1;
		transform_function       rules_transform_1;
		transform_function       rules_transform_2;

		grammar spec = grammar({
			{ "_"         , { R("\\A[ \\t]+") } },
			{ "beq"       , { R("\\A&=") } },
			{ "ceq"       , { R("\\A:=") } },
			{ "document"  , { rule() << "line" << Q::star << "eof" << M::discard } },
			{ "entry"     , {
				rule() << "recursive",
				rule() << "regex",
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

        using entry = grammar::value_type;

		void line_transform_1(tree *a, void *x) {
            // entry -> grammar { insert }
            // entry -> grammar { concat } ' FIXME

            entry arg0;
            a->children[0].transform(&arg0);

            // ((grammar *)x)->concat(arg0);

            grammar& g = *(grammar *)x;
            auto& e = g[arg0.first];
            auto& rs = arg0.second;
            e.insert(e.end(), rs.begin(), rs.end());
		}

		void modifier_transform_1(tree *a, void *x) {
			// * .front -> predicate_modifier
            new (x) predicate_modifier((predicate_modifier)a->match.front());
		}

		void name_transform_1(tree *a, void *x) {
            // * -> std::string
            new (x) std::string(a->match);
		}

		void predicate_transform_1(tree *a, void *x) {
			// predicate_modifier? predicate_name predicate_quantifier? -> predicate

            predicate_modifier arg0;
            predicate_name arg1;
            predicate_quantifier arg2;

            auto iter = a->children.begin();

            if(iter != a->children.end() and iter->match_name() == "modifier")
                (iter++)->transform(&arg0);

            if(iter != a->children.end() and iter->match_name() == "name")
                (iter++)->transform(&arg1);

            if(iter != a->children.end() and iter->match_name() == "quantifier")
                (iter++)->transform(&arg2);

            new (x) predicate(arg0, arg1, arg2);
		}

		void predicates_transform_1(tree *a, void *x) {
			// predicate & -> predicates { push_back }

            predicate arg0;
            a->children[0].transform(&arg0);
            ((predicates *)x)->push_back(arg0);

            a->children[1].transform(x);
		}

		void predicates_transform_2(tree *a, void *x) {
			// predicate -> predicates { push_back }

            predicate arg0;
            a->children[0].transform(&arg0);
            ((predicates *)x)->push_back(arg0);
		}

        void quantifier_transform_1(tree *a, void *x) {
            // * -> predicate_quantifier
            new (x) predicate_quantifier(a->match);
		}

        void recursive_transform_1(tree *a, void *x) {
            // var rules -> entry

            var arg0;
            a->children[0].transform(&arg0);

            rules arg1;
            a->children[1].transform(&arg1);

            new (x) entry(arg0, arg1);
		}

        void regex_transform_1(tree *a, void *x) {
            // var rule::terminal_type -> entry ' FIXME

            var arg0;
            a->children[0].transform(&arg0);

            rule::terminal_type arg1;
            a->children[1].transform(&arg1);

            new (x) entry(arg0, { arg1 });
		}

		void regexre_transform_1(tree *a, void *x) {
			// * -> rule::terminal_type
            new (x) rule::terminal_type("\\A" + a->match); // FIXME: handle the prepend
		}

		void rules_transform_1(tree *a, void *x) {
			// predicates & -> rules { push_back }

            predicates arg0;
            a->children[0].transform(&arg0);
            ((rules *)x)->push_back(arg0);

            a->children[1].transform(x);
		}

		void rules_transform_2(tree *a, void *x) {
			// predicates -> rules { push_back }

            predicates arg0;
            a->children[0].transform(&arg0);
            ((rules *)x)->push_back(arg0);
		}

	}
}

#undef R
