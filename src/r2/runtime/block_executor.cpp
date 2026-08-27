#include "r2/runtime/block_executor.h"

#include <condition_variable>
#include <deque>
#include <exception>
#include <map>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <utility>
#include <vector>

namespace hz::r2 {

struct FastBlockExecutor::State {
    explicit State(const BlockPlannerOptions options)
        : planner_options(options) {}

    BlockPlannerOptions planner_options;
    std::deque<FastBlockTask> pending;
    std::map<std::uint32_t, FastBlockResult> completed;
    std::vector<std::thread> workers;
    std::mutex mutex;
    std::condition_variable work_ready;
    std::condition_variable result_ready;
    std::exception_ptr failure;
    bool stopping = false;

    void rethrow_failure() const {
        if (failure) {
            std::rethrow_exception(failure);
        }
    }

    static void worker_loop(State* state);
};

void FastBlockExecutor::State::worker_loop(State* const state) {
    try {
        BlockPlanner planner(state->planner_options);
        for (;;) {
            FastBlockTask task;
            {
                std::unique_lock<std::mutex> lock(state->mutex);
                state->work_ready.wait(lock, [&] {
                    return state->stopping || !state->pending.empty();
                });
                if (state->stopping) {
                    return;
                }
                task = std::move(state->pending.front());
                state->pending.pop_front();
            }

            FastBlockResult result;
            result.index = task.index;
            result.checksum = task.checksum;
            result.uncompressed_size = static_cast<std::uint32_t>(task.raw.size());
            result.decision = planner.plan(ByteView(task.raw));

            {
                std::lock_guard<std::mutex> lock(state->mutex);
                if (state->stopping) {
                    return;
                }
                const auto inserted = state->completed.emplace(
                    result.index, std::move(result));
                if (!inserted.second) {
                    throw std::runtime_error(
                        "Duplicate Fast block completion index");
                }
            }
            state->result_ready.notify_all();
        }
    } catch (...) {
        {
            std::lock_guard<std::mutex> lock(state->mutex);
            if (!state->failure) {
                state->failure = std::current_exception();
            }
            state->stopping = true;
            state->pending.clear();
        }
        state->work_ready.notify_all();
        state->result_ready.notify_all();
    }
}

FastBlockExecutor::FastBlockExecutor(BlockPlannerOptions planner_options,
                                     const std::uint32_t worker_count)
    : state_(std::make_unique<State>(planner_options)) {
    if (planner_options.policy != CandidatePolicy::Fast) {
        throw std::invalid_argument(
            "Fast block executor requires the Fast policy");
    }
    if (worker_count == 0U) {
        throw std::invalid_argument("Fast block executor requires a worker");
    }
    state_->workers.reserve(worker_count);
    try {
        for (std::uint32_t worker = 0; worker < worker_count; ++worker) {
            state_->workers.emplace_back(State::worker_loop, state_.get());
        }
    } catch (...) {
        {
            std::lock_guard<std::mutex> lock(state_->mutex);
            state_->stopping = true;
        }
        state_->work_ready.notify_all();
        for (std::thread& worker : state_->workers) {
            if (worker.joinable()) {
                worker.join();
            }
        }
        throw;
    }
}

FastBlockExecutor::~FastBlockExecutor() {
    if (!state_) {
        return;
    }
    {
        std::lock_guard<std::mutex> lock(state_->mutex);
        state_->stopping = true;
        state_->pending.clear();
    }
    state_->work_ready.notify_all();
    state_->result_ready.notify_all();
    for (std::thread& worker : state_->workers) {
        if (worker.joinable()) {
            worker.join();
        }
    }
}

void FastBlockExecutor::submit(FastBlockTask task) {
    if (task.raw.empty()) {
        throw std::invalid_argument("Fast executor received an empty block");
    }
    {
        std::lock_guard<std::mutex> lock(state_->mutex);
        state_->rethrow_failure();
        if (state_->stopping) {
            throw std::runtime_error("Fast block executor is stopping");
        }
        state_->pending.push_back(std::move(task));
    }
    state_->work_ready.notify_one();
}

FastBlockResult FastBlockExecutor::take(const std::uint32_t expected_index) {
    std::unique_lock<std::mutex> lock(state_->mutex);
    state_->result_ready.wait(lock, [&] {
        return state_->failure ||
               state_->completed.find(expected_index) != state_->completed.end() ||
               state_->stopping;
    });
    state_->rethrow_failure();
    const auto result = state_->completed.find(expected_index);
    if (result == state_->completed.end()) {
        throw std::runtime_error("Fast block executor stopped before completion");
    }
    FastBlockResult ordered = std::move(result->second);
    state_->completed.erase(result);
    return ordered;
}

}  // namespace hz::r2
