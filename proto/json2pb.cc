#include "json2pb.h"
#include "jsoncpp/json/json.h"

static void ParseField2Json(const google::protobuf::Message& protobufMsg,
	const google::protobuf::FieldDescriptor& field, int32_t lIndex, Json::Value& stJsonObj);
static void ParsePb2JsonObj(const google::protobuf::Message& protobufMsg, Json::Value& stJsonObj);

void ParseField2Json(const google::protobuf::Message& protobufMsg,
	const google::protobuf::FieldDescriptor& field, int32_t lIndex, Json::Value& stJsonObj)
{
	const google::protobuf::Reflection *pstReflection = protobufMsg.GetReflection();

	std::cout << field.name() << std::endl;

/* =======设置json的value======= */
#define JSON_PUT_VAL(type,grfunc,gfunc) case type:{if (field.is_repeated()){\
stJsonObj[field.name()].append(pstReflection->grfunc(protobufMsg, &field, lIndex));}else{\
stJsonObj[field.name()] = pstReflection->gfunc(protobufMsg, &field);}break;}
/* =======设置json的value======= */

	switch (field.cpp_type())
	{
		JSON_PUT_VAL(google::protobuf::FieldDescriptor::CPPTYPE_INT32, GetRepeatedInt32, GetInt32)
		JSON_PUT_VAL(google::protobuf::FieldDescriptor::CPPTYPE_UINT32, GetRepeatedUInt32, GetUInt32)
		//JSON_PUT_VAL(google::protobuf::FieldDescriptor::CPPTYPE_INT64, GetRepeatedInt64, GetInt64)
		//JSON_PUT_VAL(google::protobuf::FieldDescriptor::CPPTYPE_UINT64, GetRepeatedUInt64, GetUInt64)
		JSON_PUT_VAL(google::protobuf::FieldDescriptor::CPPTYPE_FLOAT, GetRepeatedFloat, GetFloat)
		JSON_PUT_VAL(google::protobuf::FieldDescriptor::CPPTYPE_DOUBLE, GetRepeatedDouble, GetDouble)
		JSON_PUT_VAL(google::protobuf::FieldDescriptor::CPPTYPE_BOOL, GetRepeatedBool, GetBool)
		JSON_PUT_VAL(google::protobuf::FieldDescriptor::CPPTYPE_STRING, GetRepeatedString, GetString)
		case google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE:
		{
			Json::Value stJsonObjItem;
			if (field.is_repeated())
			{
				const google::protobuf::Message &stMessage =
					pstReflection->GetRepeatedMessage(protobufMsg, &field, lIndex);
				ParsePb2JsonObj(stMessage, stJsonObjItem);
				stJsonObj[field.name()].append(stJsonObjItem);
			}
			else
			{
				const google::protobuf::Message &stMessage
					= pstReflection->GetMessage(protobufMsg, &field);
				ParsePb2JsonObj(stMessage, stJsonObjItem);
				stJsonObj[field.name()] = stJsonObjItem;
			}
			break;
		}
	default:
		break;
	}
}

void ParsePb2JsonObj(const google::protobuf::Message& protobufMsg, Json::Value& stJsonObj)
{
	const google::protobuf::Descriptor *pstDescriptor = protobufMsg.GetDescriptor();
	const google::protobuf::Reflection *pstReflection = protobufMsg.GetReflection();

	std::vector<const google::protobuf::FieldDescriptor*> fields;
	pstReflection->ListFields(protobufMsg, &fields);

	for (const auto& field : fields)
	{
		if (field->is_repeated())
		{
			int32_t lSize = pstReflection->FieldSize(protobufMsg, field);
			if (0==lSize)
			{
				stJsonObj[field->name()] = nullptr;
				continue;
			}
			for (size_t i = 0; i < lSize; i++)
			{
				ParseField2Json(protobufMsg, *field, i, stJsonObj);
			}
		}
		else
		{
			ParseField2Json(protobufMsg, *field, 0, stJsonObj);
		}
	}
}

void CJson2Pb::ParsePb2Json(const google::protobuf::Message& protobufMsg, std::string& strJson)
{
	Json::Value stJsonRoot;

	ParsePb2JsonObj(protobufMsg, stJsonRoot);

	Json::StyledWriter style_writer;
	strJson = style_writer.write(stJsonRoot);

	return;
}

static void ParseJsonObj2Pb(const Json::Value& stJsonObj, google::protobuf::Message& protobufMsg);
static void ParseJsonObj2Field(const Json::Value& stJsonObj,
	const google::protobuf::FieldDescriptor& fieldDescriptor,
	google::protobuf::Message& protobufMsg);

#define JSONCPP_IS_XX(json,key,valtype) if(!json.isMember(key)){ throw CJson2PbException(1,key+std::string{" is not exist"});}\
if(json[key].type()!=valtype){throw CJson2PbException{1,key+std::string{" is not "}+std::string{#valtype}};}
#define JSONCPP_IS_INT32(json,key) JSONCPP_IS_XX(json,key,Json::intValue)
#define JSONCPP_IS_UINT32(json,key) JSONCPP_IS_XX(json,key,Json::uintValue)
#define JSONCPP_IS_DOUBLE(json,key) JSONCPP_IS_XX(json,key,Json::realValue)
#define JSONCPP_IS_STRING(json,key) JSONCPP_IS_XX(json,key,Json::stringValue)
#define JSONCPP_IS_BOOL(json,key) JSONCPP_IS_XX(json,key,Json::booleanValue)
#define JSONCPP_IS_ARRAY(json,key) JSONCPP_IS_XX(json,key,Json::arrayValue)
#define JSONCPP_IS_OBJECT(json,key) JSONCPP_IS_XX(json,key,Json::objectValue)

void ParseJsonObj2Field(const Json::Value& stJsonObj,
	const google::protobuf::FieldDescriptor& fieldDescriptor,
	google::protobuf::Message& protobufMsg)
{
	const google::protobuf::Reflection *pstReflection = protobufMsg.GetReflection();

/* ==========从json中获取对应值========== */
#define JSON_GET_VAL(fmt,gtype,jsonfunc,afunc,sfunc) case gtype:{if (fieldDescriptor.is_repeated()){\
if (stJsonObj.type()!=fmt){throw CJson2PbException(1,fieldDescriptor.name()+std::string{" is not "}+std::string{#fmt});}\
pstReflection->afunc(&protobufMsg, &fieldDescriptor, stJsonObj.jsonfunc());}else{\
JSONCPP_IS_XX(stJsonObj, fieldDescriptor.name(),fmt);\
pstReflection->sfunc(&protobufMsg, &fieldDescriptor, stJsonObj[fieldDescriptor.name()].jsonfunc());}break;}
/* ==========从json中获取对应值========== */

	switch (fieldDescriptor.cpp_type())
	{
		JSON_GET_VAL(Json::intValue, google::protobuf::FieldDescriptor::CPPTYPE_INT32, asInt, AddInt32, SetInt32);
		JSON_GET_VAL(Json::uintValue, google::protobuf::FieldDescriptor::CPPTYPE_UINT32, asUInt, AddUInt32, SetUInt32);
		JSON_GET_VAL(Json::realValue, google::protobuf::FieldDescriptor::CPPTYPE_FLOAT, asFloat, AddFloat, SetFloat);
		JSON_GET_VAL(Json::realValue, google::protobuf::FieldDescriptor::CPPTYPE_DOUBLE, asDouble, AddDouble, SetDouble);
		JSON_GET_VAL(Json::booleanValue, google::protobuf::FieldDescriptor::CPPTYPE_BOOL, asBool, AddBool, SetBool);
		JSON_GET_VAL(Json::stringValue, google::protobuf::FieldDescriptor::CPPTYPE_STRING, asString, AddString, SetString);
		case  google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE:
		{
			google::protobuf::Message *pstMessage = nullptr;
			if (fieldDescriptor.is_repeated())
			{
				pstMessage = pstReflection->AddMessage(&protobufMsg, &fieldDescriptor);
				if (stJsonObj.type()!=Json::objectValue)
				{
					throw CJson2PbException(1, fieldDescriptor.name() + std::string{ " is not object" });
				}
				ParseJsonObj2Pb(stJsonObj, *pstMessage);
			}
			else
			{
				JSONCPP_IS_OBJECT(stJsonObj, fieldDescriptor.name());
				pstMessage = pstReflection->MutableMessage(&protobufMsg, &fieldDescriptor);
				ParseJsonObj2Pb(stJsonObj[fieldDescriptor.name()], *pstMessage);
			}
			
			break;
		}
	default:
	{
		break;
	}
	}
}


void ParseJsonObj2Pb(const Json::Value& stJsonObj, google::protobuf::Message& protobufMsg)
{
	const google::protobuf::Descriptor *pstDescriptor = protobufMsg.GetDescriptor();
	int32_t lfieldCnt = pstDescriptor->field_count();
	for (int32_t lIdx = 0; lIdx < lfieldCnt; lIdx++)
	{
		const google::protobuf::FieldDescriptor *pstFieldDescriptor = pstDescriptor->field(lIdx);
		if (pstFieldDescriptor->is_repeated())
		{
			JSONCPP_IS_ARRAY(stJsonObj, pstFieldDescriptor->name());
			for (int32_t i; i < stJsonObj[pstFieldDescriptor->name()].size(); i++)
			{
				ParseJsonObj2Field(stJsonObj[pstFieldDescriptor->name()][i], *pstFieldDescriptor, protobufMsg);
			}
		}
		else
		{
			ParseJsonObj2Field(stJsonObj, *pstFieldDescriptor, protobufMsg);
		}
	}
}

void CJson2Pb::ParseJson2Pb(const std::string& strJson, google::protobuf::Message& protobufMsg)
{
	Json::Value stJsonObj;
	Json::Reader reader;
	if (!reader.parse(strJson, stJsonObj))
	{
		throw CJson2PbException(-1, "parse json root occur err");
	}

	ParseJsonObj2Pb(stJsonObj, protobufMsg);

	return;
}

