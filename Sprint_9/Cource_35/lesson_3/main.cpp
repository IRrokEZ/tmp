#include <cstdint>
#include <iostream>
#include <limits>

using namespace std;

int main() {
    int64_t a;
    int64_t b;
    cin >> a >> b;
    if ((b > 0) && (a > (std::numeric_limits<int64_t>::max() - b))) {
        std::cout << "Overflow!";
        return 0;
    }
    if ((b < 0) && (a < (std::numeric_limits<int64_t>::min() - b))) {
        std::cout << "Overflow!";
        return 0;
    }
    std::cout << a + b << endl;
}