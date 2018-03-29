#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <iostream>

#include <openssl/ssl.h>

#include "aktualizr_secondary.h"
#include "aktualizr_secondary_config.h"
#include "aktualizr_secondary_discovery.h"
#include "aktualizr_secondary_ipc.h"
#ifdef OPCUA_SECONDARY_ENABLED
#include "aktualizr_secondary_opcua.h"
#endif
#include "utils.h"

#include "logging.h"

namespace bpo = boost::program_options;

void check_secondary_options(const bpo::options_description &description, const bpo::variables_map &vm) {
  if (vm.count("help") != 0) {
    std::cout << description << '\n';
    exit(EXIT_SUCCESS);
  }
  if (vm.count("version") != 0) {
    std::cout << "Current aktualizr-secondary version is: " << AKTUALIZR_VERSION << "\n";
    exit(EXIT_SUCCESS);
  }
}

bpo::variables_map parse_options(int argc, char *argv[]) {
  bpo::options_description description("aktualizr-secondary command line options");
  // clang-format off
  description.add_options()
      ("help,h", "print usage")
      ("version,v", "Current aktualizr-secondary version")
      ("loglevel", bpo::value<int>(), "set log level 0-4 (trace, debug, warning, info, error)")
      ("config,c", bpo::value<std::string>()->required(), "toml configuration file")
      ("server-port,p", bpo::value<int>(), "command server listening port")
      ("discovery-port,d", bpo::value<int>(), "discovery service listening port (0 to disable)")
      ("ecu-serial", bpo::value<std::string>(), "serial number of secondary ecu")
      ("ecu-hardware-id", bpo::value<std::string>(), "hardware ID of secondary ecu");
  // clang-format on

  bpo::variables_map vm;
  std::vector<std::string> unregistered_options;
  try {
    bpo::basic_parsed_options<char> parsed_options =
        bpo::command_line_parser(argc, argv).options(description).allow_unregistered().run();
    bpo::store(parsed_options, vm);
    check_secondary_options(description, vm);
    bpo::notify(vm);
    unregistered_options = bpo::collect_unrecognized(parsed_options.options, bpo::include_positional);
    if (vm.count("help") == 0 && !unregistered_options.empty()) {
      std::cout << description << "\n";
      exit(EXIT_FAILURE);
    }
  } catch (const bpo::required_option &ex) {
    if (ex.get_option_name() == "--config") {
      std::cout << ex.get_option_name() << " is missing.\nYou have to provide a valid configuration "
                                           "file using toml format. See the example configuration file "
                                           "in config/sota_secondary.toml"
                << std::endl;
      exit(EXIT_FAILURE);
    } else {
      // print the error and append the default commandline option description
      std::cout << ex.what() << std::endl << description;
      exit(EXIT_SUCCESS);
    }
  } catch (const bpo::error &ex) {
    check_secondary_options(description, vm);

    // log boost error
    LOG_WARNING << "boost command line option error: " << ex.what();

    // print the error message to the standard output too, as the user provided
    // a non-supported commandline option
    std::cout << ex.what() << '\n';

    // set the returnValue, thereby ctest will recognize
    // that something went wrong
    exit(EXIT_FAILURE);
  }

  return vm;
}

/*****************************************************************************/
int main(int argc, char *argv[]) {
  logger_init();

  bpo::variables_map commandline_map = parse_options(argc, argv);

  // check for loglevel
  if (commandline_map.count("loglevel") != 0) {
    // set the log level from command line option
    boost::log::trivial::severity_level severity =
        static_cast<boost::log::trivial::severity_level>(commandline_map["loglevel"].as<int>());
    if (severity < boost::log::trivial::trace) {
      LOG_DEBUG << "Invalid log level";
      severity = boost::log::trivial::trace;
    }
    if (boost::log::trivial::fatal < severity) {
      LOG_WARNING << "Invalid log level";
      severity = boost::log::trivial::fatal;
    }
    if (severity <= boost::log::trivial::debug) {
      SSL_load_error_strings();
    }
    logger_set_threshold(severity);
  }

  LOG_INFO << "Aktualizr-secondary version " AKTUALIZR_VERSION " starting";
  LOG_DEBUG << "Current directory: " << boost::filesystem::current_path().string();
  // Initialize config with default values, the update with config, then with cmd
  boost::filesystem::path secondary_config_path(commandline_map["config"].as<std::string>());
  if (!boost::filesystem::exists(secondary_config_path)) {
    std::cout << "aktualizr-secondary: configuration file " << boost::filesystem::absolute(secondary_config_path)
              << " not found. Exiting." << std::endl;
    exit(EXIT_FAILURE);
  }

  int ret = 0;
  std::thread discovery_thread;
  std::unique_ptr<AktualizrSecondaryDiscovery> discovery;
  try {
    AktualizrSecondaryConfig config(secondary_config_path, commandline_map);

    // storage (share class with primary)
    boost::shared_ptr<INvStorage> storage = INvStorage::newStorage(config.storage);

#ifdef OPCUA_SECONDARY_ENABLED
    // OPC-UA bridge server has integrated discovery
    AktualizrSecondaryOpcua secondary(config, storage);
#else
    AktualizrSecondary secondary(config, storage);

    // discovery service
    discovery.reset(new AktualizrSecondaryDiscovery(config.network, secondary));
    discovery_thread = std::thread(&AktualizrSecondaryDiscovery::run, discovery.get());
#endif

    secondary.run();

  } catch (std::runtime_error &exc) {
    LOG_ERROR << "Error: " << exc.what();

    ret = 1;
  }

  if (discovery != nullptr) {
    try {
      discovery->stop();
      discovery_thread.join();
    } catch (std::runtime_error &exc) {
      LOG_ERROR << "Error: " << exc.what();
      ret = 1;
    }
  }

  return ret;
}
