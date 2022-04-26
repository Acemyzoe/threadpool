#include <iostream> // std::cout
#include <vector>	// std::vector
// #include <thread>		// std::thread
#include <future>	 // std::packaged_task, std::future
#include <string>	 // std::string, std::stoi
#include <stdexcept> // std::runtime_error

class T_test_task
{
public:
	typedef std::vector<std::function<int(int const &)>> T_array_tasks;

private:
	typedef std::packaged_task<int(int const &)> T_packaged_task;
	typedef std::vector<T_packaged_task> T_array_packaged_tasks;
	typedef std::vector<std::future<int>> T_array_futures;

	// Execute sequence of tasks
	static void thread_func(T_array_packaged_tasks &tasks, T_array_futures &futures)
	{
		try
		{
			for (size_t i = 0; i < tasks.size(); ++i)
				tasks[i](futures[i].get());
		}
		catch (std::exception &ex)
		{
			std::cerr << "Catch an exception in: thread_func(). Exception: " << ex.what() << std::endl;
		}
		catch (...)
		{
			std::cerr << "Catch an exception in: thread_func() \n";
		}
	}

public:
	// Prepare and run tasks
	static void run_tasks(int const &in_val, T_array_tasks const &t1_tasks, T_array_tasks const &t2_tasks)
	{
		try
		{
			// Async packeged tasks
			T_array_packaged_tasks t1_packaged_tasks, t2_packaged_tasks;
			for (auto &i : t1_tasks)
				t1_packaged_tasks.emplace_back(i);
			for (auto &i : t2_tasks)
				t2_packaged_tasks.emplace_back(i);

			// Tasks await these futures
			T_array_futures t1_futures, t2_futures;

			// First init value
			std::promise<int> start_value;
			t2_futures.emplace_back(start_value.get_future());

			for (auto &i : t2_packaged_tasks)
				t1_futures.emplace_back(i.get_future());
			for (auto &i : t1_packaged_tasks)
				t2_futures.emplace_back(i.get_future());

			// Try execute in multiple threads
			try
			{
				start_value.set_value(in_val);
				// throw(std::runtime_error("Can't create thread!"));
				//  Start tasks in threads
				std::thread([&]
							{ thread_func(t2_packaged_tasks, t2_futures); })
					.detach(); // 1st thread
				// throw(std::runtime_error("Can't create thread!"));
				thread_func(t1_packaged_tasks, t1_futures); // 2nd thread (current)

				std::cout << "Done in multi-threaded mode. \n";
			}
			catch (...)
			{
				// If can't execute in multiple threads, then try execute in single thread
				int current_val = in_val;
				for (size_t i = 0; i < t2_tasks.size(); ++i)
				{
					current_val = t2_tasks[i](current_val);
					current_val = t1_tasks[i](current_val);
				}
				std::cout << "Done in single-threaded mode. \n";
			}
		}
		catch (std::exception &ex)
		{
			std::cerr << "Catch an exception in: run_tasks(). Exception: " << ex.what() << std::endl;
		}
		catch (...)
		{
			std::cerr << "Catch an exception in: run_tasks() \n";
		}
	}
};

int main(const int argc, char *const argv[])
{
	try
	{
		int in_val = 1;
		std::cout << "Usage:   Racurs_v1.exe [init_val]" << std::endl;
		std::cout << "Default: Racurs_v1.exe " << in_val << std::endl;

		if (argc > 1)
			in_val = std::stoi(argv[1]);

		// Tasks for both threads
		T_test_task::T_array_tasks t1_tasks, t2_tasks;
		t1_tasks.emplace_back([](volatile int const &val)
							  { return val * 10; });
		t1_tasks.emplace_back([](volatile int const &val)
							  { std::cout << val << std::endl; return val; });

		t2_tasks.emplace_back([](volatile int const &val)
							  { return val + 1; });
		t2_tasks.emplace_back([](volatile int const &val)
							  { return val + 3; });

		T_test_task::run_tasks(in_val, t1_tasks, t2_tasks);
	}
	catch (...)
	{
		std::cerr << "Catch an exception in: main() \n";
	}

	// int b;
	// std::cin >> b;
	return 0;
}
