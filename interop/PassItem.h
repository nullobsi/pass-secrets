//
// Created by nullobsi on 2021/03/17.
//

#ifndef PASS_FDO_SECRETS_PASSITEM_H
#define PASS_FDO_SECRETS_PASSITEM_H
#include <filesystem>
#include <map>

class PassItem {
public:
	explicit PassItem(std::filesystem::path location_);
	~PassItem();

	uint64_t getCreated();
	std::map<std::string, std::string> getAttrib();
	std::string getLabel();
	std::string getId();
	std::string getType();

	bool hasAttrib(const std::string& key, const std::string& val);

	bool isUnlocked();
	bool unlock();
	void lock();

	size_t getSecretLength();
	uint8_t *getSecret();

	// TODO: Set attrib
	// TODO: Set label
	// TODO: Set type
	// TODO: Allow deletion


private:
	std::filesystem::path location;

	// info
	std::map<std::string, std::string> attrib;
	uint64_t created;
	std::string label;
	std::string id;
	std::string type;

	size_t secretLength = 0;
	uint8_t *secret;

};


#endif //PASS_FDO_SECRETS_PASSITEM_H
