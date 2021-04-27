#include "WifiFileConfig.hpp"

WifiFileConfig::WifiFileConfig(FIL& file) {
	// TODO Auto-generated constructor stub

}

WifiFileConfig::~WifiFileConfig() {
	// TODO Auto-generated destructor stub
}

const std::vector<APPassword>& WifiFileConfig::APPasswords() {
	return ap_passwords_;
}
