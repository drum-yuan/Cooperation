
#include <fstream>
#include <thread>
#include <chrono>
#include "vdi-sender-api.h"
#include "co-interface.h"

using namespace std;

#pragma comment( linker, "/subsystem:windows /entry:mainCRTStartup" )

static string _appGuid;
static string _appName;
static bool _needReconnect = true;
static void on_sender_disconnect()
{
	_needReconnect = true;
}

int main(int argc, char* argv[])
{
	string line;
	string serverIp;
	string uuid;
	int has_center = 0;
	ifstream setting_ini("C:\\vm_conf");
	if (setting_ini.is_open()) {
		while (getline(setting_ini, line)) {
			if (line.substr(0, 14) == "controller_ip=") {
				serverIp = line.substr(14);
			}
			if (line.substr(0, 5) == "uuid=") {
				uuid = line.substr(5);
			}
			if (line.substr(0, 11) == "has_center=") {
				has_center = stoi(line.substr(11));
			}
		}
		setting_ini.close();
	}
	if (!has_center) {
		sender_create();
		while (true) {
			sender_start(30000);
			sender_stop();
			this_thread::sleep_for(std::chrono::milliseconds(1000));
		}
		return 0;
	}

	if (serverIp.length() == 0) {
		return -1;
	}
	coclient_create(serverIp + ":30000");
	RDSHInfo rdshInfo;
	_appName = "uuid-" + uuid;

	while (true) {
		if (_needReconnect) {
			RDSHInfo rdshInfo;
			while (!coclient_register_compute_node(_appName, rdshInfo, _appGuid)) {
				this_thread::sleep_for(std::chrono::milliseconds(1000));
			}
			_needReconnect = false;
			coclient_set_sender_disconnect_callback(on_sender_disconnect);
		}
		else {
			this_thread::sleep_for(std::chrono::milliseconds(1000));
		}
	}
	return 0;
}