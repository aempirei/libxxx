#pragma once
#define R(s) rule(rule::terminal_type(s))

namespace xxx {
	namespace local {

		using M = predicate_modifier;
		using Q = predicate_quantifier;

		transform_function M_transform_1;
		transform_function Q_transform_1;
		transform_function composite_transform_1;
		transform_function document_transform_1;
		transform_function entry_transform_1;
		transform_function grammar_transform_1;
		transform_function grammar_transform_2;
		transform_function predicate_transform_1;
		transform_function predicates_transform_1;
		transform_function predicates_transform_2;
		transform_function regex_transform_1;
		transform_function rule_transform_1;
		transform_function rule_transform_2;
		transform_function rules_transform_1;
		transform_function rules_transform_2;
		transform_function terminal_transform_1;
		transform_function var_transform_1;

		grammar spec = grammar({
			{ "M"         , { R("\\A[!>~]") >> M_transform_1 } },
			{ "Q"         , { R("\\A[?*+]") >> Q_transform_1 } },
			{ "_"         , { R("\\A[ \\t]+") } },
			{ "composite" , { rule() << "predicates" >> composite_transform_1 } },
			{ "document"  , { rule() << "grammar" << "eof" << M::discard >> document_transform_1 } },
			{ "entry"     , { rule() << "ws" << M::discard << "var" << "_" << M::discard << "eq" << M::discard << "_" << M::discard << "rules" << "eol" << M::discard >> entry_transform_1 } },
			{ "eof"       , { R("\\A\\z") } },
			{ "eol"       , { R("\\A\\s*(?:$|\\z)") } },
			{ "eq"        , { R("\\A=") } },
			{ "fs"        , { R("\\A\\/") } },
			{ "grammar"   , {
				rule() << "entry" << "grammar" >> grammar_transform_1,
				rule() << "entry" >> grammar_transform_2,
			} },
			{ "predicate" , { rule() << "M" << Q::question << "var" << "Q" << Q::question >> predicate_transform_1 } },
			{ "predicates", {
				rule() << "predicate" << "_" << M::discard << "predicates" >> predicates_transform_1,
				rule() << "predicate" >> predicates_transform_2,
			} },
			{ "regex"     , { R("\\A\\/((?:\\\\.|[^\\/])*)\\/") >> regex_transform_1 } },
			{ "rule"      , {
				rule() << "composite" >> rule_transform_1,
				rule() << "terminal" >> rule_transform_2,
			} },
			{ "rules"     , {
				rule() << "rule" << "_" << M::discard << "fs" << M::discard << "_" << M::discard << "rules" >> rules_transform_1,
				rule() << "rule" >> rules_transform_2,
			} },
			{ "terminal"  , { rule() << "regex" >> terminal_transform_1 } },
			{ "var"       , { R("\\A\\w+") >> var_transform_1 } },
			{ "ws"        , { R("\\A\\s*") } },
		});

		void M_transform_1(tree *a, void *x) {
			// terminal rule : M = /[!>~]/
			*(M *)x = M(a->match);
		}

		void Q_transform_1(tree *a, void *x) {
			// terminal rule : Q = /[?*+]/
			*(Q *)x = Q(a->match);
		}

		void composite_transform_1(tree *a, void *x) {
			predicates arg0;
			auto iter = a->children.begin();
			(iter++)->transform(&arg0);
			if(iter != a->children.end())
				throw std::runtime_error("not all arguments processed by composite rule");
			*(composite *)x = composite(arg0);
		}

		void document_transform_1(tree *a, void *x) {
			grammar arg0;
			auto iter = a->children.begin();
			(iter++)->transform(&arg0);
			if(iter != a->children.end())
				throw std::runtime_error("not all arguments processed by document rule");
			*(document *)x = document(arg0);
		}

		void entry_transform_1(tree *a, void *x) {
			var arg0;
			rules arg1;
			auto iter = a->children.begin();
			(iter++)->transform(&arg0);
			(iter++)->transform(&arg1);
			if(iter != a->children.end())
				throw std::runtime_error("not all arguments processed by entry rule");
			*(entry *)x = entry(arg0,arg1);
		}

		void grammar_transform_1(tree *a, void *x) {
			entry arg0;
			grammar arg1;
			auto iter = a->children.begin();
			(iter++)->transform(&arg0);
			(iter++)->transform(&arg1);
			if(iter != a->children.end())
				throw std::runtime_error("not all arguments processed by grammar rule");
			*(grammar *)x = grammar(arg0,arg1);
		}

		void grammar_transform_2(tree *a, void *x) {
			entry arg0;
			auto iter = a->children.begin();
			(iter++)->transform(&arg0);
			if(iter != a->children.end())
				throw std::runtime_error("not all arguments processed by grammar rule");
			*(grammar *)x = grammar(arg0);
		}

		void predicate_transform_1(tree *a, void *x) {
			M arg0;
			var arg1;
			Q arg2;
			auto iter = a->children.begin();
			if(iter != a->children.end() and iter->match_name() == "M")
				(iter++)->transform(&arg0);
			(iter++)->transform(&arg1);
			if(iter != a->children.end() and iter->match_name() == "Q")
				(iter++)->transform(&arg2);
			if(iter != a->children.end())
				throw std::runtime_error("not all arguments processed by predicate rule");
			*(predicate *)x = predicate(arg0,arg1,arg2);
		}

		void predicates_transform_1(tree *a, void *x) {
			predicate arg0;
			predicates arg1;
			auto iter = a->children.begin();
			(iter++)->transform(&arg0);
			(iter++)->transform(&arg1);
			if(iter != a->children.end())
				throw std::runtime_error("not all arguments processed by predicates rule");
			*(predicates *)x = predicates(arg0,arg1);
		}

		void predicates_transform_2(tree *a, void *x) {
			predicate arg0;
			auto iter = a->children.begin();
			(iter++)->transform(&arg0);
			if(iter != a->children.end())
				throw std::runtime_error("not all arguments processed by predicates rule");
			*(predicates *)x = predicates(arg0);
		}

		void regex_transform_1(tree *a, void *x) {
			// terminal rule : regex = /\/((?:\\.|[^\/])*)\//
			*(regex *)x = regex(a->match);
		}

		void rule_transform_1(tree *a, void *x) {
			composite arg0;
			auto iter = a->children.begin();
			(iter++)->transform(&arg0);
			if(iter != a->children.end())
				throw std::runtime_error("not all arguments processed by rule rule");
			*(rule *)x = rule(arg0);
		}

		void rule_transform_2(tree *a, void *x) {
			terminal arg0;
			auto iter = a->children.begin();
			(iter++)->transform(&arg0);
			if(iter != a->children.end())
				throw std::runtime_error("not all arguments processed by rule rule");
			*(rule *)x = rule(arg0);
		}

		void rules_transform_1(tree *a, void *x) {
			rule arg0;
			rules arg1;
			auto iter = a->children.begin();
			(iter++)->transform(&arg0);
			(iter++)->transform(&arg1);
			if(iter != a->children.end())
				throw std::runtime_error("not all arguments processed by rules rule");
			*(rules *)x = rules(arg0,arg1);
		}

		void rules_transform_2(tree *a, void *x) {
			rule arg0;
			auto iter = a->children.begin();
			(iter++)->transform(&arg0);
			if(iter != a->children.end())
				throw std::runtime_error("not all arguments processed by rules rule");
			*(rules *)x = rules(arg0);
		}

		void terminal_transform_1(tree *a, void *x) {
			regex arg0;
			auto iter = a->children.begin();
			(iter++)->transform(&arg0);
			if(iter != a->children.end())
				throw std::runtime_error("not all arguments processed by terminal rule");
			*(terminal *)x = terminal(arg0);
		}

		void var_transform_1(tree *a, void *x) {
			// terminal rule : var = /\w+/
			*(var *)x = var(a->match);
		}

	}
}

#undef R
