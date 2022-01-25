#include "settings.hpp"

#include <ProgramOptions.hxx>
#include <eta/core/colors.hpp>
#include <eta/core/help.hpp>
#include <iostream>

#include "errors.hpp"

#define PRINT_ERROR(error, descr) eta::print_error(static_cast<int>(errors_t::error), #error, descr)

static auto print_help() -> void {
    eta::set_color(std::cerr, eta::color_t::green);
    std::cerr << "eta";
    eta::set_color(std::cerr, eta::color_t::standard);
    std::cerr << " 0.2\n"
                 "A program implementing TRedA reduction algorithm.\n"
                 "It reads its inputs on stdin and write output on stdout.\n\n";

    eta::print_section("USAGE");
    eta::set_color(std::cerr, eta::color_t::standard);
    std::cerr << "    eta [OPTIONS]...\n\n";

    eta::print_section("OPTIONS");
    eta::print_subsection("General options");
    eta::print_option('h', "help", "Show this help.");
    eta::print_option('D', "debug", "Print debug informations.");
    eta::print_option('c', "check", "Enable integrity and validity checks.");
    eta::print_option(0,
                      "replay",
                      "Replay the trace and validate each event is correctly predicted using the "
                      "reduced trace.");
    eta::print_subsection("Input options");
    eta::print_option('n', "non-printable", "Don't ignore non-printable characters.");
    eta::print_option('l', "logs", "Take each line of input as an atomic item");
    eta::print_option('L', "lines", "Take each line of input as a different string to reduce");
    eta::print_option('b', "binary-input", "Expect input to be in binary format.");
    eta::print_option('f', "input-file", "Read input from a file.");
    eta::print_option('s', "input-string", "Give string at input directly.");
    eta::print_subsection("Output options");
    eta::print_option('i', "print-input", "Print input before.");
    eta::print_option('d', "dot", "Produce output under dot format.");
    eta::print_option('g', "grammar", "Format output as a grammar.");
    eta::print_option('B', "binary-output", "Use binary format for output");
    eta::print_option('e', "expend", "Print original input");
    eta::print_option('t', "tree", "Produce output as tree");
    eta::print_option('T', "terminals", "Print used terminals");
    eta::print_option(0, "no-color", "Don't use color and formating in output.");
#ifdef ETA_GUI_ENABLED
    eta::print_option(0, "gui", "Display result in interactive window.");
#endif

    std::cerr << '\n';
    eta::print_section("ERRORS");
    PRINT_ERROR(BAD_ARGUMENTS, "Incompatibily between arguments given by the user.");
    PRINT_ERROR(INPUT_OUTPUT_MISMATCH, "Too many inputs for output mode.");
    PRINT_ERROR(CHECK_FAILED, "Data structure checks failed. Report this as a bug.");
    PRINT_ERROR(NOT_IMPLEMENTED_FEATURE, "The requested feature is not implemented yet.");
    PRINT_ERROR(FAILED_TO_OPEN_INPUT_FILE, "The specified input file cannot be open.");
}

auto count_bools() { return 0; }
template <typename... T> auto count_bools(bool b, T &&... others) {
    return (b ? 1 : 0) + count_bools(others...);
}

auto parse_settings(int argc, char ** argv) -> settings_t {
    auto parser = po::parser {};
    auto settings = settings_t {};

    // general
    auto & debug_opt = parser["debug"].abbreviation('D');
    auto & help_opt = parser["help"].abbreviation('h');
    auto & check_opt = parser["check"].abbreviation('c');
    auto & replay_opt = parser["replay"];

    // input
    auto & non_printable_opt = parser["non-printable"].abbreviation('n');
    auto & line_opt = parser["lines"].abbreviation('L');
    auto & logs_opt = parser["logs"].abbreviation('l');
    auto & binary_input_opt = parser["binary-input"].abbreviation('b');
    auto & input_file_opt = parser["input-file"].abbreviation('f').type(po::string).multi();
    auto & input_str_opt = parser["input-string"].abbreviation('s').type(po::string).multi();

    // output
    auto & print_input_opt = parser["print-input"].abbreviation('i');
    auto & dot_opt = parser["dot"].abbreviation('d');
    auto & grammar_opt = parser["grammar"].abbreviation('g');
    auto & binary_output_opt = parser["binary-output"].abbreviation('B');
    auto & no_color_opt = parser["no-color"];
    auto & expend_opt = parser["expend"].abbreviation('e');
    auto & tree_opt = parser["tree"].abbreviation('t');
    auto & terminals_opt = parser["terminals"].abbreviation('T');
#ifdef ETA_GUI_ENABLED
    auto & gui_opt = parser["gui"];
#endif

    if (!parser(argc, argv)) {
        exit(errors_t::BAD_ARGUMENTS);
    }

    // general
    auto const debug = debug_opt.was_set();
    auto const help = help_opt.was_set();
    auto const check = check_opt.was_set();
    auto const no_color = no_color_opt.was_set();
    auto const replay = replay_opt.was_set();

    // input
    auto const non_printable = non_printable_opt.was_set();
    auto const lines = line_opt.was_set();
    auto const logs = logs_opt.was_set();
    auto const binary_input = binary_input_opt.was_set();
    auto const input_file = input_file_opt.was_set();
    auto const input_string = input_str_opt.was_set();

    // output
    auto const print_input = print_input_opt.was_set();
    auto const dot = dot_opt.was_set();
    auto const grammar = grammar_opt.was_set();
    auto const binary_output = binary_output_opt.was_set();
    auto const expend = expend_opt.was_set();
    auto const tree = tree_opt.was_set();
    auto const terminals = terminals_opt.was_set();
#ifdef ETA_GUI_ENABLED
    auto const gui = gui_opt.was_set();
#else
    auto constexpr gui = false;
#endif

    // general settings
    settings.debug = debug;
    settings.check = check;
    settings.replay = replay;

    eta::disable_colors(no_color);

    // input settings
    if (count_bools(non_printable, lines, binary_input, logs) > 1) {
        set_color(std::cerr, eta::color_t::red);
        std::cerr << "error: ";
        set_color(std::cerr, eta::color_t::standard);
        std::cerr << "--non-printable, --lines, --logs"
                     " and --binary-input are mutually exclusive.\n\n";
        print_help();
        exit(errors_t::BAD_ARGUMENTS);
    }
    if (binary_input)
        settings.input_mode = input_t::binary;
    else if (non_printable)
        settings.input_mode = input_t::non_printable;
    else if (lines)
        settings.input_mode = input_t::lines;
    else if (logs)
        settings.input_mode = input_t::logs;
    else
        settings.input_mode = input_t::text;

    if (input_file && input_string) {
        set_color(std::cerr, eta::color_t::red);
        std::cerr << "error: ";
        set_color(std::cerr, eta::color_t::standard);
        std::cerr << "--input-string and --input_file are mutually exclusive.\n\n";
        print_help();
        exit(errors_t::BAD_ARGUMENTS);
    }
    if (input_file) {
        // settings.input_file = input_file_opt.get().string; TODO
        settings.input_src = input_src_t::file;
        for (auto const & file_name : input_file_opt)
            settings.input_data.emplace_back(file_name.string);
    } else if (input_string) {
        settings.input_src = input_src_t::argument;
        for (auto const & input : input_str_opt)
            settings.input_data.emplace_back(input.string);
    } else {
        settings.input_src = input_src_t::std_in;
    }

    // output settings

    if (count_bools(dot, grammar, expend, binary_output, terminals, tree, gui) > 1) {
        set_color(std::cerr, eta::color_t::red);
        std::cerr << "error: ";
        set_color(std::cerr, eta::color_t::standard);
        std::cerr << "--dot, --expend, --grammar, --tree, --terminals"
#ifdef ETA_GUI_ENABLED
                     ", --gui"
#endif
                     "and --binary-output are mutually exclusive.\n\n";
        print_help();
        exit(errors_t::BAD_ARGUMENTS);
    }

    if (dot)
        settings.output_mode = output_t::dot;
#ifdef ETA_GUI_ENABLED
    else if (gui)
        settings.output_mode = output_t::gui;
#endif
    else if (grammar)
        settings.output_mode = output_t::grammar;
    else if (binary_output)
        settings.output_mode = output_t::binary;
    else if (expend)
        settings.output_mode = output_t::expend;
    else if (tree)
        settings.output_mode = output_t::tree;
    else if (terminals)
        settings.output_mode = output_t::terminals;
    else
        settings.output_mode = output_t::reduced;

    if (print_input && (binary_output || terminals)) {
        set_color(std::cerr, eta::color_t::red);
        std::cerr << "error: ";
        set_color(std::cerr, eta::color_t::standard);
        std::cerr << "--print-input is not compatible with --binary-output and --terminals.\n\n";
        print_help();
        exit(errors_t::BAD_ARGUMENTS);
    }

    settings.print_input = print_input;

    // print help if needed
    if (help) {
        print_help();
        exit(errors_t::NO_ERROR);
    }

    return settings;
}

