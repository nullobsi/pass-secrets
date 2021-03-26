//
// Created by nullobsi on 2021/03/17.
//

#ifndef PASS_FDO_SECRETS_SESSION_H
#define PASS_FDO_SECRETS_SESSION_H

#include "../adaptors/sessionadaptor.h"

class SecretService;

class Session : public sdbus::AdaptorInterfaces<org::freedesktop::Secret::Session_adaptor> {
public:

	Session(std::weak_ptr<SecretService> parent,
	        sdbus::IConnection &conn,
	        std::string path);

	~Session();

	virtual std::pair<std::vector<uint8_t>, std::vector<uint8_t>> encryptSecret(uint8_t *secret, size_t len) = 0;
	virtual sdbus::Variant getOutput() = 0;

private:
	void
	Close(sdbus::MethodCall msg);

	std::weak_ptr<SecretService> parent;
};


#endif //PASS_FDO_SECRETS_SESSION_H
