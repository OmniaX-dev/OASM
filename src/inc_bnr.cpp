#include <iostream>
#include <fstream>

using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::ofstream;
using std::ifstream;

int main(void)
{
    string filename("build.nr");
    long int number;

    ifstream input_file(filename);
    if (!input_file.is_open())
	{
        cerr << "Could not open the input file - '" << filename << "'" << endl;
        return EXIT_FAILURE;
    }

	if (input_file >> number)
	{
		number++;
	}
    input_file.close();

	ofstream output_file(filename, ofstream::out | ofstream::trunc);
    if (!output_file.is_open())
	{
        cerr << "Could not open the output file - '" << filename << "'" << endl;
        return EXIT_FAILURE;
    }
	output_file << number;

	output_file.close();
    return EXIT_SUCCESS;
}