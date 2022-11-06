#include "JobSystem.h"

JobSystem::JobSystem() {

}
JobSystem::~JobSystem() {

}
JobSystem& JobSystem::getInstance() {
	static JobSystem gJobSystem;
	return gJobSystem;
}