#pragma once
#include <string>
#include <vector>

#ifdef WIN32
#define CO_API __declspec(dllexport)
#else
#define CO_API
#endif

struct RDSHInfo {
	std::string rdsh_ip;
	std::string domain;
	std::string user;
	std::string password;
};

struct NodeInfo {
	std::string app_guid;
	std::string app_name;
	std::string sirius_url;
	std::string host_name;
	int status;
};

struct CoUsersInfo {
	unsigned int user_num;
	std::vector<std::string> user_list;
	std::string publisher;
	std::string operater;
};

typedef void(*SenderDisconnectCallback)();

CO_API void coclient_create(const std::string& server_url);

CO_API void coclient_destroy();

CO_API bool coclient_register_compute_node(const std::string& app_name, const RDSHInfo& rdsh_info, std::string& app_guid);

CO_API void coclient_unregister_compute_node(const std::string& app_guid);

CO_API bool coclient_start_compute_node(const std::string& app_name, const RDSHInfo& rdsh_info);

CO_API void coclient_stop_compute_node();

CO_API int coclient_get_compute_node_list(std::vector<NodeInfo>& node_list);

CO_API int coclient_start_receiver(const NodeInfo& node);

CO_API void coclient_stop_receiver(int ins_id);

CO_API void coclient_show_receiver(int ins_id);

CO_API void coclient_hide_receiver(int ins_id);

CO_API void coclient_get_users_info(int ins_id, CoUsersInfo& users_info);

CO_API void coclient_start_operate(int ins_id);

CO_API void coclient_send_picture();

CO_API std::vector<int> coclient_get_current_receiver();

CO_API void coclient_set_sender_disconnect_callback(SenderDisconnectCallback on_sender_disconnect);