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

CO_API void coclient_create(const std::string& coclient_path);

CO_API void coclient_destroy();

CO_API bool coclient_register_compute_node(const std::string& app_name, const RDSHInfo& rdsh_info, std::string& app_guid);

CO_API void coclient_unregister_compute_node(const std::string& app_guid);

CO_API bool coclient_start_compute_node(const std::string& app_name, const RDSHInfo& rdsh_info);

CO_API void coclient_stop_compute_node();

CO_API int coclient_get_compute_node_list(std::vector<NodeInfo>& node_list);

CO_API int coclient_start_receiver(const NodeInfo& node);

CO_API void coclient_stop_receiver(int ins_id);

CO_API void coclient_show_user_list(int ins_id);

CO_API void coclient_start_operate(int ins_id);
