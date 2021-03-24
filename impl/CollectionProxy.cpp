//
// Created by nullobsi on 2021/03/23.
//

#include "CollectionProxy.h"
#include "Collection.h"

CollectionProxy::CollectionProxy(sdbus::IConnection &conn,
                                 std::string path,
                                 std::weak_ptr<Collection> parent_) : sdbus::AdaptorInterfaces<org::freedesktop::Secret::Collection_adaptor>(conn, std::move(path)), parent(std::move(parent_)) {
	registerAdaptor();
}

CollectionProxy::~CollectionProxy() {
	unregisterAdaptor();
}

sdbus::ObjectPath
CollectionProxy::Delete() {
	return parent.lock()->Delete();
}

std::vector<sdbus::ObjectPath>
CollectionProxy::SearchItems(const std::map<std::string, std::string> &attributes) {
	return parent.lock()->SearchItems(attributes);
}

std::tuple<sdbus::ObjectPath, sdbus::ObjectPath>
CollectionProxy::CreateItem(const std::map<std::string, sdbus::Variant> &properties,
                            const sdbus::Struct<sdbus::ObjectPath, std::vector<uint8_t>, std::vector<uint8_t>, std::string> &secret,
                            const bool &replace) {
	return parent.lock()->CreateItem(properties, secret, replace);
}

std::vector<sdbus::ObjectPath>
CollectionProxy::Items() {
	return parent.lock()->Items();
}

std::string
CollectionProxy::Label() {
	return parent.lock()->Label();
}

void
CollectionProxy::Label(const std::string &value) {
	parent.lock()->Label(value);
}

bool
CollectionProxy::Locked() {
	return parent.lock()->Locked();
}

uint64_t
CollectionProxy::Created() {
	return parent.lock()->Created();
}

uint64_t
CollectionProxy::Modified() {
	return parent.lock()->Modified();
}
