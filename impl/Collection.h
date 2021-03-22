//
// Created by nullobsi on 2021/03/17.
//

#ifndef PASS_FDO_SECRETS_COLLECTION_H
#define PASS_FDO_SECRETS_COLLECTION_H

#include "../interop/PassCollection.h"
#include <sdbus-c++/sdbus-c++.h>
#include <utility>
#include "../adaptors/collectionadaptor.h"

class SecretService;

class Item;

class Collection : public sdbus::AdaptorInterfaces<org::freedesktop::Secret::Collection_adaptor> {
public:
	Collection(std::shared_ptr<PassCollection> backend_,
	           sdbus::IConnection &conn,
	           std::string path,
	           std::weak_ptr<SecretService> parent_);

	~Collection();

	std::shared_ptr<PassCollection>
	GetBacking();

	std::vector<sdbus::ObjectPath>
	SearchItems(const std::map<std::string, std::string> &attributes) override;

protected:
	sdbus::ObjectPath
	Delete() override;

	std::tuple<sdbus::ObjectPath, sdbus::ObjectPath>
	CreateItem(const std::map<std::string, sdbus::Variant> &properties,
	           const sdbus::Struct<sdbus::ObjectPath, std::vector<uint8_t>, std::vector<uint8_t>, std::string> &secret,
	           const bool &replace) override;

	std::vector<sdbus::ObjectPath>
	Items() override;

	std::string
	Label() override;

	void
	Label(const std::string &value) override;

	bool
	Locked() override;

	uint64_t
	Created() override;

	uint64_t
	Modified() override;

private:
	std::shared_ptr<PassCollection> backend;
	std::weak_ptr<SecretService> parent;
	std::map<std::string, std::unique_ptr<Item>> items;
};


#endif //PASS_FDO_SECRETS_COLLECTION_H
