//
// Created by nullobsi on 2021/03/17.
//

#include "Collection.h"
#include "SecretService.h"
#include "Item.h"

#include <utility>


Collection::Collection(std::shared_ptr<PassCollection> backend_,
                       sdbus::IConnection &conn,
                       std::string path,
                       std::weak_ptr<SecretService> parent_)
		: sdbus::AdaptorInterfaces<org::freedesktop::Secret::Collection_adaptor>(conn, std::move(path)),
		  backend(std::move(backend_)), parent(std::move(parent_)) {
	registerAdaptor();
}

Collection::~Collection() {
	unregisterAdaptor();
}

sdbus::ObjectPath
Collection::Delete() {
	// TODO: Delete
	return sdbus::ObjectPath("/");
}

std::vector<sdbus::ObjectPath>
Collection::SearchItems(const std::map<std::string, std::string> &attributes) {
	auto it = backend->searchItems(attributes);
	std::vector<sdbus::ObjectPath> r;
	r.reserve(it.size());
	for (const auto &id : it) {
		r.push_back(items[id]->getPath());
	}
	return r;
}

std::tuple<sdbus::ObjectPath, sdbus::ObjectPath>
Collection::CreateItem(const std::map<std::string, sdbus::Variant> &properties,
                       const sdbus::Struct<sdbus::ObjectPath, std::vector<uint8_t>, std::vector<uint8_t>, std::string> &secret,
                       const bool &replace) {
	// TODO: Create items
	return std::tuple<sdbus::ObjectPath, sdbus::ObjectPath>();
}

std::vector<sdbus::ObjectPath>
Collection::Items() {
	std::vector<sdbus::ObjectPath> r;
	auto it = backend->getItems();
	r.reserve(it.size());
	for (const auto &item : it) {
		r.push_back(items[item->getId()]->getPath());
	}
	return r;
}

std::string
Collection::Label() {
	return backend->getLabel();
}

void
Collection::Label(const std::string &value) {
	// TODO: Set label
}

bool
Collection::Locked() {
	// TODO: Lock/unlock
	return false;
}

uint64_t
Collection::Created() {
	return backend->getCreated();
}

uint64_t
Collection::Modified() {
	return 0;
}

std::shared_ptr<PassCollection>
Collection::GetBacking() {
	return backend;
}