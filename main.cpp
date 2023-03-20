#include <iostream>
#ifdef _WIN32
#include "windows.h"
#endif

#include <future>
#include <optional>
#include <random>
#include <chrono>

using namespace std::chrono;

template <typename TIt>
void selectionSort(TIt begin, TIt end)
{
	auto minFunc = [end](TIt localBegin, std::promise<TIt>& minPromise)
	{
		auto min = std::min_element(localBegin, end);
		minPromise.set_value(min);
	};

	while (begin != end)
	{
		std::promise<TIt> minPromise;
		auto minFuture = minPromise.get_future();
		auto mf = std::async(minFunc, begin, std::ref(minPromise));
		mf.wait();
		auto min = minFuture.get();
		std::swap(*begin, *min);
		++begin;
	}
}



int main()
{
	setlocale(LC_ALL, "Russian");
#ifdef _WIN32
	SetConsoleCP(1251);
#endif

	std::mt19937 gen(steady_clock::now().time_since_epoch().count());
	std::uniform_int_distribution<int> dis(-100, 100);
	auto rand_num([&dis, &gen]() mutable { return dis(gen); });
	std::vector<int> vec(100);
	std::generate(vec.begin(), vec.end(), rand_num);

	std::cout << "Исходный вектор" << std::endl;
	std::for_each(vec.cbegin(), vec.cend(), [](int v) { std::cout << v << " ";  });
	std::cout << std::endl;

	selectionSort(vec.begin(), vec.end());

	std::cout << "Отсортированный вектор" << std::endl;
	std::for_each(vec.cbegin(), vec.cend(), [](int v) { std::cout << v << " ";  });
	std::cout << std::endl;

}