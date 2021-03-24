//
// Created by nullobsi on 2021/03/23.
//

#ifndef PASS_SECRETS_ITEMPROXY_H
#define PASS_SECRETS_ITEMPROXY_H

#include <memory>
#include <sdbus-c++/sdbus-c++.h>
#include "../adaptors/itemadaptor.h"

class Item;

class ItemProxy : public sdbus::AdaptorInterfaces<org::freedesktop::Secret::Item_adaptor> {
public:
	ItemProxy(sdbus::IConnection &conn,
	          std::string path, std::weak_ptr<Item> parent_);

	~ItemProxy();

	sdbus::ObjectPath Delete() override;
	sdbus::Struct<sdbus::ObjectPath, std::vector<uint8_t>, std::vector<uint8_t>, std::string> GetSecret(const sdbus::ObjectPath& session) override;
	void SetSecret(const sdbus::Struct<sdbus::ObjectPath, std::vector<uint8_t>, std::vector<uint8_t>, std::string>& secret) override;
	bool Locked() override;
	std::map<std::string, std::string> Attributes() override;
	void Attributes(const std::map<std::string, std::string>& value) override;
	std::string Label() override;
	void Label(const std::string& value) override;
	std::string Type() override;
	void Type(const std::string& value) override;
	uint64_t Created() override;
	uint64_t Modified() override;

private:
	std::weak_ptr<Item> parent;
};


#endif //PASS_SECRETS_ITEMPROXY_H
