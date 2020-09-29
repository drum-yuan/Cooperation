#include "util.h"
#include "sender.h"
#include "receiver.h"

ofstream log_file;
static Sender* _Sender = NULL;
static Receiver* _Receiver = NULL;

void coclient_create(const std::string& server_url)
{
	if (!log_file.is_open()) {
		log_file.open("co-client.log", ios_base::out | ios_base::app);
	}

	if (server_url.length() == 0) {
		LOG_ERROR("server url null");
		log_file.close();
		return;
	}
	LOG_INFO("server url %s", server_url.c_str());
	_Sender = new Sender(server_url);
	_Receiver = new Receiver(server_url);
	LOG_INFO("co-client created");
}

void coclient_destroy()
{
	LOG_INFO("coclient destroy");
	delete _Sender;
	_Sender = NULL;
	delete _Receiver;
	_Receiver = NULL;
	//log_file.close();
}

bool coclient_register_compute_node(const std::string& app_name, const RDSHInfo& rdsh_info, std::string& app_guid)
{
	if (_Sender) {
		return _Sender->register_compute_node(app_name, rdsh_info, app_guid);
	}
	else {
		LOG_ERROR("Sender is null");
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
		_Receiver->stop(ins_id, true);
	}
}

void coclient_show_receiver(int ins_id)
{
	if (_Receiver) {
		_Receiver->show(ins_id);
	}
}

void coclient_hide_receiver(int ins_id)
{
	if (_Receiver) {
		_Receiver->hide(ins_id);
	}
}

void coclient_get_users_info(int ins_id, CoUsersInfo& users_info)
{
	if (_Receiver) {
		_Receiver->get_users_info(ins_id, users_info);
	}
}

void coclient_start_operate(int ins_id)
{
	if (_Receiver) {
		_Receiver->start_operate(ins_id);
	}
}

void coclient_send_picture()
{
	if (_Sender) {
		_Sender->send_picture();
	}
}

vector<int> coclient_get_current_receiver()
{
	vector<int> receiver_list;
	if (_Receiver) {
		receiver_list = _Receiver->get_current_receiver(); 
	}
	return receiver_list;
}

void coclient_set_sender_disconnect_callback(SenderDisconnectCallback on_sender_disconnect)
{
	if (_Sender) {
		_Sender->set_disconnect_callback(on_sender_disconnect);
	}
}