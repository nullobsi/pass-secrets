// 
// Created by nullobsi on 2023-10-19.
//

#ifndef PASS_FDO_SECRETS_PREXCEPT
#define PASS_FDO_SECRETS_PREXCEPT
#include <stdexcept>

class runtime_error_p : std::runtime_error {
public:
	runtime_error_p(const std::string &what_arg);
	runtime_error_p(const char *what_arg );
	runtime_error_p(const runtime_error &other ) noexcept;
};

#endif
