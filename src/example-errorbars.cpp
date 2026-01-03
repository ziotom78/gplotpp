/* Copyright 2020 Maurizio Tomasi
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "gplot++.h"

int main(void) {
  Gnuplot plt{};
  std::vector<double> x{1, 2, 3, 4, 5}, y{5, 2, 4, 1, 3};
  std::vector<double> xerr{0.2, 0.2, 0.1, 0.1, 0.2},
      yerr{0.3, 0.4, 0.2, 0.6, 0.7};

  // Save the plot into a PNG file with the desired size (in pixels)
  plt.redirect_to_png("errorbars.png", "800,600");

  /* Create four plots with the following layout (2 rows, 2 columns):
   *
   * +------------------------+------------------------+
   * |                        |                        |
   * |        Plot #1         |        Plot #2         |
   * |                        |                        |
   * +------------------------+------------------------+
   * |                        |                        |
   * |        Plot #3         |        Plot #4         |
   * |                        |                        |
   * +------------------------+------------------------+
   */
  plt.multiplot(2, 2, "Title");

  // Plot #1
  plt.set_xlabel("X axis");
  plt.set_ylabel("Y axis");
  plt.plot(x, y, "", Gnuplot::LineStyle::POINTS);
  plt.show(); // Always call "show"!

  // Plot #2: X error bar
  plt.set_xlabel("X axis");
  plt.set_ylabel("Y axis");
  plt.plot_xerr(x, y, xerr);
  plt.show(); // Always call "show"!

  // Plot #3: Y error bar
  plt.set_xlabel("X axis");
  plt.set_ylabel("Y axis");
  plt.plot_yerr(x, y, yerr);
  plt.show(); // Always call "show"!

  // Plot #4: X and Y error bars
  plt.set_xlabel("X axis");
  plt.set_ylabel("Y axis");
  plt.plot_xyerr(x, y, xerr, yerr);
  plt.show(); // Always call "show"!
}
