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
	// с учебной целью возвращать результат будем через future, получаемый из promise
	auto minFunc = [end](TIt localBegin, TIt localEnd, std::promise<TIt> minPromise)
	{
		auto min = std::min_element(localBegin, end);
		minPromise.set_value(min);
	};

	
	while (begin != end)
	{
		// Делим массив на части, в каждой части ищем минимум параллельно
		// потом ищем минимум из найденных минимумов.
		auto size = std::distance(begin, end);
		auto batchCount =  size < 100 ? 1 : std::thread::hardware_concurrency();
		auto wholeBatchSize = size / batchCount;
		auto additionalItems = size % batchCount;
		auto iterationBegin = begin;
		auto localBegin = begin;
		std::vector<std::future<TIt>> futures1;
		std::vector<std::future<void>> futures2;

		for (size_t i = 0; i < batchCount; ++i)
		{
			auto currentBatchSize = wholeBatchSize;
			if (additionalItems > 0)
			{
				++currentBatchSize;
				--additionalItems;
			}

			auto localEnd = localBegin;
			std::advance(localEnd, currentBatchSize);

			std::promise<TIt> minPromise;
			futures1.push_back(minPromise.get_future());

			if (i == batchCount - 1)
			{
				minFunc(localBegin, localEnd, std::move(minPromise));
			}
			else
			{
				futures2.push_back(std::async(minFunc, localBegin, localEnd, std::move(minPromise)));
			}

			localBegin = localEnd;
		}

		for (auto& f : futures2)
		{
			f.wait();
		}

		std::optional<TIt> min;

		for (auto& f : futures1)
		{
			auto currIt = f.get();
			if (!min.has_value() || *min.value() > *currIt)
			{
				min = currIt;
			}
		}

		std::swap(*min.value(), *iterationBegin);
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