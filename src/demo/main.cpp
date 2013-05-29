#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/spirit/include/qi.hpp>

#include <boost/spirit/include/phoenix.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/fusion/include/adapt_struct.hpp>

#include <boost/spirit/include/support_utree.hpp>
#include <boost/variant/recursive_wrapper.hpp>

#include <re2/filtered_re2.h>
#include <re2/stringpiece.h>
#include <re2/re2.h>

#include "city.h"

#define BOOST_SPIRIT_UNICODE

namespace qi    = boost::spirit::qi;
namespace phx   = boost::phoenix;

typedef std::string quoted_string;
typedef int64_t integer;
struct op_and;
struct op_not;
struct op_or;
struct op_slice;
struct op_xor;

struct op_regex
{
    std::string pattern;
};

struct op_range
{
    op_range() : start(0), limit(0) {}
    op_range(integer s, integer l) : start(s), limit(l) {}
    op_range(const std::string& s) : start(hash(s)), limit(0) {}

    uint64_t
    hash(const std::string& input)
    {
        return CityHash64(input.c_str(), input.size());
    }

    integer start;
    integer limit;
};

typedef boost::variant<integer,
                       op_range,
                       op_slice,
                       op_regex,
                       boost::recursive_wrapper<op_or>,
                       boost::recursive_wrapper<op_not>,
                       boost::recursive_wrapper<op_and>,
                       boost::recursive_wrapper<op_xor>,
                       boost::recursive_wrapper<op_or>
                       > expr;

struct op_slice
{
    std::string index;
    std::vector<expr> values;
};

struct op_and
{
    std::vector<expr> children;
};

struct op_not
{
    expr child;
};

struct op_or
{
    std::vector<expr> children;
};

struct op_xor
{
    std::vector<expr> children;
};


struct printer :
    boost::static_visitor<std::string>
{
    std::string
    operator()(const std::string& v) const
    {
        return v;
    }

    std::string
    operator()(const integer v) const
    {
        return boost::lexical_cast<std::string>(v);
    }

    std::string
    operator()(const op_and& o) const
    {
        return print("&", o.children.begin(), o.children.end());
    }

    std::string
    operator()(const op_or& o) const
    {
        return print("|", o.children.begin(), o.children.end());
    }

    std::string
    operator()(const op_xor& o) const
    {
        return print("^", o.children.begin(), o.children.end());
    }

    std::string
    operator()(const op_not& o) const
    {
        return std::string("(! ") + boost::apply_visitor(*this, o.child) + ")";
    }

    std::string
    operator()(const op_range& o) const
    {
        return std::string("(range ")
            + boost::lexical_cast<std::string>(o.start)
            + " "
            + boost::lexical_cast<std::string>(o.limit)
            + ")";
    }

    std::string
    operator()(const op_regex& o) const
    {
        return std::string("(regex \"") + o.pattern + "\")";
    }

    std::string
    operator()(const op_slice& o) const
    {
        if (o.values.empty()) {
            return std::string("(slice \"") + o.index + "\")";
        }
        else {
            return print("slice \"" + o.index + "\"", o.values.begin(), o.values.end());
        }
    }

    template<class Iterator>
    inline std::string
    print(const std::string& op,
          Iterator it,
          Iterator end) const
    {
        std::list<std::string> temp;
        for (; it != end; ++it) {
            temp.push_back(boost::apply_visitor(*this, *it));
        }

        return std::string("(") + op + " " + boost::algorithm::join(temp, " ") + ")";
    }
};

std::ostream&
operator<<(std::ostream& os,
           const expr& e)
{
    os << boost::apply_visitor(printer(), e);
    return os;
}

template <typename Iterator, typename Skipper = qi::space_type>
    struct parser : qi::grammar<Iterator, expr(), Skipper>
{
    parser()
        : parser::base_type(expr_)
    {
        qi::uint_parser<unsigned char, 16, 2, 2> hex2;

        expr_ = ('(' >> (slice_ | or_ | and_ | xor_ | not_)  >> ')');

        byte_string_ = qi::lexeme['#' > +hex2 > '#'];
        quoted_string_ %= qi::lexeme ['"' >> *(qi::char_ - qi::char_('\\') - qi::char_('"') | '\\' >> qi::char_) >> '"'];
        integer_ = boost::spirit::lexeme[qi::no_case["0x"] > qi::hex] | boost::spirit::lexeme['0' >> qi::oct] | qi::int_;

        // value_ = "(value"
        //     >> (integer_) [qi::_val = phx::construct<op_range>(qi::_1, 0)]
        //     >> (quoted_string_ | byte_string_) [phx::bind(&op_range::hash, qi::_val) = qi::_1]
        //     > ")";

        range_ = ("(range" > integer_ > integer_ > ")") [qi::_val = phx::construct<op_range>(qi::_1, qi::_2)];

        regex_ = ("(regex" > quoted_string_  > ")") [phx::bind(&op_regex::pattern, qi::_val) = qi::_1];

        slice_ = "slice"
            > (quoted_string_) [phx::bind(&op_slice::index, qi::_val) = qi::_1]
            > *(regex_ | range_ | value_) [phx::push_back(phx::bind(&op_slice::values, qi::_val), qi::_1)];

        and_ = "and"
            > +(expr_) [phx::push_back(phx::bind(&op_and::children, qi::_val), qi::_1)];

        not_ = "not"
            > (expr_) [phx::bind(&op_not::child, qi::_val) = qi::_1];

        or_ = "or"
            > +(expr_) [phx::push_back(phx::bind(&op_or::children, qi::_val), qi::_1)];

        xor_ = "xor"
            > +(expr_) [phx::push_back(phx::bind(&op_xor::children, qi::_val), qi::_1)];

        BOOST_SPIRIT_DEBUG_NODE(and_);
        BOOST_SPIRIT_DEBUG_NODE(expr_);
        BOOST_SPIRIT_DEBUG_NODE(integer_);
        BOOST_SPIRIT_DEBUG_NODE(not_);
        BOOST_SPIRIT_DEBUG_NODE(or_);
        BOOST_SPIRIT_DEBUG_NODE(range_);
        BOOST_SPIRIT_DEBUG_NODE(regex_);
        BOOST_SPIRIT_DEBUG_NODE(value_);
        BOOST_SPIRIT_DEBUG_NODE(xor_);
    }

  private:
    qi::rule<Iterator, expr(), Skipper>     expr_;
    qi::rule<Iterator, integer(), Skipper>  integer_;
    qi::rule<Iterator, op_and(), Skipper>   and_;
    qi::rule<Iterator, op_not(), Skipper>   not_;
    qi::rule<Iterator, op_or(), Skipper>    or_;
    qi::rule<Iterator, op_range(), Skipper> range_;
    qi::rule<Iterator, op_range(), Skipper> value_;
    qi::rule<Iterator, op_regex(), Skipper> regex_;
    qi::rule<Iterator, op_slice(), Skipper> slice_;
    qi::rule<Iterator, op_xor(), Skipper>   xor_;

    qi::rule<Iterator, boost::spirit::binary_string_type()>              byte_string_;
    qi::rule<Iterator, std::string(), qi::space_type, qi::locals<char> > quoted_string_;
};

int main()
{
    re2::FilteredRE2 re2;
    re2::StringPiece pattern("foobiz.*bar");
    RE2::Options options;

    std::vector<std::string> pieces;

    int id = 0;
    re2.Add(pattern, options, &id);
    re2.Compile(&pieces);

    BOOST_FOREACH(const std::string& p, pieces) {
        std::cout << "piece: " << p << "\n";
    }

    std::string test_strings[] = {
        "(slice \"ταБЬℓσ\")",
        "(slice \"a\")",
        "(slice \"a\" (range 1 2))",
        "(slice \"a\" (range 1 2) (range 3 4))",
        "(slice \"a\" (regex \"foo.*bar\"))",
        "(slice \"a\" (range 1 2) (regex \"foo.*bar\"))",
        "(or (slice \"a\"))",
        "(or (slice \"a\" (range 1 2)))",
        "(or (slice \"a\") (slice \"b\"))",
        "(or (slice \"a\") (slice \"b\" (range 1 2)))",
        "(and (or (slice \"a\") (slice \"b\")) (or (slice \"a\") (slice \"b\")))",
        "(not (and (or (slice \"a\") (slice \"b\")) (or (slice \"a\") (slice \"b\"))))"
    };

    for (int i = 0; i < 10; ++i)
    {
        std::string input = test_strings[i];
        std::string::const_iterator f(input.begin());
        std::string::const_iterator l(input.end());
        parser<std::string::const_iterator> p;

        try {
            expr result;
            bool ok = qi::phrase_parse(f, l, p, qi::space, result);

            if (!ok) {
                std::cerr << "invalid input\n";
            }
            else {
                std::cout << "result: " << result << "\n";
            }
        }
        catch (const qi::expectation_failure<std::string::const_iterator>& e)
        {
            std::cerr << "expectation_failure at '" << std::string(e.first, e.last) << "'\n";
        }

        if (f != l) {
            std::cerr << "unparsed: '" << std::string(f,l) << "'\n";
        }
    }

    return 0;
}
