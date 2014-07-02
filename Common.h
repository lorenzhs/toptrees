#pragma once

// for std::setw
#define NUM_DIGITS(v) ((v >= 1000000000) ? 10 : (v >= 100000000) ? 9 : (v >= 10000000) ? 8 : (v >= 1000000) ? 7 : (v >= 100000) ? 6 : (v >= 10000) ? 5 : (v >= 1000) ? 4 : (v >= 100) ? 3 : (v >= 10) ? 2 : 1)

enum MergeType { NO_MERGE = -1, VERT_WITH_BBN, VERT_NO_BBN, HORZ_LEFT_BBN, HORZ_RIGHT_BBN, HORZ_NO_BBN };
