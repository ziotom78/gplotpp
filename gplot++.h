#pragma once

/* gplot++
 *
 * A header-only C++ interface to Gnuplot.
 *
 * Copyright 2020 Maurizio Tomasi
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
 *
 * Version history
 *
 * - 0.2.0 (2020/12/14): 3D plots
 *
 * - 0.1.0 (2020/11/29): first release
 *
 */

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

// The "sleep" function is non-standard
#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

const unsigned GNUPLOTPP_VERSION = 0x000201;
const unsigned GNUPLOTPP_MAJOR_VERSION = (GNUPLOTPP_VERSION & 0xFF0000) >> 16;
const unsigned GNUPLOTPP_MINOR_VERSION = (GNUPLOTPP_VERSION & 0x00FF00) >> 8;
const unsigned GNUPLOTPP_PATCH_VERSION = (GNUPLOTPP_VERSION & 0xFF);

/**
 * High-level interface to the Gnuplot executable
 *
 * This class establishes a connection with a new Gnuplot instance and
 * sends commands to it through a pipe. It is able to directly plot
 * vectors of numbers by saving them in temporary files.
 */
class Gnuplot {
private:
  // Create a name for a temporary file and return it
  std::string tmp_file_name() {
    // Calling "tmpnam" makes GCC emit a warning, but there is no
    // portable way to create a temporary file name in C++
    std::string filename{std::tmpnam(nullptr)};
    files_to_delete.push_back(filename);

    return filename;
  }

  std::string escape_quotes(const std::string &s) {
    std::string result{};

    for (char c : s) {
      if (c != '\'')
        result.push_back(c);
      else
        result += "''";
    }

    return result;
  }

public:
  enum class LineStyle {
    DOTS,
    LINES,
    POINTS,
    LINESPOINTS,
    STEPS,
    BOXES,
  };

  enum class AxisScale {
    LINEAR,
    LOGX,
    LOGY,
    LOGXY,
  };

  Gnuplot(const char *executable_name = "gnuplot", bool persist = true)
      : connection{}, series{}, files_to_delete{}, is_3dplot{false} {
    std::stringstream os;
    // The --persist flag lets Gnuplot keep running after the C++
    // program has completed its execution
    os << executable_name;
    if (persist)
      os << " --persist";
    connection = popen(os.str().c_str(), "w");

    set_xrange();
    set_yrange();
    set_zrange();

    // See
    // https://stackoverflow.com/questions/28152719/how-to-make-gnuplot-use-the-unicode-minus-sign-for-negative-numbers
    sendcommand("set encoding utf8\n");
    sendcommand("set minussign");
  }

  ~Gnuplot() {
    // Bye bye, Gnuplot!
    if (connection) {
      pclose(connection);
      connection = nullptr;
    }

    // Let some time pass before removing the files, so that Gnuplot
    // can finish displaying the last plot.
    sleep(1);

    // Now remove the data files
    for (const auto &fname : files_to_delete) {
      std::remove(fname.c_str());
    }
  }

  /* This is the most low-level method in the Gnuplot class! It
         returns `true` if the send command was successful, `false`
         otherwise. */
  bool sendcommand(const char *str) {
    if (!ok())
      return false;

    fputs(str, connection);
    fputc('\n', connection);
    fflush(connection);

    return true;
  }

  bool sendcommand(const std::string &str) { return sendcommand(str.c_str()); }
  bool sendcommand(const std::stringstream &stream) {
    return sendcommand(stream.str());
  }

  bool ok() { return connection != nullptr; }

  /* Save the plot to a PNG file instead of displaying a window */
  bool redirect_to_png(const std::string &filename,
                       const std::string &size = "800,600") {
    std::stringstream os;

    os << "set terminal pngcairo color enhanced size " << size << "\n"
       << "set output '" << filename << "'\n";
    return sendcommand(os);
  }

  /* Save the plot to a PDF file instead of displaying a window */
  bool redirect_to_pdf(const std::string &filename,
                       std::string size = "16cm,12cm") {
    std::stringstream os;

    os << "set terminal pdfcairo color enhanced size " << size << "\n"
       << "set output '" << filename << "'\n";
    return sendcommand(os);
  }

  /* Set the label on the X axis */
  bool set_xlabel(const std::string &label) {
    std::stringstream os;
    os << "set xlabel '" << escape_quotes(label) << "'";
    return sendcommand(os);
  }

  /* Set the label on the Y axis */
  bool set_ylabel(const std::string &label) {
    std::stringstream os;
    os << "set ylabel '" << escape_quotes(label) << "'";
    return sendcommand(os);
  }

  /* Set the minimum and maximum value to be displayed along the X axis */
  void set_xrange(double min = NAN, double max = NAN) {
    xrange = format_range(min, max);
  }

  /* Set the minimum and maximum value to be displayed along the X axis */
  void set_yrange(double min = NAN, double max = NAN) {
    yrange = format_range(min, max);
  }

  /* Set the minimum and maximum value to be displayed along the X axis */
  void set_zrange(double min = NAN, double max = NAN) {
    zrange = format_range(min, max);
  }

  bool set_logscale(AxisScale scale) {
    switch (scale) {
    case AxisScale::LOGX:
      return sendcommand("set logscale x");
    case AxisScale::LOGY:
      return sendcommand("set logscale y");
    case AxisScale::LOGXY:
      return sendcommand("set logscale xy");
    default:
      return sendcommand("unset logscale");
    }
  }

  template <typename T>
  void plot(const std::vector<T> &y, const std::string &label = "",
            LineStyle style = LineStyle::LINES) {
    if (y.empty())
      return;

    if (!series.empty()) {
      assert(!is_3dplot);
    }

    std::string filename{tmp_file_name()};
    std::ofstream of{filename};
    assert(of.good());
    for (const auto &val : y) {
      of << val << "\n";
    }

    series.push_back(GnuplotSeries{filename, style, label, "0:1"});
    is_3dplot = false;
  }

  template <typename T, typename U>
  void plot(const std::vector<T> &x, const std::vector<U> &y,
            const std::string &label = "", LineStyle style = LineStyle::LINES) {
    assert(x.size() == y.size());

    if (x.empty())
      return;

    if (!series.empty()) {
      assert(!is_3dplot);
    }

    std::string filename{tmp_file_name()};
    std::ofstream of{filename};
    assert(of.good());
    for (size_t i{}; i < x.size(); ++i) {
      of << x[i] << " " << y[i] << "\n";
    }

    series.push_back(GnuplotSeries{filename, style, label, "1:2"});
    is_3dplot = false;
  }

  template <typename T, typename U>
  void plot3d(const std::vector<T> &x, const std::vector<U> &y,
              const std::vector<U> &z, const std::string &label = "",
              LineStyle style = LineStyle::LINES) {
    assert(x.size() == y.size());
    assert(x.size() == z.size());

    if (x.empty())
      return;

    if (!series.empty()) {
      assert(is_3dplot);
    }

    std::string filename{tmp_file_name()};
    std::ofstream of{filename};
    assert(of.good());
    for (size_t i{}; i < x.size(); ++i) {
      of << x[i] << " " << y[i] << " " << z[i] << "\n";
    }

    series.push_back(GnuplotSeries{filename, style, label, "1:2:3"});
    is_3dplot = true;
  }

  template <typename T>
  void histogram(const std::vector<T> &values, size_t nbins,
                 const std::string &label = "",
                 LineStyle style = LineStyle::BOXES) {
    assert(nbins > 0);

    if (values.empty())
      return;

    if (!series.empty()) {
      assert(!is_3dplot);
    }

    double min = *std::min_element(values.begin(), values.end());
    double max = *std::max_element(values.begin(), values.end());
    double binwidth = (max - min) / nbins;

    std::vector<size_t> bins(nbins);
    for (const auto &val : values) {
      int index = (val - min) / binwidth;
      if (index >= int(nbins))
        --index;

      bins.at(index)++;
    }

    std::string filename{tmp_file_name()};
    std::ofstream of{filename};
    assert(of.good());
    for (size_t i{}; i < nbins; ++i) {
      of << min + binwidth * (i + 0.5) << " " << bins[i] << "\n";
    }

    series.push_back(GnuplotSeries{filename, style, label, "1:2"});
    is_3dplot = false;
  }

  bool multiplot(int nrows, int ncols, const std::string &title = "") {
    std::stringstream os;
    os << "set multiplot layout " << nrows << ", " << ncols << " title '"
       << escape_quotes(title) << "'\n";
    return sendcommand(os);
  }

  bool show(bool call_reset = true) {
    std::stringstream os;
    os << "set style fill solid 0.5\n";

    if (is_3dplot) {
      os << "splot " << xrange << " " << yrange << " " << zrange << " ";
    } else {
      os << "plot " << xrange << " " << yrange << " ";
    }
    for (size_t i{}; i < series.size(); ++i) {
      const GnuplotSeries &s = series.at(i);
      os << "'" << s.filename << "' using " << s.column_range << " with "
         << style_to_str(s.line_style) << " title '" << escape_quotes(s.title)
         << "'";

      if (i + 1 < series.size())
        os << ", ";
    }

    bool result = sendcommand(os);
    if (result && call_reset)
      reset();

    return result;
  }

  void reset() {
    series.clear();
    set_xrange();
    set_yrange();
    is_3dplot = false;
  }

private:
  struct GnuplotSeries {
    std::string filename;
    LineStyle line_style;
    std::string title;
    std::string column_range;
  };

  std::string style_to_str(LineStyle style) {
    switch (style) {
    case LineStyle::DOTS:
      return "dots";
    case LineStyle::POINTS:
      return "points";
    case LineStyle::LINESPOINTS:
      return "linespoints";
    case LineStyle::STEPS:
      return "steps";
    case LineStyle::BOXES:
      return "boxes";
    default:
      return "lines";
    }
  }

  std::string format_range(double min = NAN, double max = NAN) {
    if (std::isnan(min) || std::isnan(max))
      return "[]";

    std::stringstream os;
    os << "[" << min << ":" << max << "]";

    return os.str();
  }

  FILE *connection;
  std::vector<GnuplotSeries> series;
  std::vector<std::string> files_to_delete;
  std::string xrange;
  std::string yrange;
  std::string zrange;
  bool is_3dplot;
};
