
#include <iostream>
#include <cstring>
#include <utility>
#include <stdexcept>
#include <cmath>
#include <cstdlib>
#include <boost/program_options.hpp>
#include "logical_expr.hpp"
#include "quine_mccluskey.hpp"
#include <fstream>
#include <string>

using namespace std;

unsigned int start_s=clock();
// the code you wish to time goes here

template<typename Property>
void print_term_expr(const logical_expr::logical_term<Property> &term, 
            char first_char = 'A', char inverter = '~')
{
    for( int i = 0; i < term.size(); ++i ) {
        if( term[i] == false )  cout << inverter;
        if( term[i] != logical_expr::dont_care )
            cout << static_cast<char>(first_char + i);
    }
}

template<typename TermType>
void print_func_expr(
        const logical_expr::logical_function<TermType> &func,
        char first_char = 'A', const string &funcname = "f", char inverter = '~')
{
    cout << funcname << " = ";
    for( auto it = func.begin(); it != func.end(); ++it ) {
        print_term_expr(*it, first_char, inverter);
        if( it + 1 != func.end() )
            cout << " + ";
    }
    cout << endl;
}

template<typename TermType>
void print_truth_table(
        const logical_expr::logical_function<TermType> &f, 
        char first_char = 'A', const string &funcname ="f" 
    )
{
    cout << "Truth Table: ";
    print_func_expr(f, first_char, funcname);
    for( char c = first_char; c != first_char + f.term_size(); ++c )
        cout << c;
    cout << " | " << funcname << "()" << endl;
    for( int i = 0; i < f.term_size() + 6; ++i )
        cout << ((i == f.term_size() + 1) ? '|' : '-');
    cout << endl;
    logical_expr::arg_generator<> generator(0, std::pow(2, f.term_size()), f.term_size());
    for( auto arg : generator )
        cout << arg << " |  " << f(arg) << endl;
}

int main(int argc, char **argv)
{
     ifstream infile;
    int exit_code = EXIT_SUCCESS;
    try {
        bool print_process = true;
        char first_char = 'A';
        constexpr char inverter = '~';

        //
        // Parse command line options
        //
        using namespace boost::program_options;
        options_description opt("Options");
        opt.add_options()
            ("quiet,q", "never print the information of the process of simplifying")
            ("first-char,c", value<char>(), "specify a character of the first variable used for input expression")
            ("help,h", "display this help and exit");
        variables_map argmap;
        store(parse_command_line(argc, argv, opt), argmap);
        notify(argmap);
        if( argmap.count("help") ) {
            std::cout << opt << endl;
            return EXIT_SUCCESS;
        }
        if( argmap.count("quiet") )
            print_process = false;
        if( argmap.count("first-char") )
            first_char = argmap["first-char"].as<char>();

        // Input a target logical function to be simplfied from stdin
        if( print_process )
            cout << "This is the Quine-McCluskey simplifier"   << endl
                 << "Enter a logical function to be simplified"   << endl
                 << "    (ex. \"f(A, B, C) = A + BC + ~A~B + ABC\" )" << endl
                 << "[*] Input: " << flush;
        string line;
       
        infile.open ("/Users/suchethapanduranga/Downloads/Quine-McCluskey-master/sample/in4.txt");
        
            getline(infile,line); // Saves the line in line.
            cout<<line; // Prints our STRING.
        
        infile.close();
     
        //getline(cin, line);

        // Parse input logical expression and return tokenized
        logical_expr::function_parser<inverter, true> parser(line, first_char);
        auto token = parser.parse();
        // Create a logical function with logical_term<term_mark>
        typedef quine_mccluskey::simplifier::property_type PropertyType;
        typedef quine_mccluskey::simplifier::term_type TermType;
        logical_expr::logical_function<TermType> function;
        for( string term : token.second )
            function += logical_expr::parse_logical_term<PropertyType, inverter>(term, token.first.size(), first_char);

        // Create a simplifier using Quine-McCluskey algorithm
        quine_mccluskey::simplifier qm(function);
        if( print_process ) {
            cout << endl << "Sum of products form:" << endl;
            print_truth_table(qm.get_std_spf(), first_char);    // Print the function in sum of products form
            cout << endl << "Compressing ..." << endl;
            qm.compress_table(true);                            // Compress the compression table
            cout << endl << "Prime implicants: " << endl;
            for( const auto &term : qm.get_prime_implicants() ) {      // Print the prime implicants
                print_term_expr(term, first_char);
                cout << "  ";
            }
            cout << endl << endl << "Result of simplifying:" << endl;
        }
        else
            qm.compress_table(false);

        for( const auto &func : qm.simplify() )        // Simplify and print its results
            print_func_expr(func, first_char, parser.function_name() + "\'");
        unsigned int stop_s=clock();
        cout << "Run time of this operation: " << (stop_s - start_s)/double(CLOCKS_PER_SEC)*1000 << "seconds"<< endl;
        
    }
    
    catch( std::exception &e ) {
        cerr << endl << "[-] Exception: " << e.what() << endl;
        exit_code = EXIT_FAILURE;
    }
    return exit_code;
}

