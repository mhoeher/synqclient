#include <SynqClient/DirectorySynchronizer>

int main(int, char**) {
    auto ds = new SynqClient::DirectorySynchronizer();
    delete ds;
}
