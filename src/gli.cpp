#include "gli/gli.h"
#include <array>
#include <cmath>

namespace gli::deleters {
	auto buffer(GLuint h) -> void { glDeleteBuffers(1, &h); }

	auto texture(GLuint h) -> void { glDeleteTextures(1, &h); }

	auto vertex_array(GLuint h) -> void { glDeleteVertexArrays(1, &h); }

	auto framebuffer(GLuint h) -> void { glDeleteFramebuffers(1, &h); }

	auto render_buffer(GLuint h) -> void { glDeleteRenderbuffers(1, &h); }

	auto sampler(GLuint h) -> void { glDeleteSamplers(1, &h); }

	auto query(GLuint h) -> void { glDeleteQueries(1, &h); }

	auto program(GLuint h) -> void { glDeleteProgram(h); }

	auto shader(GLuint h) -> void { glDeleteShader(h); }

	auto sync_object(GLsync h) -> void { glDeleteSync(h); }
}
namespace gli {
	auto buffer::create_handle() -> void {
		GLuint handle;
		glCreateBuffers(1, &handle);

		if (!glIsBuffer(handle)) {
			return;
		}

		reset(handle);
	}

	auto buffer::create_storage(GLenum buffer_flags, GLsizei capacity, const void* data) -> void {
		const auto handle = get();
		glNamedBufferStorage(
			handle,
			capacity,
			data,
			buffer_flags
		);
	}
}
namespace gli {
	auto texture::create_handle(GLenum texture_type) -> void {
		GLuint handle;
		glCreateTextures(texture_type, 1, &handle);

		if (!glIsTexture(handle)) {
			return;
		}

		reset(handle);
	}

	auto texture::create_handle_as_view(
		GLuint source_handle,
		GLenum target,
		GLenum internal_format,
		GLuint min_level,
		GLuint num_levels,
		GLuint min_layer,
		GLuint num_layers
	) -> void {
		GLuint handle;
		glGenTextures(1, &handle);

		glTextureView(
			handle,
			target,
			source_handle,
			internal_format,
			min_level,
			num_levels,
			min_layer,
			num_layers
		);
		if (!glIsTexture(handle)) {
			return;
		}

		reset(handle);
	}

	auto texture::create_storage(GLenum pixel_format, GLint x, GLint y, GLint z, GLint mip_count) -> void {
		// Scrapyard_Assert(x > 0);
		// Scrapyard_Assert(y > 0);
		// Scrapyard_Assert(z > 0);
		// Scrapyard_Assert(mip_count > 0);

		GLint texture_type = 0;
		const auto handle = get();
		glGetTextureParameteriv(handle, GL_TEXTURE_TARGET, &texture_type);

		switch (texture_type) {
			case GL_TEXTURE_1D: {
				glTextureStorage1D(
					handle,
					mip_count,
					pixel_format,
					x
				);
				break;
			}
			case GL_TEXTURE_1D_ARRAY:
			case GL_TEXTURE_2D:
			case GL_TEXTURE_CUBE_MAP : {
				glTextureStorage2D(
					handle,
					mip_count,
					pixel_format,
					x,
					y
				);
				break;
			}
			case GL_TEXTURE_2D_ARRAY:
			case GL_TEXTURE_CUBE_MAP_ARRAY:
			case GL_TEXTURE_3D: {
				glTextureStorage3D(
					handle,
					mip_count,
					pixel_format,
					x,
					y,
					z
				);
				break;
			}
			default: break;
		}
	}
}
namespace gli {
	auto renderbuffer::create_handle() -> void {
		GLuint handle;
		glCreateRenderbuffers(1, &handle);

		if (!glIsRenderbuffer(handle)) {
			return;
		}

		reset(handle);
	}

	auto renderbuffer::create_storage(GLenum pixel_format, GLint x, GLint y) -> void {
		// Scrapyard_Assert(dimensions.x > 0);
		// Scrapyard_Assert(dimensions.y > 0);

		const auto handle = get();

		glNamedRenderbufferStorage(
			handle,
			pixel_format,
			x,
			y
		);
	}
}
namespace gli {
	auto vertex_array::create_handle() -> void {
		GLuint handle;
		glCreateVertexArrays(1, &handle);

		if (!glIsVertexArray(handle)) {
			return;
		}

		reset(handle);
	}

	auto vertex_array::set_index_buffer(const buffer& buffer) -> void {
		glVertexArrayElementBuffer(get(), buffer.get());
	}

	auto vertex_array::set_vertex_buffer(
		GLint attribute_index,
		const buffer& buffer,
		std::span<const GLenum> elements,
		GLuint buffer_offset,
		GLuint vertex_advance_rate
	) -> GLint {
		const auto vertex_array_handle = get();
		const auto buffer_handle = buffer.get();
		GLint buffer_relative_offset = 0;

		auto buffer_binding_point = attribute_index;
		for (GLenum element_type: elements) {
			const auto element_underlying_type = get_type_underlying_type(element_type);
			const auto element_used_attribute_count = get_type_used_attribute_count(element_type);
			const auto element_component_count = get_type_component_count(element_type);
			const auto element_byte_size = element_component_count * get_type_byte_size(element_underlying_type);

			glVertexArrayAttribBinding(
				vertex_array_handle,
				attribute_index,
				buffer_binding_point
			);

			glVertexArrayBindingDivisor(
				vertex_array_handle,
				attribute_index,
				vertex_advance_rate
			);

			for (GLint used_attribute_index = 0;
				 used_attribute_index < element_used_attribute_count;
				 ++used_attribute_index) {

				glEnableVertexArrayAttrib(
					vertex_array_handle,
					attribute_index
				);

				if (is_type_integer_format(element_type)) {
					glVertexArrayAttribIFormat(
						vertex_array_handle,
						attribute_index,
						element_component_count,
						element_underlying_type,
						buffer_relative_offset
					);
				} else if (is_type_floating_or_fixed_point_format(element_type)) {
					const auto buffer_element_normalized = GL_FALSE;
					glVertexArrayAttribFormat(
						vertex_array_handle,
						attribute_index,
						element_component_count,
						element_underlying_type,
						buffer_element_normalized,
						buffer_relative_offset
					);
				} else if (is_type_double_format(element_type)) {
					glVertexArrayAttribLFormat(
						vertex_array_handle,
						attribute_index,
						element_component_count,
						element_underlying_type,
						buffer_relative_offset
					);
				}

				++attribute_index;
				buffer_relative_offset += element_byte_size;
			}
		}
		glVertexArrayVertexBuffer(
			vertex_array_handle,
			buffer_binding_point,
			buffer_handle,
			buffer_offset,
			buffer_relative_offset
		);

		return attribute_index;
	}

	auto vertex_array::set_vertex_buffer(
		GLint attribute_index,
		const buffer& buffer,
		GLenum element_type,
		GLuint buffer_offset,
		GLuint vertex_advance_rate
	) -> GLint {
		const auto vertex_array_handle = get();
		const auto buffer_handle = buffer.get();

		GLint buffer_relative_offset = 0;
		auto buffer_binding_point = attribute_index;
		const auto element_underlying_type = get_type_underlying_type(element_type);
		const auto element_used_attribute_count = get_type_used_attribute_count(element_type);
		const auto element_component_count = get_type_component_count(element_type);
		const auto element_byte_size = element_component_count * get_type_byte_size(element_underlying_type);

		glVertexArrayAttribBinding(
			vertex_array_handle,
			attribute_index,
			buffer_binding_point
		);

		glVertexArrayBindingDivisor(
			vertex_array_handle,
			attribute_index,
			vertex_advance_rate
		);

		for (GLint used_attribute_index = 0;
			 used_attribute_index < element_used_attribute_count;
			 ++used_attribute_index) {

			glEnableVertexArrayAttrib(
				vertex_array_handle,
				attribute_index
			);

			if (is_type_integer_format(element_type)) {
				glVertexArrayAttribIFormat(
					vertex_array_handle,
					attribute_index,
					element_component_count,
					element_underlying_type,
					buffer_relative_offset
				);
			} else if (is_type_floating_or_fixed_point_format(element_type)) {
				const auto buffer_element_normalized = GL_FALSE;
				glVertexArrayAttribFormat(
					vertex_array_handle,
					attribute_index,
					element_component_count,
					element_underlying_type,
					buffer_element_normalized,
					buffer_relative_offset
				);
			} else if (is_type_double_format(element_type)) {
				glVertexArrayAttribLFormat(
					vertex_array_handle,
					attribute_index,
					element_component_count,
					element_underlying_type,
					buffer_relative_offset
				);
			}

			++attribute_index;
			buffer_relative_offset += element_byte_size;
		}

		glVertexArrayVertexBuffer(
			vertex_array_handle,
			buffer_binding_point,
			buffer_handle,
			buffer_offset,
			buffer_relative_offset
		);

		return attribute_index;
	}

	auto vertex_array::get_type_underlying_type(GLenum type) -> GLenum {
		switch (type) {
			case GL_FLOAT:
			case GL_FLOAT_VEC2:
			case GL_FLOAT_VEC3:
			case GL_FLOAT_VEC4:

			case GL_FLOAT_MAT2:
			case GL_FLOAT_MAT3:
			case GL_FLOAT_MAT4: return GL_FLOAT;

			case GL_DOUBLE:
			case GL_DOUBLE_VEC2:
			case GL_DOUBLE_VEC3:
			case GL_DOUBLE_VEC4:

			case GL_DOUBLE_MAT2:
			case GL_DOUBLE_MAT3:
			case GL_DOUBLE_MAT4: return GL_DOUBLE;

			case GL_INT:
			case GL_INT_VEC2:
			case GL_INT_VEC3:
			case GL_INT_VEC4: return GL_INT;

			case GL_UNSIGNED_INT:
			case GL_UNSIGNED_INT_VEC2:
			case GL_UNSIGNED_INT_VEC3:
			case GL_UNSIGNED_INT_VEC4: return GL_UNSIGNED_INT;

			default: return GL_NONE;
		}
	}

	auto vertex_array::get_type_used_attribute_count(GLenum type) -> GLint {
		switch (type) {
			case GL_FLOAT:
			case GL_INT:
			case GL_UNSIGNED_INT:
			case GL_DOUBLE:

			case GL_FLOAT_VEC2:
			case GL_INT_VEC2:
			case GL_UNSIGNED_INT_VEC2:
			case GL_DOUBLE_VEC2:

			case GL_FLOAT_VEC3:
			case GL_INT_VEC3:
			case GL_UNSIGNED_INT_VEC3:
			case GL_DOUBLE_VEC3:

			case GL_FLOAT_VEC4:
			case GL_INT_VEC4:
			case GL_UNSIGNED_INT_VEC4:
			case GL_DOUBLE_VEC4: return 1;

			case GL_DOUBLE_MAT2:
			case GL_FLOAT_MAT2: return 2;

			case GL_DOUBLE_MAT3:
			case GL_FLOAT_MAT3: return 3;

			case GL_DOUBLE_MAT4:
			case GL_FLOAT_MAT4: return 4;
			default: return 0;
		}
	}

	auto vertex_array::get_type_component_count(GLenum type) -> GLint {
		switch (type) {
			case GL_FLOAT:
			case GL_INT:
			case GL_UNSIGNED_INT:
			case GL_DOUBLE: return 1;

			case GL_FLOAT_VEC2:
			case GL_INT_VEC2:
			case GL_UNSIGNED_INT_VEC2:
			case GL_DOUBLE_VEC2: return 2;

			case GL_FLOAT_VEC3:
			case GL_INT_VEC3:
			case GL_UNSIGNED_INT_VEC3:
			case GL_DOUBLE_VEC3: return 3;

			case GL_FLOAT_VEC4:
			case GL_INT_VEC4:
			case GL_UNSIGNED_INT_VEC4:
			case GL_DOUBLE_VEC4:

			case GL_FLOAT_MAT2:
			case GL_FLOAT_MAT3:
			case GL_FLOAT_MAT4:

			case GL_DOUBLE_MAT2:
			case GL_DOUBLE_MAT3:
			case GL_DOUBLE_MAT4: return 4;
			default: return 0;
		}
	}

	auto vertex_array::get_type_byte_size(GLenum type) -> GLint {
		switch (type) {
			case GL_INT: return sizeof(GLint);
			case GL_UNSIGNED_INT: return sizeof(GLuint);
			case GL_FLOAT: return sizeof(GLfloat);
			case GL_DOUBLE: return sizeof(GLdouble);

			default: break;
		}
		return 0;
	}

	auto vertex_array::is_type_integer_format(GLenum type) -> GLboolean {
		switch (type) {
			case GL_FLOAT:
			case GL_FLOAT_VEC2:
			case GL_FLOAT_VEC3:
			case GL_FLOAT_VEC4:

			case GL_FLOAT_MAT2:
			case GL_FLOAT_MAT3:
			case GL_FLOAT_MAT4: return GL_FALSE;

			case GL_DOUBLE:
			case GL_DOUBLE_VEC2:
			case GL_DOUBLE_VEC3:
			case GL_DOUBLE_VEC4:

			case GL_DOUBLE_MAT2:
			case GL_DOUBLE_MAT3:
			case GL_DOUBLE_MAT4: return GL_FALSE;

			case GL_INT:
			case GL_INT_VEC2:
			case GL_INT_VEC3:
			case GL_INT_VEC4: return GL_TRUE;

			case GL_UNSIGNED_INT:
			case GL_UNSIGNED_INT_VEC2:
			case GL_UNSIGNED_INT_VEC3:
			case GL_UNSIGNED_INT_VEC4: return GL_TRUE;

			default: return GL_FALSE;
		}
	}

	auto vertex_array::is_type_floating_or_fixed_point_format(GLenum type) -> GLboolean {
		switch (type) {
			case GL_FLOAT:
			case GL_FLOAT_VEC2:
			case GL_FLOAT_VEC3:
			case GL_FLOAT_VEC4:

			case GL_FLOAT_MAT2:
			case GL_FLOAT_MAT3:
			case GL_FLOAT_MAT4: return GL_TRUE;

			case GL_DOUBLE:
			case GL_DOUBLE_VEC2:
			case GL_DOUBLE_VEC3:
			case GL_DOUBLE_VEC4:

			case GL_DOUBLE_MAT2:
			case GL_DOUBLE_MAT3:
			case GL_DOUBLE_MAT4: return GL_FALSE;

			case GL_INT:
			case GL_INT_VEC2:
			case GL_INT_VEC3:
			case GL_INT_VEC4: return GL_FALSE;

			case GL_UNSIGNED_INT:
			case GL_UNSIGNED_INT_VEC2:
			case GL_UNSIGNED_INT_VEC3:
			case GL_UNSIGNED_INT_VEC4: return GL_FALSE;

			default: return GL_FALSE;
		}
	}

	auto vertex_array::is_type_double_format(GLenum type) -> GLboolean {
		switch (type) {
			case GL_FLOAT:
			case GL_FLOAT_VEC2:
			case GL_FLOAT_VEC3:
			case GL_FLOAT_VEC4:

			case GL_FLOAT_MAT2:
			case GL_FLOAT_MAT3:
			case GL_FLOAT_MAT4: return GL_FALSE;

			case GL_DOUBLE:
			case GL_DOUBLE_VEC2:
			case GL_DOUBLE_VEC3:
			case GL_DOUBLE_VEC4:

			case GL_DOUBLE_MAT2:
			case GL_DOUBLE_MAT3:
			case GL_DOUBLE_MAT4: return GL_TRUE;

			case GL_INT:
			case GL_INT_VEC2:
			case GL_INT_VEC3:
			case GL_INT_VEC4: return GL_FALSE;

			case GL_UNSIGNED_INT:
			case GL_UNSIGNED_INT_VEC2:
			case GL_UNSIGNED_INT_VEC3:
			case GL_UNSIGNED_INT_VEC4: return GL_FALSE;

			default: return GL_FALSE;
		}
	}
}
namespace gli {
	auto framebuffer::create_handle() -> void {
		GLuint handle;
		glCreateFramebuffers(1, &handle);

		if (!glIsFramebuffer(handle)) {
			return;
		}

		reset(handle);
	}

	auto framebuffer::set_render_target(
		GLint attachment,
		const texture& texture,
		GLint texture_mip_map_level
	) -> void {
		glNamedFramebufferTexture(
			get(),
			attachment,
			texture.get(),
			texture_mip_map_level
		);
	}

	auto framebuffer::set_render_target(
		GLint attachment,
		const texture& texture,
		GLint texture_mip_map_level,
		GLint texture_layer
	) -> void {
		glNamedFramebufferTextureLayer(
			get(),
			attachment,
			texture.get(),
			texture_mip_map_level,
			texture_layer
		);
	}

	auto framebuffer::set_render_target(
		GLint attachment,
		const renderbuffer& renderbuffer
	) -> void {
		glNamedFramebufferRenderbuffer(
			get(),
			attachment,
			GL_RENDERBUFFER,
			renderbuffer.get()
		);
	}

	auto framebuffer::set_draw_buffer(
		GLenum color_attachment
	) -> void {
		glNamedFramebufferDrawBuffer(
			get(),
			color_attachment
		);
	}

	auto framebuffer::set_draw_buffers(
		std::span<GLenum> color_attachments
	) -> void {
		glNamedFramebufferDrawBuffers(
			get(),
			static_cast<GLsizei>(color_attachments.size()),
			color_attachments.data()
		);
	}
}
namespace gli {
	auto sampler::create_handle() -> void {
		GLuint handle;
		glCreateSamplers(1, &handle);

		if (!glIsSampler(handle)) {
			return;
		}

		reset(handle);
	}

	auto sampler::set_filters(GLenum minification_filter, GLenum magnification_filter) -> void {
		const auto handle = get();
		glSamplerParameteri(handle, GL_TEXTURE_MIN_FILTER, minification_filter);
		glSamplerParameteri(handle, GL_TEXTURE_MAG_FILTER, magnification_filter);
	}

	auto sampler::set_wrapping(GLenum wrap_s, GLenum wrap_t, GLenum wrap_r) -> void {
		const auto handle = get();
		glSamplerParameteri(handle, GL_TEXTURE_WRAP_S, wrap_s);
		glSamplerParameteri(handle, GL_TEXTURE_WRAP_T, wrap_t);
		glSamplerParameteri(handle, GL_TEXTURE_WRAP_R, wrap_r);
	}

	auto sampler::set_anisotropy(GLfloat anisotropy_level) -> void {
		const auto handle = get();
		GLfloat max_anisotropy;
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &max_anisotropy);

		const auto clamped_anisotropy = std::min(anisotropy_level, max_anisotropy);
		glSamplerParameterf(handle, GL_TEXTURE_MAX_ANISOTROPY, clamped_anisotropy);
	}
}
namespace gli {
	auto query::create_handle(GLenum target) -> void {
		GLuint handle;
		glCreateQueries(target, 1, &handle);

		if (!glIsQuery(handle)) {
			return;
		}

		reset(handle);
	}
}
namespace gli {
	auto shader::create_handle(GLenum shader_type) -> void {
		auto handle = glCreateShader(shader_type);

		if (!glIsShader(handle)) {
			return;
		}

		reset(handle);
	}

	auto shader::compile(std::string_view source) -> void {
		if (source.empty()) {
			return;
		}

		std::array<std::string_view, 1> sources{source};
		compile(sources);
	}

	auto shader::compile(std::span<std::string_view> sources) -> void {
		if (sources.empty()) {
			return;
		}

		constexpr static auto max_source_count = 64;

		std::array<const char*, max_source_count> source_pointers{};
		std::array<GLint, max_source_count> source_sizes{};
		GLsizei source_offset = 0;

		const auto add_source = [&](std::string_view source) {
			if (source_offset >= max_source_count) {
				return;
			}

			source_pointers[source_offset] = source.data();
			source_sizes[source_offset] = static_cast<GLint>(source.size());
			++source_offset;
		};
		add_source("#version 460 core\n");

		const auto handle = get();
		for (const auto view: sources) {
			add_source(view);
		}

		glShaderSource(
			handle,
			source_offset,
			source_pointers.data(),
			source_sizes.data()
		);
		glCompileShader(handle);
	}

	auto shader::is_compiled_successfully() const -> GLboolean {
		const auto handle = get();
		GLint compile_status = 0;
		glGetShaderiv(handle, GL_COMPILE_STATUS, &compile_status);
		return compile_status == GL_TRUE;
	}

	auto shader::get_info_log() const -> std::string {
		std::string info_log;
		const auto handle = get();
		GLint max_length = 0;
		glGetShaderiv(handle, GL_INFO_LOG_LENGTH, &max_length);
		if (max_length > 0) {
			GLint written_size = 0;
			info_log.resize(max_length, '\0');
			glGetShaderInfoLog(handle, max_length, &written_size, info_log.data());
		}
		return info_log;
	}

	auto shader::get_source() const -> std::string {
		std::string source;
		const auto handle = get();
		GLint max_length = 0;
		glGetShaderiv(handle, GL_SHADER_SOURCE_LENGTH, &max_length);
		if (max_length > 0) {
			GLint written_size = 0;
			source.resize(max_length, '\0');
			glGetShaderSource(handle, max_length, &written_size, source.data());
		}
		return source;
	}
}
namespace gli {
	auto program::create_handle() -> void {
		auto handle = glCreateProgram();

		if (!glIsProgram(handle)) {
			return;
		}

		reset(handle);
	}

	auto program::link(std::span<shader> shaders) -> void {
		if (shaders.empty()) {
			return;
		}
		const auto handle = get();

		for (const auto& shader: shaders) {
			glAttachShader(handle, shader.get());
		}

		glLinkProgram(handle);

		for (const auto& shader: shaders) {
			glDetachShader(handle, shader.get());
		}
	}

	auto program::is_linked_successfully() const -> GLboolean {
		const auto handle = get();
		GLint link_status = 0;
		glGetProgramiv(handle, GL_LINK_STATUS, &link_status);
		return link_status == GL_TRUE;
	}

	auto program::get_info_log() -> std::string {
		const auto handle = get();

		std::string info_log;

		GLint max_length = 0;
		glGetProgramiv(handle, GL_INFO_LOG_LENGTH, &max_length);
		if (max_length > 0) {
			GLint written_size = 0;
			info_log.resize(max_length, '\0');
			glGetProgramInfoLog(handle, max_length, &written_size, info_log.data());
		}

		return info_log;
	}
}
namespace gli {
	auto sync_object::push_fence_sync() -> void {
		auto handle = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);

		if (!glIsSync(handle)) {
			return;
		}

		reset(handle);
	}

	auto sync_object::is_passed(GLuint64 timeout) const -> bool {
		const auto handle = get();
		if (handle == nullptr) {
			return true;
		}
		const auto result = glClientWaitSync(handle, GL_SYNC_FLUSH_COMMANDS_BIT, timeout);
		return result == GL_ALREADY_SIGNALED || result == GL_CONDITION_SATISFIED;

	}
}
namespace gli {
	static message_callback_t s_message_callback = nullptr;
}
namespace gli {
	auto initialize(GLADloadproc loader) -> bool {
		if (!gladLoadGLLoader(loader)) {
			return false;
		}

		return true;
	}

	auto set_debug_output(message_callback_t message_callback) -> void {
		if (message_callback != nullptr) {
			s_message_callback = message_callback;

			auto error_callback = [](
				GLuint source,
				GLuint type,
				GLuint id,
				GLuint severity,
				GLint length,
				const char* message,
				const void*
			) -> void {
				s_message_callback(std::string_view{message, static_cast<std::size_t>(length)});
			};

			glEnable(GL_DEBUG_OUTPUT);
			glDebugMessageCallback(error_callback, nullptr);

			glDebugMessageControl(
				GL_DONT_CARE,
				GL_DEBUG_TYPE_PUSH_GROUP,
				GL_DONT_CARE,
				0,
				nullptr,
				GL_FALSE
			);
			glDebugMessageControl(
				GL_DONT_CARE,
				GL_DEBUG_TYPE_POP_GROUP,
				GL_DONT_CARE,
				0,
				nullptr,
				GL_FALSE
			);
		} else {
			glDisable(GL_DEBUG_OUTPUT);
		}
	}

	auto is_extension_supported(extension extension) -> bool {
		static auto get_supported_extensions = []() -> std::array<bool, index_of(extension::count)> {
			std::array<bool, index_of(extension::count)> supported_extensions{};
			supported_extensions[index_of(extension::anisotropic_filtering)] =
				GLAD_GL_EXT_texture_filter_anisotropic > 0;
			supported_extensions[index_of(extension::s3tc_compression)] = GLAD_GL_EXT_texture_compression_s3tc > 0;
			supported_extensions[index_of(extension::sparse_texture)] = GLAD_GL_ARB_sparse_texture > 0;
			supported_extensions[index_of(extension::sparse_texture2)] = GLAD_GL_ARB_sparse_texture2 > 0;

			return supported_extensions;
		};

		static auto supported_extensions = get_supported_extensions();

		return supported_extensions[index_of(extension)];
	}

	auto compute_resolution_mip_count(GLint x, GLint y) -> GLint {
		return static_cast<GLint>(std::log2(std::max(x, y))) + 1;
	}

	auto offset_cast_voidptr(ptrdiff_t offset) -> void* {
		return reinterpret_cast<void*>(offset);
	}

	auto offset_cast_intptr(ptrdiff_t offset) -> GLintptr {
		return GLintptr{offset};
	}

	auto create_and_compile_program(
		std::string_view vertex_shader_source,
		std::string_view fragment_shader_source
	) -> std::optional<program> {
		std::array<shader, 2> shaders;
		shaders[0].create_handle(GL_VERTEX_SHADER);
		shaders[0].compile(vertex_shader_source);

		shaders[1].create_handle(GL_FRAGMENT_SHADER);
		shaders[1].compile(fragment_shader_source);

		if (!shaders[0].is_compiled_successfully()) {
			if (s_message_callback != nullptr) {
				s_message_callback(shaders[0].get_info_log());
			}
			return std::nullopt;
		}

		if (!shaders[1].is_compiled_successfully()) {
			if (s_message_callback != nullptr) {
				s_message_callback(shaders[1].get_info_log());
			}
			return std::nullopt;
		}

		program out;
		out.create_handle();
		out.link(shaders);

		if (!out.is_linked_successfully()) {
			if (s_message_callback != nullptr) {
				s_message_callback(out.get_info_log());
			}
			return std::nullopt;
		}

		return out;
	}

	auto create_fullscreen_pass_program(
		std::string_view fragment_shader_source
	) -> std::optional<program> {
		constexpr std::string_view vertex_shader_source{
			"layout(location = 0) out vec2 vUv;\n"
			"\n"
			"void main() {\n"
			"    const vec2 positions[3] = vec2[3](\n"
			"        vec2(-1.0, -1.0),\n"
			"        vec2( 3.0, -1.0),\n"
			"        vec2(-1.0,  3.0) \n"
			"    );\n"
			"\n"
			"    const vec2 position = positions[gl_VertexID];\n"
			"    gl_Position = vec4(position, 0.0, 1.0);\n"
			"\n"
			"    vUv = (position * 0.5) + 0.5;\n"
			"}"
		};

		return create_and_compile_program(
			vertex_shader_source,
			fragment_shader_source
		);
	}
}