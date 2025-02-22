#ifndef REACHABLE_GOALSUBSETS_CEGAR_PRUNING_H
#define REACHABLE_GOALSUBSETS_CEGAR_PRUNING_H


#include "msgs_collection.h"
#include "../goal_subsets/goal_subset.h"
#include "../../potentials/potential_goals_heuristic.h"
#include "../../heuristic.h"
#include "msgs_evaluation_context.h"

#include <memory>

namespace new_goal_subset_heuristic {

class NewGoalSubsetHeuristic : public Heuristic {

private:

    std::vector<FactPair> soft_goal_list;
    std::vector<FactPair> hard_goal_list;
    std::vector<FactPair> all_goal_list;

    std::shared_ptr<Evaluator> h;

    std::shared_ptr<Heuristic> goals_heuristic;

    bool initialized = false;

    int min(int x, int y);
    int max(int x, int y);

public:
    explicit NewGoalSubsetHeuristic(const options::Options &opts);

    int compute_heuristic(const State &ancestor_state, MSGSCollection* current_msgs);
    int compute_heuristic(const State &ancestor_state);
    virtual EvaluationResult compute_result(EvaluationContext &eval_context) override;
    virtual void print_statistics() const override;
    
    virtual GoalSubsets get_msgs() const;
    virtual void init_msgs(MSGSCollection goals);
};

}

#endif
