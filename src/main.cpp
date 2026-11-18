#include "lex/lexer_generator.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace {

void print_usage(std::string_view program_name) {
    std::cout << "Usage: " << program_name << " [options] <input.l>\n\n"
              << "A modern C++20 Lexical Analyzer Generator (Lex)\n"
              << "Designed in the style of Bjarne Stroustrup (RAII, type safety, value semantics)\n\n"
              << "Options:\n"
              << "  -o <file>             Specify output C++ source file (default: lex.yy.cpp)\n"
              << "  --header <file>       Specify output C++ header file (optional)\n"
              << "  --class-name <name>   Name of generated lexer class (default: Lexer)\n"
              << "  --namespace <name>    Namespace for generated lexer (optional)\n"
              << "  --dot-nfa <file>      Export combined NFA to Graphviz DOT format\n"
              << "  --dot-dfa <file>      Export minimized DFA to Graphviz DOT format\n"
              << "  -v, --verbose         Print compilation statistics\n"
              << "  -h, --help            Display this help message and exit\n";
}

} // namespace

int main(int argc, char* argv[]) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    std::string input_file;
    std::string output_source = "lex.yy.cpp";
    std::string output_header;
    std::string dot_nfa_file;
    std::string dot_dfa_file;
    bool verbose = false;

    lex::Generator_options options;

    for (int i = 1; i < argc; ++i) {
        std::string_view arg = argv[i];
        if (arg == "-h" || arg == "--help") {
            print_usage(argv[0]);
            return 0;
        } else if (arg == "-v" || arg == "--verbose") {
            verbose = true;
        } else if (arg == "-o" && i + 1 < argc) {
            output_source = argv[++i];
        } else if (arg == "--header" && i + 1 < argc) {
            output_header = argv[++i];
        } else if (arg == "--class-name" && i + 1 < argc) {
            options.class_name = argv[++i];
        } else if (arg == "--namespace" && i + 1 < argc) {
            options.namespace_name = argv[++i];
        } else if (arg == "--dot-nfa" && i + 1 < argc) {
            dot_nfa_file = argv[++i];
        } else if (arg == "--dot-dfa" && i + 1 < argc) {
            dot_dfa_file = argv[++i];
        } else if (arg.front() == '-') {
            std::cerr << "lex_cpp: unrecognized command-line option '" << arg << "'\n";
            return 1;
        } else {
            if (!input_file.empty()) {
                std::cerr << "lex_cpp: multiple input files specified: '" << input_file
                          << "' and '" << arg << "'\n";
                return 1;
            }
            input_file = std::string(arg);
        }
    }

    if (input_file.empty()) {
        std::cerr << "lex_cpp: no input specification file specified.\n";
        return 1;
    }

    try {
        std::ifstream in(input_file);
        if (!in.is_open()) {
            std::cerr << "lex_cpp: unable to open input file '" << input_file << "'\n";
            return 1;
        }

        std::ostringstream ss;
        ss << in.rdbuf();
        std::string content = ss.str();

        lex::Lexer_generator generator{options};
        auto result = generator.generate(content);

        if (verbose) {
            std::cout << "Lex compilation statistics for '" << input_file << "':\n"
                      << "  Combined NFA states: " << result.nfa_states << "\n"
                      << "  Subset DFA states:   " << result.dfa_states << "\n"
                      << "  Minimized DFA states:" << result.min_dfa_states << "\n";
        }

        // Write output source file
        {
            std::ofstream out(output_source);
            if (!out.is_open()) {
                std::cerr << "lex_cpp: unable to write output source to '" << output_source << "'\n";
                return 1;
            }
            out << result.source_code;
        }

        // Write output header file if requested
        if (!output_header.empty()) {
            std::ofstream out(output_header);
            if (!out.is_open()) {
                std::cerr << "lex_cpp: unable to write output header to '" << output_header << "'\n";
                return 1;
            }
            out << result.header_code;
        }

        // Write DOT graph files if requested
        if (!dot_nfa_file.empty()) {
            std::ofstream out(dot_nfa_file);
            if (out.is_open()) {
                out << result.dot_nfa;
            }
        }

        if (!dot_dfa_file.empty()) {
            std::ofstream out(dot_dfa_file);
            if (out.is_open()) {
                out << result.dot_dfa;
            }
        }

        if (verbose) {
            std::cout << "Generated scanner written to '" << output_source << "'\n";
        }

    } catch (const std::exception& e) {
        std::cerr << "lex_cpp: error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
