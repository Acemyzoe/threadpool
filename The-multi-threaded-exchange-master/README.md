The-multi-threaded-exchange
===========================

Simple example of exchange of data between threads, performing strongly related tasks.

Statement of the problem - have current Thread 1 and is created an additional Thread 2, and then:
<table>
<tr><td>Thread 1</td>      <td>Thread 2</td></tr>
<tr><td> int val = 1; </td><td> - </td></tr>
<tr><td> - </td>           <td> val += 1; </td></tr>
<tr><td> val *= 10;   </td><td> - </td></tr>
<tr><td> - </td>           <td> val += 3; </td></tr>
<tr><td> cout << val; </td><td> - </td></tr>
</table>
Initial value can set with the command line.: example.exe 2
And by default it equal to 1.

When an error in multi-threaded execution, programm try to perform this in single-threaded mode.

Source code contain examples of using different features.


This example is Windows-only since Vista and requires MSVS2010(_MSC_VER >= 1600) or MinGW >= 4.7.2:
- main_winapi_cond_var.cpp - by using WINAPI: CRITICAL_SECTION & CONDITION_VARIABLE


This example is cross-platform and requires Boost >= 1.53 and MSVS2010 (_MSC_VER >= 1600):
- main_boost_asio.cpp - by using boost::asio::io_service and thread pool with single created thread


These examples are cross-platform and require C++11 (_MSC_VER >= 1700 or MinGW/GCC >= 4.7.2):
- main_cpp11_packaged_task.cpp - by using std::packaged_task/std::future, with manual management of async executing 
- main_cpp11_atomic.cpp - by using std::atomic, with waiting changes in spin-loop
- main_cpp11_cond_var.cpp - by using std::condition_variable, with waiting for notification from another thread