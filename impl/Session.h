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

private:
	void
	Close() override;

	std::weak_ptr<SecretService> parent;
};


#endif //PASS_FDO_SECRETS_SESSION_H
