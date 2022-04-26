#include <iostream>		// std::cout
#include <vector>		// std::vector
#include <memory>		// std::shared_ptr, std::make_shared
#include <thread>		// std::thread
#include <atomic>		// std::atomic
#include <functional>	// std::function
#include <string>		// std::string, std::stoi
#include <stdexcept>	// std::runtime_error

class T_test_task {
public:
	typedef std::vector<std::function<int(volatile int const&)>> T_array_tasks;
private:

	static void thread_func(int current_val, std::shared_ptr<std::atomic<int>> val, T_array_tasks const& array_tasks) {
		try {
			for(auto &i : array_tasks) {
				while(current_val == val->load(std::memory_order_acquire));

				current_val = val->load(std::memory_order_consume);
				current_val = i(current_val);
				val->store(current_val, std::memory_order_release);
			}
		} catch(std::exception &ex) {
			std::cerr << "Catch an exception in: thread_func(). Exception: " << ex.what() << std::endl;
		} catch(...) {
			std::cerr << "Catch an exception in: thread_func() \n";
		}
	}

public:

	// Prepare and run tasks
	static void run_tasks(int const& in_val, T_array_tasks const& t1_tasks, T_array_tasks const& t2_tasks)
	{
		try {
			auto val = std::make_shared<std::atomic<int>>();
			*val = in_val;

			// Try execute in multiple threads
			try {
				//throw(std::runtime_error("Can't create thread!"));
				// Start tasks in threads
				std::thread([&]{ thread_func(in_val + 1, val, t2_tasks); } ).detach();	// 1st thread
				//throw(std::runtime_error("Can't create thread!"));
				thread_func(in_val, val, t1_tasks);										// 2nd thread (current)

				std::cout << "Done in multi-threaded mode. \n";
			} catch(...) {
				// If can't execute in multiple threads, then try execute in single thread
				int current_val = in_val;
				for(size_t i = 0; i < t2_tasks.size(); ++i) {
					current_val = t2_tasks[i](current_val);
					current_val = t1_tasks[i](current_val);
				}
				std::cout << "Done in single-threaded mode. \n";
			}
		} catch(std::exception &ex) {
			std::cerr << "Catch an exception in: run_tasks(). Exception: " << ex.what() << std::endl;
		} catch(...) {
			std::cerr << "Catch an exception in: run_tasks() \n";
		}
	}
};

int main(const int argc, char *const argv[]) {
	try {
		int in_val = 1;
		std::cout << "Usage:   Racurs_v2.exe [init_val]" << std::endl;
		std::cout << "Default: Racurs_v2.exe " << in_val << std::endl;

		if(argc > 1)
			in_val = std::stoi(argv[1]);

		// Tasks for both threads
		T_test_task::T_array_tasks t1_tasks, t2_tasks;
		t1_tasks.emplace_back([](volatile int const& val){ return val * 10; });
		t1_tasks.emplace_back([](volatile int const& val){ std::cout << val << std::endl; return val; });

		t2_tasks.emplace_back([](volatile int const& val){ return val + 1; });
		t2_tasks.emplace_back([](volatile int const& val){ return val + 3; });

		T_test_task::run_tasks(in_val, t1_tasks, t2_tasks);

	} catch(...) {
		std::cerr << "Catch an exception in: main() \n";
	}

	int b;
	std::cin >> b;
	return 0;
}