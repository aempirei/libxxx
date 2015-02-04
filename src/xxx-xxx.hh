#pragma once
#define R(s) rule(rule::terminal_type(s))

namespace xxx {
	namespace local {

		using M = predicate_modifier;
        using Q = predicate_quantifier;

		transform_function  rules_transform_1;
		transform_function  rules_transform_2;
		transform_function  quantifier_transform_1;
		transform_function   recursive_transform_1;
		transform_function       regex_transform_1;
		transform_function     regexre_transform_1;

		grammar spec = grammar({

			{ "_"         , { R("\\A[ \\t]+") } },
			{ "eq"        , { R("\\A=") } },
			{ "document"  , { rule() << "line" << Q::star << "eof" << M::discard } },

            { "entry"     , {
                rule() << "recursive",
                rule() << "regex"
            } },

            { "eof"       , { R("\\A\\z") } },
            { "eol"       , { R("\\A\\s*(?:$|\\z)") } },
            { "fs"        , { R("\\A\\/") } },

            // entry -> grammar .concat

            { "line"      , { rule() << "ws" << M::discard << "entry" << "eol" << M::discard >> [](tree *a, void *x) {
                entry arg0;
                a->children[0].transform(&arg0);
                ((grammar *)x)->concat(arg0);
            } } },

            // * .front -> predicate_modifier ' FIXME: deal with cast
            // * -> std::string

            { "modifier"  , { R("\\A[!^>~]") >> [](tree *a, void *x) { new (x) predicate_modifier((predicate_modifier)a->match.front()); } } },
            { "var"       , { R("\\A(?:::|\\w)*\\w") >> [](tree *a, void *x) { new (x) std::string(a->match); } } },

            // predicate_modifier? predicate_name predicate_quantifier? -> predicate

			{ "predicate" , { rule() << "modifier" << Q::question << "var" << "quantifier" << Q::question >> [](tree *a, void *x) {

                predicate_modifier arg0;
                predicate_name arg1;
                predicate_quantifier arg2;

                auto iter = a->children.begin();

                if(iter != a->children.end() and iter->match_name() == "modifier")
                    (iter++)->transform(&arg0);

                if(iter != a->children.end() and iter->match_name() == "var")
                    (iter++)->transform(&arg1);

                if(iter != a->children.end() and iter->match_name() == "quantifier")
                    (iter++)->transform(&arg2);

                new (x) predicate(arg0, arg1, arg2);

            } } },

            // predicate & -> predicates .push_back
            // predicate -> predicates .push_back

			{ "predicates", {

				rule() << "predicate" << "_" << M::discard << "predicates" >> [](tree *a, void *x) {

                    predicate arg0;
                    a->children[0].transform(&arg0);
                    ((predicates *)x)->push_back(arg0);

                    a->children[1].transform(x);
                },

				rule() << "predicate" >> [](tree *a, void *x) {

                    predicate arg0;
                    a->children[0].transform(&arg0);
                    ((predicates *)x)->push_back(arg0);
                }
			} },

			{ "qm"        , { R("\\A\\?") } },
			{ "quantifier", { R("\\A[*?+]") >> quantifier_transform_1 } },
			{ "recursive" , { rule() << "var" << "_" << M::discard << "eq" << M::discard << "_" << M::discard << "rules" >> recursive_transform_1 } },
			{ "regex"     , { rule() << "var" << "_" << M::discard << "eq" << M::discard << "_" << M::discard << "regexre" >> regex_transform_1 } },
			{ "regexre"   , { R("\\A\\/((?:\\/|[^\\/\\n])*)\\/\\s*$") >> regexre_transform_1 } },

			{ "rules"     , {
				rule() << "predicates" << "_" << M::discard << "fs" << M::discard << "_" << M::discard << "rules" >> rules_transform_1,
				rule() << "predicates" >> rules_transform_2,
			} },

			{ "ws"        , { R("\\A\\s*") } },
		});

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
            // var rule::terminal_type {} -> entry

            var arg0;
            a->children[0].transform(&arg0);

            rule::terminal_type arg1;
            a->children[1].transform(&arg1);

            new (x) entry(arg0, { arg1 });
		}

		void regexre_transform_1(tree *a, void *x) {
			// * -> rule::terminal_type
            new (x) rule::terminal_type(a->match);
		}

		void rules_transform_1(tree *a, void *x) {
			// predicates & -> rules .push_back

            predicates arg0;
            a->children[0].transform(&arg0);
            ((rules *)x)->push_back(arg0);

            a->children[1].transform(x);
		}

		void rules_transform_2(tree *a, void *x) {
			// predicates -> rules .push_back

            predicates arg0;
            a->children[0].transform(&arg0);
            ((rules *)x)->push_back(arg0);
		}

	}
}

#undef R
