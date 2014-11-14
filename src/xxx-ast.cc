#include <xxx.hh>

namespace xxx {

	static range<ssize_t> parse_recursive(const grammar&, std::string, const std::string&, ast&, ssize_t);
	static std::string ast_str_recursive(const ast&, int, bool);
	static std::string ast_xml_recursive(const ast&, int, int, int);

	ast::ast() {
	}

	ast::ast(const grammar& g, FILE *fp) {
		parse(g, fp);
	}

	ast::ast(const grammar& g, const std::string& s) {
		parse(g, s);
	}

	template<class T, class F> static size_t sum(int sum0, const T& xs, F f) {

		if(xs.empty())
			return 1;

		size_t sum = sum0;

		for(const auto& x : xs)
			sum += f(x);

		return sum;

	}

	size_t ast::node_count() const {
		return sum(1, children, [](const ast& x) -> size_t { return x.node_count(); });
	}

	size_t ast::leaf_count() const {
		return sum(0, children, [](const ast& x) -> size_t { return x.node_count(); });
	}

	void ast::parse(const grammar& g, FILE *fp) {

		std::string s;

		char buf[512];

		while(!feof(fp)) {

			int n = fread(buf, 1, sizeof(buf), fp);

			if(n != sizeof(buf) && ferror(fp)) {
				std::string s("fread(): ");
				s += strerror(errno);
				throw new std::runtime_error(s);
			}

			s.append(buf, n);
		}

		parse(g, s);
	}

	void ast::parse(const grammar& g, const std::string& s) {

		range<ssize_t> ab = parse_recursive(g, "document", s, *this, 0);

		if(ab.first == -1) {

			std::stringstream ss;
			ss << "ast::parse failed at offset " << ab.second;

			std::cerr  << s.substr(ab.second, std::string::npos);

			throw std::runtime_error(ss.str());
		}
	}

	static range<ssize_t> parse_recursive(const grammar& g, std::string name, const std::string& s, ast& q, ssize_t offset) {

		const auto iter = g.find(name);

		if(iter == g.end()) {
			std::stringstream ss;
			ss << "grammar rule not found -- \"" << name << '"';
			throw std::runtime_error(ss.str());
		}

		const auto& rules = iter->second;

		ssize_t current = offset;

		q.offset = offset;
		q.name = name;

		for(const auto& rule : rules) {

			bool success = true;

			boost::smatch matches;
			std::string ms;

			current = offset;

			q.children.clear();
			q.type = rule.type;

			switch(rule.type) {

				case rule_type::terminal:

					ms = s.substr(offset, std::string::npos);

					if(!boost::regex_search(ms, matches, rule.terminal)) {
						success = false;
						break;
					}

					q.matches.resize(matches.size());

					for(size_t n = 0; n < matches.size(); n++)
						q.matches[n] = matches[n];

					current += q.matches[0].size();

					break;

				case rule_type::recursive:

					for(const auto& predicate : rule.recursive) {

						size_t n;

						for(n = 0; n < predicate.quantifier.second; n++) {

							ast qq;

							auto next = parse_recursive(g, predicate.name, s, qq, current);

							if(next.first == -1)
								break;

							if(predicate.modifier == predicate_modifier::push) {

								q.children.push_back(qq);

							} else if(predicate.modifier == predicate_modifier::lift) {

								if(qq.type != rule_type::recursive) {
									throw new std::runtime_error(
											"attempting to lift non-recursive ast node"
											);
								}

								for(const auto& qqq : qq.children)
									q.children.push_back(qqq);

							} else if(predicate.modifier == predicate_modifier::peek) {

								next.second = current;

							} else if(predicate.modifier == predicate_modifier::discard) {

								// discard
							}

							current = next.second;
						}

						if(n < predicate.quantifier.first) {
							success = false;
							break;
						}
					}

					break;

				default:

					throw std::runtime_error("undefined rule type");
			}

			if(success)
				return range<ssize_t>(offset,current);
		}

		return range<ssize_t>(-1,current);
	}

	static std::string ast_str_recursive(const ast& q, int depth=0, bool basic = false) {

		std::stringstream ss;

		if(basic) {
			ss << q.name;
		} else {
			ss << std::setw(4) << q.offset << " " << std::setw(depth) << "" << q.name;
		}
		switch(q.type) {
			case rule_type::terminal:
				ss << " =~ " << '"' << q.matches[0] << '"' << std::endl;
				break;
			case rule_type::recursive:

				if(q.children.size() == 1) {
					ss << ' ' << ast_str_recursive(q.children.back(), depth + 2, true);
				} else {

					ss << std::endl;

					for(const auto& qq : q.children)
						ss << ast_str_recursive(qq, depth + 2);
				}

				break;
		}

		return ss.str();
	}

	std::string ast::str() const {
		return ast_str_recursive(*this);
	}

	static std::string ast_xml_recursive(const ast& q, int df = 0, int dm = 1, int dl = 0) {


		const std::string xmlns = "xmlns:xxx=\"http://www.256.bz/xmlns/xxx\"";

		std::stringstream ss;

		std::string content;
		std::string tag = q.name;

		if(!tag.empty()) {
			if(isdigit(tag[0])) {
				tag = "_" + tag;
			} else if(tag[0] == '-' || tag[0] == '.') {
				tag = "xxx" + tag;
			}
		}

		ss << std::string(df, ' ') << '<' << tag;
		if(tag == "document")
			ss << ' ' << xmlns;

		switch(q.type) {

			case rule_type::terminal:

				content = q.matches[0];

				content = boost::regex_replace(content, boost::regex("&"), "&amp;");
				content = boost::regex_replace(content, boost::regex("<"), "&lt;");
				content = boost::regex_replace(content, boost::regex(">"), "&gt;");
				content = boost::regex_replace(content, boost::regex("\""), "&quot;");
				content = boost::regex_replace(content, boost::regex("'"), "&apos;");

				ss << '>' << content << "</" << tag << '>';

				break;

			case rule_type::recursive:

				if(q.children.empty()) {

					ss << " />";

				} else {

					ss << '>';

					if(q.leaf_count() == 1) {

						ss << ast_xml_recursive(q.children.back());
						ss << "</" << tag << '>';

					} else if(q.children.size() == 1) {

						ss << ast_xml_recursive(q.children.back(), 0, dm, dl);
						ss << "</" << tag << '>';

					} else {

						ss << std::endl;

						for(const auto& qq : q.children) {
							ss << ast_xml_recursive(qq, dm, dm + 1, dl + 1) << std::endl;
						}

						ss << std::string(dl, ' ') << "</" << tag << '>';
					}
				}

				break;
		}

		return ss.str();
	}


	std::string ast::xml() const {
		const std::string xml_header = "<?xml version=\"1.0\"?>\n";

		return xml_header + ast_xml_recursive(*this);
	}

	std::string ast_code_recursive(const ast& a) {

		std::stringstream ss;

		ss << "static xxx::grammar define_grammar() {" << std::endl;
		ss << "using namespace xxx;" << std::endl;
		ss << "grammar g;" << std::endl;
		ss << "using R = rule;" << std::endl;
		ss << "using M = predicate_modifier;" << std::endl;
		ss << "const auto N = rule_type::recursive;" << std::endl;
		ss << "const auto T = rule_type::terminal;" << std::endl;
		if(a.name == "document") {
			for(const auto& b : a.children) {

				const auto& name = b.children[0].matches[0];

				if(b.name == "terminal") {

					std::string reg = b.children[1].matches[0].substr(1,std::string::npos);

					reg.pop_back();

					std::string escreg;

					for(size_t n = 0; n < reg.length(); n++) {
						if(reg[n] == '\\' or reg[n] == '"')
							escreg.push_back('\\');
						escreg.push_back(reg[n]);
					}

					ss << "g[\"" << name << "\"] = { R(T) << \"" << escreg << "\" };" << std::endl;

				} else {

					ss << "g[\"" << name << "\"] = { ";

					// ordered

					bool single = (b.children[1].children.size() == 1);

					for(const auto& c : b.children[1].children) {

						// predicates

						if(not single)
							ss << std::endl;

						ss << "{ R(N) ";

						for(const auto& d : c.children) {

							// predicate

							auto iter = d.children.begin();

							predicate p;

							if(iter->name == "modifier") {
								if(iter->matches[0] == "^") {
									p.modifier = predicate_modifier::lift;
								} else if(iter->matches[0] == "!") {
									p.modifier = predicate_modifier::discard;
								} else if(iter->matches[0] == ">") {
									p.modifier = predicate_modifier::peek;
								}

								iter++;
							} else {
								p.modifier = predicate_modifier::push;
							}

							if(iter->name == "name") {
								p.name = iter->matches[0];
								iter++;
							}

							if(iter != d.children.end() && iter->name == "quantifier") {
								if(iter->matches[0] == "*") {
									p.quantifier = q::star;
								} else if(iter->matches[0] == "+") {
									p.quantifier = q::plus;
								} else if(iter->matches[0] == "?") {
									p.quantifier = q::question;
								}
							} else {
								p.quantifier = q::one;
							}

							ss << "<< \"" << p.name << "\" ";

							if(p.quantifier != q::one) {
								ss << (
										(p.quantifier == q::star    ) ? "<< q::star "     :
										(p.quantifier == q::plus    ) ? "<< q::plus "     :
										(p.quantifier == q::question) ? "<< q::question " : "");
							}

							if(p.modifier != predicate_modifier::push) {
								ss << (
										(p.modifier == predicate_modifier::lift   ) ? "<< M::lift "    :
										(p.modifier == predicate_modifier::discard) ? "<< M::discard " :
										(p.modifier == predicate_modifier::peek   ) ? "<< M::peek "    : "");
							}
						}

						if(single)
							ss << "} ";
						else
							ss << "},";
					}

					if(not single)
						ss << std::endl;

					ss << "};" << std::endl;
				}
			}
		}

		ss << "return g;" << std::endl;
		ss << "}" << std::endl;

		return ss.str();
	}

	std::string ast::code() const {
		return ast_code_recursive(*this);
	}


}
