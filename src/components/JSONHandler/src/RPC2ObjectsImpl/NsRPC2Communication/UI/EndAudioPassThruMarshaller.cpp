#include "../src/../include/JSONHandler/RPC2Objects/NsRPC2Communication/UI/EndAudioPassThru.h"
#include "../src/ALRPCObjectsImpl/V1/ResultMarshaller.h"
#include "../src/../src/RPC2ObjectsImpl//NsRPC2Communication/UI/EndAudioPassThruMarshaller.h"

/*
  interface	NsRPC2Communication::UI
  version	1.2
  generated at	Fri Dec 14 06:14:25 2012
  source stamp	Fri Dec 14 06:14:23 2012
  author	robok0der
*/

using namespace NsRPC2Communication::UI;

bool EndAudioPassThruMarshaller::checkIntegrity(EndAudioPassThru& s)
{
  return checkIntegrityConst(s);
}


bool EndAudioPassThruMarshaller::fromString(const std::string& s,EndAudioPassThru& e)
{
  try
  {
    Json::Reader reader;
    Json::Value json;
    if(!reader.parse(s,json,false))  return false;
    if(!fromJSON(json,e))  return false;
  }
  catch(...)
  {
    return false;
  }
  return true;
}


const std::string EndAudioPassThruMarshaller::toString(const EndAudioPassThru& e)
{
  Json::FastWriter writer;
  return checkIntegrityConst(e) ? writer.write(toJSON(e)) : "";
}


bool EndAudioPassThruMarshaller::checkIntegrityConst(const EndAudioPassThru& s)
{
  return true;
}


Json::Value EndAudioPassThruMarshaller::toJSON(const EndAudioPassThru& e)
{
  Json::Value json(Json::objectValue);
  if(!checkIntegrityConst(e))
    return Json::Value(Json::nullValue);

  json["jsonrpc"]=Json::Value("2.0");
  json["method"]=Json::Value("UI.EndAudioPassThru");

  json["id"]=Json::Value(e.getId());
  json["params"]=Json::Value(Json::objectValue);
  json["params"]["appId"]=Json::Value(e.appId);;
  return json;
}


bool EndAudioPassThruMarshaller::fromJSON(const Json::Value& json,EndAudioPassThru& c)
{
  try
  {
    if(!json.isObject())  return false;
    if(!json.isMember("jsonrpc") || !json["jsonrpc"].isString() || json["jsonrpc"].asString().compare("2.0"))  return false;
    if(!json.isMember("method") || !json["method"].isString() || json["method"].asString().compare("UI.EndAudioPassThru"))  return false;
    if(!json.isMember("id") || !json["id"].isInt()) return false;
    c.setId(json["id"].asInt());

    if(!json.isMember("params")) return false;

    Json::Value js=json["params"];
    if(!js.isObject())  return false;
    if(!js.isMember("appId") || !js["appId"].isInt())  return false;
    c.appId=js["appId"].asInt();

  }
  catch(...)
  {
    return false;
  }
  return checkIntegrity(c);
}
