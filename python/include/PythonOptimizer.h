#ifndef LIGHTDB_PYTHON_OPTIMIZER_H
#define LIGHTDB_PYTHON_OPTIMIZER_H

#include <boost/python.hpp>
#include "Optimizer.h"
#include "HeuristicOptimizer.h"

namespace lightdb::python {
    class OptimizerWrapper : public lightdb::optimization::Optimizer, public boost::python::wrapper<lightdb::optimization::Optimizer> {
        protected:
            const std::vector<std::shared_ptr<lightdb::optimization::OptimizerRule>> rules() const {
                return this->get_override("rules")();
            }
    };
}

#endif //LIGHTDB_PYTHON_OPTIMIZER_H