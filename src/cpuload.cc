#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <chrono>

#include <unistd.h>

namespace zutils {

    using StatType = unsigned long long;

    enum Stats {
        User = 0,
        Nice,
        System,
        Idle,
        Iowait,
        Irq,
        Softirq,
        Steal,
        Guest,
        Guest_nice,
    };

    std::vector<StatType> read_stats() {
        std::ifstream ifs("/proc/stat", std::ifstream::in);

        std::string stat_data, f;

        std::getline(ifs, stat_data, '\n');

        std::istringstream iss(stat_data);

        std::vector<StatType> stats;
        stats.reserve(10);

        // get out "cpu " prefix from stream
        iss >> f;

        // read stats values
        while (!iss.eof()) {
            StatType value = 0;
            iss >> value;
            stats.push_back(value);
        }

        return stats;
    }

    StatType calc_idle(const std::vector<StatType>& stats) {
        return stats[Idle] - stats[Iowait];
    }

    StatType calc_nonidle(const std::vector<StatType>& stats) {
        return stats[User] + stats[Nice] + stats[System] + stats[Irq] + stats[Softirq] + stats[Steal];
    }

    struct CalculatedStats {
        bool empty;
        StatType idle;
        StatType nonidle;
        StatType total;

        CalculatedStats() {
            empty       = true;
        }

        CalculatedStats(const std::vector<StatType>& stats) {
            empty       = false;
            idle        = calc_idle(stats);
            nonidle     = calc_nonidle(stats);
            total       = idle + nonidle;
        }
    };

    double cpu_load(unsigned long tm_ms) {

        static CalculatedStats prev;

        static double result            = 1.0f;
        static auto last_call_time      = std::chrono::steady_clock::now();

        auto current_call_time          = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::milliseconds>(current_call_time - last_call_time) < std::chrono::milliseconds(tm_ms)) {
            return result;
        }

        last_call_time = current_call_time;

        if (prev.empty) {
            prev = CalculatedStats(read_stats());
            return result;
        }

        CalculatedStats curr(read_stats());

        auto total = curr.total - prev.total;
        auto idle = curr.idle - prev.idle;

        prev = curr;

        if (total > 0.0f) {
            result = double(total - idle) / double(total);
        } else {
            result = 1.0f;
        }

        return result;
    }

} // namespace zutils

#if 0
#include <iostream>

int main(int argc, char **argv) {

    std::cout << "cpu load: " << zutils::cpu_load() << std::endl;

    return 0;
}
#endif
