#include <xxx.hh>

namespace xxx {

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

    void *ast::transform() {
        void *result;
        match_rule->transform(this, &result);
        return result;
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

        auto ab = parse_recursive(g, "document", s, 0);

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

    const var& ast::name() const {
        return match_entry->first;
    }

    std::pair<ssize_t,ssize_t> ast::parse_recursive(const grammar& g, const var& my_name, const std::string& s, ssize_t my_offset) {

        offset = my_offset;

        match_entry = g.find(my_name);

		if(match_entry == g.end())
            throw std::runtime_error("grammar rule not found -- \"" + my_name + '"');

		const auto& rs = match_entry->second;

        for(match_rule = rs.begin(); match_rule != rs.end(); match_rule++) {

            children.clear();

            if(match_rule->type == rule_type::recursive) {

                ssize_t current = offset;
                bool success = true;

                for(const predicate& p : match_rule->recursive) {

                    size_t n;

                    ssize_t rewind = current;

                    for(n = 0; n < p.quantifier.second; n++) {

                        ast y;

                        auto next = y.parse_recursive(g, p.name, s, current);

                        if(next.first == -1)
                            break;

                        if(p.modifier == predicate_modifier::push)
                            children.push_back(y);

                        current = next.second;
                    }

                    if(n < p.quantifier.first)
                        success = false;

                    if(p.modifier == predicate_modifier::peek_negative)
                        success = not success;

                    if(p.modifier == predicate_modifier::peek_positive or p.modifier == predicate_modifier::peek_negative)
                        current = rewind;

                    if(not success)
                        break;
                }

                if(success)
                    return std::pair<ssize_t,ssize_t>(offset,current);

            } else {

                boost::smatch matches;
                std::string ms = s.substr(offset, std::string::npos);

                if(boost::regex_search(ms, matches, match_rule->regex)) {
                    match = matches[matches.size() > 1 ? 1 : 0];
                    return std::pair<ssize_t,ssize_t>(offset,offset + matches[0].length());
                }
            }

        }

		return std::pair<ssize_t,ssize_t>(-1,offset);
	}

	static std::string ast_str_recursive(const ast& x, int depth = 0, bool basic = false) {

		std::stringstream ss;

		if(basic)
            ss << x.name();
		else
			ss << std::setw(4) << x.offset << " " << std::setw(depth) << "" << x.name();

		switch(x.match_rule->type) {

			case rule_type::regex:

                ss << ' ' << (char)x.match_rule->type << "= " << x.match << std::endl;
                break;

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
		std::string tag = x.name();

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

		switch(x.match_rule->type) {

			case rule_type::regex:

				content = x.match;

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
