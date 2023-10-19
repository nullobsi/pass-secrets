// 
// Created by nullobsi on 2023-10-19.
//

#include "PrintedRuntimeException.h"
#include <iostream>

runtime_error_p::runtime_error_p(const std::string &what_arg) : std::runtime_error(what_arg) {
	std::cerr << what_arg << std::endl;	
}

runtime_error_p::runtime_error_p(const char *what_arg) : std::runtime_error(what_arg) {
	std::cerr << what_arg << std::endl;	
}
runtime_error_p::runtime_error_p(const runtime_error &other ) noexcept : std::runtime_error(other) {
	std::cerr << other.what() << std::endl;
};

