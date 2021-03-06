// CS460 -- HW2
// Amandeep Gill
// read, parse, sort, and display a list of fraction in proper reduced form
// written in "idiomatic" c++

#include <iostream>
#include <fstream>
#include <set>
#include "Fraction.h"

typedef struct {
    Fraction *f;
} FWrap;

// overloaded comparison operators
bool operator<(const FWrap &lhs, const FWrap &rhs) { 
    return *lhs.f < *rhs.f; 
}

/* main function */
int main(int argc, const char* argv[]) {
    std::multiset<FWrap> fs;
    Fraction sum;
    
    std::ifstream input(argv[1]);
    while (input.good()) {
        Fraction in;
        if (input >> in) {
            FWrap fw;
            fw.f = new Fraction(in);
            sum += *fw.f;
            fs.insert(fw);
        }
        else if (input.eof()) {
            break;
        }
        else {
            return -1;
        }
    }

    std::cout << "The sum of the fractions is: " << sum << std::endl;

    for (auto fw : fs) {
        std::cout << *fw.f << std::endl;
    }

    return 0;
}
