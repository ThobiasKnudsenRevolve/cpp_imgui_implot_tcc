#include "test_tcc.hpp"
#include "test_imgui.hpp"
#include "test_mocking_server.hpp"

int main() {
    libtcc_test();
    test_imgui();
    test_mocking_server();

    return 0;
}