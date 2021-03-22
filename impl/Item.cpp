//
// Created by nullobsi on 2021/03/22.
//

#include "Item.h"

#include <utility>

bool
Item::Locked() {
	return backend->isUnlocked();
}

std::map<std::string, std::string>
Item::Attributes() {
	return backend->getAttrib();
}

// TODO: Set attributes
void
Item::Attributes(const std::map<std::string, std::string> &value) {

}

std::string
Item::Label() {
	return backend->getLabel();
}

void
Item::Label(const std::string &value) {
	// TODO: Set label
}

std::string
Item::Type() {
	return backend->getType();
}

void
Item::Type(const std::string &value) {
	// TODO: Set type
}

uint64_t
Item::Created() {
	return backend->getCreated();
}

uint64_t
Item::Modified() {
	return 0;
}

sdbus::ObjectPath
Item::Delete() {
	// TODO: Delete
}

Item::Item(std::shared_ptr<PassItem> backend_,
           sdbus::IConnection &conn,
           std::string path,
           std::weak_ptr<Collection> parent_) : backend(std::move(backend_)),
                                                sdbus::AdaptorInterfaces<org::freedesktop::Secret::Item_adaptor>(conn,
                                                                                                                 std::move(
		                                                                                                                 path)),
                                                parent(std::move(parent_)) {
	registerAdaptor();
}

Item::~Item() {
	unregisterAdaptor();
}

sdbus::Struct<sdbus::ObjectPath, std::vector<uint8_t>, std::vector<uint8_t>, std::string>
Item::GetSecret(const sdbus::ObjectPath &session) {
	if (!backend->isUnlocked()) {
		throw sdbus::createError()
	}
	return sdbus::Struct<sdbus::ObjectPath, std::vector<uint8_t>, std::vector<uint8_t>, std::string>(nullptr);
}

sdbus::ObjectPath
Item::getPath() {
	return sdbus::ObjectPath(getObject().getObjectPath());
}
