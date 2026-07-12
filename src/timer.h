#pragma once

#include <string_view>
#include <chrono>

#include "fmt/format.h"

using namespace std::literals;

// trivial timer object that just writes v xxus to std::out where v is supplied in constructor
// so
// int my_func ()
// {
//		timer("my func: ");
//      // do something to time
// }
// etc.
//
// more is left as exercise for now.
//
class timer
{
private:
	std::string_view v_;
	bool cancel_;
	decltype(std::chrono::high_resolution_clock().now()) start_;
	timer(timer&) = delete;
public:
	timer(std::string_view v) : v_{ v }, cancel_{ false }, start_ { std::chrono::high_resolution_clock().now() }
	{
	}
	~timer()
	{
		auto end = std::chrono::high_resolution_clock().now();
		if(!cancel_)
			fmt::println("({}: {}us)", v_, std::chrono::duration_cast<std::chrono::microseconds>(end - start_).count());
	}
	void cancel()
	{
		cancel_ = true;
	}
};