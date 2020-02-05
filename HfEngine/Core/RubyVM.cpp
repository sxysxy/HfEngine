#include <Core.h>

HFENGINE_NAMESPACE_BEGIN

thread_local std::unique_ptr<RubyVM> currentRubyVM;

HFENGINE_NAMESPACE_END