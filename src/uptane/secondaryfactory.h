#ifndef UPTANE_SECONDARYFACTORY_H_
#define UPTANE_SECONDARYFACTORY_H_

#include <boost/make_shared.hpp>
#include <boost/shared_ptr.hpp>

#include "logging.h"
#include "uptane/ipuptanesecondary.h"
#include "uptane/legacysecondary.h"
#include "uptane/opcuasecondary.h"
#include "uptane/secondaryconfig.h"
#include "uptane/secondaryinterface.h"
#include "uptane/virtualsecondary.h"

namespace Uptane {

class SecondaryFactory {
 public:
  static boost::shared_ptr<SecondaryInterface> makeSecondary(const SecondaryConfig& sconfig) {
    switch (sconfig.secondary_type) {
      case kVirtual:
        return boost::make_shared<VirtualSecondary>(sconfig);
        break;
      case kLegacy:
        return boost::make_shared<LegacySecondary>(sconfig);
        break;
      case kIpUptane:
        return boost::make_shared<IpUptaneSecondary>(sconfig);
      case kOpcuaUptane:
#ifdef OPCUA_SECONDARY_ENABLED
        return boost::make_shared<OpcuaSecondary>(sconfig);
#else
        LOG_ERROR << "Built with no OPC-UA secondary support";
        return boost::shared_ptr<SecondaryInterface>();  // NULL-equivalent
#endif
      default:
        LOG_ERROR << "Unrecognized secondary type: " << sconfig.secondary_type;
        return boost::shared_ptr<SecondaryInterface>();  // NULL-equivalent
    }
  }
};
}  // namespace Uptane

#endif  // UPTANE_SECONDARYFACTORY_H_
