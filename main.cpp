#include <sdbus-c++/sdbus-c++.h>
#include "impl/SecretService.h"

int
main() {
	auto conn = sdbus::createSessionBusConnection();
	conn->requestName("org.freedesktop.secrets");

	auto service = std::make_shared<SecretService>(*conn, "/org/freedesktop/secrets");


	conn->enterEventLoop();
}
