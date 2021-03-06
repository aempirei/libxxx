#include <xxx.hh>

namespace xxx {

    static std::string tree_str_recursive(const tree&, int, bool);
    static std::string tree_xml_recursive(const tree&, int, int, int);

    tree::tree() {
    }

    tree::tree(const grammar& g, FILE *fp) {
        parse(g, fp);
    }

    tree::tree(const grammar& g, const std::string& s) {
        parse(g, s);
    }

    void tree::clear() {
        *this = tree();
    }

    template<typename T, typename F> static size_t sum(size_t sum0, size_t sum, const T& xs, F f) {

        if(xs.empty())
            return sum0;

        for(const auto& x : xs)
            sum += (x.*f)();

        return sum;

    }

    void tree::transform(void *x) {
        match_rule.transform(this, x);
    }

    size_t tree::node_count() const {
        return sum(1, 1, children, &tree::node_count);
    }

    size_t tree::leaf_count() const {
        return sum(1, 0, children, &tree::leaf_count);
    }

    void tree::parse(const grammar& g, FILE *fp) {

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

    void tree::parse(const grammar& g, const std::string& s) {

        if(not parse(g, "document", s, 0)) {

            std::stringstream ss;

            ss << "tree::parse failed at offset " << (last + 1) << ": ";

            if(last + 33 < (ssize_t)s.length())
                ss << s.substr(last + 1, 32) << "...";
            else
                ss << s.substr(last + 1, std::string::npos);

            throw std::runtime_error(ss.str());
        }
    }

    bool tree::parse(const grammar& g, const var& name, const std::string& s, ssize_t my_offset) {

        offset = my_offset;

        ssize_t current = offset;

        auto match_def = g.find(name);

        if(match_def == g.end())
            throw std::runtime_error("definition not found for \"" + name + "\"");

        match_name = match_def->first;

		for(const auto& mr : match_def->second) {
		
			match_rule = mr;

            children.clear();

            if(match_rule.type  == rule_type::composite) {

                bool success = true;

                current = offset;

                for(const auto& p : match_rule.composite) {

					ssize_t rewind = current;

					tree y;

					if(y.parse(g, p.name, s, current)) {

						if(p.modifier == predicate::M::push)
							children.push_back(y);
						current = y.last + 1;

					} else if(p.quantifier == predicate::Q::one) {

						success = false;
					}

                    if(p.modifier == predicate::M::peek)
                        current = rewind;

                    if(not success)
                        break;
                }

                if(success) {
                    last = current - 1;
                    return true;
                }

            } else {

                boost::smatch matches;
                std::string ms = s.substr(offset, std::string::npos);

                if(boost::regex_search(ms, matches, match_rule.terminal)) {
                    match = matches[matches.size() > 1 ? 1 : 0];
                    current = offset + matches[0].length();
                    last = current - 1;
                    return true;
                }
            }
        }

        last = current - 1;
        return false;
    }

    static std::string tree_str_recursive(const tree& x, int depth = 0, bool basic = false) {

        std::stringstream ss;

        if(basic)
            ss << x.match_name;
        else
            ss << std::setw(4) << x.offset << " " << std::setw(depth) << "" << x.match_name;

        if(x.match_rule.type  == rule_type::terminal) {

            ss << ' ' << (char)x.match_rule.type  << "= " << x.match << std::endl;

        } else {

            if(x.children.size() == 1) {

                ss << ' ' << tree_str_recursive(x.children.back(), depth + 2, true);

            } else {

                ss << std::endl;

                for(const auto& y : x.children)
                    ss << tree_str_recursive(y, depth + 2);
            }
        }

        return ss.str();
    }

    std::string tree::str() const {
        return match_name.empty() ? "" : tree_str_recursive(*this);
    }

    static std::string tree_xml_recursive(const tree& x, int df = 0, int dm = 1, int dl = 0) {


        const std::string xmlns = "xmlns:xxx=\"http://www.256.bz/xmlns/xxx\"";

        std::stringstream ss;

        std::string content;
        std::string tag = x.match_name;

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

        if(x.match_rule.type  == rule_type::terminal) {

            content = x.match;

            content = boost::regex_replace(content, boost::regex("&"), "&amp;");
            content = boost::regex_replace(content, boost::regex("<"), "&lt;");
            content = boost::regex_replace(content, boost::regex(">"), "&gt;");
            content = boost::regex_replace(content, boost::regex("\""), "&quot;");
            content = boost::regex_replace(content, boost::regex("'"), "&apos;");

            ss << '>' << content << "</" << tag << '>';

        } else {

            if(x.children.empty()) {

                ss << " />";

            } else {

                ss << '>';

                if(x.leaf_count() == 1) {

                    ss << tree_xml_recursive(x.children.back());
                    ss << "</" << tag << '>';

                } else if(x.children.size() == 1) {

                    ss << tree_xml_recursive(x.children.back(), 0, dm, dl);
                    ss << "</" << tag << '>';

                } else {

                    ss << std::endl;

                    for(const auto& y : x.children) {
                        ss << tree_xml_recursive(y, dm, dm + 1, dl + 1) << std::endl;
                    }

                    ss << std::string(dl, ' ') << "</" << tag << '>';
                }
            }
        }

        return ss.str();
    }


    std::string tree::xml() const {
        const std::string xml_header = "<?xml version=\"1.0\"?>\n";

        return xml_header + (match_name.empty() ? "" : tree_xml_recursive(*this));
    }
}
