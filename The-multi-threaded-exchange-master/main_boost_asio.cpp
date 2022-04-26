#include <iostream>		// std::cout
#include <vector>		// std::vector
#include <functional>	// std::function
#include <string>		// std::string, std::stoi
#include <stdexcept>	// std::runtime_error


#include <boost/shared_ptr.hpp>			// boost::shared_ptr
#include <boost/thread/thread.hpp>		// boost::thread
#include <boost/thread/mutex.hpp>		// boost::mutex
#include <boost/thread/lock_guard.hpp>	// boost::lock_guard
#include <boost/asio.hpp>				// boost::asio::io_service, boost::asio::io_service::work
namespace ba = boost::asio;

class T_test_task {
public:
	typedef std::vector<std::function<int(volatile int const&)>> T_array_tasks;
private:

	static void thread_func(volatile int &val, const volatile size_t i, T_array_tasks const& tasks, boost::shared_ptr<boost::lock_guard<boost::mutex>> lock_ptr) { 
		try {
			val = tasks.at(i)(val); 
		} catch(std::exception &ex) {
			std::cerr << "Catch an exception in: thread_func(). Exception: " << ex.what() << std::endl;
		} catch(...) {
			std::cerr << "Catch an exception in: thread_func() \n";
		}
	}				

	static void loop_tasks(volatile int &val, ba::io_service &io_srv, T_array_tasks const& t1_tasks, T_array_tasks const& t2_tasks)
	{
		try {
			boost::mutex mtx;
			for(volatile size_t i = 0; i < t1_tasks.size(); ++i) {
				// Start tasks in threads
				{
					boost::shared_ptr<boost::lock_guard<boost::mutex>> lock_ptr(new boost::lock_guard<boost::mutex>(mtx));
					io_srv.post( boost::bind(thread_func, boost::ref(val), i, boost::ref(t2_tasks), lock_ptr) );	// 1st thread
				}

				{
					boost::shared_ptr<boost::lock_guard<boost::mutex>> lock_ptr(new boost::lock_guard<boost::mutex>(mtx));
					thread_func(val, i, t1_tasks, lock_ptr);														// 2nd thread (current)	
				}
			}
		} catch(std::exception &ex) {
			std::cerr << "Catch an exception in: loop_tasks(). Exception: " << ex.what() << std::endl;
		} catch(...) {
			std::cerr << "Catch an exception in: loop_tasks() \n";
		}
	}

public:

	// Prepare and run tasks
	static void run_tasks(int const& in_val, T_array_tasks const& t1_tasks, T_array_tasks const& t2_tasks)
	{
		try {
			volatile int val = in_val;
			ba::io_service io_srv;
			ba::io_service::work worker(io_srv);
			boost::shared_ptr<boost::thread> thread_ptr;
			
			// Try execute in multiple threads
			try {
				//throw(std::runtime_error("Can't create thread!"));
				// Start thread in threads-pool
				thread_ptr.reset(new boost::thread([&io_srv]{ io_srv.run(); } ));
				//throw(std::runtime_error("Can't create thread!"));
				loop_tasks(val, io_srv, t1_tasks, t2_tasks);
				
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
			io_srv.stop();
			if(thread_ptr) thread_ptr->join();
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
		std::cout << "Usage:   Racurs_v5.exe [init_val]" << std::endl;
		std::cout << "Default: Racurs_v5.exe " << in_val << std::endl;

		if(argc > 1)
			in_val = std::stoi(argv[1]);

		// Tasks for both threads
		T_test_task::T_array_tasks t1_tasks, t2_tasks;
		t1_tasks.emplace_back([](volatile int const& val) -> int { return val * 10; });
		t1_tasks.emplace_back([](volatile int const& val) -> int { std::cout << val << std::endl; return val; });

		t2_tasks.emplace_back([](volatile int const& val) -> int { return val + 1; });
		t2_tasks.emplace_back([](volatile int const& val) -> int { return val + 3; });

		T_test_task::run_tasks(in_val, t1_tasks, t2_tasks);

	} catch(...) {
		std::cerr << "Catch an exception in: main() \n";
	}

	int b;
	std::cin >> b;
	return 0;
}