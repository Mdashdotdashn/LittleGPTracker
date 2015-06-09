
#include "SubService.h"
#include "ServiceRegistry.h"

SubService::SubService(int fourCC) {
	fourCC_=fourCC ;
	ServiceRegistry::Instance()->Register(this) ;
} ;

SubService::~SubService() {
	ServiceRegistry::Instance()->Unregister(this) ;
} ;
