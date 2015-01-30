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
			{ "document"  , { rule() << "line" << q::star << M::lift << "eof" << M::discard >> document_transform_1 } },
			{ "entry"     , {
				rule() << "recursive" >> entry_transform_1,
				rule() << "regex" >> entry_transform_2,
			} },
			{ "eof"       , { R("\\A\\z") } },
			{ "eol"       , { R("\\A\\s*(?:$|\\z)") } },
			{ "fs"        , { R("\\A\\/") } },
			{ "leq"       , { R("\\A\"=") } },
			{ "line"      , { rule() << "ws" << M::discard << "entry" << M::lift << "eol" << M::discard >> line_transform_1 } },
			{ "modifier"  , { R("\\A[!^>~]") >> modifier_transform_1 } },
			{ "name"      , { R("\\A\\w+") >> name_transform_1 } },
			{ "predicate" , { rule() << "modifier" << q::question << "name" << "quantifier" << q::question >> predicate_transform_1 } },
			{ "predicates", {
				rule() << "predicate" << "_" << M::discard << "predicates" << M::lift >> predicates_transform_1,
				rule() << "predicate" >> predicates_transform_2,
			} },
			{ "qm"        , { R("\\A\\?") } },
			{ "quantifier", { R("\\A[*?+]") >> quantifier_transform_1 } },
			{ "recursive" , { rule() << "name" << "_" << M::discard << "ceq" << M::discard << "_" << M::discard << "rules" >> recursive_transform_1 } },
			{ "regex"     , { rule() << "name" << "_" << M::discard << "req" << M::discard << "_" << M::discard << "regexre" >> regex_transform_1 } },
			{ "regexre"   , { R("\\A\\/((?:\\/|[^\\/\\n])*)\\/\\s*$") >> regexre_transform_1 } },
			{ "req"       , { R("\\A~=") } },
			{ "rules"     , {
				rule() << "predicates" << "_" << M::discard << "fs" << M::discard << "_" << M::discard << "rules" << M::lift >> rules_transform_1,
				rule() << "predicates" >> rules_transform_2,
			} },
			{ "ws"        , { R("\\A\\s*") } },
		});

		void document_transform_1(ast *a) {
			// [line]
		}

		void entry_transform_1(ast *a) {
			// recursive
		}

		void entry_transform_2(ast *a) {
			// regex
		}

		void line_transform_1(ast *a) {
			// entry
		}

		void modifier_transform_1(ast *a) {
			// std::string
		}

		void name_transform_1(ast *a) {
			// std::string
		}

		void predicate_transform_1(ast *a) {
			// modifier? name quantifier?
		}

		void predicates_transform_1(ast *a) {
			// predicate predicates
		}

		void predicates_transform_2(ast *a) {
			// predicate
		}

		void quantifier_transform_1(ast *a) {
			// std::string
		}

		void recursive_transform_1(ast *a) {
			// name rules
		}

		void regex_transform_1(ast *a) {
			// name regexre
		}

		void regexre_transform_1(ast *a) {
			// std::string
		}

		void rules_transform_1(ast *a) {
			// predicates rules
		}

		void rules_transform_2(ast *a) {
			// predicates
		}

	}
}

#undef R
