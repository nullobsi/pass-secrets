//
// Created by nullobsi on 2021/03/17.
//

#include "SecretService.h"
#include "Session.h"
#include <nanoid/nanoid.h>
#include <utility>
#include <iostream>
#include <filesystem>

using namespace std;

SecretService::SecretService(sdbus::IConnection &conn,
                             std::string path) : AdaptorInterfaces(conn, std::move(path)), sessions(), collections(),
                                                 store() {

	for (auto c : store.GetCollections()) {
		auto id = c->getId();
		auto collection = make_unique<Collection>(c, this->getObject().getConnection(),
		                                          "/org/freedesktop/secrets/collection/" + id, this->weak_from_this());
		collections.insert({id, move(collection)});
	}

	registerAdaptor();
}

SecretService::~SecretService() {
	unregisterAdaptor();
}

std::tuple<sdbus::Variant, sdbus::ObjectPath>
SecretService::OpenSession(const std::string &algorithm,
                           const sdbus::Variant &input) {

	while (!discarded.empty()) {
		discarded.pop_back();
	}

	if (!algorithm.empty()) {
		throw sdbus::Error("org.freedesktop.DBus.Error.NotSupported", "Only plain is supported");
	}

	// TODO: find a way to track calling process
	auto session = make_unique<Session>(this->weak_from_this(), this->getObject().getConnection(),
	                                    "/org/freedesktop/secrets/session/" + nanoid::generate(
			                                    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890_"));
	auto sessionPath = session->getObjectPath();
	sessions.insert({sessionPath, move(session)});

	return {sdbus::Variant(""), sdbus::ObjectPath(sessionPath)};
}

void
SecretService::CloseSession(const std::string &path) {
	// TODO: better way to delete without segfault
	discarded.push_back(move(sessions.extract(path).mapped()));
}

std::tuple<sdbus::ObjectPath, sdbus::ObjectPath>
SecretService::CreateCollection(const std::map<std::string, sdbus::Variant> &properties,
                                const std::string &alias) {
	// TODO: Create collection
	return std::tuple<sdbus::ObjectPath, sdbus::ObjectPath>();
}

std::tuple<std::vector<sdbus::ObjectPath>, std::vector<sdbus::ObjectPath>>
SecretService::SearchItems(const std::map<std::string, std::string> &attributes) {
	// TODO: Search all collections
	vector<sdbus::ObjectPath> locked;
	vector<sdbus::ObjectPath> unlocked;
//	for (const auto &collection : collections) {
//		collection.second->SearchItems(attributes)
//	}
}

std::tuple<std::vector<sdbus::ObjectPath>, sdbus::ObjectPath>
SecretService::Unlock(const std::vector<sdbus::ObjectPath> &objects) {
	// TODO: Unlock
	return std::tuple<std::vector<sdbus::ObjectPath>, sdbus::ObjectPath>();
}

std::tuple<std::vector<sdbus::ObjectPath>, sdbus::ObjectPath>
SecretService::Lock(const std::vector<sdbus::ObjectPath> &objects) {
	// TODO: Lock
	return std::tuple<std::vector<sdbus::ObjectPath>, sdbus::ObjectPath>();
}

void
SecretService::LockService() {
	// TODO: Lock service
}

sdbus::ObjectPath
SecretService::ChangeLock(const sdbus::ObjectPath &collection) {
	// TODO: Change lock
	// this isn't mentioned in the specification (gnome keyring specific?)
	return sdbus::ObjectPath();
}

std::map<sdbus::ObjectPath, sdbus::Struct<sdbus::ObjectPath, std::vector<uint8_t>, std::vector<uint8_t>, std::string>>
SecretService::GetSecrets(const std::vector<sdbus::ObjectPath> &items,
                          const sdbus::ObjectPath &session) {
	// TODO: Get secrets
	return std::map<sdbus::ObjectPath, sdbus::Struct<sdbus::ObjectPath, std::vector<uint8_t>, std::vector<uint8_t>, std::string>>();
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
	// TODO: Set alias
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
