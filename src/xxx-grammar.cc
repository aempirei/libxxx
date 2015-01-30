#include <xxx.hh>

namespace xxx {

    std::string grammar::to_s(string_format_type type) const {
        switch(type) {
            default:
            case string_format_type::xxx: return to_s_xxx(); break;
            case string_format_type::js : return to_s_js (); break;
            case string_format_type::cc : return to_s_cc (); break;
        }
    }

    std::string grammar::to_s_js() const {
        throw std::runtime_error("grammar::to_s_js() unimplemented");
        return "";
    }

    vars grammar::transforms() const {

        std::set<var> u;

        if(find("document") != end())
            u.insert("document");

        for(const auto& x : *this)
            for(const auto& r : x.second)
                if(r.type == rule_type::recursive)
                    for(const auto& p : r.recursive)
                        if(p.modifier == predicate_modifier::push or p.modifier == predicate_modifier::lift)
                            u.insert(p.name);

        return vars(u.begin(), u.end());
    }

	std::string grammar::to_s_cc() const {

        std::stringstream ss;

        ss << "namespace xxx {" << std::endl;
        ss << "\tstatic grammar define_grammar() {" << std::endl;
        ss << "\t\tusing M = predicate_modifier;" << std::endl;
        ss << "\t\treturn grammar({" << std::endl;

        size_t w = 0;

        for(const auto& x : *this)
            if(x.first.length() > w)
                w = x.first.length();

        for(const auto& x : *this) {

            ss << "\t\t\t{ \"" << x.first << "\"" << std::setw(w - x.first.length()) << "" << ", {";

            if(x.second.size() < 2)
                ss << ' ';
            else
                ss << std::endl << "\t\t\t\t";

            auto iter = x.second.begin();

            ss << iter->to_cc();

            while(++iter != x.second.end())
                ss << ',' << std::endl << "\t\t\t\t" << iter->to_cc();

            if(x.second.size() < 2)
                ss << ' ';
            else
                ss << std::endl << "\t\t\t";

            ss << "} }," << std::endl;
        }

        ss << "\t\t});" << std::endl;
        ss << "\t}" << std::endl;

        ss << "\tnamespace transform {" << std::endl;

        for(const auto& s : transforms())
            ss << "\t\tstruct " << s << ';' << std::endl;

        for(const auto& s : transforms()) {

            ss << "\t\tstruct " << s << " {" << std::endl;

            const auto& rs = at(s);

            for(const auto& r : rs) {

                ss << "\t\t\t" << s << '(';

                auto sig = r.to_sig();
                auto iter = sig.begin();

                ss << *iter;

                while(++iter != sig.end())
                    ss << ", " << *iter;

                ss << ");" << std::endl;
            }

            ss << "\t\t};" << std::endl;
        }

        ss << "\t}" << std::endl;
        ss << "}" << std::endl;

        return ss.str();
    }

	std::string grammar::to_s_xxx() const {

        std::stringstream ss;

        for(const auto& x : *this)
            for(const auto& r : x.second)
                ss << x.first << ' ' << (char)r.type << "= " << r.str() << std::endl;

		return ss.str();
	}
}
