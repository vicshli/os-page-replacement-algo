#pragma once

namespace pager
{
class Pager;
}

namespace memref
{

class Reference
{
public:
    Reference(int pid, int delta);
    virtual ~Reference();
    Reference &operator=(Reference &rhs);
    virtual int simulate(int prior_refnum, int proc_size, pager::Pager &pager, int time_accessed);

protected:
    int pid_;
    int delta_;
    static const int INIT_FACTOR_;
};
} // namespace memref