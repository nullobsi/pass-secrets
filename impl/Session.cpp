//
// Created by nullobsi on 2021/03/17.
//

#include "Session.h"
#include "SecretService.h"

#include <utility>


Session::Session(std::weak_ptr<SecretService> parent_,
                 sdbus::IConnection &conn,
                 std::string path) : AdaptorInterfaces(conn, std::move(path)), parent(std::move(parent_)) {
	sdbus::Flags flags;
	flags.set(sdbus::Flags::METHOD_NO_REPLY);
	this->getObject()
	    .registerMethod(INTERFACE_NAME, "Close", "", "", [this](sdbus::MethodCall call) -> void { this->Close(call); },
	                    flags);
	registerAdaptor();
}

void
Session::Close(sdbus::MethodCall msg) {
	parent.lock()->DiscardSession(this->getObjectPath());
}

Session::~Session() {
	this->unregisterAdaptor();
};
