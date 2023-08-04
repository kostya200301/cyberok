#ifndef DBCONECTOR
#define DBCONECTOR

#include <stdlib.h>
#include <iostream>
#include <locale.h>
#include <sstream>
#include <regex>
#include <cmath>
#include "mysql_connection.h"
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/prepared_statement.h>
#include <ctime>
#include <string>
#include <chrono>
#include <ctime>
#include <sstream>
#include <fstream>
#include "Logger.h"

using namespace std;


std::string getWhoisInfo(const std::string& domain);


class DBConnector {
private:
	sql::Connection* con;
	string server;
	string username;
	string password;
	string DBname;
	string Tname;
	string TnameWh;

public:

	DBConnector() {}

	DBConnector(const string& server_, const string& username_, const string& password_, const string& DBname_, const string& Tname_, const string& TnameWh_) {
		sql::Driver* driver;
		try {
			driver = get_driver_instance();
			this->con = driver->connect(server_, username_, password_);
			sql::Statement* stmt;
        		stmt = this->con->createStatement();
			stmt->execute("CREATE DATABASE IF NOT EXISTS " + DBname_ + ";");

	        	this->con->setSchema(DBname_);

			stmt->execute("CREATE TABLE IF NOT EXISTS " + Tname_ + " (id serial PRIMARY KEY, ip TEXT, hostname TEXT, datetime BIGINT);");
			stmt->execute("CREATE TABLE IF NOT EXISTS " + TnameWh_ + " (id serial PRIMARY KEY, hostname TEXT, whois LONGTEXT);");
			delete stmt;

			this->server = server_;
			this->username = username_;
			this->password = password_;
			this->DBname = DBname_;
			this->Tname = Tname_;
			this->TnameWh = TnameWh_;
		} catch (sql::SQLException e) {
			root.error("Could not connect to server. Error message: " + string(e.what()));
		}
	}


	void MakeCSVFileDB(string filename) {
        	sql::Statement* stmt;
        	sql::ResultSet* res;
		try {
        		stmt = this->con->createStatement();

			ofstream out;
			out.open(filename);
			int i = 0;
        		res = stmt->executeQuery("SELECT * FROM " + this->Tname + ";");
       			while (res->next()) {
				string tmpData = "";
                		tmpData += res->getString("hostname") + "; ";
                		tmpData += res->getString("ip") + "; ";
                		tmpData += res->getString("datetime") + "\n";
				out << tmpData;
				i++;
        		}
			out.close();
		} catch (sql::SQLException e) {
			root.error("SQL ERROR. Error message: " + string(e.what()));
                }

	}


	bool AddOrNot(string hostname, string ip) {
		sql::Statement* stmt;
		sql::ResultSet* res;
		try {
			stmt = this->con->createStatement();

			long long time = 0;

			res = stmt->executeQuery("SELECT * FROM " + this->Tname + " WHERE hostname = '" + hostname + "' " + "ORDER BY id DESC LIMIT 1;");
			while (res->next()) {
				time = res->getInt64("datetime");
			}
			if (time == 0) {
				return true;
			}
			res = stmt->executeQuery("SELECT * FROM " + this->Tname + " WHERE hostname = '" + hostname + "' AND " + "ip = '" + ip + "' AND datetime = " + to_string(time) + ";");
			while (res->next()) {
				return false;
			}
			return true;

		} catch (sql::SQLException e) {
			root.error("SQL ERROR. Error message: " + string(e.what()));
			return false;
                }

	}




	bool AddData(map<string, vector<string>>& data) {
		sql::PreparedStatement* pstmt;
		try {
                	pstmt = this->con->prepareStatement("INSERT INTO " + this->Tname + "(ip, hostname, datetime) VALUES(?,?,?)");
			auto curTime = time(NULL);
			stringstream ss;
			ss << time(NULL);
			for (auto p : data) {
				for (auto hn : p.second) {
					if (this->AddOrNot(hn, p.first)) {
						pstmt->setString(1, p.first);
						pstmt->setString(2, hn);
						pstmt->setString(3, ss.str());
						pstmt->execute();
					}
				}
			}
			return true;
		} catch (sql::SQLException e) {
			root.error("SQL ERROR. Error message: " + string(e.what()));
			return false;
                }

	}

	string GetWhois(string hostname) {
		sql::Statement* stmt;
        	sql::ResultSet* res;
		try {
        		stmt = this->con->createStatement();

			string whois = "";

        		res = stmt->executeQuery("SELECT * FROM " + this->TnameWh + " WHERE hostname = '" + hostname + "' " + ";");
        		while (res->next()) {
        		        return res->getString("whois");
        		}
			return whois;
		} catch (sql::SQLException e) {
			root.error("SQL ERROR. Error message: " + string(e.what()));
                        return "";
                }

	}

	map<string, string> AddWhois(vector<string>& hostnames) {
		map<string, string> mapHostWhois;
		sql::PreparedStatement* pstmt;
		try {
			for (auto p : hostnames) {
				string whoisInfo = GetWhois(p);
				if (whoisInfo.size() == 0) {
					string info = getWhoisInfo(p);
					if (info.size() != 0) {
               					pstmt = this->con->prepareStatement("INSERT INTO " + this->TnameWh + "(hostname, whois) VALUES(?,?)");
						pstmt->setString(1, p);
        					pstmt->setString(2, info);
						pstmt->execute();
						mapHostWhois[p] = info;
					} else {
						mapHostWhois[p] = "-";
					}
				} else {
					mapHostWhois[p] = whoisInfo;
				}
			}
			return mapHostWhois;

		} catch (sql::SQLException e) {
			root.error("SQL ERROR. Error message: " + string(e.what()));
                        return map<string, string>();
                }

	}



	map<string, int> GetHostnamesByIp(string ip) {
		sql::Statement* stmt;
        	sql::ResultSet* res;
		try {
        		stmt = this->con->createStatement();

			map<string, int> mapHost;

        		res = stmt->executeQuery("SELECT * FROM " + this->Tname + " WHERE ip = '" + ip + "' " + ";");
        		while (res->next()) {
        		        mapHost[res->getString("hostname")] += 1;
        		}
			return mapHost;
		} catch (sql::SQLException e) {
			root.error("SQL ERROR. Error message: " + string(e.what()));
                        return map<string, int>();
                }

	}

	map<string, int> GetIpByHostname(string hostname) {
		sql::Statement* stmt;
        	sql::ResultSet* res;
		try {
        		stmt = this->con->createStatement();

			map<string, int> mapHost;

        		res = stmt->executeQuery("SELECT * FROM " + this->Tname + " WHERE hostname = '" + hostname + "' " + ";");
        		while (res->next()) {
        		        mapHost[res->getString("ip")] += 1;
        		}
			return mapHost;
		} catch (sql::SQLException e) {
			root.error("SQL ERROR. Error message: " + string(e.what()));
                        return map<string, int>();
                }

	}

	map<string, int> GetAllHostNames() {
		sql::Statement* stmt;
        	sql::ResultSet* res;
		try {
        		stmt = this->con->createStatement();
			map<string, int> mapHost;
        		res = stmt->executeQuery("SELECT * FROM " + this->Tname + ";");
        		while (res->next()) {
        		        mapHost[res->getString("hostname")] += 1;
        		}
			return mapHost;
		} catch (sql::SQLException e) {
			root.error("SQL ERROR. Error message: " + string(e.what()));
                        return map<string, int>();
                }

	}

	vector<pair<string, long long>> GetHostNameHistory(string hostname) {
		sql::Statement* stmt;
        	sql::ResultSet* res;
		try {
        		stmt = this->con->createStatement();

			vector<pair<string, long long>> hostVec;

        		res = stmt->executeQuery("SELECT * FROM " + this->Tname + " WHERE hostname = '" + hostname + "' " + ";");
        		while (res->next()) {
        		        hostVec.push_back(make_pair(res->getString("ip"), res->getInt64("datetime")));
        		}
			return hostVec;
		} catch (sql::SQLException e) {
			root.error("SQL ERROR. Error message: " + string(e.what()));
                        return vector<pair<string, long long>>();
                }

	}

};


#endif
