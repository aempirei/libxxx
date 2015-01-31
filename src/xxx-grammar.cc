#include <xxx.hh>

namespace xxx {

    std::string grammar::to_js() const {
        throw std::runtime_error("grammar::to_js() unimplemented");
        return "";
    }

    std::set<var> grammar::appendix() const {

        std::set<var> u;

        if(find("document") != end())
            u.insert("document");

        for(const auto& x : *this)
            for(const auto& r : x.second)
                if(r.type == rule_type::composite)
                    for(const auto& p : r.composite)
                        if(p.modifier == predicate_modifier::push)
                            u.insert(p.name); 

        return u;
    }

	std::string grammar::to_cc() const {

        std::stringstream ss;

        ss << "#pragma once" << std::endl;
        ss << "#define R(s) rule(rule::terminal_type(s))" << std::endl;

        ss << std::endl;

        ss << "namespace xxx {" << std::endl;
        ss << "\tnamespace local {" << std::endl;

        ss << std::endl;

        ss << "\t\tusing M = predicate_modifier;" << std::endl;
        ss << "\t\tusing Q = predicate_quantifier;" << std::endl;

        ss << std::endl;

        auto ts = appendix();

        //
        // transform declarations
        //

        for(const auto& s : ts)
            for(size_t n = 0; n < at(s).size(); n++)
                ss << "\t\ttransform_function " << s << "_transform_" << (n + 1) << ';' << std::endl;

        ss << std::endl;

        //
        // grammar definition
        //

        ss << "\t\tgrammar spec = grammar({" << std::endl;

        size_t w = 0;

        for(const auto& x : *this)
            if(x.first.length() > w)
                w = x.first.length();

        for(const auto& x : *this) {

            const auto& s = x.first;
            const auto& rs = x.second;

            ss << "\t\t\t{ \"" << s << "\"" << std::setw(w - s.length()) << "" << ", {";

            if(rs.size() == 1) {

                ss << ' ' << rs.front().to_cc();

                if(ts.find(s) != ts.end())
                    ss << " >> " << s << "_transform_1";

                ss << ' '; 

            } else {

                ss << std::endl;

                for(size_t n = 0; n < rs.size(); n++) {

                    const auto& r = rs[n];

                    ss << "\t\t\t\t" << r.to_cc();
                    if(ts.find(s) != ts.end())
                        ss << " >> " << s << "_transform_" << (n + 1);
                    ss << ',' << std::endl;
                }

                ss << "\t\t\t";
            }

            ss << "} }," << std::endl;
        }

        ss << "\t\t});" << std::endl;

        ss << std::endl;

        //
        // transform definitions
        //

        for(const auto& s : ts) {

            const auto& rs = at(s);

            for(size_t n = 0; n < rs.size(); n++) {

                const auto& r = rs[n];

                ss << "\t\tvoid " << s << "_transform_" << (n + 1) << "(tree *a, void *x) {" << std::endl;

                ss << "\t\t\t//";

                for(const auto& sig : r.to_sig())
                    ss << ' ' << sig;

                ss << std::endl;

                ss << "\t\t}" << std::endl;

                ss << std::endl;
            }
        }

        ss << "\t}" << std::endl;
        ss << "}" << std::endl;

        ss << std::endl;

        ss << "#undef R" << std::endl;

        return ss.str();
    }

	std::string grammar::to_xxx() const {

        std::stringstream ss;

        for(const auto& x : *this)
            for(const auto& r : x.second)
                ss << x.first << ' ' << (char)r.type << "= " << r.str() << std::endl;

		return ss.str();
	}
}
