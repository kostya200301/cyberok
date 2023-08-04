
#include <cpprest/http_listener.h>
#include <cpprest/json.h>
#include <cpprest/http_listener.h>
#include <cpprest/json.h>
#include "restserver.h"
#include "Logger.h"
#include <thread>

//#include "DomainResolver.h"
using namespace std;
using namespace web;
using namespace web::http;
using namespace web::http::experimental::listener;


RestServer::RestServer() {}

RestServer::RestServer(DomainResolver* dr_, const string& serverREST) {
		this->dr = dr_;
//                http_listener listener("http://localhost:8080/api");
                http_listener listener("http://" + serverREST);
                listener.support(methods::POST, handle_post);

                try {
                        listener.open().wait();
                        std::cout << "Listening for requests at: " << listener.uri().to_string() << std::endl;
                        //std::cin.get();
			while (work) {
                                this_thread::sleep_for(std::chrono::seconds(1));
			}
                        listener.close().wait();
                } catch (const std::exception& e) {
                        std::cout << e.what() << std::endl;
                }
        }

DomainResolver* RestServer::dr = new DomainResolver();

void RestServer::handle_post(http_request request) {
                // Извлечь JSON-тело из запроса
                request.extract_json().then([request](pplx::task<json::value> task) {
                try {
                        const json::value& body = task.get();
                        // Проверить, что полученное тело является объектом JSON
                        if (body.is_object()) {
				std::string action = "";
				if (!body.has_field(U("action"))) {
					root.info("Wrong request received");
            				http_response response(status_codes::BadRequest); // Используем код 400 (Некорректный запрос) для сообщения об ошибке.
            				response.set_body(U("Ошибка: некорректный формат запроса."));
           				request.reply(response);
					return;
				}
				action = body.at(U("action")).as_string();

				if (action == "get_ip_by_fqdn") {
					root.info("Request get_ip_by_fqdn received");
                    			//std::wstring fqdn = body[U("data")][U("fqdn")].as_string();
					const web::json::array& dataArray = body.at(U("data")).as_array();
					vector<string> hostnames;
        				for (const auto& item : dataArray) {
        					if (item.is_string()) {
                					const utility::string_t& strValue = item.as_string();
				                	hostnames.push_back(strValue);
            					}
        				}
					map<string, vector<string>> mapHostIp = dr->GetHostNamesVec(hostnames);

                   			 // Создать JSON-объект для ответа
					web::json::value jsonObj;

					for (const auto& obj : mapHostIp) {
						web::json::value arr = web::json::value::array(obj.second.size());
						int index = 0;
						for (const auto& value : obj.second) {
							arr[index++] = web::json::value::string(value);
						}
						jsonObj[utility::conversions::to_string_t(obj.first)] = arr;
					}

                   			 // Отправить ответ с JSON-телом
                   			 request.reply(status_codes::OK, jsonObj);
				} else if (action == "get_fqdn_by_ip") {
					root.info("Request get_fqdn_by_ip received");
					const web::json::array& dataArray = body.at(U("data")).as_array();
					vector<string> ips;
        				for (const auto& item : dataArray) {
        					if (item.is_string()) {
                					const utility::string_t& strValue = item.as_string();
				                	ips.push_back(strValue);
            					}
        				}
					map<string, vector<string>> mapIpHost = dr->GetIpVec(ips);

                   			 // Создать JSON-объект для ответа
					web::json::value jsonObj;

					for (const auto& obj : mapIpHost) {
						web::json::value arr = web::json::value::array(obj.second.size());
						int index = 0;
						for (const auto& value : obj.second) {
							arr[index++] = web::json::value::string(value);
						}
						jsonObj[utility::conversions::to_string_t(obj.first)] = arr;
					}

                   			 // Отправить ответ с JSON-телом
                   			 request.reply(status_codes::OK, jsonObj);
				} else if (action == "get_whois_info") {
					root.info("Request get_whois_info received");
					const web::json::array& dataArray = body.at(U("data")).as_array();
                                        vector<string> hostnames;
                                        for (const auto& item : dataArray) {
                                                if (item.is_string()) {
                                                        const utility::string_t& strValue = item.as_string();
                                                        hostnames.push_back(strValue);
                                                }
                                        }

					map<string, string> mapHostWhois = dr->GetWhoisInfoByHosts(hostnames);
					web::json::value jsonObj;
    					for (const auto& obj : mapHostWhois) {
//						cout << obj.first << " " << obj.second << endl;
						jsonObj[utility::conversions::to_string_t(obj.first)] = web::json::value::string(obj.second);
    					}

                                        // Отправить ответ с JSON-телом
                                        request.reply(status_codes::OK, jsonObj);

				} else if (action == "get_hostname_history") {
					root.info("Request get_hostname_history received");
					const web::json::array& dataArray = body.at(U("data")).as_array();
                                        map<string, map<long long, vector<string>>> mapHostIpDate;
                                        for (const auto& item : dataArray) {
                                                if (item.is_string()) {
                                                        const utility::string_t& strValue = item.as_string();
							for (auto p : dr->GetHostNameHistory(strValue)) {
		                                        	mapHostIpDate[strValue][p.second].push_back(p.first);

							}
                                                }
                                        }

					web::json::value jsonObj;
                                        for (const auto& obj : mapHostIpDate) {
                                                web::json::value object = web::json::value::object();
                                                for (const auto& value : obj.second) {
	                                                int index = 0;
                                                	web::json::value arr = web::json::value::array(value.second.size());
							for (const auto& ip : value.second) {
                                                		arr[index++] = web::json::value::string(utility::conversions::to_string_t(ip));
							}
							// time_t to normal datetime format
							time_t currentTime = value.first;
   							struct tm* timeinfo = std::localtime(&currentTime);
    							const char* format = "%Y-%m-%d %H:%M:%S";
    							char buffer[80];
    							std::strftime(buffer, sizeof(buffer), format, timeinfo);

							object[buffer] = arr;
                                                }

                                                jsonObj[obj.first] = object;
                                        }
                                        // Отправить ответ с JSON-телом
                                        request.reply(status_codes::OK, jsonObj);

				} else {
					root.info("Wrong request received");
                                        http_response response(status_codes::BadRequest); // Используем код 400 (Некорректный запрос) для сообщения об ошибке.
                                        response.set_body(U("Ошибка: некорректный формат запроса."));
                                        request.reply(response);
                                        return;
				}

                        } else {
                                // Отправить ошибку "BadRequest", если тело запроса не является объектом JSON
                                request.reply(status_codes::BadRequest, U("Invalid JSON format in the request."));
                        }
                } catch (const http_exception& e) {
                        // Отправить ошибку "InternalError", если возникла исключительная ситуация при обработке запроса
                        request.reply(status_codes::InternalError, e.what());
                }}).wait();
        }



