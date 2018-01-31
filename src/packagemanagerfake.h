#ifndef PACKAGEMANAGERFAKE_H_
#define PACKAGEMANAGERFAKE_H_

#include <sstream>

#include <boost/algorithm/hex.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/make_shared.hpp>
#include <boost/shared_ptr.hpp>

#include "crypto.h"
#include "packagemanagerinterface.h"

int current_package = 0;

class PackageFake : public PackageInterface {
 public:
  PackageFake(const std::string &ref_name_in, const std::string &refhash_in, const std::string &treehub_in)
      : PackageInterface(ref_name_in, refhash_in, treehub_in) {}

  ~PackageFake() {}

  data::InstallOutcome install(const PackageConfig &config) const {
    (void)config;
    ++current_package;
    return data::InstallOutcome(data::OK, "Installation successful");
  }

  Json::Value toEcuVersion(const std::string &ecu_serial, const Json::Value &custom) const {
    Json::Value installed_image;
    installed_image["filepath"] = ref_name;
    installed_image["fileinfo"]["length"] = 0;
    installed_image["fileinfo"]["hashes"]["sha256"] = refhash;

    Json::Value value;
    value["attacks_detected"] = "";
    value["installed_image"] = installed_image;
    value["ecu_serial"] = ecu_serial;
    value["previous_timeserver_time"] = "1970-01-01T00:00:00Z";
    value["timeserver_time"] = "1970-01-01T00:00:00Z";
    if (custom != Json::nullValue) {
      value["custom"] = custom;
    }
    return value;
  }
};

class PackageManagerFake : public PackageManagerInterface {
 public:
  Json::Value getInstalledPackages() {
    Json::Value packages(Json::arrayValue);
    return packages;
  }

  std::string getCurrent() {
    std::stringstream stream;
    stream << current_package;
    return boost::algorithm::to_lower_copy(boost::algorithm::hex(Crypto::sha256digest(stream.str())));
  }

  boost::shared_ptr<PackageInterface> makePackage(const std::string &branch_name_in, const std::string &refhash_in,
                                                  const std::string &treehub_in) {
    return boost::make_shared<PackageFake>(branch_name_in, refhash_in, treehub_in);
  }
};

#endif  // PACKAGEMANAGERFAKE_H_
