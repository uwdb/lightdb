#ifndef LIGHTDB_HEURISTICOPTIMIZER_H
#define LIGHTDB_HEURISTICOPTIMIZER_H

#include "Optimizer.h"
#include "Rules.h"

namespace lightdb::optimization {

    class HeuristicOptimizer : public Optimizer {
    public:
        using Optimizer::Optimizer;

    protected:
        const rule_vector rules() override {
            return { make_rule<ChooseDecoders>(),
                     make_rule<ChooseUnion>(),
                     make_rule<ChooseEncoders>() }; }
    };

}

#endif //LIGHTDB_HEURISTICOPTIMIZER_H
