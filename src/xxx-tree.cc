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

    template<typename T, typename F> static size_t sum(size_t sum0, size_t sum, const T& xs, F f) {

        if(xs.empty())
            return sum0;

        for(const auto& x : xs)
            sum += (x.*f)();

        return sum;

    }

    void tree::transform(void *x) {
        match_rule->transform(this, x);
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

        auto ab = parse_recursive(g, "document", s, 0);

        if(ab.first == -1) {

            std::stringstream ss;
            ss << "tree::parse failed at offset " << ab.second << " (";

            if(ab.second + 16 < (ssize_t)s.length()) {
                ss << s.substr(ab.second, ab.second + 16) << "...";
            } else {
                ss << s.substr(ab.second, std::string::npos);
            }

            ss << ")";

            throw std::runtime_error(ss.str());
        }
    }

    const var& tree::match_name() const {
        return match_def->first;
    }

    rule_type tree::match_type() const {
        return match_rule->type;
    }

    std::pair<ssize_t,ssize_t> tree::parse_recursive(const grammar& g, const var& name, const std::string& s, ssize_t my_offset) {

        offset = my_offset;

        match_def = g.find(name);

        if(match_def == g.end())
            throw std::runtime_error("definition not found for \"" + name + "\"");

        const auto& rs = match_def->second;

        for(match_rule = rs.begin(); match_rule != rs.end(); match_rule++) {

            children.clear();

            if(match_type() == rule_type::composite) {

                ssize_t current = offset;
                bool success = true;

                for(const auto& p : match_rule->composite) {

                    size_t n;

                    ssize_t rewind = current;

                    for(n = 0; n < p.upper(); n++) {

                        tree y;

                        auto next = y.parse_recursive(g, p.name, s, current);

                        if(next.first == -1)
                            break;

                        if(p.modifier == predicate_modifier::push)
                            children.push_back(y);
                        else if(p.modifier == predicate_modifier::lift)
                            children.insert(children.end(), y.children.begin(), y.children.end());

                        current = next.second;
                    }

                    if(n < p.lower())
                        success = false;

                    if(p.modifier == predicate_modifier::peek)
                        current = rewind;

                    if(not success)
                        break;
                }

                if(success)
                    return std::pair<ssize_t,ssize_t>(offset,current);

            } else {

                boost::smatch matches;
                std::string ms = s.substr(offset, std::string::npos);

                if(boost::regex_search(ms, matches, match_rule->terminal)) {
                    match = matches[matches.size() > 1 ? 1 : 0];
                    return std::pair<ssize_t,ssize_t>(offset,offset + matches[0].length());
                }
            }
        }

        return std::pair<ssize_t,ssize_t>(-1,offset);
    }

    static std::string tree_str_recursive(const tree& x, int depth = 0, bool basic = false) {

        std::stringstream ss;

        if(basic)
            ss << x.match_name();
        else
            ss << std::setw(4) << x.offset << " " << std::setw(depth) << "" << x.match_name();

        if(x.match_type() == rule_type::terminal) {

            ss << ' ' << (char)x.match_type() << "= " << x.match << std::endl;

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
        return tree_str_recursive(*this);
    }

    static std::string tree_xml_recursive(const tree& x, int df = 0, int dm = 1, int dl = 0) {


        const std::string xmlns = "xmlns:xxx=\"http://www.256.bz/xmlns/xxx\"";

        std::stringstream ss;

        std::string content;
        std::string tag = x.match_name();

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

        if(x.match_type() == rule_type::terminal) {

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

        return xml_header + tree_xml_recursive(*this);
    }
}
