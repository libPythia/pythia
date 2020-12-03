#include "settings.hpp"

#include <ProgramOptions.hxx>
#include <iostream>
#include <rang.hpp>

#include "colors.hpp"
#include "errors.hpp"

static auto print_option(char abbrev, char const * option, char const * descr) -> void {
    set_color(std::cerr, color_t::green);
    std::cerr << "    ";
    if (abbrev != 0) {
        std::cerr << '-' << abbrev;
        set_color(std::cerr, color_t::standard);
        std::cerr << ", ";
    } else
        std::cerr << "    ";
    set_color(std::cerr, color_t::green);
    std::cerr << "--" << option << '\n';
    set_color(std::cerr, color_t::standard);
    std::cerr << "            " << descr << '\n';
}

static auto print_section(char const * section) -> void {
    set_color(std::cerr, color_t::yellow);
    std::cerr << section << '\n';
    set_color(std::cerr, color_t::standard);
}

static auto print_subsection(char const * subsection) -> void {
    set_color(std::cerr, color_t::standard);
    std::cerr << "  " << subsection << '\n';
}

static auto print_error(int err_code, char const * name, char const * descr) -> void {
    set_color(std::cerr, color_t::green);
    std::cerr << "    " << err_code;
    set_color(std::cerr, color_t::standard);
    std::cerr << ", ";
    set_color(std::cerr, color_t::green);
    std::cerr << name;
    set_color(std::cerr, color_t::standard);
    std::cerr << "\n            " << descr << '\n';
}
#define PRINT_ERROR(error, descr) print_error(static_cast<int>(errors_t::error), #error, descr)

static auto print_help() -> void {
    set_color(std::cerr, color_t::green);
    std::cerr << "eta";
    set_color(std::cerr, color_t::standard);
    std::cerr << " 0.2\n"
                 "A program implementing TRedA reduction algorithm.\n"
                 "It reads its inputs on stdin and write output on stdout.\n\n";

    print_section("USAGE");
    set_color(std::cerr, color_t::standard);
    std::cerr << "    eta [OPTIONS]...\n\n";

    print_section("OPTIONS");
    print_subsection("General options");
    print_option('h', "help", "Show this help.");
    print_option('D', "debug", "Print debug informations.");
    print_option('c', "check", "Enable integrity and validity checks.");
    print_subsection("Input options");
    print_option('n', "non-printable", "Don't ignore non-printable characters.");
    print_option('l', "lines", "Take each line of input as a different input to reduce");
    print_option('b', "binary-input", "Expect input to be in binary format.");
    print_subsection("Output options");
    print_option('i', "print-input", "Print input before.");
    print_option('d', "dot", "Produce output under dot format.");
    print_option('g', "grammar", "Format output as a grammar.");
    print_option('B', "binary-output", "Use binary format for output");
    print_option(0, "no-color", "Don't use color and formating in output.");

    std::cerr << '\n';
    print_section("ERRORS");
    PRINT_ERROR(BAD_ARGUMENTS, "Incompatibily between arguments given by the user.");
    PRINT_ERROR(INPUT_OUTPUT_MISMATCH, "Too many inputs for output mode.");
    PRINT_ERROR(CHECK_FAILED, "Data structure checks failed. Report this as a bug.");
    PRINT_ERROR(NOT_IMPLEMENTED_FEATURE, "The requested feature is not implemented yet.");
}

auto parse_settings(int argc, char ** argv) -> settings_t {
    auto parser = po::parser {};
    auto settings = settings_t {};

    // general
    auto & debug_opt = parser["debug"].abbreviation('D');
    auto & help_opt = parser["help"].abbreviation('h');
    auto & check_opt = parser["check"].abbreviation('c');

    // input
    auto & non_printable_opt = parser["non-printable"].abbreviation('n');
    auto & line_opt = parser["lines"].abbreviation('l');
    auto & binary_input_opt = parser["binary-input"].abbreviation('b');

    // output
    auto & print_input_opt = parser["print-input"].abbreviation('i');
    auto & dot_opt = parser["dot"].abbreviation('d');
    auto & grammar_opt = parser["grammar"].abbreviation('g');
    auto & binary_output_opt = parser["binary-output"].abbreviation('B');
    auto & no_color_opt = parser["no-color"];

    if (!parser(argc, argv)) {
        exit(errors_t::BAD_ARGUMENTS);
    }

    // general
    auto const debug = debug_opt.was_set();
    auto const help = help_opt.was_set();
    auto const check = check_opt.was_set();
    auto const no_color = no_color_opt.was_set();

    // input
    auto const non_printable = non_printable_opt.was_set();
    auto const lines = line_opt.was_set();
    auto const binary_input = binary_input_opt.was_set();

    // output
    auto const print_input = print_input_opt.was_set();
    auto const dot = dot_opt.was_set();
    auto const grammar = grammar_opt.was_set();
    auto const binary_output = binary_output_opt.was_set();

    // general settings
    settings.debug = debug;
    settings.check = check;
    if (no_color)
        rang::setControlMode(rang::control::Off);
    else
        rang::setControlMode(rang::control::Auto);

    // input settings
    if ((non_printable && lines) || (lines && binary_input) || (non_printable && binary_input)) {
        set_color(std::cerr, color_t::red);
        std::cerr << "error: ";
        set_color(std::cerr, color_t::standard);
        std::cerr << "--non-printable, --lines and --binary-input are mutually exclusive.\n\n";
        print_help();
        exit(errors_t::BAD_ARGUMENTS);
    }
    if (binary_input)
        settings.input_mode = input_t::binary;
    else if (non_printable)
        settings.input_mode = input_t::non_printable;
    else if (lines)
        settings.input_mode = input_t::lines;
    else
        settings.input_mode = input_t::text;

    // output settings

    if ((dot && grammar) || (grammar && binary_output) || (dot && binary_output)) {
        set_color(std::cerr, color_t::red);
        std::cerr << "error: ";
        set_color(std::cerr, color_t::standard);
        std::cerr << "--dot, --grammar and --binary-output are mutually exclusive.\n\n";
        print_help();
        exit(errors_t::BAD_ARGUMENTS);
    }

    if (dot)
        settings.output_mode = output_t::dot;
    else if (grammar)
        settings.output_mode = output_t::grammar;
    else if (binary_output)
        settings.output_mode = output_t::binary;
    else
        settings.output_mode = output_t::reduced;

    if (print_input && binary_output) {
        set_color(std::cerr, color_t::red);
        std::cerr << "error: ";
        set_color(std::cerr, color_t::standard);
        std::cerr << "--print-input is not compatible with --binary-output.\n\n";
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
