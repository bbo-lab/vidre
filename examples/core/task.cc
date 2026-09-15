// example_task_usage.cc
#include <vidre/thread/task.hh>
#include <iostream>
#include <chrono>
#include <vector>

using namespace vidre;

// Example 1: Simple counter with termination check
void counter_handler(shared<task> self, int max_count, const std::string& name) {
    task::scope scope(self);
    
    int count = 0;
    while (!scope.isterminated() && count < max_count) {
        ++count;
        std::cout << name << ": " << count << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    
    std::cout << name << " finished at " << count 
              << (scope.isterminated() ? " (terminated)" : " (completed)") << std::endl;
}

// Example 2: Worker that processes data until termination
void worker_handler(shared<task> self, int id) {
    task::scope scope(self);
    
    int processed = 0;
    while (!scope.isterminated()) {
        // Simulate processing
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        ++processed;
        if (processed % 5 == 0) {
            std::cout << "Worker " << id << ": processed " << processed << " items" << std::endl;
        }
    }
    
    std::cout << "Worker " << id << " terminated after " << processed << " items" << std::endl;
}

// Example 3: Real-time sensor simulation
void sensor_handler(shared<task> self, int sensor_id, int sample_rate_hz) {
    task::scope scope(self);
    
    auto period = std::chrono::milliseconds(1000 / sample_rate_hz);
    int samples = 0;
    
    while (!scope.isterminated()) {
        auto start = std::chrono::steady_clock::now();
        
        // Simulate reading sensor
        ++samples;
        std::cout << "Sensor " << sensor_id << ": sample " << samples << std::endl;
        
        // Sleep until next period
        auto elapsed = std::chrono::steady_clock::now() - start;
        if (elapsed < period) {
            std::this_thread::sleep_for(period - elapsed);
        }
    }
    
    std::cout << "Sensor " << sensor_id << " stopped after " << samples << " samples" << std::endl;
}

int main() {
    std::cout << "=== Task Example Usage ===" << std::endl;
    
    // Example 1: Create and join a counter task
    std::cout << "\n[Example 1: Counter Task]" << std::endl;
    {
        auto counter = task::make(counter_handler, 5, "Counter-A");
        std::cout << "Counter-A isrunning? " << counter->isrunning() << std::endl;
        counter->join();  // Wait for completion
        std::cout << "Counter-A joined. isrunning? " << counter->isrunning() << std::endl;
    }
  
    // Example 2: Multiple workers with graceful termination
    std::cout << "\n[Example 2: Multiple Workers]" << std::endl;
    {
        std::vector<shared<task>> workers;
        for (int i = 0; i < 3; ++i) {
            workers.push_back(task::make(worker_handler, i));
        }
        
        // Let them run for 2 seconds
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));
        
        // Signal all to terminate
        std::cout << "Signaling all workers to terminate..." << std::endl;
        for (auto& w : workers) {
            w->terminate();
        }
        
        // Wait for all to finish
        for (auto& w : workers) {
            w->join();
        }
        std::cout << "All workers joined." << std::endl;
    }
    
    // Example 3: Real-time sensor simulation with priority
    std::cout << "\n[Example 3: Real-time Sensor]" << std::endl;
    {
        auto sensor = task::make(sensor_handler, 42, 10);  // 10 Hz sensor
        
        // Set high priority (if supported)
        if (sensor->priority(0.9f) == status::ok) {
            std::cout << "Sensor priority set to high (0.9)" << std::endl;
        } else {
            std::cout << "Sensor priority setting not supported on this platform" << std::endl;
        }
        
        // Let it run for 1.5 seconds
        std::this_thread::sleep_for(std::chrono::milliseconds(1500));
        
        // Terminate and wait
        sensor->terminate();
        sensor->join();
        std::cout << "Sensor stopped." << std::endl;
    }
    
    // Example 4: Detached task (runs independently)
    std::cout << "\n[Example 4: Detached Task]" << std::endl;
    {
        auto detached = task::make(counter_handler, 8, "Detached-Counter");
        
        // Detach - it runs independently
        detached->detach();
        std::cout << "Task detached. It will continue running independently." << std::endl;
        
        // Let it run for 3 seconds
        std::cout << "Main thread waiting 3 seconds..." << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(3000));
        
        // The detached task will finish on its own when it reaches max_count
        // The shared_ptr in the lambda keeps it alive
        std::cout << "Main thread done. Detached task will finish on its own." << std::endl;
    } // detached goes out of scope, but the thread is still running
      // The task is kept alive by the lambda capture in make()
      // When it finishes, the task is destroyed automatically
    
    // Example 5: Using deadline scheduling (if supported)
    std::cout << "\n[Example 5: Deadline Scheduling]" << std::endl;
    {
        auto deadline_task = task::make(worker_handler, 99);
        
        // Set deadline scheduling (if supported)
        using namespace std::chrono_literals;
        if (deadline_task->set_deadline(100ms, 10ms, 90ms) == status::ok) {
            std::cout << "Deadline scheduling set successfully." << std::endl;
        } else {
            std::cout << "Deadline scheduling not supported on this platform." << std::endl;
        }
        
        // Let it run briefly
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
        deadline_task->terminate();
        deadline_task->join();
    }
    
    // Example 6: Scope usage demonstration
    std::cout << "\n[Example 6: Scope Usage]" << std::endl;
    {
        auto scope_task = task::make([](shared<task> self) {
            // The scope helper manages running_ flag
            task::scope scope(self);
            
            std::cout << "Inside task: running_ = " << self->isrunning() << std::endl;
            
            // Simulate work
            for (int i = 0; i < 3 && !scope.isterminated(); ++i) {
                std::cout << "Working... iteration " << i << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(300));
            }
            
            // When scope goes out of scope, running_ = false
            std::cout << "Exiting scope. running_ will be set to false." << std::endl;
        });
        
        scope_task->join();
        std::cout << "After join: isrunning? " << scope_task->isrunning() << std::endl;
    }

    std::cout << "\n=== All examples completed ===" << std::endl;
    return 0;
}

