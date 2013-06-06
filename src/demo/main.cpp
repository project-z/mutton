#include "query_parser.hpp"
#include "query_printer.hpp"
#include "naive_query_planner.hpp"

std::ostream&
operator<<(std::ostream& os,
           const mtn::expr& e)
{
    os << boost::apply_visitor(mtn::query_printer_t(), e);
    return os;
}

int main()
{
    std::string test_strings[] = {
        "(slice \"ταБЬℓσ\")",
        "(slice \"a\")",
        "(slice \"a\" (range 1 2))",
        "(slice \"a\" (range 1 2) (range 3 4))",
        "(slice \"a\" (regex \"foo.*bar\"))",
        "(slice \"a\" (range 1 2) (regex \"foo.*bar\"))",
        "(or (slice \"a\"))",
        "(or (slice \"randint\" (range 1 184467440737095516)))",
        "(or (slice \"big64uint\" (range 1 18446744073709551616)))",
        "(or (slice \"big128uint\" (range 1 340282366920938463463374607431768211455)))",
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
        mtn::query_parser_t<std::string::const_iterator> p;

        try {
            mtn::expr result;
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
