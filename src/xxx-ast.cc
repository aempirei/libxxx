#include <xxx.hh>

namespace xxx {

	static std::pair<ssize_t,ssize_t> parse_recursive(const grammar&, const var&, const std::string&, ast&, ssize_t);
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

	template<typename T, typename F> static size_t sum(size_t sum0, size_t sum, const T& xs, F f) {

		if(xs.empty())
			return sum0;

		for(const auto& x : xs)
			sum += (x.*f)();

		return sum;

	}

	size_t ast::node_count() const {
		return sum(1, 1, children, &ast::node_count);
	}

	size_t ast::leaf_count() const {
		return sum(1, 0, children, &ast::leaf_count);
	}

	void ast::parse(const grammar& g, FILE *fp) {

		std::string s;

		char buf[1024];

		while(not feof(fp)) {

			int n = fread(buf, 1, sizeof(buf), fp);

			if(n != sizeof(buf) and ferror(fp)) {
				std::string s("fread(): ");
				s += strerror(errno);
				throw new std::runtime_error(s);
			}

			s.append(buf, n);
		}

		parse(g, s);
	}

	void ast::parse(const grammar& g, const std::string& s) {

        auto ab = parse_recursive(g, "document", s, *this, 0);

		if(ab.first == -1) {

			std::stringstream ss;
			ss << "ast::parse failed at offset " << ab.second << " (";

			if(ab.second + 16 < (ssize_t)s.length()) {
				ss << s.substr(ab.second, ab.second + 16) << "...";
			} else {
				ss << s.substr(ab.second, std::string::npos);
			}

			ss << ")";

			throw std::runtime_error(ss.str());
		}
	}

	static std::pair<ssize_t,ssize_t> parse_recursive(const grammar& g, const var& name, const std::string& s, ast& x, ssize_t offset) {

        x.entry = g.find(name);

		if(x.entry == g.end()) {
			std::stringstream ss;
			ss << "grammar rule not found -- \"" << name << '"';
			throw std::runtime_error(ss.str());
		}

		const auto& rules = x.entry->second;

		ssize_t current = offset;

		x.offset = offset;
		x.name = name;

        for(x.subentry = rules.begin(); x.subentry != rules.end(); x.subentry++) {

            const auto& rule = *x.subentry;

			bool success = true;

			boost::smatch matches;
			std::string ms;

			current = offset;

			x.children.clear();
			x.type = rule.type;

			switch(rule.type) {

                case rule_type::literal:

                    ms = s.substr(offset, rule.literal.length());

                    if(ms != rule.literal) {
                        success = false;
                        break;
                    }

                    x.matches.resize(1);
                    x.matches[0] = ms;
                    current += x.matches[0].length();

                    break;

                case rule_type::builtin:

                    ms = s.substr(offset, 1);

                    if(ms.empty() or not rule.builtin(ms[0])) {
                        success = false;
                        break;
                    }

                    x.matches.resize(1);
                    x.matches[0] = ms;
                    current += x.matches[0].length();

                    break;

				case rule_type::regex:

					ms = s.substr(offset, std::string::npos);

					if(not boost::regex_search(ms, matches, rule.regex)) {
						success = false;
						break;
					}

                    x.matches.resize(1);
                    x.matches[0] = matches[matches.size() > 1 ? 1 : 0];
					current += matches[0].length();

					break;

				case rule_type::recursive:

					for(const predicate& p : rule.recursive) {

						size_t n;

						for(n = 0; n < p.quantifier.second; n++) {

							ast y;

							auto next = parse_recursive(g, p.name, s, y, current);

							if(next.first == -1)
								break;

							if(p.modifier == predicate_modifier::push) {

								x.children.push_back(y);

							} else if(p.modifier == predicate_modifier::lift) {

								if(y.type != rule_type::recursive) {
									throw new std::runtime_error(
											"attempting to lift non-recursive ast node"
											);
								}

								for(const auto& z : y.children)
									x.children.push_back(z);

							} else if(p.modifier == predicate_modifier::peek) {

								next.second = current;

							} else if(p.modifier == predicate_modifier::discard) {

								// discard
							}

							current = next.second;
						}

						if(n < p.quantifier.first) {
							success = false;
							break;
						}
					}

					break;
			}

			if(success)
				return std::pair<ssize_t,ssize_t>(offset,current);
		}

		return std::pair<ssize_t,ssize_t>(-1,current);
	}

	static std::string ast_str_recursive(const ast& x, int depth = 0, bool basic = false) {

		std::stringstream ss;

		if(basic) {
			ss << x.name;
		} else {
			ss << std::setw(4) << x.offset << " " << std::setw(depth) << "" << x.name;
		}
		switch(x.type) {

            case rule_type::literal: ss << ' ' << '"' << "= " << x.matches[0] << std::endl; break;
			case rule_type::builtin: ss << ' ' << '&' << "= " << x.matches[0] << std::endl; break;
			case rule_type::regex  : ss << ' ' << '~' << "= " << x.matches[0] << std::endl; break;

			case rule_type::recursive:

				if(x.children.size() == 1) {
					ss << ' ' << ast_str_recursive(x.children.back(), depth + 2, true);
				} else {

					ss << std::endl;

					for(const auto& y : x.children)
						ss << ast_str_recursive(y, depth + 2);
				}

				break;
		}

		return ss.str();
	}

	std::string ast::str() const {
		return ast_str_recursive(*this);
	}

	static std::string ast_xml_recursive(const ast& x, int df = 0, int dm = 1, int dl = 0) {


		const std::string xmlns = "xmlns:xxx=\"http://www.256.bz/xmlns/xxx\"";

		std::stringstream ss;

		std::string content;
		std::string tag = x.name;

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

		switch(x.type) {

			case rule_type::literal:
			case rule_type::builtin:
			case rule_type::regex:

				content = x.matches[0];

				content = boost::regex_replace(content, boost::regex("&"), "&amp;");
				content = boost::regex_replace(content, boost::regex("<"), "&lt;");
				content = boost::regex_replace(content, boost::regex(">"), "&gt;");
				content = boost::regex_replace(content, boost::regex("\""), "&quot;");
				content = boost::regex_replace(content, boost::regex("'"), "&apos;");

				ss << '>' << content << "</" << tag << '>';

				break;

			case rule_type::recursive:

				if(x.children.empty()) {

					ss << " />";

				} else {

					ss << '>';

					if(x.leaf_count() == 1) {

						ss << ast_xml_recursive(x.children.back());
						ss << "</" << tag << '>';

					} else if(x.children.size() == 1) {

						ss << ast_xml_recursive(x.children.back(), 0, dm, dl);
						ss << "</" << tag << '>';

					} else {

						ss << std::endl;

						for(const auto& y : x.children) {
							ss << ast_xml_recursive(y, dm, dm + 1, dl + 1) << std::endl;
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
}
