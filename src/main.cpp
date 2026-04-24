#include <iostream>
#include <boost/filesystem.hpp>

int main() {
    boost::filesystem::path p = boost::filesystem::current_path();

    std::cout << "current path: " << p << "\n";

    for (auto& entry : boost::filesystem::directory_iterator(p)) {
        std::cout << entry.path().filename() << "\n";
    }

    return 0;
}
