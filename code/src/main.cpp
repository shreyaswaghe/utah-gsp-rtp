#include <RngStream.h>

#include <argparse/argparse.hpp>
#include <highfive/H5Easy.hpp>
#include <vector>

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
    /*Experiment Configs*/

    char* jobid_env;
    long jobid;
    if (NULL == (jobid_env = std::getenv("SLURM_JOB_ID"))) {
        jobid = 0;
    } else {
        jobid = std::stol(jobid_env);
    }

    std::string experiment_name;
    char* experiment_name_env;
    if (NULL == (experiment_name_env = std::getenv("EXPERIMENT_NAME"))) {
        experiment_name = "TEST";
    } else {
        experiment_name = experiment_name_env;
    }

    std::string result_dir{"results/"+experiment_name};
    std::cout << jobid << std::endl;

    int num_simulations = 5000;
    double L = 1.0;
    double v = 0.01;
    double init_pos = 0.2;
    double reset_pos = 0.2;

    double theta = 1.0;
    double r = 1.0;

    argparse::ArgumentParser parser("rtp_sim");

    parser.add_argument("rate_reset")
        .required()
        .help("The rate of resetting to reset_pos.")
        .store_into(r);

    parser.add_argument("rate_switch")
        .required()
        .help("The rate of switching directions.")
        .store_into(theta);

    parser.add_argument("L")
        .help("Size of the interval")
        .default_value(1.0)
        .store_into(L);

    parser.add_argument("v")
        .help("Particle velocity")
        .default_value(0.01)
        .store_into(v);

    parser.add_argument("num_simulations")
        .help("The number of realizations to run.")
        .default_value(5000)
        .store_into(num_simulations);

    parser.add_argument("init_pos")
        .help(
            "The initialization position within the interval. Defaults to "
            "0.5L.")
        .default_value(-1.0)
        .store_into(init_pos);

    parser.add_argument("reset_pos")
        .help("The reseting position within the interval. Defaults to 0.5L.")
        .default_value(-1.0)
        .store_into(reset_pos);

    try {
        parser.parse_args(argc, argv);
    } catch (const std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        std::cerr << parser << std::endl;
        std::exit(1);
    }

    init_pos = (init_pos < 0) ? 0.5 * L : init_pos;
    reset_pos = (reset_pos < 0) ? 0.5 * L : reset_pos;

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
    double time = 0.0;
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

    H5Easy::File writefile(result_dir + "/" + std::to_string(jobid) + ".h5",
                           H5Easy::File::Create);

    H5Easy::dump(writefile, "rawdata/interval_exit_time", exit_time);
    H5Easy::dump(writefile, "rawdata/end_hit", end_hit);
    H5Easy::dump(writefile, "rawdata/first_passage_time", fp_time);

    H5Easy::dump(writefile, "metadata/reset_rate", r);
    H5Easy::dump(writefile, "metadata/switch_rate", theta);
    H5Easy::dump(writefile, "metadata/L", L);
    H5Easy::dump(writefile, "metadata/v", v);
    H5Easy::dump(writefile, "metadata/init_pos", init_pos);
    H5Easy::dump(writefile, "metadata/reset_pos", reset_pos);
    H5Easy::dump(writefile, "metadata/num_simulations", num_simulations);
}
