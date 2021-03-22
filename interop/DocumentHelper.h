//
// Created by nullobsi on 2021/03/17.
//

#ifndef PASS_FDO_SECRETS_DOCUMENTHELPER_H
#define PASS_FDO_SECRETS_DOCUMENTHELPER_H
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/ostreamwrapper.h"
#include "rapidjson/istreamwrapper.h"
#include "iostream"
#include "sdbus-c++/sdbus-c++.h"


namespace DHelper {
	using namespace std;
	using namespace rapidjson;
	// documents
	void
	WriteDocument(const Document& d,
	                         ostream &f);

	Document
	ReadDocument(istream &f);


	Value
	SerializeAttrib(map<string,string> &dict, MemoryPoolAllocator<CrtAllocator> &alloc);

	map<string, string> ReadAttrib(Value &d);
}

#endif //PASS_FDO_SECRETS_DOCUMENTHELPER_H
