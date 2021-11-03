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
#include <cmath>
#include <vector>

using namespace std;

int main(void) {
  Gnuplot gnuplot{};

  vector<double> x{}, y{}, z{};
  vector<double> vx{}, vy{}, vz{};

  // Sample a regular grid. Use a small tilt to avoid sampling
  // the grid at the origin
  for (double cur_x{-10.1}; cur_x <= 10; cur_x += 2) {
    for (double cur_y{-10.1}; cur_y <= 10; cur_y += 2) {
      for (double cur_z{-10.1}; cur_z <= 10; cur_z += 2) {
        x.push_back(cur_x);
        y.push_back(cur_y);
        z.push_back(cur_z);

        double r{sqrt(pow(cur_x, 2) + pow(cur_y, 2))};
        double cur_vx{-cur_x / r};
        double cur_vy{-cur_y / r};
        double cur_vz{-cur_z / r};
        vx.push_back(cur_vx);
        vy.push_back(cur_vy);
        vz.push_back(cur_vz);
      }
    }
  }

  gnuplot.redirect_to_png("example-vec3d.png");
  gnuplot.plot_vectors3d(x, y, z, vx, vy, vz);
  gnuplot.set_xrange(-10, 10);
  gnuplot.set_yrange(-10, 10);
  gnuplot.set_zrange(-10, 10);
  gnuplot.show();
}
