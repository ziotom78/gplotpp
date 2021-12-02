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

string read_file(const char *file_name) {
    INFO("Reading text file ", file_name);

    FILE *fin{fopen(file_name, "rt")};
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

TEST_CASE("simple plot") {
    {
        Gnuplot plt{};

        plt.sendcommand("set terminal dumb 40 20");
        plt.sendcommand("set output 'simple-plot.txt'");

        vector<double> x{1, 2, 3, 4, 5};
        vector<double> y{5, 4, 3, 2, 1};

        plt.plot(x, y);
        plt.show();
    }  // Call Gnuplot::~Gnuplot() for plt and save the file

    wait();

    string file_contents{read_file("simple-plot.txt")};
    CHECK(file_contents ==
          "                                        "
          "      +-----------------------------+   "
          "    5 |**  +  +   +  +  +   +  +  ++|   "
          "      |  **                         |   "
          "  4.5 |-+  **                     +-|   "
          "    4 |-+    **                   +-|   "
          "      |        **                   |   "
          "  3.5 |-+        **               +-|   "
          "      |            **               |   "
          "    3 |-+            *            +-|   "
          "      |               **            |   "
          "  2.5 |-+               **        +-|   "
          "      |                   **        |   "
          "    2 |-+                   **    +-|   "
          "  1.5 |-+                     **  +-|   "
          "      |                         **  |   "
          "    1 |++  +  +   +  +  +   +  +  **|   "
          "      +-----------------------------+   "
          "       1  1.5 2  2.5 3 3.5  4 4.5  5    "
          "                                        ");
}

TEST_CASE("complex") {
    {
        Gnuplot plt{};

        plt.sendcommand("set terminal dumb 40 20");
        plt.sendcommand("set output 'complex.txt'");

        vector<double> x{1, 2, 3, 4, 5};
        vector<double> y{5, 4, 3, 2, 1};

        plt.plot(x, y, "Series #1", Gnuplot::LineStyle::POINTS);
        plt.plot(x, x, "Series #2", Gnuplot::LineStyle::POINTS);
        plt.set_xlabel("X axis");
        plt.set_ylabel("Y axis");
        plt.show();
    }  // Call Gnuplot::~Gnuplot() for plt and save the file

    wait();

    string file_contents{read_file("complex.txt")};
    CHECK(file_contents ==
          "                                        "
          "         +--------------------------+   "
          "       5 |C+ +  +  +   +  +  +  + +D|   "
          "     4.5 |-+      Series #1    C  +-|   "
          "         |        Series #2    D    |   "
          "       4 |-+    C            D    +-|   "
          "         |                          |   "
          "     3.5 |-+                      +-|   "
          "       3 |-+           D          +-|   "
          "         |                          |   "
          "     2.5 |-+                      +-|   "
          "         |                          |   "
          "       2 |-+    D            C    +-|   "
          "         |                          |   "
          "     1.5 |-+                      +-|   "
          "       1 |D+ +  +  +   +  +  +  + +C|   "
          "         +--------------------------+   "
          "          1 1.5 2 2.5  3 3.5 4 4.5 5    "
          "                   X axis               "
          "                                        ");
}

TEST_CASE("histogram") {
    {
        Gnuplot plt{};

        plt.sendcommand("set terminal dumb 40 20");
        plt.sendcommand("set output 'histogram.txt'");

        vector<double> samples{1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3,
                               3, 4, 4, 4, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6};

        plt.histogram(samples, 3);
        plt.show();
    }  // Call Gnuplot::~Gnuplot() for plt and save the file

    wait();

    string file_contents{read_file("histogram.txt")};
    CHECK(file_contents ==
          "                                        "
          "       +----------------------------+   "
          "    12 |-++   +   +   +  +  ********|   "
          "       |                    *       |   "
          "  11.5 |-+                  *     +-|   "
          "       |                    *       |   "
          "       |                    *       |   "
          "    11 |********            *     +-|   "
          "       |       *            *       |   "
          "  10.5 |-+     *            *     +-|   "
          "       |       *            *       |   "
          "    10 |-+     *            *     +-|   "
          "       |       *            *       |   "
          "       |       *            *       |   "
          "   9.5 |-+     *            *     +-|   "
          "       |       *            *       |   "
          "     9 |-++   +**************+   ++-|   "
          "       +----------------------------+   "
          "          2  2.5  3  3.5 4  4.5  5      "
          "                                        ");
}
