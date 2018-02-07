#include "types.h"

#include <stdexcept>

namespace data {
Json::Value Package::toJson() {
  Json::Value json;
  json["name"] = name;
  json["version"] = version;
  return json;
}

Package Package::fromJson(const std::string& json_str) {
  Json::Reader reader;
  Json::Value json;
  reader.parse(json_str, json);
  Package package;
  package.name = json["name"].asString();
  package.version = json["version"].asString();
  return package;
}

Json::Value UpdateAvailable::toJson() {
  Json::Value json;
  json["update_id"] = update_id;
  json["signature"] = signature;
  json["description"] = description;
  json["request_confirmation"] = request_confirmation;
  json["size"] = size;
  return json;
}

UpdateAvailable UpdateAvailable::fromJson(const std::string& json_str) {
  Json::Reader reader;
  Json::Value json;
  reader.parse(json_str, json);
  return fromJson(json);
}

UpdateAvailable UpdateAvailable::fromJson(const Json::Value& json) {
  UpdateAvailable ua;
  ua.update_id = json["update_id"].asString();
  ua.signature = json["signature"].asString();
  ua.description = json["description"].asString();
  ua.request_confirmation = json["request_confirmation"].asBool();
  ua.size = json["size"].asUInt();
  return ua;
}

Json::Value DownloadComplete::toJson() {
  Json::Value json;
  json["update_id"] = update_id;
  json["update_image"] = update_image;
  json["signature"] = signature;
  return json;
}

DownloadComplete DownloadComplete::fromJson(const std::string& json_str) {
  Json::Reader reader;
  Json::Value json;
  reader.parse(json_str, json);
  DownloadComplete dc;
  dc.update_id = json["update_id"].asString();
  dc.update_image = json["update_image"].asString();
  dc.signature = json["signature"].asString();
  return dc;
}

Json::Value OperationResult::toJson() {
  Json::Value json;
  json["id"] = id;
  json["result_code"] = result_code;
  json["result_text"] = result_text;
  return json;
}

UpdateReport OperationResult::toReport() {
  UpdateReport report;
  report.update_id = id;
  report.operation_results.push_back(*this);
  return report;
}

OperationResult OperationResult::fromJson(const std::string& json_str) {
  Json::Reader reader;
  Json::Value json;
  reader.parse(json_str, json);
  OperationResult operation_result;
  operation_result.id = json["id"].asString();
  operation_result.result_code = static_cast<UpdateResultCode>(json["result_code"].asUInt());
  operation_result.result_text = json["result_text"].asString();
  return operation_result;
}

OperationResult OperationResult::fromOutcome(const std::string& id, const InstallOutcome& outcome) {
  OperationResult operation_result;
  operation_result.id = id;
  operation_result.result_code = outcome.first;
  operation_result.result_text = outcome.second;
  return operation_result;
}

Json::Value UpdateReport::toJson() {
  Json::Value json;
  json["update_id"] = update_id;

  Json::Value operation_results_json(Json::arrayValue);
  for (std::vector<data::OperationResult>::iterator it = operation_results.begin(); it != operation_results.end();
       ++it) {
    operation_results_json.append(it->toJson());
  }
  json["operation_results"] = operation_results_json;
  return json;
}

UpdateReport UpdateReport::fromJson(const std::string& json_str) {
  Json::Reader reader;
  Json::Value json;
  reader.parse(json_str, json);
  UpdateReport update_report;
  update_report.update_id = json["update_id"].asString();
  for (unsigned int index = 0; index < json["operation_results"].size(); ++index) {
    update_report.operation_results.push_back(
        OperationResult::fromJson(Json::FastWriter().write(json["operation_results"][index])));
  }
  return update_report;
}

PackageManagerCredentials::PackageManagerCredentials(const CryptoKey& cryptokey)
    : ca_path(cryptokey.getCa()), pkey_path(cryptokey.getPkey()), cert_path(cryptokey.getCert()) {}

const std::string& PackageManagerCredentials::ca_file() const { return ca_path; }

const std::string& PackageManagerCredentials::pkey_file() const { return pkey_path; }

const std::string& PackageManagerCredentials::cert_file() const { return cert_path; }
}
// vim: set tabstop=2 shiftwidth=2 expandtab:
