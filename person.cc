#include "person.pb.h"
#include "proto/json2pb.h"

int main()
{
	Person p;
	p.set_age(20);
	p.set_name("mark");
	p.set_weight(131.4);
	Address* a = p.mutable_address();
	a->set_plate_number(3);
	a->set_street("wenchanglu");

	std::string strJson;
	CJson2Pb::ParsePb2Json(p, strJson);
	std::cout << strJson << std::endl;

	std::cout << "=======================" << std::endl;

	Person p2;
	CJson2Pb::ParseJson2Pb(strJson, p2);
	std::string strJson2;
	CJson2Pb::ParsePb2Json(p2, strJson2);
	std::cout << strJson2 << std::endl;
	return 1;
}