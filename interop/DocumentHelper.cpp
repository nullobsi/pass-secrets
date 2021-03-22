//
// Created by nullobsi on 2021/03/17.
//

#include "DocumentHelper.h"

using namespace std;
using namespace rapidjson;

namespace DHelper {
	// documents
	void
	WriteDocument(const Document& d,
	              ostream &f) {

		OStreamWrapper wrapper(f);
		Writer<OStreamWrapper> writer(wrapper);
		d.Accept(writer);
		writer.Flush();
	}

	Document
	ReadDocument(istream &f) {
		IStreamWrapper wrapper(f);
		Document d;
		d.ParseStream(wrapper);
		return d;
	}


	Value SerializeAttrib(map<string, string> &dict, MemoryPoolAllocator<CrtAllocator> &alloc) {
		Value d;
		d.SetObject();
		for (auto &entry : dict) {
			d.AddMember(GenericStringRef(entry.first.c_str()), entry.second, alloc);
		}
		return d;
	}

	map<string, string>
	ReadAttrib(Value &d) {
		if (!d.IsObject()) {
			throw std::runtime_error("Invalid value!");
		}

		map<string, string> rtn;

		for (auto &entry : d.GetObject()) {
			rtn.insert(entry.name.GetString(), entry.value.GetString());
		}

		return rtn;
	}


}