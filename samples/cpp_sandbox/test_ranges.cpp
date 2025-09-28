#include <print>
#include <vector>
#include <ranges>
#include <string_view>
#include <regex>
#include <cassert>
#include <charconv>
#include <map>

using namespace std::string_view_literals;

auto parse_int(std::string_view str) {
    int value;
    auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), value);
    return (ec == std::errc{}) ? std::optional{value} : std::nullopt;
}

void test_ranges_01() {
    std::vector vec{1, 2, 3, 4, 5, 6};

    auto v = vec | 
        std::views::filter([] (int value) {
            return (value & 1) == 0;
            }) | 
        std::views::transform([](int value) {
                return value * value;
                });

    std::println("{}", v);
}

void test_ranges_02() {

    char const* const test_data = "string1\n"
        "string2\n"
        "string3\n";

    auto v = std::string_view(test_data) | 
        std::views::split('\n') | 
        std::views::filter([] (auto const& range_val) {
                return !range_val.empty();
                }) |
        std::views::transform([] (auto const& val) {
            return std::string_view(val.begin(), val.end());
            }) |
        std::ranges::to<std::vector>();

    std::println("{}", v);
}

void test_ranges_03() {
    std::string_view test_str = "Game 1: 3 blue, 4 red; 1 red, 2 green, 6 blue; 2 green\n"
        "Game 3: 8 green, 6 blue, 20 red; 5 blue, 4 red, 13 green; 5 green, 1 red\n"
        "Game 4: 1 green, 3 red, 6 blue; 3 green, 6 red; 3 green, 15 blue, 14 red\n"
        "Game 5: 6 red, 1 blue, 3 green; 2 blue, 1 red, 2 green\n"
        "Game 2: 1 blue, 2 green; 3 green, 4 blue, 1 red; 1 green, 1 blue"sv;
    std::regex game_regex(R"(Game (\d+))");
    std::regex color_regex(R"((\d+) (red|green|blue))");

    struct ColorValue {
        std::string color;
        int value;
    };

     auto parser = test_str | 
        std::views::split('\n') |
        std::views::transform([] (auto const& val) {
            return std::string_view(val.begin(), val.end());
            }) |
        std::views::transform([&game_regex, &color_regex] (auto const& val) {
            std::println("Line processing");
            auto const sep_index = val.find(':');
            assert(sep_index != std::string_view::npos);

            auto const game_str = val.substr(0, sep_index);
            auto const value_str = val.substr(sep_index + 1);

            std::cmatch game_match;
            bool const res = std::regex_search(game_str.begin(), game_str.end(), game_match, game_regex);
            assert(res);

            auto const game_value = parse_int(game_match[1].str()).value_or(0);

            auto v2 = value_str | 
                std::views::split(';') |
                std::views::transform([&color_regex] (auto const& val) {
                    std::println("  > elem processng");
                    // Convert range to string for regex_iterator
                    std::string_view val_str(val.begin(), val.end());
                    std::vector<ColorValue> values;

                    // Extract all color matches in one pass using regex_iterator
                    for (std::cregex_iterator it(val_str.begin(), val_str.end(), color_regex);
                         it != std::cregex_iterator{}; ++it) {
                        const std::cmatch& match = *it;
                        values.emplace_back(match[2].str(), parse_int(match[1].str()).value_or(0));
                    }

                    return values;
                });

            std::pair<int, std::vector<std::vector<ColorValue>>> game_desc;
            game_desc.first = game_value;
            game_desc.second = std::ranges::to<std::vector<std::vector<ColorValue>>>(v2);

            return game_desc;
    }) ;

    for ( auto const& val : parser) 
    {
        std::println("Game {}", val.first);

        for (auto const& color_set : val.second) {
            std::println("{{");

            for (auto const& color : color_set) {
                std::println("  {} : {}", color.color, color.value);
            }

            std::println("}}");
        }
    }
}

void test_ranges_04() {

    std::map<int, int> values = {
        {1,2},
        {3,4},
        {5,6}
    };

    for (auto val : values | std::views::keys ) {
        std::println("keys : {}", val);
    }

    for (auto val : values | std::views::values ) {
        std::println("val : {}", val);
    }
}

void test_ranges_05() {

    std::vector<std::string> vec_str = {
        "hello",
        "world",
        "!"
    };

    const int val[] = {
        1, 2
    };

    for (auto [val1, val2] : std::views::zip(vec_str, val))
    {
        std::println("{} {}", val1, val2);
    }
}

void test_ranges() 
{
    test_ranges_01();
    test_ranges_02();
    test_ranges_03();
    test_ranges_04();
    test_ranges_05();
}
