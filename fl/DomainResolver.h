#ifndef DOMAIN_RESOLVER_H
#define DOMAIN_RESOLVER_H

//#include <map>
//#include <vector>
#include "DBConector.h"
#include <string>
//#include "restserver.h"

#include <log4cpp/Category.hh>
#include <log4cpp/OstreamAppender.hh>
#include <log4cpp/FileAppender.hh>
#include <log4cpp/PatternLayout.hh>


class RestServer;


class DomainResolver {
private:
	DBConnector dbcon;
	DBConnector dbconToUpdate;
	int updateInterval; // min
	int updateStatus;
	RestServer* restServer;
	static void StartUpdating(DomainResolver* dr, int interval);
	static void StartREST(DomainResolver* dr, const string& serverREST);
public:
//	string server;

	DomainResolver();

	DomainResolver(int interval, const std::string& serverBD, const std::string& serverREST, const  std::string& username, const  std::string& password, const  std::string& dbName, const  std::string& tName, const  std::string& tNameWh);

	vector<string> GetIpFromHostname(string hostname);

	bool IsValidIpHost(string hostname, string ip);

	vector<string> GetDataFromFile(string filename);

	map<string, vector<string> > GetIpByHostnamesVector(vector<string>& hostNames);

	map<string, vector<string> > GetIpByHostnamesMap(map<string, int>& hostNames);

	map<string, string> GetWhoisInfoByHosts(vector<string>& hosts);

	bool AddData(map<string, vector<string> >& data, DBConnector* con);

	map<string, int> GetAllHostNames(DBConnector* con);


	void Update(DBConnector* con);

	void GetCSVFile(string name);

	vector<pair<string, long long>> GetHostNameHistory(string hostname);


	map<string, vector<string>> GetHostNames(string filename);

	map<string, vector<string>> GetHostNamesVec(vector<string>& hostnames);

	map<string, vector<string>> GetIpVec(vector<string>& ips);

	map<string, vector<string>> GetIp(string filename);


};


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


#endif
