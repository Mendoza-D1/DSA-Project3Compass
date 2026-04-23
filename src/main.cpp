#include <iostream>

#include "CampusCompass.h"

using namespace std;

int main() {
    // initialize your main project object
    CampusCompass compass;

    // ingest CSV data
    if (!compass.ParseCSV("data/edges.csv", "data/classes.csv")) {
        return 1;
    }

    // the below is example code for parsing commandline input
    int no_of_lines;
    string command;
    cin >> no_of_lines;
    cin.ignore(); // ignore newline that first cin left over

    for (int i = 0; i < no_of_lines; i++) {
        getline(cin, command);

        // parse your commands however you see fit
        compass.ParseCommand(command);
    }

    return 0;
}