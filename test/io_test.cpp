#include "io.hpp"

const int THREAD_SIZE = 2;
int main() {
    std::cout << std::thread::hardware_concurrency() << '\n';
    auto read = [&](std::string file_path, std::vector<std::string> &store) {
        BufferedReader reader("../data/test.data");
        while (reader.has_next()) {
            std::string str = reader.next_str();
            store.push_back(str);
        }
        reader.close();
    };
    std::vector<std::string> paths = {"../data/test.data", "../data/test1.data", "../data/test2.data"};
    std::vector<std::thread> threads(THREAD_SIZE);
    std::vector<std::vector<std::string>> stores(THREAD_SIZE);

    std::atomic_int p(0);
    auto get_job = [&]() {
        return std::atomic_fetch_add_explicit(&p, 1, std::memory_order_relaxed);
    };
    auto run = [&](int index) {
        for (int x; (x = get_job()) < paths.size(); read(paths[x], stores[index]));
    };
    for (uint32_t i = 0; i < THREAD_SIZE; i++)
        threads[i] = std::thread(run, i);
    for (auto &thread : threads)
        thread.join();

    for (auto &v : stores) {
        for (auto &str : v)
            std::cout << str << '\n';
    }
    return 0;
}
