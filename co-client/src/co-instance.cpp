#include "util.h"
#include "sender.h"
#include "receiver.h"

ofstream log_file;
static Sender* _Sender = NULL;
static Receiver* _Receiver = NULL;

void coclient_create()
{
	string coclient_dir;
#ifdef WIN32
	char user_path[1024] = { 0 };
	if (GetEnvironmentVariable("USERPROFILE", user_path, sizeof(user_path)) > 0) {
		coclient_dir = string(user_path) + "/co-client";
		if (_access(coclient_dir.c_str(), 0) != 0) {
			_mkdir(coclient_dir.c_str());
		}
	}
#else
	char* p = getenv("HOME");
	if (p) {
		coclient_dir = string(p) + "/.co-client";
		if (access(coclient_dir.c_str(), 0) != 0) {
			mkdir(coclient_dir.c_str());
		}
	}
#endif
	if (coclient_dir.length() > 0) {
		log_file.open(coclient_dir + "/co-client.log", ios_base::out | ios_base::app);
	}

	LOG_INFO("read server.ini");
	string line;
	string server_url;
	ifstream server_ini("server.ini");
	if (server_ini.is_open()) {
		while (getline(server_ini, line)) {
			if (line.substr(0, 4) == "URL=") {
				server_url = line.substr(4);
			}
		}
		server_ini.close();
	}
	if (server_url.length() == 0) {
		LOG_ERROR("server url null");
		return;
	}
	LOG_INFO("server url %s", server_url.c_str());
	_Sender = new Sender(server_url);
	_Receiver = new Receiver(server_url);
}

void coclient_destroy()
{
	delete _Sender;
	delete _Receiver;
	log_file.close();
}

bool coclient_register_compute_node(const std::string& app_name, const RDSHInfo& rdsh_info, std::string& app_guid)
{
	if (_Sender) {
		return _Sender->register_compute_node(app_name, rdsh_info, app_guid);
	}
	else {
		return false;
	}
}

void coclient_unregister_compute_node(const std::string& app_guid)
{
	if (_Sender) {
		_Sender->unregister_compute_node(app_guid);
	}
}

bool coclient_start_compute_node(const std::string& app_name, const RDSHInfo& rdsh_info)
{
	if (_Sender) {
		return _Sender->start_compute_node(app_name, rdsh_info);
	}
	else {
		return false;
	}
}

void coclient_stop_compute_node()
{
	if (_Sender) {
		_Sender->stop_compute_node();
	}
}

int coclient_get_compute_node_list(std::vector<NodeInfo>& node_list)
{
	if (_Receiver) {
		return _Receiver->get_compute_node_list(node_list);
	}
	else {
		return -1;
	}
}

int coclient_start_receiver(const NodeInfo& node)
{
	if (_Receiver) {
		int ins_id = _Receiver->start(node);
		_Receiver->set_fullscreen(ins_id);
		return ins_id;
	}
	else {
		return -1;
	}
}

void coclient_stop_receiver(int ins_id)
{
	if (_Receiver) {
		_Receiver->stop(ins_id);
	}
}

void coclient_show_user_list(int ins_id)
{
	if (_Receiver) {
		_Receiver->show_user_list(ins_id);
	}
}

void coclient_start_operate(int ins_id)
{
	if (_Receiver) {
		_Receiver->start_operate(ins_id);
	}
}