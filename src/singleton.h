#pragma once

namespace util
{

	template<typename T>
	class singleton
	{
	public:
		singleton() = default;
		virtual ~singleton() = default;

		static T* instance()
		{
			// Since C++11 this is thread safe, yay, no more locks and crap
			static T* instance = new T();
			return instance;
		}

		// Disable copy as this is a singleton
		singleton &operator=(const singleton&) = delete;
		singleton(const singleton&) = delete;
	};

}