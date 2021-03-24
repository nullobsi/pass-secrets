//
// Created by nullobsi on 2021/03/17.
//

#include "PassStore.h"
#include "DocumentHelper.h"
#include <filesystem>
#include <iostream>
#include <nanoid/nanoid.h>

using namespace std;

PassStore::PassStore() {
	auto storeLoc = getenv("PASSWORD_STORE_DIR");
	auto homeDir = getenv("HOME");
	if (storeLoc) {
		storePrefix = string(storeLoc);
		storePrefix /= "secretservice";
	} else if (homeDir) {
		storePrefix = string(homeDir);
		storePrefix /= ".password-store/secretservice";
	} else {
		throw runtime_error("Could not find password store! Please set $HOME or $PASSWORD_STORE_DIR");
	}

	if (!filesystem::exists(storePrefix)) {
		filesystem::create_directory(storePrefix);
	}

	for (auto &entry : filesystem::directory_iterator(storePrefix)) {
		if (entry.is_directory()) {
			if (filesystem::exists(entry.path() / "collection.json")) {
				try {
					auto collection = make_shared<PassCollection>(entry.path());
					collections.insert({collection->getId(), collection});
					cout << "Loaded collection " + entry.path().generic_string() << endl;
				} catch (std::runtime_error &e) {
					cerr << e.what() << endl;
				}
			}
		}
	}

	if (!filesystem::exists(storePrefix / "default" / "collection.json")) {
		createDefaultCollection();
	}
}

void
PassStore::createDefaultCollection() {
	if (!filesystem::exists(storePrefix / "default"))
		filesystem::create_directory(storePrefix / "default");

	CreateCollection("Default Keyring", "default");
}

std::shared_ptr<PassCollection>
PassStore::CreateCollection(const std::string &label,
                            const std::string &alias) {
	using namespace rapidjson;
	Document d;
	d.SetObject();

	// label, created date, object path ID
	auto created = std::time(nullptr);
	auto id = nanoid::generate();

	d.AddMember("label", label, d.GetAllocator());
	d.AddMember("created", created, d.GetAllocator());
	d.AddMember("id", id, d.GetAllocator());
	d.AddMember("alias", alias, d.GetAllocator());

	filesystem::path location = storePrefix / id;
	fstream metadataFile;
	metadataFile.open(location / "collection.json", ios::out | ios::trunc);
	DHelper::WriteDocument(d, metadataFile);
	metadataFile.close();

	auto c = make_shared<PassCollection>(location, label, id, created, alias);
	collections.insert({id, c});
	return c;
}

std::vector<std::shared_ptr<PassCollection>>
PassStore::GetCollections() {
	vector<shared_ptr<PassCollection>> rtn;
	for (const auto &entry : collections) {
		rtn.push_back(entry.second);
	}

	return rtn;
}



