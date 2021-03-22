//
// Created by nullobsi on 2021/03/22.
//

#ifndef PASS_FDO_SECRETS_ITEM_H
#define PASS_FDO_SECRETS_ITEM_H

#include "../interop/PassItem.h"
#include "sdbus-c++/sdbus-c++.h"
#include "../adaptors/itemadaptor.h"

class Collection;

class Item : public sdbus::AdaptorInterfaces<org::freedesktop::Secret::Item_adaptor> {
public:
	Item(std::shared_ptr<PassItem> backend_,
	     sdbus::IConnection &conn,
	     std::string path,
	     std::weak_ptr<Collection> parent_);

	~Item();

	sdbus::ObjectPath
	getPath();

protected:
	sdbus::ObjectPath
	Delete() override;

	sdbus::Struct<sdbus::ObjectPath, std::vector<uint8_t>, std::vector<uint8_t>, std::string>
	GetSecret(const sdbus::ObjectPath &session) override;

	void
	SetSecret(const sdbus::Struct<sdbus::ObjectPath, std::vector<uint8_t>, std::vector<uint8_t>, std::string> &secret) override;

	bool
	Locked() override;

	std::map<std::string, std::string>
	Attributes() override;

	void
	Attributes(const std::map<std::string, std::string> &value) override;

	std::string
	Label() override;

	void
	Label(const std::string &value) override;

	std::string
	Type() override;

	void
	Type(const std::string &value) override;

	uint64_t
	Created() override;

	uint64_t
	Modified() override;


private:
	std::shared_ptr<PassItem> backend;
	std::weak_ptr<Collection> parent;
};


#endif //PASS_FDO_SECRETS_ITEM_H
