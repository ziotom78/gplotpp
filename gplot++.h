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
 * - 0.9.1 (2025/02/06): bug fixes in method `histogram`
 *
 * - 0.9.0 (2024/11/19): new method `redirect_to_animated_gif`
 *
 * - 0.8.0 (2024/10/15): new methods `add_point_xerr`,
 *                       `add_point_yerr`, and `add_point_xyerr`,
 *                       and new overloads for functions
 *                       `plot_xerr`, `plot_yerr`, `plot_xyerr`
 *
 * - 0.7.0 (2023/11/27): new methods `add_point`
 *
 * - 0.6.0 (2023/01/26): new method `set_title`
 *
 * - 0.5.0 (2021/12/02): use a smarter algorithm to specify ranges
 *                       add `redirect_to_dumb` (and TerminalType enum)
 *
 * - 0.4.0 (2021/11/04): 2D/3D vector plots, SVG saving
 *
 * - 0.3.1 (2021/10/23): error bars
 *
 * - 0.3.0 (2021/10/23): support for Windows
 *
 * - 0.2.1 (2020/12/16): ensure that commands sent to Gnuplot are executed
 *   immediately
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
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

const unsigned GNUPLOTPP_VERSION = 0x000901;
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
  static FILE *safe_popen(const char *name, const char *mode) {
#ifdef _WIN32
    return _popen(name, mode);
#else
    return popen(name, mode);
#endif
  }

  static int safe_pclose(FILE *f) {
#ifdef _WIN32
    return _pclose(f);
#else
    return pclose(f);
#endif
  }

  static void safe_sleep(unsigned seconds) {
#ifdef _WIN32
    Sleep(seconds * 1000); // Sleep on Windows requires milliseconds
#else
    sleep(seconds);
#endif
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

  std::vector<double> list_of_x;
  std::vector<double> list_of_y;
  std::vector<double> list_of_xerr;
  std::vector<double> list_of_yerr;

  void check_consistency() const {
      assert(list_of_x.size() == list_of_y.size());

      if(list_of_xerr.size() > 0)
        assert(list_of_xerr.size() == list_of_x.size());
      if(list_of_yerr.size() > 0)
        assert(list_of_yerr.size() == list_of_y.size());
  }

public:
  enum class LineStyle {
    DOTS,
    LINES,
    POINTS,
    LINESPOINTS,
    STEPS,
    BOXES,
    X_ERROR_BARS,
    Y_ERROR_BARS,
    XY_ERROR_BARS,
    VECTORS,
  };

  enum class AxisScale {
    LINEAR,
    LOGX,
    LOGY,
    LOGXY,
  };

  enum class TerminalMode {
	MONO,
	ANSI,
	ANSI256,
	ANSIRGB,
  };

  Gnuplot(const char *executable_name = "gnuplot", bool persist = true)
      : connection{}, series{}, files_to_delete{}, is_3dplot{false} {
    std::stringstream os;
    // The --persist flag lets Gnuplot keep running after the C++
    // program has completed its execution
    os << executable_name;
    if (persist)
      os << " --persist";
    connection = safe_popen(os.str().c_str(), "w");

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
      safe_pclose(connection);
      connection = nullptr;
    }

    // Let some time pass before removing the files, so that Gnuplot
    // can finish displaying the last plot.
    safe_sleep(1);

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

  /* Save the plot to a SVG file instead of displaying a window */
  bool redirect_to_svg(const std::string &filename,
                       const std::string &size = "800,600") {
    std::stringstream os;

    os << "set terminal svg enhanced mouse standalone size " << size << "\n"
       << "set output '" << filename << "'\n";
    return sendcommand(os);
  }

  /* Save the plot to an animated GIF. The delay between frames is specified
   * by the `delay_ms` parameter (in milliseconds). If `loop` is `true` (the
   * default), the animation will loop indefinitely, otherwise it will just
   * play once and stop at the last frame.
   *
   * Note that frames are saved only once you call `Gnuplot::show()`. Therefore,
   * if you want to put N frames in your animation, you have to call
   * `Gnuplot::show()` exactly N times.
   */
  bool redirect_to_animated_gif(const std::string &filename,
								const std::string &size = "800,600",
								int delay_ms = 50,
								bool loop = true) {
	std::stringstream os;

	// Unfortunately, Gnuplot requires the delay to be expressed in units of 1/100 s
	// instead of the more common unit 1/1000 s, so we need to divide by 10.
	// We do not support a fixed number of repetitions: either you loop infinitely
	// or play the animation just once
	os << "set terminal gif animate delay " << delay_ms / 10 << " loop " << (loop ? 0 : 1) << "\n"
	   << "set output '" << filename << "'\n";
	return sendcommand(os);
  }

  /* Send the plot to the terminal or to a text file */
  bool redirect_to_dumb(const std::string &filename = "",
                        unsigned int width = 80,
						unsigned int height = 50,
						TerminalMode mode = TerminalMode::MONO) {
    std::stringstream os;

	os << "set terminal dumb size " << width << " " << height;

	switch(mode) {
	case TerminalMode::MONO: os << "mono"; break;
	case TerminalMode::ANSI: os << "ansi"; break;
	case TerminalMode::ANSI256: os << "ansi256"; break;
	case TerminalMode::ANSIRGB: os << "ansirgb"; break;
	default: os << "mono";
	}

	os << "\n";

	if (! filename.empty()) {
	  os << "set output '" << filename << "'\n";
	}

    return sendcommand(os);
  }

  bool set_title(const std::string &title) {
	std::stringstream os;
	os << "set title '" << escape_quotes(title) << "'";
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
    _plot(label, style, false, y);
  }

  template <typename T, typename U>
  void plot(const std::vector<T> &x, const std::vector<U> &y,
            const std::string &label = "", LineStyle style = LineStyle::LINES) {
    _plot(label, style, false, x, y);
  }

  template <typename T, typename U, typename V>
  void plot_xerr(const std::vector<T> &x, const std::vector<U> &y,
                 const std::vector<V> &err, const std::string &label = "") {
    _plot(label, LineStyle::X_ERROR_BARS, false, x, y, err);
  }

  template <typename T, typename U, typename V>
  void plot_yerr(const std::vector<T> &x, const std::vector<U> &y,
                 const std::vector<V> &err, const std::string &label = "") {
    _plot(label, LineStyle::Y_ERROR_BARS, false, x, y, err);
  }

  template <typename T, typename U, typename V, typename W>
  void plot_xyerr(const std::vector<T> &x, const std::vector<U> &y,
                  const std::vector<V> &xerr, const std::vector<W> &yerr,
                  const std::string &label = "") {
    _plot(label, LineStyle::XY_ERROR_BARS, false, x, y, xerr, yerr);
  }

  template <typename T, typename U, typename V, typename W>
  void plot_vectors(const std::vector<T> &x, const std::vector<U> &y,
                    const std::vector<V> &vx, const std::vector<W> &vy,
                    const std::string &label = "") {
    _plot(label, LineStyle::VECTORS, false, x, y, vx, vy);
  }

  template <typename T, typename U>
  void plot3d(const std::vector<T> &x, const std::vector<U> &y,
              const std::vector<U> &z, const std::string &label = "",
              LineStyle style = LineStyle::LINES) {
    _plot(label, style, true, x, y, z);
  }

  template <typename T, typename U, typename V, typename W>
  void plot_vectors3d(const std::vector<T> &x, const std::vector<U> &y,
                      const std::vector<T> &z, const std::vector<V> &vx,
                      const std::vector<W> &vy, const std::vector<T> &vz,
                      const std::string &label = "") {
    _plot(label, LineStyle::VECTORS, true, x, y, z, vx, vy, vz);
  }

  /* Add a point and a X error bar to the list of samples to be plotted */
  void add_point_xerr(double x, double y, double err) {
    check_consistency();

    list_of_x.push_back(x);
    list_of_y.push_back(y);
    list_of_xerr.push_back(err);
  }

  /* Add a point and a Y error bar to the list of samples to be plotted */
  void add_point_yerr(double x, double y, double err) {
    check_consistency();

    list_of_x.push_back(x);
    list_of_y.push_back(y);
    list_of_yerr.push_back(err);
  }

  /* Add a point and two X/Y error bars to the list of samples to be plotted */
  void add_point_xyerr(double x, double y, double xerr, double yerr) {
    check_consistency();

    list_of_x.push_back(x);
    list_of_y.push_back(y);
    list_of_xerr.push_back(xerr);
    list_of_yerr.push_back(yerr);
  }
  /* Add a point to the list of samples to be plotted */
  void add_point(double x, double y) {
      check_consistency();

      list_of_x.push_back(x);
      list_of_y.push_back(y);
  }

  /* Add a value to the list of samples to be plotted */
  void add_point(double y) {
      add_point(static_cast<double>(list_of_x.size()), y);
  }

  /* Return the number of points added by `add_point` */
  int get_num_of_points() const {
      check_consistency();

      return (int) list_of_x.size();
  }

  /* Return the list of abscissas for the points added by `add_point` */
  const std::vector<double> & get_points_x() const { return list_of_x; }

    /* Return the list of ordinates for the points added by `add_point` */
  const std::vector<double> & get_points_y() const { return list_of_y; }

  /* Create a plot using the values set with the method `add_point` */
  void plot(const std::string &label = "", LineStyle style = LineStyle::LINES) {
      check_consistency();

      _plot(label, style, false, list_of_x, list_of_y);
  }

  /* Create a plot with X error bars using the values set with the method `add_point` */
  void plot_xerr(const std::string &label = "") {
    check_consistency();

    plot_xerr(list_of_x, list_of_y, list_of_xerr, label);
  }

  /* Create a plot with X error bars using the values set with the method `add_point` */
  void plot_yerr(const std::string &label = "") {
    check_consistency();

    plot_yerr(list_of_x, list_of_y, list_of_yerr, label);
  }

  /* Create a plot with X error bars using the values set with the method `add_point` */
  void plot_xyerr(const std::string &label = "") {
    check_consistency();

    plot_xyerr(list_of_x, list_of_y, list_of_xerr, list_of_yerr, label);
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

    auto min_iter = std::min_element(values.begin(), values.end());
    auto max_iter = std::max_element(values.begin(), values.end());
    double min, max, binwidth;

    std::vector<size_t> bins{};

  	// Check if all the elements are the same
  	if (min_iter != max_iter) {
  	  min = *min_iter;
  	  max = *max_iter;
  	  binwidth = (max - min) / nbins;

  	  bins.resize(nbins);
  	  for (const auto &val : values) {
    		int index = static_cast<int>((val - min) / binwidth);
    		if (index >= int(nbins))
    		  --index;

    		bins.at(index)++;
  	  }
  	} else {
  	  // Just one bin…

  	  min = max = *min_iter;
  	  binwidth = 1.0;
  	  nbins = 1;
  	  bins.push_back(static_cast<double>(values.size()));
  	}

    std::stringstream of;
    for (size_t i{}; i < nbins; ++i) {
      of << min + binwidth * (i + 0.5) << " " << bins[i] << "\n";
    }

    series.push_back(GnuplotSeries{of.str(), style, label, "1:2"});
    is_3dplot = false;
  }

  // Ask Gnuplot to use a multiple-plot layout
  bool multiplot(int nrows, int ncols, const std::string &title = "") {
    std::stringstream os;
    os << "set multiplot layout " << nrows << ", " << ncols << " title '"
       << escape_quotes(title) << "'\n";
    return sendcommand(os);
  }

  // Force Gnuplot to draw all the series sent through any of the `plot`
  // commands
  bool show(bool call_reset = true) {
    if (series.empty())
      return true;

    std::stringstream os;
    os << "set style fill solid 0.5\n";

    // Write the data in separate series
    for (size_t i{}; i < series.size(); ++i) {
      const GnuplotSeries &s = series.at(i);
      os << "$Datablock" << i << " << EOD\n" << s.data_string << "\nEOD\n";
    }

    if (is_3dplot) {
      os << "splot " << xrange << " " << yrange << " " << zrange << " ";
    } else {
      os << "plot " << xrange << " " << yrange << " ";
    }

    // Plot the series we have just defined
    for (size_t i{}; i < series.size(); ++i) {
      const GnuplotSeries &s = series.at(i);
      os << "$Datablock" << i << " using " << s.column_range << " with "
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

  // Remove all the series from memory and start with a blank plot
  void reset() {
    series.clear();
    set_xrange();
    set_yrange();
    is_3dplot = false;
  }

private:
  void _print_ith_elements(std::ostream &, std::ostream &, int, size_t) {}

  template <typename T, typename... Args>
  void _print_ith_elements(std::ostream &os, std::ostream &fmts, int index,
                           size_t i, const std::vector<T> &v, Args... args) {
    os << v[i] << " ";

    if (i == 0) {
      if (index > 1)
        fmts << ':';
      fmts << index;
    }

    _print_ith_elements(os, fmts, index + 1, i, args...);
  }

  template <typename T, typename... Args>
  void _plot(const std::string &label, LineStyle style, bool is_this_3dplot,
             const std::vector<T> &v, Args... args) {
    if (v.empty())
      return;

    if (!series.empty()) {
      // Check that we are not adding a 3D plot to a 2D plot, or vice versa
      assert(is_3dplot == is_this_3dplot);
    }

    std::stringstream of;
    std::stringstream fmtstring;
    for (size_t i{}; i < v.size(); ++i) {
      _print_ith_elements(of, fmtstring, 1, i, v, args...);
      of << "\n";
    }

    series.push_back(GnuplotSeries{of.str(), style, label, fmtstring.str()});
    is_3dplot = is_this_3dplot;
  }

  struct GnuplotSeries {
    std::string data_string;
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
    case LineStyle::X_ERROR_BARS:
      return "xerrorbars";
    case LineStyle::Y_ERROR_BARS:
      return "yerrorbars";
    case LineStyle::XY_ERROR_BARS:
      return "xyerrorbars";
    case LineStyle::VECTORS:
      return "vectors";
    default:
      return "lines";
    }
  }

  std::string format_range(double min = NAN, double max = NAN) {
    if (std::isnan(min) && std::isnan(max))
      return "[]";

    std::stringstream os;
    os << "[";

    if (std::isnan(min))
      os << "*";
    else
      os << min;

    os << ":";
    if (std::isnan(max))
      os << "*";
    else
      os << max;

    os << "]";

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
