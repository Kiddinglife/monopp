#pragma once

#include "core/object.h"
#include <memory>

namespace mono
{
namespace managed_interface
{

#define register_basic_mono_converter_for_wrapper(type)                                                      \
	template <>                                                                                              \
	struct convert_mono_type<type>                                                                           \
	{                                                                                                        \
		using mono_type_name = MonoObject*;                                                                  \
                                                                                                             \
		static auto to_mono(type wrapper) -> mono_type_name                                                  \
		{                                                                                                    \
			return managed_interface::mono_object_wrapper<type>::create(std::move(wrapper))                  \
				.get_internal_ptr();                                                                         \
		}                                                                                                    \
                                                                                                             \
		static auto from_mono_unboxed(mono_type_name object) -> type                                         \
		{                                                                                                    \
			return managed_interface::mono_object_wrapper<type>::get_native_object(mono_object(object));     \
		}                                                                                                    \
                                                                                                             \
		static auto from_mono_boxed(MonoObject* object) -> type                                              \
		{                                                                                                    \
			return managed_interface::mono_object_wrapper<type>::get_native_object(mono_object(object));     \
		}                                                                                                    \
	}

template <typename T>
class mono_object_wrapper : public object
{
public:
	explicit mono_object_wrapper(const mono_object& mono_object, T obj);
	~mono_object_wrapper() final;

	/*!
	 * Create a new MonoObject and associate this wrapper to it.
	 */
	static auto create(T obj) -> mono_object;

	/*!
	 * Create a wrapper for an existing MonoObject.
	 */
	static void create(const mono_object& mono_object, T obj);

	static auto& get_native_object(const mono_object& mono_object);

private:
	T native_object;
};

template <typename T>
auto mono_object_wrapper<T>::create(T obj) -> mono_object
{
	auto instance = object::object_type->new_instance();
	create(instance, std::move(obj));
	return instance;
}

template <typename T>
void mono_object_wrapper<T>::create(const mono_object& mono_object, T obj)
{
	std::make_unique<mono_object_wrapper<T>>(mono_object, std::move(obj)).release();
}

template <typename T>
mono_object_wrapper<T>::mono_object_wrapper(const mono_object& mono_object, T obj)
	: object(mono_object)
	, native_object(std::move(obj))
{
}

template <typename T>
mono_object_wrapper<T>::~mono_object_wrapper() = default;

template <typename T>
auto& mono_object_wrapper<T>::get_native_object(const mono_object& mono_object)
{
	return object::get_managed_object_as<mono_object_wrapper<T>>(mono_object).native_object;
}

} // namespace managed_interface
} // namespace mono
