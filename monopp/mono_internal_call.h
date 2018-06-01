#pragma once

#include "mono_config.h"

#include "mono_assembly.h"
#include "mono_type_conversion.h"

BEGIN_MONO_INCLUDE
#include <mono/metadata/loader.h>
END_MONO_INCLUDE

namespace mono
{

template <typename F>
inline void add_internal_call(const std::string& name, F&& func)
{
	mono_add_internal_call(name.c_str(), reinterpret_cast<const void*>(func));
}

template <typename signature_t, signature_t& signature>
struct mono_jit_internal_call_wrapper;

template <typename... args_t, void (&func)(const mono_object&, args_t...)>
struct mono_jit_internal_call_wrapper<void(const mono_object&, args_t...), func>
{
	static void wrapper(MonoObject* obj,
						typename convert_mono_type<std::decay_t<args_t>>::mono_type_name... args)
	{
		func(mono_object(obj),
			 convert_mono_type<std::decay_t<args_t>>::from_mono_unboxed(std::move(args))...);
	}
};

template <typename return_t, typename... args_t, return_t (&func)(const mono_object&, args_t...)>
struct mono_jit_internal_call_wrapper<return_t(const mono_object&, args_t...), func>
{
	static typename convert_mono_type<std::decay_t<return_t>>::mono_type_name
	wrapper(MonoObject* obj, typename convert_mono_type<std::decay_t<args_t>>::mono_type_name... args)
	{
		return convert_mono_type<std::decay_t<return_t>>::to_mono(
			func(mono_object(obj),
				 convert_mono_type<std::decay_t<args_t>>::from_mono_unboxed(std::move(args))...));
	}
};

/*!
 * Wrap a non-static function for mono::add_internal_call, where automatic type
 * converstion is done through convert_mono_type. Add your own specialisation implementation
 * of this class to support more types.
 */
#define internal_call(func) &mono::mono_jit_internal_call_wrapper<decltype(func), func>::wrapper

} // namespace mono
