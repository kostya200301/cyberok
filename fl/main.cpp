#include <iostream>
#include <string>
#include <log4cpp/Category.hh>
#include <log4cpp/OstreamAppender.hh>
#include <log4cpp/FileAppender.hh>
#include <log4cpp/PatternLayout.hh>
#include <thread>
#include "DomainResolver.h"

log4cpp::Category& root = log4cpp::Category::getRoot();
int work = 1;

using namespace std;

string GetENVparams(const char* el) {
	const char* interval_str = getenv(el);
	if (interval_str != NULL) {
         	return interval_str;
        } else {
                root.error("Error getting environment parameter (" + std::string(el)  + ")");
		throw "Error getting environment parameter (" + std::string(el)  + ")";
        }


}

int main( )
{

	// Настройка логгера
	log4cpp::PatternLayout *layout = new log4cpp::PatternLayout();
	layout->setConversionPattern("%d [%p] %m%n");

	log4cpp::PatternLayout *layout2 = new log4cpp::PatternLayout();
	layout2->setConversionPattern("%d [%p] %m%n");

	log4cpp::Appender *consoleAppender = new log4cpp::OstreamAppender("console", &std::cout);
	consoleAppender->setLayout(layout);

	log4cpp::Appender *fileAppender = new log4cpp::FileAppender("file", "logs.log");
	fileAppender->setLayout(layout2);


	root.setPriority(log4cpp::Priority::DEBUG);
	root.addAppender(consoleAppender);
	root.addAppender(fileAppender);


        int interval = std::stoi(GetENVparams("INTERVAL"));
	const std::string serverBD = GetENVparams("SERVER_BD");
	const std::string serverREST = GetENVparams("SERVER_REST");
        const std::string username = GetENVparams("USERNAME");
        const std::string password = GetENVparams("PASSWORD");
        const std::string dbName = GetENVparams("DBNAME");
        const std::string tName = GetENVparams("TNAME");
        const std::string tNameWh = GetENVparams("TNAMEWH");


	root.info("Server start\nInterval: " + to_string(interval) + "\nServerBD: " + serverBD + "\nServerREST: " + serverREST + "\nUsername: " + username + "\nPassword: ****" + "\nDB_NAME: " + dbName + "\nTable_name: " + tName + "\nTable_name_whois: " + tNameWh);

	DomainResolver res(interval, serverBD, serverREST, username, password, dbName, tName, tNameWh);

	cout << "Press any button to stop\n";

	char a;
	cin >> a;
	work = 0;
	cout << "Ending...\n";
	this_thread::sleep_for(std::chrono::seconds(2));

	root.info("Server stop");
	return 0;
}

	/*
	int g;
	cin >> g;
	map<string, vector<string>> v = res->GetIp("ip.txt");
	for (auto p : v) {
		cout << p.first << ": ";
		for (auto h : p.second) {
			cout << h << " ";
		}
		cout << endl;
	}

	while (true) {
		string hn;
		cout << "hostname: ";
		cin >> hn;
		auto his = res->GetHostNameHistory(hn);
		for (auto l : his) {
			cout << l.first << " " << l.second << endl;
		}
	}
	*/
//	res->GetCSVFile("out.csv");

//	MakeCSVFileDB(con, "out.csv", "BK", "ip");
//	for (auto p : data) {
//		cout << p.first << ": ";
//		for (int i = 0; i < p.second.size(); i++) {
//			cout << p.second[i] << " ";
//		}
//		cout << endl;
//	}

	//GetHostnameFromIp("79.139.145.9");
	//GetIpFromHostname("79-139-145-9.dynamic.spd-mgts.ru");
