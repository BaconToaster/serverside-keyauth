#pragma once
#include "logger.hpp"
#include "packet.hpp"

namespace keyauth {
    using json = ::nlohmann::json;

    const std::string name = "Your-Application-Name";
    const std::string owner_id = "Your-Owner-ID";
    const std::string allowed_characters = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890-."; // add characters that occur in your keys here

    size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp) { // this is just taken from the keyauth cpp source
        ((std::string*)userp)->append((char*)contents, size * nmemb);
        return size * nmemb;
    }

    std::string request(std::string data) { // this is also from the keyauth source, i just changed it to the unencrypted backend 
                                            // (we don't need to encrypt the requests since they are sent from the server to keyauth)
        CURL* curl = curl_easy_init();
        if (!curl) {
            logger->warning("failed to init curl");
            return "null";
        }

        std::string to_return;

        curl_easy_setopt(curl, CURLOPT_URL, "https://keyauth.win/api/1.1/");

        curl_easy_setopt(curl, CURLOPT_USERAGENT, "KeyAuth");
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, true);

        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);

        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &to_return);

        if (curl_easy_perform(curl) != CURLE_OK) {
            logger->warning("failed to send a request to keyauth");
            return "null";
        }

        curl_easy_cleanup(curl);
        long http_code;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

        if (http_code == 429) {
            logger->warning("keyauth returned 429");
            return "null";
        }

        return to_return;
    }

    keyauth_result check_user(const std::string& key, const std::string& hwid, const std::string& version) {
        if (key.find_first_not_of(allowed_characters) != std::string::npos || hwid.find_first_not_of(allowed_characters) != std::string::npos ||
            version.find_first_not_of(allowed_characters) != std::string::npos) { // dont let the users inject unwanted characters into the url
            return keyauth_result::INVALID;
        }

        std::string session_id = "";
        { // from the keyauth php source
            std::string data = "type=init&ver=" + version + "&name=" + keyauth::name + "&ownerid=" + keyauth::owner_id;
            std::string response = request(data);
            if (!json::accept(response)) {
                logger->warning("keyauth returned invalid data %s", response.c_str());
                return keyauth_result::FAILED;
            }
            json result = json::parse(response);
            if (result["success"]) {
                session_id = result["sessionid"];
            }
            else if (result["message"] == "invalidver") {
                return keyauth_result::OUTDATED;
            }
            else {
                logger->warning("keyauth responded %s", std::string(result["message"]).c_str());
                return keyauth_result::FAILED;
            }
        }

        { // from the keyauth php source
            std::string data = "type=license&key=" + key + "&hwid=" + hwid + "&sessionid=" + session_id + "&name=" + keyauth::name + "&ownerid=" + keyauth::owner_id;
            std::string response = request(data);
            if (!json::accept(response)) {
                logger->warning("keyauth returned invalid data as a response to the license request %s", response.c_str());
                return keyauth_result::FAILED;
            }
            json result = json::parse(response);
            if (result["success"]) {
                return keyauth_result::VALID;
            }
            else {
                logger->warning("keyauth responded to license request: %s", std::string(result["message"]).c_str());
                return keyauth_result::INVALID;
            }
        }

        // how did we get here?
        return keyauth_result::FAILED;
    }
}