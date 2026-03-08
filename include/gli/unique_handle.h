#pragma once

#include <optional>

namespace gli {
	template<typename T, auto Deleter>
	struct unique_handle {
		using optional_t = std::optional<T>;

	public:
		unique_handle();
		explicit unique_handle(T value);
		unique_handle(const unique_handle&) = delete;
		unique_handle(unique_handle&& other) noexcept;
		~unique_handle();

	public:
		[[nodiscard]] auto get() const -> T;
		auto reset(optional_t value = std::nullopt) -> void;
		[[nodiscard]] auto release() -> optional_t;
		[[nodiscard]] auto is_valid() const -> bool;

	public:
		auto operator=(const unique_handle& other) noexcept -> unique_handle& = delete;
		auto operator=(unique_handle&& other) noexcept -> unique_handle&;
		auto operator=(T value) noexcept -> unique_handle&;

	private:
		std::optional<T> m_Value;
	};
}

namespace gli {
	template<typename T, auto Deleter>
	unique_handle<T, Deleter>::unique_handle()
		: m_Value{} { }

	template<typename T, auto Deleter>
	unique_handle<T, Deleter>::unique_handle(T value)
		: m_Value{value} { }

	template<typename T, auto Deleter>
	unique_handle<T, Deleter>::unique_handle(unique_handle&& other) noexcept
		: m_Value{other.release()} { }

	template<typename T, auto Deleter>
	unique_handle<T, Deleter>::~unique_handle() {
		reset();
	}

	template<typename T, auto Deleter>
	auto unique_handle<T, Deleter>::get() const -> T {
		return m_Value.value();
	}

	template<typename T, auto Deleter>
	auto unique_handle<T, Deleter>::reset(optional_t value) -> void {
		if (m_Value.has_value() && m_Value.value() != value) {
			Deleter(m_Value.value());
		}
		m_Value = value;
	}

	template<typename T, auto Deleter>
	auto unique_handle<T, Deleter>::release() -> optional_t {
		auto value = m_Value;
		m_Value.reset();
		return value;
	}

	template<typename T, auto Deleter>
	auto unique_handle<T, Deleter>::is_valid() const -> bool {
		return m_Value.has_value();
	}

	template<typename T, auto Deleter>
	auto unique_handle<T, Deleter>::operator=(unique_handle&& other) noexcept -> unique_handle& {
		if (this != &other) {
			reset(other.release());
		}
		return *this;
	}

	template<typename T, auto Deleter>
	auto unique_handle<T, Deleter>::operator=(T value) noexcept -> unique_handle& {
		reset(value);
		return *this;
	}
}
