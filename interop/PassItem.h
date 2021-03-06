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
	PassItem(std::filesystem::path location_, std::string label_, uint64_t created_, std::string id_, std::map<std::string, std::string> attrib_, std::string type_, uint8_t *secret_, size_t secretLen_, uint64_t modified_);

	~PassItem();

	uint64_t
	getCreated();

	uint64_t
	getModified();

	std::map<std::string, std::string>
	getAttrib();

	std::string
	getLabel();

	std::string
	getId();

	std::string
	getType();

	bool
	hasAttrib(const std::string &key,
	          const std::string &val);

	bool
	isUnlocked();

	bool
	unlock();

	void
	lock();

	size_t
	getSecretLength();

	uint8_t *
	getSecret();

	void
	setLabel(std::string n);

	void
	setAttrib(std::map<std::string, std::string> n);

	void
	setType(std::string n);

	void
	setSecret(uint8_t *data, size_t n);

	void
	updateMetadata();

	void
	Delete();


private:
	std::filesystem::path location;

	// info
	std::map<std::string, std::string> attrib;
	uint64_t created;
	uint64_t modified;
	std::string label;
	std::string id;
	std::string type;

	size_t secretLength = 0;
	uint8_t *secret = nullptr;

};


#endif //PASS_FDO_SECRETS_PASSITEM_H
