#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#include <openssl/rand.h>

static void csprng_bytes(unsigned char* buf, size_t len) {
    if (RAND_bytes(buf, static_cast<int>(len)) != 1) {
        throw std::runtime_error("RAND_bytes failed");
    }
}

static size_t uniform_index(size_t n) {
    if (n == 0) throw std::invalid_argument("n must be > 0");
    const unsigned int max = 256 - (256 % n);
    unsigned char b;
    do { csprng_bytes(&b, 1); } while (b >= max);
    return b % n;
}

template <typename T>
static void secure_shuffle(std::vector<T>& v) {
    for (size_t i = v.size(); i > 1; --i) {
        size_t j = uniform_index(i);
        std::swap(v[i - 1], v[j]);
    }
}

static std::string generate_password(
    size_t length,
    const std::string& specials_override,
    bool no_special
) {
    if (length < 16) throw std::invalid_argument("length must be >= 16");

    const std::string U = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    const std::string L = "abcdefghijklmnopqrstuvwxyz";
    const std::string D = "0123456789";
    const std::string S = specials_override.empty()
        ? std::string("!@#$%^&*()-_=+[]{};:,.?")
        : specials_override;

    std::vector<const std::string*> cats = { &U, &L, &D };
    if (!no_special) cats.push_back(&S);

    std::vector<char> out;
    out.reserve(length);

    for (const auto* c : cats) {
        for (int k = 0; k < 2; ++k) {
            out.push_back((*c)[uniform_index(c->size())]);
        }
    }

    std::string all;
    for (auto* c : cats) all += *c;

    while (out.size() < length) {
        out.push_back(all[uniform_index(all.size())]);
    }

    secure_shuffle(out);
    return std::string(out.begin(), out.end());
}

static void write_txt(const std::string& path, const std::vector<std::string>& pwds) {
    std::ofstream f(path, std::ios::binary);
    if (!f) throw std::runtime_error("Failed to open TXT file for writing: " + path);
    for (const auto& p : pwds) {
        f << p << "\r\n";
    }
}

static std::string csv_escape(const std::string& s) {
    bool needs_quotes = false;
    for (unsigned char c : s) {
        if (c == ',' || c == '"' || c == '\n' || c == '\r') { needs_quotes = true; break; }
    }
    if (!needs_quotes) return s;
    std::string out = "\"";
    for (char c : s) out += (c == '"' ? std::string("\"\"") : std::string(1, c));
    out += "\"";
    return out;
}

static void write_csv(const std::string& path, const std::vector<std::string>& pwds) {
    std::ofstream f(path, std::ios::binary);
    if (!f) throw std::runtime_error("Failed to open CSV file for writing: " + path);

    const unsigned char bom[3] = { 0xEF, 0xBB, 0xBF };
    f.write(reinterpret_cast<const char*>(bom), 3);

    f << "#,Password\r\n";
    for (size_t i = 0; i < pwds.size(); ++i) {
        f << (i + 1) << "," << csv_escape(pwds[i]) << "\r\n";
    }
}

struct Options {
    size_t count = 10;
    size_t length = 16;
    std::string specials;
    bool quiet = false;
    bool nospecial = false;
    std::string out_txt;
    std::string out_csv;
};

static void print_usage(const char* argv0) {
    std::cerr <<
        "Usage: " << argv0 << " [options]\n"
        "Options:\n"
        "  -n <N>          Number of passwords (default 1)\n"
        "  -l <L>          Password length (>=16, default 16)\n"
        "  --specials s    Override special characters set\n"
        "  --nospecial     Exclude special characters entirely\n"
        "  -txt <file>     Save passwords to a .txt file (one per line)\n"
        "  -csv <file>     Save passwords to a CSV (Excel-friendly, numbered)\n"
        "  -q              Quiet mode (only print passwords to stdout)\n"
        "  -h              Show help\n";
}

static bool parse_size_t(const std::string& s, size_t& out) {
    try {
        size_t idx = 0;
        unsigned long long v = std::stoull(s, &idx, 10);
        if (idx != s.size()) return false;
        out = static_cast<size_t>(v);
        return true;
    }
    catch (...) { return false; }
}

static Options parse_args(int argc, char** argv) {
    Options opt;
    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        if (a == "-h") {
            print_usage(argv[0]);
            std::exit(0);
        }
        else if (a == "-n") {
            if (i + 1 >= argc) { std::cerr << "Missing value for -n\n"; std::exit(1); }
            if (!parse_size_t(argv[++i], opt.count) || opt.count == 0) {
                std::cerr << "Invalid count.\n"; std::exit(1);
            }
        }
        else if (a == "-l") {
            if (i + 1 >= argc) { std::cerr << "Missing value for -l\n"; std::exit(1); }
            if (!parse_size_t(argv[++i], opt.length) || opt.length < 16) {
                std::cerr << "Invalid length (must be >=16).\n"; std::exit(1);
            }
        }
        else if (a == "--specials") {
            if (i + 1 >= argc) { std::cerr << "Missing value for --specials\n"; std::exit(1); }
            opt.specials = argv[++i];
        }
        else if (a == "--nospecial") {
            opt.nospecial = true;
        }
        else if (a == "-txt") {
            if (i + 1 >= argc) { std::cerr << "Missing value for -txt\n"; std::exit(1); }
            opt.out_txt = argv[++i];
        }
        else if (a == "-csv") {
            if (i + 1 >= argc) { std::cerr << "Missing value for -csv\n"; std::exit(1); }
            opt.out_csv = argv[++i];
        }
        else if (a == "-q") {
            opt.quiet = true;
        }
        else {
            std::cerr << "Unknown option: " << a << "\n";
            print_usage(argv[0]);
            std::exit(1);
        }
    }
    return opt;
}

int main(int argc, char** argv) {
    Options opt = parse_args(argc, argv);

    try {
        std::vector<std::string> pwds;
        pwds.reserve(opt.count);

        for (size_t i = 0; i < opt.count; ++i) {
            pwds.emplace_back(generate_password(opt.length, opt.specials, opt.nospecial));
        }

        if (!opt.quiet) {
            std::cout << "Generated " << pwds.size() << " password(s) of length "
                << opt.length << (opt.nospecial ? " (no specials)" : "") << ".\n";
        }

        for (const auto& p : pwds) {
            std::cout << p << "\n";
        }

        if (!opt.out_txt.empty()) {
            write_txt(opt.out_txt, pwds);
            if (!opt.quiet) std::cout << "Wrote TXT: " << opt.out_txt << "\n";
        }

        if (!opt.out_csv.empty()) {
            write_csv(opt.out_csv, pwds);
            if (!opt.quiet) std::cout << "Wrote CSV: " << opt.out_csv << "\n";
        }

    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
