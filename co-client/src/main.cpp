#include <string.h>
#include <fstream> 
#include <iostream> 
#include "Sender.h"
#include "Receiver.h"


int main(int argc, char** argv)
{
	if (argc < 2) {
		return -1;
	}

	printf("read server.ini\n");
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
		}
		server_ini.close();
	}
	if (server_url.length() == 0) {
		return -1;
	}

	Sender* pSender;
	Receiver* pReceiver;
	string app_guid;
	std::vector<NodeInfo> node_list;
	if (strncmp(argv[1], "/Sender", 7) == 0) {
		pSender = new Sender(server_url);
		printf("As sender %s\n", server_url.c_str());
		if (argc > 2 && argv[2] != NULL) {
			if (pSender) {
				pSender->register_compute_node(string(argv[2]), rdsh_info, app_guid);
			}
		}
		else {
			if (pSender) {
				pSender->register_compute_node(string("desktop"), rdsh_info, app_guid);
			}
		}
		while (getchar() != 'q') {
			Sleep(1000);
		}
	}
	else if (strncmp(argv[1], "/Receiver", 7) == 0) {
		pReceiver = new Receiver(server_url);
		int num = pReceiver->get_compute_node_list(node_list);
		if (num == 0) {
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
			if (app_no > 0 && app_no <= node_list.size() && pReceiver) {
				pReceiver->start(node_list[app_no - 1]);
			}
			Sleep(1000);
		}
	}

	return 0;
}