
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <string>
#include "DBConector.h"
#include <thread>

#include <log4cpp/Category.hh>
#include <log4cpp/OstreamAppender.hh>
#include <log4cpp/FileAppender.hh>
#include <log4cpp/PatternLayout.hh>

#include "DomainResolver.h"
#include "restserver.h"
#include "whois.cpp"
#include "Logger.h"



//using namespace std;


DomainResolver::DomainResolver(){}

DomainResolver::DomainResolver(int interval, const std::string& serverBD, const std::string& serverREST, const  std::string& username, const  std::string& password, const  std::string& dbName, const  std::string& tName, const  std::string& tNameWh){
	this->dbcon = DBConnector(serverBD, username, password, dbName, tName, tNameWh);
	this->dbconToUpdate = DBConnector(serverBD, username, password, dbName, tName, tNameWh);
        this->updateInterval = interval;
        this->updateStatus = 0;
        //start nre thread
        thread newThread(StartUpdating, this, this->updateInterval);
        newThread.detach();

        thread newThreadREST(StartREST, this, serverREST);
        newThreadREST.detach();
}

void DomainResolver::StartREST(DomainResolver* dr, const string& serverREST) {
	dr->restServer = new RestServer(dr, serverREST);
}

void DomainResolver::StartUpdating(DomainResolver* dr, int interval) {
                while (true) {
			root.info("Start updating DB...");
                        dr->updateStatus = 1;
                        dr->Update(&(dr->dbconToUpdate));
                        dr->updateStatus = 0;
                        root.info("End updating DB...");
			for (int i = 0; i < interval * 60; i++) {
				if (!work) {return;}
                        	this_thread::sleep_for(std::chrono::seconds(1));
			}
                }
}

void DomainResolver::Update(DBConnector* con = NULL) {
                auto allHN = this->GetAllHostNames(con);
                auto data = this->GetIpByHostnamesMap(allHN);
                this->AddData(data, con);
}

map<string, int> DomainResolver::GetAllHostNames(DBConnector* con = NULL) {
	if (con) {
		return this->dbcon.GetAllHostNames();
	}
	return con->GetAllHostNames();
}


map<string, vector<string> > DomainResolver::GetIpByHostnamesMap(map<string, int>& hostNames) {
                map<string, vector<string>> data;
                for (auto hn : hostNames) {
                        vector<string> ips = GetIpFromHostname(hn.first);
                        for (int j = 0; j < ips.size(); j++) {
                                data[ips[j]].push_back(hn.first);
                        }
                }
                return data;
        }

 bool DomainResolver::AddData(map<string, vector<string> >& data, DBConnector* con = NULL) {
                if (con == NULL) {
                        cout << "in Normal\n";
                        return this->dbcon.AddData(data);
                }
                return con->AddData(data);
        }

      vector<string> DomainResolver::GetIpFromHostname(string hostname) {
                char* hostn = (char*) hostname.c_str();
              struct in_addr addr;
              struct hostent * host_info;
                vector<string> ips;
              host_info = gethostbyname ( hostn );
                int i = 0;

                if ( host_info != NULL ) {
                        // cout << "\nHostname : " << host_info->h_name;
                        while ( host_info->h_addr_list[i] != 0 ) {
                                addr.s_addr = *(u_long *) host_info->h_addr_list[i++];
                                // cout<<"\nIP Address "<< inet_ntoa(addr);
                                ips.push_back(inet_ntoa(addr));
                        }
                } else {
                        cout << "error";
                }
                return ips;
        }



bool DomainResolver::IsValidIpHost(string hostname, string ip) {
                vector<string> ips = this->GetIpFromHostname(hostname);
                for (int i = 0; i < ips.size(); i++) {
                        if (ips[i] == ip) {
                                return true;
                        }
                }
                return false;
        }


vector<string> DomainResolver::GetDataFromFile(string filename) {
                ifstream file;
                file.open(filename);
                vector<string> vectorData;
                if (file.is_open()) {
                        string tmpData;
                        while (file >> tmpData) {
                                vectorData.push_back(tmpData);
                        }
                        file.close();
                }
                return vectorData;
        }



map<string, vector<string> > DomainResolver::GetIpByHostnamesVector(vector<string>& hostNames) {
                map<string, vector<string>> data;
                for (int i = 0; i < hostNames.size(); i++) {
                        vector<string> ips = GetIpFromHostname(hostNames[i]);
                        for (int j = 0; j < ips.size(); j++) {
                                data[ips[j]].push_back(hostNames[i]);
                        }
                }
                return data;
        }

map<string, string> DomainResolver::GetWhoisInfoByHosts(vector<string>& hosts) {
	return this->dbcon.AddWhois(hosts);
}

void DomainResolver::GetCSVFile(string name) {
                this->dbcon.MakeCSVFileDB(name);
        }


vector<pair<string, long long>> DomainResolver::GetHostNameHistory(string hostname) {
                root.info("Function was called to get history by hostname");
                 return this->dbcon.GetHostNameHistory(hostname);
                //cout << "Plz wait. Updating....\n";
                //return vector<pair<string, long long>>();
        }


        map<string, vector<string>> DomainResolver::GetHostNames(string filename) {
                root.info("Function was called to get ip by hostname");
                vector<string> hostnames = GetDataFromFile(filename);
                map<string, vector<string>> mapHostnames = GetIpByHostnamesVector(hostnames);
                AddData(mapHostnames);

                map<string, vector<string>> result;
                for (int i = 0; i < hostnames.size(); i++) {
                        map<string, int> ips = this->dbcon.GetIpByHostname(hostnames[i]);
                        for (auto p : ips) {
                                if (IsValidIpHost(hostnames[i], p.first)) {
                                        result[hostnames[i]].push_back(p.first);
                                }
                        }
                }
                return result;
        }

        map<string, vector<string>> DomainResolver::GetHostNamesVec(vector<string>& hostnames) {
                root.info("Function was called to get ip by hostnames");
		map<string, vector<string>> mapHostnames = GetIpByHostnamesVector(hostnames);
                AddData(mapHostnames);

                map<string, vector<string>> result;
                for (int i = 0; i < hostnames.size(); i++) {
                        map<string, int> ips = this->dbcon.GetIpByHostname(hostnames[i]);
                        for (auto p : ips) {
                                if (IsValidIpHost(hostnames[i], p.first)) {
                                        result[hostnames[i]].push_back(p.first);
                                }
                        }
                }
                return result;
        }


	map<string, vector<string>> DomainResolver::GetIpVec(vector<string>& ips) {
		root.info("function was called to get hostnames by ip");
                map<string, vector<string> > result;
                for (int i = 0; i < ips.size(); i++) {
                        map<string, int> hostnames = this->dbcon.GetHostnamesByIp(ips[i]);
                        for (auto p: hostnames) {
                                if (IsValidIpHost(p.first, ips[i])) {
                                        result[ips[i]].push_back(p.first);
                                }
                        }
                }
                for (auto ip : ips) {
                        if (result[ip].size() == 0) {
                                result[ip] = {};
                        }
                }
                return result;
        }

	map<string, vector<string>> DomainResolver::GetIp(string filename) {
		root.info("function was called to get hostnames by ip");
                vector<string> ips = GetDataFromFile(filename);
                map<string, vector<string> > result;
                for (int i = 0; i < ips.size(); i++) {
                        map<string, int> hostnames = this->dbcon.GetHostnamesByIp(ips[i]);
                        for (auto p: hostnames) {
                                if (IsValidIpHost(p.first, ips[i])) {
                                        result[ips[i]].push_back(p.first);
                                }
                        }
                }
                for (auto ip : ips) {
                        if (result[ip].size() == 0) {
                                result[ip] = {};
                        }
                }
                return result;
        }




/*string GetHostnameFromIp(string ip) {
	char *ash = (char*) ip.c_str();
	struct in_addr in;
        struct hostent *hp;
	if( inet_aton( ash, &in ) ) {
		if(hp = gethostbyaddr( (char*)&in.s_addr, sizeof(in.s_addr), AF_INET) ){
                        printf( "%s\n", hp->h_name );
		}
                else {
                        printf( "gethostbyaddr error: %d\n", h_errno );
		}
	}
	else {
 		printf( "inet_aton error\n" );
	}
}
*/


