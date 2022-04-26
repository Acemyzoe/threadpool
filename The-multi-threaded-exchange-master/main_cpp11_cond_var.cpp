#include <iostream>		// std::cout
#include <vector>		// std::vector
#include <memory>		// std::shared_ptr, std::make_shared
#include <thread>		// std::thread
#include <mutex>		// std::mutex
#include <functional>	// std::function
#include <condition_variable>		// std::condition_variable
#include <string>		// std::string, std::stoi
#include <stdexcept>	// std::runtime_error


class T_test_task {
public:
	typedef std::vector<std::function<int(volatile int const&)>> T_array_tasks;
private:

	static void thread_func(const bool current_flag, 
							std::shared_ptr<volatile bool> flag_thread, 
							std::shared_ptr<volatile int> val, 
							std::shared_ptr<std::mutex> mtx, 
							std::shared_ptr<std::condition_variable> wait_notify, 
							std::shared_ptr<std::condition_variable> send_notify, 
							T_array_tasks const& array_tasks) 
	{
		try {
			for(auto &i : array_tasks) {
				std::unique_lock<std::mutex> lock(*mtx);
				wait_notify->wait(lock, [&] { return current_flag == *flag_thread; });
				*val = i(*val);
				*flag_thread = !*flag_thread;
				send_notify->notify_one();
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
			auto flag_thread = std::make_shared<volatile bool>(1);
			auto val = std::make_shared<volatile int>(in_val);			
			auto mtx = std::make_shared<std::mutex>();
			auto launch_thread1 = std::make_shared<std::condition_variable>(), launch_thread2 = std::make_shared<std::condition_variable>();

			// Try execute in multiple threads
			try {
				//throw(std::runtime_error("Can't create thread!"));
				// Start tasks in threads
				std::thread([&]{ thread_func(1, flag_thread, val, mtx, launch_thread1, launch_thread2, t2_tasks); } ).detach();	// 1st thread
				//throw(std::runtime_error("Can't create thread!"));
				thread_func(0, flag_thread, val, mtx, launch_thread2, launch_thread1, t1_tasks);								// 2nd thread (current)	

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

void my_unexpected() {
	std::cerr << "Unexpected called\n";
	exit(0);
}

void my_terminate () {
	std::cerr << "Terminate handler called\n";
	exit(0);
}

#ifdef _MSC_VER
#include <windows.h>
// Generate exception by using std::runtime_error
void trans_func(unsigned int error_code, EXCEPTION_POINTERS* pExp) { throw std::runtime_error("Windows SEH!"); }

// Windwos SEH-exception init. Must compile with: /EHa
void seh_exception_init() { ::_set_se_translator( trans_func ); }
#else
// In not-Windows OS nothing to do
void seh_exception_init() {}
#endif

int main(const int argc, char *const argv[]) {
	try {
		// Init Exceptions
		std::set_unexpected(my_unexpected);
		std::set_terminate(my_terminate);
		seh_exception_init();

		int in_val = 1;
		std::cout << "Usage:   Racurs_v3.exe [init_val]" << std::endl;
		std::cout << "Default: Racurs_v3.exe " << in_val << std::endl;

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