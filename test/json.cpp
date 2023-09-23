#include <gtest/gtest.h>
#include <parrot/json.h>
#include <sstream>

void expect_json_string_equal(std::string actual, std::string expected)
{
    std::erase_if(actual, [](char c) { return std::isspace(c) || c == '\n'; });
    std::erase_if(expected, [](char c) { return std::isspace(c) || c == '\n'; });
    EXPECT_EQ(actual, expected);
}

void expect_json_equal(const parrot::Json& json, const std::string& expected)
{
    std::stringstream ss;
    ss << json;
    expect_json_string_equal(ss.str(), expected);
}

TEST (Json, Main)
{
    parrot::Json json;
    auto writer = json.get();
    writer["color"] = std::string_view("red");
    writer["size"] = 10;
    writer["element"]["asdf"] = 12.1;
    writer["element"]["fdsa"] = true;
    *writer["array"].push_back() = 0.1;
    *writer["array"].push_back() = 0.2;
    writer["array"][0] = -0.1;

    expect_json_equal(json, R"(
    {
      "color": "red",
      "size": 10,
      "element": {
        "asdf": 12.1,
        "fdsa": true
      },
      "array": [
        -0.1,
        0.2
      ]
    }
    )");

    parrot::Json json2, json3;
    const std::string json2_string = R"({
        "abcd": 1,
        "list": [ "red", "yellow", "orange" ],
        "object": { "one": 1, "two": 2, "null": null, "array": [ false, false, true ] },
        "null_again": null
    })";
    const std::string json3_string = R"({
        "abcd": 1,
        "list": [ "red", "yellow", "orange" ],
        "object": { "one": 1, "two": 2, "null": null, "array": { "one": 1, "two": 2, "null": null, "array": [ false, false, true ] } },
        "null_again": null
    })";

    json2 = json2_string;
    expect_json_equal(json2, json2_string);

    copy(json2, json3);
    copy(json2.get()["object"], json3.get()["object"]["array"]);
    expect_json_equal(json3, json3_string);
}
