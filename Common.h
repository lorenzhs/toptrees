#pragma once

#include <random>
#include <stack>
#include <sys/stat.h>

#ifdef NDEBUG
const bool global_debug = false;
#else
const bool global_debug = true;
#endif

// Branch prediction hints
#define likely(x)   __builtin_expect((x), 1)
#define unlikely(x) __builtin_expect((x), 0)

// hacky way to calculate the number of digits in a number, for std::setw
#define NUM_DIGITS(v) ((v >= 1000000000) ? 10 : (v >= 100000000) ? 9 : (v >= 10000000) ? 8 : (v >= 1000000) ? 7 : (v >= 100000) ? 6 : (v >= 10000) ? 5 : (v >= 1000) ? 4 : (v >= 100) ? 3 : (v >= 10) ? 2 : 1)

template <typename T>
T log2(T val) {
    T result(0);
    while (val > 1) {
        result += 1;
        val >>= 1;
    }
    return result;
}

/// Types of merges in the top tree (see top tree compression paper for details)
enum MergeType { NO_MERGE = -1, VERT_WITH_BBN, VERT_NO_BBN, HORZ_LEFT_BBN, HORZ_RIGHT_BBN, HORZ_NO_BBN };


// adapted from: http://www.boost.org/doc/libs/1_55_0/doc/html/hash/reference.html#boost.hash_combine
// the magic number is the binary extension of the golden ratio
// the idea is that every bit is random. A more detailed explanation is available at
// http://stackoverflow.com/questions/4948780/magic-number-in-boosthash-combine
template <typename T>
static void boost_hash_combine(uint &seed, const T &val) {
    std::hash<T> hasher;
    seed ^= hasher(val) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

/// the type of random generator used
typedef std::mt19937 RandomGeneratorType;
/// shared random generator
std::mt19937 &getRandomGenerator() {
    static std::mt19937 engine{};
    return engine;
}


/// Recursively create a path (similar to "mkdir -p")
/// see also http://stackoverflow.com/a/11366985 by StackOverflow user "Mark"
bool makePathRecursive(std::string path, int permissions = 0755) {
    int nRC = mkdir(path.c_str(), permissions);
    if (nRC == -1) {
        switch (errno) {
        case ENOENT:
            // parent didn't exist, try to create it recursively
            if (makePathRecursive(path.substr(0, path.find_last_of('/'))))
                // Now, try to create again.
                return 0 == mkdir(path.c_str(), permissions);
            else
                return false;
        case EEXIST:
            // Exists (may not be a directory though)
            struct stat s;
            stat(path.c_str(), &s);
            return S_ISDIR(s.st_mode);
        default:
            return false;
        }
    }
    // successfully created by mkdir call, no recursion needed
    return true;
}

namespace std {
template <typename T>
ostream &operator<<(ostream &os, const vector<T> &v) {
    const size_t size = v.size();
    os << "vec(" << size << ")[";
    for (size_t i = 0; i < size; ++i) {
        os << v[i] << (i + 1 < size ? ", " : "");
    }
    return os << "];" << endl;
}

template <typename T>
ostream &operator<<(ostream &os, const deque<T> &d) {
    const size_t size = d.size();
    os << "deque(" << size << ")[";
    for (size_t i = 0; i < size; ++i) {
        os << d[i] << (i + 1 < size ? ", " : "");
    }
    return os << "];" << endl;
}

template <typename T1, typename T2>
ostream &operator<<(ostream &os, const pair<T1, T2> &p) {
    return os << "(" << p.first << "," << p.second << ")";
}
}


// Hack for op<<ing std::stack http://stackoverflow.com/a/4523216
template <typename T, typename Container>
const Container& container(const std::stack<T, Container> &stack) {
    struct HackedStack : private std::stack<T, Container> {
        static const Container& container(const std::stack<T, Container> &stack) {
            return stack.*&HackedStack::c;
        }
    };
    return HackedStack::container(stack);
}

template <typename T,
          template <typename T2,
                    typename Container = std::deque<T2>> class Adapter,
          typename Stream>
Stream& operator<< (Stream &out, const Adapter<T> &adapter) {
    auto data = container(adapter);

    out << "[";
    auto it = data.begin(), end = data.end();
    while (it != end) {
        out << *it++ << (it < end ? ", " : "");
    }
    return out << "]";
}
