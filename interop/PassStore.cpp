//
// Created by nullobsi on 2021/03/17.
//

#include "PassStore.h"
#include "DocumentHelper.h"
#include <filesystem>
#include <iostream>
#include <sstream>
#include <nanoid/nanoid.h>

using namespace std;

string PassStore::passLocation;

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
		try {
			filesystem::create_directory(storePrefix);
		} catch (std::filesystem::filesystem_error &e) {
			throw runtime_error("Could not create directory " + storePrefix.string());
		}
	}

	bool hasDefault = false;

	for (auto &entry : filesystem::directory_iterator(storePrefix)) {
		if (entry.is_directory()) {
			if (filesystem::exists(entry.path() / "collection.json")) {
				try {
					auto collection = make_shared<PassCollection>(entry.path());
					collections.insert({collection->getId(), collection});
					cout << "Loaded collection " + entry.path().generic_string() << endl;
					if (collection->getAlias() == "default") hasDefault = true;
				} catch (std::runtime_error &e) {
					cerr << e.what() << endl;
				}
			}
		}
	}
	if (!hasDefault) createDefaultCollection();

	if (passLocation.empty()) {
		namespace fs = std::filesystem;
		stringstream path = stringstream(getenv("PATH"));
		vector<string> pathEntries;
		string token;
		while (getline(path, token, ':')) {
			pathEntries.push_back(token);
		}
		for (const auto &dirName : pathEntries) {
			fs::directory_iterator j;
			// Handle the edge-case where a directory in
			// $PATH doesn't exist.
			try {
				j = fs::directory_iterator(dirName);
			} catch (fs::filesystem_error &e) {
				std::cerr << "Path directory " << dirName
					  << " doesn't exist, skipping" << std::endl;
				continue;
			}
			fs::directory_iterator i(dirName);
			for (const auto &file : i) {
				if (file.is_regular_file() && file.path().filename() == "pass") {
					std::cout << "Found pass at " + file.path().string() << std::endl;
					passLocation = file.path().string();
					goto finish;
				}
			}
		}
		throw std::runtime_error("Pass not found in path!");
		finish:
		return;
	}
}

void
PassStore::createDefaultCollection() {
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
	if (!filesystem::exists(location)) {
		filesystem::create_directory(location);
	}
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



