#include "relaxation_iterated_search.h"

#include "../option_parser.h"
#include "../plugin.h"
#include "modified_init_task.h"
#include "../tasks/root_task.h"
#include "../heuristic.h"
#include "../explicit_mugs_search/msgs_collection.h"
#include "../goal_subsets/output_handler.h"
#include "../../search_engines/eager_search.h"

#include <iostream>

using namespace std;
using namespace goalsubset;

namespace relaxation_iterated_search {
RelaxationIteratedSearch::RelaxationIteratedSearch(const Options &opts, options::Registry &registry,
                               const options::Predefinitions &predefinitions)
    : SearchEngine(opts),
    engine_configs(opts.get_list<ParseTree>("engine_configs")),
    registry(registry),
    predefinitions(predefinitions),
    // pruning_method(opts.get<shared_ptr<PruningMethod>>("pruning")),
    propagate_msgs(opts.get<bool>("propagate_msgs")){

    taskRelaxationTracker = new TaskRelaxationTracker(task);
}

shared_ptr<SearchEngine> RelaxationIteratedSearch::get_search_engine() {

    //update global root task with current relaxed task
    relaxedTask = taskRelaxationTracker->next_relaxed_task();
    tasks::g_root_task = make_shared<extra_tasks::ModifiedInitTask>(task, relaxedTask->get_init());

    cout << "*******************************************************" << endl;
    cout << "Iteration: " << relaxedTask->get_name() << endl;

    cout << "Init state: "  << endl;
    for (FactPair fp : relaxedTask->get_init())
        cout << fp.var << " = " << fp.value << endl;

    if(propagate_msgs) {
        for (RelaxedTask *t: relaxedTask->get_lower_cover()) {
                relaxedTask->add_msgs(t->get_msgs());
        }
    }

    // pruning_method->init_msgs(relaxedTask->get_msgs());
    // pruning_method->set_abstract_task(tasks::g_root_task);
    
    cout << "gen new engine" << endl;

    OptionParser parser(engine_configs[0], registry, predefinitions, false);
    shared_ptr<SearchEngine> engine(parser.start_parsing<shared_ptr<SearchEngine>>());

    cout << "update pruning method" << endl;
    static_pointer_cast<eager_search::EagerSearch>(engine)->get_pruning_method()->init_msgs(relaxedTask->get_msgs());

    cout << "Starting search log: ";
    kptree::print_tree_bracketed(engine_configs[0], cout);
    cout << endl;

    return engine;
}

shared_ptr<SearchEngine> RelaxationIteratedSearch::create_phase() {

    if (taskRelaxationTracker->has_next_relaxed_task()) {
        return get_search_engine();
    } else {
        return nullptr;
    }
}

SearchStatus RelaxationIteratedSearch::step() {
    shared_ptr<SearchEngine> current_search = create_phase();
    if (!current_search) {
        return found_solution() ? SOLVED : FAILED;
    }

    cout << "start search" << endl;
    current_search->search();

    relaxedTask->add_msgs(static_pointer_cast<eager_search::EagerSearch>(current_search)->get_pruning_method()->get_msgs());
    // relaxedTask->print();


    if (current_search->found_solution()){
        relaxedTask->propagate_solvable(relaxedTask->get_msgs());
    }

    current_search->print_statistics();

    const SearchStatistics &current_stats = current_search->get_statistics();
    statistics.inc_expanded(current_stats.get_expanded());
    statistics.inc_evaluated_states(current_stats.get_evaluated_states());
    statistics.inc_evaluations(current_stats.get_evaluations());
    statistics.inc_generated(current_stats.get_generated());
    statistics.inc_generated_ops(current_stats.get_generated_ops());
    statistics.inc_reopened(current_stats.get_reopened());

    return step_return_value();
}

SearchStatus RelaxationIteratedSearch::step_return_value() {
    if(taskRelaxationTracker->has_next_relaxed_task()){
        return IN_PROGRESS;
    }

    return SOLVED;
}

void RelaxationIteratedSearch::print_statistics() const {
    cout << "Cumulative statistics:" << endl;
    statistics.print_detailed_statistics();

    // taskRelaxationTracker->results_to_file();
}

void RelaxationIteratedSearch::save_plan_if_necessary() {
    // We don't need to save here, as we automatically save after
    // each successful search iteration.
}

static shared_ptr<SearchEngine> _parse(OptionParser &parser) {
    parser.document_synopsis("Relaxation Iterated search", "");
    parser.add_list_option<ParseTree>("engine_configs",
                                      "list of search engines for each phase");
    parser.add_option<bool>("propagate_msgs",
                            "propagate the maximally solvable goal subsets",
                            "true");
    // parser.add_option<shared_ptr<PruningMethod>>(
    //     "pruning",
    //     "TODO",
    //     "null()");
    SearchEngine::add_options_to_parser(parser);
    Options opts = parser.parse();

    opts.verify_list_non_empty<ParseTree>("engine_configs");

    if (parser.help_mode()) {
        return nullptr;
    } else if (parser.dry_run()) {
        //check if the supplied search engines can be parsed
        for (const ParseTree &config : opts.get_list<ParseTree>("engine_configs")) {
            OptionParser test_parser(config, parser.get_registry(),
                                     parser.get_predefinitions(), true);
            test_parser.start_parsing<shared_ptr<SearchEngine>>();
        }
        return nullptr;
    } else {
        return make_shared<RelaxationIteratedSearch>(opts, parser.get_registry(),
                                           parser.get_predefinitions());
    }
}

static Plugin<SearchEngine> _plugin("relaxation_iterated", _parse);
}
