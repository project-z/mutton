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

#define BOOST_SPIRIT_UNICODE

namespace qi    = boost::spirit::qi;
namespace phx   = boost::phoenix;

typedef std::string quoted_string;
typedef int64_t integer;
struct op_and;
struct op_or;
struct op_not;
struct op_xor;
struct op_range;
struct op_slice;

typedef boost::variant<integer,
                       op_range,
                       op_slice,
                       boost::recursive_wrapper<op_or>,
                       boost::recursive_wrapper<op_not>,
                       boost::recursive_wrapper<op_and>,
                       boost::recursive_wrapper<op_xor>,
                       boost::recursive_wrapper<op_or>
                       > expr;

struct op_range
{
    op_range() : start(0), limit(0) {}
    op_range(integer s, integer l) : start(s), limit(l) {}
    integer start;
    integer limit;
};

struct op_slice
{
    std::string index;
    std::vector<expr> ranges;
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
        return std::string("[")
            + boost::lexical_cast<std::string>(o.start)
            + " "
            + boost::lexical_cast<std::string>(o.limit)
            + "]";
    }

    std::string
    operator()(const op_slice& o) const
    {
        if (o.ranges.empty()) {
            return std::string("(slice \"") + o.index + "\")";
        }
        else {
            return print("slice \"" + o.index + "\"", o.ranges.begin(), o.ranges.end());
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

        expr_ = ('(' > (slice_ | or_ | and_ | xor_ | not_)  > ')');

        byte_string_ = qi::lexeme['#' > +hex2 > '#'];
        quoted_string_ %= qi::lexeme ['"' >> *(qi::char_ - qi::char_('\\') - qi::char_('"') | '\\' >> qi::char_) >> '"'];
        integer_ = boost::spirit::lexeme[qi::no_case["0x"] > qi::hex] | boost::spirit::lexeme['0' >> qi::oct] | qi::int_;

        range_ = ("[" > integer_ > integer_ > "]") [ qi::_val = phx::construct<op_range>(qi::_1, qi::_2) ];

        slice_ = "slice"
            > (quoted_string_) [phx::bind(&op_slice::index, qi::_val) = qi::_1]
            > *(range_) [phx::push_back(phx::bind(&op_slice::ranges, qi::_val), qi::_1)];

        and_ = "and"
            > +(expr_) [phx::push_back(phx::bind(&op_and::children, qi::_val), qi::_1)];

        not_ = "not"
            > (expr_) [phx::bind(&op_not::child, qi::_val) = qi::_1];

        or_ = "or"
            > +(expr_) [phx::push_back(phx::bind(&op_or::children, qi::_val), qi::_1)];

        xor_ = "xor"
            > +(expr_) [phx::push_back(phx::bind(&op_xor::children, qi::_val), qi::_1)];

        BOOST_SPIRIT_DEBUG_NODE(expr_);
        BOOST_SPIRIT_DEBUG_NODE(or_);
        BOOST_SPIRIT_DEBUG_NODE(xor_);
        BOOST_SPIRIT_DEBUG_NODE(and_);
        BOOST_SPIRIT_DEBUG_NODE(not_);
        BOOST_SPIRIT_DEBUG_NODE(integer_);
        BOOST_SPIRIT_DEBUG_NODE(range_);
    }

  private:
    qi::rule<Iterator, integer(), Skipper>  integer_;
    qi::rule<Iterator, op_range(), Skipper> range_;
    qi::rule<Iterator, op_slice(), Skipper> slice_;
    qi::rule<Iterator, op_and(), Skipper>   and_;
    qi::rule<Iterator, op_or(), Skipper>    or_;
    qi::rule<Iterator, op_not(), Skipper>   not_;
    qi::rule<Iterator, op_xor(), Skipper>   xor_;
    qi::rule<Iterator, expr(), Skipper>     expr_;

    qi::rule<Iterator, boost::spirit::binary_string_type()>              byte_string_;
    qi::rule<Iterator, std::string(), qi::space_type, qi::locals<char> > quoted_string_;

};

int main()
{
    for (auto& input : std::list<std::string> {
            "(slice \"ταБЬℓσ\")",
            "(slice \"a\")",
            "(slice \"a\" [1 2])",
            "(slice \"a\" [1 2] [3 4])",
            "(or (slice \"a\"))",
            "(or (slice \"a\" [1 2]))",
            "(or (slice \"a\") (slice \"b\"))",
            "(or (slice \"a\") (slice \"b\" [2 4]))",
            "(and (or (slice \"a\") (slice \"b\")) (or (slice \"a\") (slice \"b\")))",
            "(not (and (or (slice \"a\") (slice \"b\")) (or (slice \"a\") (slice \"b\"))))",
            })
    {
        auto f(std::begin(input)), l(std::end(input));
        parser<decltype(f)> p;

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
        catch (const qi::expectation_failure<decltype(f)>& e)
        {
            std::cerr << "expectation_failure at '" << std::string(e.first, e.last) << "'\n";
        }

        if (f != l) {
            std::cerr << "unparsed: '" << std::string(f,l) << "'\n";
        }
    }

    return 0;
}
