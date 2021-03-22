//
// Created by nullobsi on 2021/03/17.
//

#include "Session.h"
#include "SecretService.h"

#include <utility>

void
Session::Close() {
	parent.lock()->CloseSession(this->getObjectPath());
}

Session::Session(std::weak_ptr<SecretService> parent_,
                 sdbus::IConnection &conn,
                 std::string path) : AdaptorInterfaces(conn, std::move(path)), parent(std::move(parent_)) {
	registerAdaptor();
}

Session::~Session() {
	unregisterAdaptor();
}
