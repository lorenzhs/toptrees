#pragma once

#include <iostream>

// This progress bar is copied from one of my own earlier projects
// I wrote it myself but not for this project specifically
// As it is just eye-candy, please consider it not part of this project

/// A simple progress bar
class ProgressBar {
public:
	/// Create a new progress bar
	/// \param max the value that constitutes 100%
	/// \param out the output stream to draw the progress bar on
	/// \param barwidth the width of the bar in characters
	ProgressBar(const long long max, std::ostream &out = std::cout, int barwidth = 70)
		: max(max),
		  pos(0),
		  lastprogress(-1),
		  out(out),
		  barwidth(barwidth),
		  doDraw((out.rdbuf() == std::cout.rdbuf() || out.rdbuf() == std::cerr.rdbuf())) {
	}

	/// increase progress by 1 step (not percent!)
	void step() {
		++pos;
		draw();
	}

	/// set progress to a position
	/// \param newpos position to set the progress to (steps, not percent!)
	void stepto(long long newpos) {
		pos = newpos;
		draw();
	}

	void operator++() {
		step();
	}

	/// remove all traces of the bar from the output stream
	void undraw() {
		out << "\r";
		// "[" + "] " + percent (3) + " %" = up to 8 chars
		for (int i = 0; i < barwidth + 8; ++i) {
			out << " ";
		}
		out << "\r";
	}

protected:
	// adapted from StackOverflow user "leemes":
	// http://stackoverflow.com/a/14539953
	/// Draw the progress bar to the output stream
	void draw() {
		if (!doDraw) return;
		int progress = (int)((pos * 100) / max);
		if (progress == lastprogress) return;

		out << "[";
		int pos = barwidth * progress / 100;
		for (int i = 0; i < barwidth; ++i) {
			if (i < pos)
				out << "=";
			else if (i == pos)
				out << ">";
			else
				out << " ";
		}
		out << "] " << progress << " %\r";
		out.flush();

		lastprogress = progress;
	}

private:
	long long max;
	long long pos;
	int lastprogress;
	std::ostream &out;
	const int barwidth;
	const bool doDraw;
};
