#include <iostream>

#include "jobmix.h"
#include "../io/randintreader.h"
#include "../memref/memref.h"
#include "../memref/memref_back.h"
#include "../memref/memref_init.h"
#include "../memref/memref_jmp.h"
#include "../memref/memref_rand.h"
#include "../memref/memref_seq.h"

namespace driver
{
JobMix::JobMix(int id, int proc_count, bool is_uniform, std::vector<JobMixPerProcess> jobmixes)
    : ID_(id), PROC_COUNT_(proc_count), IS_UNIFORM_(is_uniform), JOBMIXES_(jobmixes)
{
    auto calc_rand_ref_dist = [](double a, double b, double c, int s) -> double { return (1 - a - b - c) / (double)s; };

    auto calc_seq_ref_threshold = [](double a) -> double { return a; };
    auto calc_back_ref_threshold = [](double seqthres, double b) -> double { return seqthres + b; };
    auto calc_jmp_ref_threshold = [](double backthres, double c) -> double { return backthres + c; };

    for (auto &jobmix : JOBMIXES_)
    {
        jobmix.rand_ref_dist = calc_rand_ref_dist(jobmix.sequential_ref_dist,
                                                  jobmix.backward_ref_dist,
                                                  jobmix.jump_ref_dist,
                                                  PROC_COUNT_);

        jobmix.sequential_ref_threshold = calc_seq_ref_threshold(jobmix.sequential_ref_dist);
        jobmix.backward_ref_threshold = calc_back_ref_threshold(jobmix.sequential_ref_threshold,
                                                                jobmix.backward_ref_dist);
        jobmix.jump_ref_threshold = calc_jmp_ref_threshold(jobmix.backward_ref_threshold,
                                                           jobmix.jump_ref_dist);
    }
}

int JobMix::process_count() const
{
    return PROC_COUNT_;
}

std::shared_ptr<memref::Reference> JobMix::next_ref_type(double quotient, int pid, io::RandIntReader &randintreader) const
{
    int access_idx = IS_UNIFORM_ ? 0 : (pid - 1);

    if (0.0 <= quotient && quotient <= JOBMIXES_[access_idx].sequential_ref_threshold)
    {
        return std::unique_ptr<memref::Reference>{new memref::SequentialReference(pid)};
    }
    else if (quotient <= JOBMIXES_[access_idx].backward_ref_threshold)
    {
        return std::unique_ptr<memref::Reference>{new memref::BackwardReference(pid)};

    }
    else if (quotient <= JOBMIXES_[access_idx].jump_ref_threshold)
    {
        return std::unique_ptr<memref::Reference>{new memref::JumpReference(pid)};
    }
    else
    {
        int randnum = randintreader.read_next_int();
        return std::unique_ptr<memref::Reference>{new memref::RandomReference(pid, randnum)};
    }
}

void JobMix::print() const
{
    std::cout << "JobMix " << ID_ << ":\n"
              << std::endl;

    for (auto &jobmix : JOBMIXES_)
    {
        std::cout << "JobMix sequential ref dist (A): " << jobmix.sequential_ref_dist
                  << "\tthreshold: " << jobmix.sequential_ref_threshold << "\n"
                  << "JobMix backward ref dist (B): " << jobmix.backward_ref_dist
                  << "\tthreshold: " << jobmix.backward_ref_threshold << "\n"
                  << "JobMix jump ref dist (C): " << jobmix.jump_ref_dist
                  << "\tthreshold: " << jobmix.jump_ref_threshold << "\n"
                  << "JobMix rand ref dist (D): " << jobmix.rand_ref_dist << "\n"
                  << std::endl;
    }
}
} // namespace driver