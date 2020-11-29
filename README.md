# gplot++

![](https://github.com/ziotom78/gplotpp/workflows/make-checks/badge.svg)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

A header-only C++ interface to Gnuplot.

This repository contains the file `gplot++.h`, which provides a way for C++ programs to connect to a Gnuplot instance to produce plots.

A few features of this library are the following:

- Header-only library: very easy to install
- Plot `std::vector` variables
- Multiple series in the same plot
- Multiple plots (via `Gnuplot::multiplot`)
- Logarithmic axes (via `Gnuplot::set_logscale`)
- Histograms (via `Gnuplot::histogram`)
- Custom ranges (via `Gnuplot::set_xrange` and `Gnuplot::set_yrange`)
- Possibility to save the plots in PNG and PDF files

## Examples

Here is the output of one of the examples:

![](./images/complex.png)

The source code of the example is in file `example-complex.cpp`:

```c++
#include "gplot++.h"

int main(void) {
  Gnuplot plt{};
  std::vector<double> x{1, 2, 3, 4, 5}, y{5, 2, 4, 1, 3};

  // Save the plot into a PNG file with the desired size (in pixels)
  plt.redirect_to_png("complex.png", "800,600");

  /* Create two plots with the following layout (2 rows, 1 column):
   *
   * +------------------------+
   * |                        |
   * |        Plot #1         |
   * |                        |
   * +------------------------+
   * |                        |
   * |        Plot #2         |
   * |                        |
   * +------------------------+
   */
  plt.multiplot(2, 1, "Title");

  // Plot #1
  plt.set_xlabel("X axis");
  plt.set_ylabel("Y axis");
  plt.plot(x, y, "x-y plot");
  plt.plot(y, x, "y-x plot", Gnuplot::LineStyle::LINESPOINTS);
  plt.show(); // Always call "show"!

  // Plot #2
  plt.set_xlabel("Value");
  plt.set_ylabel("Number of counts");
  plt.histogram(y, 2, "Histogram");
  plt.set_xrange(-1, 7);
  plt.set_yrange(0, 5);
  plt.show(); // Always call "show"!
}
```

## Low-level interface

You can pass commands to Gnuplot using the method `Gnuplot::sendcommand`:

```c++
Gnuplot plt{};

plt.sendcommand("set xlabel 'X axis'");
plt.sendcommand("plot sin(x)");
```

## Similar libraries

There are several other libraries like gplot++ around. These are the
ones I referenced while developing my own's:

-   [gnuplot-iostream](https://github.com/dstahlke/gnuplot-iostream):
    it has much more features than gplot++, but it is slightly more
    difficult to install and use.
-   [gnuplot-cpp](https://github.com/martinruenz/gnuplot-cpp): another
    header-only library. It lacks support for high-level stuff like
    `std::vector` plotting.
