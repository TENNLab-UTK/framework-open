#include "framework.hpp"
#include "utils/json_helpers.hpp"
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <unistd.h>
#include <unordered_set>
#include <vector>

using namespace std;
using namespace neuro;
using nlohmann::json;

Network* load_network(Processor** pp, const json& network_json) {
    Network* net;
    json proc_params;
    string proc_name;
    Processor* p;

    net = new Network();
    net->from_json(network_json);

    p = *pp;
    if (p == nullptr) {
        proc_params = net->get_data("proc_params");
        proc_name = net->get_data("other")["proc_name"];
        p = Processor::make(proc_name, proc_params);
        *pp = p;
    }

    if (p->get_network_properties().as_json() !=
        net->get_properties().as_json()) {
        // throw SRE("network and processor properties do not match.");
    }

    if (!p->load_network(net)) {
        fprintf(stderr, "Failed to load network.\n");
        exit(1);
    }
    // track_all_neuron_events(p, net);

    return net;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "usage: %s network_json activity_denom\n", argv[0]);
        exit(1);
    }
    json network_json;
    vector<string> json_source = {argv[1]};
    int activity_denom = stoi(argv[2]);
    if (activity_denom == 0) {
        activity_denom = INT32_MAX;
    }

    ifstream fin(argv[1]);
    fin >> network_json;

    Processor* p = nullptr;
    Network* n = load_network(&p, network_json);

    std::chrono::duration<double, std::ratio<1>> d =
        std::chrono::duration<double, std::ratio<1>>::zero();

    for (int frames = 0; frames < 6; frames++) {
        chrono::time_point<chrono::steady_clock> tp =
            chrono::steady_clock::now();
        for (int outer = 0; outer < 340; outer++) {
            for (int i = 0; i < n->num_inputs(); i++) {
                if (rand() % activity_denom == 0) {
                    p->apply_spike(Spike(i, 0, 1));
                }
            }

            p->run(1);
            // for (auto a : p->output_counts()) {
            //     printf("%d", a);
            // }
            // putchar('\n');
        }

        d += std::chrono::duration_cast<
            std::chrono::duration<double, std::ratio<1>>>(
            chrono::steady_clock::now() - tp);
    }

    printf("Total frame time:   %7.5f\n", d.count());
    printf("Average frame time: %7.20f\n", d.count() / 6.0);
}
