//
// Created by nullobsi on 2021/03/17.
//

#include "Collection.h"
#include "SecretService.h"
#include "Item.h"
#include "CollectionProxy.h"
#include <memory>
#include <utility>


Collection::Collection(std::shared_ptr<PassCollection> backend_,
                       sdbus::IConnection &conn,
                       std::string path,
                       std::weak_ptr<SecretService> parent_)
		: sdbus::AdaptorInterfaces<org::freedesktop::Secret::Collection_adaptor, sdbus::Properties_adaptor>(conn, std::move(path)),
		  backend(std::move(backend_)), parent(std::move(parent_)) {
	registerAdaptor();
}

Collection::~Collection() {
	unregisterAdaptor();
}

sdbus::ObjectPath
Collection::Delete() {
	backend->Delete();
	parent.lock()->DiscardCollection(this->backend->getId());
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
	auto nAttrib = properties.count("org.freedesktop.Secret.Item.Attributes") && properties.at("org.freedesktop.Secret.Item.Attributes").containsValueOfType<std::map<std::string, std::string>>() ? properties.at("org.freedesktop.Secret.Item.Attributes").get<std::map<std::string, std::string>>() : std::map<std::string, std::string>();
	auto nLabel = properties.count("org.freedesktop.Secret.Item.Label") && properties.at("org.freedesktop.Secret.Item.Label").containsValueOfType<std::string>() ? properties.at("org.freedesktop.Secret.Item.Label").get<std::string>() : "Secret";
	auto nType = properties.count("org.freedesktop.Secret.Item.Type") && properties.at("org.freedesktop.Secret.Item.Type").containsValueOfType<std::string>() ? properties.at("org.freedesktop.Secret.Item.Type").get<std::string>() : (nAttrib.count("xdg:schema") ? nAttrib["xdg:schema"] : "org.freedesktop.Secret.Generic");
	auto existing = InternalSearchItems(nAttrib);
	if (!existing.empty() && !replace) {
		// TODO: this error is not part of spec
		throw sdbus::Error("org.freedesktop.Secret.Error.ObjectExists", "Such an object already exists in the store");
	}
	auto data = secret.get<2>();
	auto nData = (uint8_t *)malloc(data.size() * sizeof(uint8_t));
	memcpy(nData, data.data(), data.size()*sizeof(uint8_t));

	if (replace && !existing.empty()) {
		auto item = existing[0]->getBackend();
		item->setAttrib(std::move(nAttrib));
		item->setLabel(std::move(nLabel));
		item->setType(std::move(nType));
		item->updateMetadata();
		item->setSecret(nData, data.size());
		ItemCreated(existing[0]->getPath());
		return std::tuple(existing[0]->getPath(), "/");
	}
	auto item = this->backend->CreateItem(nData, data.size(), move(nAttrib), move(nLabel), move(nType));
	auto nItem = std::make_shared<Item>(item, this->getObject().getConnection(), this->getObjectPath() + "/" + item->getId(), weak_from_this());
	updateItem(nItem);
	items.insert({item->getId(), nItem});
	ItemCreated(nItem->getPath());
	return std::tuple(nItem->getPath(), "/");
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
	backend->setLabel(value);
	backend->updateMetadata();
}

bool
Collection::Locked() {
	return std::any_of(items.cbegin(), items.cend(),
	                   [](const std::pair<std::string, std::shared_ptr<Item>> &entry) -> bool {
		                   return entry.second->Locked();
	                   });
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

std::vector<std::shared_ptr<Item>>
Collection::InternalSearchItems(const std::map<std::string, std::string> &attributes) {
	auto it = backend->searchItems(attributes);
	std::vector<std::shared_ptr<Item>> r;
	r.reserve(it.size());
	for (const auto &id : it) {
		r.push_back(items[id]);
	}
	return r;
}

std::map<std::string, std::shared_ptr<Item>> &
Collection::getItemMap() {
	return items;
}

void
Collection::DiscardObjects() {
	while (!discarded.empty()) {
		discarded.pop_back();
	}
}

void
Collection::InitItems() {
	for (const auto &item : backend->getItems()) {
		items.insert({
				             item->getId(), std::make_unique<Item>(item, this->getObject().getConnection(),
				                                                   this->getObjectPath() + "/" + item->getId(),
				                                                   weak_from_this())
		             });
	}
	updateAlias();
}



void
Collection::DiscardItem(std::string id) {
	auto ptr = items.extract(id).mapped();
	ItemDeleted(ptr->getPath());
	backend->RemoveItem(ptr->getBackend()->getId());
	discarded.push_back(move(ptr));
}

void
Collection::updateAlias() {
	std::string path = "";
	if (backend->getAlias().empty()) {
		if (proxy) {
			proxy.reset();
		}
	} else {
		path = "/org/freedesktop/secrets/aliases/" + backend->getAlias();
		proxy = std::make_unique<CollectionProxy>(this->getObject().getConnection(), path, weak_from_this());
	}
	for (const auto &item : items) {
		item.second->updateProxy(path);
	}
}

void
Collection::updateItem(std::shared_ptr<Item> item) {
	if (!backend->getAlias().empty()) {
		auto path = "/org/freedesktop/secrets/aliases/" + backend->getAlias();
		item->updateProxy(path);
	} else {
		item->updateProxy("");
	}
}

// TODO: do proxies have to use proxied item path?
void
Collection::ItemCreated(const sdbus::ObjectPath &item) {
	emitItemCreated(item);
	emitPropertiesChangedSignal("org.freedesktop.Secret.Collection", {"Items"});
	if (proxy) proxy->emitItemCreated(item);
}

void
Collection::ItemDeleted(const sdbus::ObjectPath &item) {
	emitItemDeleted(item);
	emitPropertiesChangedSignal("org.freedesktop.Secret.Collection", {"Items"});
	if (proxy) proxy->emitItemDeleted(item);
}

void
Collection::ItemChanged(const sdbus::ObjectPath &item) {
	emitItemChanged(item);
	emitPropertiesChangedSignal("org.freedesktop.Secret.Collection", {"Items"});
	if (proxy) proxy->emitItemChanged(item);
}

std::shared_ptr<SecretService>
Collection::GetService() {
	return parent.lock();
}
