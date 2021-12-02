#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

DOCTEST_MAKE_STD_HEADERS_CLEAN_FROM_WARNINGS_ON_WALL_BEGIN
#include <cassert>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <thread>
#include <vector>
DOCTEST_MAKE_STD_HEADERS_CLEAN_FROM_WARNINGS_ON_WALL_END

#include "../gplot++.h"

using namespace std;

string read_file(const string & file_name) {
    INFO("Reading text file ", file_name);

    FILE *fin{fopen(file_name.c_str(), "rt")};
    CHECK(fin != nullptr);

    string result;

    while (true) {
        int next_char = fgetc(fin);
        if (next_char == EOF) break;

        char c{static_cast<char>(next_char)};
        if (c != '\n' && c != '\r' && c != '\f') result += c;
    }

    return result;
}

// This is useful to make the program wait until Gnuplot finishes
// writing stuff on the "dumb" terminal
void wait() { std::this_thread::sleep_for(std::chrono::milliseconds(1000)); }

TEST_CASE("complex") {
  const string file_name{"complex.svg"};
  
    {
        Gnuplot plt{};

		plt.redirect_to_svg(file_name);

        vector<double> x{1, 2, 3, 4, 5};
        vector<double> y{5, 4, 3, 2, 1};

        plt.plot(x, y, "Series #1", Gnuplot::LineStyle::POINTS);
        plt.plot(x, x, "Series #2", Gnuplot::LineStyle::POINTS);
        plt.set_xlabel("X axis");
        plt.set_ylabel("Y axis");
        plt.show();
    }  // Call Gnuplot::~Gnuplot() for plt and save the file

    wait();

    string file_contents{read_file(file_name)};

	CHECK(file_contents.find("Series #1") != string::npos);
	CHECK(file_contents.find("Series #2") != string::npos);
	CHECK(file_contents.find("X axis") != string::npos);
	CHECK(file_contents.find("Y axis") != string::npos);
}
