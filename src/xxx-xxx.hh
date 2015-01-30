namespace xxx {
	static grammar define_grammar() {
		grammar g;
		using M = predicate_modifier;
		g["document"] = { { rule() << "_rule" << q::star << M::lift << "eof" << M::discard } 		};
		g["_rule"] = { { rule() << "ws" << M::discard << "rule" << M::lift << "eol" << M::discard } 		};
		g["rule"] = {
			{ rule() << "recursive" },
			{ rule() << "regex" },
		};
		g["recursive"] = { { rule() << "name" << "_" << M::discard << "ceq" << M::discard << "_" << M::discard << "rules" } 		};
		g["regex"] = { { rule() << "name" << "_" << M::discard << "req" << M::discard << "_" << M::discard << "regexre" } 		};
		g["rules"] = {
			{ rule() << "predicates" << "_" << M::discard << "fs" << M::discard << "_" << M::discard << "rules" << M::lift },
			{ rule() << "predicates" },
		};
		g["predicates"] = {
			{ rule() << "predicate" << "_" << M::discard << "predicates" << M::lift },
			{ rule() << "predicate" },
		};
		g["predicate"] = { { rule() << "modifier" << q::question << "name" << "quantifier" << q::question } 		};
		g["fs"] = { rule(rule_type::regex, "\\/") };
		g["qm"] = { rule(rule_type::regex, "\\?") };
		g["_"] = { rule(rule_type::regex, "[ \\t]+") };
		g["eof"] = { rule(rule_type::regex, "\\z") };
		g["eol"] = { rule(rule_type::regex, "\\s*(?:$|\\z)") };
		g["modifier"] = { rule(rule_type::regex, "[!^>~]") };
		g["name"] = { rule(rule_type::regex, "\\w+") };
		g["ws"] = { rule(rule_type::regex, "\\s*") };
		g["quantifier"] = { rule(rule_type::regex, "[*?+]") };
		g["regexre"] = { rule(rule_type::regex, "\\/((?:\\/|[^\\/\\n])*)\\/\\s*$") };
		g["ceq"] = { rule(rule_type::regex, ":=") };
		g["req"] = { rule(rule_type::regex, "~=") };
		g["leq"] = { rule(rule_type::regex, "\"=") };
		g["beq"] = { rule(rule_type::regex, "&=") };
		return g;
	}
}
