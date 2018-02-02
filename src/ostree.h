#ifndef OSTREE_H_
#define OSTREE_H_

#include <string>

#include <glib/gi18n.h>
#include <ostree-1/ostree.h>
#include <boost/shared_ptr.hpp>

#include "config.h"
#include "packagemanagerinterface.h"
#include "types.h"
#include "uptane/cryptokey.h"

class OstreePackage : public PackageInterface {
 public:
  OstreePackage(const std::string &ref_name_in, const std::string &refhash_in, const std::string &treehub_in);
  virtual ~OstreePackage() {}
  virtual data::InstallOutcome install(const data::PackageManagerCredentials &cred, const PackageConfig &config) const;
};

class OstreeManager : public PackageManagerInterface {
 public:
  OstreeManager(const PackageConfig &pconfig);
  virtual Json::Value getInstalledPackages();
  virtual std::string getCurrent();
  virtual boost::shared_ptr<PackageInterface> makePackage(const std::string &branch_name_in,
                                                          const std::string &refhash_in, const std::string &treehub_in);
  boost::shared_ptr<OstreeDeployment> getStagedDeployment();
  static boost::shared_ptr<OstreeSysroot> LoadSysroot(const boost::filesystem::path &path);
  static bool addRemote(OstreeRepo *repo, const std::string &remote, const std::string &url,
                        const data::PackageManagerCredentials &cred);

 private:
  boost::filesystem::path sysroot_dir;
  boost::filesystem::path packages_file;
};

#endif  // OSTREE_H_
