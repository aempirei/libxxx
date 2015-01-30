namespace xxx {
	static grammar define_grammar() {
		grammar g;
		using M = predicate_modifier;
		g["_"] = { rule::regex_type("\\A[ \\t]+") };
		g["_rule"] = { rule() << "ws" << M::discard << "rule" << M::lift << "eol" << M::discard };
		g["beq"] = { rule::regex_type("\\A&=") };
		g["ceq"] = { rule::regex_type("\\A:=") };
		g["document"] = { rule() << "_rule" << q::star << M::lift << "eof" << M::discard };
		g["eof"] = { rule::regex_type("\\A\\z") };
		g["eol"] = { rule::regex_type("\\A\\s*(?:$|\\z)") };
		g["fs"] = { rule::regex_type("\\A\\/") };
		g["leq"] = { rule::regex_type("\\A\"=") };
		g["modifier"] = { rule::regex_type("\\A[!^>~]") };
		g["name"] = { rule::regex_type("\\A\\w+") };
		g["predicate"] = { rule() << "modifier" << q::question << "name" << "quantifier" << q::question };
		g["predicates"] = {
			rule() << "predicate" << "_" << M::discard << "predicates" << M::lift,
			rule() << "predicate",
		};
		g["qm"] = { rule::regex_type("\\A\\?") };
		g["quantifier"] = { rule::regex_type("\\A[*?+]") };
		g["recursive"] = { rule() << "name" << "_" << M::discard << "ceq" << M::discard << "_" << M::discard << "rules" };
		g["regex"] = { rule() << "name" << "_" << M::discard << "req" << M::discard << "_" << M::discard << "regexre" };
		g["regexre"] = { rule::regex_type("\\A\\/((?:\\/|[^\\/\\n])*)\\/\\s*$") };
		g["req"] = { rule::regex_type("\\A~=") };
		g["rule"] = {
			rule() << "recursive",
			rule() << "regex",
		};
		g["rules"] = {
			rule() << "predicates" << "_" << M::discard << "fs" << M::discard << "_" << M::discard << "rules" << M::lift,
			rule() << "predicates",
		};
		g["ws"] = { rule::regex_type("\\A\\s*") };
		return g;
	}
}
