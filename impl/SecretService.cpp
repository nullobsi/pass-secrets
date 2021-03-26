//
// Created by nullobsi on 2021/03/17.
//

#include "SecretService.h"
#include "Session.h"
#include <nanoid/nanoid.h>
#include <utility>
#include <iostream>
#include <filesystem>
#include "Item.h"
#include "sessions/sessions.h"

using namespace std;

SecretService::SecretService(sdbus::IConnection &conn,
                             std::string path) : AdaptorInterfaces(conn, std::move(path)) {

	registerAdaptor();
}

SecretService::~SecretService() {
	unregisterAdaptor();
}

std::tuple<sdbus::Variant, sdbus::ObjectPath>
SecretService::OpenSession(const std::string &algorithm,
                           const sdbus::Variant &input) {
	unique_ptr<Session> session;
	auto ptr = weak_from_this();
	auto &conn = this->getObject().getConnection();
	auto path = "/org/freedesktop/secrets/session/" + nanoid::generate();
	sdbus::Variant rtn;
	if (algorithm.empty() || algorithm == "plain") {
		session = make_unique<Sessions::Plain>(ptr, conn, path);
		rtn = session->getOutput();
	} else {
		throw sdbus::Error("org.freedesktop.DBus.Error.NotSupported", "Only plain is supported");
	}

	sessions.insert({path, move(session)});

	return {rtn, path};
}

std::tuple<sdbus::ObjectPath, sdbus::ObjectPath>
SecretService::CreateCollection(const std::map<std::string, sdbus::Variant> &properties,
                                const std::string &alias) {
	auto nLabel = properties.count("org.freedesktop.Secret.Collection.Label") && properties.at("org.freedesktop.Secret.Collection.Label").containsValueOfType<std::string>() ? properties.at("org.freedesktop.Secret.Collection.Label").get<std::string>() : "Collection";
	auto nCollection = store.CreateCollection(nLabel, alias);
	auto coll = make_shared<Collection>(nCollection, this->getObject().getConnection(), "/org/freedesktop/secrets/collection/" + nCollection->getId(), weak_from_this());
	collections.insert({nCollection->getId(), coll});
	return std::tuple(coll->getObjectPath(), "/");
}

std::tuple<std::vector<sdbus::ObjectPath>, std::vector<sdbus::ObjectPath>>
SecretService::SearchItems(const std::map<std::string, std::string> &attributes) {
	vector<sdbus::ObjectPath> locked;
	vector<sdbus::ObjectPath> unlocked;
	for (const auto &collection : collections) {
		vector<shared_ptr<Item>> items = collection.second->InternalSearchItems(attributes);
		for (const auto &item : items) {
			if (item->Locked()) {
				locked.push_back(item->getPath());
			} else {
				unlocked.push_back(item->getPath());
			}
		}
	}
	return std::tuple(unlocked, locked);
}


std::tuple<std::vector<sdbus::ObjectPath>, sdbus::ObjectPath>
SecretService::Unlock(const std::vector<sdbus::ObjectPath> &objects) {
	// TODO: Prompt?
	std::vector<sdbus::ObjectPath> unlocked;
	for (const auto &item : fromObjectPath((const vector<string> &)objects)) {
		if (!item->Locked() || item->getBackend()->unlock()) {
			item->emitPropertiesChangedSignal("org.freedesktop.Secret.Item", {"Locked"});
			collections[item->getCollectionId()]->emitPropertiesChangedSignal("org.freedesktop.Secret.Collection", {"Locked"});
			unlocked.push_back(item->getPath());
		}
	}

	return std::tuple(unlocked, "/");
}

std::tuple<std::vector<sdbus::ObjectPath>, sdbus::ObjectPath>
SecretService::Lock(const std::vector<sdbus::ObjectPath> &objects) {
	// TODO: Prompt?
	std::vector<sdbus::ObjectPath> locked;
	for (const auto &item : fromObjectPath((const vector<string> &)objects)) {
		// TODO: could this cause a segfault?
		item->getBackend()->lock();
		item->emitPropertiesChangedSignal("org.freedesktop.Secret.Item", {"Locked"});
		collections[item->getCollectionId()]->emitPropertiesChangedSignal("org.freedesktop.Secret.Collection", {"Locked"});
		locked.push_back(item->getPath());
	}
	return std::tuple(locked, "/");
}

void
SecretService::LockService() {
	for (const auto &entry : collections) {
		for (const auto &item : entry.second->GetBacking()->getItems()) {
			item->lock();
		}
	}
}

sdbus::ObjectPath
SecretService::ChangeLock(const sdbus::ObjectPath &collection) {
	// TODO: Change lock
	// this isn't mentioned in the specification (gnome keyring specific?)
	return sdbus::ObjectPath("/");
}

std::map<sdbus::ObjectPath, sdbus::Struct<sdbus::ObjectPath, std::vector<uint8_t>, std::vector<uint8_t>, std::string>>
SecretService::GetSecrets(const std::vector<sdbus::ObjectPath> &items,
                          const sdbus::ObjectPath &session) {
	auto it = fromObjectPath((const vector<string> &)items);
	std::map<sdbus::ObjectPath, sdbus::Struct<sdbus::ObjectPath, std::vector<uint8_t>, std::vector<uint8_t>, std::string>> retn;
	for (const auto &item : it) {
		auto res = item->GetSecret(session);
		retn.insert({item->getPath(), res});
	}
	return retn;
}

sdbus::ObjectPath
SecretService::ReadAlias(const std::string &name) {
	for (auto &entry : collections) {
		auto alias = entry.second->GetBacking()->getAlias();
		if (alias == name) {
			return sdbus::ObjectPath(entry.second->getObjectPath());
		}
	}
	return sdbus::ObjectPath("/");
}

void
SecretService::SetAlias(const std::string &name,
                        const sdbus::ObjectPath &collection) {
	std::filesystem::path objPath((string)collection);
	auto collId = objPath.filename().generic_string();
	if (!collections.count(collId)) {
		throw sdbus::Error("org.freedesktop.Secret.Error.NoSuchObject", "No such item or collection exists.");
	}
	collections[collId]->GetBacking()->setAlias(name);
	collections[collId]->GetBacking()->updateMetadata();
	collections[collId]->updateAlias();
}

std::vector<sdbus::ObjectPath>
SecretService::Collections() {
	std::vector<sdbus::ObjectPath> cs;
	cs.reserve(collections.size());

	for (auto &entry : collections) {
		cs.emplace_back(entry.second->getObjectPath());
	}

	return cs;
}

std::vector<std::shared_ptr<Item>>
SecretService::fromObjectPath(const std::vector<std::string> &paths) {
	std::vector<std::shared_ptr<Item>> items;
	for (const auto &objectPath : paths) {
		auto oPath = std::filesystem::path((string)objectPath);
		if (oPath.has_parent_path()) {
			if (oPath.parent_path().filename() == "collection") { // Find all items
				auto collId = oPath.filename().generic_string();
				if (collections.count(collId)) {
					for (const auto &item : collections[collId]->getItemMap()) {
						items.push_back(item.second);
					}
				}
			} else { // this should be a normal item
				// obtain ids from path
				auto itemId = oPath.filename().generic_string();
				auto collId = oPath.parent_path().filename().generic_string();
				// if exists
				if (collections.count(collId) && collections[collId]->getItemMap().count(itemId)) {
					auto item = collections[collId]->getItemMap()[itemId];
					items.push_back(item);
				}
			}
		}
	}
	return items;
}

void
SecretService::DiscardSession(const std::string &path) {
	discardedSessions.push_back(move(sessions.extract(path).mapped()));
}

void
SecretService::DiscardCollection(std::string id) {
	discardedCollections.push_back(move(collections.extract(id).mapped()));
}

void
SecretService::DiscardObjects() {
	while (!discardedSessions.empty()) {
		discardedSessions.pop_back();
	}
	while (!discardedCollections.empty()) {
		discardedCollections.pop_back();
	}
	for (const auto &entry : collections) {
		entry.second->DiscardObjects();
	}
}

void
SecretService::InitCollections() {
	for (auto c : store.GetCollections()) {
		auto id = c->getId();
		auto collection = make_shared<Collection>(c, this->getObject().getConnection(),
		                                          "/org/freedesktop/secrets/collection/" + id, this->weak_from_this());
		collection->InitItems();
		collections.insert({id, move(collection)});
	}
}

std::pair<std::vector<uint8_t>, std::vector<uint8_t>>
SecretService::EncryptSecret(const string &path, uint8_t *data, size_t len) {
	return std::move(sessions[path]->encryptSecret(data, len));
}


