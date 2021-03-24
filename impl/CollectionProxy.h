//
// Created by nullobsi on 2021/03/23.
//

#ifndef PASS_SECRETS_COLLECTIONPROXY_H
#define PASS_SECRETS_COLLECTIONPROXY_H
#include "sdbus-c++/sdbus-c++.h"
#include "../adaptors/collectionadaptor.h"

class Collection;

class CollectionProxy : public sdbus::AdaptorInterfaces<org::freedesktop::Secret::Collection_adaptor>{
public:
	CollectionProxy(sdbus::IConnection &conn,
			std::string path, std::weak_ptr<Collection> parent_);

	~CollectionProxy();

	sdbus::ObjectPath Delete() override;
	std::vector<sdbus::ObjectPath> SearchItems(const std::map<std::string, std::string>& attributes) override;
	std::tuple<sdbus::ObjectPath, sdbus::ObjectPath> CreateItem(const std::map<std::string, sdbus::Variant>& properties, const sdbus::Struct<sdbus::ObjectPath, std::vector<uint8_t>, std::vector<uint8_t>, std::string>& secret, const bool& replace) override;
	std::vector<sdbus::ObjectPath> Items() override;
	std::string Label() override;
	void Label(const std::string& value) override;
	bool Locked() override;
	uint64_t Created() override;
	uint64_t Modified() override;

private:
	std::weak_ptr<Collection> parent;
};


#endif //PASS_SECRETS_COLLECTIONPROXY_H
