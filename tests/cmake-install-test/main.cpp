#include <SynqClient/DirectorySynchronizer>

int main(int, char**) {
    auto *sync = new SynqClient::DirectorySynchronizer();
    delete sync;
    return 0;
}
