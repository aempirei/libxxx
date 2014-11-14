static xxx::grammar define_grammar() {

	using namespace xxx;

	using R = rule;
	using M = predicate_modifier;

	const auto N = rule_type::recursive;
	const auto T = rule_type::terminal;

	grammar g;

	g["document"]	= { { R(N) << "_rule" << q::star << M::lift << "eof" << M::discard } };
	g["_rule"]		= { { R(N) << "ws" << M::discard << "rule" << M::lift << "eol" << M::discard } };
	g["recursive"]	= { { R(N) << "name" << "_" << M::discard << "ceq" << M::discard << "_" << M::discard << "ordered" } };
	g["terminal"]	= { { R(N) << "name" << "_" << M::discard << "req" << M::discard << "_" << M::discard << "regex" } };
	g["predicate"]	= { { R(N) << "modifier" << q::question << "name" << "quantifier" << q::question } };

	g["rule"]		= { 
		{ R(N) << "recursive" },
		{ R(N) << "terminal" },
	};
	g["ordered"]	= { 
		{ R(N) << "predicates" << "_" << M::discard << "fs" << M::discard << "_" << M::discard << "ordered" << M::lift },
		{ R(N) << "predicates" },
	};
	g["predicates"]	= { 
		{ R(N) << "predicate" << "_" << M::discard << "predicates" << M::lift },
		{ R(N) << "predicate" },
	};

	g["fs"]			= { R(T) << "\\/" };
	g["ceq"]		= { R(T) << ":=" };
	g["qm"]			= { R(T) << "\\?" };
	g["_"]			= { R(T) << "[ \\t]+" };
	g["eof"]		= { R(T) << "\\z" };
	g["eol"]		= { R(T) << "\\s*($|\\z)" };
	g["modifier"]	= { R(T) << "[!^>]" };
	g["name"]		= { R(T) << "[-.\\w]+" };
	g["quantifier"]	= { R(T) << "[*?+]" };
	g["regex"]		= { R(T) << "/(\\/|[^\\/\\n])*/" };
	g["ws"]			= { R(T) << "\\s*" };
	g["req"]		= { R(T) << "\\~=" };

	return g;
}
