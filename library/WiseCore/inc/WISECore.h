/****************************************************************************/
/* Copyright(C) : Advantech Technologies, Inc.								*/
/* Create Date  : 2016/03/01 by Scott Chang								    */
/* Modified Date: 2016/03/01 by Scott Chang									*/
/* Abstract     : WISE Core API definition									*/
/* Reference    : None														*/
/****************************************************************************/

#ifndef _WISE_AGENT_LITE_H_
#define _WISE_AGENT_LITE_H_

#include <stdbool.h>

typedef enum {
   core_offline = 0,
   core_online = 1, 
} lite_conn_status;

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#include <windows.h>
#ifndef WISECORE_API
	#define WISECORE_API __declspec(dllexport)
#endif
#else
	#define WISECORE_API
#endif

typedef void (*CORE_CONNECTED_CALLBACK)();
typedef void (*CORE_LOSTCONNECTED_CALLBACK)();
typedef void (*CORE_DISCONNECT_CALLBACK)();
typedef void (*CORE_MESSAGE_RECV_CALLBACK)(const char* topic, const void *pkt, const long pktlength);
typedef void (*CORE_RENAME_CALLBACK)(const char* name, const int cmdid, const char* sessionid, const char* devid);
typedef void (*CORE_UPDATE_CALLBACK)(const char* loginID, const char* loginPW, const int port, const char* path, const char* md5, const int cmdid, const char* sessionid, const char* devid);
typedef void (*CORE_SERVER_RECONNECT_CALLBACK)(const char* devid);
typedef void (*CORE_GET_CAPABILITY_CALLBACK)(const void *pkt, const long pktlength, const char* devid);
typedef void (*CORE_START_REPORT_CALLBACK)(const void *pkt, const long pktlength, const char* devid);
typedef void (*CORE_STOP_REPORT_CALLBACK)(const void *pkt, const long pktlength, const char* devid);
typedef void (*CORE_QUERY_HEARTBEATRATE_CALLBACK)(const char* sessionid,const char* devid);
typedef void (*CORE_UPDATE_HEARTBEATRATE_CALLBACK)(const int heartbeatrate, const char* sessionid, const char* devid);
typedef long long (*CORE_GET_TIME_TICK_CALLBACK)();
#ifdef __cplusplus
extern "C" {
#endif

/* 
 * Function: core_initialize
 *
 * General initialization of the WISEAgentLite API. Prior to calling any WISEAgentLite API functions, 
 * the library needs to be initialized by calling this function. The status code for all 
 * WISEAgentLite API function will be lite_no_init unless this function is called.
 *
 * Parameters:
 * 	strClientID -	the unique id for connection.
 * 	strHostName -	local device name.
 * 	strMAC -		local device MAC address
 *
 * Returns:
 * 	boolean value for success or not.
 */
WISECORE_API bool core_initialize(char* strClientID, char* strHostName, char* strMAC);

/* 
 * Function: core_uninitialize
 *
 * General function to uninitialized the WISEAgentLite API library that should be called before program exit.
 *
 * Parameters:
 * 	None
 *
 * Returns:
 * 	None
 */
WISECORE_API void core_uninitialize();

/* 
 * Function: core_product_info_set
 *
 * assign the production information let server to identify the target device.
 *
 * Parameters:
 * 	strSerialNum -		device serial number.
 *  strParentID -		parent ID.
 * 	strVersion -		agent version.
 * 	strType -			device type.
 * 	strProduct -		product name.
 * 	strManufacture -	manufacture name.
 *
 * Returns:
 * 	boolean value for success or not.	
 */
WISECORE_API bool core_product_info_set(char* strSerialNum, char* strParentID, char* strVersion, char* strType, char* strProduct, char* strManufacture);

/* 
 * Function: lite_os_info_set
 *
 * assign the OS information let server to identify the target device.
 *
 * Parameters:
 * 	strOSName -			OS name.
 * 	strOSArch -			OS architecture.
 * 	iTotalPhysMemKB -	total physical memory (Kilobyte ).
 * 	strMACs -			MAC address list.
 *
 * Returns:
 * 	boolean value for success or not.
 */
WISECORE_API bool core_os_info_set(char* strOSName, char* strOSArch, int iTotalPhysMemKB, char* strMACs);

/* 
 * Function: core_platform_info_set
 *
 * assign the platform information let server to identify the target device.
 *
 * Parameters:
 * 	strBIOSVersion -		device BIOS version.
 * 	strPlatformName -		board name.
 * 	strProcessorName -		processor name.
 *
 * Returns:
 * 	boolean value for success or not.
 */
WISECORE_API bool core_platform_info_set(char* strBIOSVersion, char* strPlatformName, char* strProcessorName);

/* 
 * Function: core_local_ip_set
 *
 * assign local ip for remote control. User can set local ip directory or remain NULL 
 * to retrieve the local ip while connected automatically (depends on WISECarrier Library).
 *
 * Parameters:
 * 	strLocalIP -			local ip address.
 *
 * Returns:
 * 	boolean value for success or not.	
 */
WISECORE_API bool core_local_ip_set(char* strLocalIP);

/* 
 * Function: core_account_bind
 *
 * user account, on remote server, to management the device
 *
 * Parameters:
 * 	strLoginID -		login ID.
 * 	strLoginPW -		login password.
 *
 * Returns:
 * 	boolean value for success or not.	
 */
WISECORE_API bool core_account_bind(char* strLoginID, char* strLoginPW);

/* 
 * Function: core_connection_callback_set
 *
 * Register the callback function to handle the connection event.
 *
 * Parameters:
 * 	on_connect -		Function Pointer to handle connect success event.
 *  on_lost_connect -	Function Pointer to handle lost connect event,
 *						The SAClient will reconnect automatically, if left as NULL.
 *  on_disconnect -		Function Pointer to handle disconnect event.
 *  on_msg_recv -		Function Pointer to handle message receive event.
 *
 * Returns:
 * 	boolean value for success or not.	
 */
WISECORE_API bool core_connection_callback_set(CORE_CONNECTED_CALLBACK on_connect, CORE_LOSTCONNECTED_CALLBACK on_lostconnect, CORE_DISCONNECT_CALLBACK on_disconnect, CORE_MESSAGE_RECV_CALLBACK on_msg_recv);

/* 
 * Function: core_action_callback_set
 *
 * Register the callback function to handle the action event.
 *
 * Parameters:
 * 	on_rename -		Function Pointer to handle rename event.
 *  on_update -		Function Pointer to handle update event.
 *
 * Returns:
 * 	boolean value for success or not.	
 */
WISECORE_API bool core_action_callback_set(CORE_RENAME_CALLBACK on_rename, CORE_UPDATE_CALLBACK on_update);

/*
 * Function: core_action_response
 *
 * Send rename, update or heartbeat rate update action response back to server.
 *
 * Parameters:
 *  cmdid -			command ID of request action
 * 	sessionid -		session ID of request action.
 *  success -		result of request action.
 *  devid -			device ID of request device.
 *
 * Returns:
 *  boolean value for success or not.	
 */
WISECORE_API bool core_action_response(const int cmdid, const char * sessoinid, bool success, const char* devid);

/* 
 * Function: core_server_reconnect_callback_set
 *
 * Register the callback function to handle the server reconnect event.
 *
 * Parameters:
 * 	on_server_reconnect -		Function Pointer to handle server reconnect event.
 *
 * Returns:
 * 	boolean value for success or not.	
 */
WISECORE_API bool core_server_reconnect_callback_set(CORE_SERVER_RECONNECT_CALLBACK on_server_reconnect);

/*
 * Function: core_iot_callback_set
 *
 * Register the callback function to handle the get IoT capability or start/stop report sensor data.
 *
 * Parameters:
 * 	on_get_capability	-	Function Pointer to handle get capability event.
 *  on_start_report		-	Function Pointer to handle start report event.
 *  on_stop_report		-	Function Pointer to handle stop report event.
 *
 * Returns:
 * 	boolean value for success or not.	
 */
WISECORE_API bool core_iot_callback_set(CORE_GET_CAPABILITY_CALLBACK on_get_capability, CORE_START_REPORT_CALLBACK on_start_report, CORE_STOP_REPORT_CALLBACK on_stop_report);

/*
 * Function: core_time_tick_callback_set
 *
 * Register the callback function to assign time tick.
 *
 * Parameters:
 * 	None
 *
 * Returns:
 * 	boolean value for success or not.	
 */
WISECORE_API bool core_time_tick_callback_set(CORE_GET_TIME_TICK_CALLBACK get_time_tick);

/* 
 * Function: core_heartbeat_callback_set
 *
 * Register the callback function to handle the heartbeat rate query and update command.
 *
 * Parameters:
 * 	on_query_heartbeatrate	-	Function Pointer to handle heartbeat rate query event.
 *  on_update_heartbeatrate	-	Function Pointer to handle heartbeat rate update event.
 *
 * Returns:
 * 	boolean value for success or not.	
 */
WISECORE_API bool core_heartbeat_callback_set(CORE_QUERY_HEARTBEATRATE_CALLBACK on_query_heartbeatrate, CORE_UPDATE_HEARTBEATRATE_CALLBACK on_update_heartbeatrate);

/*
 * Function: core_heartbeatratequery_response
 *
 * Send heartbeat rate update response back to server.
 *
 * Parameters:
 *  heartbeatrate - current heartbeat rate.
 * 	sessionid -		session ID of request action.
 *  devid -			device ID of request device.
 *
 * Returns:
 *  boolean value for success or not.	
 */
WISECORE_API bool core_heartbeatratequery_response(const int heartbeatrate, const char * sessoinid, const char* devid);

/*
 * Function: lite_tls_set
 *
 * Configure the client for certificate based SSL/TLS support. Must be called
 * before <wc_connect>.
 *
 * Cannot be used in conjunction with <core_tls_psk_set>.
 *
 * Define the Certificate Authority certificates to be trusted (ie. the server
 * certificate must be signed with one of these certificates) using cafile.
 *
 * If the server you are connecting to requires clients to provide a
 * certificate, define certfile and keyfile with your client certificate and
 * private key. If your private key is encrypted, provide a password callback
 * function.
 *
 * Parameters:
 *  cafile -      path to a file containing the PEM encoded trusted CA
 *                certificate files. Either cafile or capath must not be NULL.
 *  capath -      path to a directory containing the PEM encoded trusted CA
 *                certificate files. See mosquitto.conf for more details on
 *                configuring this directory. Either cafile or capath must not
 *                be NULL.
 *  certfile -    path to a file containing the PEM encoded certificate file
 *                for this client. If NULL, keyfile must also be NULL and no
 *                client certificate will be used.
 *  keyfile -     path to a file containing the PEM encoded private key for
 *                this client. If NULL, certfile must also be NULL and no
 *                client certificate will be used.
 *  password -	  if keyfile is encrypted, set the password to allow your client
 *                to pass the correct password for decryption.
 *
 * Returns:
 * 	boolean value for success or not.	
 *
 */
WISECORE_API bool core_tls_set(const char *cafile, const char *capath, const char *certfile, const char *keyfile, const char *password);

/*
 * Function: core_tls_psk_set
 *
 * Configure the client for pre-shared-key based TLS support. Must be called
 * before <wc_connect>.
 *
 * Cannot be used in conjunction with <core_tls_set>.
 *
 * Parameters:
 *  psk -      the pre-shared-key in hex format with no leading "0x".
 *  identity - the identity of this client. May be used as the username
 *             depending on the server settings.
 *	ciphers -  a string describing the PSK ciphers available for use. See the
 *	           "openssl ciphers" tool for more information. If NULL, the
 *	           default ciphers will be used.
 *
 * Returns:
 * 	boolean value for success or not.	
 *
 */
WISECORE_API bool core_tls_psk_set(const char *psk, const char *identity, const char *ciphers);

/* 
 * Function: core_connect
 *
 * Connect to server that defined in Configuration data of lite_initialize parameters.
 *
 * Parameters:
 * 	strServerIP -	remote server URL or IP address.
 * 	iServerPort -	connection protocol listen port.
 * 	strConnID -		connection protocol access id
 * 	strConnPW -		connection protocol access password
 *
 * Returns:
 * 	boolean value for success or not.	
 */
WISECORE_API bool core_connect(char* strServerIP, int iServerPort, char* strConnID, char* strConnPW);

/* 
 * Function: core_disconnect
 *
 * Disconnect from server.
 *
 * Parameters:
 * 	bForce -	Is force to disconnect.
 *
 * Returns:
 * 	None
 */
WISECORE_API void core_disconnect(bool bForce);

/* 
 * Function: core_device_register
 *
 * Send device information, wrapped in JSON format, to register device.
 *
 * Parameters:
 * 	none
 *
 * Returns:
 * 	boolean value for success or not.	
 */
WISECORE_API bool core_device_register();

/* 
 * Function: core_platform_register
 *
 * Send platform information, wrapped in JSON format, to register device platform.
 *
 * Parameters:
 * 	none
 *
 * Returns:
 * 	boolean value for success or not.	
 */
WISECORE_API bool core_platform_register();

/* 
 * Function: core_heartbeat_send
 *
 * Send heartbeat message, wrapped in JSON format, to server.
 *
 * Parameters:
 * 	none
 *
 * Returns:
 * 	boolean value for success or not.	
 */
WISECORE_API bool core_heartbeat_send();

/* 
 * Function: core_publish
 *
 * Send message, wrapped in JSON format, to server on specific topic.
 *
 * Parameters:
 * 	topic -		the MQTT topic to publish.
 * 	pkt -		the message to publish, in JSON string struct.
 *  pktlength -	the message length.
 * 	retain	-	enable flag to retain this message in broker.
 *  qos		-	QoS 1, 2, 3
 *
 * Returns:
 * 	boolean value for success or not.	
 */
WISECORE_API bool core_publish(char const * topic, void * pkt, long pktlength, int retain, int qos);

/* 
 * Function: core_subscribe
 *
 * subscribe and receive the message from server on specific topic.
 *
 * Parameters:
 * 	topic -					the topic to subscribe.
 *  qos		-	QoS 1, 2, 3
 *
 * Returns:
 * 	boolean value for success or not.	
 */
WISECORE_API bool core_subscribe(char const * topic, int qos);

/* 
 * Function: core_unsubscribe
 *
 * stop to receive message from server on specific topic.
 *
 * Parameters:
 * 	topic -					the topic to unsubscripted.
 *
 * Returns:
 * 	boolean value for success or not.	
 */
WISECORE_API bool core_unsubscribe(char const * topic);

/*
 * Function: core_address_get
 *
 * To get local IP address.
 *
 * Parameters:
 *	address 	-	to get local IP address.
 *
 * Returns:
 *	true or false.
 */
WISECORE_API bool core_address_get(char *address);

/* 
 * Function: core_error_string_get
 *
 * Call to obtain a const string description the error number.
 *
 * Parameters:
 *	rc - an error number.
 *
 * Returns:
 *	A constant string describing the error.
 */
WISECORE_API const char* core_error_string_get();

#ifdef __cplusplus
}
#endif

#endif //_WISE_AGENT_LITE_H_
