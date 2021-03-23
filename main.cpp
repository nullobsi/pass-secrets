#include <sdbus-c++/sdbus-c++.h>
#include <thread>
#include <mutex>
#include <poll.h>
#include <iostream>
#include "impl/SecretService.h"

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
int
main() {
	auto conn = sdbus::createSessionBusConnection();
	conn->requestName("org.freedesktop.secrets");

	auto service = std::make_shared<SecretService>(*conn, "/org/freedesktop/secrets");

	while(true){
		std::cout << "\tProcessing..." << std::endl;
		if (conn->processPendingRequest()) {
			std::cout << "\tDone!" << std::endl;
		}

		service->DiscardObjects();

		auto dat = conn->getEventLoopPollData();
		struct pollfd polling[] = {{
				                           dat.fd,
				                           dat.events,
				                           0
		                           }};
		auto timeout = dat.timeout_usec == (uint64_t) -1 ? (uint64_t)-1 : (dat.timeout_usec+999)/1000;
		auto res = poll(polling, 1, timeout);
		std::cout << "Timeout/Event" << std::endl;
		std::cout << "\tn.Events: " + std::to_string(res) << std::endl;
	}
}

#pragma clang diagnostic pop