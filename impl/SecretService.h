//
// Created by nullobsi on 2021/03/17.
//

#ifndef PASS_FDO_SECRETS_SECRETSERVICE_H
#define PASS_FDO_SECRETS_SECRETSERVICE_H

#include <sdbus-c++/sdbus-c++.h>
#include "../adaptors/secretsadaptor.h"
#include "../interop/PassStore.h"
#include "Collection.h"

class Session;

class SecretService
		: public sdbus::AdaptorInterfaces<org::freedesktop::Secret::Service_adaptor>,
		  public std::enable_shared_from_this<SecretService> {
public:
	SecretService(sdbus::IConnection &conn,
	              std::string path);

	~SecretService();

	void
	CloseSession(const std::string &path);

protected:
	std::tuple<sdbus::Variant, sdbus::ObjectPath>
	OpenSession(const std::string &algorithm,
	            const sdbus::Variant &input) override;

	std::tuple<sdbus::ObjectPath, sdbus::ObjectPath>
	CreateCollection(const std::map<std::string, sdbus::Variant> &properties,
	                 const std::string &alias) override;

	std::tuple<std::vector<sdbus::ObjectPath>, std::vector<sdbus::ObjectPath>>
	SearchItems(const std::map<std::string, std::string> &attributes) override;

	std::tuple<std::vector<sdbus::ObjectPath>, sdbus::ObjectPath>
	Unlock(const std::vector<sdbus::ObjectPath> &objects) override;

	std::tuple<std::vector<sdbus::ObjectPath>, sdbus::ObjectPath>
	Lock(const std::vector<sdbus::ObjectPath> &objects) override;

	void
	LockService() override;

	sdbus::ObjectPath
	ChangeLock(const sdbus::ObjectPath &collection) override;

	std::map<sdbus::ObjectPath, sdbus::Struct<sdbus::ObjectPath, std::vector<uint8_t>, std::vector<uint8_t>, std::string>>
	GetSecrets(const std::vector<sdbus::ObjectPath> &items,
	           const sdbus::ObjectPath &session) override;

	sdbus::ObjectPath
	ReadAlias(const std::string &name) override;

	void
	SetAlias(const std::string &name,
	         const sdbus::ObjectPath &collection) override;

	std::vector<sdbus::ObjectPath>
	Collections() override;

private:
	std::map<std::string, std::unique_ptr<Session>> sessions;
	std::vector<std::unique_ptr<Session>> discarded;

	PassStore store;
	std::map<std::string, std::unique_ptr<Collection>> collections;
};

#endif //PASS_FDO_SECRETS_SECRETSERVICE_H
