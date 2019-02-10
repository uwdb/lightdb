#ifndef LIGHTDB_HEURISTICOPTIMIZER_H
#define LIGHTDB_HEURISTICOPTIMIZER_H

#include "Optimizer.h"
#include "Rules.h"

namespace lightdb::optimization {

    class HeuristicOptimizer : public Optimizer {
    public:
        using Optimizer::Optimizer;

    protected:
        const rule_vector rules() const override {
            return { make_rule<ChooseMaterializedScans>(),
                     make_rule<ChooseDecoders>(),
                     make_rule<ChooseUnion>(),
                     make_rule<ChooseSelection>(),
                     make_rule<ChooseInterpolate>(),
                     make_rule<ChooseDiscretize>(),
                     make_rule<ChooseLinearScale>(),
                     make_rule<ChooseMap>(),
                     make_rule<ChoosePartition>(),
                     make_rule<ChooseSubquery>(),
                     make_rule<ChooseEncoders>(),
                     make_rule<ChooseStore>(),
                     make_rule<ChooseSink>()};
        }
    };
}

#endif //LIGHTDB_HEURISTICOPTIMIZER_H
