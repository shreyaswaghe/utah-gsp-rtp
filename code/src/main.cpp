#include <RngStream.h>
#include <vector>

#include <highfive/H5Easy.hpp>

void half_prob(RngStream& rng, std::vector<double>& out) {
    for (int i = 0; i < 1000; i++) {
        out[i] = (rng.RandU01());
    }
}

void exp_prob(double& theta, RngStream& rng, std::vector<double>& out) {
    for (int i = 0; i < 1000; i++) {
        out[i] = (-theta * std::log(rng.RandU01()));
    }
}

int main(int argc, char* argv[]) {
    std::string jobid = argv[1];

    int num_simulations = 5;

    double theta = 1.0;
    double r = 1.0;
    double time = 0.0;

    double L = 1.0;
    double v = 0.01;
    double init_pos = 0.2;
    double reset_pos = 0.2;

    std::vector<double> rand_switch;
    std::vector<double> rand_reset;
    std::vector<double> rand_dir;

    rand_dir.reserve(1000);
    rand_reset.reserve(1000);
    rand_switch.reserve(1000);

    std::vector<double> exit_time;
    std::vector<double> end_hit;
    std::vector<double> fp_time;

    // exit time
    double x = init_pos;
    int r_idx = 0;
    int s_idx = 0;
    int p_idx = 0;
    double dt = 0.0;
    double p = 0.0;
    double next_reset, next_switch;


    RngStream rng1("");
    RngStream rng2("");
    RngStream rng3("");

    half_prob(rng1, rand_dir);
    exp_prob(r, rng2, rand_reset);
    exp_prob(theta, rng3, rand_switch);

    for (int sim_count = 0; sim_count < num_simulations; sim_count++) {
        std::cout << "Sim count: " << sim_count << std::endl;

        while (true) {
            if ((next_reset = rand_reset[r_idx++]) <
                (next_switch = rand_switch[s_idx++])) {  // reset
                dt = next_reset;
                x = reset_pos;

                p = rand_dir[p_idx++];
                v = ((p > 0.5) - (p < 0.5)) * std::abs(v);

                s_idx--;

            } else {  // direction switch
                dt = next_switch;
                x += v * dt;

                v = -v;
                r_idx--;
            }

            time += dt;

            if (r_idx == 999) {
                std::cout << "Refilling reset" << std::endl;
                exp_prob(r, rng2, rand_reset);
                r_idx = 0;
            }
            if (s_idx == 999) {
                std::cout << "Refilling switch" << std::endl;
                exp_prob(theta, rng3, rand_switch);
                s_idx = 0;
            }
            if (p_idx == 999) {
                std::cout << "Refilling p" << std::endl;
                half_prob(rng1, rand_dir);
                p_idx = 0;
            }

            if (x < 0 || x > L) {
                break;
            }
        }

        double hit = (x > L) ? L : 0;

        exit_time.push_back(time);
        end_hit.push_back(hit);

        // p = exp(-(k+r).*abs(length)./v)./2
        x = hit;
        time = 0.0;
        while (true) {
            if ((next_reset = rand_reset[r_idx++]) <
                (next_switch = rand_switch[s_idx++])) {  // reset
                dt = next_reset;
                x = reset_pos;

                p = rand_dir[p_idx++];
                v = ((p > 0.5) - (p < 0.5)) * std::abs(v);

                s_idx--;

            } else {  // direction switch
                dt = next_switch;
                x += v * dt;

                v = -v;
                r_idx--;
            }

            time += dt;

            if (r_idx == 999) {
                exp_prob(r, rng2, rand_reset);
                r_idx = 0;
            }
            if (s_idx == 999) {
                exp_prob(theta, rng3, rand_switch);
                s_idx = 0;
            }
            if (p_idx == 999) {
                half_prob(rng1, rand_dir);
                p_idx = 0;
            }

            if (hit && x < 0) {
                break;
            } else if (!hit && x > L) {
                break;
            }
        }
        fp_time.push_back(time);
    }
}


