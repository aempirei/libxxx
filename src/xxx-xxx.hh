#pragma once
#define R(s) rule(rule::terminal_type(s))

namespace xxx {
	namespace local {

		using M = predicate::M;
		using Q = predicate::Q;

		transform_function document_transform_1;
		transform_function entry_transform_1;
		transform_function grammar_transform_1;
		transform_function grammar_transform_2;
		transform_function pm_transform_1;
		transform_function pq_transform_1;
		transform_function predicate_transform_1;
		transform_function predicates_transform_1;
		transform_function predicates_transform_2;
		transform_function regex_transform_1;
		transform_function repl_transform_1;
		transform_function rule_transform_1;
		transform_function rule_transform_2;
		transform_function rules_transform_1;
		transform_function rules_transform_2;
		transform_function var_transform_1;

		grammar spec = grammar({
			{ "_"         , { R("\\A[ \\t]+") } },
			{ "document"  , { rule() << "grammar" << "eof" << M::drop >> document_transform_1 } },
			{ "entry"     , { rule() << "ws" << M::drop << "var" << "_" << M::drop << "eq" << M::drop << "_" << M::drop << "rules" << "eol" << M::drop >> entry_transform_1 } },
			{ "eof"       , { R("\\A\\z") } },
			{ "eol"       , { R("\\A\\s*(?:$|\\z)") } },
			{ "eq"        , { R("\\A=") } },
			{ "fs"        , { R("\\A\\/") } },
			{ "grammar"   , {
				rule() << "entry" << "grammar" >> grammar_transform_1,
				rule() << "entry" >> grammar_transform_2,
			} },
			{ "pm"        , { R("\\A[=!>]") >> pm_transform_1 >> "head" } },
			{ "pq"        , { R("\\A\\?") >> pq_transform_1 >> "head" } },
			{ "predicate" , { rule() << "pm" << Q::maybe << "var" << "pq" << Q::maybe >> predicate_transform_1 } },
			{ "predicates", {
				rule() << "predicate" << "_" << M::drop << "predicates" >> predicates_transform_1,
				rule() << "predicate" >> predicates_transform_2,
			} },
			{ "ra"        , { R("\\A->") } },
			{ "regex"     , { R("\\A\\/((?:\\\\.|[^\\/])*)\\/") >> regex_transform_1 } },
			{ "repl"      , { rule() << "_" << M::drop << "ra" << M::drop << "_" << M::drop << "var" >> repl_transform_1 } },
			{ "rule"      , {
				rule() << "predicates" >> rule_transform_1,
				rule() << "regex" << "repl" << Q::maybe >> rule_transform_2,
			} },
			{ "rules"     , {
				rule() << "rule" << "_" << M::drop << "fs" << M::drop << "_" << M::drop << "rules" >> rules_transform_1,
				rule() << "rule" >> rules_transform_2,
			} },
			{ "var"       , { R("\\A\\w+") >> var_transform_1 } },
			{ "ws"        , { R("\\A\\s*") } },
		});

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

		void pm_transform_1(tree *a, void *x) {
			// terminal rule : pm = /[=!>]/
			*(pm *)x = pm(head(a->match));
		}

		void pq_transform_1(tree *a, void *x) {
			// terminal rule : pq = /\?/
			*(pq *)x = pq(head(a->match));
		}

		void predicate_transform_1(tree *a, void *x) {
			pm arg0;
			var arg1;
			pq arg2;
			auto iter = a->children.begin();
			if(iter != a->children.end() and iter->match_name == "pm")
				(iter++)->transform(&arg0);
			(iter++)->transform(&arg1);
			if(iter != a->children.end() and iter->match_name == "pq")
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
			*(regex *)x = regex((a->match));
		}

		void repl_transform_1(tree *a, void *x) {
			var arg0;
			auto iter = a->children.begin();
			(iter++)->transform(&arg0);
			if(iter != a->children.end())
				throw std::runtime_error("not all arguments processed by repl rule");
			*(repl *)x = repl(arg0);
		}

		void rule_transform_1(tree *a, void *x) {
			predicates arg0;
			auto iter = a->children.begin();
			(iter++)->transform(&arg0);
			if(iter != a->children.end())
				throw std::runtime_error("not all arguments processed by rule rule");
			*(rule *)x = rule(arg0);
		}

		void rule_transform_2(tree *a, void *x) {
			regex arg0;
			repl arg1;
			auto iter = a->children.begin();
			(iter++)->transform(&arg0);
			if(iter != a->children.end() and iter->match_name == "repl")
				(iter++)->transform(&arg1);
			if(iter != a->children.end())
				throw std::runtime_error("not all arguments processed by rule rule");
			*(rule *)x = rule(arg0,arg1);
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

		void var_transform_1(tree *a, void *x) {
			// terminal rule : var = /\w+/
			*(var *)x = var((a->match));
		}

	}
}

#undef R
