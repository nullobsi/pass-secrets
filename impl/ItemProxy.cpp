//
// Created by nullobsi on 2021/03/23.
//

#include "ItemProxy.h"
#include "Item.h"

ItemProxy::ItemProxy(sdbus::IConnection &conn,
                     std::string path,
                     std::weak_ptr<Item> parent_): parent(std::move(parent_)), sdbus::AdaptorInterfaces<org::freedesktop::Secret::Item_adaptor>(conn, std::move(path)) {
	registerAdaptor();
}

ItemProxy::~ItemProxy() {
	unregisterAdaptor();
}

sdbus::ObjectPath
ItemProxy::Delete() {
	return parent.lock()->Delete();
}

sdbus::Struct<sdbus::ObjectPath, std::vector<uint8_t>, std::vector<uint8_t>, std::string>
ItemProxy::GetSecret(const sdbus::ObjectPath &session) {
	return parent.lock()->GetSecret(session);
}

void
ItemProxy::SetSecret(const sdbus::Struct<sdbus::ObjectPath, std::vector<uint8_t>, std::vector<uint8_t>, std::string> &secret) {
	return parent.lock()->SetSecret(secret);
}

bool
ItemProxy::Locked() {
	return parent.lock()->Locked();
}

std::map<std::string, std::string>
ItemProxy::Attributes() {
	return parent.lock()->Attributes();
}

void
ItemProxy::Attributes(const std::map<std::string, std::string> &value) {
	return parent.lock()->Attributes(value);
}

std::string
ItemProxy::Label() {
	return parent.lock()->Label();
}

void
ItemProxy::Label(const std::string &value) {
	return parent.lock()->Label(value);
}

std::string
ItemProxy::Type() {
	return parent.lock()->Type();
}

void
ItemProxy::Type(const std::string &value) {
	return parent.lock()->Type(value);
}

uint64_t
ItemProxy::Created() {
	return parent.lock()->Created();
}

uint64_t
ItemProxy::Modified() {
	return parent.lock()->Modified();
}
