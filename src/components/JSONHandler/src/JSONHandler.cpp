
#include "JSONHandler/JSONHandler.h"
#include <algorithm>
#include <string.h>
#include <json/reader.h>
#include <json/writer.h>
#include "JSONHandler/RegisterAppInterfaceResponse.h"


JSONHandler::JSONHandler( AxisCore::ProtocolHandler * protocolHandler )
:mProtocolHandler( protocolHandler )
{
    mFactory = new MobileRPCFactory();
}
    
JSONHandler::~JSONHandler()
{
    mProtocolHandler = 0;
    delete mFactory;
    mFactory = 0;
}

/*Methods from IProtocolObserver*/
void JSONHandler::setProtocolHandler( AxisCore::ProtocolHandler * protocolHandler )
{
    mProtocolHandler = protocolHandler;
}
void JSONHandler::sessionStartedCallback(const UInt8 sessionID)
{}

void JSONHandler::sessionEndedCallback(const UInt8 sessionID)
{}

void JSONHandler::dataReceivedCallback(const UInt8 sessionID, const UInt32 messageID, const UInt32 dataSize)
{
    UInt8 *data = new UInt8[dataSize];

    mProtocolHandler -> receiveData(sessionID, messageID, AxisCore::SERVICE_TYPE_RPC, dataSize, data);

    char * input = new char[dataSize];
    for ( int i = 0; i < dataSize; ++i )
    {
        if ( !isspace(data[i]) ) 
        {
            input[i] = data[i];
        }
    }
    
    mCurrentMessage = createObjectFromJSON( std::string( input, dataSize - 1 ) );

    RegisterAppInterfaceResponse response = mFactory -> createRegisterAppInterfaceResponse( *mCurrentMessage );
    Json::Value parameters = mFactory -> serializeRegisterAppInterfaceResponse( response );

    Json::Value root = createJSONFromObject( response );
    if ( root.isNull() )
    {
        return;
    }

    root["parameters"] = parameters;
    std::string responseString = jsonToString( root );
    UInt8* pData;
    pData = new UInt8[responseString.length() + 1];
    memcpy (pData, responseString.c_str(), responseString.length() + 1);
    mProtocolHandler -> sendData(sessionID,  AxisCore::SERVICE_TYPE_RPC, 
        responseString.size() + 1, pData, false);

    // OnHMIStatus
    OnHMIStatus notification 	= mFactory->createOnHMIStatus();
    Json::Value parameters1 	= mFactory->serializeOnHMIStatus( notification );

    Json::Value root1 = createJSONFromObject( notification );
    if ( root1.isNull() )       	
    {
        return;
    }

    root1["parameters"] = parameters1;
    std::string notificationString = jsonToString( root1 );
    UInt8* pData1;
    pData1 = new UInt8[notificationString.length() + 1];
    memcpy (pData1, notificationString.c_str(), notificationString.length() + 1);
    mProtocolHandler -> sendData(sessionID,  AxisCore::SERVICE_TYPE_RPC, 
        notificationString.size() + 1, pData1, false);

}
/*end of methods from IProtocolObserver*/

MobileRPCMessage * JSONHandler::createObjectFromJSON( const std::string & jsonString )
{
    Json::Value root;   
    Json::Reader reader;
    bool parsingSuccessful = reader.parse( jsonString, root );
    if ( !parsingSuccessful ) 
    {
        return 0;
    }

    MobileRPCMessage * result = 0;    
    unsigned int protocolVersion = 0;

    result = checkMessageTypeForProtocol1( root );
    if ( result )
    {        
        return result;
    } 
    else 
    {
        return 0; //version 2 or higher
    }
}

std::string JSONHandler::serializeObjectToJSON( const MobileRPCMessage & mobileRPCObject )
{
    if ( mobileRPCObject.getProtocolVersion() == 1 )
    {
        return serializeObjectToJSONProtocol1( mobileRPCObject );
    }
}

std::string JSONHandler::serializeObjectToJSONProtocol1 ( const MobileRPCMessage & mobileRPCObject )
{
    Json::Value root = createJSONFromObject( mobileRPCObject );
    return jsonToString( root );
}

Json::Value JSONHandler::createJSONFromObject ( const MobileRPCMessage & mobileRPCObject )
{
    Json::Value root;
    std::string messageType;
    switch( mobileRPCObject.getMessageType() ) {
        case 0:
            messageType = "request";
            break;
        case 1:
            messageType = "response";
            break;
        case 2:
            messageType = "notification";
            break;
        default:
            messageType = "";
    }
    
    Json::Value typeValue;
    
    if ( mobileRPCObject.getCorrelationID() != 0 )
    {
        typeValue["correlationID"] = mobileRPCObject.getCorrelationID();
    }
    if ( !mobileRPCObject.getFunctionName().empty() )
    {
        typeValue["name"] = mobileRPCObject.getFunctionName();
    }
    root[messageType] = typeValue;
    return root;
}

std::string JSONHandler::jsonToString( const Json::Value & jsonObject )
{
    Json::FastWriter writer;
    std::string root_to_print = writer.write( jsonObject );
    return root_to_print;
}

MobileRPCMessage * JSONHandler::checkMessageTypeForProtocol1 ( const Json::Value & root )
{
    if ( !root["request"].isNull() )
    {
        return generateRequestVersion1( root["request"] );
    }
    if ( !root["response"].isNull() )
    {
        return generateResponseVersion1( root["response"] );
    }
    if ( !root["notification"].isNull() )
    {
        return generateNotificationVersion1( root["notification"] );
    }

    return 0;
}

MobileRPCRequest * JSONHandler::generateRequestVersion1( const Json::Value & root ) 
{
    MobileRPCRequest * request = new MobileRPCRequest( 1 );
    return (MobileRPCRequest *)fillMessageWithData( root, request );
}

MobileRPCResponse * JSONHandler::generateResponseVersion1( const Json::Value & root ) 
{
    MobileRPCResponse * response = new MobileRPCResponse( 1 );
    return (MobileRPCResponse *)fillMessageWithData( root, response );
}

MobileRPCNotification * JSONHandler::generateNotificationVersion1( const Json::Value & root )
{
    MobileRPCNotification * notification = new MobileRPCNotification( 1 );
    return (MobileRPCNotification *) fillMessageWithData( root, notification );
}

MobileRPCMessage * JSONHandler::fillMessageWithData ( const Json::Value & jsonMessage, 
    MobileRPCMessage * message )
{
    unsigned int correlationID = 0;
    Json::Value value;
    value = jsonMessage["correlationID"];
    if ( !value.isNull() ) {
        correlationID = value.asInt();
        message->setCorrelationID( correlationID );
    }    

    value = jsonMessage["name"];
    if ( !value.isNull() )
    {
        message->setFunctionName( value.asString() );
    }

    value = jsonMessage["parameters"];
    if ( !value.isNull() )
    {
        Json::Value paramValue;
        paramValue["parameters"] = value;
        Json::FastWriter writer;
        std::string paramsToString = writer.write( paramValue );
        message -> setParametersString( paramsToString );
    }

    return message;
}

Json::Value JSONHandler::getParametersFromJSON( const std::string & jsonString )
{
    Json::Value root;   
    Json::Reader reader;
    bool parsingSuccessful = reader.parse( jsonString, root );
    if ( !parsingSuccessful ) 
    {
        return Json::Value::null;
    }

    return root["parameters"];
}

const MobileRPCFactory * JSONHandler::getFactory() const
{
    return mFactory;
}

MobileRPCMessage * JSONHandler::getRPCObject() const
{
    return mCurrentMessage;
}