#pragma once

#include <string>
#include <string_view>
#include <span>
#include <glad/glad.h>

#include "unique_handle.h"

namespace gli::deleters {
	auto buffer(GLuint h) -> void;
	auto texture(GLuint h) -> void;
	auto vertex_array(GLuint h) -> void;
	auto framebuffer(GLuint h) -> void;
	auto render_buffer(GLuint h) -> void;
	auto sampler(GLuint h) -> void;
	auto query(GLuint h) -> void;
	auto program(GLuint h) -> void;
	auto shader(GLuint h) -> void;
	auto sync_object(GLsync h) -> void;
}
namespace gli {
	struct buffer : gli::unique_handle<GLuint, deleters::buffer> {
		using gli::unique_handle<GLuint, deleters::buffer>::unique_handle;

	public:
		auto create_handle() -> void;
		auto create_storage(GLenum buffer_flags, GLsizei capacity, const void* data = nullptr) -> void;
	};
}
namespace gli {
	struct texture : gli::unique_handle<GLuint, deleters::texture> {
		using gli::unique_handle<GLuint, deleters::texture>::unique_handle;

	public:
		auto create_handle(GLenum texture_type) -> void;
		auto create_handle_as_view(
			GLuint source_handle,
			GLenum target,
			GLenum internal_format,
			GLuint min_level,
			GLuint num_levels,
			GLuint min_layer,
			GLuint num_layers
		) -> void;
		auto create_storage(GLenum pixel_format, GLint x, GLint y, GLint z, GLint mip_count) -> void;
	};
}
namespace gli {
	struct renderbuffer : gli::unique_handle<GLuint, deleters::render_buffer> {
		using gli::unique_handle<GLuint, deleters::render_buffer>::unique_handle;

	public:
		auto create_handle() -> void;
		auto create_storage(GLenum pixel_format, GLint x, GLint y) -> void;
	};
}
namespace gli {
	struct vertex_array : gli::unique_handle<GLuint, deleters::vertex_array> {
		using gli::unique_handle<GLuint, deleters::vertex_array>::unique_handle;

	public:
		auto create_handle() -> void;
		auto set_index_buffer(const buffer& buffer) -> void;
		auto set_vertex_buffer(
			GLint attribute_index,
			const buffer& buffer,
			std::span<const GLenum> elements,
			GLuint buffer_offset = 0,
			GLuint vertex_advance_rate = 0
		) -> GLint;
		auto set_vertex_buffer(
			GLint attribute_index,
			const buffer& buffer,
			GLenum element_type,
			GLuint buffer_offset = 0,
			GLuint vertex_advance_rate = 0
		) -> GLint;

	private:
		static auto get_type_underlying_type(GLenum type) -> GLenum;
		static auto get_type_used_attribute_count(GLenum type) -> GLint;
		static auto get_type_component_count(GLenum type) -> GLint;
		static auto get_type_byte_size(GLenum type) -> GLint;
		static auto is_type_integer_format(GLenum type) -> GLboolean;
		static auto is_type_floating_or_fixed_point_format(GLenum type) -> GLboolean;
		static auto is_type_double_format(GLenum type) -> GLboolean;
	};
}
namespace gli {
	struct framebuffer : gli::unique_handle<GLuint, deleters::framebuffer> {
		using gli::unique_handle<GLuint, deleters::framebuffer>::unique_handle;

	public:
		auto create_handle() -> void;
		auto set_render_target(
			GLint attachment,
			const texture& texture,
			GLint texture_mip_map_level
		) -> void;
		auto set_render_target(
			GLint attachment,
			const texture& texture,
			GLint texture_mip_map_level,
			GLint texture_layer
		) -> void;
		auto set_render_target(
			GLint attachment,
			const renderbuffer& renderbuffer
		) -> void;
		auto set_draw_buffer(
			GLenum color_attachment
		) -> void;
		auto set_draw_buffers(
			std::span<GLenum> color_attachments
		) -> void;
	};
}
namespace gli {
	struct sampler : gli::unique_handle<GLuint, deleters::sampler> {
		using gli::unique_handle<GLuint, deleters::sampler>::unique_handle;

	public:
		auto create_handle() -> void;
		auto set_filters(GLenum minification_filter, GLenum magnification_filter) -> void;
		auto set_wrapping(GLenum wrap_s, GLenum wrap_t, GLenum wrap_r) -> void;
		auto set_anisotropy(GLfloat anisotropy_level) -> void;
	};
}
namespace gli {
	struct query : gli::unique_handle<GLuint, deleters::query> {
		using gli::unique_handle<GLuint, deleters::query>::unique_handle;

	public:
		auto create_handle(GLenum target) -> void;
	};
}
namespace gli {
	struct shader : gli::unique_handle<GLuint, deleters::shader> {
		using gli::unique_handle<GLuint, deleters::shader>::unique_handle;

	public:
		auto create_handle(GLenum shader_type) -> void;
		auto compile(std::string_view source) -> void;
		auto compile(std::span<std::string_view> sources) -> void;
		[[nodiscard]] auto is_compiled_successfully() const -> GLboolean;
		[[nodiscard]] auto get_info_log() const -> std::string;
		[[nodiscard]] auto get_source() const -> std::string;
	};
}
namespace gli {
	struct program : gli::unique_handle<GLuint, deleters::program> {
		using gli::unique_handle<GLuint, deleters::program>::unique_handle;

	public:
		auto create_handle() -> void;
		auto link(std::span<shader> shaders) -> void;
		[[nodiscard]] auto is_linked_successfully() const -> GLboolean;
		[[nodiscard]] auto get_info_log() -> std::string;
	};
}
namespace gli {
	struct sync_object : gli::unique_handle<GLsync, deleters::sync_object> {
		using gli::unique_handle<GLsync, deleters::sync_object>::unique_handle;

	public:
		auto push_fence_sync() -> void;
		[[nodiscard]] auto is_passed(GLuint64 timeout = 1) const -> bool;
	};
}
namespace gli {
	enum class extension {
		anisotropic_filtering = 0,
		s3tc_compression,
		sparse_texture,
		sparse_texture2,
		count
	};

	constexpr static auto index_of(extension extension) -> GLsizei {
		return static_cast<GLsizei>(extension);
	}
}
namespace gli {
	using message_callback_t = void (*)(std::string_view);
}
namespace gli {
	auto initialize(GLADloadproc loader) -> bool;
	auto set_debug_output(message_callback_t message_callback) -> void;
	auto is_extension_supported(extension extension) -> bool;

	auto compute_resolution_mip_count(GLint x, GLint y) -> GLint;
	auto offset_cast_voidptr(ptrdiff_t offset) -> void*;
	auto offset_cast_intptr(ptrdiff_t offset) -> GLintptr;

	auto create_and_compile_program(
		std::string_view vertex_shader_source,
		std::string_view fragment_shader_source
	) -> std::optional<program>;

	auto create_fullscreen_pass_program(
		std::string_view fragment_shader_source
	) -> std::optional<program>;
}