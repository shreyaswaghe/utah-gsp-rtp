#include <RngStream.h>

#include <argparse/argparse.hpp>
#include <highfive/H5Easy.hpp>
#include <vector>

#include <semaphore.h>
#include <fcntl.h>

#include "progressbar.hpp"

class Particle
{
    public:
        Particle(double pos, double vel): position(pos), velocity(vel)
        {
            new_velocity();
        }

        void move(double dt)
        {
            position += velocity * dt;
        }

        void switch_direction(double dt)
        {
            move(dt);
            velocity *= -1;
        }

        void reset_to(double pos, double dt)
        {
            move(dt);
            position = pos;
            new_velocity();
        }

        void new_velocity()
        {
            velocity = ((rng_vel.RandU01() < 0.5)? -1 : 1) * std::abs(velocity);
        }

        double get_position()
        {
            return position;
        }
    
    private:
        double position;
        double velocity;
        RngStream rng_vel = RngStream();
};


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

double interval_exit(double r, double s, double init_pos, double reset_pos, double v, double L, double& end_hit)
{
    std::vector<double> rand_switch;
    std::vector<double> rand_reset;

    rand_reset.reserve(1000);
    rand_switch.reserve(1000);

    RngStream rng1("");
    RngStream rng2("");

    exp_prob(r, rng1, rand_reset);
    exp_prob(s, rng2, rand_switch);
   
    int switch_index = 0;
    int reset_index = 0;
    double dt = 0.0;
    double next_reset, next_switch;
    double time;
    time = 0.0;

    Particle p(init_pos, v);
    while (p.get_position() > 0 && p.get_position() < L)
    {
            if((next_reset = rand_reset[reset_index]) < (next_switch = rand_switch[switch_index]))
            {
                // resetting
                dt = next_reset;
                p.reset_to(reset_pos, dt);
                reset_index++;
                if(reset_index >= 999)
                {
                    exp_prob(r, rng1, rand_reset);
                    reset_index = 0;
                }
            }
            else {
                // switching
                dt = next_switch;
                p.switch_direction(dt);
                switch_index++;
                if(switch_index >= 999){
                    exp_prob(s, rng2, rand_switch);
                    switch_index=0;
                }
            }
            time += dt;
    }

    end_hit = p.get_position();
    return time;
}


double fpt_from_0(double r, double s, double reset_pos, double v,double L)
{
    std::vector<double> rand_switch;
    std::vector<double> rand_reset;

    rand_reset.reserve(1000);
    rand_switch.reserve(1000);

    RngStream rng1("");
    RngStream rng2("");

    exp_prob(r, rng1, rand_reset);
    exp_prob(s, rng2, rand_switch);
   
    int switch_index = 0;
    int reset_index = 0;
    double dt = 0.0;
    double next_reset, next_switch;
    double time;
    time = 0.0;

    Particle p(0, v);
    while (p.get_position() < L)
    {
            if(((next_reset = rand_reset[reset_index])) < (next_switch = rand_switch[switch_index]))
            {
                // resetting
                dt = next_reset;
                p.reset_to(reset_pos, dt);
                reset_index++;
                if(reset_index >= 999)
                {
                    exp_prob(r, rng1, rand_reset);
                    reset_index = 0;
                }
            }
            else {
                // switching
                dt = next_switch;
                p.switch_direction(dt);
                switch_index++;
                if(switch_index >= 999){
                    exp_prob(s, rng2, rand_switch);
                    switch_index = 0;
                }
            }
            time += dt;
    }

    return time;
}

double fpt_from_L(double r, double s, double reset_pos, double v, double L)
{
    std::vector<double> rand_switch;
    std::vector<double> rand_reset;

    rand_reset.reserve(1000);
    rand_switch.reserve(1000);

    RngStream rng1("");
    RngStream rng2("");

    exp_prob(r, rng1, rand_reset);
    exp_prob(s, rng2, rand_switch);
   
    int switch_index = 0;
    int reset_index = 0;
    double dt = 0.0;
    double next_reset, next_switch;
    double time;
    time = 0.0;

    Particle p(L, v);
    while (p.get_position() > 0)
    {
            if(((next_reset = rand_reset[reset_index])) < (next_switch = rand_switch[switch_index]))
            {
                // resetting
                dt = next_reset;
                p.reset_to(reset_pos, dt);
                reset_index++;
                if(reset_index >= 999)
                {
                    exp_prob(r, rng1, rand_reset);
                    reset_index = 0;
                }
            }
            else {
                // switching
                dt = next_switch;
                p.switch_direction(dt);
                switch_index++;
                if(switch_index >= 999){
                    exp_prob(s, rng2, rand_switch);
                    switch_index = 0;
                }
            }
            time += dt;
    }

    return time;
}

int main(int argc, char* argv[]) {
    /*Experiment Configs*/

    char* jobid_env;
    unsigned long jobid;
    if (NULL == (jobid_env = std::getenv("SLURM_JOB_ID"))) {
        jobid = 10101;
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

    parser.add_argument("--L")
        .help("Size of the interval")
        .default_value(1.0)
        .store_into(L);

    parser.add_argument("--v")
        .help("Particle velocity")
        .default_value(0.01)
        .store_into(v);

    parser.add_argument("--num_simulations")
        .help("The number of realizations to run.")
        .default_value(5000)
        .store_into(num_simulations);

    parser.add_argument("--init_pos")
        .help(
            "The initialization position within the interval. Defaults to "
            "0.5L.")
        .default_value(-1.0)
        .store_into(init_pos);

    parser.add_argument("--reset_pos")
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

    std::vector<double> exit_time;
    std::vector<double> end_hit;
    std::vector<double> fp_time;

    exit_time.reserve(num_simulations);
    end_hit.reserve(num_simulations);
    fp_time.reserve(num_simulations);

    unsigned long seed[6] {jobid, jobid, jobid, jobid, jobid, jobid};
    RngStream::SetPackageSeed(seed);

    RngStream rng1("");
    RngStream rng2("");
        
    for(int sim_count = 0;sim_count < num_simulations; sim_count++)
    {
        double end;
        exit_time.push_back(interval_exit(r, theta,init_pos, reset_pos, v, L, end));
        if (end >= L)
        {
            end_hit.push_back(L);
            fp_time.push_back(fpt_from_L(r, theta, reset_pos, v, L));
        }
        else 
        {
            end_hit.push_back(0);
            fp_time.push_back(fpt_from_0(r, theta, reset_pos, v, L));
        }

    }


    std::cout << "finished sims" << std::endl;
    std::cout << std::flush;
    
    {
        std::cout << "Beginning writing" << std::endl << std::flush;
        H5Easy::File writefile(result_dir + "/" + std::to_string(jobid) + ".h5",
                               H5Easy::File::OpenOrCreate);

        H5Easy::DumpOptions dump_opt;
        dump_opt.set(H5Easy::Flush::True);
        dump_opt.set(H5Easy::Compression());
        dump_opt.set(H5Easy::DumpMode::Overwrite);

        H5Easy::dump(writefile, "rawdata/interval_exit_time", exit_time);
        H5Easy::dump(writefile, "rawdata/end_hit", end_hit);
        H5Easy::dump(writefile, "rawdata/first_passage_time", fp_time);

        H5Easy::dump(writefile, "metadata/reset_rate", r);
        H5Easy::dump(writefile, "metadata/switch_rate", theta);
        H5Easy::dump(writefile, "metadata/L", L);
        H5Easy::dump(writefile, "metadata/v", std::abs(v));
        H5Easy::dump(writefile, "metadata/init_pos", init_pos);
        H5Easy::dump(writefile, "metadata/reset_pos", reset_pos);
        H5Easy::dump(writefile, "metadata/num_simulations", num_simulations);

        writefile.flush();
    }
}
