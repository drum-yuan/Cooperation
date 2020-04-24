#include <string.h>
#include <fstream>
#include <iostream>
#include "sender.h"
#include "receiver.h"

ofstream log_file("co-client.log", ios_base::out | ios_base::app);

int main(int argc, char** argv)
{
	LOG_INFO("read server.ini");
	string line;
	string server_url;
	RDSHInfo rdsh_info;
	rdsh_info.rdsh_ip = "localhost";
	rdsh_info.domain = "";
	ifstream server_ini("server.ini");
	if (server_ini.is_open()) {
		while (getline(server_ini, line)) {
			if (line.substr(0, 4) == "URL=") {
				server_url = line.substr(4);
			}
			else if (line.substr(0, 5) == "RDSH=") {
				rdsh_info.rdsh_ip = line.substr(5);
			}
			else if (line.substr(0, 7) == "DOMAIN=") {
				rdsh_info.domain = line.substr(7);
			}
			else if (line.substr(0, 5) == "USER=") {
				rdsh_info.user = line.substr(5);
			}
			else if (line.substr(0, 4) == "KEY=") {
				rdsh_info.password = line.substr(4);
			}
		}
		server_ini.close();
	}
	if (server_url.length() == 0) {
		return -1;
	}
	LOG_INFO("server url %s", server_url.c_str());
	Sender* pSender = NULL;
	Receiver* pReceiver = NULL;
	if (argc > 1 && strncmp(argv[1], "/Start", 6) == 0) {
		pSender = new Sender(server_url);
		if (!pSender) {
			return -1;
		}
		if (argc > 2 && argv[2] != NULL) {
			pSender->start_compute_node(string(argv[2]), rdsh_info);
			pSender->run();
			pSender->stop_compute_node();
		}
	}
	else if (argc > 1 && strncmp(argv[1], "/Receiver", 9) == 0) {
		pReceiver = new Receiver(server_url);
		if (!pReceiver) {
			return -1;
		}
		LOG_INFO("As receiver");
		vector<NodeInfo> node_list;
		int num = pReceiver->get_compute_node_list(node_list);
		if (num == 0) {
			LOG_ERROR("compute node is not exist\n");
			return 0;
		}
		printf("Current node list:\n");
		for (int i = 0; i < node_list.size(); i++) {
			printf("    %d. app guid:%s app name:%s sirius url:%s status:%d\n",
				i + 1, node_list[i].app_guid.c_str(), node_list[i].app_name.c_str(), node_list[i].sirius_url.c_str(), node_list[i].status);
		}
		printf("Please choose the order number!\n");
		int app_no = 0;
		while (scanf("%d", &app_no) > 0) {
			if (app_no > 0 && app_no <= node_list.size()) {
				int ins_id = pReceiver->start(node_list[app_no - 1]);
				pReceiver->set_fullscreen(ins_id);
			}
			util_sleep(1000);
		}
	}
	else {
		pSender = new Sender(server_url);
		if (!pSender) {
			return -1;
		}
		LOG_INFO("As sender");
		string app_guid;
		while (!pSender->register_compute_node("", rdsh_info, app_guid)) {
			util_sleep(1000);
		}
		pSender->run();
		pSender->unregister_compute_node(app_guid);
	}

	return 0;
}
