#include "UI.hpp"

int main() {
    std::cout << "***Project Kendinsky***\n";
    System sym;
    // sym.init("../pre_data/aes_core.hierarchy.v.table/aes_core.table");
    // sym.init("../pre_data/lut_s44.hierarchy.v.table/$paramod\\lut\\INPUTS=s32'00000000000000000000000000000100.table");
    // sym.init_unpre("../data/aes_core.v.table");
    // sym.write_networkX("../output_data/test.o");
    sym.load("../pre_data/aes_core.hierarchy.v.table/aes_core");
    sym.store("../output_data");
    sym.load("../output_data");
    sym.store("../output_data");
    std::cout << "test finished\n";
    return 0;
}