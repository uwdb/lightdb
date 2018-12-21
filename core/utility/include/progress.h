#ifndef LIGHTDB_PROGRESS_H
#define LIGHTDB_PROGRESS_H

#include <string>

class tqdm;

namespace lightdb {

class Progress {
public:
    explicit Progress(int total = 1, const std::string &label = "");

    ~Progress();

    void label(const std::string &label);
    void progress(int current, int total);
    void total(int total);
    void reset();
    void finish();

    Progress& operator++();
    Progress& operator++(int);

private:
    std::unique_ptr<tqdm> bar_;
    int current_;
    int total_;
};

} // namespace lightdb

#endif //LIGHTDB_PROGRESS_H
