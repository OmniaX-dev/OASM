#ifndef __ENUMS__HPP__
#define __ENUMS__HPP__

namespace Omnia
{
	namespace common
	{
		enum class eErrorLevel {
			NoError = 0,
			Info,
			Suggestion,
			Warning,
			Error,
			Critical
		};
		
		enum class eNamespaceMemberType {
			Integer = 0,
			String,
			Color,
			Object,
			
			Null
		};
	}
}

#endif
