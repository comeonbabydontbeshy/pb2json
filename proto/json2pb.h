#pragma once

#include <stdint.h>
#include <stdexcept>
#include "google/protobuf/message.h"
#include "google/protobuf/util/json_util.h"

class CJson2PbException:public std::runtime_error
{
public:
	CJson2PbException(uint32_t ulErrCode,const std::string& strErrMsg):
		std::runtime_error(strErrMsg), m_ulErrCode(ulErrCode){}
private:
	uint32_t m_ulErrCode;
};

class CJson2Pb
{
public:
	static void ParsePb2Json(const google::protobuf::Message& protobufMsg, std::string& strJson);

	static void ParseJson2Pb(const std::string& strJson, google::protobuf::Message& protobufMsg);
};