#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <cmath>
#include <memory>

// Test to see if there's a namespace conflict
namespace MusicTrainer {
namespace music {
namespace rules {

// Test class
class TestClass {
public:
    void test() {
        // Test if std:: is properly resolved
        std::string test_string = "Hello, world!";
        std::cout << "Test string: " << test_string << std::endl;
    }
};

} // namespace rules
} // namespace music
} // namespace MusicTrainer

// Test program to diagnose namespace issues
int main() {
    // Test standard library functionality outside of any namespace
    ::std::cout << "Testing standard library outside namespace..." << ::std::endl;
    
    // Test vector
    ::std::vector<int> v = {1, 2, 3};
    ::std::cout << "Vector size: " << v.size() << ::std::endl;
    
    // Test stringstream
    ::std::stringstream ss;
    ss << "Test string";
    ::std::cout << "Stringstream: " << ss.str() << ::std::endl;
    
    // Test math functions
    double val = ::std::abs(-5.5);
    ::std::cout << "Abs value: " << val << ::std::endl;
    
    return 0;
}