#include <sdbus-c++/sdbus-c++.h>
#include <thread>
#include <mutex>
#include <poll.h>
#include <iostream>
#include "impl/SecretService.h"

int
main() {

	std::unique_ptr<sdbus::IConnection> conn;
	try {
		conn = sdbus::createSessionBusConnection();
	} catch (std::runtime_error &e) {
		std::cerr << "There was an error creating DBus connection." << std::endl;
		std::cerr << "Error: " << e.what() << std::endl;
	}
	conn->requestName("org.freedesktop.secrets");

    std::shared_ptr<SecretService> service;
    try {
        service = std::make_shared<SecretService>(*conn, "/org/freedesktop/secrets");
    } catch(sdbus::Error &e) {
        std::cerr << "There was an error registering to DBus." << std::endl;
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

	service->InitCollections();
	while (true) {
		if (!conn->processPendingRequest()) {
			std::cerr << "There was an error processing DBus events!" << std::endl;
			return 1;
		}
		service->DiscardObjects();

		auto dat = conn->getEventLoopPollData();
		struct pollfd polling[] = {{
				                           dat.fd, dat.events, 0
		                           }};
		auto timeout = dat.timeout_usec == (uint64_t)-1 ? (uint64_t)-1 : (dat.timeout_usec + 999) / 1000;
		auto res = poll(polling, 1, timeout);
		if (res < 1 && errno == EINTR) {
			std::cerr << "Got interrupt, exiting..." << std::endl;
			return 0;
		}
	}
}