//
// Created by nullobsi on 2021/03/17.
//

#ifndef PASS_FDO_SECRETS_PASSSTORE_H
#define PASS_FDO_SECRETS_PASSSTORE_H

#include <string>
#include <filesystem>
#include <rapidjson/document.h>
#include <fstream>
#include <map>
#include <vector>
#include "PassCollection.h"

class PassStore {
public:
	PassStore();

	void
	CreateCollection(const std::string &label,
	                 const std::string &name);

	std::vector<std::shared_ptr<PassCollection>>
	GetCollections();

private:
	void
	createDefaultCollection();

	// password store location
	std::filesystem::path storePrefix;

	void
	addExistingCollection(std::filesystem::path location);

	// collections
	std::map<std::string, std::shared_ptr<PassCollection>> collections;


};


#endif //PASS_FDO_SECRETS_PASSSTORE_H
