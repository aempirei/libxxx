namespace xxx {
	static grammar define_grammar() {
		using M = predicate_modifier;
		return grammar({
			{ "_"         , { rule::regex_type("\\A[ \\t]+") } },
			{ "beq"       , { rule::regex_type("\\A&=") } },
			{ "ceq"       , { rule::regex_type("\\A:=") } },
			{ "document"  , { rule() << "line" << q::star << M::lift << "eof" << M::discard } },
			{ "eof"       , { rule::regex_type("\\A\\z") } },
			{ "eol"       , { rule::regex_type("\\A\\s*(?:$|\\z)") } },
			{ "expr"      , {
				rule() << "recursive",
				rule() << "regex"
			} },
			{ "fs"        , { rule::regex_type("\\A\\/") } },
			{ "leq"       , { rule::regex_type("\\A\"=") } },
			{ "line"      , { rule() << "ws" << M::discard << "expr" << M::lift << "eol" << M::discard } },
			{ "modifier"  , { rule::regex_type("\\A[!^>~]") } },
			{ "name"      , { rule::regex_type("\\A\\w+") } },
			{ "predicate" , { rule() << "modifier" << q::question << "name" << "quantifier" << q::question } },
			{ "predicates", {
				rule() << "predicate" << "_" << M::discard << "predicates" << M::lift,
				rule() << "predicate"
			} },
			{ "qm"        , { rule::regex_type("\\A\\?") } },
			{ "quantifier", { rule::regex_type("\\A[*?+]") } },
			{ "recursive" , { rule() << "name" << "_" << M::discard << "ceq" << M::discard << "_" << M::discard << "rules" } },
			{ "regex"     , { rule() << "name" << "_" << M::discard << "req" << M::discard << "_" << M::discard << "regexre" } },
			{ "regexre"   , { rule::regex_type("\\A\\/((?:\\/|[^\\/\\n])*)\\/\\s*$") } },
			{ "req"       , { rule::regex_type("\\A~=") } },
			{ "rules"     , {
				rule() << "predicates" << "_" << M::discard << "fs" << M::discard << "_" << M::discard << "rules" << M::lift,
				rule() << "predicates"
			} },
			{ "ws"        , { rule::regex_type("\\A\\s*") } },
		});
	}
	namespace transform {
		struct document;
		struct expr;
		struct line;
		struct modifier;
		struct name;
		struct predicate;
		struct predicates;
		struct quantifier;
		struct recursive;
		struct regex;
		struct regexre;
		struct rules;
		struct document {
			document(const std::list<line>&);
		};
		struct expr {
			expr(const recursive&);
			expr(const regex&);
		};
		struct line {
			line(const expr&);
		};
		struct modifier {
			modifier(const std::string&);
		};
		struct name {
			name(const std::string&);
		};
		struct predicate {
			predicate(const modifier *, const name&, const quantifier *);
		};
		struct predicates {
			predicates(const predicate&, const predicates&);
			predicates(const predicate&);
		};
		struct quantifier {
			quantifier(const std::string&);
		};
		struct recursive {
			recursive(const name&, const rules&);
		};
		struct regex {
			regex(const name&, const regexre&);
		};
		struct regexre {
			regexre(const std::string&);
		};
		struct rules {
			rules(const predicates&, const rules&);
			rules(const predicates&);
		};
	}
}
